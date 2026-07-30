#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct LowSideLoadEnergy : uint8_t
    {
        ResistiveIndicator,
        InductiveInert
    };

    enum struct LowSideFlybackRequirement : uint8_t
    {
        NotRequired,
        Required
    };

    enum struct LowSideFlybackDeclaration : uint8_t
    {
        Absent,
        Present
    };

    enum struct LowSideDriveState : uint8_t
    {
        Off,
        Requested,
        Rejected,
        Cancelled,
        Fault,
        Shutdown
    };

    enum struct LowSideDriveReason : uint8_t
    {
        None,
        SourceIneligible,
        BudgetExceeded,
        BaseBudgetInsufficient,
        FlybackMissing,
        ProducerFault,
        SequenceDiscontinuity,
        TimestampDiscontinuity,
        Expired,
        CapacityExceeded,
        Cancelled
    };

    struct LowSideCurrentBudget
    {
        uint32_t supplyLimitUa;
        uint32_t reservedSupplyUa;
        uint32_t fixtureLoadCeilingUa;
        uint32_t deviceContinuousUa;
        uint32_t gpioSourceCeilingUa;
        uint32_t forcedGainNumerator;
        uint32_t forcedGainDenominator;
        uint32_t baseResistanceOhms;
        uint16_t baseResistanceTolerancePermille;
        uint16_t logicHighMinimumMv;
        uint16_t logicHighMaximumMv;
        uint16_t baseEmitterMaximumMv;
        uint16_t collectorEmitterOperatingMaximumMv;
        Duration maximumActiveDuration;
        Duration dutyWindow;
        uint16_t maximumDutyPermille;
    };

    struct LowSideDriverDescriptor
    {
        uint16_t                  schemaRevision;
        uint32_t                  specimenFamilyReference;
        uint32_t                  specimenReference;
        uint16_t                  specimenRevision;
        uint16_t                  electricalEvidenceRevision;
        uint32_t                  sourcePacketDigest;
        uint32_t                  configurationId;
        uint16_t                  configurationRevision;
        LowSideLoadEnergy         loadEnergy;
        LowSideFlybackRequirement flybackRequirement;
        LowSideFlybackDeclaration flybackDeclaration;
        bool                      sourceEligible;
        uint32_t                  flybackDiodeIdentity;
        uint16_t                  flybackDiodeRevision;
        uint8_t                   flybackOrientationCode;
        uint8_t                   flybackReturnCode;
        uint32_t                  flybackRepetitiveReverseMv;
        uint32_t                  flybackForwardCurrentUa;
        LowSideCurrentBudget      budget;
    };

    struct LowSideDriveRequest
    {
        uint32_t  sessionId;
        uint32_t  runId;
        uint16_t  stepId;
        uint32_t  requestId;
        uint8_t   sourceId;
        uint16_t  sourceConfigurationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
        uint32_t  lifecycleGeneration;
        bool      logicalActive;
        uint32_t  requestedLoadUa;
        Duration  requestedActiveDuration;
        Status    producerStatus;
    };

    struct LowSideControl
    {
        uint32_t  lifecycleGeneration;
        uint32_t  sessionId;
        uint32_t  runId;
        uint16_t  stepId;
        uint32_t  controlId;
        uint8_t   sourceId;
        uint16_t  sourceConfigurationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
        bool      offConfirmed;
        Status    producerStatus;
    };

    struct LowSideDriveIntent
    {
        uint32_t           lifecycleGeneration;
        uint32_t           driverDescriptorIdentityDigest;
        uint32_t           specimenReference;
        uint16_t           specimenRevision;
        uint16_t           electricalEvidenceRevision;
        uint32_t           policyConfigurationId;
        uint16_t           policyConfigurationRevision;
        uint32_t           sessionId;
        uint32_t           runId;
        uint16_t           stepId;
        uint32_t           requestId;
        LowSideDriveState  state;
        LowSideDriveReason reason;
        bool               logicalActive;
        bool               outputLevelHigh;
        uint32_t           requiredBaseUa;
        uint32_t           admittedBaseUa;
        uint32_t           admittedLoadUa;
        TimePoint          expiresAt;
        Status             producerStatus;
    };

    struct BoundedLowSideDriverPolicy
    {
        explicit BoundedLowSideDriverPolicy (
            const LowSideDriverDescriptor& descriptor) noexcept;

        BoundedLowSideDriverPolicy (const BoundedLowSideDriverPolicy&) = delete;
        BoundedLowSideDriverPolicy&
        operator= (const BoundedLowSideDriverPolicy&)                        = delete;
        BoundedLowSideDriverPolicy (BoundedLowSideDriverPolicy&&)            = delete;
        BoundedLowSideDriverPolicy& operator= (BoundedLowSideDriverPolicy&&) = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

        Status beginSession (uint32_t sessionId, uint32_t runId) noexcept;
        Status apply        (const LowSideDriveRequest& request,
                             LowSideDriveIntent&        intent) noexcept;
        Status update       (TimePoint now, LowSideDriveIntent& intent) noexcept;
        Status cancel       (const LowSideControl& control,
                             LowSideDriveIntent&   intent) noexcept;
        Status reset        () noexcept;

        LowSideDriveIntent snapshot () const noexcept;

#if defined(ADK_TESTING)
        void seedLifecycleGenerationForTest (uint32_t generation) noexcept;
#endif

      private:
        struct Reservation
        {
            TimePoint start;
            TimePoint end;
        };

        LowSideDriverDescriptor descriptor_;
        LowSideDriveIntent      intent_;
        Reservation             reservations_[8];
        uint32_t                lifecycleGeneration_;
        uint32_t                lastSequence_;
        TimePoint               lastObservedAt_;
        uint32_t                lastRequestDigest_;
        uint8_t                 sourceId_;
        uint8_t                 reservationCount_;
        bool                    initialized_;
        bool                    hasRequest_;
        bool                    lastWasControl_;
        bool                    lastWasUpdate_;
        bool                    hasChronology_;
    };

    Status validateLowSideDriverDescriptor (
        const LowSideDriverDescriptor& descriptor) noexcept;
    uint32_t lowSideDriverDescriptorIdentityDigest (
        const LowSideDriverDescriptor& descriptor) noexcept;
} // namespace adk
