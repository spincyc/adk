#include "one_wire_transaction_policy.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange       = 0x80000000UL;
        constexpr uint8_t  initializedFlag = 1U << 0;
        constexpr uint8_t  cleanupFlag     = 1U << 1;
        constexpr uint8_t  closingFlag     = 1U << 2;
        constexpr uint8_t  receiptFlag     = 1U << 3;
        constexpr uint8_t  searchIdFlag    = 1U << 4;
        constexpr uint8_t  cleanupTriggerFlag = searchIdFlag;
        constexpr uint8_t  searchCompFlag  = 1U << 5;
        constexpr uint8_t  failedFlag      = 1U << 6;
        constexpr uint8_t  searchBranchFlag = 1U << 7;

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validOperation (OneWireOperation operation) noexcept
        {
            return operation >= OneWireOperation::ResetPresence &&
                   operation <= OneWireOperation::MatchRomReadScratchpad;
        }

        bool validSupply (OneWireSupplyMode supply) noexcept
        {
            return supply == OneWireSupplyMode::ExternallyPowered ||
                   supply == OneWireSupplyMode::ParasitePower;
        }

        bool validIntent (OneWireLineIntent intent) noexcept
        {
            return intent >= OneWireLineIntent::Release &&
                   intent <= OneWireLineIntent::Sample;
        }

        bool validWindow (MicrosecondDuration minimum,
                          MicrosecondDuration maximum) noexcept
        {
            return minimum.microseconds () != 0 &&
                   minimum.microseconds () <= maximum.microseconds () &&
                   maximum.microseconds () < halfRange;
        }

        MicrosecondTimePoint add (MicrosecondTimePoint point,
                                  MicrosecondDuration  duration) noexcept
        {
            return MicrosecondTimePoint (point.microseconds () +
                                         duration.microseconds ());
        }

        bool atOrAfter (MicrosecondTimePoint value,
                        MicrosecondTimePoint boundary) noexcept
        {
            return value.elapsedSince (boundary).microseconds () < halfRange;
        }

        bool inWindow (MicrosecondTimePoint value, MicrosecondTimePoint earliest,
                       MicrosecondTimePoint latest) noexcept
        {
            return atOrAfter (value, earliest) && atOrAfter (latest, value);
        }

        bool zeroRom (const OneWireRomCode& rom) noexcept
        {
            uint8_t combined = 0;
            for (uint8_t index = 0; index < 8; ++index)
            {
                combined = static_cast<uint8_t> (combined | rom.bytes[index]);
            }
            return combined == 0;
        }

        bool sameRom (const OneWireRomCode& left,
                      const OneWireRomCode& right) noexcept
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

        OneWireRomCode emptyRom () noexcept
        {
            return {{0, 0, 0, 0, 0, 0, 0, 0}};
        }

        OneWireOperationRequest emptyRequest () noexcept
        {
            return {0,
                    OneWireOperation::ResetPresence,
                    emptyRom (),

                    {emptyRom (), 0, false},

                    MicrosecondTimePoint (),
                    OneWireSupplyMode::ExternallyPowered,
                    StatusCode::NotInitialized};
        }

        uint8_t commandFor (OneWireOperation operation) noexcept
        {
            switch (operation)
            {
                case OneWireOperation::SearchRomPass: return 0xf0;
                case OneWireOperation::ReadRomSingleDrop: return 0x33;
                case OneWireOperation::MatchRomReadPowerSupply: return 0xb4;
                case OneWireOperation::MatchRomStartConversion:
                case OneWireOperation::MatchRomReadConversionStatus: return 0x44;
                case OneWireOperation::MatchRomReadScratchpad: return 0xbe;
                default: return 0;
            }
        }

        uint16_t writeSlots (OneWireOperation operation) noexcept
        {
            if (operation == OneWireOperation::ResetPresence)
            {
                return 0;
            }
            if (operation == OneWireOperation::SearchRomPass ||
                operation == OneWireOperation::ReadRomSingleDrop)
            {
                return 8;
            }
            return 80;
        }

        uint16_t readSlots (OneWireOperation operation) noexcept
        {
            switch (operation)
            {
                case OneWireOperation::SearchRomPass: return 128;
                case OneWireOperation::ReadRomSingleDrop: return 64;
                case OneWireOperation::MatchRomReadPowerSupply:
                case OneWireOperation::MatchRomReadConversionStatus: return 1;
                case OneWireOperation::MatchRomReadScratchpad: return 72;
                default: return 0;
            }
        }

        uint16_t requiredSlots (OneWireOperation operation) noexcept
        {
            if (operation == OneWireOperation::SearchRomPass)
            {
                return 200;
            }
            return static_cast<uint16_t> (
                writeSlots (operation) + readSlots (operation));
        }

        OneWireStepReceipt emptyReceipt () noexcept
        {
            return {0,
                    0,
                    0,
                    MicrosecondTimePoint (),
                    0,
                    0,
                    0,
                    0,
                    OneWireOperation::ResetPresence,
                    OneWirePhase::Inert,
                    0,
                    0,
                    OneWireLineIntent::Release,
                    false,
                    false,
                    StatusCode::NotInitialized};
        }

        bool sameReceipt (const OneWireStepReceipt& left,
                          const OneWireStepReceipt& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt.microseconds () ==
                       right.observedAt.microseconds () &&
                   left.ownerToken == right.ownerToken &&
                   left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.requestSequence == right.requestSequence &&
                   left.transactionGeneration == right.transactionGeneration &&
                   left.operation == right.operation && left.phase == right.phase &&
                   left.phaseSequence == right.phaseSequence &&
                   left.slotIndex == right.slotIndex &&
                   left.appliedIntent == right.appliedIntent &&
                   left.sampledHigh == right.sampledHigh &&
                   left.accepted == right.accepted && left.status == right.status;
        }
    } // namespace

    OneWireTransactionPolicy::OneWireTransactionPolicy (
        const OneWireTransactionConfig& config) noexcept
        : config_                (config),
          snapshot_              (),
          lifecycleStartedAt_    (),
          phaseStartedAt_        (),
          slotStartedAt_         (),
          lastAcceptedAt_        (),
          lifecycleGeneration_   (0),
          transactionGeneration_ (0),
          phaseSequence_         (0),
          lastReceipt_           (),
          slotIndex_             (0),
          terminalQuality_       (OneWireTransactionQuality::Unqualified),
          terminalStatus_        (StatusCode::Ok),
          searchLastDiscrepancy_ (0),
          step_                  (Step::Cleanup),
          flags_                 (0)
    {
        lastReceipt_ = emptyReceipt ();

        clearSnapshot ();
    }

    Status OneWireTransactionPolicy::validateConfig () const noexcept
    {
        if (config_.ownerToken == 0 || config_.configurationRevision == 0 ||
            config_.expectedReceiptSourceId == 0 ||
            config_.expectedReceiptConfigurationRevision == 0 ||
            !validWindow (config_.resetLowMinimum, config_.resetLowMaximum) ||
            !validWindow (config_.resetReleaseMinimum, config_.resetReleaseMaximum) ||
            !validWindow (config_.presenceStartMinimum, config_.presenceStartMaximum) ||
            !validWindow (config_.presenceLowMinimum, config_.presenceLowMaximum) ||
            !validWindow (config_.writeZeroLowMinimum, config_.writeZeroLowMaximum) ||
            !validWindow (config_.writeOneLowMinimum, config_.writeOneLowMaximum) ||
            !validWindow (config_.readInitiateMinimum, config_.readInitiateMaximum) ||
            !validWindow (config_.readSampleMinimum, config_.readSampleMaximum) ||
            !validWindow (config_.completeSlotMinimum, config_.completeSlotMaximum) ||
            !validWindow (config_.slotRecoveryMinimum, config_.slotRecoveryMaximum) ||

            (config_.presenceStartMinimum.microseconds () >
                 config_.resetReleaseMaximum.microseconds () ||
             config_.resetReleaseMinimum.microseconds () >
                 config_.presenceStartMaximum.microseconds ()) ||

            config_.transactionDeadline.microseconds () == 0 ||
            config_.transactionDeadline.microseconds () >= halfRange ||
            config_.maximumSlots == 0)
        {
            return StatusCode::InvalidConfiguration;
        }
        return StatusCode::Ok;
    }

    void OneWireTransactionPolicy::clearSnapshot () noexcept
    {
        snapshot_.operation    = OneWireOperation::ResetPresence;
        snapshot_.phase        = OneWirePhase::Inert;
        snapshot_.quality      = OneWireTransactionQuality::Unqualified;
        snapshot_.request      = emptyRequest ();

        snapshot_.searchResult = {emptyRom (), 0, false};

        snapshot_.returnedRom  = emptyRom ();
        for (uint8_t index = 0; index < 9; ++index)
        {
            snapshot_.readBytes[index] = 0;
        }
        snapshot_.readByteCount     = 0;
        snapshot_.acceptedSlotCount = 0;
        snapshot_.presenceSeen      = false;
        snapshot_.releaseRequested  = false;
        snapshot_.releaseConfirmed  = false;
        snapshot_.completedAt       = MicrosecondTimePoint ();
        snapshot_.status            = StatusCode::NotInitialized;
        snapshot_.ownerToken        = 0;
        snapshot_.lifecycleGeneration = 0;
        snapshot_.configurationRevision = 0;
        snapshot_.transactionGeneration = 0;
        terminalQuality_            = OneWireTransactionQuality::Unqualified;
        terminalStatus_             = StatusCode::Ok;
        searchLastDiscrepancy_      = 0;
    }

    Status OneWireTransactionPolicy::startCleanup (MicrosecondTimePoint now,
                                                   OneWireStepIntent&   intent,
                                                   bool closing) noexcept
    {
        if (lifecycleGeneration_ == UINT32_MAX || phaseSequence_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        ++lifecycleGeneration_;
        ++phaseSequence_;
        flags_ |= initializedFlag | cleanupFlag;
        if (closing)
        {
            flags_ |= closingFlag;
        }
        else
        {
            flags_ &= static_cast<uint8_t> (~closingFlag);
        }
        step_                      = Step::Cleanup;
        lifecycleStartedAt_        = now;
        phaseStartedAt_            = now;
        lastAcceptedAt_            = now;
        snapshot_.phase            = OneWirePhase::RollingBack;
        snapshot_.quality          = OneWireTransactionQuality::ReleaseUnconfirmed;
        snapshot_.releaseRequested = true;
        snapshot_.releaseConfirmed = false;
        snapshot_.status = (flags_ & failedFlag) != 0 ?
                               terminalStatus_ :
                               Status (StatusCode::Ok);
        snapshot_.ownerToken            = config_.ownerToken;
        snapshot_.lifecycleGeneration   = lifecycleGeneration_;
        snapshot_.configurationRevision = config_.configurationRevision;
        snapshot_.transactionGeneration = transactionGeneration_;

        return emitCurrent (now, intent);
    }

    Status OneWireTransactionPolicy::completeReleased (
        MicrosecondTimePoint now, OneWireStepIntent& intent) noexcept
    {
        snapshot_.phase            = OneWirePhase::Complete;
        snapshot_.quality          = OneWireTransactionQuality::Complete;
        snapshot_.releaseRequested = true;
        snapshot_.releaseConfirmed = false;
        snapshot_.completedAt      = now;
        snapshot_.status           = StatusCode::Ok;
        terminalQuality_           = OneWireTransactionQuality::Complete;
        terminalStatus_            = StatusCode::Ok;
        step_                      = Step::Cleanup;
        phaseStartedAt_            = now;
        flags_ |= cleanupFlag;

        return emitCurrent (now, intent);
    }

    Status
    OneWireTransactionPolicy::initialize (MicrosecondTimePoint now,
                                          OneWireStepIntent&   releaseIntent) noexcept
    {
        if ((flags_ & initializedFlag) != 0)
        {
            if (!validProgression (now))
            {
                return StatusCode::InvalidArgument;
            }
            lastAcceptedAt_ = now;
            return emitCurrent (phaseStartedAt_, releaseIntent);
        }
        const Status valid = validateConfig ();

        if (!valid.ok ())
        {
            return valid;
        }
        clearSnapshot ();

        return startCleanup (now, releaseIntent, false);
    }

    bool OneWireTransactionPolicy::addressedOperation () const noexcept
    {
        return snapshot_.operation >= OneWireOperation::MatchRomReadPowerSupply &&
               snapshot_.operation <= OneWireOperation::MatchRomReadScratchpad;
    }

    bool OneWireTransactionPolicy::validProgression (
        MicrosecondTimePoint now) const noexcept
    {
        return now.elapsedSince (lastAcceptedAt_).microseconds () < halfRange &&
               now.elapsedSince (lifecycleStartedAt_).microseconds () < halfRange;
    }

    bool OneWireTransactionPolicy::currentWriteBit () const noexcept
    {
        const uint16_t index = slotIndex_;
        if (snapshot_.operation == OneWireOperation::SearchRomPass &&
            index >= 8)
        {
            return (flags_ & searchBranchFlag) != 0;
        }
        uint8_t        byte  = commandFor (snapshot_.operation);
        uint8_t        bit   = static_cast<uint8_t> (index);

        if (addressedOperation ())
        {
            if (index < 8)
            {
                byte = 0x55;
                bit  = static_cast<uint8_t> (index);
            }
            else if (index < 72)
            {
                const uint16_t romIndex = static_cast<uint16_t> (index - 8);
                byte = snapshot_.request.addressedRom.bytes[romIndex / 8U];
                bit  = static_cast<uint8_t> (romIndex % 8U);
            }
            else
            {
                bit = static_cast<uint8_t> (index - 72);
            }
        }
        return ((byte >> bit) & 1U) != 0;
    }

    Status
    OneWireTransactionPolicy::emitCurrent (MicrosecondTimePoint now,
                                           OneWireStepIntent&   intent) const noexcept
    {
        intent.ownerToken            = config_.ownerToken;
        intent.lifecycleGeneration   = lifecycleGeneration_;
        intent.configurationRevision = config_.configurationRevision;
        intent.requestSequence       = snapshot_.request.requestSequence;
        intent.transactionGeneration = transactionGeneration_;
        intent.operation             = snapshot_.operation;
        intent.phase                 = snapshot_.phase;
        intent.phaseSequence         = phaseSequence_;
        intent.slotIndex             = slotIndex_;
        intent.writeBit              = false;
        intent.lineIntent            = OneWireLineIntent::Release;
        intent.sampleRequired        = false;
        intent.earliestAt            = now;
        intent.latestAt              = now;
        intent.addressedRom          = snapshot_.request.addressedRom;

        switch (step_)
        {
            case Step::Cleanup: intent.lineIntent = OneWireLineIntent::Release; break;
            case Step::ResetDrive:
                intent.lineIntent = OneWireLineIntent::DriveLow;
                break;
            case Step::ResetRelease:
                intent.lineIntent = OneWireLineIntent::Release;
                intent.earliestAt = add (phaseStartedAt_, config_.resetLowMinimum);
                intent.latestAt   = add (phaseStartedAt_, config_.resetLowMaximum);
                break;
            case Step::PresenceSample:
                intent.lineIntent     = OneWireLineIntent::Sample;
                intent.sampleRequired = true;
                intent.earliestAt =
                    add (phaseStartedAt_,
                         config_.presenceStartMinimum.microseconds () >
                                 config_.resetReleaseMinimum.microseconds () ?
                             config_.presenceStartMinimum :
                             config_.resetReleaseMinimum);
                intent.latestAt =
                    add (phaseStartedAt_,
                         config_.presenceStartMaximum.microseconds () <
                                 config_.resetReleaseMaximum.microseconds () ?
                             config_.presenceStartMaximum :
                             config_.resetReleaseMaximum);
                break;
            case Step::PresenceRelease:
                intent.lineIntent     = OneWireLineIntent::Sample;
                intent.sampleRequired = true;
                intent.earliestAt = add (phaseStartedAt_, config_.presenceLowMinimum);
                intent.latestAt   = add (phaseStartedAt_, config_.presenceLowMaximum);
                break;
            case Step::SlotDrive:
                intent.lineIntent = OneWireLineIntent::DriveLow;
                intent.writeBit =
                    snapshot_.phase == OneWirePhase::WriteSlot ?
                        currentWriteBit () :
                        true;
                break;
            case Step::SlotSample:
                intent.lineIntent     = OneWireLineIntent::Sample;
                intent.sampleRequired = true;
                intent.earliestAt = add (slotStartedAt_, config_.readSampleMinimum);
                intent.latestAt   = add (slotStartedAt_, config_.readSampleMaximum);
                break;
            case Step::SlotRelease:
                intent.lineIntent = OneWireLineIntent::Release;
                if (snapshot_.phase == OneWirePhase::WriteSlot)
                {
                    const bool bit = currentWriteBit ();
                    intent.earliestAt =
                        add (slotStartedAt_, bit ? config_.writeOneLowMinimum
                                                 : config_.writeZeroLowMinimum);
                    intent.latestAt =
                        add (slotStartedAt_, bit ? config_.writeOneLowMaximum
                                                 : config_.writeZeroLowMaximum);
                }
                else
                {
                    intent.earliestAt =
                        add (slotStartedAt_, config_.readInitiateMinimum);
                    intent.latestAt =
                        add (slotStartedAt_, config_.readInitiateMaximum);
                }
                break;
            case Step::SlotComplete:
                intent.lineIntent = OneWireLineIntent::Release;
                intent.earliestAt =
                    add (slotStartedAt_, config_.completeSlotMinimum);
                intent.latestAt =
                    add (slotStartedAt_, config_.completeSlotMaximum);
                break;
            case Step::SlotRecovery:
                intent.lineIntent = OneWireLineIntent::Release;
                intent.earliestAt =
                    add (phaseStartedAt_, config_.slotRecoveryMinimum);
                intent.latestAt =
                    add (phaseStartedAt_, config_.slotRecoveryMaximum);
                break;
        }
        return StatusCode::Ok;
    }

    Status OneWireTransactionPolicy::begin (MicrosecondTimePoint           now,
                                            const OneWireOperationRequest& request,
                                            OneWireStepIntent& intent) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if ((flags_ & cleanupFlag) != 0 ||
            (snapshot_.quality == OneWireTransactionQuality::Pending))
        {
            return StatusCode::ResourceBusy;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }
        if (!validStatus (request.status) || !validOperation (request.operation) ||
            !validSupply (request.supplyMode) || request.requestSequence == 0 ||

            request.startedAt.microseconds () != now.microseconds ())
        {
            return StatusCode::InvalidArgument;
        }
        if (!request.status.ok ())
        {
            return request.status;
        }
        if (request.supplyMode == OneWireSupplyMode::ParasitePower)
        {
            return StatusCode::Unsupported;
        }
        const bool continuation =
            request.operation ==
                OneWireOperation::MatchRomReadConversionStatus &&
            snapshot_.phase == OneWirePhase::Complete &&
            snapshot_.quality == OneWireTransactionQuality::Complete &&
            snapshot_.operation ==
                OneWireOperation::MatchRomStartConversion &&
            !zeroRom (request.addressedRom) &&

            sameRom (request.addressedRom,
                     snapshot_.request.addressedRom);
        const bool addressed =
            request.operation >= OneWireOperation::MatchRomReadPowerSupply;
        if ((addressed && zeroRom (request.addressedRom)) ||
            (request.operation == OneWireOperation::ReadRomSingleDrop &&
             !config_.singleDrop) ||
            (request.operation == OneWireOperation::SearchRomPass &&
             (request.search.lastDiscrepancy > 64 ||
              (request.search.lastDevice && request.search.lastDiscrepancy != 0))))
        {
            return addressed ? StatusCode::InvalidArgument : StatusCode::Unsupported;
        }
        const uint16_t operationSlots = continuation ?
                                            1 :
                                            requiredSlots (request.operation);
        if (operationSlots > config_.maximumSlots ||
            (request.operation ==
                 OneWireOperation::MatchRomReadConversionStatus &&
             !continuation))
        {
            return request.operation ==
                           OneWireOperation::MatchRomReadConversionStatus ?
                       StatusCode::InvalidArgument :
                       StatusCode::CapacityExceeded;
        }
        const uint32_t phaseReserve =
            static_cast<uint32_t> (operationSlots) * 5U + 6U;
        if (lifecycleGeneration_ == UINT32_MAX ||
            transactionGeneration_ == UINT32_MAX ||
            phaseSequence_ > UINT32_MAX - phaseReserve ||
            ((flags_ & receiptFlag) != 0 &&
             lastReceipt_.sequence == UINT32_MAX))
        {
            return StatusCode::CapacityExceeded;
        }

        ++transactionGeneration_;
        ++phaseSequence_;
        snapshot_.operation    = request.operation;
        snapshot_.phase        = continuation ? OneWirePhase::ReadSlot :
                                                OneWirePhase::ResetLow;
        snapshot_.quality      = OneWireTransactionQuality::Pending;
        snapshot_.request      = request;
        snapshot_.searchResult = {emptyRom (), 0, false};

        snapshot_.returnedRom  = emptyRom ();
        for (uint8_t index = 0; index < 9; ++index)
        {
            snapshot_.readBytes[index] = 0;
        }
        snapshot_.readByteCount     = 0;
        snapshot_.acceptedSlotCount = 0;
        snapshot_.presenceSeen      = false;
        snapshot_.releaseRequested  = false;
        snapshot_.releaseConfirmed  = false;
        snapshot_.completedAt       = MicrosecondTimePoint ();
        snapshot_.status            = StatusCode::Ok;
        snapshot_.ownerToken        = config_.ownerToken;
        snapshot_.lifecycleGeneration = lifecycleGeneration_;
        snapshot_.configurationRevision = config_.configurationRevision;
        snapshot_.transactionGeneration = transactionGeneration_;
        phaseStartedAt_             = now;
        slotStartedAt_              = now;
        lastAcceptedAt_             = now;
        slotIndex_                  = 0;
        searchLastDiscrepancy_      = 0;
        terminalQuality_            = OneWireTransactionQuality::Unqualified;
        terminalStatus_             = StatusCode::Ok;
        step_                       = continuation ? Step::SlotDrive :
                                                     Step::ResetDrive;
        flags_ &= static_cast<uint8_t> (
            ~ (failedFlag | searchIdFlag | searchCompFlag |
               searchBranchFlag));

        return emitCurrent (now, intent);
    }

    bool OneWireTransactionPolicy::receiptMatches (
        const OneWireStepReceipt& receipt) const noexcept
    {
        OneWireStepIntent expected;
        emitCurrent (phaseStartedAt_, expected);
        return receipt.sourceId == config_.expectedReceiptSourceId &&
               receipt.configurationRevision ==
                   config_.expectedReceiptConfigurationRevision &&
               receipt.ownerToken == expected.ownerToken &&
               receipt.lifecycleGeneration == expected.lifecycleGeneration &&
               receipt.requestSequence == expected.requestSequence &&
               receipt.transactionGeneration == expected.transactionGeneration &&
               receipt.operation == expected.operation &&
               receipt.phase == expected.phase &&
               receipt.phaseSequence == expected.phaseSequence &&
               receipt.slotIndex == expected.slotIndex &&
               receipt.appliedIntent == expected.lineIntent;
    }

    Status OneWireTransactionPolicy::fail (MicrosecondTimePoint      now,
                                           OneWireTransactionQuality quality,
                                           Status                    status,
                                           OneWireStepIntent&        intent,
                                           bool receiptTriggered) noexcept
    {
        if (lifecycleGeneration_ == UINT32_MAX ||
            phaseSequence_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        snapshot_.phase      = OneWirePhase::Fault;
        snapshot_.quality    = quality;
        snapshot_.status     = status;
        terminalQuality_     = quality;
        terminalStatus_      = status;
        flags_ |= failedFlag;
        if (receiptTriggered)
        {
            flags_ |= cleanupTriggerFlag;
        }
        else
        {
            flags_ &= static_cast<uint8_t> (~cleanupTriggerFlag);
        }
        const Status cleanup = startCleanup (now, intent, false);

        if (!cleanup.ok ())
        {
            snapshot_.status = cleanup;
            return cleanup;
        }
        return status;
    }

    Status OneWireTransactionPolicy::update (MicrosecondTimePoint      now,
                                             const OneWireStepReceipt& receipt,
                                             OneWireStepIntent&        intent) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if (!validStatus (receipt.status) ||
            !validIntent (receipt.appliedIntent))
        {
            return StatusCode::InvalidArgument;
        }
        if (receipt.observedAt.microseconds () != now.microseconds ())
        {
            return StatusCode::InvalidArgument;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }

        if ((flags_ & receiptFlag) != 0 && receipt.sequence == lastReceipt_.sequence)
        {
            if (!sameReceipt (receipt, lastReceipt_) ||
                (receipt.lifecycleGeneration != lifecycleGeneration_ &&
                 ((flags_ & cleanupTriggerFlag) == 0 ||
                  (flags_ & cleanupFlag) == 0)))
            {
                return StatusCode::InvalidArgument;
            }
            return emitCurrent (receipt.observedAt, intent);
        }
        if ((flags_ & cleanupFlag) != 0)
        {
            return StatusCode::ResourceBusy;
        }
        if (snapshot_.quality != OneWireTransactionQuality::Pending ||
            !receiptMatches (receipt))
        {
            return StatusCode::InvalidArgument;
        }
        if (receipt.sequence == 0 ||
            ((flags_ & receiptFlag) != 0 &&
             lastReceipt_.sequence == UINT32_MAX) ||
            ((flags_ & receiptFlag) != 0 &&
             receipt.sequence - lastReceipt_.sequence >= halfRange))
        {
            return StatusCode::InvalidArgument;
        }
        const uint32_t transactionElapsed =
            now.elapsedSince (snapshot_.request.startedAt).microseconds ();
        if (transactionElapsed >= halfRange)
        {
            return StatusCode::InvalidArgument;
        }
        if (transactionElapsed >
            config_.transactionDeadline.microseconds ())
        {
            flags_ |= receiptFlag;
            lastReceipt_    = receipt;
            lastAcceptedAt_ = receipt.observedAt;
            return fail (now, OneWireTransactionQuality::TimedOut,
                         StatusCode::Timeout, intent, true);
        }

        OneWireStepIntent expected;
        emitCurrent (phaseStartedAt_, expected);

        if (!inWindow (receipt.observedAt, expected.earliestAt, expected.latestAt))
        {
            flags_ |= receiptFlag;
            lastReceipt_    = receipt;
            lastAcceptedAt_ = receipt.observedAt;
            return fail (now, OneWireTransactionQuality::TimedOut, StatusCode::Timeout,
                         intent, true);
        }
        if (!receipt.status.ok () || !receipt.accepted)
        {
            flags_ |= receiptFlag;
            lastReceipt_    = receipt;
            lastAcceptedAt_ = receipt.observedAt;
            const Status status = receipt.status.ok ()
                                      ? Status (StatusCode::HardwareFailure)
                                      : receipt.status;
            return fail (now, OneWireTransactionQuality::ProducerFault, status, intent,
                         true);
        }

        flags_ |= receiptFlag;
        lastReceipt_    = receipt;
        lastAcceptedAt_ = receipt.observedAt;
        if (receipt.sequence == UINT32_MAX)
        {
            return fail (now, OneWireTransactionQuality::ProducerFault,
                         StatusCode::CapacityExceeded, intent, true);
        }
        if (step_ == Step::SlotRecovery)
        {
            ++snapshot_.acceptedSlotCount;
        }
        if (phaseSequence_ == UINT32_MAX)
        {
            return fail (now, OneWireTransactionQuality::ProducerFault,
                         StatusCode::CapacityExceeded, intent, true);
        }
        ++phaseSequence_;

        if (step_ == Step::ResetDrive)
        {
            step_           = Step::ResetRelease;
            phaseStartedAt_ = receipt.observedAt;
        }
        else if (step_ == Step::ResetRelease)
        {
            step_           = Step::PresenceSample;
            snapshot_.phase = OneWirePhase::PresenceWindow;
            phaseStartedAt_ = receipt.observedAt;
        }
        else if (step_ == Step::PresenceSample)
        {
            if (receipt.sampledHigh)
            {
                return fail (now, OneWireTransactionQuality::NoPresence, StatusCode::Ok,
                             intent, true);
            }
            snapshot_.presenceSeen = true;
            step_                  = Step::PresenceRelease;
            phaseStartedAt_        = receipt.observedAt;
        }
        else if (step_ == Step::PresenceRelease)
        {
            if (!receipt.sampledHigh)
            {
                return fail (now, OneWireTransactionQuality::Collision, StatusCode::Ok,
                             intent, true);
            }
            if (writeSlots (snapshot_.operation) == 0)
            {
                return completeReleased (now, intent);
            }
            snapshot_.phase = OneWirePhase::WriteSlot;
            step_           = Step::SlotDrive;
            slotIndex_      = 0;
            phaseStartedAt_ = receipt.observedAt;
            slotStartedAt_  = receipt.observedAt;
        }
        else if (step_ == Step::SlotDrive)
        {
            step_           = Step::SlotRelease;
            phaseStartedAt_ = receipt.observedAt;
            slotStartedAt_  = receipt.observedAt;
        }
        else if (step_ == Step::SlotSample)
        {
            const uint16_t bitIndex = slotIndex_;
            if (snapshot_.operation == OneWireOperation::ReadRomSingleDrop)
            {
                if (receipt.sampledHigh)
                {
                    snapshot_.returnedRom.bytes[bitIndex / 8U] |=
                        static_cast<uint8_t> (1U << (bitIndex % 8U));
                }
            }
            else if (snapshot_.operation != OneWireOperation::SearchRomPass)
            {
                if (receipt.sampledHigh && bitIndex < 72)
                {
                    snapshot_.readBytes[bitIndex / 8U] |=
                        static_cast<uint8_t> (1U << (bitIndex % 8U));
                }
            }
            else if (bitIndex >= 8)
            {
                const uint8_t searchOffset =
                    static_cast<uint8_t> ((bitIndex - 8U) % 3U);
                const uint8_t flag =
                    searchOffset == 0 ? searchIdFlag : searchCompFlag;
                if (receipt.sampledHigh)
                {
                    flags_ |= flag;
                }
                else
                {
                    flags_ &= static_cast<uint8_t> (~flag);
                }
            }
            step_           = Step::SlotComplete;
            phaseStartedAt_ = receipt.observedAt;
        }
        else if (step_ == Step::SlotRelease)
        {
            step_ = snapshot_.phase == OneWirePhase::ReadSlot ?
                        Step::SlotSample :
                        Step::SlotComplete;
            phaseStartedAt_ = receipt.observedAt;
        }
        else if (step_ == Step::SlotComplete)
        {
            step_           = Step::SlotRecovery;
            phaseStartedAt_ = receipt.observedAt;
        }
        else if (step_ == Step::SlotRecovery)
        {
            if (snapshot_.operation == OneWireOperation::SearchRomPass &&
                slotIndex_ >= 8)
            {
                const uint8_t searchOffset =
                    static_cast<uint8_t> ((slotIndex_ - 8U) % 3U);
                if (searchOffset == 1)
                {
                    const bool idHigh = (flags_ & searchIdFlag) != 0;
                    const bool complementHigh =
                        (flags_ & searchCompFlag) != 0;
                    if (idHigh && complementHigh)
                    {
                        return fail (
                            now, OneWireTransactionQuality::NoPresence,
                            StatusCode::Ok, intent, true);
                    }

                    const uint8_t bitNumber =
                        static_cast<uint8_t> ((slotIndex_ - 8U) / 3U + 1U);
                    bool branch = idHigh;
                    if (!idHigh && !complementHigh)
                    {
                        if (bitNumber <
                            snapshot_.request.search.lastDiscrepancy)
                        {
                            branch =
                                ((snapshot_.request.search.rom.bytes
                                      [(bitNumber - 1U) / 8U] >>
                                  ((bitNumber - 1U) % 8U)) &
                                 1U) != 0;
                        }
                        else
                        {
                            branch =
                                bitNumber ==
                                snapshot_.request.search.lastDiscrepancy;
                        }
                        if (!branch)
                        {
                            searchLastDiscrepancy_ = bitNumber;
                        }
                    }
                    if (branch)
                    {
                        flags_ |= searchBranchFlag;
                        snapshot_.searchResult.rom.bytes
                            [(bitNumber - 1U) / 8U] |=
                            static_cast<uint8_t> (
                                1U << ((bitNumber - 1U) % 8U));
                    }
                    else
                    {
                        flags_ &= static_cast<uint8_t> (
                            ~searchBranchFlag);
                    }
                }

                ++slotIndex_;
                if (slotIndex_ >= 200)
                {
                    snapshot_.searchResult.lastDiscrepancy =
                        searchLastDiscrepancy_;
                    snapshot_.searchResult.lastDevice =
                        searchLastDiscrepancy_ == 0;
                    return completeReleased (now, intent);
                }
                snapshot_.phase =
                    ((slotIndex_ - 8U) % 3U) == 2U ?
                        OneWirePhase::WriteSlot :
                        OneWirePhase::ReadSlot;
                step_           = Step::SlotDrive;
                phaseStartedAt_ = receipt.observedAt;
                slotStartedAt_  = receipt.observedAt;
                return emitCurrent (now, intent);
            }

            ++slotIndex_;
            if (snapshot_.phase == OneWirePhase::WriteSlot &&
                slotIndex_ >= writeSlots (snapshot_.operation))
            {
                slotIndex_ =
                    snapshot_.operation == OneWireOperation::SearchRomPass ?
                        8 :
                        0;
                if (readSlots (snapshot_.operation) == 0)
                {
                    return completeReleased (now, intent);
                }
                snapshot_.phase = OneWirePhase::ReadSlot;
                step_           = Step::SlotDrive;
            }
            else if (snapshot_.phase == OneWirePhase::ReadSlot &&
                     slotIndex_ >= readSlots (snapshot_.operation))
            {
                snapshot_.readByteCount =
                    static_cast<uint8_t> ((readSlots (snapshot_.operation) + 7U) / 8U);
                return completeReleased (now, intent);
            }
            else
            {
                step_ = Step::SlotDrive;
            }
            phaseStartedAt_ = receipt.observedAt;
            slotStartedAt_  = receipt.observedAt;
        }

        if (snapshot_.acceptedSlotCount > config_.maximumSlots)
        {
            return fail (now, OneWireTransactionQuality::ProducerFault,
                         StatusCode::CapacityExceeded, intent, true);
        }

        return emitCurrent (now, intent);
    }

    Status OneWireTransactionPolicy::advance (MicrosecondTimePoint now,
                                              OneWireStepIntent&   intent) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }
        if ((flags_ & cleanupFlag) != 0 ||
            snapshot_.quality != OneWireTransactionQuality::Pending)
        {
            lastAcceptedAt_ = now;
            return StatusCode::Ok;
        }
        const uint32_t elapsed =
            now.elapsedSince (snapshot_.request.startedAt).microseconds ();
        if (elapsed >= halfRange)
        {
            return StatusCode::InvalidArgument;
        }
        if (elapsed <= config_.transactionDeadline.microseconds ())
        {
            lastAcceptedAt_ = now;
            return StatusCode::Ok;
        }
        return fail (now, OneWireTransactionQuality::TimedOut, StatusCode::Timeout,
                     intent);
    }

    Status OneWireTransactionPolicy::cancel (MicrosecondTimePoint now,
                                             OneWireStepIntent&   intent) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if ((flags_ & cleanupFlag) != 0)
        {
            return StatusCode::ResourceBusy;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }
        return fail (now, OneWireTransactionQuality::ProducerFault,
                     StatusCode::InvalidArgument, intent);
    }

    Status OneWireTransactionPolicy::reset (MicrosecondTimePoint now,
                                            OneWireStepIntent&   releaseIntent) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }
        if (lifecycleGeneration_ == UINT32_MAX ||
            phaseSequence_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        clearSnapshot ();
        flags_ &= static_cast<uint8_t> (
            ~ (failedFlag | searchIdFlag | searchCompFlag |
               searchBranchFlag));
        flags_ |= initializedFlag;
        return startCleanup (now, releaseIntent, false);
    }

    Status
    OneWireTransactionPolicy::shutdown (MicrosecondTimePoint now,
                                        OneWireStepIntent&   releaseIntent) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }
        flags_ &= static_cast<uint8_t> (~cleanupTriggerFlag);
        return startCleanup (now, releaseIntent, true);
    }

    Status OneWireTransactionPolicy::confirmCleanup (
        MicrosecondTimePoint now, const OneWireStepReceipt& receipt) noexcept
    {
        if ((flags_ & initializedFlag) == 0)
        {
            return StatusCode::NotInitialized;
        }
        if (!validStatus (receipt.status) ||
            !validIntent (receipt.appliedIntent))
        {
            return StatusCode::InvalidArgument;
        }
        if (receipt.observedAt.microseconds () != now.microseconds ())
        {
            return StatusCode::InvalidArgument;
        }
        if (!validProgression (now))
        {
            return StatusCode::InvalidArgument;
        }

        if ((flags_ & receiptFlag) != 0 && receipt.sequence == lastReceipt_.sequence)
        {
            return receipt.lifecycleGeneration == lifecycleGeneration_ &&
                           sameReceipt (receipt, lastReceipt_)
                       ? snapshot_.status
                       : Status (StatusCode::InvalidArgument);
        }
        if ((flags_ & cleanupFlag) == 0 || !receiptMatches (receipt) ||
            !inWindow (receipt.observedAt, phaseStartedAt_, phaseStartedAt_) ||
            receipt.sequence == 0 ||
            ((flags_ & receiptFlag) != 0 &&
             lastReceipt_.sequence == UINT32_MAX) ||
            ((flags_ & receiptFlag) != 0 &&
             receipt.sequence - lastReceipt_.sequence >= halfRange))
        {
            return StatusCode::InvalidArgument;
        }
        if (!receipt.status.ok () || !receipt.accepted)
        {
            snapshot_.quality = OneWireTransactionQuality::ReleaseUnconfirmed;
            snapshot_.status  = receipt.status.ok ()
                                    ? Status (StatusCode::HardwareFailure)
                                    : receipt.status;
            flags_ |= receiptFlag;
            lastReceipt_    = receipt;
            lastAcceptedAt_ = receipt.observedAt;
            return snapshot_.status;
        }

        snapshot_.releaseConfirmed = true;
        flags_ &= static_cast<uint8_t> (~cleanupFlag);
        flags_ |= receiptFlag;
        lastReceipt_    = receipt;
        lastAcceptedAt_ = receipt.observedAt;
        if ((flags_ & closingFlag) != 0)
        {
            flags_ = 0;
            clearSnapshot ();
        }
        else if ((flags_ & failedFlag) != 0)
        {
            snapshot_.phase   = OneWirePhase::Fault;
            snapshot_.quality = terminalQuality_;
            snapshot_.status  = terminalStatus_;
        }
        else if (terminalQuality_ == OneWireTransactionQuality::Complete)
        {
            snapshot_.phase            = OneWirePhase::Complete;
            snapshot_.quality          = OneWireTransactionQuality::Complete;
            snapshot_.releaseRequested = true;
            snapshot_.releaseConfirmed = true;
            snapshot_.status           = StatusCode::Ok;
        }
        else
        {
            snapshot_.phase   = OneWirePhase::Inert;
            snapshot_.quality = OneWireTransactionQuality::Unqualified;
            snapshot_.status  = StatusCode::Ok;
        }
        return StatusCode::Ok;
    }

    Status OneWireTransactionPolicy::snapshot (
        OneWireTransactionSnapshot& snapshot) const noexcept
    {
        snapshot = snapshot_;
        return StatusCode::Ok;
    }

    Status OneWireTransactionPolicy::completedEvidence (
        OneWireTransactionSnapshot& evidence) const noexcept
    {
        if (snapshot_.phase == OneWirePhase::Complete &&
            snapshot_.quality == OneWireTransactionQuality::Complete &&
            snapshot_.releaseRequested && snapshot_.releaseConfirmed &&
            snapshot_.status.ok ())
        {
            evidence = snapshot_;
            return StatusCode::Ok;
        }
        return snapshot_.quality == OneWireTransactionQuality::Complete ?
                   StatusCode::ResourceBusy :
                   StatusCode::NotInitialized;
    }

    bool OneWireTransactionPolicy::initialized () const noexcept
    {
        return (flags_ & initializedFlag) != 0;
    }

#if defined(ADK_TESTING)
    void OneWireTransactionPolicy::seedSequencesForTest (
        uint32_t lifecycleGeneration,
        uint32_t transactionGeneration,
        uint32_t phaseSequence) noexcept
    {
        lifecycleGeneration_   = lifecycleGeneration;
        transactionGeneration_ = transactionGeneration;
        phaseSequence_         = phaseSequence;
    }
#endif
} // namespace adk
