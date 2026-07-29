#include "qualified_18b20_probe_set_policy.h"

#include <limits.h>
#include <string.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool equalRom (const OneWireRomCode& left,
                       const OneWireRomCode& right) noexcept
        {
            return memcmp (left.bytes, right.bytes, sizeof left.bytes) == 0;
        }

        bool emptyRom (const OneWireRomCode& rom) noexcept
        {
            static const OneWireRomCode empty = {{0, 0, 0, 0, 0, 0, 0, 0}};
            return equalRom (rom, empty);
        }

        uint8_t crc8 (const uint8_t* bytes, uint8_t count) noexcept
        {
            uint8_t crc = 0;
            for (uint8_t index = 0; index < count; ++index)
            {
                crc ^= bytes[index];
                for (uint8_t bit = 0; bit < 8; ++bit)
                    crc = (crc & 1U) != 0
                              ? static_cast<uint8_t> ((crc >> 1U) ^ 0x8cU)
                              : static_cast<uint8_t> (crc >> 1U);
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

        uint32_t resolutionDeadline (Ds18b20Resolution resolution) noexcept
        {
            static const uint16_t deadlines[] = {94, 188, 375, 750};
            return deadlines[static_cast<uint8_t> (resolution)];
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

        bool equalSearch (const OneWireSearchState& left,
                          const OneWireSearchState& right) noexcept
        {
            return equalRom (left.rom, right.rom) &&
                   left.lastDiscrepancy == right.lastDiscrepancy &&
                   left.lastDevice == right.lastDevice;
        }

        bool validOperation (OneWireOperation value) noexcept
        {
            return value >= OneWireOperation::ResetPresence &&
                   value <= OneWireOperation::MatchRomReadScratchpad;
        }

        bool validSupply (OneWireSupplyMode value) noexcept
        {
            return value >= OneWireSupplyMode::ExternallyPowered &&
                   value <= OneWireSupplyMode::ParasitePower;
        }

        uint16_t requiredSlots (OneWireOperation operation) noexcept
        {
            switch (operation)
            {
                case OneWireOperation::SearchRomPass:
                    return 200;
                case OneWireOperation::MatchRomStartConversion:
                    return 80;
                case OneWireOperation::MatchRomReadConversionStatus:
                    return 1;
                case OneWireOperation::MatchRomReadScratchpad:
                    return 152;
                default:
                    return 0;
            }
        }

        uint8_t requiredBytes (OneWireOperation operation) noexcept
        {
            switch (operation)
            {
                case OneWireOperation::MatchRomReadConversionStatus:
                    return 1;
                case OneWireOperation::MatchRomReadScratchpad:
                    return 9;
                default:
                    return 0;
            }
        }

        Status validateTransaction (
            const OneWireTransactionSnapshot& transaction,
            OneWireOperation                  operation) noexcept
        {
            const bool operationValid = validOperation (transaction.operation);
            const bool requestOperationValid =
                validOperation (transaction.request.operation);
            const bool supplyValid =
                validSupply (transaction.request.supplyMode);
            const uint16_t slots = requiredSlots (operation);
            const uint8_t  bytes = requiredBytes (operation);
            const uint32_t completed =
                transaction.completedAt.microseconds ();
            const uint32_t started =
                transaction.request.startedAt.microseconds ();
            const StatusCode requestError =
                transaction.request.status.error ();
            const StatusCode transactionError = transaction.status.error ();
            if (!operationValid || !requestOperationValid || !supplyValid ||
                transaction.operation != operation ||
                transaction.request.operation != operation ||
                transaction.phase != OneWirePhase::Complete ||
                transaction.quality != OneWireTransactionQuality::Complete ||
                transaction.request.requestSequence == 0 ||
                transaction.transactionGeneration == 0 ||
                transaction.ownerToken == 0 ||
                transaction.lifecycleGeneration == 0 ||
                transaction.configurationRevision == 0 ||
                transaction.request.supplyMode !=
                    OneWireSupplyMode::ExternallyPowered ||
                requestError != StatusCode::Ok ||
                transactionError != StatusCode::Ok ||
                !transaction.presenceSeen ||
                !transaction.releaseRequested ||
                !transaction.releaseConfirmed ||
                transaction.acceptedSlotCount != slots ||
                transaction.readByteCount != bytes ||
                !notBefore (completed, started))
                return StatusCode::InvalidArgument;
            return {};
        }

        void normalize (const OneWireTransactionSnapshot& source,
                        Ds18b20NormalizedTransactionRef&  target) noexcept
        {
            target.requestSequence = source.request.requestSequence;
            target.transactionGeneration = source.transactionGeneration;
            target.startedAt       = source.request.startedAt;
            target.completedAt     = source.completedAt;
        }

        int8_t configuredIndex (const QualifiedDs18b20SetConfig& config,
                                const OneWireRomCode& rom) noexcept
        {
            for (uint8_t index = 0; index < 4; ++index)
                if (equalRom (config.probes[index].rom, rom))
                    return static_cast<int8_t> (index);
            return -1;
        }

        void clearProbe (Ds18b20NormalizedProbeWitness& probe) noexcept
        {
            memset (static_cast<void*> (&probe), 0, sizeof probe);
        }

        void clearBuilder (Ds18b20CycleBuilder& builder) noexcept
        {
            memset (static_cast<void*> (&builder), 0, sizeof builder);
        }

        const Ds18b20NormalizedTransactionRef* taggedReference (
            const Ds18b20NormalizedSearchPass searchPasses[4],
            const Ds18b20NormalizedProbeWitness probes[4],
            uint8_t tag) noexcept
        {
            if (tag >= 1U && tag <= 4U)
                return &searchPasses[tag - 1U].transaction;
            if (tag < 5U || tag > 16U)
                return 0;
            const uint8_t offset = static_cast<uint8_t> (tag - 5U);
            const uint8_t probe  = static_cast<uint8_t> (offset / 3U);
            switch (offset % 3U)
            {
                case 0: return &probes[probe].conversionStart;
                case 1: return &probes[probe].conversionStatus;
                default: return &probes[probe].scratchpadRead;
            }
        }

        bool followsTaggedTransaction (
            const OneWireTransactionSnapshot& transaction,
            const Ds18b20NormalizedSearchPass searchPasses[4],
            const Ds18b20NormalizedProbeWitness probes[4],
            uint8_t tag) noexcept
        {
            const Ds18b20NormalizedTransactionRef* prior =
                taggedReference (searchPasses, probes, tag);
            return prior == 0 ||
                   (forward (transaction.transactionGeneration,
                             prior->transactionGeneration) &&
                    forward (transaction.request.requestSequence,
                             prior->requestSequence) &&
                    notBefore (transaction.request.startedAt.microseconds (),
                               prior->completedAt.microseconds ()));
        }

        bool referencesFollow (
            const Ds18b20NormalizedSearchPass searchPasses[4],
            uint8_t searchCount,
            const Ds18b20NormalizedProbeWitness probes[4],
            const Ds18b20NormalizedTransactionRef& anchor) noexcept
        {
            for (uint8_t index = 0; index < searchCount; ++index)
            {
                const Ds18b20NormalizedTransactionRef& candidate =
                    searchPasses[index].transaction;
                if (!forward (candidate.requestSequence,
                              anchor.requestSequence) ||
                    !forward (candidate.transactionGeneration,
                              anchor.transactionGeneration) ||
                    !notBefore (candidate.startedAt.microseconds (),
                                anchor.completedAt.microseconds ()))
                    return false;
            }
            for (uint8_t index = 0; index < 4; ++index)
            {
                const Ds18b20NormalizedTransactionRef* candidates[3] = {
                    &probes[index].conversionStart,
                    &probes[index].conversionStatus,
                    &probes[index].scratchpadRead};
                for (uint8_t candidateIndex = 0;
                     candidateIndex < 3; ++candidateIndex)
                {
                    const Ds18b20NormalizedTransactionRef& candidate =
                        *candidates[candidateIndex];
                    if (candidate.transactionGeneration != 0 &&
                        (!forward (candidate.requestSequence,
                                   anchor.requestSequence) ||
                         !forward (candidate.transactionGeneration,
                                   anchor.transactionGeneration) ||
                         !notBefore (candidate.startedAt.microseconds (),
                                     anchor.completedAt.microseconds ())))
                        return false;
                }
            }
            return true;
        }

        void clearQualified (QualifiedDs18b20Probe& probe,
                             const Ds18b20ProbeConfig& config) noexcept
        {
            memset (static_cast<void*> (&probe), 0, sizeof probe);
            probe.rom        = config.rom;
            probe.resolution = config.resolution;
            probe.quality    = Ds18b20ProbeQuality::Unqualified;
            probe.status     = {};
        }
    }

    Ds18b20CycleBuilder::Ds18b20CycleBuilder () noexcept
    {
        clearBuilder (*this);
    }

    Qualified18B20ProbeSetPolicy::Qualified18B20ProbeSetPolicy (
        const QualifiedDs18b20SetConfig& config) noexcept
        : config_ (config), snapshot_ (), lastCycle_ (), policyGeneration_ (0),
          initialized_ (false), hasLastCycle_ (false)
    {
        for (uint8_t index = 0; index < 4; ++index)
            clearQualified (snapshot_.probes[index], config_.probes[index]);
        snapshot_.quality = Ds18b20SetQuality::Unqualified;
        snapshot_.faultMask = 0x0fU;
    }

    Status Qualified18B20ProbeSetPolicy::initialize () noexcept
    {
        if (initialized_)
            return {};
        if (config_.expectedSourceId == 0 ||
            config_.expectedConfigurationRevision == 0 ||
            config_.expectedOneWireOwnerToken == 0 ||
            config_.expectedOneWireConfigurationRevision == 0)
            return StatusCode::InvalidConfiguration;
        for (uint8_t index = 0; index < 4; ++index)
        {
            const Ds18b20ProbeConfig& probe = config_.probes[index];
            if (!validRom (probe.rom) || !validResolution (probe.resolution) ||
                probe.conversionDeadline.milliseconds () == 0 ||
                probe.conversionDeadline.milliseconds () >
                    resolutionDeadline (probe.resolution) ||
                probe.maximumAge.milliseconds () == 0 ||
                probe.maximumAge.milliseconds () >= halfRange ||
                probe.minimumRawSixteenths > probe.maximumRawSixteenths)
                return StatusCode::InvalidConfiguration;
            for (uint8_t prior = 0; prior < index; ++prior)
                if (equalRom (probe.rom, config_.probes[prior].rom))
                    return StatusCode::InvalidConfiguration;
        }
        initialized_ = true;
        reset ();
        if (!initialized_)
            return StatusCode::CapacityExceeded;
        return {};
    }

    void Qualified18B20ProbeSetPolicy::reset () noexcept
    {
        if (!initialized_)
            return;
        if (policyGeneration_ == UINT32_MAX)
        {
            initialized_ = false;
            return;
        }
        ++policyGeneration_;
        memset (static_cast<void*> (&snapshot_), 0, sizeof snapshot_);
        for (uint8_t index = 0; index < 4; ++index)
            clearQualified (snapshot_.probes[index], config_.probes[index]);
        snapshot_.quality = Ds18b20SetQuality::Unqualified;
        snapshot_.faultMask = 0x0fU;
        clearBuilder (lastCycle_);
        hasLastCycle_ = false;
    }

    Status Qualified18B20ProbeSetPolicy::beginCycle (
        TimePoint now, uint8_t sourceId, uint16_t configurationRevision,
        uint32_t cycleSequence, TimePoint observedAt,
        Ds18b20CycleBuilder& builder) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        if (sourceId != config_.expectedSourceId ||
            configurationRevision != config_.expectedConfigurationRevision ||
            cycleSequence == 0 ||
            !notBefore (now.milliseconds (), observedAt.milliseconds ()))
            return StatusCode::InvalidArgument;
        clearBuilder (builder);
        builder.sourceId              = sourceId;
        builder.configurationRevision = configurationRevision;
        builder.cycleSequence         = cycleSequence;
        builder.observedAt            = observedAt;
        builder.policyGeneration      = policyGeneration_;
        builder.cycleBegun            = true;
        return {};
    }

    Status Qualified18B20ProbeSetPolicy::ingestSearchPass (
        Ds18b20CycleBuilder& builder,
        const OneWireTransactionSnapshot& transaction,
        const OneWireSearchState& requestSearch) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        Status valid = validateTransaction (
            transaction, OneWireOperation::SearchRomPass);
        const bool transactionValid = valid.ok ();
        const Status invalidArgument = StatusCode::InvalidArgument;
        if (!transactionValid ||
            builder.sourceId != config_.expectedSourceId ||
            builder.configurationRevision !=
                config_.expectedConfigurationRevision ||
            builder.cycleSequence == 0 || !builder.cycleBegun ||
            builder.policyGeneration != policyGeneration_ ||
            builder.searchFinished ||
            builder.searchPassCount >= 4 ||
            (builder.oneWireOwnerToken != 0 &&
             (builder.oneWireOwnerToken != transaction.ownerToken ||
              builder.oneWireLifecycleGeneration !=
                  transaction.lifecycleGeneration ||
              builder.oneWireConfigurationRevision !=
                  transaction.configurationRevision)) ||
            transaction.ownerToken != config_.expectedOneWireOwnerToken ||
            transaction.configurationRevision !=
                config_.expectedOneWireConfigurationRevision ||
            !emptyRom (transaction.request.addressedRom) ||
            !emptyRom (transaction.returnedRom) ||
            requestSearch.lastDiscrepancy > 64U ||
            transaction.request.search.lastDiscrepancy > 64U ||
            transaction.searchResult.lastDiscrepancy > 64U ||
            !equalSearch (requestSearch, transaction.request.search))
            return transactionValid ? invalidArgument : valid;

        if (builder.searchPassCount == 0)
        {
            if (!emptyRom (requestSearch.rom) ||
                requestSearch.lastDiscrepancy != 0 ||
                requestSearch.lastDevice)
                return StatusCode::InvalidArgument;
        }
        else
        {
            const Ds18b20NormalizedSearchPass& previous =
                builder.searchPasses[builder.searchPassCount - 1U];
            const bool searchMatches =
                equalSearch (requestSearch, previous.completedSearch);
            const bool generationFollows = forward (
                transaction.transactionGeneration,
                previous.transaction.transactionGeneration);
            const bool requestFollows = forward (
                transaction.request.requestSequence,
                previous.transaction.requestSequence);
            if (previous.completedSearch.lastDevice || !searchMatches ||
                !generationFollows || !requestFollows ||
                !notBefore (transaction.request.startedAt.microseconds (),
                            previous.transaction.completedAt.microseconds ()))
                return StatusCode::InvalidArgument;
        }

        if (builder.oneWireOwnerToken == 0)
        {
            builder.oneWireOwnerToken = transaction.ownerToken;
            builder.oneWireLifecycleGeneration =
                transaction.lifecycleGeneration;
            builder.oneWireConfigurationRevision =
                transaction.configurationRevision;
        }
        Ds18b20NormalizedSearchPass& target =
            builder.searchPasses[builder.searchPassCount++];
        normalize (transaction, target.transaction);
        target.requestSearch    = requestSearch;
        target.completedSearch  = transaction.searchResult;
        builder.lastTransactionTag = builder.searchPassCount;
        return {};
    }

    Status Qualified18B20ProbeSetPolicy::finishSearch (
        Ds18b20CycleBuilder& builder, bool searchComplete,
        bool searchOverCapacity, Status producerStatus) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        if (builder.sourceId != config_.expectedSourceId ||
            builder.configurationRevision !=
                config_.expectedConfigurationRevision ||
            builder.cycleSequence == 0 || !builder.cycleBegun ||
            builder.policyGeneration != policyGeneration_ ||
            builder.searchFinished ||
            (searchComplete && searchOverCapacity) ||
            producerStatus.error () > StatusCode::HardwareFailure)
            return StatusCode::InvalidArgument;
        if (searchComplete)
        {
            if (!producerStatus.ok () || builder.searchPassCount == 0 ||
                !builder.searchPasses[builder.searchPassCount - 1U]
                     .completedSearch.lastDevice)
                return StatusCode::InvalidArgument;
        }
        if (searchOverCapacity &&
            (builder.searchPassCount != 4 ||
             builder.searchPasses[3].completedSearch.lastDevice))
            return StatusCode::InvalidArgument;
        builder.searchComplete     = searchComplete;
        builder.searchOverCapacity = searchOverCapacity;
        builder.searchFinished     = true;
        builder.status             = producerStatus;
        return {};
    }

    Status Qualified18B20ProbeSetPolicy::ingestConversionStart (
        Ds18b20CycleBuilder& builder, uint32_t conversionGeneration,
        const OneWireTransactionSnapshot& transaction) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        Status valid = validateTransaction (
            transaction, OneWireOperation::MatchRomStartConversion);
        const int8_t index = configuredIndex (config_,
                                              transaction.request.addressedRom);
        if (!valid.ok () || conversionGeneration == 0 || index < 0 ||
            builder.sourceId != config_.expectedSourceId ||
            builder.configurationRevision !=
                config_.expectedConfigurationRevision ||
            builder.cycleSequence == 0 || !builder.cycleBegun ||
            builder.policyGeneration != policyGeneration_ ||
            !builder.searchFinished || !builder.searchComplete ||
            !builder.status.ok () ||
            (builder.oneWireOwnerToken != 0 &&
             (builder.oneWireOwnerToken != transaction.ownerToken ||
              builder.oneWireLifecycleGeneration !=
                  transaction.lifecycleGeneration ||
              builder.oneWireConfigurationRevision !=
                  transaction.configurationRevision)) ||
            transaction.ownerToken != config_.expectedOneWireOwnerToken ||
            transaction.configurationRevision !=
                config_.expectedOneWireConfigurationRevision ||
            builder.probes[index].conversionStart.transactionGeneration != 0)
            return valid.ok () ? Status (StatusCode::InvalidArgument) : valid;
        if (!followsTaggedTransaction (
                transaction, builder.searchPasses, builder.probes,
                builder.lastTransactionTag))
            return StatusCode::InvalidArgument;
        if (builder.oneWireOwnerToken == 0)
        {
            builder.oneWireOwnerToken = transaction.ownerToken;
            builder.oneWireLifecycleGeneration =
                transaction.lifecycleGeneration;
            builder.oneWireConfigurationRevision =
                transaction.configurationRevision;
        }
        Ds18b20NormalizedProbeWitness& witness = builder.probes[index];
        clearProbe (witness);
        witness.rom = transaction.request.addressedRom;
        witness.conversionGeneration = conversionGeneration;
        normalize (transaction, witness.conversionStart);
        witness.conversionStatusPresent = false;
        builder.lastTransactionTag =
            static_cast<uint8_t> (5U + static_cast<uint8_t> (index) * 3U);
        ++builder.probeCount;
        return {};
    }

    Status Qualified18B20ProbeSetPolicy::ingestConversionStatus (
        Ds18b20CycleBuilder& builder, uint32_t conversionGeneration,
        const OneWireTransactionSnapshot& transaction) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        Status valid = validateTransaction (
            transaction, OneWireOperation::MatchRomReadConversionStatus);
        const int8_t index = configuredIndex (config_,
                                              transaction.request.addressedRom);
        if (!valid.ok () || conversionGeneration == 0 || index < 0 ||
            (builder.oneWireOwnerToken != 0 &&
             (builder.oneWireOwnerToken != transaction.ownerToken ||
              builder.oneWireLifecycleGeneration !=
                  transaction.lifecycleGeneration ||
              builder.oneWireConfigurationRevision !=
                  transaction.configurationRevision)) ||
            transaction.ownerToken != config_.expectedOneWireOwnerToken ||
            transaction.configurationRevision !=
                config_.expectedOneWireConfigurationRevision ||
            transaction.readBytes[0] > 1U || !builder.cycleBegun ||
            builder.policyGeneration != policyGeneration_ ||
            !builder.searchFinished)
            return valid.ok () ? Status (StatusCode::InvalidArgument) : valid;
        Ds18b20NormalizedProbeWitness& witness = builder.probes[index];
        if (witness.conversionGeneration != conversionGeneration ||
            witness.conversionStart.transactionGeneration == 0 ||
            witness.conversionStatusPresent ||
            transaction.transactionGeneration !=
                witness.conversionStart.transactionGeneration + 1U ||
            transaction.request.requestSequence !=
                witness.conversionStart.requestSequence + 1U ||
            !notBefore (transaction.request.startedAt.microseconds (),
                        witness.conversionStart.completedAt.microseconds ()))
            return StatusCode::InvalidArgument;
        if (!followsTaggedTransaction (
                transaction, builder.searchPasses, builder.probes,
                builder.lastTransactionTag))
            return StatusCode::InvalidArgument;
        normalize (transaction, witness.conversionStatus);
        witness.conversionStatusPresent = true;
        witness.conversionCompletedHigh = transaction.readBytes[0] != 0;
        builder.lastTransactionTag =
            static_cast<uint8_t> (6U + static_cast<uint8_t> (index) * 3U);
        return {};
    }

    Status Qualified18B20ProbeSetPolicy::ingestScratchpad (
        Ds18b20CycleBuilder& builder, uint32_t conversionGeneration,
        TimePoint scratchpadObservedAt,
        const OneWireTransactionSnapshot& transaction) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        Status valid = validateTransaction (
            transaction, OneWireOperation::MatchRomReadScratchpad);
        const int8_t index = configuredIndex (config_,
                                              transaction.request.addressedRom);
        if (!valid.ok () || conversionGeneration == 0 || index < 0 ||
            (builder.oneWireOwnerToken != 0 &&
             (builder.oneWireOwnerToken != transaction.ownerToken ||
              builder.oneWireLifecycleGeneration !=
                  transaction.lifecycleGeneration ||
              builder.oneWireConfigurationRevision !=
                  transaction.configurationRevision)) ||
            transaction.ownerToken != config_.expectedOneWireOwnerToken ||
            transaction.configurationRevision !=
                config_.expectedOneWireConfigurationRevision ||
            !builder.cycleBegun ||
            builder.policyGeneration != policyGeneration_ ||
            !builder.searchFinished)
            return valid.ok () ? Status (StatusCode::InvalidArgument) : valid;
        Ds18b20NormalizedProbeWitness& witness = builder.probes[index];
        const uint32_t predecessor =
            witness.conversionStatusPresent
                ? witness.conversionStatus.transactionGeneration
                : witness.conversionStart.transactionGeneration;
        const uint32_t predecessorRequest =
            witness.conversionStatusPresent
                ? witness.conversionStatus.requestSequence
                : witness.conversionStart.requestSequence;
        const uint32_t predecessorCompletedAt =
            (witness.conversionStatusPresent
                 ? witness.conversionStatus.completedAt
                 : witness.conversionStart.completedAt)
                .microseconds ();
        if (witness.conversionGeneration != conversionGeneration ||
            witness.conversionStart.transactionGeneration == 0 ||
            witness.scratchpadRead.transactionGeneration != 0 ||
            transaction.transactionGeneration != predecessor + 1U ||
            transaction.request.requestSequence != predecessorRequest + 1U ||
            !notBefore (transaction.request.startedAt.microseconds (),
                        predecessorCompletedAt))
            return StatusCode::InvalidArgument;
        if (!followsTaggedTransaction (
                transaction, builder.searchPasses, builder.probes,
                builder.lastTransactionTag))
            return StatusCode::InvalidArgument;
        normalize (transaction, witness.scratchpadRead);
        builder.lastTransactionTag =
            static_cast<uint8_t> (7U + static_cast<uint8_t> (index) * 3U);
        witness.scratchpadObservedAt  = scratchpadObservedAt;
        memcpy (witness.scratchpad, transaction.readBytes,
                sizeof witness.scratchpad);
        return {};
    }

    Status Qualified18B20ProbeSetPolicy::finalizeCycle (
        TimePoint now, const Ds18b20CycleBuilder& builder,
        QualifiedDs18b20Snapshot& output) noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        const bool noTransactionWitness =
            builder.searchPassCount == 0 && builder.probeCount == 0 &&
            !builder.status.ok ();
        if (builder.sourceId != config_.expectedSourceId ||
            builder.configurationRevision !=
                config_.expectedConfigurationRevision ||
            builder.cycleSequence == 0 || !builder.cycleBegun ||
            builder.policyGeneration != policyGeneration_ ||
            !builder.searchFinished || builder.searchPassCount > 4 ||
            builder.probeCount > 4 ||
            (builder.searchComplete && builder.searchOverCapacity) ||
            (!noTransactionWitness &&
             (builder.oneWireOwnerToken !=
                  config_.expectedOneWireOwnerToken ||
              builder.oneWireConfigurationRevision !=
                  config_.expectedOneWireConfigurationRevision ||
              builder.oneWireLifecycleGeneration == 0)) ||
            !notBefore (now.milliseconds (), builder.observedAt.milliseconds ()))
            return StatusCode::InvalidArgument;

        if (hasLastCycle_)
        {
            if (builder.cycleSequence == lastCycle_.cycleSequence)
            {
                bool same = false;
                if (noTransactionWitness &&
                    lastCycle_.searchPassCount == 0 &&
                    lastCycle_.probeCount == 0 &&
                    !lastCycle_.status.ok ())
                {
                    same =
                        builder.sourceId == lastCycle_.sourceId &&
                        builder.configurationRevision ==
                            lastCycle_.configurationRevision &&
                        builder.cycleSequence == lastCycle_.cycleSequence &&
                        builder.observedAt == lastCycle_.observedAt &&
                        builder.policyGeneration ==
                            lastCycle_.policyGeneration &&
                        builder.cycleBegun == lastCycle_.cycleBegun &&
                        builder.searchFinished ==
                            lastCycle_.searchFinished &&
                        builder.searchComplete ==
                            lastCycle_.searchComplete &&
                        builder.searchOverCapacity ==
                            lastCycle_.searchOverCapacity &&
                        builder.status == lastCycle_.status;
                }
                else
                    same = memcmp (&builder, &lastCycle_,
                                   sizeof builder) == 0;
                if (!same)
                    return StatusCode::InvalidArgument;
                output = snapshot_;
                return snapshot_.status;
            }
            if (!forward (builder.cycleSequence, lastCycle_.cycleSequence) ||
                !forward (builder.observedAt.milliseconds (),
                          lastCycle_.observedAt.milliseconds ()) ||
                (!noTransactionWitness &&
                 lastCycle_.oneWireLifecycleGeneration != 0 &&
                 builder.oneWireLifecycleGeneration !=
                     lastCycle_.oneWireLifecycleGeneration))
                return StatusCode::InvalidArgument;
        }
        if (hasLastCycle_)
        {
            const Ds18b20NormalizedTransactionRef* anchor =
                taggedReference (
                    lastCycle_.searchPasses, lastCycle_.probes,
                    lastCycle_.lastTransactionTag);
            if (anchor != 0 &&
                !referencesFollow (
                    builder.searchPasses, builder.searchPassCount,
                    builder.probes, *anchor))
                return StatusCode::InvalidArgument;
        }
        for (uint8_t index = 0; index < 4; ++index)
        {
            const Ds18b20NormalizedProbeWitness& witness =
                builder.probes[index];
            const uint32_t scratchObservedAt =
                witness.scratchpadObservedAt.milliseconds ();
            const uint32_t cycleObservedAt =
                builder.observedAt.milliseconds ();
            const uint32_t priorObservedAt =
                snapshot_.probes[index].observedAt.milliseconds ();
            const QualifiedDs18b20Probe& prior = snapshot_.probes[index];
            if (prior.readTransactionGeneration != 0 &&
                !notBefore (now.milliseconds (), priorObservedAt))
                return StatusCode::InvalidArgument;
            if (witness.scratchpadRead.transactionGeneration != 0 &&
                (!notBefore (scratchObservedAt, cycleObservedAt) ||
                 !notBefore (now.milliseconds (),
                             scratchObservedAt)))
                return StatusCode::InvalidArgument;
            if (witness.scratchpadRead.transactionGeneration != 0 &&
                prior.readTransactionGeneration != 0 &&
                !notBefore (scratchObservedAt, priorObservedAt))
                return StatusCode::InvalidArgument;
            if (witness.conversionStart.transactionGeneration != 0 &&
                prior.conversionGeneration != 0 &&
                (!forward (witness.conversionGeneration,
                           prior.conversionGeneration) ||
                 !forward (
                     witness.conversionStart.transactionGeneration,
                     prior.readTransactionGeneration)))
                return StatusCode::InvalidArgument;
        }

        QualifiedDs18b20Snapshot next = snapshot_;
        next.sourceId              = builder.sourceId;
        next.configurationRevision = builder.configurationRevision;
        next.cycleSequence         = builder.cycleSequence;
        next.observedAt            = builder.observedAt;
        next.validCount            = 0;
        next.presentMask           = 0;
        next.faultMask             = 0;
        next.status                = {};

        bool invalidDiscovered = false;
        bool duplicate         = false;
        uint8_t duplicateMask  = 0;
        bool unknown           = false;
        uint8_t seenMask       = 0;
        if (builder.searchComplete)
        {
            if (builder.searchPassCount == 0 ||
                !builder.searchPasses[builder.searchPassCount - 1U]
                     .completedSearch.lastDevice)
                return StatusCode::InvalidArgument;
            for (uint8_t pass = 0; pass < builder.searchPassCount; ++pass)
            {
                const OneWireRomCode& rom =
                    builder.searchPasses[pass].completedSearch.rom;
                if (!validRom (rom))
                    invalidDiscovered = true;
                const int8_t index = configuredIndex (config_, rom);
                if (index < 0)
                    unknown = validRom (rom) || unknown;
                else if ((seenMask & (1U << index)) != 0)
                {
                    duplicate = true;
                    duplicateMask = static_cast<uint8_t> (
                        duplicateMask | (1U << index));
                }
                else
                    seenMask = static_cast<uint8_t> (seenMask | (1U << index));
            }
            next.presentMask = seenMask;
        }

        const bool transport =
            !builder.status.ok () || !builder.searchComplete ||
            builder.searchOverCapacity || invalidDiscovered;
        bool missing = false;
        for (uint8_t index = 0; index < 4; ++index)
        {
            QualifiedDs18b20Probe& result = next.probes[index];
            const Ds18b20ProbeConfig& config = config_.probes[index];
            const Ds18b20NormalizedProbeWitness& witness =
                builder.probes[index];
            result.rom           = config.rom;
            result.cycleSequence = builder.cycleSequence;
            result.resolution    = config.resolution;
            const bool priorTrusted =
                result.readTransactionGeneration != 0 &&
                result.freshThrough != result.observedAt;
            if (priorTrusted &&
                notBefore (now.milliseconds (),
                           result.observedAt.milliseconds ()))
                result.age = now.elapsedSince (result.observedAt);

            if (transport)
            {
                result.quality = Ds18b20ProbeQuality::TransportFault;
                result.status  = builder.status.ok ()
                                     ? Status (StatusCode::HardwareFailure)
                                     : builder.status;
            }
            else if ((duplicateMask & (1U << index)) != 0)
            {
                result.quality = Ds18b20ProbeQuality::DuplicateIdentity;
                result.status  = {};
            }
            else if ((seenMask & (1U << index)) == 0)
            {
                result.quality = Ds18b20ProbeQuality::Missing;
                result.status  = {};
                missing        = true;
            }
            else if (witness.conversionStart.transactionGeneration == 0)
            {
                result.quality = Ds18b20ProbeQuality::TransportFault;
                result.status  = StatusCode::HardwareFailure;
            }
            else
            {
                const uint32_t statusAt =
                    witness.conversionStatus.completedAt.microseconds ();
                const uint32_t conversionAt =
                    witness.conversionStart.startedAt.microseconds ();
                const uint32_t elapsed = statusAt - conversionAt;
                const uint32_t deadlineUs =
                    resolutionDeadline (config.resolution) * 1000UL;
                const int16_t scratchRaw = static_cast<int16_t> (
                    static_cast<uint16_t> (witness.scratchpad[0]) |
                    static_cast<uint16_t> (
                        static_cast<uint16_t> (witness.scratchpad[1]) << 8U));
                const uint8_t scratchResolution =
                    static_cast<uint8_t> ((witness.scratchpad[4] >> 5U) & 3U);
                const bool resetDefault =
                    witness.scratchpadRead.transactionGeneration != 0 &&
                    crc8 (witness.scratchpad, 8) == witness.scratchpad[8] &&
                    (witness.scratchpad[4] & 0x9fU) == 0x1fU &&
                    scratchResolution ==
                        static_cast<uint8_t> (config.resolution) &&
                    scratchRaw == 1360 &&
                    !witness.conversionCompletedHigh;
                if (resetDefault)
                {
                    result.rawSixteenths        = scratchRaw;
                    result.lowerRawSixteenths   = scratchRaw;
                    result.upperRawSixteenths   = scratchRaw;
                    result.observedAt           = witness.scratchpadObservedAt;
                    result.freshThrough         = witness.scratchpadObservedAt;
                    result.age = now.elapsedSince (witness.scratchpadObservedAt);
                    result.conversionGeneration =
                        witness.conversionGeneration;
                    result.readTransactionGeneration =
                        witness.scratchpadRead.transactionGeneration;
                    result.quality =
                        Ds18b20ProbeQuality::ResetDefaultWithoutConversion;
                    result.status = {};
                }
                else if (
                    witness.conversionStatus.transactionGeneration == 0)
                {
                    result.quality = Ds18b20ProbeQuality::TransportFault;
                    result.status  = StatusCode::HardwareFailure;
                }
                else if (!witness.conversionCompletedHigh)
                {
                    result.quality =
                        elapsed < deadlineUs
                            ? Ds18b20ProbeQuality::ConversionPending
                            : Ds18b20ProbeQuality::TransportFault;
                    result.status =
                        result.quality == Ds18b20ProbeQuality::TransportFault
                            ? Status (StatusCode::Timeout)
                            : Status ();
                }
                else if (elapsed > deadlineUs ||
                         witness.scratchpadRead.transactionGeneration == 0)
                {
                    result.quality = Ds18b20ProbeQuality::TransportFault;
                    result.status  = elapsed > deadlineUs
                                         ? Status (StatusCode::Timeout)
                                         : Status (StatusCode::HardwareFailure);
                }
                else if (crc8 (witness.scratchpad, 8) !=
                         witness.scratchpad[8])
                {
                    result.quality =
                        Ds18b20ProbeQuality::ScratchpadCrcFault;
                    result.status = {};
                }
                else
                {
                    const uint8_t configByte = witness.scratchpad[4];
                    const uint8_t resolutionBits =
                        static_cast<uint8_t> ((configByte >> 5U) & 3U);
                    const Ds18b20Resolution resolution =
                        static_cast<Ds18b20Resolution> (resolutionBits);
                    if ((configByte & 0x9fU) != 0x1fU ||
                        resolution != config.resolution)
                    {
                        result.quality =
                            Ds18b20ProbeQuality::ResolutionMismatch;
                        result.status = {};
                    }
                    else
                    {
                        const uint16_t rawBits =
                            static_cast<uint16_t> (witness.scratchpad[0]) |
                            static_cast<uint16_t> (
                                static_cast<uint16_t> (witness.scratchpad[1])
                                << 8U);
                        const uint8_t undefinedMask =
                            static_cast<uint8_t> (
                                (UINT8_C (1)
                                 << (3U - static_cast<uint8_t> (resolution)))
                                - 1U);
                        const uint16_t maskedBits = static_cast<uint16_t> (
                            rawBits &
                            static_cast<uint16_t> (~undefinedMask));
                        const int32_t maskedWide =
                            maskedBits <= INT16_MAX
                                ? static_cast<int32_t> (maskedBits)
                                : static_cast<int32_t> (maskedBits) - 65536L;
                        const int16_t masked =
                            static_cast<int16_t> (maskedWide);
                        const int32_t upperWide =
                            maskedWide + undefinedMask;
                        const int32_t delta =
                            static_cast<int32_t> (masked) -
                            static_cast<int32_t> (result.rawSixteenths);
                        const uint32_t magnitude =
                            static_cast<uint32_t> (delta < 0 ? -delta : delta);
                        result.rawSixteenths   = masked;
                        result.lowerRawSixteenths = masked;
                        result.upperRawSixteenths =
                            upperWide > INT16_MAX
                                ? INT16_MAX
                                : static_cast<int16_t> (upperWide);
                        result.observedAt = witness.scratchpadObservedAt;
                        const uint32_t freshAt =
                            witness.scratchpadObservedAt.milliseconds ();
                        const uint32_t maximumAge =
                            config.maximumAge.milliseconds ();
                        result.freshThrough = TimePoint (
                            freshAt + maximumAge);
                        result.age        = now.elapsedSince (
                            witness.scratchpadObservedAt);
                        result.conversionGeneration =
                            witness.conversionGeneration;
                        result.readTransactionGeneration =
                            witness.scratchpadRead.transactionGeneration;
                        if (masked < config.minimumRawSixteenths ||
                                 upperWide >
                                     config.maximumRawSixteenths ||
                                 (priorTrusted &&
                                  magnitude >
                                      config.maximumStepRawSixteenths))
                            result.quality =
                                Ds18b20ProbeQuality::ImplausibleStep;
                        else if (result.age > config.maximumAge)
                            result.quality = Ds18b20ProbeQuality::Stale;
                        else
                            result.quality = Ds18b20ProbeQuality::Current;
                        result.status = {};
                    }
                }
            }
            if (result.quality == Ds18b20ProbeQuality::Current)
                ++next.validCount;
            else
                next.faultMask =
                    static_cast<uint8_t> (next.faultMask | (1U << index));
            if (next.status.ok () && !result.status.ok ())
                next.status = result.status;
        }

        if (transport)
            next.quality = Ds18b20SetQuality::TransportFault;
        else if (duplicate)
            next.quality = Ds18b20SetQuality::DuplicateIdentity;
        else if (unknown)
            next.quality = Ds18b20SetQuality::UnknownIdentity;
        else if (missing)
            next.quality = Ds18b20SetQuality::Missing;
        else
            next.quality = Ds18b20SetQuality::Complete;
        if (!builder.status.ok ())
            next.status = builder.status;

        snapshot_ = next;
        if (noTransactionWitness && hasLastCycle_)
        {
            lastCycle_.sourceId              = builder.sourceId;
            lastCycle_.configurationRevision =
                builder.configurationRevision;
            lastCycle_.cycleSequence    = builder.cycleSequence;
            lastCycle_.observedAt       = builder.observedAt;
            lastCycle_.policyGeneration = builder.policyGeneration;
            lastCycle_.searchPassCount  = 0;
            lastCycle_.probeCount       = 0;
            lastCycle_.cycleBegun       = builder.cycleBegun;
            lastCycle_.searchFinished   = builder.searchFinished;
            lastCycle_.searchComplete   = builder.searchComplete;
            lastCycle_.searchOverCapacity =
                builder.searchOverCapacity;
            lastCycle_.status = builder.status;
        }
        else
            memcpy (static_cast<void*> (&lastCycle_), &builder,
                    sizeof builder);
        hasLastCycle_ = true;
        output        = snapshot_;
        return snapshot_.status;
    }

    Status Qualified18B20ProbeSetPolicy::snapshot (
        QualifiedDs18b20Snapshot& snapshot) const noexcept
    {
        if (!initialized_)
            return StatusCode::NotInitialized;
        snapshot = snapshot_;
        return {};
    }

    bool Qualified18B20ProbeSetPolicy::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
