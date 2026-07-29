#include "clue_constraint_model.h"

namespace adk {
    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool canonicalSource (const ClueSourceIdentity& source) noexcept
        {
            return source.sourceId == 0 && source.configurationRevision == 0 &&
                   source.sessionEpoch == 0;
        }

        bool sameSource (const ClueSourceIdentity& left,
                         const ClueSourceIdentity& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sessionEpoch == right.sessionEpoch;
        }

        bool validSource (const ClueSourceIdentity& source) noexcept
        {
            return source.sourceId != 0 && source.configurationRevision != 0 &&
                   source.sessionEpoch != 0;
        }

        bool validCategory (ClueCategory category) noexcept
        {
            return static_cast<uint8_t> (category) <=
                   static_cast<uint8_t> (ClueCategory::Mismatch);
        }

        bool validQuality (ClueQuality quality) noexcept
        {
            return static_cast<uint8_t> (quality) <=
                   static_cast<uint8_t> (ClueQuality::TimingFault);
        }

        bool canonicalObservation (const ClueObservation& observation) noexcept
        {
            return observation.clueId == 0 &&
                   observation.category == ClueCategory::Absent &&
                   observation.quality == ClueQuality::Invalid &&
                   canonicalSource (observation.source) &&
                   observation.sourceSequence == 0 &&
                   observation.observedAt.microseconds () == 0 &&
                   observation.status.ok               ();
        }

        bool canonicalTerm (const ClueTerm& term) noexcept
        {
            return term.clueId == 0 && term.relation == ClueTermRelation::Equals &&
                   term.category == ClueCategory::Absent;
        }

        bool canonicalRule (const ClueRuleDefinition& rule) noexcept
        {
            if (rule.ruleId != 0 || rule.termCount != 0 || rule.prerequisiteCount != 0)
            {
                return false;
            }
            for (uint8_t index = 0; index < 4; ++index)
            {
                if (!canonicalTerm (rule.terms[index]) ||
                    rule.prerequisiteRuleIds[index] != 0)
                {
                    return false;
                }
            }
            return true;
        }

    } // namespace

    ClueConstraintModel::CompactEvidence::CompactEvidence () noexcept
        : sourceSequence (0), observedAt (0), status {}, category (
              ClueCategory::Absent),
          quality (ClueQuality::Invalid), present (false)
    {
    }

    ClueConstraintModel::PreparedUpdate::PreparedUpdate () noexcept
        : ownerToken (nullptr), lifecycleGeneration (0), preparationGeneration (0),
          proposedSnapshot {}
    {
    }

    ClueConstraintModel::PreparedObservation::PreparedObservation () noexcept
        : sourceSequence (0), observedAt (0), status {}, categoryAndQuality (0)
    {
    }

    ClueConstraintModel::ClueConstraintModel (
        const ClueConstraintConfig& config) noexcept
        : expectedSources_{}, rules_{}, evidence_{}, ruleSnapshots_{},
          maximumEvidenceAge_        (config.maximumEvidenceAge), snapshot_{},
          lifecycleGeneration_       (0), topologicalOrder_{},
          clueCount_                 (config.clueCount),
          ruleCount_                 (config.ruleCount),
          configurationValid_        (true),
          initialized_               (false),
          lifecycleExhausted_        (false),
          preparationActive_         (false), preparedObservations_{},
          preparedNow_               (0),
          preparationGeneration_     (0),
          preparedMask_              (0),
          preparedSatisfiedRuleMask_ (0),
          preparedBlockedRuleMask_   (0),
          preparedDisposition_       (ClueModelDisposition::Uninitialized),
          preparedRuleDispositions_ {}
    {
        snapshot_.configurationRevision = config.configurationRevision;
        snapshot_.instanceEpoch         = config.instanceEpoch;
        snapshot_.disposition           = ClueModelDisposition::Uninitialized;

        if (config.configurationRevision == 0 || config.instanceEpoch == 0 ||
            clueCount_ == 0 || clueCount_ > 12 || ruleCount_ == 0 || ruleCount_ > 12 ||
            maximumEvidenceAge_.microseconds () >= halfRange)
        {
            configurationValid_ = false;
        }

        for (uint8_t clue = 0; clue < 12; ++clue)
        {
            expectedSources_[clue] = config.expectedSources[clue];
            if ((clue < clueCount_ && !validSource (expectedSources_[clue])) ||
                (clue >= clueCount_ && !canonicalSource (expectedSources_[clue])))
            {
                configurationValid_ = false;
            }
        }

        for (uint8_t ruleIndex = 0; ruleIndex < 12; ++ruleIndex)
        {
            const ClueRuleDefinition& source = config.rules[ruleIndex];
            CompactRule&              target = rules_[ruleIndex];
            ruleSnapshots_[ruleIndex] = {ruleIndex, ClueRuleDisposition::Unevaluated,
                                         UINT8_MAX, UINT8_MAX};

            if (ruleIndex >= ruleCount_)
            {
                if (!canonicalRule (source))
                {
                    configurationValid_ = false;
                }
                continue;
            }

            if (source.ruleId != ruleIndex || source.termCount > 4 ||
                source.prerequisiteCount > 4)
            {
                configurationValid_ = false;
            }
            target.termCount = source.termCount <= 4 ? source.termCount : 0;
            target.prerequisiteCount =
                source.prerequisiteCount <= 4 ? source.prerequisiteCount : 0;
            for (uint8_t term = 0; term < 4; ++term)
            {
                const ClueTerm& input    = source.terms[term];
                target.termClueIds[term] = input.clueId;
                target.termKinds[term] =
                    static_cast<uint8_t> (static_cast<uint8_t> (input.category) << 1U) |
                    static_cast<uint8_t> (input.relation);
                if (term < source.termCount)
                {
                    if (input.clueId >= clueCount_ || !validCategory (input.category) ||
                        static_cast<uint8_t> (input.relation) >
                            static_cast<uint8_t> (ClueTermRelation::NotEquals))
                    {
                        configurationValid_ = false;
                    }
                    for (uint8_t prior = 0; prior < term; ++prior)
                    {
                        if (source.terms[prior].clueId == input.clueId)
                        {
                            configurationValid_ = false;
                        }
                    }
                }
                else if (!canonicalTerm (input))
                {
                    configurationValid_ = false;
                }
            }
            for (uint8_t prerequisite = 0; prerequisite < 4; ++prerequisite)
            {
                const uint8_t input = source.prerequisiteRuleIds[prerequisite];
                target.prerequisiteRuleIds[prerequisite] = input;
                if (prerequisite < source.prerequisiteCount)
                {
                    if (input >= ruleCount_ || input == ruleIndex)
                    {
                        configurationValid_ = false;
                    }
                    for (uint8_t prior = 0; prior < prerequisite; ++prior)
                    {
                        if (source.prerequisiteRuleIds[prior] == input)
                        {
                            configurationValid_ = false;
                        }
                    }
                }
                else if (input != 0)
                {
                    configurationValid_ = false;
                }
            }
        }

        uint16_t emitted = 0;
        const uint8_t boundedRuleCount = ruleCount_ <= 12 ? ruleCount_ : 12;
        for (uint8_t position = 0; position < boundedRuleCount; ++position)
        {
            bool found = false;
            for (uint8_t candidate = 0; candidate < boundedRuleCount; ++candidate)
            {
                if ((emitted & (UINT16_C (1) << candidate)) != 0)
                {
                    continue;
                }
                bool ready = true;
                for (uint8_t edge = 0; edge < rules_[candidate].prerequisiteCount;
                     ++edge)
                {
                    const uint8_t dependency =
                        rules_[candidate].prerequisiteRuleIds[edge];
                    if (dependency >= boundedRuleCount ||
                        (emitted & (UINT16_C (1) << dependency)) == 0)
                    {
                        ready = false;
                    }
                }
                if (ready)
                {
                    topologicalOrder_[position] = candidate;
                    emitted |= static_cast<uint16_t> (
                        UINT16_C (1) << candidate);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                configurationValid_ = false;
            }
        }
    }

    ClueConstraintModel::~ClueConstraintModel () noexcept
    {
        shutdown ();
    }

    bool ClueConstraintModel::advanceLifecycle () noexcept
    {
        preparationActive_ = false;
        if (lifecycleExhausted_ || lifecycleGeneration_ == UINT32_MAX)
        {
            lifecycleExhausted_ = true;
            return false;
        }
        ++lifecycleGeneration_;
        return true;
    }

    void ClueConstraintModel::clearEvaluation (Status status) noexcept
    {
        for (uint8_t clue = 0; clue < 12; ++clue)
        {
            evidence_[clue] = CompactEvidence ();
        }
        for (uint8_t ruleIndex = 0; ruleIndex < 12; ++ruleIndex)
        {
            ruleSnapshots_[ruleIndex] = {ruleIndex, ClueRuleDisposition::Unevaluated,
                                         UINT8_MAX, UINT8_MAX};
        }
        snapshot_.generation        = 0;
        snapshot_.satisfiedRuleMask = 0;
        snapshot_.blockedRuleMask   = 0;
        snapshot_.disposition = initialized_ ? ClueModelDisposition::Incomplete
                                             : ClueModelDisposition::Uninitialized;
        snapshot_.status      = status;
    }

    Status ClueConstraintModel::initialize () noexcept
    {
        if (initialized_)
        {
            return snapshot_.status;
        }
        if (!configurationValid_)
        {
            snapshot_.status      = StatusCode::InvalidConfiguration;
            snapshot_.disposition = ClueModelDisposition::InvalidConfiguration;
            return snapshot_.status;
        }
        if (lifecycleExhausted_ || !advanceLifecycle ())
        {
            snapshot_.status      = StatusCode::CapacityExceeded;
            snapshot_.disposition = ClueModelDisposition::InternalFault;
            return snapshot_.status;
        }
        initialized_ = true;
        clearEvaluation (Status{});
        return Status{};
    }

    void ClueConstraintModel::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }
        const bool advanced = advanceLifecycle ();
        initialized_        = false;
        clearEvaluation (advanced ? Status{} : StatusCode::CapacityExceeded);
        if (!advanced)
        {
            snapshot_.disposition = ClueModelDisposition::InternalFault;
        }
    }

    void ClueConstraintModel::reset () noexcept
    {
        const bool advanced = advanceLifecycle ();
        clearEvaluation                        (
            advanced ? Status {} : StatusCode::CapacityExceeded);
        if (!advanced)
        {
            initialized_          = false;
            snapshot_.disposition = ClueModelDisposition::InternalFault;
        }
    }

    bool ClueConstraintModel::initialized () const noexcept
    {
        return initialized_;
    }

    Status
    ClueConstraintModel::preflightUpdate (const ClueConstraintUpdate& input,
                                          PreparedUpdate& prepared) const noexcept
    {
        const Status status = prepareInput (input);

        if (!status.ok ())
        {
            prepared = PreparedUpdate ();
            return status;
        }
        prepared.ownerToken            = this;
        prepared.lifecycleGeneration   = lifecycleGeneration_;
        prepared.preparationGeneration = preparationGeneration_;
        prepared.proposedSnapshot      = snapshot_;
        prepared.proposedSnapshot.generation =
            snapshot_.generation == UINT32_MAX ? UINT32_MAX
                                               : snapshot_.generation + 1U;
        prepared.proposedSnapshot.satisfiedRuleMask =
            preparedSatisfiedRuleMask_;
        prepared.proposedSnapshot.blockedRuleMask = preparedBlockedRuleMask_;
        prepared.proposedSnapshot.disposition     = preparedDisposition_;
        prepared.proposedSnapshot.status          = Status{};
        return Status{};
    }

    Status ClueConstraintModel::prepareInput (
        const ClueConstraintUpdate& input) const noexcept
    {
        preparationActive_ = false;
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (lifecycleExhausted_)
        {
            return StatusCode::CapacityExceeded;
        }
        if (preparationGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        ++preparationGeneration_;
        if ((input.observationMask &
             static_cast<uint16_t> (~ ((UINT16_C (1) << clueCount_) - 1U))) != 0)
        {
            return StatusCode::InvalidArgument;
        }

        for (uint8_t clue = 0; clue < 12; ++clue)
        {
            const bool selected = (input.observationMask & (UINT16_C (1) << clue)) != 0;
            const ClueObservation& observation = input.observations[clue];
            if (!selected)
            {
                if (!canonicalObservation (observation))
                {
                    return StatusCode::InvalidArgument;
                }
                continue;
            }
            if (clue >= clueCount_ || observation.clueId != clue ||
                !validCategory (observation.category) ||
                !validQuality  (observation.quality) ||
                !sameSource    (observation.source, expectedSources_[clue]))
            {
                return StatusCode::InvalidArgument;
            }
            const uint32_t age =
                input.now.elapsedSince (observation.observedAt).microseconds ();
            if (age >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }

            const CompactEvidence& current = evidence_[clue];
            if (current.present)
            {
                const uint32_t sequenceDelta =
                    observation.sourceSequence - current.sourceSequence;
                if (sequenceDelta == 0)
                {
                    if (observation.category != current.category ||
                        observation.quality != current.quality ||
                        observation.observedAt.microseconds () !=
                            current.observedAt.microseconds () ||
                        observation.status != current.status)
                    {
                        return StatusCode::InvalidArgument;
                    }
                }
                else if (sequenceDelta >= halfRange)
                {
                    return StatusCode::InvalidArgument;
                }
                else
                {
                    const uint32_t timeDelta =
                        observation.observedAt.elapsedSince (current.observedAt)
                            .microseconds ();
                    if (timeDelta >= halfRange)
                    {
                        return StatusCode::InvalidArgument;
                    }
                }
            }
            PreparedObservation& next = preparedObservations_[clue];

            next.sourceSequence = observation.sourceSequence;
            next.observedAt     = observation.observedAt;
            next.status         = observation.status;
            next.categoryAndQuality =
                static_cast<uint8_t> (observation.category) |
                static_cast<uint8_t> (
                    static_cast<uint8_t> (observation.quality) << 3U);
        }

        for (uint8_t clue = 0; clue < clueCount_; ++clue)
        {
            const bool selected =
                (input.observationMask & (UINT16_C (1) << clue)) != 0;
            const bool present = selected || evidence_[clue].present;
            const MicrosecondTimePoint observedAt =
                selected ? preparedObservations_[clue].observedAt
                         : evidence_[clue].observedAt;
            if (present &&
                input.now.elapsedSince (observedAt)
                        .microseconds () >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
        }

        for (uint8_t index = 0; index < 6; ++index)
        {
            preparedRuleDispositions_[index] = 0;
        }
        uint16_t satisfiedMask = 0;
        uint16_t            blockedMask       = 0;
        bool                hasInvalid        = false;
        bool                hasStale          = false;
        bool                hasContradictory  = false;
        bool                hasIncomplete     = false;
        for (uint8_t position = 0; position < ruleCount_; ++position)
        {
            const uint8_t      ruleId     = topologicalOrder_[position];
            const CompactRule& definition = rules_[ruleId];
            ClueRuleDisposition disposition = ClueRuleDisposition::Satisfied;
            for (uint8_t prerequisite = 0;
                 prerequisite < definition.prerequisiteCount; ++prerequisite)
            {
                const uint8_t prerequisiteId =
                    definition.prerequisiteRuleIds[prerequisite];
                const uint8_t shift =
                    static_cast<uint8_t> ((prerequisiteId & 1U) * 4U);
                const ClueRuleDisposition prerequisiteDisposition =
                    static_cast<ClueRuleDisposition> (
                        static_cast<uint8_t> (
                            preparedRuleDispositions_[prerequisiteId >> 1U] >>
                            shift) &
                        UINT8_C (0x0f));
                if (prerequisiteDisposition !=
                    ClueRuleDisposition::Satisfied)
                {
                    disposition = ClueRuleDisposition::BlockedByPrerequisite;
                    break;
                }
            }
            if (disposition == ClueRuleDisposition::Satisfied)
            {
                uint8_t strongestRank = 0;
                for (uint8_t term = 0; term < definition.termCount; ++term)
                {
                    const uint8_t clueId = definition.termClueIds[term];
                    const bool selected =
                        (input.observationMask & (UINT16_C (1) << clueId)) != 0;
                    const CompactEvidence& retained = evidence_[clueId];
                    const bool             present  = selected || retained.present;
                    const ClueCategory category =
                        selected
                            ? static_cast<ClueCategory> (
                                  preparedObservations_[clueId].categoryAndQuality &
                                  0x07U)
                            : retained.category;
                    const ClueQuality quality =
                        selected
                            ? static_cast<ClueQuality> (
                                  preparedObservations_[clueId].categoryAndQuality >>
                                  3U)
                            : retained.quality;
                    const Status status =
                        selected ? preparedObservations_[clueId].status
                                 : retained.status;
                    const MicrosecondTimePoint observedAt =
                        selected ? preparedObservations_[clueId].observedAt
                                 : retained.observedAt;
                    ClueRuleDisposition candidate =
                        ClueRuleDisposition::Satisfied;
                    if (!present)
                    {
                        candidate = ClueRuleDisposition::MissingEvidence;
                    }
                    else if (!status.ok () || quality == ClueQuality::Invalid ||
                             quality == ClueQuality::SourceFault ||
                             quality == ClueQuality::TimingFault)
                    {
                        candidate = ClueRuleDisposition::InvalidEvidence;
                    }
                    else if (quality == ClueQuality::Stale ||
                             input.now.elapsedSince (observedAt).microseconds () >
                                 maximumEvidenceAge_.microseconds ())
                    {
                        candidate = ClueRuleDisposition::StaleEvidence;
                    }
                    else if (quality == ClueQuality::Contradictory)
                    {
                        candidate = ClueRuleDisposition::ContradictoryEvidence;
                    }
                    else
                    {
                        const uint8_t kind = definition.termKinds[term];
                        const bool equal =
                            category == static_cast<ClueCategory> (kind >> 1U);
                        if (((kind & 1U) == 0) != equal)
                        {
                            candidate = ClueRuleDisposition::Unsatisfied;
                        }
                    }
                    const uint8_t rank =
                        candidate == ClueRuleDisposition::InvalidEvidence         ? 5
                        : candidate == ClueRuleDisposition::StaleEvidence         ? 4
                        : candidate == ClueRuleDisposition::ContradictoryEvidence ? 3
                        : candidate == ClueRuleDisposition::MissingEvidence       ? 2
                        : candidate == ClueRuleDisposition::Unsatisfied           ? 1
                                                                                  : 0;
                    if (rank > strongestRank)
                    {
                        strongestRank = rank;
                        disposition   = candidate;
                    }
                }
            }
            const uint8_t dispositionShift =
                static_cast<uint8_t> ((ruleId & 1U) * 4U);
            preparedRuleDispositions_[ruleId >> 1U] |=
                static_cast<uint8_t> (
                    static_cast<uint8_t> (disposition) << dispositionShift);
            if (disposition == ClueRuleDisposition::Satisfied)
            {
                satisfiedMask |= static_cast<uint16_t> (UINT16_C (1) << ruleId);
            }
            else
            {
                blockedMask |= static_cast<uint16_t> (UINT16_C (1) << ruleId);
                hasInvalid = hasInvalid ||
                             disposition == ClueRuleDisposition::InvalidEvidence;
                hasStale = hasStale ||
                           disposition == ClueRuleDisposition::StaleEvidence;
                hasContradictory =
                    hasContradictory ||
                    disposition == ClueRuleDisposition::ContradictoryEvidence;
                hasIncomplete =
                    hasIncomplete ||
                    (disposition != ClueRuleDisposition::InvalidEvidence &&
                     disposition != ClueRuleDisposition::StaleEvidence &&
                     disposition != ClueRuleDisposition::ContradictoryEvidence);
            }
        }

        preparedNow_       = input.now;
        preparedMask_      = input.observationMask;
        preparedSatisfiedRuleMask_ = satisfiedMask;
        preparedBlockedRuleMask_   = blockedMask;
        preparedDisposition_ =
            hasInvalid         ? ClueModelDisposition::InvalidEvidence
            : hasStale         ? ClueModelDisposition::StaleEvidence
            : hasContradictory ? ClueModelDisposition::ContradictoryEvidence
            : hasIncomplete    ? ClueModelDisposition::Incomplete
                               : ClueModelDisposition::Solved;
        preparationActive_ = true;
        return Status{};
    }

    void ClueConstraintModel::evaluate (MicrosecondTimePoint now) noexcept
    {
        snapshot_.generation =
            snapshot_.generation == UINT32_MAX ? UINT32_MAX : snapshot_.generation + 1U;
        snapshot_.satisfiedRuleMask = 0;
        snapshot_.blockedRuleMask   = 0;
        snapshot_.status            = Status{};
        bool hasInvalid                     = false;
        bool hasStale                       = false;
        bool hasContradictory               = false;
        bool hasIncomplete                  = false;

        for (uint8_t position = 0; position < ruleCount_; ++position)
        {
            const uint8_t      ruleId     = topologicalOrder_[position];
            const CompactRule& definition = rules_[ruleId];
            ClueRuleSnapshot   result     = {ruleId, ClueRuleDisposition::Satisfied,
                                             UINT8_MAX, UINT8_MAX};
            for (uint8_t prerequisite = 0; prerequisite < definition.prerequisiteCount;
                 ++prerequisite)
            {
                const uint8_t prerequisiteId =
                    definition.prerequisiteRuleIds[prerequisite];
                if (ruleSnapshots_[prerequisiteId].disposition !=
                    ClueRuleDisposition::Satisfied)
                {
                    result.disposition = ClueRuleDisposition::BlockedByPrerequisite;
                    result.firstBlockingPrerequisite = prerequisite;
                    snapshot_.blockedRuleMask |=
                        static_cast<uint16_t> (UINT16_C (1) << ruleId);
                    hasIncomplete = true;
                    break;
                }
            }

            if (result.disposition == ClueRuleDisposition::Satisfied)
            {
                ClueRuleDisposition strongest = ClueRuleDisposition::Satisfied;
                uint8_t             first     = UINT8_MAX;
                for (uint8_t term = 0; term < definition.termCount; ++term)
                {
                    const CompactEvidence& cell =
                        evidence_[definition.termClueIds[term]];
                    ClueRuleDisposition candidate = ClueRuleDisposition::Satisfied;
                    if (!cell.present)
                    {
                        candidate = ClueRuleDisposition::MissingEvidence;
                    }
                    else if (!cell.status.ok () ||
                             cell.quality == ClueQuality::Invalid ||
                             cell.quality == ClueQuality::SourceFault ||
                             cell.quality == ClueQuality::TimingFault)
                    {
                        candidate = ClueRuleDisposition::InvalidEvidence;
                    }
                    else if (cell.quality == ClueQuality::Stale ||
                             now.elapsedSince (cell.observedAt).microseconds () >
                                 maximumEvidenceAge_.microseconds ())
                    {
                        candidate = ClueRuleDisposition::StaleEvidence;
                    }
                    else if (cell.quality == ClueQuality::Contradictory)
                    {
                        candidate = ClueRuleDisposition::ContradictoryEvidence;
                    }
                    else
                    {
                        const uint8_t      kind = definition.termKinds[term];
                        const ClueCategory expected =
                            static_cast<ClueCategory> (kind >> 1U);
                        const bool equal   = cell.category == expected;
                        const bool matches = (kind & 1U) == 0 ? equal : !equal;
                        if (!matches)
                        {
                            candidate = ClueRuleDisposition::Unsatisfied;
                        }
                    }

                    const uint8_t rank =
                        candidate == ClueRuleDisposition::InvalidEvidence         ? 5
                        : candidate == ClueRuleDisposition::StaleEvidence         ? 4
                        : candidate == ClueRuleDisposition::ContradictoryEvidence ? 3
                        : candidate == ClueRuleDisposition::MissingEvidence       ? 2
                        : candidate == ClueRuleDisposition::Unsatisfied           ? 1
                                                                                  : 0;
                    const uint8_t strongestRank =
                        strongest == ClueRuleDisposition::InvalidEvidence         ? 5
                        : strongest == ClueRuleDisposition::StaleEvidence         ? 4
                        : strongest == ClueRuleDisposition::ContradictoryEvidence ? 3
                        : strongest == ClueRuleDisposition::MissingEvidence       ? 2
                        : strongest == ClueRuleDisposition::Unsatisfied           ? 1
                                                                                  : 0;
                    if (rank > strongestRank)
                    {
                        strongest = candidate;
                        first     = term;
                    }
                }
                result.disposition       = strongest;
                result.firstBlockingTerm = first;
            }

            ruleSnapshots_[ruleId] = result;
            if (result.disposition == ClueRuleDisposition::Satisfied)
            {
                snapshot_.satisfiedRuleMask |=
                    static_cast<uint16_t> (UINT16_C (1) << ruleId);
            }
            else
            {
                snapshot_.blockedRuleMask |=
                    static_cast<uint16_t> (UINT16_C (1) << ruleId);
                if (result.disposition == ClueRuleDisposition::InvalidEvidence)
                {
                    hasInvalid = true;
                }
                else if (result.disposition == ClueRuleDisposition::StaleEvidence)
                {
                    hasStale = true;
                }
                else if (result.disposition ==
                         ClueRuleDisposition::ContradictoryEvidence)
                {
                    hasContradictory = true;
                }
                else
                {
                    hasIncomplete = true;
                }
            }
        }
        for (uint8_t unused = ruleCount_; unused < 12; ++unused)
        {
            ruleSnapshots_[unused] = {unused, ClueRuleDisposition::Unevaluated,
                                      UINT8_MAX, UINT8_MAX};
        }

        snapshot_.disposition =
            hasInvalid         ? ClueModelDisposition::InvalidEvidence
            : hasStale         ? ClueModelDisposition::StaleEvidence
            : hasContradictory ? ClueModelDisposition::ContradictoryEvidence
            : hasIncomplete    ? ClueModelDisposition::Incomplete
                               : ClueModelDisposition::Solved;
    }

    Result<ClueConstraintSnapshot> ClueConstraintModel::preparedSnapshot (
        const PreparedUpdate& prepared) const noexcept
    {
        const Result<ClueEvidenceSnapshot> validation =
            preparedEvidence (prepared, 0);
        if (!validation.ok ())
        {
            return {validation.status (), {}};
        }
        return {Status{}, prepared.proposedSnapshot};
    }

    Result<ClueEvidenceSnapshot> ClueConstraintModel::preparedEvidence (
        const PreparedUpdate& prepared, uint8_t clueId) const noexcept
    {
        const uint32_t proposedGeneration =
            snapshot_.generation == UINT32_MAX ? UINT32_MAX
                                               : snapshot_.generation + 1U;
        if (!initialized_ || lifecycleExhausted_ || !preparationActive_ ||
            clueId >= clueCount_ || prepared.ownerToken != this ||
            prepared.lifecycleGeneration != lifecycleGeneration_ ||
            prepared.preparationGeneration != preparationGeneration_ ||
            prepared.proposedSnapshot.configurationRevision !=
                snapshot_.configurationRevision ||
            prepared.proposedSnapshot.instanceEpoch != snapshot_.instanceEpoch ||
            prepared.proposedSnapshot.generation != proposedGeneration ||
            prepared.proposedSnapshot.satisfiedRuleMask !=
                preparedSatisfiedRuleMask_ ||
            prepared.proposedSnapshot.blockedRuleMask !=
                preparedBlockedRuleMask_ ||
            prepared.proposedSnapshot.disposition != preparedDisposition_ ||
            !prepared.proposedSnapshot.status.ok ())
        {
            const ClueEvidenceSnapshot empty = {
                false, ClueCategory::Absent, ClueQuality::Invalid, {}, 0,
                MicrosecondTimePoint (0), Status {}};
            return {StatusCode::InvalidArgument, empty};
        }

        if ((preparedMask_ & (UINT16_C (1) << clueId)) == 0)
        {
            return evidence (clueId);
        }

        const PreparedObservation& value = preparedObservations_[clueId];
        return {
            Status{},
            {true,
             static_cast<ClueCategory> (value.categoryAndQuality & 0x07U),
             static_cast<ClueQuality> (value.categoryAndQuality >> 3U),
             expectedSources_[clueId], value.sourceSequence, value.observedAt,
             value.status}};
    }

    void ClueConstraintModel::applyPreparedUpdate (
        const PreparedUpdate& prepared) noexcept
    {
        if (!initialized_ || lifecycleExhausted_ || !preparationActive_ ||
            prepared.ownerToken != this ||
            prepared.lifecycleGeneration != lifecycleGeneration_ ||
            prepared.preparationGeneration != preparationGeneration_ ||
            prepared.proposedSnapshot.configurationRevision !=
                snapshot_.configurationRevision ||
            prepared.proposedSnapshot.instanceEpoch != snapshot_.instanceEpoch ||
            prepared.proposedSnapshot.generation !=
                (snapshot_.generation == UINT32_MAX
                     ? UINT32_MAX
                     : snapshot_.generation + 1U) ||
            prepared.proposedSnapshot.satisfiedRuleMask !=
                preparedSatisfiedRuleMask_ ||
            prepared.proposedSnapshot.blockedRuleMask !=
                preparedBlockedRuleMask_ ||
            prepared.proposedSnapshot.disposition != preparedDisposition_ ||
            !prepared.proposedSnapshot.status.ok ())
        {
            return;
        }
        applyCurrentPreparation ();
    }

    void ClueConstraintModel::applyCurrentPreparation () noexcept
    {
        preparationActive_ = false;
        for (uint8_t clue = 0; clue < 12; ++clue)
        {
            if ((preparedMask_ & (UINT16_C (1) << clue)) == 0)
            {
                continue;
            }
            const PreparedObservation& observation = preparedObservations_[clue];
            CompactEvidence&           next        = evidence_[clue];

            next.sourceSequence = observation.sourceSequence;
            next.observedAt     = observation.observedAt;
            next.status         = observation.status;
            next.category       = static_cast<ClueCategory> (
                observation.categoryAndQuality & 0x07U);
            next.quality = static_cast<ClueQuality> (
                observation.categoryAndQuality >> 3U);
            next.present        = true;
        }
        evaluate (preparedNow_);
        snapshot_.satisfiedRuleMask = preparedSatisfiedRuleMask_;
        snapshot_.blockedRuleMask   = preparedBlockedRuleMask_;
        snapshot_.disposition       = preparedDisposition_;
        snapshot_.status            = Status{};
    }

    Status ClueConstraintModel::update (const ClueConstraintUpdate& input) noexcept
    {
        const Status status = prepareInput (input);

        if (!status.ok ())
        {
            return status;
        }
        applyCurrentPreparation ();
        return Status{};
    }

    ClueConstraintSnapshot ClueConstraintModel::snapshot () const noexcept
    {
        return snapshot_;
    }

    Result<ClueEvidenceSnapshot>
    ClueConstraintModel::evidence (uint8_t clueId) const noexcept
    {
        if (clueId >= clueCount_)
        {
            const ClueEvidenceSnapshot empty = {
                false, ClueCategory::Absent, ClueQuality::Invalid, {}, 0,
                MicrosecondTimePoint (0), Status {}};
            return {StatusCode::InvalidArgument, empty};
        }
        const CompactEvidence&   value = evidence_[clueId];
        const ClueSourceIdentity source =
            value.present ? expectedSources_[clueId] : ClueSourceIdentity{};
        return {Status{},
                {value.present, value.category, value.quality, source,
                 value.sourceSequence, value.observedAt, value.status}};
    }

    Result<ClueRuleSnapshot> ClueConstraintModel::rule (uint8_t ruleId) const noexcept
    {
        if (ruleId >= ruleCount_)
        {
            return {StatusCode::InvalidArgument, {}};
        }
        return {Status{}, ruleSnapshots_[ruleId]};
    }
} // namespace adk
