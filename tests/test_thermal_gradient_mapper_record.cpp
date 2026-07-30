#include <thermal_gradient_mapper.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <type_traits>

namespace adk {
#if defined(ADK_TESTING)
    struct ThermalGradientMapperTestAccess
    {
        static void setNextRecordSequence (ThermalGradientMapper& mapper,
                                           uint32_t               value) noexcept
        {
            mapper.nextRecordSequence_ = value;
        }

        static void setRecordSequenceExhausted (ThermalGradientMapper& mapper,
                                                bool                   value) noexcept
        {
            mapper.recordSequenceExhausted_ = value;
        }
    };
#endif
} // namespace adk

namespace {
    constexpr uint32_t mapperOwner   = UINT32_C (0x31415926);
    constexpr uint16_t mapperRev     = 37;
    constexpr uint8_t  setSource     = 11;
    constexpr uint16_t setRev        = 41;
    constexpr uint32_t controlOwner  = UINT32_C (0x27182818);
    constexpr uint8_t  controlSource = 13;
    constexpr uint16_t controlRev    = 43;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    template <typename Value> void clearBytes (Value& value)
    {
        unsigned char* bytes = reinterpret_cast<unsigned char*> (&value);
        for (std::size_t index = 0; index < sizeof (value); ++index)
        {
            bytes[index] = 0;
        }
    }

    template <typename Value> void copyBytes (Value& destination, const Value& source)
    {
        unsigned char* destinationBytes =
            reinterpret_cast<unsigned char*> (&destination);
        const unsigned char* sourceBytes =
            reinterpret_cast<const unsigned char*> (&source);
        for (std::size_t index = 0; index < sizeof (destination); ++index)
        {
            destinationBytes[index] = sourceBytes[index];
        }
    }

    uint8_t crc8 (const uint8_t* bytes, uint8_t count)
    {
        uint8_t crc = 0;
        for (uint8_t index = 0; index < count; ++index)
        {
            uint8_t value = bytes[index];
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const bool mix = ((crc ^ value) & 1U) != 0;
                crc >>= 1;
                if (mix)
                {
                    crc ^= 0x8cU;
                }
                value >>= 1;
            }
        }
        return crc;
    }

    adk::OneWireRomCode rom (uint8_t serial)
    {
        adk::OneWireRomCode value = {{0x28, serial, 0x11, 0x22, 0x33, 0x44, 0x55, 0}};
        value.bytes[7]            = crc8 (value.bytes, 7);
        return value;
    }

    bool sameRom (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)
    {
        return std::memcmp (left.bytes, right.bytes, sizeof (left.bytes)) == 0;
    }

    adk::ThermalMapperConfig config (uint8_t spatialCount = 3)
    {
        adk::ThermalMapperConfig value = {mapperOwner,
                                          mapperRev,
                                          setSource,
                                          setRev,
                                          {rom (1), rom (2), rom (3), rom (4)},
                                          {rom (3), rom (1), rom (4), rom (2)},
                                          spatialCount,
                                          controlOwner,
                                          controlSource,
                                          controlRev,
                                          adk::Duration (50),
                                          8};
        for (uint8_t index = spatialCount; index < 4; ++index)
        {
            std::memset (value.spatialOrder[index].bytes, 0,
                         sizeof (value.spatialOrder[index].bytes));
        }
        return value;
    }

    adk::QualifiedDs18b20Probe probe (uint8_t slot, int16_t raw, uint32_t cycleSequence,
                                      uint32_t observedAt)
    {
        return {rom (static_cast<uint8_t> (slot + 1U)),
                cycleSequence,
                static_cast<uint32_t> (100U + slot),
                static_cast<uint32_t> (200U + slot),
                adk::TimePoint (observedAt),
                adk::TimePoint (observedAt + 100U),
                raw,
                static_cast<int16_t> (raw - 1),
                static_cast<int16_t> (raw + 1),
                static_cast<adk::Ds18b20Resolution> (slot),
                adk::Ds18b20ProbeQuality::Current,
                adk::Duration (0),
                adk::StatusCode::Ok};
    }

    adk::QualifiedDs18b20Snapshot probes (uint32_t cycleSequence = 7,
                                          uint32_t observedAt    = 100)
    {
        adk::QualifiedDs18b20Snapshot value;
        clearBytes (value);
        value.sourceId              = setSource;
        value.configurationRevision = setRev;
        value.cycleSequence         = cycleSequence;
        value.observedAt            = adk::TimePoint (observedAt);
        value.probes[0]             = probe          (0, 160, cycleSequence, observedAt);
        value.probes[1]             = probe          (1, 176, cycleSequence, observedAt);
        value.probes[2]             = probe          (2, 192, cycleSequence, observedAt);
        value.probes[3]             = probe          (3, 208, cycleSequence, observedAt);
        value.validCount            = 4;
        value.presentMask           = 0x0f;
        value.faultMask             = 0;
        value.quality               = adk::Ds18b20SetQuality::Complete;
        value.status                = adk::StatusCode::Ok;
        return value;
    }

    adk::ThermalMapperControl control (uint32_t sequence = 1, uint32_t observedAt = 100,
                                       bool recordEdge = false, bool nextEdge = false)
    {
        return {controlOwner,
                controlSource,
                controlRev,
                sequence,
                adk::TimePoint (observedAt),
                nextEdge,
                recordEdge,
                adk::StatusCode::Ok};
    }

    adk::ThermalMapperEnvelope envelope (uint32_t now = 100, uint32_t cycleSequence = 7,
                                         uint32_t controlSequence = 1,
                                         bool recordEdge = false, bool nextEdge = false)
    {
        return {adk::TimePoint (now), probes (cycleSequence, now),
                control (controlSequence, now, recordEdge, nextEdge)};
    }

    bool sameProbe (const adk::ThermalMapperRecordProbe& left,
                    const adk::ThermalMapperRecordProbe& right)
    {
        return sameRom (left.rom, right.rom) &&
               left.lowerRawSixteenths == right.lowerRawSixteenths &&
               left.upperRawSixteenths == right.upperRawSixteenths &&
               left.resolution == right.resolution && left.quality == right.quality &&
               left.age == right.age &&
               left.conversionGeneration == right.conversionGeneration &&
               left.readTransactionGeneration == right.readTransactionGeneration &&
               left.status == right.status;
    }

    bool sameGradient (const adk::ThermalGradientPair& left,
                       const adk::ThermalGradientPair& right)
    {
        return left.leftSlot == right.leftSlot && left.rightSlot == right.rightSlot &&
               left.lowerRawSixteenths == right.lowerRawSixteenths &&
               left.upperRawSixteenths == right.upperRawSixteenths &&
               left.quality == right.quality && left.faultMask == right.faultMask;
    }

    bool sameRecord (const adk::ThermalMapperRecordImage& left,
                     const adk::ThermalMapperRecordImage& right)
    {
        for (uint8_t index = 0; index < 4; ++index)
        {
            if (!sameProbe (left.probes[index], right.probes[index]))
            {
                return false;
            }
        }
        for (uint8_t index = 0; index < 3; ++index)
        {
            if (!sameGradient (left.gradients[index], right.gradients[index]))
            {
                return false;
            }
        }
        for (uint8_t index = 0; index < 4; ++index)
        {
            if (!sameRom (left.sourceRoms[index], right.sourceRoms[index]))
            {
                return false;
            }
        }
        return left.ownerToken == right.ownerToken &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.configurationRevision == right.configurationRevision &&
               left.recordSequence == right.recordSequence &&
               left.recordEdgeOwnerToken == right.recordEdgeOwnerToken &&
               left.recordEdgeSourceId == right.recordEdgeSourceId &&
               left.recordEdgeConfigurationRevision ==
                   right.recordEdgeConfigurationRevision &&
               left.recordEdgeSequence == right.recordEdgeSequence &&
               left.recordEdgeObservedAt == right.recordEdgeObservedAt &&
               left.setSourceId == right.setSourceId &&
               left.setConfigurationRevision == right.setConfigurationRevision &&
               left.setCycleSequence == right.setCycleSequence &&
               left.setObservedAt == right.setObservedAt &&
               left.mappedAt == right.mappedAt &&
               left.witnessDigest == right.witnessDigest &&
               left.formatVersion == right.formatVersion &&
               left.probeCount == right.probeCount &&
               left.gradientCount == right.gradientCount &&
               left.health == right.health && left.faultMask == right.faultMask;
    }

    bool sameResult (const adk::ThermalMapperResult& left,
                     const adk::ThermalMapperResult& right)
    {
        return std::memcmp (&left.intent, &right.intent, sizeof (left.intent)) == 0 &&
               sameRecord (left.record, right.record) &&
               left.hasRecord == right.hasRecord && left.status == right.status;
    }

    struct Hash
    {
        uint32_t value;

        Hash () : value (UINT32_C (0x811c9dc5))
        {
        }

        void byte (uint8_t input)
        {
            value = (value ^ input) * UINT32_C (0x01000193);
        }

        void u16 (uint16_t input)
        {
            byte (static_cast<uint8_t> (input));
            byte (static_cast<uint8_t> (input >> 8U));
        }

        void u32 (uint32_t input)
        {
            byte (static_cast<uint8_t> (input));
            byte (static_cast<uint8_t> (input >> 8U));
            byte (static_cast<uint8_t> (input >> 16U));
            byte (static_cast<uint8_t> (input >> 24U));
        }

        void s16 (int16_t input)
        {
            u16 (static_cast<uint16_t> (input));
        }

        void s32 (int32_t input)
        {
            u32 (static_cast<uint32_t> (input));
        }
    };

    uint32_t independentDigest (const adk::ThermalMapperRecordImage& record)
    {
        Hash       hash;
        const char domain[] = "ADK.THERMAL.MAPPER.RECORD.V1";
        for (const char character : domain)
        {
            if (character != '\0')
            {
                hash.byte (static_cast<uint8_t> (character));
            }
        }

        hash.byte (record.formatVersion);
        hash.u32  (record.ownerToken);
        hash.u32  (record.lifecycleGeneration);
        hash.u16  (record.configurationRevision);
        hash.u32  (record.recordSequence);
        hash.u32  (record.recordEdgeOwnerToken);
        hash.byte (record.recordEdgeSourceId);
        hash.u16  (record.recordEdgeConfigurationRevision);
        hash.u32  (record.recordEdgeSequence);
        hash.u32  (record.recordEdgeObservedAt.milliseconds ());
        hash.byte (record.setSourceId);
        hash.u16  (record.setConfigurationRevision);
        hash.u32  (record.setCycleSequence);
        hash.u32  (record.setObservedAt.milliseconds ());
        hash.u32  (record.mappedAt.milliseconds ());
        for (uint8_t index = 0; index < 4; ++index)
        {
            for (uint8_t byte = 0; byte < 8; ++byte)
            {
                hash.byte (record.sourceRoms[index].bytes[byte]);
            }
        }
        hash.byte (record.probeCount);
        for (uint8_t index = 0; index < record.probeCount; ++index)
        {
            const adk::ThermalMapperRecordProbe& probeValue = record.probes[index];
            for (uint8_t byte = 0; byte < 8; ++byte)
            {
                hash.byte (probeValue.rom.bytes[byte]);
            }
            hash.s16  (probeValue.lowerRawSixteenths);
            hash.s16  (probeValue.upperRawSixteenths);
            hash.byte (static_cast<uint8_t> (probeValue.resolution));
            hash.byte (static_cast<uint8_t> (probeValue.quality));
            hash.u32  (probeValue.age.milliseconds ());
            hash.u32  (probeValue.conversionGeneration);
            hash.u32  (probeValue.readTransactionGeneration);
            hash.byte (static_cast<uint8_t> (probeValue.status.error ()));
        }
        hash.byte (record.gradientCount);
        for (uint8_t index = 0; index < record.gradientCount; ++index)
        {
            const adk::ThermalGradientPair& gradient = record.gradients[index];
            hash.byte (gradient.leftSlot);
            hash.byte (gradient.rightSlot);
            hash.s32  (gradient.lowerRawSixteenths);
            hash.s32  (gradient.upperRawSixteenths);
            hash.byte (static_cast<uint8_t> (gradient.quality));
            hash.byte (gradient.faultMask);
        }
        hash.byte (static_cast<uint8_t> (record.health));
        hash.byte (record.faultMask);
        return hash.value;
    }

    adk::ThermalMapperResult update (adk::ThermalGradientMapper&       mapper,
                                     const adk::ThermalMapperEnvelope& input)
    {
        adk::ThermalMapperResult result;
        clearBytes (result);
        result.status            = adk::StatusCode::InternalInvariant;
        const adk::Status status = mapper.update (input, result);
        require                                  (status == result.status, "returned and published status agree");
        return result;
    }

    adk::ThermalMapperRecordImage
    hiddenReturnProbe (adk::ThermalMapperRecordImage value)
    {
        return value;
    }

    void testFixedAbiAndCallerCanaries ()
    {
        static_assert (std::is_standard_layout<adk::ThermalMapperRecordImage>::value,
                       "record image remains standard layout");
        static_assert (std::is_trivially_copyable<adk::ThermalMapperRecordImage>::value,
                       "record image remains trivially copyable");
        static_assert (sizeof (adk::ThermalMapperRecordImage) <= 384,
                       "record image remains within its hard ABI budget");
        static_assert (sizeof (adk::ThermalMapperResult) <= 512,
                       "host result remains bounded despite host alignment");
        typedef adk::ThermalMapperRecordImage (*HiddenReturnSignature) (
            adk::ThermalMapperRecordImage);
        HiddenReturnSignature hiddenReturn = &hiddenReturnProbe;
        require (hiddenReturn != nullptr, "record ABI supports hidden return probing");

        struct GuardedResult
        {
            uint32_t                 before;
            adk::ThermalMapperResult value;
            uint32_t                 after;
        };

        adk::ThermalGradientMapper mapper (config ());
        require                           (mapper.initialize (adk::TimePoint (90)).ok (), "mapper initializes");
        GuardedResult guarded;
        clearBytes                                        (guarded);
        guarded.before                         = UINT32_C (0x12345678);
        guarded.after                          = UINT32_C (0x89abcdef);
        const adk::ThermalMapperEnvelope input = envelope (100, 7, 1, true);
        require                                           (mapper.update (input, guarded.value).ok (), "record update succeeds");
        require                                           (guarded.before == UINT32_C (0x12345678) &&
                     guarded.after == UINT32_C (0x89abcdef),
                 "result fill preserves caller canaries");
        const adk::ThermalMapperRecordImage returned =
            hiddenReturn (guarded.value.record);
        require (sameRecord (returned, guarded.value.record),
                 "hidden-return probe preserves the complete record value");
    }

    void testGoldenRecordAndCanonicalUnusedCells ()
    {
        adk::ThermalGradientMapper mapper (config ());
        require                           (mapper.initialize (adk::TimePoint (90)).ok (), "mapper initializes");
        const adk::ThermalMapperResult result =
            update (mapper, envelope (100, 7, 1, true));
        require (result.status.ok () && result.hasRecord, "record edge emits record");

        const adk::ThermalMapperRecordImage& record = result.record;
        require (record.ownerToken == mapperOwner &&
                     record.configurationRevision == mapperRev &&
                     record.recordSequence == 1 &&
                     record.recordEdgeOwnerToken == controlOwner &&
                     record.recordEdgeSourceId == controlSource &&
                     record.recordEdgeConfigurationRevision == controlRev &&
                     record.recordEdgeSequence == 1 &&
                     record.recordEdgeObservedAt == adk::TimePoint (100) &&
                     record.setSourceId == setSource &&
                     record.setConfigurationRevision == setRev &&
                     record.setCycleSequence == 7 &&
                     record.setObservedAt == adk::TimePoint (100) &&
                     record.mappedAt == adk::TimePoint      (100) &&
                     record.formatVersion == 1 && record.probeCount == 3 &&
                     record.gradientCount == 2,
                 "golden record freezes top-level fields");
        require (sameRom (record.probes[0].rom, rom (3)) &&
                     sameRom (record.probes[1].rom, rom (1)) &&
                     sameRom (record.probes[2].rom, rom (4)),
                 "golden record uses configured spatial order");

        adk::ThermalMapperRecordProbe zeroProbe;
        clearBytes (zeroProbe);
        const adk::ThermalGradientPair zeroGradient = {};
        require (std::memcmp (&record.probes[3], &zeroProbe, sizeof (zeroProbe)) == 0,
                 "unused record probe is canonical zero");
        require (std::memcmp (&record.gradients[2], &zeroGradient,
                              sizeof (zeroGradient)) == 0,
                 "unused record gradient is canonical zero");
        require (record.witnessDigest == independentDigest (record),
                 "record digest matches independent explicit little-endian oracle");
        require (record.witnessDigest == UINT32_C (0x36d0f904),
                 "canonical record has the fixed V1 golden digest");
    }

    void testDigestCoversEveryAuthoritativeField ()
    {
        adk::ThermalGradientMapper mapper (config ());
        require                           (mapper.initialize (adk::TimePoint (90)).ok (), "mapper initializes");
        const adk::ThermalMapperRecordImage golden =
            update (mapper, envelope (100, 7, 1, true)).record;

#define REQUIRE_DIGEST_MUTATION(statement)                                             \
    do                                                                                 \
    {                                                                                  \
        adk::ThermalMapperRecordImage changed = golden;                                \
        statement;                                                                     \
        require (independentDigest (changed) != independentDigest (golden),            \
                 "digest covers " #statement);                                         \
    }                                                                                  \
    while (false)

        REQUIRE_DIGEST_MUTATION (changed.ownerToken ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.lifecycleGeneration ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.configurationRevision ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.recordSequence ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.recordEdgeOwnerToken ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.recordEdgeSourceId ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.recordEdgeConfigurationRevision ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.recordEdgeSequence ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.recordEdgeObservedAt = adk::TimePoint (101));
        REQUIRE_DIGEST_MUTATION (changed.setSourceId ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.setConfigurationRevision ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.setCycleSequence ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.setObservedAt = adk::TimePoint (101));
        REQUIRE_DIGEST_MUTATION (changed.mappedAt = adk::TimePoint (101));
        REQUIRE_DIGEST_MUTATION (changed.sourceRoms[0].bytes[0] ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.formatVersion ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.probeCount ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.gradientCount ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.health = adk::ThermalGradientHealth::Fault);
        REQUIRE_DIGEST_MUTATION (changed.faultMask ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].rom.bytes[0] ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].lowerRawSixteenths ^= 1);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].upperRawSixteenths ^= 1);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].resolution =
                                     adk::Ds18b20Resolution::Bits12);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].quality =
                                     adk::Ds18b20ProbeQuality::Stale);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].age = adk::Duration (1));
        REQUIRE_DIGEST_MUTATION (changed.probes[0].conversionGeneration ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].readTransactionGeneration ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.probes[0].status =
                                     adk::StatusCode::HardwareFailure);
        REQUIRE_DIGEST_MUTATION (changed.gradients[0].leftSlot ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.gradients[0].rightSlot ^= 1U);
        REQUIRE_DIGEST_MUTATION (changed.gradients[0].lowerRawSixteenths ^= 1);
        REQUIRE_DIGEST_MUTATION (changed.gradients[0].upperRawSixteenths ^= 1);
        REQUIRE_DIGEST_MUTATION (changed.gradients[0].quality =
                                     adk::ThermalGradientQuality::Fault);
        REQUIRE_DIGEST_MUTATION (changed.gradients[0].faultMask ^= 1U);
#undef REQUIRE_DIGEST_MUTATION
    }

    void testRecordEdgeExactlyOnceAndAtomicDuplicates ()
    {
        adk::ThermalGradientMapper mapper (config ());
        require                           (mapper.initialize (adk::TimePoint (90)).ok (), "mapper initializes");

        const adk::ThermalMapperResult noEdge =
            update (mapper, envelope (100, 7, 1, false));
        require (noEdge.status.ok () && !noEdge.hasRecord,
                 "accepted frame without edge emits no record");

        adk::ThermalMapperEnvelope edge = envelope       (101, 7, 2, true);
        edge.probes.observedAt          = adk::TimePoint (100);
        for (uint8_t index = 0; index < 4; ++index)
        {
            edge.probes.probes[index].observedAt   = adk::TimePoint (100);
            edge.probes.probes[index].freshThrough = adk::TimePoint (200);
        }
        const adk::ThermalMapperResult first = update (mapper, edge);
        require                                       (first.status.ok () && first.hasRecord &&
                     first.record.recordSequence == 1,
                 "fresh edge emits exactly first record");

        const adk::ThermalMapperResult replay = update (mapper, edge);
        require                                        (replay.status.ok () && !replay.hasRecord,
                 "byte-identical held edge replay emits no second record");

        adk::ThermalMapperEnvelope changed           = edge;
        changed.control.nextEdge                     = true;
        adk::ThermalMapperResult       canary        = first;
        const adk::ThermalMapperResult before        = canary;
        const adk::Status              changedStatus = mapper.update (changed, canary);
        require                                                      (!changedStatus.ok () && sameResult (canary, before),
                 "changed duplicate rejects without mutating caller result");

        adk::ThermalMapperEnvelope successor  = edge;
        successor.now                         = adk::TimePoint (102);
        successor.control.sequence            = 3;
        successor.control.observedAt          = adk::TimePoint (102);
        const adk::ThermalMapperResult second = update         (mapper, successor);
        require                                                (second.status.ok () && second.hasRecord &&
                     second.record.recordSequence == 2,
                 "next fresh edge emits exactly one successor record");
    }

    void testRecordIsPageIndependentAndFaultsAreExplicit ()
    {
        adk::ThermalGradientMapper overallMapper (config ());
        adk::ThermalGradientMapper pagedMapper   (config ());
        require                                  (overallMapper.initialize (adk::TimePoint (90)).ok () &&
                     pagedMapper.initialize (adk::TimePoint (90)).ok (),
                 "page comparison mappers initialize");

        adk::ThermalMapperEnvelope     firstInput = envelope (100, 7, 1, true, false);
        const adk::ThermalMapperResult first      = update   (overallMapper, firstInput);
        require                                              (first.hasRecord, "first record exists");

        adk::ThermalMapperEnvelope secondInput = firstInput;
        secondInput.control.nextEdge           = true;
        const adk::ThermalMapperResult second  = update (pagedMapper, secondInput);
        require                                         (second.hasRecord, "record with page edge exists");
        require                                         (second.record.probeCount == first.record.probeCount &&
                     second.record.gradientCount == first.record.gradientCount,
                 "full record cardinality is independent of selected page");
        for (uint8_t index = 0; index < first.record.probeCount; ++index)
        {
            require (
                sameProbe (second.record.probes[index], first.record.probes[index]),
                "page selection does not alter record probes");
        }
        for (uint8_t index = 0; index < first.record.gradientCount; ++index)
        {
            require (sameGradient (second.record.gradients[index],
                                   first.record.gradients[index]),
                     "page selection does not alter record gradients");
        }

        adk::ThermalGradientMapper mapper (config ());
        require                           (mapper.initialize (adk::TimePoint (90)).ok (),
                 "fault mapper initializes");
        adk::ThermalMapperEnvelope faultInput = envelope (104, 8, 1, true);
        faultInput.probes.probes[0].quality   = adk::Ds18b20ProbeQuality::Missing;
        faultInput.probes.probes[0].status    = adk::StatusCode::Ok;
        faultInput.probes.validCount          = 3;
        faultInput.probes.presentMask         = 0x0e;
        faultInput.probes.faultMask           = 0x01;
        faultInput.probes.quality             = adk::Ds18b20SetQuality::Missing;
        faultInput.probes.status              = adk::StatusCode::Ok;
        const adk::ThermalMapperResult fault  = update (mapper, faultInput);
        require                                        (fault.status.ok () && fault.hasRecord &&
                     fault.record.health == adk::ThermalGradientHealth::Fault &&
                     fault.record.faultMask != 0 &&
                     fault.record.probes[1].quality ==
                         adk::Ds18b20ProbeQuality::Missing,
                 "fault record retains mapped fault instead of numeric substitution");
        require (fault.record.probes[1].lowerRawSixteenths == 0 &&
                     fault.record.probes[1].upperRawSixteenths == 0,
                 "fault probe uses canonical zero numeric fields");
        require (
            fault.record.gradients[0].quality == adk::ThermalGradientQuality::Fault &&
                fault.record.gradients[1].quality == adk::ThermalGradientQuality::Fault,
            "interior mapped fault marks both incident gradients");
        require (fault.record.gradients[0].faultMask == 0x02 &&
                     fault.record.gradients[1].faultMask == 0x02,
                 "both incident gradients retain the exact mapped fault bit");
        require (fault.record.gradients[0].lowerRawSixteenths == 0 &&
                     fault.record.gradients[0].upperRawSixteenths == 0 &&
                     fault.record.gradients[1].lowerRawSixteenths == 0 &&
                     fault.record.gradients[1].upperRawSixteenths == 0,
                 "fault gradients use canonical zero numeric fields");
        require (fault.record.witnessDigest == independentDigest (fault.record),
                 "fault record digest covers explicit fault image");
    }

#if defined(ADK_TESTING)
    void testRecordSequenceExhaustion ()
    {
        adk::ThermalGradientMapper mapper                           (config ());
        require                                                     (mapper.initialize (adk::TimePoint (90)).ok (), "mapper initializes");
        adk::ThermalGradientMapperTestAccess::setNextRecordSequence (
            mapper, std::numeric_limits<uint32_t>::max ());

        const adk::ThermalMapperResult last =
            update (mapper, envelope (100, 7, 1, true));
        require (last.status.ok () && last.hasRecord &&
                     last.record.recordSequence ==
                         std::numeric_limits<uint32_t>::max (),
                 "maximum nonzero record sequence emits exactly once");

        adk::ThermalMapperEnvelope     next    = envelope (101, 8, 2, true);
        adk::ThermalMapperResult       guarded = last;
        const adk::ThermalMapperResult before  = guarded;
        const adk::Status              status  = mapper.update (next, guarded);
        require                                                (status.error () == adk::StatusCode::CapacityExceeded &&
                     sameResult (guarded, before),
                 "record sequence faults before zero and preserves caller image");

        adk::ThermalGradientMapperTestAccess::setRecordSequenceExhausted (mapper,
                                                                          false);
        adk::ThermalGradientMapperTestAccess::setNextRecordSequence (mapper, 1);
        const adk::ThermalMapperResult repaired = update            (mapper, next);
        require                                                     (repaired.status.ok () && repaired.hasRecord &&
                     repaired.record.recordSequence == 1,
                 "test seam proves exhaustion state is the rejecting cause");
    }
#endif

    void testFullAtomicReplayTrace ()
    {
        adk::ThermalGradientMapper left  (config ());
        adk::ThermalGradientMapper right (config ());
        require                          (left.initialize (adk::TimePoint (90)).ok () &&
                     right.initialize (adk::TimePoint (90)).ok (),
                 "replay mappers initialize");

        adk::ThermalMapperEnvelope trace[4] = {
            envelope (100, 7, 1, false, false), envelope (101, 8, 2, true, true),
            envelope (102, 8, 2, true, false), envelope (103, 9, 3, true, false)};
        trace[1].probes.probes[1].quality = adk::Ds18b20ProbeQuality::Stale;
        trace[1].probes.probes[1].status  = adk::StatusCode::Ok;
        trace[1].probes.faultMask         = 0x02;
        trace[1].probes.validCount        = 3;
        trace[2]                          = trace[1];
        trace[2].control.nextEdge         = false;
        trace[3].probes.probes[1]         = probe (1, 180, 9, 103);

        adk::ThermalMapperResult leftPrior;
        adk::ThermalMapperResult rightPrior;
        clearBytes (leftPrior);
        clearBytes (rightPrior);
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::ThermalMapperResult leftResult;
            adk::ThermalMapperResult rightResult;
            copyBytes                                    (leftResult, leftPrior);
            copyBytes                                    (rightResult, rightPrior);
            const adk::Status leftStatus  = left.update  (trace[index], leftResult);
            const adk::Status rightStatus = right.update (trace[index], rightResult);
            require                                      (leftStatus == rightStatus, "replay statuses match");
            if (leftStatus.ok                            ())
            {
                require (sameResult (leftResult, rightResult),
                         "accepted replay images are byte-equivalent");
                copyBytes (leftPrior, leftResult);
                copyBytes (rightPrior, rightResult);
            }
            else
            {
                require (sameResult (leftResult, leftPrior) &&
                             sameResult (rightResult, rightPrior),
                         "rejected replay step preserves both caller images");
            }
        }
    }
} // namespace

int main ()
{
    testFixedAbiAndCallerCanaries                   ();
    testGoldenRecordAndCanonicalUnusedCells         ();
    testDigestCoversEveryAuthoritativeField         ();
    testRecordEdgeExactlyOnceAndAtomicDuplicates    ();
    testRecordIsPageIndependentAndFaultsAreExplicit ();
#if defined(ADK_TESTING)
    testRecordSequenceExhaustion ();
#endif
    testFullAtomicReplayTrace ();
    return 0;
}
