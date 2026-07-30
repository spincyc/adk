#include "thermal_gradient_mapper.h"

#include <limits.h>
#include <string.h>

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        template <typename Value> void clearValue (Value& value) noexcept
        {
            uint8_t* bytes = reinterpret_cast<uint8_t*> (&value);
            for (size_t index = 0; index < sizeof value; ++index)
            {
                bytes[index] = 0;
            }
        }

        bool forward (uint32_t later, uint32_t earlier) noexcept
        {
            const uint32_t distance = later - earlier;
            return distance != 0 && distance < halfRange;
        }

        bool notBefore (uint32_t later, uint32_t earlier) noexcept
        {
            return later == earlier || forward (later, earlier);
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool equalRom (const OneWireRomCode& left, const OneWireRomCode& right) noexcept
        {
            return memcmp (left.bytes, right.bytes, sizeof left.bytes) == 0;
        }

        uint8_t crc8 (const uint8_t* bytes, uint8_t count) noexcept
        {
            uint8_t crc = 0;
            for (uint8_t index = 0; index < count; ++index)
            {
                crc ^= bytes[index];
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & 1U) != 0 ? static_cast<uint8_t> ((crc >> 1U) ^ 0x8cU)
                                          : static_cast<uint8_t> (crc >> 1U);
                }
            }
            return crc;
        }

        bool validRom (const OneWireRomCode& rom) noexcept
        {
            return rom.bytes[0] == 0x28U && crc8 (rom.bytes, 7) == rom.bytes[7];
        }

        bool validResolution (Ds18b20Resolution resolution) noexcept
        {
            return resolution >= Ds18b20Resolution::Bits9 &&
                   resolution <= Ds18b20Resolution::Bits12;
        }

        bool validProbeQuality (Ds18b20ProbeQuality quality) noexcept
        {
            return quality >= Ds18b20ProbeQuality::Unqualified &&
                   quality <= Ds18b20ProbeQuality::TransportFault;
        }

        bool validSetQuality (Ds18b20SetQuality quality) noexcept
        {
            return quality >= Ds18b20SetQuality::Unqualified &&
                   quality <= Ds18b20SetQuality::Missing;
        }

        bool equalProbe (const QualifiedDs18b20Probe& left,
                         const QualifiedDs18b20Probe& right) noexcept
        {
            return equalRom (left.rom, right.rom) &&
                   left.cycleSequence == right.cycleSequence &&
                   left.conversionGeneration == right.conversionGeneration &&
                   left.readTransactionGeneration == right.readTransactionGeneration &&
                   left.observedAt == right.observedAt &&
                   left.freshThrough == right.freshThrough &&
                   left.rawSixteenths == right.rawSixteenths &&
                   left.lowerRawSixteenths == right.lowerRawSixteenths &&
                   left.upperRawSixteenths == right.upperRawSixteenths &&
                   left.resolution == right.resolution &&
                   left.quality == right.quality && left.age == right.age &&
                   left.status == right.status;
        }

        bool equalSnapshot (const QualifiedDs18b20Snapshot& left,
                            const QualifiedDs18b20Snapshot& right) noexcept
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
                if (!equalProbe (left.probes[index], right.probes[index]))
                {
                    return false;
                }
            }
            return true;
        }

        bool equalControl (const ThermalMapperControl& left,
                           const ThermalMapperControl& right) noexcept
        {
            return left.ownerToken == right.ownerToken &&
                   left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt &&
                   left.nextEdge == right.nextEdge &&
                   left.recordEdge == right.recordEdge && left.status == right.status;
        }

        ThermalGradientIntent emptyIntent (Status status) noexcept
        {
            ThermalGradientIntent intent;
            clearValue (intent);
            intent.health          = ThermalGradientHealth::Qualifying;
            intent.pageKind        = ThermalMapperPageKind::Overall;
            intent.outputsInactive = !status.ok ();
            return intent;
        }

        bool validConfig (const ThermalMapperConfig& config) noexcept
        {
            if (config.ownerToken == 0 || config.configurationRevision == 0 ||
                config.expectedSetSourceId == 0 ||
                config.expectedSetConfigurationRevision == 0 ||
                config.expectedControlOwnerToken == 0 ||
                config.expectedControlSourceId == 0 ||
                config.expectedControlConfigurationRevision == 0 ||
                config.spatialCount < 2 || config.spatialCount > 4 ||
                config.maximumControlAge.milliseconds () == 0 ||
                config.maximumControlAge.milliseconds () >= halfRange ||
                config.meaningfulGradientRawSixteenths == 0 ||
                config.meaningfulGradientRawSixteenths > 2880)
            {
                return false;
            }

            for (uint8_t source = 0; source < 4; ++source)
            {
                if (!validRom (config.sourceRoms[source]))
                {
                    return false;
                }
                for (uint8_t prior = 0; prior < source; ++prior)
                {
                    if (equalRom (config.sourceRoms[source], config.sourceRoms[prior]))
                    {
                        return false;
                    }
                }
            }
            for (uint8_t mapped = 0; mapped < config.spatialCount; ++mapped)
            {
                bool found = false;
                for (uint8_t source = 0; source < 4; ++source)
                {
                    found = found || equalRom (config.spatialOrder[mapped],
                                               config.sourceRoms[source]);
                }
                if (!found)
                {
                    return false;
                }
                for (uint8_t prior = 0; prior < mapped; ++prior)
                {
                    if (equalRom (config.spatialOrder[mapped],
                                  config.spatialOrder[prior]))
                    {
                        return false;
                    }
                }
            }
            static const OneWireRomCode zero = {};
            for (uint8_t index = config.spatialCount; index < 4; ++index)
            {
                if (!equalRom (config.spatialOrder[index], zero))
                {
                    return false;
                }
            }
            return true;
        }

        bool validSnapshot (const ThermalMapperConfig&      config,
                            const QualifiedDs18b20Snapshot& snapshot,
                            TimePoint                       now) noexcept
        {
            if (snapshot.sourceId != config.expectedSetSourceId ||
                snapshot.configurationRevision !=
                    config.expectedSetConfigurationRevision ||
                snapshot.cycleSequence == 0 || !validSetQuality (snapshot.quality) ||
                !validStatus                                    (snapshot.status) ||
                !notBefore                                      (now.milliseconds (), snapshot.observedAt.milliseconds ()) ||
                (snapshot.presentMask & 0xf0U) != 0 ||
                (snapshot.faultMask & 0xf0U) != 0 || (snapshot.validCount > 4) ||
                (snapshot.presentMask | snapshot.faultMask) != 0x0fU)
            {
                return false;
            }

            uint8_t validCount        = 0;
            bool    hasTransportFault = false;
            bool    hasDuplicate      = false;
            bool    hasMissing        = false;
            for (uint8_t index = 0; index < 4; ++index)
            {
                const QualifiedDs18b20Probe& probe = snapshot.probes[index];
                if (!validRom (probe.rom) || !validResolution (probe.resolution) ||
                    !validProbeQuality (probe.quality) || !validStatus (probe.status) ||
                    probe.cycleSequence != snapshot.cycleSequence ||
                    probe.lowerRawSixteenths > probe.upperRawSixteenths ||
                    probe.observedAt != snapshot.observedAt)
                {
                    return false;
                }

                uint8_t sourceMatch = 0;
                for (uint8_t source = 0; source < 4; ++source)
                {
                    if (equalRom (probe.rom, config.sourceRoms[source]))
                    {
                        ++sourceMatch;
                    }
                }
                if (sourceMatch != 1)
                {
                    return false;
                }
                for (uint8_t prior = 0; prior < index; ++prior)
                {
                    if (equalRom (probe.rom, snapshot.probes[prior].rom))
                    {
                        return false;
                    }
                }

                const bool current = probe.quality == Ds18b20ProbeQuality::Current;
                if ((current && !probe.status.ok ()) ||
                    (probe.quality == Ds18b20ProbeQuality::TransportFault &&
                     probe.status.ok ()) ||
                    current != ((snapshot.faultMask & (1U << index)) == 0) ||
                    (current && (snapshot.presentMask & (1U << index)) == 0) ||
                    (probe.quality == Ds18b20ProbeQuality::Missing &&
                     (snapshot.presentMask & (1U << index)) != 0) ||
                    (current && (probe.conversionGeneration == 0 ||
                                 probe.readTransactionGeneration == 0 ||
                                 probe.rawSixteenths < probe.lowerRawSixteenths ||
                                 probe.rawSixteenths > probe.upperRawSixteenths ||
                                 !notBefore (probe.freshThrough.milliseconds (),
                                             probe.observedAt.milliseconds ()))))
                {
                    return false;
                }
                if (current)
                {
                    ++validCount;
                }
                hasTransportFault =
                    hasTransportFault ||
                    probe.quality == Ds18b20ProbeQuality::TransportFault;
                hasDuplicate = hasDuplicate ||
                               probe.quality == Ds18b20ProbeQuality::DuplicateIdentity;
                hasMissing =
                    hasMissing || probe.quality == Ds18b20ProbeQuality::Missing;
            }
            if (validCount != snapshot.validCount)
            {
                return false;
            }
            if (snapshot.quality == Ds18b20SetQuality::Complete &&
                (snapshot.presentMask != 0x0fU || hasTransportFault || hasDuplicate ||
                 hasMissing || !snapshot.status.ok ()))
            {
                return false;
            }
            if (snapshot.quality == Ds18b20SetQuality::TransportFault &&
                !hasTransportFault && snapshot.status.ok ())
            {
                return false;
            }
            if (snapshot.quality == Ds18b20SetQuality::DuplicateIdentity &&
                !hasDuplicate)
            {
                return false;
            }
            if (snapshot.quality == Ds18b20SetQuality::Missing && !hasMissing)
            {
                return false;
            }
            return true;
        }

        bool validControl (const ThermalMapperConfig&  config,
                           const ThermalMapperControl& control, TimePoint now) noexcept
        {
            const bool statusValid = validStatus (control.status);

            const bool statusOk = control.status.ok ();

            const bool timeValid =
                notBefore (now.milliseconds (), control.observedAt.milliseconds ());

            if (control.ownerToken != config.expectedControlOwnerToken ||
                control.sourceId != config.expectedControlSourceId ||
                control.configurationRevision !=
                    config.expectedControlConfigurationRevision ||
                control.sequence == 0 || !statusValid || !statusOk || !timeValid)
            {
                return false;
            }
            return now.elapsedSince (control.observedAt).milliseconds () <=
                   config.maximumControlAge.milliseconds ();
        }

        bool absentControl (const ThermalMapperControl& control) noexcept
        {
            return control.ownerToken == 0 && control.sourceId == 0 &&
                   control.configurationRevision == 0 && control.sequence == 0 &&
                   control.observedAt == TimePoint          (0) && !control.nextEdge &&
                   !control.recordEdge && control.status.ok ();
        }

        uint8_t findProbe (const QualifiedDs18b20Snapshot& probes,
                           const OneWireRomCode&           rom) noexcept
        {
            for (uint8_t index = 0; index < 4; ++index)
            {
                if (equalRom (probes.probes[index].rom, rom))
                {
                    return index;
                }
            }
            return 4;
        }

        bool freshAt (TimePoint now, const QualifiedDs18b20Probe& probe) noexcept
        {
            return notBefore (now.milliseconds (), probe.observedAt.milliseconds ()) &&
                   notBefore (probe.freshThrough.milliseconds (), now.milliseconds ());
        }

        void selectPage (ThermalGradientIntent& intent, uint8_t pageIndex) noexcept
        {
            intent.pageIndex        = pageIndex;
            intent.selectedSlot     = 0;
            intent.selectedGradient = 0;
            intent.ledSelectionMask = 0;
            if (pageIndex == 0)
            {
                intent.pageKind           = ThermalMapperPageKind::Overall;
                intent.lcdShowsIdentity   = false;
                intent.lcdShowsAgeOrFault = intent.overallFaultMask != 0;
                return;
            }
            if (pageIndex <= intent.probeCount)
            {
                const uint8_t slot        = static_cast<uint8_t> (pageIndex - 1U);
                intent.pageKind           = ThermalMapperPageKind::Probe;
                intent.selectedSlot       = slot;
                intent.ledSelectionMask   = static_cast<uint8_t> (1U << slot);
                intent.lcdShowsIdentity   = true;
                intent.lcdShowsAgeOrFault = true;
                return;
            }
            const uint8_t gradient =
                static_cast<uint8_t> (pageIndex - intent.probeCount - 1U);
            intent.pageKind         = ThermalMapperPageKind::AdjacentGradient;
            intent.selectedGradient = gradient;
            intent.ledSelectionMask =
                static_cast<uint8_t> ((1U << gradient) | (1U << (gradient + 1U)));
            intent.lcdShowsIdentity = true;
            intent.lcdShowsAgeOrFault =
                intent.gradients[gradient].quality == ThermalGradientQuality::Fault;
        }

        uint32_t hashByte (uint32_t hash, uint8_t value) noexcept
        {
            return (hash ^ value) * UINT32_C (0x01000193);
        }

        uint32_t hash16 (uint32_t hash, uint16_t value) noexcept
        {
            hash = hashByte (hash, static_cast<uint8_t> (value));
            return hashByte (hash, static_cast<uint8_t> (value >> 8U));
        }

        uint32_t hash32 (uint32_t hash, uint32_t value) noexcept
        {
            hash = hash16 (hash, static_cast<uint16_t> (value));
            return hash16 (hash, static_cast<uint16_t> (value >> 16U));
        }

        uint32_t recordDigest (const ThermalMapperRecordImage& record) noexcept
        {
            static const char domain[] = "ADK.THERMAL.MAPPER.RECORD.V1";
            uint32_t          hash     = UINT32_C (0x811c9dc5);
            for (uint8_t index = 0; index < sizeof domain - 1U; ++index)
            {
                hash = hashByte (hash, static_cast<uint8_t> (domain[index]));
            }
            hash = hashByte (hash, record.formatVersion);
            hash = hash32   (hash, record.ownerToken);
            hash = hash32   (hash, record.lifecycleGeneration);
            hash = hash16   (hash, record.configurationRevision);
            hash = hash32   (hash, record.recordSequence);
            hash = hash32   (hash, record.recordEdgeOwnerToken);
            hash = hashByte (hash, record.recordEdgeSourceId);
            hash = hash16   (hash, record.recordEdgeConfigurationRevision);
            hash = hash32   (hash, record.recordEdgeSequence);
            hash = hash32   (hash, record.recordEdgeObservedAt.milliseconds ());
            hash = hashByte (hash, record.setSourceId);
            hash = hash16   (hash, record.setConfigurationRevision);
            hash = hash32   (hash, record.setCycleSequence);
            hash = hash32   (hash, record.setObservedAt.milliseconds ());
            hash = hash32   (hash, record.mappedAt.milliseconds ());
            for (uint8_t source = 0; source < 4; ++source)
            {
                for (uint8_t octet = 0; octet < 8; ++octet)
                {
                    hash = hashByte (hash, record.sourceRoms[source].bytes[octet]);
                }
            }
            hash = hashByte (hash, record.probeCount);
            for (uint8_t index = 0; index < record.probeCount; ++index)
            {
                const ThermalMapperRecordProbe& probe = record.probes[index];
                for (uint8_t octet = 0; octet < 8; ++octet)
                {
                    hash = hashByte (hash, probe.rom.bytes[octet]);
                }
                hash = hash16   (hash, static_cast<uint16_t> (probe.lowerRawSixteenths));
                hash = hash16   (hash, static_cast<uint16_t> (probe.upperRawSixteenths));
                hash = hashByte (hash, static_cast<uint8_t> (probe.resolution));
                hash = hashByte (hash, static_cast<uint8_t> (probe.quality));
                hash = hash32   (hash, probe.age.milliseconds ());
                hash = hash32   (hash, probe.conversionGeneration);
                hash = hash32   (hash, probe.readTransactionGeneration);
                hash = hashByte (hash, static_cast<uint8_t> (probe.status.error ()));
            }
            hash = hashByte (hash, record.gradientCount);
            for (uint8_t index = 0; index < record.gradientCount; ++index)
            {
                const ThermalGradientPair& pair = record.gradients[index];
                hash                            = hashByte (hash, pair.leftSlot);
                hash                            = hashByte (hash, pair.rightSlot);
                hash = hash32                              (hash, static_cast<uint32_t> (pair.lowerRawSixteenths));
                hash = hash32                              (hash, static_cast<uint32_t> (pair.upperRawSixteenths));
                hash = hashByte                            (hash, static_cast<uint8_t> (pair.quality));
                hash = hashByte                            (hash, pair.faultMask);
            }
            hash = hashByte (hash, static_cast<uint8_t> (record.health));
            return hashByte (hash, record.faultMask);
        }

        ThermalGradientQuality classify (int32_t lower, int32_t upper,
                                         uint16_t threshold) noexcept
        {
            const int32_t positive = threshold;
            const int32_t negative = -positive;
            if (lower >= positive)
            {
                return ThermalGradientQuality::Rising;
            }
            if (upper <= negative)
            {
                return ThermalGradientQuality::Falling;
            }
            if (lower > negative && upper < positive)
            {
                return ThermalGradientQuality::Flat;
            }
            return ThermalGradientQuality::Indeterminate;
        }
    } // namespace

    ThermalGradientMapper::ThermalGradientMapper (
        const ThermalMapperConfig& config) noexcept
        : config_ (config), intent_ (emptyIntent (StatusCode::NotInitialized)),
          lastUpdateAt_            (0), lifecycleGeneration_ (0), nextRecordSequence_ (1),
          initialized_             (false), generationExhausted_ (false),
          recordSequenceExhausted_ (false), hasProbes_ (false), hasControl_ (false)
    {
        clearValue (lastProbes_);
        clearValue (lastControl_);
    }

    Status ThermalGradientMapper::initialize (TimePoint now) noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }
        if (generationExhausted_ || lifecycleGeneration_ == UINT32_MAX)
        {
            generationExhausted_ = true;
            return StatusCode::CapacityExceeded;
        }
        if (!validConfig (config_))
        {
            return StatusCode::InvalidConfiguration;
        }
        ++lifecycleGeneration_;
        nextRecordSequence_      = 1;
        recordSequenceExhausted_ = false;
        hasProbes_               = false;
        hasControl_              = false;
        clearValue (lastControl_);
        lastUpdateAt_ = now;
        intent_       = emptyIntent (StatusCode::Ok);
        initialized_  = true;
        return StatusCode::Ok;
    }

    Status ThermalGradientMapper::reset (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (generationExhausted_ || lifecycleGeneration_ == UINT32_MAX)
        {
            initialized_         = false;
            generationExhausted_ = true;
            intent_              = emptyIntent (StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }
        ++lifecycleGeneration_;
        nextRecordSequence_      = 1;
        recordSequenceExhausted_ = false;
        hasProbes_               = false;
        hasControl_              = false;
        clearValue (lastProbes_);
        clearValue (lastControl_);
        lastUpdateAt_ = now;
        intent_       = emptyIntent (StatusCode::Ok);
        return StatusCode::Ok;
    }

    Status ThermalGradientMapper::update (const ThermalMapperEnvelope& envelope,
                                          ThermalMapperResult&         result) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!notBefore (envelope.now.milliseconds (), lastUpdateAt_.milliseconds ()) ||
            !validSnapshot (config_, envelope.probes, envelope.now))
        {
            return StatusCode::InvalidArgument;
        }
        const bool controlPresent = !absentControl (envelope.control);
        if (controlPresent && !validControl        (config_, envelope.control, envelope.now))
        {
            return StatusCode::InvalidArgument;
        }

        const bool repeatedProbes =
            hasProbes_ && envelope.probes.cycleSequence == lastProbes_.cycleSequence;
        if (repeatedProbes && !equalSnapshot (envelope.probes, lastProbes_))
        {
            return StatusCode::InvalidArgument;
        }
        if (hasProbes_ && !repeatedProbes &&
            !forward (envelope.probes.cycleSequence, lastProbes_.cycleSequence))
        {
            return StatusCode::InvalidArgument;
        }
        if (!hasProbes_ && lastProbes_.cycleSequence != 0 &&
            !forward (envelope.probes.cycleSequence, lastProbes_.cycleSequence))
        {
            return StatusCode::InvalidArgument;
        }
        const bool repeatedControl = controlPresent && hasControl_ &&
                                     envelope.control.sequence == lastControl_.sequence;
        if (repeatedControl && !equalControl (envelope.control, lastControl_))
        {
            return StatusCode::InvalidArgument;
        }
        if (controlPresent && hasControl_ && !repeatedControl &&
            !forward (envelope.control.sequence, lastControl_.sequence))
        {
            return StatusCode::InvalidArgument;
        }
        if (controlPresent && !repeatedControl && hasControl_ &&
            !notBefore (envelope.control.observedAt.milliseconds (),
                        lastControl_.observedAt.milliseconds ()))
        {
            return StatusCode::InvalidArgument;
        }

        ThermalGradientIntent next;
        clearValue (next);
        next.health           = ThermalGradientHealth::Stable;
        next.probeCount       = config_.spatialCount;
        next.gradientCount    = static_cast<uint8_t> (config_.spatialCount - 1U);
        uint8_t pageIndex     = intent_.pageIndex;
        uint8_t rising        = 0;
        uint8_t falling       = 0;
        uint8_t indeterminate = 0;
        bool    haveExtrema   = false;

        for (uint8_t slot = 0; slot < config_.spatialCount; ++slot)
        {
            const uint8_t source =
                findProbe (envelope.probes, config_.spatialOrder[slot]);
            if (source >= 4)
            {
                return StatusCode::InvalidArgument;
            }
            const QualifiedDs18b20Probe& probe  = envelope.probes.probes[source];
            ThermalMapperProbeIntent&    mapped = next.probes[slot];
            mapped.rom                          = probe.rom;
            mapped.lowerRawSixteenths           = probe.lowerRawSixteenths;
            mapped.upperRawSixteenths           = probe.upperRawSixteenths;
            mapped.resolution                   = probe.resolution;
            mapped.quality                      = probe.quality;
            mapped.age         = envelope.now.elapsedSince (probe.observedAt);
            mapped.status      = probe.status;
            const bool healthy = probe.quality == Ds18b20ProbeQuality::Current &&
                                 probe.status.ok () && freshAt (envelope.now, probe);
            if (!healthy)
            {
                next.overallFaultMask |= static_cast<uint8_t> (1U << slot);
                if (!freshAt (envelope.now, probe))
                {
                    mapped.quality = Ds18b20ProbeQuality::Stale;
                    mapped.status  = StatusCode::Timeout;
                }
            }
            if (!healthy)
            {
                mapped.lowerRawSixteenths = 0;
                mapped.upperRawSixteenths = 0;
            }
            if (healthy && (!haveExtrema ||
                            mapped.lowerRawSixteenths < next.minimumLowerRawSixteenths))
            {
                next.minimumLowerRawSixteenths = mapped.lowerRawSixteenths;
                next.minimumRom                = mapped.rom;
                next.minimumTieMask            = static_cast<uint8_t> (1U << slot);
            }
            else if (healthy &&
                     mapped.lowerRawSixteenths == next.minimumLowerRawSixteenths)
            {
                next.minimumTieMask |= static_cast<uint8_t> (1U << slot);
            }
            if (healthy && (!haveExtrema ||
                            mapped.upperRawSixteenths > next.maximumUpperRawSixteenths))
            {
                next.maximumUpperRawSixteenths = mapped.upperRawSixteenths;
                next.maximumRom                = mapped.rom;
                next.maximumTieMask            = static_cast<uint8_t> (1U << slot);
            }
            else if (healthy &&
                     mapped.upperRawSixteenths == next.maximumUpperRawSixteenths)
            {
                next.maximumTieMask |= static_cast<uint8_t> (1U << slot);
            }
            haveExtrema = haveExtrema || healthy;
        }

        for (uint8_t index = 0; index < next.gradientCount; ++index)
        {
            ThermalGradientPair& pair = next.gradients[index];
            pair.leftSlot             = index;
            pair.rightSlot            = index + 1U;
            pair.lowerRawSixteenths =
                static_cast<int32_t> (next.probes[index + 1U].lowerRawSixteenths) -
                static_cast<int32_t> (next.probes[index].upperRawSixteenths);
            pair.upperRawSixteenths =
                static_cast<int32_t> (next.probes[index + 1U].upperRawSixteenths) -
                static_cast<int32_t> (next.probes[index].lowerRawSixteenths);
            pair.faultMask = static_cast<uint8_t> (
                next.overallFaultMask & ((1U << index) | (1U << (index + 1U))));
            if (pair.faultMask != 0)
            {
                pair.quality            = ThermalGradientQuality::Fault;
                pair.lowerRawSixteenths = 0;
                pair.upperRawSixteenths = 0;
            }
            else
            {
                pair.quality =
                    classify (pair.lowerRawSixteenths, pair.upperRawSixteenths,
                              config_.meaningfulGradientRawSixteenths);
            }
            if (pair.quality == ThermalGradientQuality::Rising)
            {
                ++rising;
            }
            if (pair.quality == ThermalGradientQuality::Falling)
            {
                ++falling;
            }
            if (pair.quality == ThermalGradientQuality::Indeterminate)
            {
                ++indeterminate;
            }
        }

        if (next.overallFaultMask != 0)
        {
            next.health = ThermalGradientHealth::Fault;
        }
        else if (indeterminate != 0 || (rising != 0 && falling != 0))
        {
            next.health = ThermalGradientHealth::Disagreement;
        }
        else if (rising != 0 || falling != 0)
        {
            next.health = ThermalGradientHealth::Gradient;
        }
        else
        {
            next.health = ThermalGradientHealth::Stable;
        }

        const bool    freshControl = controlPresent && !repeatedControl;
        const uint8_t pageCount    = static_cast<uint8_t> (config_.spatialCount * 2U);
        if (freshControl && envelope.control.nextEdge)
        {
            pageIndex = static_cast<uint8_t> ((pageIndex + 1U) % pageCount);
        }
        selectPage (next, pageIndex);
        next.outputsInactive = false;

        const bool produceRecord = freshControl && envelope.control.recordEdge;
        if (produceRecord && recordSequenceExhausted_)
        {
            return StatusCode::CapacityExceeded;
        }

        clearValue (result);
        memcpy     (&result.intent, &next, sizeof result.intent);
        result.status = StatusCode::Ok;
        if (produceRecord)
        {
            ThermalMapperRecordImage& record = result.record;
            record.ownerToken                = config_.ownerToken;
            record.lifecycleGeneration       = lifecycleGeneration_;
            record.configurationRevision     = config_.configurationRevision;
            record.recordSequence            = nextRecordSequence_;
            record.recordEdgeOwnerToken      = envelope.control.ownerToken;
            record.recordEdgeSourceId        = envelope.control.sourceId;
            record.recordEdgeConfigurationRevision =
                envelope.control.configurationRevision;
            record.recordEdgeSequence       = envelope.control.sequence;
            record.recordEdgeObservedAt     = envelope.control.observedAt;
            record.setSourceId              = envelope.probes.sourceId;
            record.setConfigurationRevision = envelope.probes.configurationRevision;
            record.setCycleSequence         = envelope.probes.cycleSequence;
            record.setObservedAt            = envelope.probes.observedAt;
            record.mappedAt                 = envelope.now;
            record.formatVersion            = 1;
            record.probeCount               = next.probeCount;
            record.gradientCount            = next.gradientCount;
            record.health                   = next.health;
            record.faultMask                = next.overallFaultMask;
            for (uint8_t source = 0; source < 4; ++source)
            {
                record.sourceRoms[source] = config_.sourceRoms[source];
            }
            for (uint8_t slot = 0; slot < next.probeCount; ++slot)
            {
                const uint8_t source =
                    findProbe (envelope.probes, next.probes[slot].rom);
                ThermalMapperRecordProbe&    destination = record.probes[slot];
                const QualifiedDs18b20Probe& probe = envelope.probes.probes[source];
                destination.rom                    = next.probes[slot].rom;
                destination.lowerRawSixteenths   = next.probes[slot].lowerRawSixteenths;
                destination.upperRawSixteenths   = next.probes[slot].upperRawSixteenths;
                destination.resolution           = next.probes[slot].resolution;
                destination.quality              = next.probes[slot].quality;
                destination.age                  = next.probes[slot].age;
                destination.conversionGeneration = probe.conversionGeneration;
                destination.readTransactionGeneration = probe.readTransactionGeneration;
                destination.status                    = next.probes[slot].status;
            }
            for (uint8_t index = 0; index < next.gradientCount; ++index)
            {
                record.gradients[index] = next.gradients[index];
            }
            record.witnessDigest = recordDigest (record);
            result.hasRecord     = true;
        }

        memcpy (&intent_, &next, sizeof intent_);
        lastProbes_ = envelope.probes;
        if (controlPresent)
        {
            lastControl_ = envelope.control;
        }
        lastUpdateAt_ = envelope.now;
        hasProbes_    = true;
        hasControl_   = hasControl_ || controlPresent;
        if (result.hasRecord)
        {
            if (nextRecordSequence_ == UINT32_MAX)
            {
                recordSequenceExhausted_ = true;
            }
            else
            {
                ++nextRecordSequence_;
            }
        }
        return StatusCode::Ok;
    }

    Status ThermalGradientMapper::shutdown () noexcept
    {
        initialized_ = false;
        hasProbes_   = false;
        hasControl_  = false;
        intent_      = emptyIntent (StatusCode::NotInitialized);
        return StatusCode::Ok;
    }

    Status
    ThermalGradientMapper::snapshot (ThermalGradientIntent& intent) const noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        clearValue (intent);
        memcpy     (&intent, &intent_, sizeof intent);
        return StatusCode::Ok;
    }

    bool ThermalGradientMapper::initialized () const noexcept
    {
        return initialized_;
    }
    // clang-format on
} // namespace adk
