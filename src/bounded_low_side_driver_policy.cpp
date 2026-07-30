#include "bounded_low_side_driver_policy.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool validStatus (Status status) noexcept
        {
            return static_cast<uint8_t> (status.error ()) <=
                   static_cast<uint8_t> (StatusCode::HardwareFailure);
        }

        bool validDescriptorEnums (const LowSideDriverDescriptor& descriptor) noexcept
        {
            return (descriptor.loadEnergy == LowSideLoadEnergy::ResistiveIndicator ||
                    descriptor.loadEnergy == LowSideLoadEnergy::InductiveInert) &&
                   (descriptor.flybackRequirement ==
                        LowSideFlybackRequirement::NotRequired ||
                    descriptor.flybackRequirement ==
                        LowSideFlybackRequirement::Required) &&
                   (descriptor.flybackDeclaration ==
                        LowSideFlybackDeclaration::Absent ||
                    descriptor.flybackDeclaration ==
                        LowSideFlybackDeclaration::Present);
        }

        bool validDuration (Duration duration) noexcept
        {
            return duration.milliseconds () != 0 &&
                   duration.milliseconds () < halfRange;
        }

        bool
        descriptorFlybackCanonical (const LowSideDriverDescriptor& descriptor) noexcept
        {
            if (descriptor.loadEnergy == LowSideLoadEnergy::ResistiveIndicator)
            {
                return descriptor.flybackRequirement ==
                           LowSideFlybackRequirement::NotRequired &&
                       descriptor.flybackDeclaration ==
                           LowSideFlybackDeclaration::Absent &&
                       descriptor.flybackDiodeIdentity == 0 &&
                       descriptor.flybackDiodeRevision == 0 &&
                       descriptor.flybackOrientationCode == 0 &&
                       descriptor.flybackReturnCode == 0 &&
                       descriptor.flybackRepetitiveReverseMv == 0 &&
                       descriptor.flybackForwardCurrentUa == 0;
            }

            if (descriptor.flybackRequirement != LowSideFlybackRequirement::Required)
            {
                return false;
            }
            if (descriptor.flybackDeclaration == LowSideFlybackDeclaration::Absent)
            {
                return descriptor.flybackDiodeIdentity == 0 &&
                       descriptor.flybackDiodeRevision == 0 &&
                       descriptor.flybackOrientationCode == 0 &&
                       descriptor.flybackReturnCode == 0 &&
                       descriptor.flybackRepetitiveReverseMv == 0 &&
                       descriptor.flybackForwardCurrentUa == 0;
            }
            return descriptor.flybackDiodeIdentity != 0 &&
                   descriptor.flybackDiodeRevision != 0 &&
                   descriptor.flybackOrientationCode != 0 &&
                   descriptor.flybackReturnCode != 0 &&
                   descriptor.flybackRepetitiveReverseMv != 0 &&
                   descriptor.flybackForwardCurrentUa != 0;
        }

        bool flybackComplete (const LowSideDriverDescriptor& descriptor) noexcept
        {
            if (descriptor.loadEnergy == LowSideLoadEnergy::ResistiveIndicator)
            {
                return true;
            }
            return descriptor.flybackDeclaration ==
                       LowSideFlybackDeclaration::Present &&
                   descriptor.flybackDiodeIdentity != 0 &&
                   descriptor.flybackDiodeRevision != 0 &&
                   descriptor.flybackOrientationCode != 0 &&
                   descriptor.flybackReturnCode != 0 &&
                   descriptor.flybackRepetitiveReverseMv >=
                       descriptor.budget.collectorEmitterOperatingMaximumMv &&
                   descriptor.flybackForwardCurrentUa >=
                       descriptor.budget.fixtureLoadCeilingUa;
        }

        uint32_t crcByte (uint32_t crc, uint8_t value) noexcept
        {
            crc ^= value;
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                crc = (crc & 1U) != 0 ? (crc >> 1U) ^ UINT32_C (0xEDB88320) : crc >> 1U;
            }
            return crc;
        }

        void digestU8 (uint32_t& crc, uint8_t value) noexcept
        {
            crc = crcByte (crc, value);
        }

        void digestU16 (uint32_t& crc, uint16_t value) noexcept
        {
            digestU8 (crc, static_cast<uint8_t> (value));
            digestU8 (crc, static_cast<uint8_t> (value >> 8U));
        }

        void digestU32 (uint32_t& crc, uint32_t value) noexcept
        {
            digestU16 (crc, static_cast<uint16_t> (value));
            digestU16 (crc, static_cast<uint16_t> (value >> 16U));
        }

        uint32_t requestDigest (const LowSideDriveRequest& request) noexcept
        {
            uint32_t crc = UINT32_C (0xFFFFFFFF);

            digestU32 (crc, request.sessionId);
            digestU32 (crc, request.runId);
            digestU16 (crc, request.stepId);
            digestU32 (crc, request.requestId);
            digestU8  (crc, request.sourceId);
            digestU16 (crc, request.sourceConfigurationRevision);
            digestU32 (crc, request.sequence);
            digestU32 (crc, request.observedAt.milliseconds ());
            digestU32 (crc, request.lifecycleGeneration);
            digestU8  (crc, request.logicalActive ? 1U : 0U);
            digestU32 (crc, request.requestedLoadUa);
            digestU32 (crc, request.requestedActiveDuration.milliseconds ());
            digestU8  (crc,
                      static_cast<uint8_t> (request.producerStatus.error ()));
            return crc ^ UINT32_C (0xFFFFFFFF);
        }

        uint32_t controlDigest (const LowSideControl& control) noexcept
        {
            uint32_t crc = UINT32_C (0xFFFFFFFF);

            digestU32 (crc, control.lifecycleGeneration);
            digestU32 (crc, control.sessionId);
            digestU32 (crc, control.runId);
            digestU16 (crc, control.stepId);
            digestU32 (crc, control.controlId);
            digestU8  (crc, control.sourceId);
            digestU16 (crc, control.sourceConfigurationRevision);
            digestU32 (crc, control.sequence);
            digestU32 (crc, control.observedAt.milliseconds ());
            digestU8  (crc, control.offConfirmed ? 1U : 0U);
            digestU8  (crc,
                      static_cast<uint8_t> (control.producerStatus.error ()));
            return crc ^ UINT32_C (0xFFFFFFFF);
        }

        LowSideDriveIntent baseIntent (
            const LowSideDriverDescriptor& descriptor, uint32_t descriptorDigest,
            uint32_t generation, uint32_t sessionId, uint32_t runId) noexcept
        {
            LowSideDriveIntent intent;
            intent.lifecycleGeneration = generation;
            intent.driverDescriptorIdentityDigest = descriptorDigest;
            intent.specimenReference           = descriptor.specimenReference;
            intent.specimenRevision            = descriptor.specimenRevision;
            intent.electricalEvidenceRevision  = descriptor.electricalEvidenceRevision;
            intent.policyConfigurationId       = descriptor.configurationId;
            intent.policyConfigurationRevision = descriptor.configurationRevision;
            intent.sessionId                   = sessionId;
            intent.runId                       = runId;
            intent.stepId                      = 0;
            intent.requestId                   = 0;
            intent.state                       = LowSideDriveState::Off;
            intent.reason                      = LowSideDriveReason::None;
            intent.logicalActive               = false;
            intent.outputLevelHigh             = false;
            intent.requiredBaseUa              = 0;
            intent.admittedBaseUa              = 0;
            intent.admittedLoadUa              = 0;
            intent.expiresAt                   = TimePoint ();
            intent.producerStatus              = Status    ();
            return intent;
        }

        void forceOff (LowSideDriveIntent& intent, LowSideDriveState state,
                       LowSideDriveReason reason, Status status) noexcept
        {
            intent.state           = state;
            intent.reason          = reason;
            intent.logicalActive   = false;
            intent.outputLevelHigh = false;
            intent.requiredBaseUa  = 0;
            intent.admittedBaseUa  = 0;
            intent.admittedLoadUa  = 0;
            intent.expiresAt       = TimePoint ();
            intent.producerStatus  = status;
        }

        void rejectComputed (LowSideDriveIntent& intent,
                             LowSideDriveReason reason, Status status) noexcept
        {
            intent.state           = LowSideDriveState::Rejected;
            intent.reason          = reason;
            intent.logicalActive   = false;
            intent.outputLevelHigh = false;
            intent.expiresAt       = TimePoint ();
            intent.producerStatus  = status;
        }

        bool forwardOrEqual (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        uint32_t earlierAt (uint32_t left, uint32_t right, uint32_t origin) noexcept
        {
            return left - origin < right - origin ? left : right;
        }

        uint32_t laterAt (uint32_t left, uint32_t right, uint32_t origin) noexcept
        {
            return left - origin > right - origin ? left : right;
        }
    } // namespace

    Status
    validateLowSideDriverDescriptor (const LowSideDriverDescriptor& descriptor) noexcept
    {
        const LowSideCurrentBudget& budget = descriptor.budget;
        if (!validDescriptorEnums (descriptor) || descriptor.schemaRevision == 0 ||
            descriptor.specimenFamilyReference == 0 ||
            descriptor.specimenReference == 0 || descriptor.specimenRevision == 0 ||
            descriptor.electricalEvidenceRevision == 0 ||
            descriptor.sourcePacketDigest == 0 || descriptor.configurationId == 0 ||
            descriptor.configurationRevision == 0)
        {
            return StatusCode::InvalidConfiguration;
        }
        if (budget.supplyLimitUa == 0 ||
            budget.reservedSupplyUa > budget.supplyLimitUa ||
            budget.fixtureLoadCeilingUa == 0 || budget.deviceContinuousUa == 0 ||
            budget.gpioSourceCeilingUa == 0 || budget.forcedGainNumerator == 0 ||
            budget.forcedGainDenominator == 0 || budget.baseResistanceOhms == 0 ||
            budget.baseResistanceTolerancePermille > 1000 ||
            budget.logicHighMinimumMv == 0 || budget.logicHighMaximumMv == 0 ||
            budget.logicHighMinimumMv > budget.logicHighMaximumMv ||
            budget.baseEmitterMaximumMv >= budget.logicHighMinimumMv ||
            budget.collectorEmitterOperatingMaximumMv == 0 ||
            !validDuration (budget.maximumActiveDuration) ||
            !validDuration (budget.dutyWindow) || budget.maximumDutyPermille == 0 ||
            budget.maximumDutyPermille > 1000 ||
            !descriptorFlybackCanonical (descriptor) ||
            (descriptor.flybackDeclaration == LowSideFlybackDeclaration::Present &&
             !flybackComplete (descriptor)))
        {
            return StatusCode::InvalidConfiguration;
        }

        const uint64_t lowerResistance =
            static_cast<uint64_t> (budget.baseResistanceOhms) *
            (1000U - budget.baseResistanceTolerancePermille) / 1000U;
        if (lowerResistance == 0)
        {
            return StatusCode::InvalidConfiguration;
        }
        const uint64_t maximumCurrentNumerator =
            static_cast<uint64_t> (budget.logicHighMaximumMv) * 1000U;
        const uint64_t maximumPossibleBaseUa =
            maximumCurrentNumerator / lowerResistance +
            (maximumCurrentNumerator % lowerResistance != 0 ? 1U : 0U);
        if (maximumPossibleBaseUa > budget.gpioSourceCeilingUa)
        {
            return StatusCode::InvalidConfiguration;
        }
        return Status ();
    }

    uint32_t lowSideDriverDescriptorIdentityDigest (
        const LowSideDriverDescriptor& descriptor) noexcept
    {
        static constexpr uint8_t tag[] = {'A', 'D', 'K', '7', '9', 'D', 'S', 'C'};
        uint32_t                 crc   = UINT32_C (0xFFFFFFFF);
        for (uint8_t value : tag)
        {
            digestU8 (crc, value);
        }

        digestU16 (crc, descriptor.schemaRevision);
        digestU32 (crc, descriptor.specimenFamilyReference);
        digestU32 (crc, descriptor.specimenReference);
        digestU16 (crc, descriptor.specimenRevision);
        digestU16 (crc, descriptor.electricalEvidenceRevision);
        digestU32 (crc, descriptor.sourcePacketDigest);
        digestU32 (crc, descriptor.configurationId);
        digestU16 (crc, descriptor.configurationRevision);
        digestU8  (crc, static_cast<uint8_t> (descriptor.loadEnergy));
        digestU8  (crc,
                  static_cast<uint8_t> (descriptor.flybackRequirement));
        digestU8  (crc,
                  static_cast<uint8_t> (descriptor.flybackDeclaration));
        digestU8  (crc, descriptor.sourceEligible ? 1U : 0U);
        digestU32 (crc, descriptor.flybackDiodeIdentity);
        digestU16 (crc, descriptor.flybackDiodeRevision);
        digestU8  (crc, descriptor.flybackOrientationCode);
        digestU8  (crc, descriptor.flybackReturnCode);
        digestU32 (crc, descriptor.flybackRepetitiveReverseMv);
        digestU32 (crc, descriptor.flybackForwardCurrentUa);
        digestU32 (crc, descriptor.budget.supplyLimitUa);
        digestU32 (crc, descriptor.budget.reservedSupplyUa);
        digestU32 (crc, descriptor.budget.fixtureLoadCeilingUa);
        digestU32 (crc, descriptor.budget.deviceContinuousUa);
        digestU32 (crc, descriptor.budget.gpioSourceCeilingUa);
        digestU32 (crc, descriptor.budget.forcedGainNumerator);
        digestU32 (crc, descriptor.budget.forcedGainDenominator);
        digestU32 (crc, descriptor.budget.baseResistanceOhms);
        digestU16 (crc, descriptor.budget.baseResistanceTolerancePermille);
        digestU16 (crc, descriptor.budget.logicHighMinimumMv);
        digestU16 (crc, descriptor.budget.logicHighMaximumMv);
        digestU16 (crc, descriptor.budget.baseEmitterMaximumMv);
        digestU16 (crc, descriptor.budget.collectorEmitterOperatingMaximumMv);
        digestU32 (crc, descriptor.budget.maximumActiveDuration.milliseconds ());
        digestU32 (crc, descriptor.budget.dutyWindow.milliseconds ());
        digestU16 (crc, descriptor.budget.maximumDutyPermille);

        return crc ^ UINT32_C (0xFFFFFFFF);
    }

    BoundedLowSideDriverPolicy::BoundedLowSideDriverPolicy (
        const LowSideDriverDescriptor& descriptor) noexcept
        : descriptor_ (descriptor), intent_ (), reservations_ (),
          lifecycleGeneration_ (0), lastSequence_ (0), lastObservedAt_ (),

          lastRequestDigest_ (0), sourceId_ (0), reservationCount_ (0),

          initialized_ (false), hasRequest_ (false), lastWasControl_ (false),

          lastWasUpdate_ (false), hasChronology_ (false)
    {
    }

    Status BoundedLowSideDriverPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }
        const Status status = validateLowSideDriverDescriptor (descriptor_);

        if (!status.ok () || lifecycleGeneration_ == UINT32_MAX)
        {
            return !status.ok () ? status : Status (StatusCode::CapacityExceeded);
        }
        ++lifecycleGeneration_;
        sourceId_         = 0;
        hasRequest_       = false;
        lastWasControl_   = false;
        const uint32_t descriptorDigest =
            lowSideDriverDescriptorIdentityDigest (descriptor_);
        intent_ = baseIntent (descriptor_, descriptorDigest, lifecycleGeneration_, 0,
                              0);
        initialized_      = true;
        return Status ();
    }

    void BoundedLowSideDriverPolicy::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }
        forceOff (intent_, LowSideDriveState::Shutdown, LowSideDriveReason::None,
                  Status ());
        sourceId_         = 0;
        hasRequest_       = false;
        lastWasControl_   = false;
        initialized_      = false;
    }

    bool BoundedLowSideDriverPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    Status BoundedLowSideDriverPolicy::beginSession (uint32_t sessionId,
                                                     uint32_t runId) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (sessionId == 0 || runId == 0 || intent_.state != LowSideDriveState::Off ||
            (intent_.sessionId != 0 &&
             (sessionId == intent_.sessionId || runId == intent_.runId)))
        {
            return StatusCode::InvalidArgument;
        }
        sourceId_   = 0;
        hasRequest_ = false;
        lastWasControl_ = false;
        intent_ = baseIntent (descriptor_, intent_.driverDescriptorIdentityDigest,
                              lifecycleGeneration_, sessionId, runId);

        return Status ();
    }

    Status BoundedLowSideDriverPolicy::apply (const LowSideDriveRequest& request,
                                              LowSideDriveIntent& output) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validStatus (request.producerStatus) || request.sessionId == 0 ||
            request.runId == 0 || request.stepId == 0 || request.requestId == 0 ||
            request.sourceId == 0 || request.sourceConfigurationRevision == 0 ||
            request.sequence == 0 || request.lifecycleGeneration == 0 ||
            (request.logicalActive
                 ? (!validDuration (request.requestedActiveDuration) ||
                    request.requestedActiveDuration >
                        descriptor_.budget.maximumActiveDuration ||
                    request.requestedLoadUa == 0)
                 : (request.requestedLoadUa != 0 ||
                    request.requestedActiveDuration.milliseconds () != 0)))
        {
            return StatusCode::InvalidArgument;
        }
        if (request.lifecycleGeneration != lifecycleGeneration_ ||
            request.sessionId != intent_.sessionId || request.runId != intent_.runId ||
            request.sourceConfigurationRevision != descriptor_.configurationRevision ||
            (sourceId_ != 0 && request.sourceId != sourceId_))
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t digest = requestDigest (request);
        if (hasRequest_ && request.sequence == lastSequence_)
        {
            if (lastWasControl_ || digest != lastRequestDigest_)
            {
                return StatusCode::InvalidArgument;
            }
            output = intent_;
            return Status ();
        }

        LowSideDriveIntent candidate =
            baseIntent (descriptor_, intent_.driverDescriptorIdentityDigest,
                        lifecycleGeneration_, intent_.sessionId, intent_.runId);
        candidate.stepId             = request.stepId;
        candidate.requestId          = request.requestId;

        if (hasRequest_ && request.sequence != lastSequence_ + 1U)
        {
            forceOff (candidate, LowSideDriveState::Rejected,
                      LowSideDriveReason::SequenceDiscontinuity,
                      request.producerStatus);
            sourceId_          = request.sourceId;
            lastSequence_      = request.sequence;
            if (!hasChronology_ ||
                forwardOrEqual (request.observedAt, lastObservedAt_))
            {
                lastObservedAt_ = request.observedAt;
            }
            lastRequestDigest_ = digest;
            hasRequest_        = true;
            lastWasControl_    = false;
            lastWasUpdate_     = false;
            hasChronology_     = true;
            intent_            = candidate;
            output             = candidate;
            return Status ();
        }
        if (hasChronology_ &&
            !forwardOrEqual (request.observedAt, lastObservedAt_))
        {
            if (lastWasUpdate_)
            {
                return StatusCode::InvalidArgument;
            }
            forceOff (candidate, LowSideDriveState::Rejected,
                      LowSideDriveReason::TimestampDiscontinuity,
                      request.producerStatus);
            sourceId_          = request.sourceId;
            lastSequence_      = request.sequence;
            lastRequestDigest_ = digest;
            hasRequest_        = true;
            lastWasControl_    = false;
            lastWasUpdate_     = false;
            hasChronology_     = true;
            intent_            = candidate;
            output             = candidate;
            return Status ();
        }
        if (intent_.logicalActive && request.logicalActive)
        {
            return StatusCode::InvalidArgument;
        }

        if (!request.logicalActive)
        {
            if (intent_.logicalActive && reservationCount_ != 0)
            {
                Reservation& active = reservations_[reservationCount_ - 1U];
                const uint32_t observedAt = request.observedAt.milliseconds ();

                const uint32_t admittedEnd = active.end.milliseconds ();

                const uint32_t admittedStart = active.start.milliseconds ();
                const uint32_t terminalAt =
                    earlierAt (observedAt, admittedEnd, admittedStart);
                active.end = TimePoint (terminalAt);
            }
            forceOff (candidate,
                      request.producerStatus.ok () ? LowSideDriveState::Off
                                                   : LowSideDriveState::Fault,
                      request.producerStatus.ok () ? LowSideDriveReason::None
                                                   : LowSideDriveReason::ProducerFault,
                      request.producerStatus);
        }
        else if (!request.producerStatus.ok ())
        {
            forceOff (candidate, LowSideDriveState::Fault,
                      LowSideDriveReason::ProducerFault, request.producerStatus);
        }
        else if (!descriptor_.sourceEligible)
        {
            forceOff (candidate, LowSideDriveState::Rejected,
                      LowSideDriveReason::SourceIneligible, request.producerStatus);
        }
        else if (!flybackComplete (descriptor_))
        {
            forceOff (candidate, LowSideDriveState::Rejected,
                      LowSideDriveReason::FlybackMissing, request.producerStatus);
        }
        else
        {
            const LowSideCurrentBudget& budget = descriptor_.budget;
            const uint32_t              availableSupply =
                budget.supplyLimitUa - budget.reservedSupplyUa;
            const uint64_t requestedBaseNumerator =
                static_cast<uint64_t> (request.requestedLoadUa) *
                budget.forcedGainDenominator;
            const uint64_t requiredBase =
                requestedBaseNumerator / budget.forcedGainNumerator +
                (requestedBaseNumerator % budget.forcedGainNumerator != 0 ? 1U : 0U);
            const uint32_t resistorDrop =
                budget.logicHighMinimumMv - budget.baseEmitterMaximumMv;
            const uint64_t lowerResistance =
                static_cast<uint64_t> (budget.baseResistanceOhms) *
                (1000U - budget.baseResistanceTolerancePermille) / 1000U;
            const uint64_t upperNumerator =
                static_cast<uint64_t> (budget.baseResistanceOhms) *
                (1000U + budget.baseResistanceTolerancePermille);
            const uint64_t upperResistance =
                upperNumerator / 1000U + (upperNumerator % 1000U != 0 ? 1U : 0U);
            if (lowerResistance == 0 || upperResistance == 0)
            {
                return StatusCode::InvalidArgument;
            }
            const uint64_t maximumCurrentNumerator =
                static_cast<uint64_t> (budget.logicHighMaximumMv) * 1000U;
            const uint64_t maximumResistorCurrent =
                maximumCurrentNumerator / lowerResistance +
                (maximumCurrentNumerator % lowerResistance != 0 ? 1U : 0U);
            const uint64_t resistorLimited =
                static_cast<uint64_t> (resistorDrop) * 1000U / upperResistance;
            const uint32_t admittedBase =
                static_cast<uint32_t> (resistorLimited < budget.gpioSourceCeilingUa
                                           ? resistorLimited
                                           : budget.gpioSourceCeilingUa);
            const uint64_t gainLimited  = static_cast<uint64_t> (admittedBase) *
                                          budget.forcedGainNumerator /
                                          budget.forcedGainDenominator;
            uint64_t       admittedLoad = request.requestedLoadUa;
            if (admittedLoad > availableSupply)
            {
                admittedLoad = availableSupply;
            }
            if (admittedLoad > budget.deviceContinuousUa)
            {
                admittedLoad = budget.deviceContinuousUa;
            }
            if (admittedLoad > budget.fixtureLoadCeilingUa)
            {
                admittedLoad = budget.fixtureLoadCeilingUa;
            }
            if (admittedLoad > gainLimited)
            {
                admittedLoad = gainLimited;
            }

            if (requiredBase > UINT32_MAX || maximumResistorCurrent > UINT32_MAX ||
                resistorLimited > UINT32_MAX || gainLimited > UINT32_MAX)
            {
                return StatusCode::InvalidArgument;
            }
            candidate.requiredBaseUa = static_cast<uint32_t> (requiredBase);
            candidate.admittedBaseUa = admittedBase;
            candidate.admittedLoadUa = static_cast<uint32_t> (admittedLoad);

            const uint32_t now         = request.observedAt.milliseconds ();

            const uint32_t windowStart = now - budget.dutyWindow.milliseconds ();
            uint8_t        write       = 0;
            for (uint8_t index = 0; index < reservationCount_; ++index)
            {
                const uint32_t end = reservations_[index].end.milliseconds ();
                if (end - windowStart != 0 && end - windowStart < halfRange)
                {
                    reservations_[write++] = reservations_[index];
                }
            }
            reservationCount_ = write;

            const uint32_t candidateEnd =
                now + request.requestedActiveDuration.milliseconds ();
            uint64_t reservedTicks = request.requestedActiveDuration.milliseconds ();
            for (uint8_t index = 0; index < reservationCount_; ++index)
            {
                const uint32_t start =
                    laterAt (reservations_[index].start.milliseconds (), windowStart,
                             windowStart);
                const uint32_t end =
                    earlierAt (reservations_[index].end.milliseconds (), candidateEnd,
                               windowStart);
                if (end - start < halfRange)
                {
                    reservedTicks += end - start;
                }
            }
            const uint64_t allowed =
                static_cast<uint64_t> (budget.dutyWindow.milliseconds ()) *
                budget.maximumDutyPermille;
            if (reservedTicks * 1000U > allowed)
            {
                rejectComputed (candidate, LowSideDriveReason::BudgetExceeded,
                                request.producerStatus);
            }
            else if (maximumResistorCurrent > budget.gpioSourceCeilingUa ||
                     requiredBase > admittedBase ||
                     admittedLoad < request.requestedLoadUa)
            {
                rejectComputed (
                    candidate,
                    maximumResistorCurrent > budget.gpioSourceCeilingUa ||
                            requiredBase > admittedBase
                        ? LowSideDriveReason::BaseBudgetInsufficient
                        : LowSideDriveReason::BudgetExceeded,
                    request.producerStatus);
            }
            else if (reservationCount_ == 8)
            {
                rejectComputed (candidate, LowSideDriveReason::CapacityExceeded,
                                request.producerStatus);
            }
            else
            {
                reservations_[reservationCount_++] = {request.observedAt,
                                                      TimePoint (candidateEnd)};
                candidate.state                    = LowSideDriveState::Requested;
                candidate.reason                   = LowSideDriveReason::None;
                candidate.logicalActive            = true;
                candidate.outputLevelHigh          = true;
                candidate.expiresAt                = TimePoint (candidateEnd);
                candidate.producerStatus           = request.producerStatus;
            }
        }

        sourceId_          = request.sourceId;
        lastSequence_      = request.sequence;
        lastObservedAt_    = request.observedAt;
        lastRequestDigest_ = digest;
        hasRequest_        = true;
        lastWasControl_    = false;
        lastWasUpdate_     = false;
        hasChronology_     = true;
        intent_            = candidate;
        output             = candidate;
        return Status ();
    }

    Status BoundedLowSideDriverPolicy::update (TimePoint           now,
                                               LowSideDriveIntent& output) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (hasChronology_ && !forwardOrEqual (now, lastObservedAt_))
        {
            return StatusCode::InvalidArgument;
        }

        LowSideDriveIntent candidate = intent_;
        const uint32_t     windowStart =
            now.milliseconds () - descriptor_.budget.dutyWindow.milliseconds ();
        uint8_t write = 0;
        for (uint8_t index = 0; index < reservationCount_; ++index)
        {
            const uint32_t end = reservations_[index].end.milliseconds ();
            if (end - windowStart != 0 && end - windowStart < halfRange)
            {
                reservations_[write++] = reservations_[index];
            }
        }
        reservationCount_ = write;
        if (candidate.logicalActive &&
            now.elapsedSince (candidate.expiresAt).milliseconds () < halfRange)
        {
            forceOff (candidate, LowSideDriveState::Off,
                      LowSideDriveReason::Expired, candidate.producerStatus);
        }
        lastObservedAt_ = now;
        lastWasUpdate_  = true;
        hasChronology_   = true;
        intent_         = candidate;
        output          = candidate;
        return Status ();
    }

    Status BoundedLowSideDriverPolicy::cancel (const LowSideControl& control,
                                               LowSideDriveIntent&   output) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validStatus (control.producerStatus) ||
            control.lifecycleGeneration != lifecycleGeneration_ ||
            control.sessionId != intent_.sessionId || control.runId != intent_.runId ||
            control.stepId == 0 || control.controlId == 0 || control.sourceId == 0 ||
            control.sourceId != sourceId_ ||
            control.sourceConfigurationRevision != descriptor_.configurationRevision ||
            control.sequence == 0 ||
            (hasRequest_ && !forwardOrEqual (control.observedAt, lastObservedAt_)))
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t digest = controlDigest (control);
        if (hasRequest_ && control.sequence == lastSequence_)
        {
            if (!lastWasControl_ || digest != lastRequestDigest_)
            {
                return StatusCode::InvalidArgument;
            }
            output = intent_;
            return Status ();
        }
        if (hasRequest_ && control.sequence != lastSequence_ + 1U)
        {
            return StatusCode::InvalidArgument;
        }

        LowSideDriveIntent candidate =
            baseIntent (descriptor_, intent_.driverDescriptorIdentityDigest,
                        lifecycleGeneration_, intent_.sessionId, intent_.runId);
        candidate.stepId             = control.stepId;
        candidate.requestId          = control.controlId;
        if (intent_.logicalActive && reservationCount_ != 0)
        {
            Reservation& active = reservations_[reservationCount_ - 1U];
            const uint32_t observedAt = control.observedAt.milliseconds ();

            const uint32_t admittedEnd = active.end.milliseconds ();

            const uint32_t admittedStart = active.start.milliseconds ();
            const uint32_t terminalAt =
                earlierAt (observedAt, admittedEnd, admittedStart);
            active.end = TimePoint (terminalAt);
        }
        const bool safe = control.offConfirmed && control.producerStatus.ok ();

        forceOff (candidate,
                  safe ? LowSideDriveState::Cancelled : LowSideDriveState::Fault,
                  LowSideDriveReason::Cancelled, control.producerStatus);
        lastSequence_   = control.sequence;
        lastObservedAt_ = control.observedAt;
        lastRequestDigest_ = digest;
        hasRequest_        = true;
        lastWasControl_    = true;
        lastWasUpdate_     = false;
        hasChronology_     = true;
        intent_         = candidate;
        output          = candidate;
        return Status ();
    }

    Status BoundedLowSideDriverPolicy::reset () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (lifecycleGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        ++lifecycleGeneration_;
        sourceId_         = 0;
        hasRequest_       = false;
        lastWasControl_   = false;
        intent_ = baseIntent (descriptor_, intent_.driverDescriptorIdentityDigest,
                              lifecycleGeneration_, 0, 0);

        return Status ();
    }

    LowSideDriveIntent BoundedLowSideDriverPolicy::snapshot () const noexcept
    {
        return intent_;
    }

#if defined(ADK_TESTING)
    void BoundedLowSideDriverPolicy::seedLifecycleGenerationForTest (
        uint32_t generation) noexcept
    {
        lifecycleGeneration_ = generation;
    }
#endif
} // namespace adk
