#include <small_indicator_semantics_policy.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::SmallIndicatorDescriptor descriptor ()
    {
        return {1,
                0x10203040UL,
                0x11223344UL,
                2,
                3,
                0x55667788UL,
                0x22334455UL,
                4,
                0x33445566UL,
                5,
                6,
                0x44556677UL,
                7,
                0x66778899UL,
                adk::SmallIndicatorKind::TrafficLight,
                adk::SmallIndicatorAutonomy::FollowsDrive,
                adk::SmallIndicatorSafeState::DriveInactive,
                true,
                true,
                true,
                false,
                static_cast<uint8_t> (adk::SmallIndicatorChannels::Red |
                                      adk::SmallIndicatorChannels::Amber |
                                      adk::SmallIndicatorChannels::Green),
                adk::Duration (0),
                adk::Duration (0),
                adk::Duration (100)};
    }

    bool validRow (adk::SmallIndicatorKind kind, adk::SmallIndicatorAutonomy autonomy,
                   adk::SmallIndicatorSafeState safeState, bool activeHigh,
                   bool resistor, bool driver, uint8_t mask)
    {
        using Autonomy  = adk::SmallIndicatorAutonomy;
        using Channels  = adk::SmallIndicatorChannels;
        using Kind      = adk::SmallIndicatorKind;
        using SafeState = adk::SmallIndicatorSafeState;

        if (!activeHigh)
        {
            return false;
        }

        if (kind == Kind::ActiveBuzzer)
        {
            const bool follows    = autonomy == Autonomy::FollowsDrive &&
                                    safeState == SafeState::DriveInactive;
            const bool autonomous = autonomy == Autonomy::AutonomousWhileEnabled &&
                                    safeState == SafeState::UnpoweredRequired;
            return (follows || autonomous) && !resistor && driver &&
                   mask == Channels::Sound;
        }

        if (kind == Kind::TrafficLight)
        {
            return autonomy == Autonomy::FollowsDrive &&
                   safeState == SafeState::DriveInactive && resistor && !driver &&
                   mask == static_cast<uint8_t> (Channels::Red | Channels::Amber |
                                                 Channels::Green);
        }

        if (kind == Kind::DualColorLed)
        {
            return autonomy == Autonomy::FollowsDrive &&
                   safeState == SafeState::DriveInactive && resistor && !driver &&
                   mask == static_cast<uint8_t> (Channels::Red | Channels::Green);
        }

        if (kind == Kind::AutoFlashLed)
        {
            return autonomy == Autonomy::AutonomousWhileEnabled &&
                   safeState == SafeState::UnpoweredRequired && resistor && !driver &&
                   mask == static_cast<uint8_t> (Channels::Red | Channels::Green |
                                                 Channels::Blue);
        }

        if (kind == Kind::VoltageIndicator)
        {
            return autonomy == Autonomy::ObservationOnly &&
                   safeState == SafeState::HighImpedanceRequired && resistor &&
                   !driver && mask == Channels::Voltage;
        }

        return false;
    }

    void testChannelConstants ()
    {
        require (adk::SmallIndicatorChannels::Red == 0x01, "red channel encoding");
        require (adk::SmallIndicatorChannels::Amber == 0x02, "amber channel encoding");
        require (adk::SmallIndicatorChannels::Green == 0x04, "green channel encoding");
        require (adk::SmallIndicatorChannels::Blue == 0x08, "blue channel encoding");
        require (adk::SmallIndicatorChannels::Sound == 0x10, "sound channel encoding");
        require (adk::SmallIndicatorChannels::Voltage == 0x20,
                 "voltage channel encoding");
    }

    void testExactValidityTable ()
    {
        for (uint8_t kind = 0; kind < 5; ++kind)
        {
            for (uint8_t autonomy = 0; autonomy < 3; ++autonomy)
            {
                for (uint8_t safeState = 0; safeState < 3; ++safeState)
                {
                    for (uint8_t activeHigh = 0; activeHigh < 2; ++activeHigh)
                    {
                        for (uint8_t resistor = 0; resistor < 2; ++resistor)
                        {
                            for (uint8_t driver = 0; driver < 2; ++driver)
                            {
                                for (uint16_t mask = 0; mask <= 255; ++mask)
                                {
                                    adk::SmallIndicatorDescriptor value = descriptor ();
                                    value.kind =
                                        static_cast<adk::SmallIndicatorKind> (kind);
                                    value.autonomy =
                                        static_cast<adk::SmallIndicatorAutonomy> (
                                            autonomy);
                                    value.safeState =
                                        static_cast<adk::SmallIndicatorSafeState> (
                                            safeState);
                                    value.activeHigh                = activeHigh != 0;
                                    value.populatedResistorDeclared = resistor != 0;
                                    value.populatedDriverDeclared   = driver != 0;
                                    value.declaredChannelMask =
                                        static_cast<uint8_t> (mask);

                                    const bool expected =
                                        validRow (value.kind, value.autonomy,
                                                  value.safeState, value.activeHigh,
                                                  value.populatedResistorDeclared,
                                                  value.populatedDriverDeclared,
                                                  value.declaredChannelMask);
                                    require (
                                        adk::validateSmallIndicatorDescriptor (value)
                                                .ok () == expected,
                                        "descriptor validity table is exact");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void testEveryEnumEncoding ()
    {
        for (uint16_t encoding = 0; encoding <= 255; ++encoding)
        {
            adk::SmallIndicatorDescriptor value = descriptor ();
            value.kind = static_cast<adk::SmallIndicatorKind> (encoding);
            require (adk::validateSmallIndicatorDescriptor (value).ok () ==
                         (encoding ==
                          static_cast<uint8_t> (adk::SmallIndicatorKind::TrafficLight)),
                     "kind encoding is closed");

            value          = descriptor ();
            value.autonomy = static_cast<adk::SmallIndicatorAutonomy> (encoding);
            require (adk::validateSmallIndicatorDescriptor (value).ok () ==
                         (encoding == static_cast<uint8_t> (
                                          adk::SmallIndicatorAutonomy::FollowsDrive)),
                     "autonomy encoding is closed");

            value           = descriptor ();
            value.safeState = static_cast<adk::SmallIndicatorSafeState> (encoding);
            require (adk::validateSmallIndicatorDescriptor (value).ok () ==
                         (encoding == static_cast<uint8_t> (
                                          adk::SmallIndicatorSafeState::DriveInactive)),
                     "safe-state encoding is closed");
        }
    }

    void testIdentityAndDurationLimits ()
    {
        adk::SmallIndicatorDescriptor value = descriptor ();

#define REQUIRE_ZERO_INVALID(field)                                                    \
    do                                                                                 \
    {                                                                                  \
        value       = descriptor ();                                                   \
        value.field = 0;                                                               \
        require (!adk::validateSmallIndicatorDescriptor (value).ok (),                 \
                 #field " rejects zero");                                              \
    }                                                                                  \
    while (false)

        REQUIRE_ZERO_INVALID (schemaRevision);
        REQUIRE_ZERO_INVALID (specimenFamilyReference);
        REQUIRE_ZERO_INVALID (specimenReference);
        REQUIRE_ZERO_INVALID (specimenRevision);
        REQUIRE_ZERO_INVALID (electricalEvidenceRevision);
        REQUIRE_ZERO_INVALID (sourcePacketDigest);
        REQUIRE_ZERO_INVALID (configurationId);
        REQUIRE_ZERO_INVALID (configurationRevision);
        REQUIRE_ZERO_INVALID (driverSpecimenReference);
        REQUIRE_ZERO_INVALID (driverSpecimenRevision);
        REQUIRE_ZERO_INVALID (driverElectricalEvidenceRevision);
        REQUIRE_ZERO_INVALID (driverPolicyConfigurationId);
        REQUIRE_ZERO_INVALID (driverPolicyConfigurationRevision);

#undef REQUIRE_ZERO_INVALID

        value                                        = descriptor ();
        value.expectedDriverDescriptorIdentityDigest = 0;
        require (adk::validateSmallIndicatorDescriptor (value).ok (),
                 "zero driver descriptor digest is valid");

#define REQUIRE_MAX_VALID(field, maximum)                                              \
    do                                                                                 \
    {                                                                                  \
        value       = descriptor ();                                                   \
        value.field = maximum;                                                         \
        require (adk::validateSmallIndicatorDescriptor (value).ok (),                  \
                 #field " accepts its maximum");                                       \
    }                                                                                  \
    while (false)

        REQUIRE_MAX_VALID (specimenFamilyReference, UINT32_MAX);
        REQUIRE_MAX_VALID (specimenReference, UINT32_MAX);
        REQUIRE_MAX_VALID (specimenRevision, UINT16_MAX);
        REQUIRE_MAX_VALID (electricalEvidenceRevision, UINT16_MAX);
        REQUIRE_MAX_VALID (sourcePacketDigest, UINT32_MAX);
        REQUIRE_MAX_VALID (configurationId, UINT32_MAX);
        REQUIRE_MAX_VALID (configurationRevision, UINT16_MAX);
        REQUIRE_MAX_VALID (driverSpecimenReference, UINT32_MAX);
        REQUIRE_MAX_VALID (driverSpecimenRevision, UINT16_MAX);
        REQUIRE_MAX_VALID (driverElectricalEvidenceRevision, UINT16_MAX);
        REQUIRE_MAX_VALID (driverPolicyConfigurationId, UINT32_MAX);
        REQUIRE_MAX_VALID (driverPolicyConfigurationRevision, UINT16_MAX);
        REQUIRE_MAX_VALID (expectedDriverDescriptorIdentityDigest, UINT32_MAX);
        REQUIRE_MAX_VALID (warmup, adk::Duration (0x7FFFFFFFUL));
        REQUIRE_MAX_VALID (settling, adk::Duration (0x7FFFFFFFUL));
        REQUIRE_MAX_VALID (maximumObservationAge, adk::Duration (0x7FFFFFFFUL));

#undef REQUIRE_MAX_VALID

        value = descriptor ();

        value.warmup = adk::Duration (0);

        value.settling = adk::Duration (0);

        value.maximumObservationAge = adk::Duration (0);

        require (adk::validateSmallIndicatorDescriptor (value).ok (),
                 "zero known delays and age are valid");

        value = descriptor ();

        value.warmup = adk::Duration (0x80000000UL);

        require (!adk::validateSmallIndicatorDescriptor (value).ok (),
                 "half-range warmup rejects");

        value = descriptor ();

        value.warmup = adk::Duration (UINT32_MAX);

        require (!adk::validateSmallIndicatorDescriptor (value).ok (),
                 "maximum encoded warmup rejects");

        value                = descriptor ();
        value.sourceEligible = false;
        require (adk::validateSmallIndicatorDescriptor (value).ok (),
                 "source eligibility is semantic rather than structural");
    }

    adk::LowSideDriveIntent
    driveIntent (const adk::SmallIndicatorDescriptor& value,
                 adk::LowSideDriveState  state  = adk::LowSideDriveState::Requested,
                 adk::LowSideDriveReason reason = adk::LowSideDriveReason::None)
    {
        const bool active = state == adk::LowSideDriveState::Requested;
        return {1,
                value.expectedDriverDescriptorIdentityDigest,
                value.driverSpecimenReference,
                value.driverSpecimenRevision,
                value.driverElectricalEvidenceRevision,
                value.driverPolicyConfigurationId,
                value.driverPolicyConfigurationRevision,
                7,
                9,
                11,
                13,
                state,
                reason,
                active,
                active,
                active ? 100U : 0U,
                active ? 100U : 0U,
                active ? 1000U : 0U,
                adk::TimePoint (active ? 150U : 100U),
                state == adk::LowSideDriveState::Fault
                    ? adk::StatusCode::HardwareFailure
                    : adk::StatusCode::Ok};
    }

    adk::SmallIndicatorSemanticRequest
    semanticRequest (uint32_t policySequence = 1, uint32_t requestSequence = 1,
                     uint8_t selectedMask = adk::SmallIndicatorChannels::Green)
    {
        return {1,
                7,
                9,
                11,
                13,
                4,
                5,
                policySequence,
                requestSequence,
                adk::TimePoint (100),
                selectedMask,
                adk::StatusCode::Ok};
    }

    adk::SmallIndicatorObservation
    observation (uint32_t                            sequence = 1,
                 adk::SmallIndicatorObservationState state =
                     adk::SmallIndicatorObservationState::Active,
                 uint8_t observedMask = adk::SmallIndicatorChannels::Green)
    {
        return {1,        7,
                9,        11,
                13,       17,
                4,        4,
                sequence, adk::TimePoint (110),
                state,    observedMask,
                false,    false,
                false,    adk::StatusCode::Ok};
    }

    adk::SmallIndicatorControl control (uint32_t sequence, bool offConfirmed = true,
                                        adk::Status status = adk::StatusCode::Ok)
    {
        return {1,     7, 9, 12, 19, 4, 5, sequence, adk::TimePoint (120), offConfirmed,
                status};
    }

    void advanceTuple (adk::LowSideDriveIntent&            drive,
                       adk::SmallIndicatorSemanticRequest& request,
                       adk::SmallIndicatorObservation&     observation,
                       uint32_t policySequence, uint32_t requestSequence,
                       uint32_t observationSequence, uint32_t observedAt)
    {
        drive.stepId    = 12;
        drive.requestId = 14;

        request.stepId          = 12;
        request.requestId       = 14;
        request.policySequence  = policySequence;
        request.requestSequence = requestSequence;
        request.requestedAt     = adk::TimePoint (observedAt);

        observation.stepId              = 12;
        observation.requestId           = 14;
        observation.observationId       = 18;
        observation.observationSequence = observationSequence;
        observation.observedAt          = adk::TimePoint (observedAt);
    }

    bool equalResult (const adk::SmallIndicatorSemanticResult& left,
                      const adk::SmallIndicatorSemanticResult& right)
    {
        return left.lifecycleGeneration == right.lifecycleGeneration &&
               left.sessionId == right.sessionId && left.runId == right.runId &&
               left.stepId == right.stepId && left.requestId == right.requestId &&
               left.observationId == right.observationId &&
               left.disposition == right.disposition && left.reason == right.reason &&
               left.observationState == right.observationState &&
               left.semanticActive == right.semanticActive &&
               left.semanticActiveMask == right.semanticActiveMask &&
               left.safeStateSatisfied == right.safeStateSatisfied &&
               left.autonomousBehaviorObserved == right.autonomousBehaviorObserved &&
               left.producerStatus == right.producerStatus;
    }

    adk::SmallIndicatorSemanticResult canaryResult ()
    {
        return {0xA1A2A3A4UL,
                0xB1B2B3B4UL,
                0xC1C2C3C4UL,
                0xD1D2,
                0xE1E2E3E4UL,
                0xF1F2F3F4UL,
                adk::SmallIndicatorDisposition::ProducerFault,
                adk::SmallIndicatorReason::ObservationMismatch,
                adk::SmallIndicatorObservationState::Fault,
                true,
                0x3F,
                true,
                true,
                adk::StatusCode::HardwareFailure};
    }

    void startPolicy (adk::SmallIndicatorSemanticsPolicy& policy)
    {
        require (policy.initialize ().ok (), "policy initializes");

        require (policy.beginSession (7, 9, adk::TimePoint (90)).ok (),
                 "session begins");
    }

    void testLifecycleAndHappyPath ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        adk::SmallIndicatorSemanticsPolicy policy (value);

        adk::SmallIndicatorSemanticResult output = canaryResult ();

        const adk::LowSideDriveIntent drive = driveIntent (value);

        const adk::SmallIndicatorSemanticRequest request = semanticRequest ();

        const adk::SmallIndicatorObservation observed = observation ();

        require (!policy.initialized (), "constructed policy is inactive");
        require (policy.snapshot ().lifecycleGeneration == 0,
                 "construction starts at generation zero");
        require (policy.apply (drive, request, observed, adk::TimePoint (110), output)
                         .error () == adk::StatusCode::NotInitialized,
                 "apply before initialize rejects");
        require (equalResult (output, canaryResult ()),
                 "pre-initialize rejection preserves output");

        require (policy.initialize ().ok (), "first initialize succeeds");
        require (policy.initialized (), "initialized reports true");
        require (policy.snapshot ().lifecycleGeneration == 1,
                 "initialize advances generation");
        require (policy.initialize ().ok (), "initialize is idempotent");
        require (policy.snapshot ().lifecycleGeneration == 1,
                 "idempotent initialize preserves generation");
        require (policy.beginSession (0, 9, adk::TimePoint (90)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "zero session rejects");
        require (policy.beginSession (7, 9, adk::TimePoint (90)).ok (),
                 "valid session begins");
        require (policy.beginSession (7, 9, adk::TimePoint (90)).error () ==
                     adk::StatusCode::ResourceBusy,
                 "second session rejects");

        require (
            policy.apply (drive, request, observed, adk::TimePoint (110), output).ok (),
            "happy path applies");
        require (output.disposition == adk::SmallIndicatorDisposition::Accepted,
                 "happy path accepted");
        require (output.reason == adk::SmallIndicatorReason::None,
                 "happy path has no reason");
        require (output.semanticActive &&
                     output.semanticActiveMask == adk::SmallIndicatorChannels::Green,
                 "selected green is preserved");

        const adk::SmallIndicatorSemanticResult accepted = output;
        output                                           = canaryResult ();

        require (
            policy.apply (drive, request, observed, adk::TimePoint (110), output).ok (),
            "identical apply is idempotent");
        require (equalResult (output, accepted), "duplicate result is byte-semantic");

        policy.shutdown ();

        require (!policy.initialized (), "shutdown clears initialized state");
        require (policy.snapshot ().disposition ==
                     adk::SmallIndicatorDisposition::Shutdown,
                 "shutdown publishes terminal state");
        policy.shutdown ();

        require (policy.snapshot ().disposition ==
                     adk::SmallIndicatorDisposition::Shutdown,
                 "shutdown is idempotent");
        require (policy.initialize ().ok (), "reinitialize succeeds");
        require (policy.snapshot ().lifecycleGeneration == 2,
                 "reinitialize advances generation");
        require (policy.reset ().ok (), "reset succeeds");
        require (policy.snapshot ().lifecycleGeneration == 3,
                 "reset advances generation");
    }

    void testCorrelationAndAtomicRejection ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

#define REQUIRE_DRIVE_MISMATCH(field)                                                  \
    do                                                                                 \
    {                                                                                  \
        adk::SmallIndicatorSemanticsPolicy policy (value);                             \
        /* Begin one independent policy instance. */                                   \
        startPolicy (policy);                                                          \
        /* Mutate exactly the selected drive field. */                                 \
        adk::LowSideDriveIntent changed = driveIntent (value);                         \
        ++changed.field;                                                               \
        adk::SmallIndicatorSemanticResult output = canaryResult ();                    \
        /* Preserve both state and caller storage on rejection. */                     \
        const adk::SmallIndicatorSemanticResult before = policy.snapshot ();           \
        /* Exercise the public candidate boundary. */                                  \
        require (policy.apply (changed, semanticRequest (), observation (),            \
                               adk::TimePoint (110), output)                           \
                         .error () == adk::StatusCode::InvalidArgument,                \
                 #field " mismatch rejects");                                          \
        require (equalResult (before, policy.snapshot ()),                             \
                 #field " mismatch preserves policy");                                 \
        require (equalResult (output, canaryResult ()),                                \
                 #field " mismatch preserves output");                                 \
    }                                                                                  \
    while (false)

        REQUIRE_DRIVE_MISMATCH (driverDescriptorIdentityDigest);
        REQUIRE_DRIVE_MISMATCH (specimenReference);
        REQUIRE_DRIVE_MISMATCH (specimenRevision);
        REQUIRE_DRIVE_MISMATCH (electricalEvidenceRevision);
        REQUIRE_DRIVE_MISMATCH (policyConfigurationId);
        REQUIRE_DRIVE_MISMATCH (policyConfigurationRevision);
        REQUIRE_DRIVE_MISMATCH (lifecycleGeneration);
        REQUIRE_DRIVE_MISMATCH (sessionId);
        REQUIRE_DRIVE_MISMATCH (runId);
        REQUIRE_DRIVE_MISMATCH (stepId);
        REQUIRE_DRIVE_MISMATCH (requestId);

#undef REQUIRE_DRIVE_MISMATCH

        adk::SmallIndicatorSemanticsPolicy policy (value);

        startPolicy (policy);

        adk::SmallIndicatorSemanticRequest request = semanticRequest ();

        adk::SmallIndicatorObservation observed = observation ();

        adk::SmallIndicatorSemanticResult output = canaryResult ();

        request.sourceConfigurationRevision = 0;
        require (policy.apply (driveIntent (value), request, observed,
                               adk::TimePoint (110), output)
                         .error () == adk::StatusCode::InvalidArgument,
                 "zero request source configuration rejects");
        require (equalResult (output, canaryResult ()),
                 "invalid request preserves output");

        request = semanticRequest ();
        ++observed.observationId;
        require (policy
                     .apply (driveIntent (value), request, observed,
                             adk::TimePoint (110), output)
                     .ok (),
                 "observation identity is independently attributable");
    }

    void testDriveMappings ()
    {
        struct Mapping
        {
            adk::LowSideDriveState         driveState;
            adk::LowSideDriveReason        driveReason;
            adk::SmallIndicatorDisposition disposition;
            adk::SmallIndicatorReason      reason;
        };

        const Mapping mappings[] = {
            {adk::LowSideDriveState::Rejected,
             adk::LowSideDriveReason::SourceIneligible,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::SourceIneligible},
            {adk::LowSideDriveState::Rejected, adk::LowSideDriveReason::BudgetExceeded,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverBudgetExceeded},
            {adk::LowSideDriveState::Rejected,
             adk::LowSideDriveReason::BaseBudgetInsufficient,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverBaseBudgetInsufficient},
            {adk::LowSideDriveState::Rejected, adk::LowSideDriveReason::FlybackMissing,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverFlybackMissing},
            {adk::LowSideDriveState::Rejected,
             adk::LowSideDriveReason::SequenceDiscontinuity,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverSequenceDiscontinuity},
            {adk::LowSideDriveState::Rejected,
             adk::LowSideDriveReason::TimestampDiscontinuity,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverTimestampDiscontinuity},
            {adk::LowSideDriveState::Off, adk::LowSideDriveReason::Expired,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverExpired},
            {adk::LowSideDriveState::Rejected,
             adk::LowSideDriveReason::CapacityExceeded,
             adk::SmallIndicatorDisposition::Rejected,
             adk::SmallIndicatorReason::DriverCapacityExceeded},
            {adk::LowSideDriveState::Cancelled, adk::LowSideDriveReason::Cancelled,
             adk::SmallIndicatorDisposition::Cancelled,
             adk::SmallIndicatorReason::DriverCancelled},
            {adk::LowSideDriveState::Fault, adk::LowSideDriveReason::ProducerFault,
             adk::SmallIndicatorDisposition::ProducerFault,
             adk::SmallIndicatorReason::ProducerFault},
            {adk::LowSideDriveState::Fault, adk::LowSideDriveReason::Cancelled,
             adk::SmallIndicatorDisposition::ProducerFault,
             adk::SmallIndicatorReason::DriverCancelled},
            {adk::LowSideDriveState::Shutdown, adk::LowSideDriveReason::None,
             adk::SmallIndicatorDisposition::Shutdown,
             adk::SmallIndicatorReason::DriverShutdown}};

        for (const Mapping& mapping : mappings)
        {
            const adk::SmallIndicatorDescriptor value = descriptor ();

            adk::SmallIndicatorSemanticsPolicy policy (value);

            startPolicy (policy);

            adk::SmallIndicatorSemanticResult output = canaryResult ();

            adk::SmallIndicatorSemanticRequest request = semanticRequest (1, 1, 0);
            adk::SmallIndicatorObservation     observed =
                observation (1, adk::SmallIndicatorObservationState::Inactive, 0);
            observed.safeStateObserved = true;

            require (policy
                         .apply (driveIntent (value, mapping.driveState,
                                              mapping.driveReason),
                                 request, observed,

                                 adk::TimePoint (110), output)
                         .ok (),
                     "supported drive mapping applies");
            require (output.disposition == mapping.disposition &&
                         output.reason == mapping.reason,
                     "drive mapping is exact");
        }
    }

    adk::SmallIndicatorDescriptor kindDescriptor (adk::SmallIndicatorKind kind)
    {
        adk::SmallIndicatorDescriptor value = descriptor ();
        value.kind                          = kind;
        if (kind == adk::SmallIndicatorKind::ActiveBuzzer)
        {
            value.declaredChannelMask       = adk::SmallIndicatorChannels::Sound;
            value.populatedResistorDeclared = false;
            value.populatedDriverDeclared   = true;
        }
        else if (kind == adk::SmallIndicatorKind::DualColorLed)
        {
            value.declaredChannelMask =
                adk::SmallIndicatorChannels::Red | adk::SmallIndicatorChannels::Green;
        }
        else if (kind == adk::SmallIndicatorKind::AutoFlashLed)
        {
            value.autonomy  = adk::SmallIndicatorAutonomy::AutonomousWhileEnabled;
            value.safeState = adk::SmallIndicatorSafeState::UnpoweredRequired;
            value.declaredChannelMask = adk::SmallIndicatorChannels::Red |
                                        adk::SmallIndicatorChannels::Green |
                                        adk::SmallIndicatorChannels::Blue;
        }
        else if (kind == adk::SmallIndicatorKind::VoltageIndicator)
        {
            value.autonomy  = adk::SmallIndicatorAutonomy::ObservationOnly;
            value.safeState = adk::SmallIndicatorSafeState::HighImpedanceRequired;
            value.declaredChannelMask = adk::SmallIndicatorChannels::Voltage;
        }
        return value;
    }

    void testAllConcreteKinds ()
    {
        const adk::SmallIndicatorKind kinds[] = {
            adk::SmallIndicatorKind::ActiveBuzzer,
            adk::SmallIndicatorKind::TrafficLight,
            adk::SmallIndicatorKind::DualColorLed,
            adk::SmallIndicatorKind::AutoFlashLed,
            adk::SmallIndicatorKind::VoltageIndicator};

        for (adk::SmallIndicatorKind kind : kinds)
        {
            const adk::SmallIndicatorDescriptor value = kindDescriptor (kind);

            adk::SmallIndicatorSemanticsPolicy policy (value);

            startPolicy (policy);

            adk::LowSideDriveIntent drive = driveIntent (value);

            adk::SmallIndicatorSemanticRequest request = semanticRequest ();

            adk::SmallIndicatorObservation observed = observation ();

            if (kind == adk::SmallIndicatorKind::ActiveBuzzer)
            {
                request.selectedActiveMask  = adk::SmallIndicatorChannels::Sound;
                observed.observedActiveMask = adk::SmallIndicatorChannels::Sound;
                observed.copiedLevelHigh    = true;
            }
            else if (kind == adk::SmallIndicatorKind::DualColorLed)
            {
                request.selectedActiveMask  = adk::SmallIndicatorChannels::Red;
                observed.observedActiveMask = adk::SmallIndicatorChannels::Red;
            }
            else if (kind == adk::SmallIndicatorKind::AutoFlashLed)
            {
                request.selectedActiveMask = value.declaredChannelMask;
                observed.state = adk::SmallIndicatorObservationState::Alternating;
                observed.observedActiveMask = adk::SmallIndicatorChannels::Blue;
                observed.autonomousTransitionObserved = true;
            }
            else if (kind == adk::SmallIndicatorKind::VoltageIndicator)
            {
                request.selectedActiveMask = 0;
                drive = driveIntent (value, adk::LowSideDriveState::Off,
                                     adk::LowSideDriveReason::None);
                observed.observedActiveMask = adk::SmallIndicatorChannels::Voltage;
                observed.copiedLevelHigh    = true;
            }

            adk::SmallIndicatorSemanticResult output = canaryResult ();

            require (
                policy.apply (drive, request, observed, adk::TimePoint (110), output)
                    .ok (),
                "concrete kind applies");
            require (output.disposition == adk::SmallIndicatorDisposition::Accepted,
                     "concrete kind is accepted");
        }
    }

    void testCanonicalObservationFailures ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        for (uint16_t encoding = 0; encoding <= 255; ++encoding)
        {
            adk::SmallIndicatorSemanticsPolicy policy (value);

            startPolicy (policy);

            adk::SmallIndicatorObservation observed = observation ();
            observed.state =
                static_cast<adk::SmallIndicatorObservationState> (encoding);
            adk::SmallIndicatorSemanticResult output = canaryResult ();
            const adk::Status                 status =
                policy.apply (driveIntent (value), semanticRequest (), observed,
                              adk::TimePoint (110), output);
            if (encoding >= 5)
            {
                require (!status.ok (), "observation-state encodings are closed");
            }
        }

        adk::SmallIndicatorSemanticsPolicy mismatchPolicy (value);

        startPolicy (mismatchPolicy);

        adk::SmallIndicatorObservation mismatch  = observation ();
        mismatch.observedActiveMask              = adk::SmallIndicatorChannels::Red;
        adk::SmallIndicatorSemanticResult output = canaryResult ();

        require (mismatchPolicy
                     .apply (driveIntent (value), semanticRequest (), mismatch,
                             adk::TimePoint (110), output)
                     .ok (),
                 "color mismatch publishes result");
        require (output.reason == adk::SmallIndicatorReason::ObservationMismatch,
                 "color mismatch remains reachable");

        adk::SmallIndicatorSemanticsPolicy autonomyPolicy (value);

        startPolicy (autonomyPolicy);

        adk::SmallIndicatorObservation unexpected = observation ();
        unexpected.autonomousTransitionObserved   = true;
        require (autonomyPolicy
                     .apply (driveIntent (value), semanticRequest (), unexpected,
                             adk::TimePoint (110), output)
                     .ok (),
                 "unexpected autonomy publishes result");
        require (output.reason == adk::SmallIndicatorReason::UnexpectedAutonomy,
                 "unexpected autonomy remains reachable");

        const adk::SmallIndicatorDescriptor buzzerValue =
            kindDescriptor (adk::SmallIndicatorKind::ActiveBuzzer);
        adk::SmallIndicatorSemanticsPolicy polarityPolicy (buzzerValue);

        startPolicy (polarityPolicy);

        adk::SmallIndicatorSemanticRequest soundRequest = semanticRequest ();
        soundRequest.selectedActiveMask         = adk::SmallIndicatorChannels::Sound;
        adk::SmallIndicatorObservation lowSound = observation ();
        lowSound.observedActiveMask             = adk::SmallIndicatorChannels::Sound;
        lowSound.copiedLevelHigh                = false;
        require (polarityPolicy
                     .apply (driveIntent (buzzerValue), soundRequest, lowSound,
                             adk::TimePoint (110), output)
                     .ok (),
                 "polarity mismatch publishes result");
        require (output.reason == adk::SmallIndicatorReason::PolarityMismatch,
                 "single-channel polarity mismatch remains reachable");

        adk::SmallIndicatorSemanticsPolicy missingPolicy (value);

        startPolicy (missingPolicy);
        adk::SmallIndicatorObservation missing =
            observation (1, adk::SmallIndicatorObservationState::NotObserved, 0);
        require (missingPolicy
                     .apply (driveIntent (value), semanticRequest (), missing,
                             adk::TimePoint (110), output)
                     .ok (),
                 "missing observation publishes result");
        require (output.disposition == adk::SmallIndicatorDisposition::Incomplete &&
                     output.reason == adk::SmallIndicatorReason::ObservationMissing,
                 "not-observed has distinct missing reason");

        adk::SmallIndicatorSemanticsPolicy safePolicy (value);

        startPolicy (safePolicy);

        adk::LowSideDriveIntent off = driveIntent (value, adk::LowSideDriveState::Off,
                                                   adk::LowSideDriveReason::None);
        adk::SmallIndicatorSemanticRequest offRequest = semanticRequest (1, 1, 0);
        adk::SmallIndicatorObservation     inactive =
            observation (1, adk::SmallIndicatorObservationState::Inactive, 0);
        require (
            safePolicy.apply (off, offRequest, inactive, adk::TimePoint (110), output)
                .ok (),
            "missing safe state publishes result");
        require (output.reason == adk::SmallIndicatorReason::SafeStateMismatch,
                 "safe-state mismatch remains distinct");
    }

    void testSequenceTimeAndCancel ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        adk::SmallIndicatorSemanticsPolicy policy (value);

        startPolicy (policy);

        adk::SmallIndicatorSemanticResult output = canaryResult ();

        require (policy
                     .apply (driveIntent (value), semanticRequest (2, 1),
                             observation (), adk::TimePoint (110), output)
                     .ok (),
                 "sequence gap publishes result");
        require (output.reason == adk::SmallIndicatorReason::SequenceDiscontinuity,
                 "sequence gap has exact reason");

        require (policy.reset ().ok (), "reset after discontinuity succeeds");
        require (policy.beginSession (7, 9, adk::TimePoint (90)).ok (),
                 "session restarts after reset");

        adk::SmallIndicatorSemanticRequest request = semanticRequest ();
        request.lifecycleGeneration                = 2;
        adk::SmallIndicatorObservation observed    = observation ();
        observed.lifecycleGeneration               = 2;
        adk::LowSideDriveIntent drive              = driveIntent (value);
        drive.lifecycleGeneration                  = 2;
        require (
            policy.apply (drive, request, observed, adk::TimePoint (211), output).ok (),
            "stale evidence publishes result");
        require (output.reason == adk::SmallIndicatorReason::Stale,
                 "one-over freshness rejects");

        require (policy.reset ().ok (), "second reset succeeds");
        require (policy.beginSession (7, 9, adk::TimePoint (90)).ok (),
                 "third session begins");
        adk::SmallIndicatorControl cancelled = control (1);
        cancelled.lifecycleGeneration        = 3;
        require (policy.cancel (cancelled, output).ok (), "confirmed cancel succeeds");
        require (output.disposition == adk::SmallIndicatorDisposition::Cancelled &&
                     output.reason == adk::SmallIndicatorReason::Cancelled &&
                     output.safeStateSatisfied,
                 "confirmed cancel reports safe cancellation");
        const adk::SmallIndicatorSemanticResult prior = output;
        require (policy.cancel (cancelled, output).ok (),
                 "identical cancel is idempotent");
        require (equalResult (prior, output), "duplicate cancel is identical");

        adk::SmallIndicatorControl changed = cancelled;
        ++changed.controlId;
        output = canaryResult ();

        require (policy.cancel (changed, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed cancel duplicate rejects");
        require (equalResult (output, canaryResult ()),
                 "changed cancel preserves output");

        adk::SmallIndicatorSemanticsPolicy failedPolicy (value);

        startPolicy (failedPolicy);
        adk::SmallIndicatorControl failed =
            control (1, false, adk::StatusCode::HardwareFailure);
        require (failedPolicy.cancel (failed, output).ok (),
                 "failed safe return publishes result");
        require (output.disposition == adk::SmallIndicatorDisposition::ProducerFault &&
                     output.reason == adk::SmallIndicatorReason::Cancelled &&
                     !output.safeStateSatisfied &&
                     output.producerStatus == adk::StatusCode::HardwareFailure,
                 "cancel failure preserves cause and fault");
    }

    void testStatusEncodingsAndMalformedCancel ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        for (uint16_t encoding = 11; encoding <= 255; ++encoding)
        {
            adk::SmallIndicatorSemanticsPolicy drivePolicy (value);

            startPolicy (drivePolicy);

            adk::LowSideDriveIntent drive = driveIntent (value);
            drive.producerStatus          = static_cast<adk::StatusCode> (encoding);
            adk::SmallIndicatorSemanticResult output = canaryResult ();

            require (drivePolicy
                             .apply (drive, semanticRequest (), observation (),
                                     adk::TimePoint (110), output)
                             .error () == adk::StatusCode::InvalidArgument,
                     "invalid drive status encoding rejects");
            require (equalResult (output, canaryResult ()),
                     "invalid drive status preserves output");

            adk::SmallIndicatorSemanticsPolicy requestPolicy (value);

            startPolicy (requestPolicy);

            adk::SmallIndicatorSemanticRequest request = semanticRequest ();
            request.producerStatus = static_cast<adk::StatusCode> (encoding);
            require (requestPolicy
                             .apply (driveIntent (value), request, observation (),
                                     adk::TimePoint (110), output)
                             .error () == adk::StatusCode::InvalidArgument,
                     "invalid request status encoding rejects");

            adk::SmallIndicatorSemanticsPolicy observationPolicy (value);

            startPolicy (observationPolicy);

            adk::SmallIndicatorObservation observed = observation ();
            observed.producerStatus = static_cast<adk::StatusCode> (encoding);
            require (observationPolicy
                             .apply (driveIntent (value), semanticRequest (), observed,
                                     adk::TimePoint (110), output)
                             .error () == adk::StatusCode::InvalidArgument,
                     "invalid observation status encoding rejects");

            adk::SmallIndicatorSemanticsPolicy cancelPolicy (value);

            startPolicy (cancelPolicy);
            adk::SmallIndicatorControl cancelled =
                control (1, false, static_cast<adk::StatusCode> (encoding));
            require (cancelPolicy.cancel (cancelled, output).error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid cancel status encoding rejects");
        }

        adk::SmallIndicatorSemanticsPolicy falseOkPolicy (value);

        startPolicy (falseOkPolicy);

        adk::SmallIndicatorSemanticResult output = canaryResult ();

        require (falseOkPolicy.cancel (control (1, false), output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "unconfirmed OK cancel is malformed");
        require (equalResult (output, canaryResult ()),
                 "malformed cancel preserves output");

        adk::SmallIndicatorSemanticsPolicy trueFaultPolicy (value);

        startPolicy (trueFaultPolicy);

        require (trueFaultPolicy
                         .cancel (control (1, true, adk::StatusCode::HardwareFailure),
                                  output)
                         .error () == adk::StatusCode::InvalidArgument,
                 "confirmed failed cancel is malformed");
    }

    void testTerminalAndAuthorityRegressions ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        adk::SmallIndicatorSemanticsPolicy duplicatePolicy (value);

        startPolicy (duplicatePolicy);

        adk::SmallIndicatorSemanticResult output = canaryResult ();

        require (duplicatePolicy
                     .apply (driveIntent (value), semanticRequest (), observation (),
                             adk::TimePoint (110), output)
                     .ok (),
                 "duplicate fixture first apply succeeds");

        const adk::SmallIndicatorSemanticResult accepted = output;
        adk::SmallIndicatorSemanticRequest      changed  = semanticRequest ();
        changed.selectedActiveMask = adk::SmallIndicatorChannels::Red;
        output                     = canaryResult ();

        require (duplicatePolicy
                         .apply (driveIntent (value), changed, observation (),
                                 adk::TimePoint (110), output)
                         .error () == adk::StatusCode::InvalidArgument,
                 "fieldwise changed duplicate rejects");
        require (equalResult (accepted, duplicatePolicy.snapshot ()),
                 "changed duplicate preserves accepted snapshot");
        require (equalResult (output, canaryResult ()),
                 "changed duplicate preserves caller output");

        adk::SmallIndicatorSemanticsPolicy terminalPolicy (value);

        startPolicy (terminalPolicy);

        require (terminalPolicy
                     .apply (driveIntent (value), semanticRequest (2, 1),
                             observation (), adk::TimePoint (110), output)
                     .ok (),
                 "gap publishes terminal result");
        const adk::SmallIndicatorSemanticResult terminal = terminalPolicy.snapshot ();

        require (terminal.reason == adk::SmallIndicatorReason::SequenceDiscontinuity,
                 "gap terminal reason");
        require (terminalPolicy
                         .apply (driveIntent (value), semanticRequest (),
                                 observation (), adk::TimePoint (110), output)
                         .error () == adk::StatusCode::InvalidArgument,
                 "terminal policy admits no recovery apply");
        require (equalResult (terminal, terminalPolicy.snapshot ()),
                 "terminal rejection remains latched");

        adk::SmallIndicatorSemanticsPolicy cancelAuthorityPolicy (value);

        startPolicy (cancelAuthorityPolicy);

        require (cancelAuthorityPolicy
                     .apply (driveIntent (value), semanticRequest (), observation (),
                             adk::TimePoint (110), output)
                     .ok (),
                 "authority fixture applies");
        adk::SmallIndicatorControl mismatchedSource = control (2);
        ++mismatchedSource.sourceId;
        require (cancelAuthorityPolicy.cancel (mismatchedSource, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "cancel cannot borrow a different source");

        cancelAuthorityPolicy.shutdown ();

        require (cancelAuthorityPolicy.initialize ().ok (),
                 "reinitialize after authority latch succeeds");

        require (cancelAuthorityPolicy
                     .beginSession (7, 9,
                                    adk::TimePoint (130))
                     .ok (),
                 "session restarts after authority latch");

        adk::SmallIndicatorControl freshSource = control (1);
        freshSource.lifecycleGeneration         = 2;
        freshSource.sourceId                    = 8;
        freshSource.sourceConfigurationRevision = 9;
        freshSource.observedAt                   = adk::TimePoint (130);

        require (cancelAuthorityPolicy.cancel (freshSource, output).ok (),
                 "reinitialize clears prior source authority");

        adk::SmallIndicatorSemanticsPolicy safeShapePolicy (value);

        startPolicy (safeShapePolicy);

        adk::SmallIndicatorObservation activeSafe = observation ();
        activeSafe.safeStateObserved              = true;
        output                                    = canaryResult ();

        require (safeShapePolicy
                         .apply (driveIntent (value), semanticRequest (), activeSafe,
                                 adk::TimePoint (110), output)
                         .error () == adk::StatusCode::InvalidArgument,
                 "active observation cannot claim safe state");

        adk::SmallIndicatorSemanticsPolicy selectedMaskPolicy (value);

        startPolicy (selectedMaskPolicy);

        adk::SmallIndicatorSemanticRequest twoColors = semanticRequest ();
        twoColors.selectedActiveMask =
            adk::SmallIndicatorChannels::Red | adk::SmallIndicatorChannels::Green;
        require (selectedMaskPolicy
                         .apply (driveIntent (value), twoColors, observation (),
                                 adk::TimePoint (110), output)
                         .error () == adk::StatusCode::InvalidArgument,
                 "follows-drive request selects exactly one channel");

        const adk::SmallIndicatorDescriptor autonomousValue =
            kindDescriptor (adk::SmallIndicatorKind::AutoFlashLed);
        adk::SmallIndicatorSemanticsPolicy autonomousPolicy (autonomousValue);

        startPolicy (autonomousPolicy);

        adk::LowSideDriveIntent autonomousOff =
            driveIntent (autonomousValue, adk::LowSideDriveState::Off,
                         adk::LowSideDriveReason::None);
        adk::SmallIndicatorSemanticRequest autonomousRequest = semanticRequest ();
        autonomousRequest.selectedActiveMask                 = 0;
        adk::SmallIndicatorObservation autonomousObserved    = observation ();
        autonomousObserved.state = adk::SmallIndicatorObservationState::Alternating;
        autonomousObserved.observedActiveMask = adk::SmallIndicatorChannels::Blue;
        autonomousObserved.autonomousTransitionObserved = true;
        require (autonomousPolicy
                     .apply (autonomousOff, autonomousRequest, autonomousObserved,
                             adk::TimePoint (110), output)
                     .ok (),
                 "autonomous-off disagreement publishes result");
        require (output.reason == adk::SmallIndicatorReason::UnexpectedAutonomy,
                 "autonomous activity while drive is off is explicit");
    }

    void testThreeStreamsAndTimeBoundaries ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        for (uint8_t stream = 0; stream < 3; ++stream)
        {
            adk::SmallIndicatorSemanticsPolicy policy (value);

            startPolicy (policy);

            adk::LowSideDriveIntent drive = driveIntent (value);

            adk::SmallIndicatorSemanticRequest request = semanticRequest ();

            adk::SmallIndicatorObservation observed = observation ();
            if (stream == 0)
            {
                request.policySequence = 2;
            }
            else if (stream == 1)
            {
                request.requestSequence = 2;
            }
            else
            {
                observed.observationSequence = 2;
            }

            adk::SmallIndicatorSemanticResult output = canaryResult ();

            require (
                policy.apply (drive, request, observed, adk::TimePoint (110), output)
                    .ok (),
                "stream gap publishes result");
            require (output.reason == adk::SmallIndicatorReason::SequenceDiscontinuity,
                     "every stream gap is explicit");
        }

        adk::SmallIndicatorSemanticsPolicy sequencePolicy (value);

        startPolicy (sequencePolicy);

        adk::LowSideDriveIntent drive = driveIntent (value);

        adk::SmallIndicatorSemanticRequest request = semanticRequest ();
        adk::SmallIndicatorObservation     observed =
            observation (1, adk::SmallIndicatorObservationState::NotObserved, 0);
        adk::SmallIndicatorSemanticResult output = canaryResult ();

        require (sequencePolicy
                     .apply (drive, request, observed, adk::TimePoint (110), output)

                     .ok (),
                 "sequence fixture first apply");
        advanceTuple (drive, request, observed, 2, 2, 2, 120);
        observed.state              = adk::SmallIndicatorObservationState::Active;
        observed.observedActiveMask = adk::SmallIndicatorChannels::Green;
        require (sequencePolicy
                     .apply (drive, request, observed, adk::TimePoint (120), output)

                     .ok (),
                 "three contiguous streams advance");

        adk::SmallIndicatorDescriptor ageValue = descriptor ();

        ageValue.maximumObservationAge = adk::Duration (10);

        adk::SmallIndicatorSemanticsPolicy agePolicy (ageValue);

        startPolicy (agePolicy);

        require (agePolicy
                     .apply (driveIntent (ageValue), semanticRequest (), observation (),
                             adk::TimePoint (120), output)
                     .ok (),
                 "freshness equality applies");
        require (output.disposition == adk::SmallIndicatorDisposition::Accepted,
                 "freshness maximum is inclusive");

        adk::SmallIndicatorSemanticsPolicy stalePolicy (ageValue);

        startPolicy (stalePolicy);

        require (stalePolicy
                     .apply (driveIntent (ageValue), semanticRequest (), observation (),
                             adk::TimePoint (121), output)
                     .ok (),
                 "freshness one-over publishes");
        require (output.reason == adk::SmallIndicatorReason::Stale,
                 "freshness one-over is stale");

        adk::SmallIndicatorDescriptor warmValue = descriptor ();

        warmValue.warmup = adk::Duration (21);

        adk::SmallIndicatorSemanticsPolicy warmPolicy (warmValue);

        startPolicy (warmPolicy);

        require (warmPolicy
                     .apply (driveIntent (warmValue), semanticRequest (),
                             observation (), adk::TimePoint (110), output)
                     .ok (),
                 "warmup shortfall publishes");
        require (output.reason == adk::SmallIndicatorReason::WarmupUnsatisfied,
                 "warmup shortfall remains distinct");

        adk::SmallIndicatorDescriptor settleValue = descriptor ();

        settleValue.settling = adk::Duration (11);

        adk::SmallIndicatorSemanticsPolicy settlePolicy (settleValue);

        startPolicy (settlePolicy);

        require (settlePolicy
                     .apply (driveIntent (settleValue), semanticRequest (),
                             observation (), adk::TimePoint (110), output)
                     .ok (),
                 "settling shortfall publishes");
        require (output.reason == adk::SmallIndicatorReason::SettlingUnsatisfied,
                 "settling shortfall remains distinct");

        adk::SmallIndicatorSemanticsPolicy halfRangePolicy (value);

        startPolicy (halfRangePolicy);

        adk::SmallIndicatorObservation halfRange = observation ();

        halfRange.observedAt = adk::TimePoint (0x80000064UL);

        require (halfRangePolicy
                     .apply (driveIntent (value), semanticRequest (), halfRange,
                             adk::TimePoint (0x80000064UL), output)
                     .ok (),
                 "half-range chronology publishes");
        require (output.reason == adk::SmallIndicatorReason::TimestampDiscontinuity,
                 "half-range chronology is explicit");
    }

    bool validDriveTuple (adk::LowSideDriveState state, adk::LowSideDriveReason reason,
                          bool logicalActive, bool outputLevelHigh, adk::Status status)
    {
        const bool off = !logicalActive && !outputLevelHigh;
        if (state == adk::LowSideDriveState::Off)
        {
            return off && status.ok () &&
                   (reason == adk::LowSideDriveReason::None ||
                    reason == adk::LowSideDriveReason::Expired);
        }
        if (state == adk::LowSideDriveState::Requested)
        {
            return reason == adk::LowSideDriveReason::None && logicalActive &&
                   outputLevelHigh && status.ok ();
        }
        if (state == adk::LowSideDriveState::Rejected)
        {
            const bool mapped =
                reason == adk::LowSideDriveReason::SourceIneligible ||
                reason == adk::LowSideDriveReason::BudgetExceeded ||
                reason == adk::LowSideDriveReason::BaseBudgetInsufficient ||
                reason == adk::LowSideDriveReason::FlybackMissing ||
                reason == adk::LowSideDriveReason::SequenceDiscontinuity ||
                reason == adk::LowSideDriveReason::TimestampDiscontinuity ||
                reason == adk::LowSideDriveReason::CapacityExceeded;
            return off && mapped &&
                   (status.ok () ||
                    reason == adk::LowSideDriveReason::SequenceDiscontinuity ||
                    reason == adk::LowSideDriveReason::TimestampDiscontinuity);
        }
        if (state == adk::LowSideDriveState::Cancelled)
        {
            return off && reason == adk::LowSideDriveReason::Cancelled && status.ok ();
        }
        if (state == adk::LowSideDriveState::Fault)
        {
            return off && !status.ok () &&
                   (reason == adk::LowSideDriveReason::ProducerFault ||
                    reason == adk::LowSideDriveReason::Cancelled);
        }
        return state == adk::LowSideDriveState::Shutdown && off &&
               reason == adk::LowSideDriveReason::None && status.ok ();
    }

    void testExhaustiveDriveShapeCrossProduct ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        for (uint8_t stateEncoding = 0; stateEncoding < 6; ++stateEncoding)
        {
            for (uint8_t reasonEncoding = 0; reasonEncoding < 11; ++reasonEncoding)
            {
                for (uint8_t activity = 0; activity < 2; ++activity)
                {
                    for (uint8_t level = 0; level < 2; ++level)
                    {
                        for (uint8_t statusEncoding = 0; statusEncoding <= 10;
                             ++statusEncoding)
                        {
                            const adk::LowSideDriveState state =
                                static_cast<adk::LowSideDriveState> (stateEncoding);
                            const adk::LowSideDriveReason reason =
                                static_cast<adk::LowSideDriveReason> (reasonEncoding);
                            const adk::Status status =
                                static_cast<adk::StatusCode> (statusEncoding);
                            adk::LowSideDriveIntent drive =
                                driveIntent (value, state, reason);
                            drive.logicalActive   = activity != 0;
                            drive.outputLevelHigh = level != 0;
                            drive.producerStatus  = status;

                            adk::SmallIndicatorSemanticsPolicy policy (value);

                            startPolicy (policy);

                            adk::SmallIndicatorSemanticResult  output = canaryResult ();

                            adk::SmallIndicatorSemanticRequest request =
                                semanticRequest ();

                            adk::SmallIndicatorObservation observed = observation ();
                            if (!drive.logicalActive)
                            {
                                request.selectedActiveMask = 0;
                                observed.state =
                                    adk::SmallIndicatorObservationState::Inactive;
                                observed.observedActiveMask = 0;
                                observed.safeStateObserved  = true;
                            }
                            const adk::Status applied = policy.apply (
                                drive, request, observed, adk::TimePoint (110), output);
                            require (applied.ok () ==
                                         validDriveTuple (state, reason, activity != 0,
                                                          level != 0, status),
                                     "drive state/reason/activity/level/status "
                                     "cross-product");
                        }
                    }
                }
            }
        }
    }

    void testProducerFailurePrecedenceCollisions ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        for (uint8_t failures = 1; failures < 8; ++failures)
        {
            adk::SmallIndicatorSemanticsPolicy policy (value);

            startPolicy (policy);

            adk::LowSideDriveIntent drive = driveIntent (value);

            adk::SmallIndicatorSemanticRequest request = semanticRequest ();

            adk::SmallIndicatorObservation observed = observation ();

            if ((failures & 1U) != 0)
            {
                request.producerStatus = adk::StatusCode::InvalidArgument;
            }
            if ((failures & 2U) != 0)
            {
                drive = driveIntent (value, adk::LowSideDriveState::Fault,
                                     adk::LowSideDriveReason::ProducerFault);
                drive.producerStatus       = adk::StatusCode::HardwareFailure;
                request.selectedActiveMask = 0;
                observed.state = adk::SmallIndicatorObservationState::Inactive;
                observed.observedActiveMask = 0;
                observed.safeStateObserved  = true;
            }
            if ((failures & 4U) != 0)
            {
                observed.state = adk::SmallIndicatorObservationState::Fault;
                observed.observedActiveMask = 0;
                observed.safeStateObserved  = false;
                observed.producerStatus     = adk::StatusCode::Timeout;
            }

            adk::SmallIndicatorSemanticResult output = canaryResult ();

            require (
                policy.apply (drive, request, observed, adk::TimePoint (110), output)
                    .ok (),
                "producer-failure collision publishes");
            const adk::Status expected = (failures & 1U) != 0 ? request.producerStatus
                                         : (failures & 2U) != 0
                                             ? drive.producerStatus
                                             : observed.producerStatus;
            require (output.disposition ==
                             adk::SmallIndicatorDisposition::ProducerFault &&
                         output.reason == adk::SmallIndicatorReason::ProducerFault &&
                         output.producerStatus.error () == expected.error (),
                     "producer precedence is request then drive then observation");
        }
    }

    void testShutdownFromEveryLifecycleState ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        for (uint8_t state = 0; state < 7; ++state)
        {
            adk::SmallIndicatorSemanticsPolicy policy (value);

            adk::SmallIndicatorSemanticResult output = canaryResult ();

            if (state != 0)
            {
                require (policy.initialize ().ok (), "shutdown fixture initializes");
            }
            if (state >= 2)
            {
                require (policy.beginSession (7, 9, adk::TimePoint (90)).ok (),
                         "shutdown fixture starts");
            }
            if (state == 3)
            {
                adk::SmallIndicatorDescriptor warmValue = value;
                warmValue.warmup                        = adk::Duration (100);

                adk::SmallIndicatorSemanticsPolicy warmPolicy (warmValue);

                startPolicy (warmPolicy);

                require (warmPolicy
                             .apply (driveIntent (warmValue), semanticRequest (),
                                     observation (), adk::TimePoint (110), output)
                             .ok (),
                         "incomplete shutdown fixture applies");
                warmPolicy.shutdown ();

                require (!warmPolicy.initialized () &&
                             warmPolicy.snapshot ().disposition ==
                                 adk::SmallIndicatorDisposition::Shutdown,
                         "shutdown closes incomplete state");
                continue;
            }
            if (state == 4)
            {
                require (policy
                             .apply (driveIntent (value), semanticRequest (),
                                     observation (), adk::TimePoint (110), output)
                             .ok (),
                         "accepted shutdown fixture applies");
            }
            else if (state == 5)
            {
                require (policy.cancel (control (1), output).ok (),
                         "cancelled shutdown fixture applies");
            }
            else if (state == 6)
            {
                require (policy
                             .apply (driveIntent (value), semanticRequest (2, 1),
                                     observation (), adk::TimePoint (110), output)
                             .ok (),
                         "discontinuity shutdown fixture applies");
            }

            const adk::SmallIndicatorSemanticResult beforeShutdown = policy.snapshot ();

            policy.shutdown ();

            require (!policy.initialized (), "shutdown leaves every state inactive");
            require (
                state == 0 ? equalResult (beforeShutdown, policy.snapshot ())

                           : policy.snapshot ().disposition ==
                                 adk::SmallIndicatorDisposition::Shutdown,
                "shutdown is a no-op before initialize and closes active lifecycles");
        }
    }

#if defined(ADK_TESTING)
    void testStreamRegressionExhaustionAndWrap ()
    {
        adk::SmallIndicatorDescriptor value = descriptor ();

        value.warmup = adk::Duration (1000);

        for (uint8_t stream = 0; stream < 3; ++stream)
        {
            adk::SmallIndicatorSemanticsPolicy regressionPolicy (value);

            startPolicy (regressionPolicy);

            regressionPolicy.seedSequencesForTest (2, 2, 2);

            adk::LowSideDriveIntent            drive    = driveIntent (value);

            adk::SmallIndicatorSemanticRequest request  = semanticRequest (3, 3);

            adk::SmallIndicatorObservation     observed = observation (3);
            if (stream == 0)
            {
                request.policySequence = 1;
            }
            else if (stream == 1)
            {
                request.requestSequence = 1;
            }
            else
            {
                observed.observationSequence = 1;
            }

            adk::SmallIndicatorSemanticResult output = canaryResult ();

            require (regressionPolicy
                         .apply (drive, request, observed, adk::TimePoint (110), output)

                         .ok (),
                     "stream regression publishes terminal result");
            require (output.reason == adk::SmallIndicatorReason::SequenceDiscontinuity,
                     "each stream regression is explicit");

            adk::SmallIndicatorSemanticsPolicy exhaustionPolicy (value);

            startPolicy (exhaustionPolicy);

            exhaustionPolicy.seedSequencesForTest (UINT32_MAX, UINT32_MAX, UINT32_MAX);

            request = semanticRequest (UINT32_MAX, UINT32_MAX);

            observed = observation (UINT32_MAX);
            if (stream == 0)
            {
                request.policySequence = 0;
            }
            else if (stream == 1)
            {
                request.requestSequence = 0;
            }
            else
            {
                observed.observationSequence = 0;
            }
            output = canaryResult ();

            require (exhaustionPolicy
                             .apply (driveIntent (value), request, observed,
                                     adk::TimePoint (110), output)
                             .error () == adk::StatusCode::InvalidArgument,
                     "stream exhaustion cannot wrap through reserved zero");
            require (equalResult (output, canaryResult ()),
                     "stream exhaustion preserves caller output");

            adk::SmallIndicatorSemanticsPolicy maximumPolicy (value);

            startPolicy (maximumPolicy);

            maximumPolicy.seedSequencesForTest (UINT32_MAX - 1U, UINT32_MAX - 1U,
                                                UINT32_MAX - 1U);
            request = semanticRequest (UINT32_MAX, UINT32_MAX);

            observed = observation (UINT32_MAX);

            require (maximumPolicy
                         .apply (driveIntent (value), request, observed,
                                 adk::TimePoint (110), output)
                         .ok (),
                     "maximum nonzero stream values are admitted contiguously");
            require (output.reason == adk::SmallIndicatorReason::WarmupUnsatisfied,
                     "maximum sequence tuple remains nonterminal when incomplete");
        }

        adk::SmallIndicatorSemanticsPolicy wrapPolicy (value);

        require (wrapPolicy.initialize ().ok (), "wrap policy initializes");
        require (wrapPolicy.beginSession (7, 9, adk::TimePoint (0xFFFFFFF0UL)).ok (),
                 "wrap session begins");
        adk::SmallIndicatorSemanticRequest wrapRequest = semanticRequest ();

        wrapRequest.requestedAt = adk::TimePoint (0xFFFFFFF5UL);

        adk::SmallIndicatorObservation wrapObservation = observation ();

        wrapObservation.observedAt = adk::TimePoint (5);

        adk::SmallIndicatorSemanticResult output = canaryResult ();

        require (wrapPolicy
                     .apply (driveIntent (value), wrapRequest, wrapObservation,
                             adk::TimePoint (10), output)
                     .ok (),
                 "chronology crosses wrap");
        require (output.reason == adk::SmallIndicatorReason::WarmupUnsatisfied,
                 "valid chronology wrap is not a discontinuity");
    }

    void testGenerationExhaustion ()
    {
        const adk::SmallIndicatorDescriptor value = descriptor ();

        adk::SmallIndicatorSemanticsPolicy policy (value);

        policy.seedLifecycleGenerationForTest (UINT32_MAX);

        const adk::SmallIndicatorSemanticResult before = policy.snapshot ();

        require (policy.initialize ().error () == adk::StatusCode::CapacityExceeded,
                 "generation exhaustion rejects initialize");
        require (equalResult (before, policy.snapshot ()),
                 "generation exhaustion is atomic");
    }
#endif

} // namespace

int main ()
{
    testChannelConstants ();

    testExactValidityTable ();

    testEveryEnumEncoding ();

    testIdentityAndDurationLimits ();

    testLifecycleAndHappyPath ();

    testCorrelationAndAtomicRejection ();

    testDriveMappings ();

    testAllConcreteKinds ();

    testCanonicalObservationFailures ();

    testSequenceTimeAndCancel ();

    testStatusEncodingsAndMalformedCancel ();

    testTerminalAndAuthorityRegressions ();

    testThreeStreamsAndTimeBoundaries ();

    testExhaustiveDriveShapeCrossProduct ();

    testProducerFailurePrecedenceCollisions ();

    testShutdownFromEveryLifecycleState ();

#if defined(ADK_TESTING)
    testStreamRegressionExhaustionAndWrap ();

    testGenerationExhaustion ();
#endif

    std::cout << "small indicator semantics policy tests passed\n";
    return EXIT_SUCCESS;
}
