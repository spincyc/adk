#pragma once

#include "module_threshold_descriptor.h"

#include <stdint.h>

namespace adk {

    enum struct ModuleCharacterizationLeg : uint8_t
    {
        Ascending,
        Descending,
        Verification
    };

    enum struct ModuleSweepDirection : uint8_t
    {
        Increasing,
        Decreasing,
        Unordered
    };

    enum struct ModuleCharacterizationState : uint8_t
    {
        Idle,
        Collecting,
        Complete,
        Rejected,
        Shutdown
    };

    enum struct ModuleCharacterizationReason : uint8_t
    {
        None,
        WarmupUnsatisfied,
        SettlingUnsatisfied,
        ProducerFault,
        Stale,
        SequenceDiscontinuity,
        TimestampDiscontinuity,
        DirectionViolation,
        Chatter,
        NoObservedTransitionActive,
        NoObservedTransitionInactive,
        AtLowerRail,
        AtUpperRail,
        TransitionOrientationMismatch,
        AnalogComparatorDisagreement
    };

    struct ModuleCompactWitness
    {
        bool      present;
        uint16_t  controlOrdinal;
        uint16_t  analogRaw;
        bool      comparatorAsserted;
        uint32_t  sequence;
        TimePoint observedAt;
    };

    enum struct ModuleComparatorRelation : uint8_t
    {
        Unverified,
        Consistent,
        Ambiguous,
        Disagrees
    };

    struct ModuleCharacterizationPoint
    {
        uint32_t                  sessionId;
        uint32_t                  runId;
        uint32_t                  legId;
        uint16_t                  controlOrdinal;
        ModuleCharacterizationLeg leg;
        ModuleSweepDirection      direction;
        uint8_t                   sourceId;
        uint16_t                  sourceConfigurationRevision;
        ModuleThresholdFrame      frame;
    };

    struct ModuleTransitionBracket
    {
        bool                        present;
        ModuleCharacterizationPoint before;
        ModuleCharacterizationPoint after;
    };

    struct ModuleAnalogInterval
    {
        bool     present;
        uint16_t lower;
        uint16_t upper;
    };

    struct ModuleCharacterizationConfig
    {
        uint16_t                  characterizationRevision;
        ModuleThresholdDescriptor descriptor;
        uint8_t                   requiredPointsPerLeg;
        Duration                  maximumAge;
        Duration                  maximumGap;
    };

    struct ModuleCharacterizationEvidence
    {
        uint32_t                     lifecycleGeneration;
        uint32_t                     sessionId;
        uint32_t                     runId;
        uint32_t                     legId;
        uint16_t                     characterizationRevision;
        ModuleThresholdDescriptor    descriptor;
        uint8_t                      sourceId;
        uint16_t                     sourceConfigurationRevision;
        ModuleCharacterizationState  state;
        ModuleCharacterizationReason reason;
        ModuleCharacterizationLeg    terminalLeg;
        uint8_t                      ascendingCount;
        uint8_t                      descendingCount;
        uint8_t                      verificationCount;
        ModuleTransitionBracket      ascendingBracket;
        ModuleTransitionBracket      descendingBracket;
        ModuleAnalogInterval         guaranteedInactiveInterval;
        ModuleAnalogInterval         guaranteedActiveInterval;
        ModuleAnalogInterval         ambiguityInterval;
        ModuleComparatorRelation     relation;
        ModuleCompactWitness         firstWitness;
        ModuleCompactWitness         lastWitness;
        ModuleCompactWitness         offendingBefore;
        ModuleCompactWitness         offendingAfter;
        Status                       status;
    };

    struct ModuleCharacterizationPolicy
    {
        explicit ModuleCharacterizationPolicy (
            const ModuleCharacterizationConfig& config) noexcept;

        ModuleCharacterizationPolicy (const ModuleCharacterizationPolicy&) = delete;
        ModuleCharacterizationPolicy&
        operator= (const ModuleCharacterizationPolicy&)               = delete;
        ModuleCharacterizationPolicy (ModuleCharacterizationPolicy&&) = delete;
        ModuleCharacterizationPolicy&
        operator= (ModuleCharacterizationPolicy&&) = delete;

        Status initialize   (TimePoint now) noexcept;
        Status beginSession (TimePoint now, uint32_t sessionId,
                             uint32_t runId) noexcept;
        Status beginLeg     (TimePoint now, uint32_t legId,
                             ModuleCharacterizationLeg leg,
                             ModuleSweepDirection direction) noexcept;
        Status observe      (TimePoint                          now,
                             const ModuleCharacterizationPoint& point) noexcept;
        Status finalizeLeg  (TimePoint now) noexcept;
        Status reset        (TimePoint now) noexcept;
        Status shutdown     (TimePoint now) noexcept;
        Status evidence     (ModuleCharacterizationEvidence& output) const noexcept;

#if defined(ADK_TESTING)
        void seedLifecycleGenerationForTest (uint32_t generation) noexcept;
#endif

      private:
        ModuleCharacterizationConfig   config_;
        ModuleCharacterizationEvidence evidence_;
        ModuleCompactWitness           firstLegWitness_;
        ModuleCompactWitness           previousWitness_;
        ModuleCompactWitness           priorSourceWitness_;
        uint32_t                       lastSessionId_;
        uint32_t                       lastRunId_;
        uint32_t                       lastLegId_;
        TimePoint                      lastOperationAt_;
        bool                           initialized_;
        bool                           shutdown_;
        bool                           sessionActive_;
        bool                           legActive_;
        bool                           hasPreviousPoint_;
        bool                           hasPriorSourceWitness_;
        bool                           transitionSeen_;
        bool                           relationConsistent_;
        bool                           relationAmbiguous_;
    };
} // namespace adk
