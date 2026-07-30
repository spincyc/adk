#pragma once

#include "bounded_low_side_driver_policy.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct SmallIndicatorKind : uint8_t
    {
        ActiveBuzzer,
        TrafficLight,
        DualColorLed,
        AutoFlashLed,
        VoltageIndicator
    };

    enum struct SmallIndicatorAutonomy : uint8_t
    {
        FollowsDrive,
        AutonomousWhileEnabled,
        ObservationOnly
    };

    enum struct SmallIndicatorSafeState : uint8_t
    {
        DriveInactive,
        HighImpedanceRequired,
        UnpoweredRequired
    };

    enum struct SmallIndicatorObservationState : uint8_t
    {
        NotObserved,
        Inactive,
        Active,
        Alternating,
        Fault
    };

    struct SmallIndicatorChannels
    {
        static constexpr uint8_t Red     = 0x01;
        static constexpr uint8_t Amber   = 0x02;
        static constexpr uint8_t Green   = 0x04;
        static constexpr uint8_t Blue    = 0x08;
        static constexpr uint8_t Sound   = 0x10;
        static constexpr uint8_t Voltage = 0x20;
    };

    enum struct SmallIndicatorDisposition : uint8_t
    {
        Idle,
        Eligible,
        Incomplete,
        Accepted,
        Rejected,
        ProducerFault,
        Cancelled,
        Shutdown
    };

    enum struct SmallIndicatorReason : uint8_t
    {
        None,
        SourceIneligible,
        DriverBudgetExceeded,
        DriverBaseBudgetInsufficient,
        DriverFlybackMissing,
        DriverSequenceDiscontinuity,
        DriverTimestampDiscontinuity,
        DriverExpired,
        DriverCapacityExceeded,
        DriverCancelled,
        DriverShutdown,
        ProducerFault,
        SequenceDiscontinuity,
        TimestampDiscontinuity,
        Stale,
        ObservationMissing,
        WarmupUnsatisfied,
        SettlingUnsatisfied,
        PolarityMismatch,
        SafeStateMismatch,
        AutonomousWaveformMissing,
        UnexpectedAutonomy,
        ObservationMismatch,
        Cancelled
    };

    struct SmallIndicatorDescriptor
    {
        uint16_t                schemaRevision;
        uint32_t                specimenFamilyReference;
        uint32_t                specimenReference;
        uint16_t                specimenRevision;
        uint16_t                electricalEvidenceRevision;
        uint32_t                sourcePacketDigest;
        uint32_t                configurationId;
        uint16_t                configurationRevision;
        uint32_t                driverSpecimenReference;
        uint16_t                driverSpecimenRevision;
        uint16_t                driverElectricalEvidenceRevision;
        uint32_t                driverPolicyConfigurationId;
        uint16_t                driverPolicyConfigurationRevision;
        uint32_t                expectedDriverDescriptorIdentityDigest;
        SmallIndicatorKind      kind;
        SmallIndicatorAutonomy  autonomy;
        SmallIndicatorSafeState safeState;
        bool                    sourceEligible;
        bool                    activeHigh;
        bool                    populatedResistorDeclared;
        bool                    populatedDriverDeclared;
        uint8_t                 declaredChannelMask;
        Duration                warmup;
        Duration                settling;
        Duration                maximumObservationAge;
    };

    struct SmallIndicatorSemanticRequest
    {
        uint32_t  lifecycleGeneration;
        uint32_t  sessionId;
        uint32_t  runId;
        uint16_t  stepId;
        uint32_t  requestId;
        uint8_t   sourceId;
        uint16_t  sourceConfigurationRevision;
        uint32_t  policySequence;
        uint32_t  requestSequence;
        TimePoint requestedAt;
        uint8_t   selectedActiveMask;
        Status    producerStatus;
    };

    struct SmallIndicatorObservation
    {
        uint32_t                       lifecycleGeneration;
        uint32_t                       sessionId;
        uint32_t                       runId;
        uint16_t                       stepId;
        uint32_t                       requestId;
        uint32_t                       observationId;
        uint8_t                        sourceId;
        uint16_t                       sourceConfigurationRevision;
        uint32_t                       observationSequence;
        TimePoint                      observedAt;
        SmallIndicatorObservationState state;
        uint8_t                        observedActiveMask;
        bool                           copiedLevelHigh;
        bool                           autonomousTransitionObserved;
        bool                           safeStateObserved;
        Status                         producerStatus;
    };

    struct SmallIndicatorSemanticResult
    {
        uint32_t                       lifecycleGeneration;
        uint32_t                       sessionId;
        uint32_t                       runId;
        uint16_t                       stepId;
        uint32_t                       requestId;
        uint32_t                       observationId;
        SmallIndicatorDisposition      disposition;
        SmallIndicatorReason           reason;
        SmallIndicatorObservationState observationState;
        bool                           semanticActive;
        uint8_t                        semanticActiveMask;
        bool                           safeStateSatisfied;
        bool                           autonomousBehaviorObserved;
        Status                         producerStatus;
    };

    struct SmallIndicatorControl
    {
        uint32_t  lifecycleGeneration;
        uint32_t  sessionId;
        uint32_t  runId;
        uint16_t  stepId;
        uint32_t  controlId;
        uint8_t   sourceId;
        uint16_t  sourceConfigurationRevision;
        uint32_t  policySequence;
        TimePoint observedAt;
        bool      offConfirmed;
        Status    producerStatus;
    };

    struct SmallIndicatorSemanticsPolicy
    {
        explicit SmallIndicatorSemanticsPolicy (
            const SmallIndicatorDescriptor& descriptor) noexcept;

        SmallIndicatorSemanticsPolicy (const SmallIndicatorSemanticsPolicy&) = delete;
        SmallIndicatorSemanticsPolicy&
        operator= (const SmallIndicatorSemanticsPolicy&)                = delete;
        SmallIndicatorSemanticsPolicy (SmallIndicatorSemanticsPolicy&&) = delete;
        SmallIndicatorSemanticsPolicy&
        operator= (SmallIndicatorSemanticsPolicy&&) = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

        Status beginSession (uint32_t sessionId, uint32_t runId,
                             TimePoint startedAt) noexcept;
        Status apply (const LowSideDriveIntent&            drive,
                      const SmallIndicatorSemanticRequest& request,
                      const SmallIndicatorObservation& observation, TimePoint now,
                      SmallIndicatorSemanticResult& result) noexcept;
        Status cancel (const SmallIndicatorControl&  control,
                       SmallIndicatorSemanticResult& result) noexcept;
        Status reset () noexcept;

        SmallIndicatorSemanticResult snapshot () const noexcept;

#if defined(ADK_TESTING)
        void seedLifecycleGenerationForTest (uint32_t generation) noexcept;
        void seedSequencesForTest           (uint32_t policySequence,
                                             uint32_t requestSequence,
                                             uint32_t observationSequence) noexcept;
#endif

      private:
        SmallIndicatorDescriptor      descriptor_;
        SmallIndicatorSemanticResult  result_;
        LowSideDriveIntent            cachedDrive_;
        SmallIndicatorSemanticRequest cachedRequest_;
        SmallIndicatorObservation     cachedObservation_;
        SmallIndicatorControl         cachedControl_;
        TimePoint                     cachedNow_;
        TimePoint                     startedAt_;
        TimePoint                     lastObservedAt_;
        uint32_t                      lifecycleGeneration_;
        uint32_t                      lastSequence_;
        uint32_t                      lastRequestSequence_;
        uint32_t                      lastObservationSequence_;
        uint8_t                       requestSourceId_;
        uint8_t                       observationSourceId_;
        uint16_t                      requestSourceConfigurationRevision_;
        uint16_t                      observationSourceConfigurationRevision_;
        bool                          initialized_;
        bool                          hasEvidence_;
        bool                          lastWasControl_;
        bool                          hasChronology_;
        bool                          terminal_;
    };

    Status validateSmallIndicatorDescriptor (
        const SmallIndicatorDescriptor& descriptor) noexcept;
} // namespace adk
