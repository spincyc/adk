#pragma once

#include "pulse_input.h"

#include <stdint.h>

namespace adk {

    struct InertEscapeConsoleTestAccess;

    enum struct ClueCategory : uint8_t
    {
        Absent,
        Low,
        Nominal,
        High,
        Active,
        Inactive,
        Match,
        Mismatch
    };

    enum struct ClueQuality : uint8_t
    {
        Invalid,
        Qualified,
        Degraded,
        Stale,
        Contradictory,
        SourceFault,
        TimingFault
    };

    enum struct ClueTermRelation : uint8_t
    {
        Equals,
        NotEquals
    };

    enum struct ClueRuleDisposition : uint8_t
    {
        Unevaluated,
        BlockedByPrerequisite,
        MissingEvidence,
        InvalidEvidence,
        StaleEvidence,
        ContradictoryEvidence,
        Unsatisfied,
        Satisfied
    };

    enum struct ClueModelDisposition : uint8_t
    {
        Uninitialized,
        Incomplete,
        InvalidEvidence,
        StaleEvidence,
        ContradictoryEvidence,
        Solved,
        InvalidConfiguration,
        InternalFault
    };

    struct ClueSourceIdentity
    {
        uint16_t sourceId;
        uint16_t configurationRevision;
        uint32_t sessionEpoch;
    };

    struct ClueObservation
    {
        uint8_t              clueId;
        ClueCategory         category;
        ClueQuality          quality;
        ClueSourceIdentity   source;
        uint32_t             sourceSequence;
        MicrosecondTimePoint observedAt;
        Status               status;
    };

    struct ClueTerm
    {
        uint8_t          clueId;
        ClueTermRelation relation;
        ClueCategory     category;
    };

    struct ClueRuleDefinition
    {
        uint8_t  ruleId;
        uint8_t  termCount;
        ClueTerm terms[4];
        uint8_t  prerequisiteCount;
        uint8_t  prerequisiteRuleIds[4];
    };

    struct ClueConstraintConfig
    {
        uint16_t            configurationRevision;
        uint32_t            instanceEpoch;
        MicrosecondDuration maximumEvidenceAge;
        uint8_t             clueCount;
        uint8_t             ruleCount;
        ClueSourceIdentity  expectedSources[12];
        ClueRuleDefinition  rules[12];
    };

    struct ClueEvidenceSnapshot
    {
        bool                 present;
        ClueCategory         category;
        ClueQuality          quality;
        ClueSourceIdentity   source;
        uint32_t             sourceSequence;
        MicrosecondTimePoint observedAt;
        Status               status;
    };

    struct ClueRuleSnapshot
    {
        uint8_t             ruleId;
        ClueRuleDisposition disposition;
        uint8_t             firstBlockingTerm;
        uint8_t             firstBlockingPrerequisite;
    };

    struct ClueConstraintSnapshot
    {
        uint16_t             configurationRevision;
        uint32_t             instanceEpoch;
        uint32_t             generation;
        uint16_t             satisfiedRuleMask;
        uint16_t             blockedRuleMask;
        ClueModelDisposition disposition;
        Status               status;
    };

    struct ClueConstraintUpdate
    {
        MicrosecondTimePoint now;
        uint16_t             observationMask;
        ClueObservation      observations[12];
    };

    struct ClueConstraintModel
    {
        explicit ClueConstraintModel
            (const ClueConstraintConfig& config) noexcept;
        ~ClueConstraintModel () noexcept;

        ClueConstraintModel& operator= (const ClueConstraintModel&) = delete;
        ClueConstraintModel  (const ClueConstraintModel&)           = delete;
        ClueConstraintModel& operator= (ClueConstraintModel&&)      = delete;
        ClueConstraintModel  (ClueConstraintModel&&)                = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        void   reset       () noexcept;
        bool   initialized () const noexcept;

        Status update (const ClueConstraintUpdate& input) noexcept;

        ClueConstraintSnapshot       snapshot () const noexcept;
        Result<ClueEvidenceSnapshot> evidence (uint8_t clueId) const noexcept;
        Result<ClueRuleSnapshot>     rule     (uint8_t ruleId) const noexcept;

      private:
        friend struct InertEscapeConsole;
        friend struct InertEscapeConsoleTestAccess;

        struct CompactEvidence
        {
            CompactEvidence () noexcept;

            uint32_t             sourceSequence;
            MicrosecondTimePoint observedAt;
            Status               status;
            ClueCategory         category;
            ClueQuality          quality;
            bool                 present;
        };

        struct CompactRule
        {
            uint8_t termClueIds[4];
            uint8_t termKinds[4];
            uint8_t prerequisiteRuleIds[4];
            uint8_t termCount;
            uint8_t prerequisiteCount;
        };

        struct PreparedUpdate
        {
            PreparedUpdate () noexcept;

            const void* ownerToken;
            uint32_t    lifecycleGeneration;
            uint32_t    preparationGeneration;
            ClueConstraintSnapshot proposedSnapshot;
        };

        struct PreparedObservation
        {
            PreparedObservation () noexcept;

            uint32_t             sourceSequence;
            MicrosecondTimePoint observedAt;
            Status               status;
            uint8_t              categoryAndQuality;
        };

        Status preflightUpdate
            (const ClueConstraintUpdate& input,
             PreparedUpdate&             prepared) const noexcept;
        void applyPreparedUpdate
            (const PreparedUpdate& prepared) noexcept;
        Result<ClueConstraintSnapshot> preparedSnapshot
            (const PreparedUpdate& prepared) const noexcept;
        Result<ClueEvidenceSnapshot> preparedEvidence
            (const PreparedUpdate& prepared, uint8_t clueId) const noexcept;

        bool   advanceLifecycle         () noexcept;
        void   clearEvaluation          (Status status) noexcept;
        Status prepareInput
            (const ClueConstraintUpdate& input) const noexcept;
        void   applyCurrentPreparation  () noexcept;
        void   evaluate
            (MicrosecondTimePoint now) noexcept;

        ClueSourceIdentity     expectedSources_[12];
        CompactRule            rules_[12];
        CompactEvidence        evidence_[12];
        ClueRuleSnapshot       ruleSnapshots_[12];
        MicrosecondDuration    maximumEvidenceAge_;
        ClueConstraintSnapshot snapshot_;
        uint32_t               lifecycleGeneration_;
        uint8_t                topologicalOrder_[12];
        uint8_t                clueCount_;
        uint8_t                ruleCount_;
        bool                   configurationValid_ : 1;
        bool                   initialized_ : 1;
        bool                   lifecycleExhausted_ : 1;
        mutable bool                 preparationActive_ : 1;
        mutable PreparedObservation  preparedObservations_[12];
        mutable MicrosecondTimePoint preparedNow_;
        mutable uint32_t             preparationGeneration_;
        mutable uint16_t             preparedMask_;
        mutable uint16_t             preparedSatisfiedRuleMask_;
        mutable uint16_t             preparedBlockedRuleMask_;
        mutable ClueModelDisposition preparedDisposition_;
        mutable uint8_t              preparedRuleDispositions_[6];
    };
} // namespace adk
