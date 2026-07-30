#include "small_indicator_semantics_policy.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool validStatus (Status status) noexcept
        {
            return static_cast<uint8_t> (status.error ()) <=
                   static_cast<uint8_t> (StatusCode::HardwareFailure);
        }

        bool validDuration (Duration duration) noexcept
        {
            return duration.milliseconds () < halfRange;
        }

        bool validObservationState (SmallIndicatorObservationState state) noexcept
        {
            return state == SmallIndicatorObservationState::NotObserved ||
                   state == SmallIndicatorObservationState::Inactive ||
                   state == SmallIndicatorObservationState::Active ||
                   state == SmallIndicatorObservationState::Alternating ||
                   state == SmallIndicatorObservationState::Fault;
        }

        bool validDriveShape (const LowSideDriveIntent& drive) noexcept
        {
            const bool off = !drive.logicalActive && !drive.outputLevelHigh;
            switch (drive.state)
            {
                case LowSideDriveState::Off:
                    return off && drive.producerStatus.ok () &&
                           (drive.reason == LowSideDriveReason::None ||
                            drive.reason == LowSideDriveReason::Expired);
                case LowSideDriveState::Requested:
                    return drive.reason == LowSideDriveReason::None &&
                           drive.logicalActive && drive.outputLevelHigh &&
                           drive.producerStatus.ok ();
                case LowSideDriveState::Rejected:
                    return off &&
                           (drive.reason == LowSideDriveReason::SourceIneligible ||
                            drive.reason == LowSideDriveReason::BudgetExceeded ||
                            drive.reason ==
                                LowSideDriveReason::BaseBudgetInsufficient ||
                            drive.reason == LowSideDriveReason::FlybackMissing ||
                            drive.reason == LowSideDriveReason::SequenceDiscontinuity ||
                            drive.reason ==
                                LowSideDriveReason::TimestampDiscontinuity ||
                            drive.reason == LowSideDriveReason::CapacityExceeded) &&
                           (drive.producerStatus.ok () ||
                            drive.reason == LowSideDriveReason::SequenceDiscontinuity ||
                            drive.reason == LowSideDriveReason::TimestampDiscontinuity);
                case LowSideDriveState::Cancelled:
                    return off && drive.reason == LowSideDriveReason::Cancelled &&
                           drive.producerStatus.ok ();
                case LowSideDriveState::Fault:
                    return off && !drive.producerStatus.ok () &&
                           (drive.reason == LowSideDriveReason::ProducerFault ||
                            drive.reason == LowSideDriveReason::Cancelled);
                case LowSideDriveState::Shutdown:
                    return off && drive.reason == LowSideDriveReason::None &&
                           drive.producerStatus.ok ();
            }
            return false;
        }

        bool forwardOrEqual (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool descriptorRow (const SmallIndicatorDescriptor& descriptor,
                            SmallIndicatorKind kind, SmallIndicatorAutonomy autonomy,
                            SmallIndicatorSafeState safeState, uint8_t mask,
                            bool resistor, bool driver) noexcept
        {
            return descriptor.kind == kind && descriptor.autonomy == autonomy &&
                   descriptor.safeState == safeState && descriptor.activeHigh &&
                   descriptor.declaredChannelMask == mask &&
                   descriptor.populatedResistorDeclared == resistor &&
                   descriptor.populatedDriverDeclared == driver;
        }

        bool validDescriptorRow (const SmallIndicatorDescriptor& descriptor) noexcept
        {
            return descriptorRow (descriptor, SmallIndicatorKind::ActiveBuzzer,
                                  SmallIndicatorAutonomy::FollowsDrive,
                                  SmallIndicatorSafeState::DriveInactive,
                                  SmallIndicatorChannels::Sound, false, true) ||
                   descriptorRow (descriptor, SmallIndicatorKind::ActiveBuzzer,
                                  SmallIndicatorAutonomy::AutonomousWhileEnabled,
                                  SmallIndicatorSafeState::UnpoweredRequired,
                                  SmallIndicatorChannels::Sound, false, true) ||
                   descriptorRow (descriptor, SmallIndicatorKind::TrafficLight,
                                  SmallIndicatorAutonomy::FollowsDrive,
                                  SmallIndicatorSafeState::DriveInactive,
                                  SmallIndicatorChannels::Red |
                                      SmallIndicatorChannels::Amber |
                                      SmallIndicatorChannels::Green,
                                  true, false) ||
                   descriptorRow (descriptor, SmallIndicatorKind::DualColorLed,
                                  SmallIndicatorAutonomy::FollowsDrive,
                                  SmallIndicatorSafeState::DriveInactive,
                                  SmallIndicatorChannels::Red |
                                      SmallIndicatorChannels::Green,
                                  true, false) ||
                   descriptorRow (descriptor, SmallIndicatorKind::AutoFlashLed,
                                  SmallIndicatorAutonomy::AutonomousWhileEnabled,
                                  SmallIndicatorSafeState::UnpoweredRequired,
                                  SmallIndicatorChannels::Red |
                                      SmallIndicatorChannels::Green |
                                      SmallIndicatorChannels::Blue,
                                  true, false) ||
                   descriptorRow (descriptor, SmallIndicatorKind::VoltageIndicator,
                                  SmallIndicatorAutonomy::ObservationOnly,
                                  SmallIndicatorSafeState::HighImpedanceRequired,
                                  SmallIndicatorChannels::Voltage, true, false);
        }

        SmallIndicatorSemanticResult
        baseResult (uint32_t generation, uint32_t sessionId, uint32_t runId,
                    SmallIndicatorDisposition disposition) noexcept
        {
            SmallIndicatorSemanticResult result;
            result.lifecycleGeneration = generation;
            result.sessionId           = sessionId;
            result.runId               = runId;
            result.stepId              = 0;
            result.requestId           = 0;
            result.observationId       = 0;
            result.disposition         = disposition;
            result.reason              = SmallIndicatorReason::None;
            result.observationState    = SmallIndicatorObservationState::NotObserved;
            result.semanticActive      = false;
            result.semanticActiveMask  = 0;
            result.safeStateSatisfied  = false;
            result.autonomousBehaviorObserved = false;
            result.producerStatus             = Status ();
            return result;
        }

        bool equalStatus (Status left, Status right) noexcept
        {
            return left.error () == right.error ();
        }

        bool equalDrive (const LowSideDriveIntent& left,
                         const LowSideDriveIntent& right) noexcept
        {
            return left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.driverDescriptorIdentityDigest ==
                       right.driverDescriptorIdentityDigest &&
                   left.specimenReference == right.specimenReference &&
                   left.specimenRevision == right.specimenRevision &&
                   left.electricalEvidenceRevision ==
                       right.electricalEvidenceRevision &&
                   left.policyConfigurationId == right.policyConfigurationId &&
                   left.policyConfigurationRevision ==
                       right.policyConfigurationRevision &&
                   left.sessionId == right.sessionId && left.runId == right.runId &&
                   left.stepId == right.stepId && left.requestId == right.requestId &&
                   left.state == right.state && left.reason == right.reason &&
                   left.logicalActive == right.logicalActive &&
                   left.outputLevelHigh == right.outputLevelHigh &&
                   left.requiredBaseUa == right.requiredBaseUa &&
                   left.admittedBaseUa == right.admittedBaseUa &&
                   left.admittedLoadUa == right.admittedLoadUa &&
                   left.expiresAt == right.expiresAt &&
                   equalStatus (left.producerStatus, right.producerStatus);
        }

        bool equalRequest (const SmallIndicatorSemanticRequest& left,
                           const SmallIndicatorSemanticRequest& right) noexcept
        {
            return left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.sessionId == right.sessionId && left.runId == right.runId &&
                   left.stepId == right.stepId && left.requestId == right.requestId &&
                   left.sourceId == right.sourceId &&
                   left.sourceConfigurationRevision ==
                       right.sourceConfigurationRevision &&
                   left.policySequence == right.policySequence &&
                   left.requestSequence == right.requestSequence &&
                   left.requestedAt == right.requestedAt &&
                   left.selectedActiveMask == right.selectedActiveMask &&
                   equalStatus (left.producerStatus, right.producerStatus);
        }

        bool equalObservation (const SmallIndicatorObservation& left,
                               const SmallIndicatorObservation& right) noexcept
        {
            return left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.sessionId == right.sessionId && left.runId == right.runId &&
                   left.stepId == right.stepId && left.requestId == right.requestId &&
                   left.observationId == right.observationId &&
                   left.sourceId == right.sourceId &&
                   left.sourceConfigurationRevision ==
                       right.sourceConfigurationRevision &&
                   left.observationSequence == right.observationSequence &&
                   left.observedAt == right.observedAt && left.state == right.state &&
                   left.observedActiveMask == right.observedActiveMask &&
                   left.copiedLevelHigh == right.copiedLevelHigh &&
                   left.autonomousTransitionObserved ==
                       right.autonomousTransitionObserved &&
                   left.safeStateObserved == right.safeStateObserved &&
                   equalStatus (left.producerStatus, right.producerStatus);
        }

        bool equalControl (const SmallIndicatorControl& left,
                           const SmallIndicatorControl& right) noexcept
        {
            return left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.sessionId == right.sessionId && left.runId == right.runId &&
                   left.stepId == right.stepId && left.controlId == right.controlId &&
                   left.sourceId == right.sourceId &&
                   left.sourceConfigurationRevision ==
                       right.sourceConfigurationRevision &&
                   left.policySequence == right.policySequence &&
                   left.observedAt == right.observedAt &&
                   left.offConfirmed == right.offConfirmed &&
                   equalStatus (left.producerStatus, right.producerStatus);
        }

        bool equalApply (const LowSideDriveIntent&            leftDrive,
                         const SmallIndicatorSemanticRequest& leftRequest,
                         const SmallIndicatorObservation&     leftObservation,
                         TimePoint leftNow, const LowSideDriveIntent& rightDrive,
                         const SmallIndicatorSemanticRequest& rightRequest,
                         const SmallIndicatorObservation&     rightObservation,
                         TimePoint                            rightNow) noexcept
        {
            const bool sameDrive =
                equalDrive (leftDrive, rightDrive);
            const bool sameRequest =
                equalRequest (leftRequest, rightRequest);
            const bool sameObservation =
                equalObservation (leftObservation, rightObservation);
            return sameDrive && sameRequest && sameObservation && leftNow == rightNow;
        }

        bool
        validObservationShape (const SmallIndicatorDescriptor&  descriptor,
                               const SmallIndicatorObservation& observation) noexcept
        {
            const uint8_t masks = observation.observedActiveMask;
            if ((masks & static_cast<uint8_t> (~descriptor.declaredChannelMask)) != 0)
            {
                return false;
            }
            if ((observation.state == SmallIndicatorObservationState::NotObserved ||
                 observation.state == SmallIndicatorObservationState::Fault) &&
                masks != 0)
            {
                return false;
            }
            if ((observation.state == SmallIndicatorObservationState::Fault) ==
                observation.producerStatus.ok ())
            {
                return false;
            }
            if (observation.state == SmallIndicatorObservationState::Inactive &&
                observation.observedActiveMask != 0)
            {
                return false;
            }
            if ((observation.state == SmallIndicatorObservationState::Active ||
                 observation.state == SmallIndicatorObservationState::Alternating) &&
                observation.observedActiveMask == 0)
            {
                return false;
            }
            return true;
        }

        void reject (SmallIndicatorSemanticResult& result,
                     SmallIndicatorDisposition     disposition,
                     SmallIndicatorReason          reason) noexcept
        {
            result.disposition        = disposition;
            result.reason             = reason;
            result.semanticActive     = false;
            result.semanticActiveMask = 0;
        }
    } // namespace

    Status validateSmallIndicatorDescriptor (
        const SmallIndicatorDescriptor& descriptor) noexcept
    {
        if (descriptor.schemaRevision != 1 || descriptor.specimenFamilyReference == 0 ||
            descriptor.specimenReference == 0 || descriptor.specimenRevision == 0 ||
            descriptor.electricalEvidenceRevision == 0 ||
            descriptor.sourcePacketDigest == 0 || descriptor.configurationId == 0 ||
            descriptor.configurationRevision == 0 ||
            descriptor.driverSpecimenReference == 0 ||
            descriptor.driverSpecimenRevision == 0 ||
            descriptor.driverElectricalEvidenceRevision == 0 ||
            descriptor.driverPolicyConfigurationId == 0 ||
            descriptor.driverPolicyConfigurationRevision == 0 ||
            !validDuration (descriptor.warmup) ||
            !validDuration (descriptor.settling) ||
            !validDuration (descriptor.maximumObservationAge))
        {
            return StatusCode::InvalidConfiguration;
        }
        if (!validDescriptorRow (descriptor))
        {
            return StatusCode::InvalidConfiguration;
        }
        return Status ();
    }

    SmallIndicatorSemanticsPolicy::SmallIndicatorSemanticsPolicy (
        const SmallIndicatorDescriptor& descriptor) noexcept
        : descriptor_ (descriptor), result_ (), cachedDrive_ (), cachedRequest_ (),
          cachedObservation_ (), cachedControl_ (), cachedNow_ (), startedAt_ (),

          lastObservedAt_ (), lifecycleGeneration_ (0), lastSequence_ (0),

          lastRequestSequence_ (0), lastObservationSequence_ (0), requestSourceId_ (0),

          observationSourceId_ (0), requestSourceConfigurationRevision_ (0),

          observationSourceConfigurationRevision_ (0), initialized_ (false),

          hasEvidence_ (false), lastWasControl_ (false), hasChronology_ (false),

          terminal_ (false)
    {
    }

    Status SmallIndicatorSemanticsPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }
        const Status status = validateSmallIndicatorDescriptor (descriptor_);

        if (!status.ok () || lifecycleGeneration_ == UINT32_MAX)
        {
            return !status.ok () ? status : Status (StatusCode::CapacityExceeded);
        }
        ++lifecycleGeneration_;
        requestSourceId_ = observationSourceId_ = 0;
        requestSourceConfigurationRevision_ = observationSourceConfigurationRevision_ =
            0;
        lastSequence_ = lastRequestSequence_ = lastObservationSequence_ = 0;
        hasEvidence_       = false;
        lastWasControl_    = false;
        hasChronology_     = false;
        terminal_          = false;
        result_            = baseResult (lifecycleGeneration_, 0, 0,
                                         descriptor_.sourceEligible
                                             ? SmallIndicatorDisposition::Idle
                                             : SmallIndicatorDisposition::Rejected);
        if (!descriptor_.sourceEligible)
        {
            result_.reason = SmallIndicatorReason::SourceIneligible;
        }
        initialized_ = true;
        return Status ();
    }

    void SmallIndicatorSemanticsPolicy::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }
        result_.disposition        = SmallIndicatorDisposition::Shutdown;
        result_.reason             = SmallIndicatorReason::None;
        result_.semanticActive     = false;
        result_.semanticActiveMask = 0;
        requestSourceId_ = observationSourceId_ = 0;
        requestSourceConfigurationRevision_ = observationSourceConfigurationRevision_ =
            0;
        lastSequence_ = lastRequestSequence_ = lastObservationSequence_ = 0;
        hasEvidence_       = false;
        lastWasControl_    = false;
        hasChronology_     = false;
        terminal_          = true;
        initialized_       = false;
    }

    bool SmallIndicatorSemanticsPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    Status SmallIndicatorSemanticsPolicy::beginSession (uint32_t  sessionId,
                                                        uint32_t  runId,
                                                        TimePoint startedAt) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (sessionId == 0 || runId == 0)
        {
            return StatusCode::InvalidArgument;
        }
        if (result_.sessionId != 0)
        {
            return StatusCode::ResourceBusy;
        }
        requestSourceId_ = observationSourceId_ = 0;
        requestSourceConfigurationRevision_ = observationSourceConfigurationRevision_ =
            0;
        lastSequence_ = lastRequestSequence_ = lastObservationSequence_ = 0;
        hasEvidence_       = false;
        lastWasControl_    = false;
        hasChronology_     = true;
        terminal_          = false;
        startedAt_         = startedAt;
        lastObservedAt_    = startedAt;
        result_            = baseResult (lifecycleGeneration_, sessionId, runId,
                                         descriptor_.sourceEligible
                                             ? SmallIndicatorDisposition::Eligible
                                             : SmallIndicatorDisposition::Rejected);
        if (!descriptor_.sourceEligible)
        {
            result_.reason = SmallIndicatorReason::SourceIneligible;
        }
        return Status ();
    }

    Status SmallIndicatorSemanticsPolicy::apply (
        const LowSideDriveIntent& drive, const SmallIndicatorSemanticRequest& request,
        const SmallIndicatorObservation& observation, TimePoint now,
        SmallIndicatorSemanticResult& output) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validStatus (drive.producerStatus) ||
            !validStatus (request.producerStatus) ||
            !validStatus (observation.producerStatus))
        {
            return StatusCode::InvalidArgument;
        }
        if (!validDriveShape (drive))
        {
            return StatusCode::InvalidArgument;
        }
        if (!validObservationState (observation.state) ||
            request.lifecycleGeneration == 0 || request.sessionId == 0 ||
            request.runId == 0 || request.stepId == 0 || request.requestId == 0 ||
            request.sourceId == 0 || request.sourceConfigurationRevision == 0 ||
            observation.lifecycleGeneration == 0 || observation.sessionId == 0 ||
            observation.runId == 0 || observation.stepId == 0 ||
            observation.observationId == 0 || observation.sourceId == 0 ||
            observation.sourceConfigurationRevision == 0 ||
            observation.observationSequence == 0 || request.policySequence == 0 ||
            request.requestSequence == 0 ||
            !validObservationShape (descriptor_, observation))
        {
            return StatusCode::InvalidArgument;
        }
        const bool selectedSubset =
            (request.selectedActiveMask &
             static_cast<uint8_t> (~descriptor_.declaredChannelMask)) == 0;
        const bool singleSelection =
            request.selectedActiveMask != 0 &&
            (request.selectedActiveMask &
             static_cast<uint8_t> (request.selectedActiveMask - 1U)) == 0;
        if (!selectedSubset ||
            (descriptor_.autonomy == SmallIndicatorAutonomy::FollowsDrive &&
             (drive.logicalActive ? !singleSelection
                                  : request.selectedActiveMask != 0)) ||
            (descriptor_.autonomy == SmallIndicatorAutonomy::AutonomousWhileEnabled &&
             (drive.logicalActive
                  ? request.selectedActiveMask != descriptor_.declaredChannelMask
                  : request.selectedActiveMask != 0)) ||
            (descriptor_.autonomy == SmallIndicatorAutonomy::ObservationOnly &&
             (drive.logicalActive || request.selectedActiveMask != 0)))
        {
            return StatusCode::InvalidArgument;
        }
        if ((request.selectedActiveMask != 0 ||
             observation.state == SmallIndicatorObservationState::NotObserved ||
             observation.state == SmallIndicatorObservationState::Fault) &&
            observation.safeStateObserved)
        {
            return StatusCode::InvalidArgument;
        }
        if (observation.lifecycleGeneration != lifecycleGeneration_ ||
            observation.sessionId != result_.sessionId ||
            observation.runId != result_.runId ||
            request.lifecycleGeneration != observation.lifecycleGeneration ||
            request.sessionId != observation.sessionId ||
            request.runId != observation.runId ||
            request.stepId != observation.stepId ||
            request.requestId != observation.requestId ||
            (requestSourceId_ != 0 && request.sourceId != requestSourceId_) ||
            (observationSourceId_ != 0 &&
             observation.sourceId != observationSourceId_) ||
            (requestSourceConfigurationRevision_ != 0 &&
             request.sourceConfigurationRevision !=
                 requestSourceConfigurationRevision_) ||
            (observationSourceConfigurationRevision_ != 0 &&
             observation.sourceConfigurationRevision !=
                 observationSourceConfigurationRevision_) ||
            drive.lifecycleGeneration != observation.lifecycleGeneration ||
            drive.sessionId != observation.sessionId ||
            drive.runId != observation.runId || drive.stepId != observation.stepId ||
            drive.requestId != request.requestId ||
            drive.driverDescriptorIdentityDigest !=
                descriptor_.expectedDriverDescriptorIdentityDigest ||
            drive.specimenReference != descriptor_.driverSpecimenReference ||
            drive.specimenRevision != descriptor_.driverSpecimenRevision ||
            drive.electricalEvidenceRevision !=
                descriptor_.driverElectricalEvidenceRevision ||
            drive.policyConfigurationId != descriptor_.driverPolicyConfigurationId ||
            drive.policyConfigurationRevision !=
                descriptor_.driverPolicyConfigurationRevision)
        {
            return StatusCode::InvalidArgument;
        }

        if (hasEvidence_ && !lastWasControl_ &&
            request.policySequence == cachedRequest_.policySequence)
        {
            if (!equalApply (drive, request, observation, now, cachedDrive_,
                             cachedRequest_, cachedObservation_, cachedNow_))
            {
                return StatusCode::InvalidArgument;
            }
            output = result_;
            return Status ();
        }
        if (hasEvidence_ && lastWasControl_ &&
            request.policySequence == cachedControl_.policySequence)
        {
            return StatusCode::InvalidArgument;
        }
        if (terminal_)
        {
            return StatusCode::InvalidArgument;
        }
        const bool sequenceDiscontinuity =
            (!hasEvidence_ &&
             (request.policySequence != 1 || request.requestSequence != 1 ||
              observation.observationSequence != 1)) ||
            (hasEvidence_ &&
             (request.policySequence != lastSequence_ + 1U ||
              request.requestSequence != lastRequestSequence_ + 1U ||
              observation.observationSequence != lastObservationSequence_ + 1U));
        const bool timestampDiscontinuity =
            !forwardOrEqual (request.requestedAt, startedAt_) ||
            !forwardOrEqual (observation.observedAt, request.requestedAt) ||
            !forwardOrEqual (now, observation.observedAt);

        SmallIndicatorSemanticResult candidate =
            baseResult (lifecycleGeneration_, result_.sessionId, result_.runId,
                        SmallIndicatorDisposition::Accepted);
        candidate.stepId                     = observation.stepId;
        candidate.requestId                  = observation.requestId;
        candidate.observationId              = observation.observationId;
        candidate.observationState           = observation.state;
        candidate.safeStateSatisfied         = observation.safeStateObserved;
        candidate.autonomousBehaviorObserved = observation.autonomousTransitionObserved;
        candidate.producerStatus =
            !request.producerStatus.ok () ? request.producerStatus
            : !drive.producerStatus.ok () ? drive.producerStatus
                                          : observation.producerStatus;

        if (!request.producerStatus.ok () ||
            (!drive.producerStatus.ok () &&
             drive.reason != LowSideDriveReason::Cancelled) ||
            !observation.producerStatus.ok () ||
            observation.state == SmallIndicatorObservationState::Fault)
        {
            reject (candidate, SmallIndicatorDisposition::ProducerFault,
                    SmallIndicatorReason::ProducerFault);
        }
        else if (drive.state == LowSideDriveState::Fault)
        {
            reject (candidate, SmallIndicatorDisposition::ProducerFault,
                    SmallIndicatorReason::DriverCancelled);
        }
        else if (!descriptor_.sourceEligible)
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::SourceIneligible);
        }
        else if (drive.state == LowSideDriveState::Off &&
                 drive.reason == LowSideDriveReason::Expired)
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::DriverExpired);
        }
        else if (drive.state == LowSideDriveState::Rejected)
        {
            SmallIndicatorReason reason = SmallIndicatorReason::ObservationMismatch;
            switch (drive.reason)
            {
                case LowSideDriveReason::SourceIneligible:
                    reason = SmallIndicatorReason::SourceIneligible;
                    break;
                case LowSideDriveReason::BudgetExceeded:
                    reason = SmallIndicatorReason::DriverBudgetExceeded;
                    break;
                case LowSideDriveReason::BaseBudgetInsufficient:
                    reason = SmallIndicatorReason::DriverBaseBudgetInsufficient;
                    break;
                case LowSideDriveReason::FlybackMissing:
                    reason = SmallIndicatorReason::DriverFlybackMissing;
                    break;
                case LowSideDriveReason::SequenceDiscontinuity:
                    reason = SmallIndicatorReason::DriverSequenceDiscontinuity;
                    break;
                case LowSideDriveReason::TimestampDiscontinuity:
                    reason = SmallIndicatorReason::DriverTimestampDiscontinuity;
                    break;
                case LowSideDriveReason::CapacityExceeded:
                    reason = SmallIndicatorReason::DriverCapacityExceeded;
                    break;
                default: return StatusCode::InvalidArgument;
            }
            reject (candidate, SmallIndicatorDisposition::Rejected, reason);
        }
        else if (drive.state == LowSideDriveState::Cancelled)
        {
            if (drive.reason != LowSideDriveReason::Cancelled)
            {
                return StatusCode::InvalidArgument;
            }
            reject (candidate, SmallIndicatorDisposition::Cancelled,
                    SmallIndicatorReason::DriverCancelled);
        }
        else if (drive.state == LowSideDriveState::Shutdown)
        {
            if (drive.reason != LowSideDriveReason::None)
            {
                return StatusCode::InvalidArgument;
            }
            reject (candidate, SmallIndicatorDisposition::Shutdown,
                    SmallIndicatorReason::DriverShutdown);
        }
        else if (sequenceDiscontinuity)
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::SequenceDiscontinuity);
        }
        else if (timestampDiscontinuity)
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::TimestampDiscontinuity);
        }
        else if (now.elapsedSince (observation.observedAt).milliseconds () >
                 descriptor_.maximumObservationAge.milliseconds ())
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::Stale);
        }
        else if (descriptor_.warmup.milliseconds () != 0 &&
                 (now.elapsedSince (startedAt_).milliseconds () <
                  descriptor_.warmup.milliseconds ()))
        {
            reject (candidate, SmallIndicatorDisposition::Incomplete,
                    SmallIndicatorReason::WarmupUnsatisfied);
        }
        else if (descriptor_.settling.milliseconds () != 0 &&
                 observation.observedAt.elapsedSince (request.requestedAt)
                         .milliseconds () < descriptor_.settling.milliseconds ())
        {
            reject (candidate, SmallIndicatorDisposition::Incomplete,
                    SmallIndicatorReason::SettlingUnsatisfied);
        }
        else if (drive.state == LowSideDriveState::Rejected ||
                 (drive.logicalActive && drive.state != LowSideDriveState::Requested))
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::ObservationMismatch);
        }
        else if (observation.state == SmallIndicatorObservationState::NotObserved)
        {
            reject (candidate, SmallIndicatorDisposition::Incomplete,
                    SmallIndicatorReason::ObservationMissing);
        }
        else if (observation.copiedLevelHigh !=
                 ((descriptor_.kind == SmallIndicatorKind::ActiveBuzzer ||
                   descriptor_.kind == SmallIndicatorKind::VoltageIndicator) &&
                  observation.observedActiveMask != 0))
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::PolarityMismatch);
        }
        else if (descriptor_.autonomy == SmallIndicatorAutonomy::FollowsDrive)
        {
            if (observation.autonomousTransitionObserved)
            {
                reject (candidate, SmallIndicatorDisposition::Rejected,
                        SmallIndicatorReason::UnexpectedAutonomy);
            }
            else if (observation.observedActiveMask != request.selectedActiveMask)
            {
                reject (candidate, SmallIndicatorDisposition::Rejected,
                        SmallIndicatorReason::ObservationMismatch);
            }
            else
            {
                candidate.semanticActive = drive.logicalActive;
                candidate.semanticActiveMask =
                    drive.logicalActive ? observation.observedActiveMask : 0;
                const bool observedHigh = observation.observedActiveMask != 0;
                if ((drive.logicalActive && observedHigh != descriptor_.activeHigh) ||
                    (!drive.logicalActive && observedHigh))
                {
                    reject (candidate, SmallIndicatorDisposition::Rejected,
                            SmallIndicatorReason::PolarityMismatch);
                }
            }
        }
        else if (descriptor_.autonomy == SmallIndicatorAutonomy::AutonomousWhileEnabled)
        {
            const bool observedActivity =
                observation.state == SmallIndicatorObservationState::Active ||
                observation.state == SmallIndicatorObservationState::Alternating;
            if (!drive.logicalActive && observedActivity)
            {
                reject (candidate, SmallIndicatorDisposition::Rejected,
                        SmallIndicatorReason::UnexpectedAutonomy);
            }
            else if (drive.logicalActive && !observation.autonomousTransitionObserved)
            {
                reject (candidate, SmallIndicatorDisposition::Incomplete,
                        SmallIndicatorReason::AutonomousWaveformMissing);
            }
            else
            {
                candidate.semanticActive = observedActivity;
                candidate.semanticActiveMask =
                    candidate.semanticActive ? observation.observedActiveMask : 0;
            }
        }
        else
        {
            if (drive.logicalActive || request.selectedActiveMask != 0)
            {
                reject (candidate, SmallIndicatorDisposition::Rejected,
                        SmallIndicatorReason::ObservationMismatch);
            }
            else
            {
                candidate.semanticActive =
                    observation.state == SmallIndicatorObservationState::Active;
                candidate.semanticActiveMask =
                    candidate.semanticActive ? observation.observedActiveMask : 0;
            }
        }

        if (candidate.disposition == SmallIndicatorDisposition::Accepted &&
            !observation.safeStateObserved && !candidate.semanticActive)
        {
            reject (candidate, SmallIndicatorDisposition::Rejected,
                    SmallIndicatorReason::SafeStateMismatch);
        }

        if (sequenceDiscontinuity || timestampDiscontinuity)
        {
            cachedDrive_       = drive;
            cachedRequest_     = request;
            cachedObservation_ = observation;
            cachedNow_         = now;
            hasEvidence_       = true;
            lastWasControl_    = false;
            terminal_          = true;
            result_            = candidate;
            output             = candidate;
            return Status ();
        }

        requestSourceId_                    = request.sourceId;
        observationSourceId_                = observation.sourceId;
        requestSourceConfigurationRevision_ = request.sourceConfigurationRevision;
        observationSourceConfigurationRevision_ =
            observation.sourceConfigurationRevision;
        lastRequestSequence_     = request.requestSequence;
        lastObservationSequence_ = observation.observationSequence;
        lastSequence_            = request.policySequence;
        lastObservedAt_          = observation.observedAt;
        cachedDrive_             = drive;
        cachedRequest_           = request;
        cachedObservation_       = observation;
        cachedNow_               = now;
        hasEvidence_             = true;
        lastWasControl_          = false;
        hasChronology_           = true;
        result_                  = candidate;
        terminal_ = candidate.disposition != SmallIndicatorDisposition::Incomplete;
        output    = candidate;
        return Status ();
    }

    Status SmallIndicatorSemanticsPolicy::cancel (
        const SmallIndicatorControl&  control,
        SmallIndicatorSemanticResult& output) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validStatus (control.producerStatus))
        {
            return StatusCode::InvalidArgument;
        }
        if (control.lifecycleGeneration != lifecycleGeneration_ ||
            control.sessionId != result_.sessionId || control.runId != result_.runId ||
            control.stepId == 0 || control.controlId == 0 || control.sourceId == 0 ||
            control.sourceConfigurationRevision == 0 || control.policySequence == 0 ||
            control.offConfirmed != control.producerStatus.ok () ||
            (requestSourceId_ != 0 && control.sourceId != requestSourceId_) ||
            (requestSourceConfigurationRevision_ != 0 &&
             control.sourceConfigurationRevision !=
                 requestSourceConfigurationRevision_))
        {
            return StatusCode::InvalidArgument;
        }
        const bool timestampDiscontinuity =
            hasChronology_ && !forwardOrEqual (control.observedAt, lastObservedAt_);

        if (hasEvidence_ && lastWasControl_ &&
            control.policySequence == cachedControl_.policySequence)
        {
            if (!equalControl (control, cachedControl_))
            {
                return StatusCode::InvalidArgument;
            }
            output = result_;
            return Status ();
        }
        if (hasEvidence_ && !lastWasControl_ &&
            control.policySequence == cachedRequest_.policySequence)
        {
            return StatusCode::InvalidArgument;
        }
        if (terminal_)
        {
            return StatusCode::InvalidArgument;
        }
        const bool sequenceDiscontinuity =
            (!hasEvidence_ && control.policySequence != 1) ||
            (hasEvidence_ && control.policySequence != lastSequence_ + 1U);
        if (sequenceDiscontinuity || timestampDiscontinuity)
        {
            SmallIndicatorSemanticResult candidate =
                baseResult (lifecycleGeneration_, result_.sessionId, result_.runId,
                            SmallIndicatorDisposition::Rejected);
            candidate.stepId        = control.stepId;
            candidate.observationId = control.controlId;
            candidate.reason = sequenceDiscontinuity
                                   ? SmallIndicatorReason::SequenceDiscontinuity
                                   : SmallIndicatorReason::TimestampDiscontinuity;
            cachedControl_   = control;
            hasEvidence_     = true;
            lastWasControl_  = true;
            terminal_        = true;
            result_          = candidate;
            output           = candidate;
            return Status ();
        }

        SmallIndicatorSemanticResult candidate =
            baseResult (lifecycleGeneration_, result_.sessionId, result_.runId,
                        SmallIndicatorDisposition::Cancelled);
        candidate.stepId        = control.stepId;
        candidate.observationId = control.controlId;
        candidate.reason        = SmallIndicatorReason::Cancelled;
        candidate.safeStateSatisfied =
            control.offConfirmed && control.producerStatus.ok ();
        candidate.producerStatus = control.producerStatus;
        if (!candidate.safeStateSatisfied)
        {
            candidate.disposition = SmallIndicatorDisposition::ProducerFault;
        }

        lastSequence_                       = control.policySequence;
        lastObservedAt_                     = control.observedAt;
        cachedControl_                      = control;
        hasEvidence_                        = true;
        lastWasControl_                     = true;
        hasChronology_                      = true;
        requestSourceId_                    = control.sourceId;
        requestSourceConfigurationRevision_ = control.sourceConfigurationRevision;
        result_                             = candidate;
        terminal_                           = true;
        output                              = candidate;
        return Status ();
    }

    Status SmallIndicatorSemanticsPolicy::reset () noexcept
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
        requestSourceId_ = observationSourceId_ = 0;
        requestSourceConfigurationRevision_     = 0;
        observationSourceConfigurationRevision_ = 0;
        lastSequence_                           = 0;
        lastRequestSequence_                    = 0;
        lastObservationSequence_                = 0;
        hasEvidence_                            = false;
        lastWasControl_                         = false;
        hasChronology_                          = false;
        terminal_                               = false;
        result_ =
            baseResult (lifecycleGeneration_, 0, 0, SmallIndicatorDisposition::Idle);
        return Status ();
    }

    SmallIndicatorSemanticResult
    SmallIndicatorSemanticsPolicy::snapshot () const noexcept
    {
        return result_;
    }

#if defined(ADK_TESTING)
    void SmallIndicatorSemanticsPolicy::seedLifecycleGenerationForTest (
        uint32_t generation) noexcept
    {
        lifecycleGeneration_ = generation;
    }

    void SmallIndicatorSemanticsPolicy::seedSequencesForTest (
        uint32_t policySequence, uint32_t requestSequence,
        uint32_t observationSequence) noexcept
    {
        lastSequence_                       = policySequence;
        lastRequestSequence_                = requestSequence;
        lastObservationSequence_            = observationSequence;
        cachedRequest_.policySequence       = policySequence;
        cachedRequest_.requestSequence      = requestSequence;
        cachedObservation_.observationSequence = observationSequence;
        hasEvidence_                        = true;
        lastWasControl_                     = false;
    }
#endif
} // namespace adk
