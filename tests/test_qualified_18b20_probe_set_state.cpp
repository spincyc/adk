#include <qualified_18b20_probe_set_policy.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace {
    constexpr uint32_t ownerToken   = 0x1842B20UL;
    constexpr uint16_t transportRev = 23;
    constexpr uint8_t  sourceId     = 7;
    constexpr uint16_t setRev       = 31;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
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
                    crc ^= 0x8CU;
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
        for (uint8_t index = 0; index < 8; ++index)
        {
            if (left.bytes[index] != right.bytes[index])
            {
                return false;
            }
        }
        return true;
    }

    adk::QualifiedDs18b20SetConfig config ()
    {
        const adk::Ds18b20Resolution resolutions[4] = {
            adk::Ds18b20Resolution::Bits9, adk::Ds18b20Resolution::Bits10,
            adk::Ds18b20Resolution::Bits11, adk::Ds18b20Resolution::Bits12};
        const uint32_t                 deadlines[4] = {94, 188, 375, 750};
        const adk::Ds18b20ProbeConfig  empty        = {rom (1),
                                                       adk::Ds18b20Resolution::Bits9,
                                                       adk::Duration (1),
                                                       adk::Duration (1),
                                                       0,
                                                       0,
                                                       0};
        adk::QualifiedDs18b20SetConfig value        = {
            sourceId, setRev, ownerToken, transportRev, {empty, empty, empty, empty}};
        for (uint8_t index = 0; index < 4; ++index)
        {
            value.probes[index] = {rom (static_cast<uint8_t> (index + 1)),
                                   resolutions[index],
                                   adk::Duration (deadlines[index]),
                                   adk::Duration (2000),
                                   -880,
                                   2000,
                                   320};
        }
        return value;
    }

    adk::OneWireSearchState emptySearch ()
    {
        return {{{0, 0, 0, 0, 0, 0, 0, 0}}, 0, false};
    }

    adk::OneWireTransactionSnapshot transaction (adk::OneWireOperation      operation,
                                                 const adk::OneWireRomCode& addressed,
                                                 uint32_t sequence, uint32_t generation,
                                                 uint32_t startedAt,
                                                 uint32_t completedAt)
    {
        const adk::OneWireOperationRequest request = {
            sequence,
            operation,
            addressed,
            emptySearch               (),
            adk::MicrosecondTimePoint (startedAt),
            adk::OneWireSupplyMode::ExternallyPowered,
            adk::StatusCode::Ok};
        return {operation,
                adk::OneWirePhase::Complete,
                adk::OneWireTransactionQuality::Complete,
                request,
                emptySearch (),
                {{0, 0, 0, 0, 0, 0, 0, 0}},
                {0, 0, 0, 0, 0, 0, 0, 0, 0},
                0,
                80,
                true,
                true,
                true,
                adk::MicrosecondTimePoint (completedAt),
                adk::StatusCode::Ok,
                ownerToken,
                9,
                transportRev,
                generation};
    }

    adk::OneWireTransactionSnapshot
    searchTransaction (const adk::OneWireSearchState& requestSearch,
                       const adk::OneWireRomCode& resultRom, bool lastDevice,
                       uint32_t sequence, uint32_t generation, uint32_t startedAt)
    {
        adk::OneWireTransactionSnapshot value = transaction (
            adk::OneWireOperation::SearchRomPass, {{0, 0, 0, 0, 0, 0, 0, 0}}, sequence,
            generation, startedAt, startedAt + 100);
        value.request.search    = requestSearch;
        value.searchResult      = {resultRom, 0, lastDevice};
        value.acceptedSlotCount = 200;
        return value;
    }

    uint8_t resolutionByte (adk::Ds18b20Resolution resolution)
    {
        switch (resolution)
        {
            case adk::Ds18b20Resolution::Bits9: return 0x1F;
            case adk::Ds18b20Resolution::Bits10: return 0x3F;
            case adk::Ds18b20Resolution::Bits11: return 0x5F;
            case adk::Ds18b20Resolution::Bits12: return 0x7F;
        }
        return 0;
    }

    adk::OneWireTransactionSnapshot
    scratchpadTransaction (const adk::OneWireRomCode& addressed,
                           adk::Ds18b20Resolution resolution, int16_t raw,
                           uint32_t sequence, uint32_t generation, uint32_t startedAt,
                           bool goodCrc = true)
    {
        adk::OneWireTransactionSnapshot value =
            transaction (adk::OneWireOperation::MatchRomReadScratchpad, addressed,
                         sequence, generation, startedAt, startedAt + 100);
        value.readByteCount     = 9;
        value.acceptedSlotCount = 152;
        value.readBytes[0]      = static_cast<uint8_t> (raw);
        value.readBytes[1] = static_cast<uint8_t> (static_cast<uint16_t> (raw) >> 8);
        value.readBytes[2] = 75;
        value.readBytes[3] = 70;
        value.readBytes[4] = resolutionByte (resolution);
        value.readBytes[5] = 0xFF;
        value.readBytes[6] = 0x0C;
        value.readBytes[7] = 0x10;
        value.readBytes[8] = crc8 (value.readBytes, 8);
        if (!goodCrc)
        {
            value.readBytes[8] ^= 0x01;
        }
        return value;
    }

    struct CycleSpec
    {
        uint32_t sequence;
        uint32_t observedAt;
        uint32_t transactionBase;
        uint32_t microsecondBase;
        uint32_t lifecycleGeneration;
        uint8_t  order[4];
        uint8_t  presentCount;
        int16_t  raw[4];
        uint8_t  badCrcMask;
    };

    void buildCycle (const adk::QualifiedDs18b20SetConfig&    configuration,
                     const adk::Qualified18B20ProbeSetPolicy& policy,
                     const CycleSpec& spec, adk::Ds18b20CycleBuilder& builder)
    {
        require (policy
                     .beginCycle (adk::TimePoint (spec.observedAt), sourceId, setRev,
                                  spec.sequence, adk::TimePoint (spec.observedAt),
                                  builder)
                     .ok (),
                 "cycle begins");

        adk::OneWireSearchState prior                 = emptySearch ();
        uint32_t                transactionSequence   = spec.transactionBase;
        uint32_t                transactionGeneration = spec.transactionBase;
        const uint32_t microsecondBase = spec.microsecondBase;
        for (uint8_t pass = 0; pass < spec.presentCount; ++pass)
        {
            const bool last = pass + 1 == spec.presentCount;
            const adk::OneWireRomCode found =
                spec.order[pass] < 4
                    ? configuration.probes[spec.order[pass]].rom
                    : rom (static_cast<uint8_t> (spec.order[pass] + 1U));
            const adk::OneWireTransactionSnapshot evidence =
                searchTransaction (prior, found, last, transactionSequence++,
                                   transactionGeneration++,
                                   microsecondBase + 1000U + pass * 200U);
            adk::OneWireTransactionSnapshot normalizedEvidence = evidence;
            normalizedEvidence.lifecycleGeneration = spec.lifecycleGeneration;
            require (policy.ingestSearchPass (builder, normalizedEvidence, prior).ok (),
                     "search pass ingests");
            prior = normalizedEvidence.searchResult;
        }
        require (policy.finishSearch (builder, true, false, adk::StatusCode::Ok).ok (),
                 "search finishes");

        uint8_t witnessedMask = 0;
        for (uint8_t pass = 0; pass < spec.presentCount; ++pass)
        {
            const uint8_t              slot      = spec.order[pass];
            if (slot >= 4 || (witnessedMask & (1U << slot)) != 0)
            {
                continue;
            }
            witnessedMask = static_cast<uint8_t> (witnessedMask | (1U << slot));
            const adk::OneWireRomCode& addressed = configuration.probes[slot].rom;
            const uint32_t conversionGeneration  = spec.sequence * 8U + slot + 1U;

            adk::OneWireTransactionSnapshot start =
                transaction (adk::OneWireOperation::MatchRomStartConversion, addressed,
                             transactionSequence++, transactionGeneration++,
                             microsecondBase + 3000U + pass * 1000U,
                             microsecondBase + 3100U + pass * 1000U);
            start.lifecycleGeneration = spec.lifecycleGeneration;
            start.acceptedSlotCount = 80;
            require (policy.ingestConversionStart (builder, conversionGeneration, start)
                         .ok (),
                     "conversion start ingests");

            adk::OneWireTransactionSnapshot complete =
                transaction (adk::OneWireOperation::MatchRomReadConversionStatus,
                             addressed, transactionSequence++, transactionGeneration++,
                             microsecondBase + 3200U + pass * 1000U,
                             microsecondBase + 3300U + pass * 1000U);
            complete.lifecycleGeneration = spec.lifecycleGeneration;
            complete.readByteCount     = 1;
            complete.readBytes[0]      = 1;
            complete.acceptedSlotCount = 1;
            require (
                policy.ingestConversionStatus (builder, conversionGeneration, complete)
                    .ok (),
                "conversion completion ingests");

            adk::OneWireTransactionSnapshot scratch = scratchpadTransaction (
                addressed, configuration.probes[slot].resolution, spec.raw[slot],
                transactionSequence++, transactionGeneration++,
                microsecondBase + 3400U + pass * 1000U,
                (spec.badCrcMask & (1U << slot)) == 0);
            scratch.lifecycleGeneration = spec.lifecycleGeneration;
            require (policy
                         .ingestScratchpad (builder, conversionGeneration,
                                            adk::TimePoint (spec.observedAt), scratch)
                         .ok (),
                     "scratchpad ingests");
        }
    }

    void buildNoWitness (const adk::Qualified18B20ProbeSetPolicy& policy,
                         uint32_t sequence, uint32_t observedAt,
                         adk::Ds18b20CycleBuilder& builder)
    {
        require (policy
                     .beginCycle (adk::TimePoint (observedAt), sourceId, setRev,
                                  sequence, adk::TimePoint (observedAt), builder)
                     .ok (),
                 "no-witness cycle begins");
        require (policy
                     .finishSearch (builder, false, false,
                                    adk::StatusCode::HardwareFailure)
                     .ok (),
                 "no-witness producer fault finishes search");
    }

    adk::QualifiedDs18b20Snapshot finalize (adk::Qualified18B20ProbeSetPolicy& policy,
                                            const adk::Ds18b20CycleBuilder&    builder,
                                            uint32_t now, bool expectOk = true)
    {
        adk::QualifiedDs18b20Snapshot value;
        const adk::Status             status =
            policy.finalizeCycle (adk::TimePoint (now), builder, value);
        require (status.ok () == expectOk, "finalization status matches fixture");
        return value;
    }

    bool sameSnapshot (const adk::QualifiedDs18b20Snapshot& left,
                       const adk::QualifiedDs18b20Snapshot& right)
    {
        if (left.sourceId != right.sourceId ||
            left.configurationRevision != right.configurationRevision ||
            left.cycleSequence != right.cycleSequence ||
            left.observedAt != right.observedAt ||
            left.validCount != right.validCount ||
            left.presentMask != right.presentMask ||
            left.faultMask != right.faultMask || left.quality != right.quality ||
            left.status != right.status)
        {
            return false;
        }
        for (uint8_t index = 0; index < 4; ++index)
        {
            const adk::QualifiedDs18b20Probe& a = left.probes[index];
            const adk::QualifiedDs18b20Probe& b = right.probes[index];
            if (!sameRom (a.rom, b.rom) || a.cycleSequence != b.cycleSequence ||
                a.conversionGeneration != b.conversionGeneration ||
                a.readTransactionGeneration != b.readTransactionGeneration ||
                a.observedAt != b.observedAt || a.rawSixteenths != b.rawSixteenths ||
                a.lowerRawSixteenths != b.lowerRawSixteenths ||
                a.upperRawSixteenths != b.upperRawSixteenths ||
                a.resolution != b.resolution || a.quality != b.quality ||
                a.age != b.age || a.status != b.status)
            {
                return false;
            }
        }
        return true;
    }

    CycleSpec cycle (uint32_t sequence, uint32_t observedAt)
    {
        const uint32_t transactionBase =
            sequence <= 1000U ? (sequence - 1U) * 16U + 1U : 1U;
        return {sequence,
                observedAt,
                transactionBase,
                (transactionBase - 1U) * 1000U,
                9,
                {0, 1, 2, 3},
                4,
                {160, 320, -80, 400},
                0};
    }

    void requireConfiguredOrder (const adk::QualifiedDs18b20SetConfig& configuration,
                                 const adk::QualifiedDs18b20Snapshot&  snapshot)
    {
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (
                sameRom (snapshot.probes[index].rom, configuration.probes[index].rom),
                "snapshot preserves configured ROM order");
        }
    }
} // namespace

int main ()
{
    const adk::QualifiedDs18b20SetConfig configuration = config ();

    {
        adk::Qualified18B20ProbeSetPolicy forward (configuration);
        adk::Qualified18B20ProbeSetPolicy reverse (configuration);
        require                                   (forward.initialize ().ok () && reverse.initialize ().ok (),
                 "ordering policies initialize");

        CycleSpec first  = cycle (1, 100);
        CycleSpec second = first;
        second.order[0]  = 3;
        second.order[1]  = 2;
        second.order[2]  = 1;
        second.order[3]  = 0;
        adk::Ds18b20CycleBuilder a;
        adk::Ds18b20CycleBuilder b;
        buildCycle                                           (configuration, forward, first, a);
        buildCycle                                           (configuration, reverse, second, b);
        const adk::QualifiedDs18b20Snapshot left  = finalize (forward, a, 100);
        const adk::QualifiedDs18b20Snapshot right = finalize (reverse, b, 100);
        requireConfiguredOrder                               (configuration, left);
        requireConfiguredOrder                               (configuration, right);
        require                                              (left.validCount == 4 && right.validCount == 4 &&
                     left.presentMask == 0x0F && right.presentMask == 0x0F,
                 "whole cycles publish four current configured slots");
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (left.probes[index].rawSixteenths ==
                         right.probes[index].rawSixteenths,
                     "search permutation does not change slot value");
        }
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (),
                 "global transaction ordering policy initializes");
        CycleSpec                first = cycle (1, 100);
        adk::Ds18b20CycleBuilder initial;
        buildCycle (configuration, policy, first, initial);
        const adk::QualifiedDs18b20Snapshot committed =
            finalize (policy, initial, 100);

        CycleSpec reused          = cycle (2, 200);
        reused.transactionBase    = 16;
        adk::Ds18b20CycleBuilder overlapping;
        buildCycle (configuration, policy, reused, overlapping);
        adk::QualifiedDs18b20Snapshot canary = committed;
        canary.validCount                    = 91;
        require (
            !policy.finalizeCycle (adk::TimePoint (200), overlapping, canary).ok (),
            "cross-cycle transaction generation reuse rejects");
        require (canary.validCount == 91,
                 "transaction-order rejection preserves caller output canary");
        adk::QualifiedDs18b20Snapshot retained;
        require (policy.snapshot (retained).ok () &&
                     sameSnapshot (retained, committed),
                 "transaction-order rejection preserves policy state");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (), "duplicate policy initializes");
        CycleSpec                spec = cycle    (4, 400);
        adk::Ds18b20CycleBuilder original;
        buildCycle (configuration, policy, spec, original);
        const adk::QualifiedDs18b20Snapshot committed =
            finalize (policy, original, 400);

        adk::Ds18b20CycleBuilder duplicate;
        buildCycle                                            (configuration, policy, spec, duplicate);
        const adk::QualifiedDs18b20Snapshot replay = finalize (policy, duplicate, 450);
        require                                               (sameSnapshot (committed, replay),
                 "identical duplicate at later policy time is idempotent");

        CycleSpec changedSpec = spec;
        changedSpec.order[0]  = 1;
        changedSpec.order[1]  = 0;
        adk::Ds18b20CycleBuilder changed;
        buildCycle (configuration, policy, changedSpec, changed);
        adk::QualifiedDs18b20Snapshot sentinel = replay;
        sentinel.validCount                    = 99;
        require (
            policy.finalizeCycle (adk::TimePoint (451), changed, sentinel).error () ==
                adk::StatusCode::InvalidArgument,
            "changed normalized duplicate rejects");
        require (sentinel.validCount == 99,
                 "changed duplicate leaves caller output untouched");
        adk::QualifiedDs18b20Snapshot retained;
        require (policy.snapshot (retained).ok () && sameSnapshot (retained, committed),
                 "changed duplicate leaves policy state untouched");

        CycleSpec changedProvenance = spec;
        changedProvenance.transactionBase++;
        adk::Ds18b20CycleBuilder provenance;
        buildCycle (configuration, policy, changedProvenance, provenance);
        sentinel.validCount = 98;
        require (
            !policy.finalizeCycle (adk::TimePoint (452), provenance, sentinel).ok (),
            "equal-cycle changed transaction provenance rejects");
        require (sentinel.validCount == 98,
                 "changed-provenance rejection preserves output canary");
        require (policy.snapshot (retained).ok () &&
                     sameSnapshot (retained, committed),
                 "changed-provenance rejection preserves policy state");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (), "ordering policy initializes");
        CycleSpec                maximum = cycle (UINT32_MAX, UINT32_MAX - 10U);
        maximum.transactionBase          = 1;
        adk::Ds18b20CycleBuilder beforeWrap;
        buildCycle (configuration, policy, maximum, beforeWrap);
        finalize   (policy, beforeWrap, UINT32_MAX - 10U);

        CycleSpec                wrapped = cycle (1, 5);
        wrapped.transactionBase          = 17;
        wrapped.microsecondBase          = 16000;
        adk::Ds18b20CycleBuilder afterWrap;
        buildCycle                                            (configuration, policy, wrapped, afterWrap);
        const adk::QualifiedDs18b20Snapshot result = finalize (policy, afterWrap, 5);
        require                                               (result.cycleSequence == 1,
                 "UINT32_MAX to one sequence and time rollover succeeds");

        CycleSpec                regression = cycle (UINT32_MAX, 6);
        adk::Ds18b20CycleBuilder old;
        buildCycle (configuration, policy, regression, old);
        adk::QualifiedDs18b20Snapshot sentinel = result;
        sentinel.validCount                    = 77;
        require (!policy.finalizeCycle (adk::TimePoint (6), old, sentinel).ok () &&
                     sentinel.validCount == 77,
                 "sequence regression rejects atomically");

        CycleSpec                ambiguous = cycle (0x80000001UL, 0x80000005UL);
        adk::Ds18b20CycleBuilder half;
        buildCycle (configuration, policy, ambiguous, half);
        require    (
            !policy.finalizeCycle (adk::TimePoint (0x80000005UL), half, sentinel).ok (),
            "exact-half-range ordering rejects");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (), "presence policy initializes");
        CycleSpec                initial = cycle (1, 100);
        adk::Ds18b20CycleBuilder all;
        buildCycle (configuration, policy, initial, all);
        finalize   (policy, all, 100);

        CycleSpec missing    = cycle (2, 200);
        missing.presentCount = 3;
        adk::Ds18b20CycleBuilder absent;
        buildCycle                                          (configuration, policy, missing, absent);
        const adk::QualifiedDs18b20Snapshot gone = finalize (policy, absent, 200);
        require                                             (gone.presentMask == 0x07 &&
                     gone.probes[3].quality == adk::Ds18b20ProbeQuality::Missing &&
                     gone.probes[3].rawSixteenths == initial.raw[3],
                 "missing slot retains its trusted value and configured index");

        CycleSpec returnSpec = cycle (3, 300);
        returnSpec.raw[3]    = 416;
        adk::Ds18b20CycleBuilder returned;
        buildCycle (configuration, policy, returnSpec, returned);
        const adk::QualifiedDs18b20Snapshot reappeared =
            finalize (policy, returned, 300);
        require (reappeared.presentMask == 0x0F &&
                     reappeared.probes[3].quality ==
                         adk::Ds18b20ProbeQuality::Current &&
                     reappeared.probes[3].rawSixteenths == 416,
                 "reappearing ROM resumes its stable configured slot");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (),
                 "carried observation policy initializes");
        CycleSpec                trusted = cycle (1, 1000);
        adk::Ds18b20CycleBuilder trustedBuilder;
        buildCycle (configuration, policy, trusted, trustedBuilder);
        const adk::QualifiedDs18b20Snapshot committed =
            finalize (policy, trustedBuilder, 1000);

        const uint32_t invalidTimes[2] = {200, 0x800003e8UL};
        for (uint8_t index = 0; index < 2; ++index)
        {
            CycleSpec missing = cycle (static_cast<uint32_t> (index + 2U),
                                       invalidTimes[index]);
            missing.presentCount = 1;
            missing.order[0]      = 4;
            adk::Ds18b20CycleBuilder builder;
            buildCycle (configuration, policy, missing, builder);
            adk::QualifiedDs18b20Snapshot canary = committed;
            canary.validCount                    = 81;
            require (!policy
                          .finalizeCycle (adk::TimePoint (invalidTimes[index]),
                                          builder, canary)
                          .ok () &&
                         canary.validCount == 81,
                     "carried observation backward and half-range reject atomically");
            adk::QualifiedDs18b20Snapshot retained;
            require (policy.snapshot (retained).ok () &&
                         sameSnapshot (retained, committed),
                     "carried observation rejection preserves hidden policy state");
        }
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (),
                 "cross-cycle microsecond policy initializes");
        CycleSpec                initial = cycle (1, 100);
        adk::Ds18b20CycleBuilder first;
        buildCycle (configuration, policy, initial, first);
        const adk::QualifiedDs18b20Snapshot committed =
            finalize (policy, first, 100);

        CycleSpec regressed       = cycle (2, 200);
        regressed.microsecondBase = 4000;
        adk::Ds18b20CycleBuilder early;
        buildCycle (configuration, policy, regressed, early);
        adk::QualifiedDs18b20Snapshot canary = committed;
        canary.validCount                    = 82;
        require (!policy.finalizeCycle (adk::TimePoint (200), early, canary).ok () &&
                     canary.validCount == 82,
                 "forward generations with regressed microsecond start reject");
        adk::QualifiedDs18b20Snapshot retained;
        require (policy.snapshot (retained).ok () &&
                     sameSnapshot (retained, committed),
                 "microsecond regression preserves policy state");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (), "precedence policy initializes");
        CycleSpec faults    = cycle              (1, 100);
        faults.presentCount = 3;
        faults.badCrcMask   = 0x02;
        adk::Ds18b20CycleBuilder builder;
        buildCycle                                            (configuration, policy, faults, builder);
        const adk::QualifiedDs18b20Snapshot result = finalize (policy, builder, 100);
        require                                               (result.probes[1].quality ==
                         adk::Ds18b20ProbeQuality::ScratchpadCrcFault &&
                     result.probes[3].quality == adk::Ds18b20ProbeQuality::Missing,
                 "simultaneous CRC and missing qualities both survive");
        require (result.status == result.probes[1].status,
                 "configured-slot precedence selects the earlier fault status");
    }

    {
        struct Collision
        {
            uint8_t               order[4];
            uint8_t               count;
            adk::Ds18b20SetQuality setQuality;
            adk::Ds18b20ProbeQuality slots[4];
            uint8_t               presentMask;
            uint8_t               validCount;
            uint8_t               faultMask;
        };
        const Collision collisions[] = {
            {{0, 0, 4, 1},
             4,
             adk::Ds18b20SetQuality::DuplicateIdentity,
             {adk::Ds18b20ProbeQuality::DuplicateIdentity,
              adk::Ds18b20ProbeQuality::ScratchpadCrcFault,
              adk::Ds18b20ProbeQuality::Missing,
              adk::Ds18b20ProbeQuality::Missing},
             0x03,
             0,
             0x0F},
            {{0, 4, 1, 1},
             3,
             adk::Ds18b20SetQuality::UnknownIdentity,
             {adk::Ds18b20ProbeQuality::Current,
              adk::Ds18b20ProbeQuality::ScratchpadCrcFault,
              adk::Ds18b20ProbeQuality::Missing,
              adk::Ds18b20ProbeQuality::Missing},
             0x03,
             1,
             0x0E}};

        for (const Collision& collision : collisions)
        {
            adk::Qualified18B20ProbeSetPolicy policy (configuration);
            require                                  (policy.initialize ().ok (),
                     "collision policy initializes");
            CycleSpec specification = cycle (1, 100);
            specification.presentCount = collision.count;
            specification.badCrcMask   = 0x02;
            for (uint8_t index = 0; index < 4; ++index)
            {
                specification.order[index] = collision.order[index];
            }
            adk::Ds18b20CycleBuilder builder;
            buildCycle (configuration, policy, specification, builder);
            const adk::QualifiedDs18b20Snapshot result =
                finalize (policy, builder, 100);
            require (result.quality == collision.setQuality &&
                         result.presentMask == collision.presentMask &&
                         result.validCount == collision.validCount &&
                         result.faultMask == collision.faultMask,
                     "collision table preserves set precedence and masks");
            for (uint8_t index = 0; index < 4; ++index)
            {
                require (result.probes[index].quality ==
                             collision.slots[index] &&
                             sameRom (result.probes[index].rom,
                                      configuration.probes[index].rom),
                         "collision table preserves every attributed slot");
            }
        }
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (), "reset policy initializes");
        CycleSpec                spec = cycle    (1, 100);
        adk::Ds18b20CycleBuilder builder;
        buildCycle   (configuration, policy, spec, builder);
        finalize     (policy, builder, 100);
        policy.reset ();
        adk::QualifiedDs18b20Snapshot reset;
        require                (policy.snapshot (reset).ok (), "snapshot succeeds after reset");
        requireConfiguredOrder (configuration, reset);
        require                (reset.validCount == 0 && reset.presentMask == 0 &&
                     reset.faultMask == 0x0F,
                 "reset clears evidence while retaining configured roles");
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (reset.probes[index].quality ==
                         adk::Ds18b20ProbeQuality::Unqualified,
                     "reset returns every slot to unqualified");
        }

        require                                      (policy.initialize ().ok (), "restart is idempotent after reset");
        CycleSpec                restartSpec = cycle (1, 200);
        adk::Ds18b20CycleBuilder restart;
        buildCycle (configuration, policy, restartSpec, restart);
        require    (finalize (policy, restart, 200).validCount == 4,
                 "reset permits a fresh sequence-one cycle");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (),
                 "hidden anchor policy initializes");
        CycleSpec                witnessed = cycle (1, 100);
        adk::Ds18b20CycleBuilder initial;
        buildCycle (configuration, policy, witnessed, initial);
        finalize   (policy, initial, 100);
        for (uint32_t sequence = 2; sequence <= 3; ++sequence)
        {
            adk::Ds18b20CycleBuilder gap;
            buildNoWitness (policy, sequence, sequence * 100U, gap);
            finalize       (policy, gap, sequence * 100U, false);
        }
        adk::QualifiedDs18b20Snapshot beforeReject;
        require (policy.snapshot (beforeReject).ok (), "hidden anchor state snapshots");

        CycleSpec changed               = cycle (4, 400);
        changed.transactionBase         = 1;
        changed.microsecondBase         = 0;
        changed.lifecycleGeneration     = 10;
        adk::Ds18b20CycleBuilder stale;
        buildCycle (configuration, policy, changed, stale);
        adk::QualifiedDs18b20Snapshot canary = beforeReject;
        canary.validCount                    = 83;
        require (!policy.finalizeCycle (adk::TimePoint (400), stale, canary).ok () &&
                     canary.validCount == 83,
                 "multiple no-witness gaps preserve hidden anchor rejection");
        adk::QualifiedDs18b20Snapshot retained;
        require (policy.snapshot (retained).ok () &&
                     sameSnapshot (retained, beforeReject),
                 "hidden anchor rejection is policy atomic");
    }

    {
        adk::Qualified18B20ProbeSetPolicy policy (configuration);
        require                                  (policy.initialize ().ok (),
                 "sentinel-free anchor policy initializes");
        adk::Ds18b20CycleBuilder gap;
        buildNoWitness (policy, 1, 100, gap);
        finalize       (policy, gap, 100, false);

        CycleSpec high             = cycle (2, 200);
        high.transactionBase       = 0x80000010UL;
        high.microsecondBase       = 0x80001000UL;
        adk::Ds18b20CycleBuilder witnessed;
        buildCycle (configuration, policy, high, witnessed);
        const adk::QualifiedDs18b20Snapshot committed =
            finalize (policy, witnessed, 200);
        require (committed.validCount == 4,
                 "first witness may establish a valid high-half anchor");

        CycleSpec regression       = cycle (3, 300);
        regression.transactionBase = 0x8000000fUL;
        regression.microsecondBase = 0x80000c18UL;
        adk::Ds18b20CycleBuilder old;
        buildCycle (configuration, policy, regression, old);
        adk::QualifiedDs18b20Snapshot canary = committed;
        canary.validCount                    = 84;
        require (!policy.finalizeCycle (adk::TimePoint (300), old, canary).ok () &&
                     canary.validCount == 84,
                 "regression after high-half anchor rejects atomically");
    }

    {
        adk::Qualified18B20ProbeSetPolicy first  (configuration);
        adk::Qualified18B20ProbeSetPolicy second (configuration);
        require                                  (first.initialize ().ok () && second.initialize ().ok (),
                 "twin replay policies initialize");
        CycleSpec specifications[3] = {cycle (1, 100), cycle (2, 200), cycle (3, 300)};
        specifications[1].presentCount = 3;
        specifications[2].order[0]     = 3;
        specifications[2].order[1]     = 1;
        specifications[2].order[2]     = 0;
        specifications[2].order[3]     = 2;
        specifications[2].raw[0]       = 176;

        for (const CycleSpec& specification : specifications)
        {
            adk::Ds18b20CycleBuilder a;
            adk::Ds18b20CycleBuilder b;
            buildCycle (configuration, first, specification, a);
            buildCycle (configuration, second, specification, b);
            const adk::QualifiedDs18b20Snapshot left =
                finalize (first, a, specification.observedAt);
            const adk::QualifiedDs18b20Snapshot right =
                finalize (second, b, specification.observedAt);
            require (sameSnapshot (left, right),
                     "full twin replay is byte-field identical");
        }
    }

    std::cout << "qualified 18B20 state tests passed\n";
    return EXIT_SUCCESS;
}
