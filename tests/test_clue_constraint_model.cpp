#define private public
#include <clue_constraint_model.h>
#undef private

#include <cassert>
#include <cstdint>
#include <new>
#include <type_traits>

#ifndef ADK_CLUE_CONSTRAINT_TEST_PART
#define ADK_CLUE_CONSTRAINT_TEST_PART 0
#endif

#if ADK_CLUE_CONSTRAINT_TEST_PART < 0 || ADK_CLUE_CONSTRAINT_TEST_PART > 2
#error "ADK_CLUE_CONSTRAINT_TEST_PART must be 1 or 2 when defined"
#endif

namespace {

    constexpr uint32_t halfRange = UINT32_C (0x80000000);

    adk::ClueSourceIdentity source (uint8_t clueId) noexcept
    {
        return {static_cast<uint16_t> (100U + clueId), 7,
                static_cast<uint32_t> (1000U + clueId)};
    }

    adk::ClueConstraintConfig baseConfig (uint8_t clues = 4, uint8_t rules = 4) noexcept
    {
        adk::ClueConstraintConfig result;
        result.configurationRevision = 7;
        result.instanceEpoch         = 19;
        result.maximumEvidenceAge    = adk::MicrosecondDuration (100);
        result.clueCount             = clues;
        result.ruleCount             = rules;
        for (uint8_t index = 0; index < 12; ++index)
        {
            result.expectedSources[index]         = {0, 0, 0};
            result.rules[index].ruleId            = 0;
            result.rules[index].termCount         = 0;
            result.rules[index].prerequisiteCount = 0;
            for (uint8_t slot = 0; slot < 4; ++slot)
            {
                result.rules[index].terms[slot] = {0, adk::ClueTermRelation::Equals,
                                                   adk::ClueCategory::Absent};
                result.rules[index].prerequisiteRuleIds[slot] = 0;
            }
        }
        for (uint8_t index = 0; index < clues && index < 12; ++index)
        {
            result.expectedSources[index] = source (index);
        }
        for (uint8_t index = 0; index < rules && index < 12; ++index)
        {
            result.rules[index].ruleId = index;
        }
        return result;
    }

    adk::ClueObservation
    observation (uint8_t clueId, uint32_t sequence, uint32_t observedAt,
                 adk::ClueCategory category = adk::ClueCategory::Nominal,
                 adk::ClueQuality  quality  = adk::ClueQuality::Qualified,
                 adk::Status       status   = adk::Status ()) noexcept
    {
        return {clueId,          category, quality,
                source (clueId), sequence, adk::MicrosecondTimePoint (observedAt),
                status};
    }

    adk::ClueConstraintUpdate emptyUpdate (uint32_t now) noexcept
    {
        adk::ClueConstraintUpdate result;
        result.now             = adk::MicrosecondTimePoint (now);
        result.observationMask = 0;
        for (uint8_t index = 0; index < 12; ++index)
        {
            result.observations[index] = {0,
                                          adk::ClueCategory::Absent,
                                          adk::ClueQuality::Invalid,
                                          {0, 0, 0},
                                          0,
                                          adk::MicrosecondTimePoint (),

                                          adk::Status ()};
        }
        return result;
    }

    bool sameSource (const adk::ClueSourceIdentity& left,
                     const adk::ClueSourceIdentity& right) noexcept
    {
        return left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.sessionEpoch == right.sessionEpoch;
    }

    bool sameEvidence (const adk::ClueEvidenceSnapshot& left,
                       const adk::ClueEvidenceSnapshot& right) noexcept
    {
        return left.present == right.present && left.category == right.category &&
               left.quality == right.quality &&
               sameSource (left.source, right.source) &&
               left.sourceSequence == right.sourceSequence &&
               left.observedAt.microseconds () == right.observedAt.microseconds () &&
               left.status == right.status;
    }

    bool sameRule (const adk::ClueRuleSnapshot& left,
                   const adk::ClueRuleSnapshot& right) noexcept
    {
        return left.ruleId == right.ruleId && left.disposition == right.disposition &&
               left.firstBlockingTerm == right.firstBlockingTerm &&
               left.firstBlockingPrerequisite == right.firstBlockingPrerequisite;
    }

    bool sameSnapshot (const adk::ClueConstraintSnapshot& left,
                       const adk::ClueConstraintSnapshot& right) noexcept
    {
        return left.configurationRevision == right.configurationRevision &&
               left.instanceEpoch == right.instanceEpoch &&
               left.generation == right.generation &&
               left.satisfiedRuleMask == right.satisfiedRuleMask &&
               left.blockedRuleMask == right.blockedRuleMask &&
               left.disposition == right.disposition && left.status == right.status;
    }

    void assertCanonicalEvidence (const adk::ClueEvidenceSnapshot& value)
    {
        assert (!value.present);
        assert (value.category == adk::ClueCategory::Absent);
        assert (value.quality == adk::ClueQuality::Invalid);
        assert (value.source.sourceId == 0);
        assert (value.source.configurationRevision == 0);
        assert (value.source.sessionEpoch == 0);
        assert (value.sourceSequence == 0);
        assert (value.observedAt.microseconds () == 0);
        assert (value.status.ok ());
    }

    void assertCanonicalRule (const adk::ClueRuleSnapshot& value)
    {
        assert (value.ruleId == 0);
        assert (value.disposition == adk::ClueRuleDisposition::Unevaluated);
        assert (value.firstBlockingTerm == 0);
        assert (value.firstBlockingPrerequisite == 0);
    }

    void testTypeAndLifecycleContract ()
    {
        static_assert (!std::is_copy_constructible<adk::ClueConstraintModel>::value,
                       "model must not copy");
        static_assert (!std::is_move_constructible<adk::ClueConstraintModel>::value,
                       "model must not move");

        adk::ClueConstraintConfig config = baseConfig (1, 1);

        adk::ClueConstraintModel model (config);

        assert (!model.initialized ());
        assert (model.snapshot ().disposition ==
                adk::ClueModelDisposition::Uninitialized);
        assert (model.update (emptyUpdate (1)).error () ==
                adk::StatusCode::NotInitialized);
        assert (model.initialize ().ok ());
        assert (model.initialized ());

        const adk::ClueConstraintSnapshot initialized = model.snapshot ();

        assert (model.initialize ().ok ());
        assert (sameSnapshot (initialized, model.snapshot ()));

        model.reset ();

        assert (model.initialized ());
        assert (model.snapshot ().generation == 0);
        assert (model.snapshot ().disposition == adk::ClueModelDisposition::Incomplete);

        model.shutdown ();

        assert (!model.initialized ());

        const adk::ClueConstraintSnapshot shutdown = model.snapshot ();

        model.shutdown ();

        assert (sameSnapshot (shutdown, model.snapshot ()));
        assert (model.initialize ().ok ());
        assert (model.initialized ());
    }

    void testConfigurationBoundaries ()
    {
        for (uint8_t clues = 0; clues <= 13; ++clues)
        {
            adk::ClueConstraintConfig config = baseConfig (clues <= 12 ? clues : 12, 1);
            config.clueCount                 = clues;
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().ok () == (clues >= 1 && clues <= 12));
        }
        for (uint8_t rules = 0; rules <= 13; ++rules)
        {
            adk::ClueConstraintConfig config = baseConfig (1, rules <= 12 ? rules : 12);
            config.ruleCount                 = rules;
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().ok () == (rules >= 1 && rules <= 12));
        }

        for (uint8_t field = 0; field < 3; ++field)
        {
            adk::ClueConstraintConfig config = baseConfig (1, 1);
            if (field == 0)
            {
                config.expectedSources[0].sourceId = 0;
            }
            else if (field == 1)
            {
                config.expectedSources[0].configurationRevision = 0;
            }
            else
            {
                config.expectedSources[0].sessionEpoch = 0;
            }
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().error () ==
                    adk::StatusCode::InvalidConfiguration);
        }

        for (uint8_t count = 0; count <= 5; ++count)
        {
            adk::ClueConstraintConfig config = baseConfig (4, 1);
            config.rules[0].termCount        = count;
            for (uint8_t index = 0; index < count && index < 4; ++index)
            {
                config.rules[0].terms[index] = {index, adk::ClueTermRelation::Equals,
                                                adk::ClueCategory::Nominal};
            }
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().ok () == (count <= 4));
        }

        adk::ClueConstraintConfig duplicateTerm = baseConfig (1, 1);
        duplicateTerm.rules[0].termCount        = 2;
        duplicateTerm.rules[0].terms[0]         = {0, adk::ClueTermRelation::Equals,
                                                   adk::ClueCategory::Nominal};
        duplicateTerm.rules[0].terms[1]         = duplicateTerm.rules[0].terms[0];
        adk::ClueConstraintModel duplicateTermModel (duplicateTerm);

        assert (duplicateTermModel.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::ClueConstraintConfig noncanonical   = baseConfig (1, 1);
        noncanonical.expectedSources[1].sourceId = 1;
        adk::ClueConstraintModel noncanonicalModel (noncanonical);

        assert (noncanonicalModel.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::ClueConstraintConfig age = baseConfig (1, 1);

        age.maximumEvidenceAge = adk::MicrosecondDuration (halfRange);

        adk::ClueConstraintModel ageModel (age);

        assert (ageModel.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
    }

    bool graphIsDag (uint16_t edgeMask) noexcept
    {
        bool    reach[4][4] = {};
        uint8_t bit         = 0;
        for (uint8_t from = 0; from < 4; ++from)
        {
            for (uint8_t to = 0; to < 4; ++to)
            {
                if (from != to)
                {
                    reach[from][to] = (edgeMask & (1U << bit)) != 0U;
                    ++bit;
                }
            }
        }
        for (uint8_t through = 0; through < 4; ++through)
        {
            for (uint8_t from = 0; from < 4; ++from)
            {
                for (uint8_t to = 0; to < 4; ++to)
                {
                    reach[from][to] =
                        reach[from][to] || (reach[from][through] && reach[through][to]);
                }
            }
        }
        for (uint8_t node = 0; node < 4; ++node)
        {
            if (reach[node][node])
            {
                return false;
            }
        }
        return true;
    }

    adk::ClueConstraintConfig graphConfig (uint16_t edgeMask) noexcept
    {
        adk::ClueConstraintConfig config = baseConfig (1, 4);
        uint8_t                   bit    = 0;
        for (uint8_t prerequisite = 0; prerequisite < 4; ++prerequisite)
        {
            for (uint8_t dependent = 0; dependent < 4; ++dependent)
            {
                if (prerequisite != dependent)
                {
                    if ((edgeMask & (1U << bit)) != 0U)
                    {
                        adk::ClueRuleDefinition& rule = config.rules[dependent];
                        rule.prerequisiteRuleIds[rule.prerequisiteCount++] =
                            prerequisite;
                    }
                    ++bit;
                }
            }
        }
        return config;
    }

    void testExhaustiveFourNodeDagOracle ()
    {
        for (uint16_t mask = 0; mask < 4096; ++mask)
        {
            const bool expected = graphIsDag (mask);

            adk::ClueConstraintConfig config = graphConfig (mask);

            adk::ClueConstraintModel model (config);

            assert (model.initialize ().ok () == expected);
            if (expected)
            {
                assert (model.update (emptyUpdate (10)).ok ());
                assert (model.snapshot ().satisfiedRuleMask == 0x000f);
                assert (model.snapshot ().blockedRuleMask == 0);
                assert (model.snapshot ().disposition ==
                        adk::ClueModelDisposition::Solved);
            }
        }
    }

    void testAllObservationMasksAndSlotIdentity ()
    {
        adk::ClueConstraintConfig config = baseConfig (12, 1);

        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());
        for (uint16_t mask = 0; mask < 4096; ++mask)
        {
            adk::ClueConstraintUpdate input = emptyUpdate (100);
            input.observationMask           = mask;
            for (uint8_t clue = 0; clue < 12; ++clue)
            {
                if ((mask & (1U << clue)) != 0U)
                {
                    input.observations[clue] = observation (clue, mask + 1U, 100);
                }
            }
            assert (model.update (input).ok ());
        }

        for (uint8_t clue = 0; clue < 12; ++clue)
        {
            adk::ClueConstraintUpdate wrong = emptyUpdate (200);
            wrong.observationMask           = static_cast<uint16_t> (1U << clue);
            wrong.observations[clue] =
                observation (static_cast<uint8_t> ((clue + 1U) % 12U), 1, 200);
            assert (model.update (wrong).error () == adk::StatusCode::InvalidArgument);

            adk::ClueConstraintUpdate absent         = emptyUpdate (200);
            absent.observations[clue].sourceSequence = 1;
            assert (model.update (absent).error () == adk::StatusCode::InvalidArgument);
        }

        adk::ClueConstraintUpdate highBit = emptyUpdate (200);

        highBit.observationMask = UINT16_C (0x1000);

        assert (model.update (highBit).error () == adk::StatusCode::InvalidArgument);
    }

    void testEvaluationPrecedence ()
    {
        adk::ClueConstraintConfig config = baseConfig (4, 2);
        config.rules[0].termCount        = 4;
        for (uint8_t index = 0; index < 4; ++index)
        {
            config.rules[0].terms[index] = {index, adk::ClueTermRelation::Equals,
                                            adk::ClueCategory::Nominal};
        }
        config.rules[1].prerequisiteCount      = 1;
        config.rules[1].prerequisiteRuleIds[0] = 0;
        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());

        adk::ClueConstraintUpdate input = emptyUpdate (200);
        input.observationMask           = 0x000f;
        input.observations[0] = observation (0, 1, 200, adk::ClueCategory::High);
        input.observations[1] = observation (1, 1, 200, adk::ClueCategory::Nominal,
                                             adk::ClueQuality::Contradictory);
        input.observations[2] = observation (2, 1, 0, adk::ClueCategory::Nominal,
                                             adk::ClueQuality::Qualified);
        input.observations[3] = observation (3, 1, 200, adk::ClueCategory::Nominal,
                                             adk::ClueQuality::SourceFault);
        assert (model.update (input).ok ());

        const adk::Result<adk::ClueRuleSnapshot> first = model.rule (0);

        assert (first.ok ());
        assert (first.value ().disposition ==
                adk::ClueRuleDisposition::InvalidEvidence);
        assert (first.value ().firstBlockingTerm == 3);

        const adk::Result<adk::ClueRuleSnapshot> second = model.rule (1);

        assert (second.ok ());
        assert (second.value ().disposition ==
                adk::ClueRuleDisposition::BlockedByPrerequisite);
        assert (second.value ().firstBlockingPrerequisite == 0);
        assert (model.snapshot ().disposition ==
                adk::ClueModelDisposition::InvalidEvidence);

        model.reset ();

        adk::ClueConstraintUpdate missing = emptyUpdate (200);
        missing.observationMask           = 0x0001;
        missing.observations[0] = observation (0, 1, 200, adk::ClueCategory::High);

        assert (model.update (missing).ok ());
        assert (model.rule (0).value ().disposition ==
                adk::ClueRuleDisposition::MissingEvidence);
    }

    void testFreshnessSequenceIdentityAndAtomicity ()
    {
        adk::ClueConstraintConfig config = baseConfig (2, 1);
        config.rules[0].termCount        = 1;
        config.rules[0].terms[0]         = {0, adk::ClueTermRelation::Equals,
                                            adk::ClueCategory::Nominal};
        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());

        adk::ClueConstraintUpdate accepted = emptyUpdate (200);
        accepted.observationMask           = 3;
        accepted.observations[0]           = observation (0, UINT32_MAX, 100);
        accepted.observations[1]           = observation (1, 10, 200);

        assert (model.update (accepted).ok ());

        const adk::ClueConstraintSnapshot before         = model.snapshot ();
        const adk::ClueEvidenceSnapshot   evidenceBefore = model.evidence (0).value ();

        adk::ClueConstraintUpdate duplicate = accepted;
        assert (model.update (duplicate).ok ());
        assert (model.snapshot ().generation == before.generation + 1U);

        adk::ClueConstraintUpdate wrap = emptyUpdate (201);
        wrap.observationMask           = 1;
        wrap.observations[0]           = observation (0, 0, 201);

        assert (model.update (wrap).ok ());

        const adk::ClueConstraintSnapshot stableSnapshot = model.snapshot ();
        const adk::ClueEvidenceSnapshot   stableEvidence = model.evidence (0).value ();
        for (uint8_t field = 0; field < 5; ++field)
        {
            adk::ClueConstraintUpdate bad = emptyUpdate (202);
            bad.observationMask           = 1;
            bad.observations[0]           = observation (0, 0, 201);
            if (field == 0)
            {
                bad.observations[0].category = adk::ClueCategory::High;
            }
            else if (field == 1)
            {
                bad.observations[0].quality = adk::ClueQuality::Degraded;
            }
            else if (field == 2)
            {
                bad.observations[0].source.sourceId++;
            }
            else if (field == 3)
            {
                bad.observations[0].observedAt = adk::MicrosecondTimePoint (202);
            }
            else
            {
                bad.observations[0].status =
                    adk::Status (adk::StatusCode::HardwareFailure);
            }
            assert (!model.update (bad).ok ());
            assert (sameSnapshot (stableSnapshot, model.snapshot ()));
            assert (sameEvidence (stableEvidence, model.evidence (0).value ()));
        }

        adk::ClueConstraintUpdate regression = emptyUpdate (202);
        regression.observationMask           = 1;
        regression.observations[0]           = observation (0, UINT32_MAX, 202);

        assert (!model.update (regression).ok ());

        regression.observations[0] = observation (0, halfRange, 202);

        assert (!model.update (regression).ok ());
        assert (sameSnapshot (stableSnapshot, model.snapshot ()));
        (void)evidenceBefore;
    }

    void testTimeEdges ()
    {
        const uint32_t ages[] = {0, 1, halfRange - 1U};
        for (uint8_t ageIndex = 0; ageIndex < 3; ++ageIndex)
        {
            adk::ClueConstraintConfig config = baseConfig (1, 1);

            config.maximumEvidenceAge = adk::MicrosecondDuration (ages[ageIndex]);
            config.rules[0].termCount = 1;
            config.rules[0].terms[0]  = {0, adk::ClueTermRelation::Equals,
                                         adk::ClueCategory::Nominal};
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().ok ());

            adk::ClueConstraintUpdate edge = emptyUpdate (100 + ages[ageIndex]);
            edge.observationMask           = 1;
            edge.observations[0]           = observation (0, 1, 100);

            assert (model.update (edge).ok ());
            assert (model.rule (0).value ().disposition ==
                    adk::ClueRuleDisposition::Satisfied);

            model.reset ();

            adk::ClueConstraintUpdate stale = emptyUpdate (101 + ages[ageIndex]);
            stale.observationMask           = 1;
            stale.observations[0]           = observation (0, 1, 100);
            if (ages[ageIndex] == halfRange - 1U)
            {
                assert (!model.update (stale).ok ());
            }
            else
            {
                assert (model.update (stale).ok ());
                assert (model.rule (0).value ().disposition ==
                        adk::ClueRuleDisposition::StaleEvidence);
            }
        }

        adk::ClueConstraintConfig config = baseConfig (1, 1);

        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());

        adk::ClueConstraintUpdate future = emptyUpdate (100);
        future.observationMask           = 1;
        future.observations[0]           = observation (0, 1, 101);

        assert (!model.update (future).ok ());

        future.observations[0] = observation (0, 1, 100 + halfRange);

        assert (!model.update (future).ok ());

        adk::ClueConstraintUpdate rollover = emptyUpdate (20);
        rollover.observationMask           = 1;
        rollover.observations[0]           = observation (0, 1, UINT32_MAX - 10U);

        assert (model.update (rollover).ok ());
    }

    void testInvalidQueriesAndReplay ()
    {
        adk::ClueConstraintConfig config = baseConfig (4, 4);

        adk::ClueConstraintModel left (config);

        adk::ClueConstraintModel right (config);

        assert (left.initialize ().ok ());
        assert (right.initialize ().ok ());
        for (uint8_t invalid = 4; invalid < 14; ++invalid)
        {
            const adk::Result<adk::ClueEvidenceSnapshot> evidence =
                left.evidence (invalid);
            assert (evidence.error () == adk::StatusCode::InvalidArgument);

            assertCanonicalEvidence (evidence.value ());

            const adk::Result<adk::ClueRuleSnapshot> rule = left.rule (invalid);

            assert (rule.error () == adk::StatusCode::InvalidArgument);

            assertCanonicalRule (rule.value ());
        }

        adk::ClueConstraintUpdate input = emptyUpdate (77);
        input.observationMask           = 0x000f;
        for (uint8_t clue = 0; clue < 4; ++clue)
        {
            input.observations[clue] = observation (clue, 9, 77);
        }
        assert (left.update (input).ok ());
        assert (right.update (input).ok ());
        assert (sameSnapshot (left.snapshot (), right.snapshot ()));
        for (uint8_t clue = 0; clue < 4; ++clue)
        {
            assert (sameEvidence (left.evidence (clue).value (),
                                  right.evidence (clue).value ()));
        }
        for (uint8_t ruleId = 0; ruleId < 4; ++ruleId)
        {
            assert (
                sameRule (left.rule (ruleId).value (), right.rule (ruleId).value ()));
        }
    }

    void testPrerequisiteEnumAndQualityBoundaries ()
    {
        for (uint8_t count = 0; count <= 5; ++count)
        {
            adk::ClueConstraintConfig config  = baseConfig (1, 5);
            config.rules[4].prerequisiteCount = count;
            for (uint8_t index = 0; index < count && index < 4; ++index)
            {
                config.rules[4].prerequisiteRuleIds[index] = index;
            }
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().ok () == (count <= 4));
        }

        adk::ClueConstraintConfig invalid       = baseConfig (1, 3);
        invalid.rules[2].prerequisiteCount      = 2;
        invalid.rules[2].prerequisiteRuleIds[0] = 0;
        invalid.rules[2].prerequisiteRuleIds[1] = 0;
        adk::ClueConstraintModel duplicate (invalid);

        assert (duplicate.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        invalid                                 = baseConfig (1, 1);
        invalid.rules[0].prerequisiteCount      = 1;
        invalid.rules[0].prerequisiteRuleIds[0] = 0;
        adk::ClueConstraintModel self (invalid);

        assert (self.initialize ().error () == adk::StatusCode::InvalidConfiguration);

        for (uint8_t kind = 0; kind < 3; ++kind)
        {
            adk::ClueConstraintConfig config = baseConfig (1, 1);
            config.rules[0].termCount        = 1;
            config.rules[0].terms[0]         = {0, adk::ClueTermRelation::Equals,
                                                adk::ClueCategory::Nominal};
            if (kind == 0)
            {
                config.rules[0].terms[0].relation =
                    static_cast<adk::ClueTermRelation> (2);
            }
            else if (kind == 1)
            {
                config.rules[0].terms[0].category = static_cast<adk::ClueCategory> (8);
            }
            else
            {
                config.rules[0].terms[0].clueId = 1;
            }
            adk::ClueConstraintModel model (config);

            assert (model.initialize ().error () ==
                    adk::StatusCode::InvalidConfiguration);
        }

        const adk::ClueRuleDisposition expected[] = {
            adk::ClueRuleDisposition::InvalidEvidence,
            adk::ClueRuleDisposition::Satisfied,
            adk::ClueRuleDisposition::Satisfied,
            adk::ClueRuleDisposition::StaleEvidence,
            adk::ClueRuleDisposition::ContradictoryEvidence,
            adk::ClueRuleDisposition::InvalidEvidence,
            adk::ClueRuleDisposition::InvalidEvidence};
        for (uint8_t quality = 0; quality < 7; ++quality)
        {
            for (uint8_t position = 0; position < 4; ++position)
            {
                adk::ClueConstraintConfig config = baseConfig (4, 1);
                config.rules[0].termCount        = 4;
                for (uint8_t term = 0; term < 4; ++term)
                {
                    config.rules[0].terms[term] = {term, adk::ClueTermRelation::Equals,
                                                   adk::ClueCategory::Nominal};
                }
                adk::ClueConstraintModel model (config);

                assert (model.initialize ().ok ());

                adk::ClueConstraintUpdate input = emptyUpdate (10);
                input.observationMask           = 0x000f;
                for (uint8_t clue = 0; clue < 4; ++clue)
                {
                    input.observations[clue] = observation (clue, 1, 10);
                }
                input.observations[position].quality =
                    static_cast<adk::ClueQuality> (quality);
                assert (model.update (input).ok ());
                assert (model.rule (0).value ().disposition == expected[quality]);
                assert (model.rule (0).value ().firstBlockingTerm ==
                        (expected[quality] == adk::ClueRuleDisposition::Satisfied
                             ? UINT8_MAX
                             : position));
            }
        }

        adk::ClueConstraintConfig config = baseConfig (1, 1);

        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());

        adk::ClueConstraintUpdate update = emptyUpdate (10);
        update.observationMask           = 1;
        update.observations[0]           = observation (0, 1, 10);
        update.observations[0].quality   = static_cast<adk::ClueQuality> (7);
        assert (model.update (update).error () == adk::StatusCode::InvalidArgument);
    }

    void testSourceAtomicityAndPreparedCapability ()
    {
        adk::ClueConstraintConfig config = baseConfig (3, 1);

        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());

        adk::ClueConstraintUpdate input = emptyUpdate (100);
        input.observationMask           = 7;
        for (uint8_t clue = 0; clue < 3; ++clue)
        {
            input.observations[clue] = observation (clue, 10, 100);
        }
        assert (model.update (input).ok ());

        const adk::ClueConstraintSnapshot before = model.snapshot ();

        adk::ClueEvidenceSnapshot cells[3] = {model.evidence (0).value (),
                                              model.evidence (1).value (),
                                              model.evidence (2).value ()};
        uint8_t                   order[12];
        for (uint8_t index = 0; index < 12; ++index)
        {
            order[index] = model.topologicalOrder_[index];
        }

        for (uint8_t selected = 0; selected < 3; ++selected)
        {
            for (uint8_t field = 0; field < 3; ++field)
            {
                adk::ClueConstraintUpdate bad = emptyUpdate (101);
                bad.observationMask           = 7;
                for (uint8_t clue = 0; clue < 3; ++clue)
                {
                    bad.observations[clue] = observation (clue, 11, 101);
                }
                if (field == 0)
                {
                    ++bad.observations[selected].source.sourceId;
                }
                else if (field == 1)
                {
                    ++bad.observations[selected].source.configurationRevision;
                }
                else
                {
                    ++bad.observations[selected].source.sessionEpoch;
                }
                assert (!model.update (bad).ok ());
                assert (sameSnapshot (before, model.snapshot ()));
                for (uint8_t clue = 0; clue < 3; ++clue)
                {
                    assert (sameEvidence (cells[clue], model.evidence (clue).value ()));
                }
                for (uint8_t index = 0; index < 12; ++index)
                {
                    assert (order[index] == model.topologicalOrder_[index]);
                }
            }
        }

        adk::ClueConstraintUpdate timeRegression = emptyUpdate (101);
        timeRegression.observationMask           = 1;
        timeRegression.observations[0]           = observation (0, 11, 99);

        assert (!model.update (timeRegression).ok ());
        assert (sameSnapshot (before, model.snapshot ()));
        assert (sameEvidence (cells[0], model.evidence (0).value ()));

        adk::ClueConstraintUpdate ambiguousTime = emptyUpdate (100 + halfRange);
        ambiguousTime.observationMask           = 1;
        ambiguousTime.observations[0]           = observation (0, 11, 100);

        assert (!model.update (ambiguousTime).ok ());
        assert (sameSnapshot (before, model.snapshot ()));
        assert (sameEvidence (cells[0], model.evidence (0).value ()));

        adk::ClueConstraintUpdate next = emptyUpdate (101);
        next.observationMask           = 1;
        next.observations[0]           = observation (0, 11, 101);
        adk::ClueConstraintModel::PreparedUpdate prepared;
        assert (model.preflightUpdate (next, prepared).ok ());
        assert (sameSnapshot (before, model.snapshot ()));
        adk::ClueConstraintModel::PreparedUpdate changed = prepared;
        changed.ownerToken                               = nullptr;
        model.applyPreparedUpdate (changed);

        assert (sameSnapshot (before, model.snapshot ()));
        changed = prepared;
        ++changed.lifecycleGeneration;
        model.applyPreparedUpdate (changed);

        assert (sameSnapshot (before, model.snapshot ()));
        changed = prepared;
        ++changed.preparationGeneration;
        model.applyPreparedUpdate (changed);

        assert (sameSnapshot (before, model.snapshot ()));
        ++next.observations[0].sourceSequence;
        model.applyPreparedUpdate (prepared);

        assert (model.snapshot ().generation == before.generation + 1U);
        assert (model.evidence (0).value ().sourceSequence == 11);

        const adk::ClueConstraintSnapshot applied = model.snapshot ();

        model.applyPreparedUpdate (prepared);

        assert (sameSnapshot (applied, model.snapshot ()));
    }

    void testLifecycleExhaustionAndReconstruction ()
    {
        adk::ClueConstraintConfig config = baseConfig (1, 1);

        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());
        model.lifecycleGeneration_ = UINT32_MAX;
        model.reset ();

        assert (!model.initialized ());
        assert (model.lifecycleExhausted_);
        assert (model.snapshot ().status.error () == adk::StatusCode::CapacityExceeded);
        assert (model.update (emptyUpdate (1)).error () ==
                adk::StatusCode::NotInitialized);

        adk::ClueConstraintModel shutdownModel (config);

        assert (shutdownModel.initialize ().ok ());
        shutdownModel.lifecycleGeneration_ = UINT32_MAX;
        shutdownModel.shutdown ();

        assert (!shutdownModel.initialized ());
        assert (shutdownModel.snapshot ().status.error () ==
                adk::StatusCode::CapacityExceeded);

        alignas (adk::ClueConstraintModel) unsigned char
                                  storage[sizeof (adk::ClueConstraintModel)];
        adk::ClueConstraintModel* first =
            new (storage) adk::ClueConstraintModel (config);
        assert (first->initialize ().ok ());

        first->~ClueConstraintModel ();
        adk::ClueConstraintModel* second =
            new (storage) adk::ClueConstraintModel (config);
        assert (second->initialize ().ok ());

        second->~ClueConstraintModel ();
    }

    void testPreparationGenerationAndLifecycleInvalidation ()
    {
        adk::ClueConstraintConfig config = baseConfig (1, 1);

        adk::ClueConstraintUpdate valid = emptyUpdate (10);
        valid.observationMask           = 1;
        valid.observations[0]           = observation (0, 1, 10);

        adk::ClueConstraintModel boundary (config);

        assert (boundary.initialize ().ok ());
        boundary.preparationGeneration_ = UINT32_MAX - 1U;
        adk::ClueConstraintModel::PreparedUpdate maximum;
        assert (boundary.preflightUpdate (valid, maximum).ok ());
        assert (maximum.preparationGeneration == UINT32_MAX);
        adk::ClueConstraintModel::PreparedUpdate exhausted;
        assert (boundary.preflightUpdate (valid, exhausted).error () ==
                adk::StatusCode::CapacityExceeded);
        assert (exhausted.ownerToken == nullptr);

        const adk::ClueConstraintSnapshot boundaryBefore = boundary.snapshot ();

        boundary.applyPreparedUpdate (maximum);

        assert (sameSnapshot (boundaryBefore, boundary.snapshot ()));

        adk::ClueConstraintModel failedLater (config);

        assert (failedLater.initialize ().ok ());
        adk::ClueConstraintModel::PreparedUpdate first;
        assert (failedLater.preflightUpdate (valid, first).ok ());
        adk::ClueConstraintUpdate invalid = valid;
        invalid.observationMask           = 2;
        assert (failedLater.preflightUpdate (invalid, exhausted).error () ==
                adk::StatusCode::InvalidArgument);
        const adk::ClueConstraintSnapshot failedBefore = failedLater.snapshot ();

        failedLater.applyPreparedUpdate (first);

        assert (sameSnapshot (failedBefore, failedLater.snapshot ()));

        adk::ClueConstraintModel successfulLater (config);

        assert (successfulLater.initialize ().ok ());
        assert (successfulLater.preflightUpdate (valid, first).ok ());

        valid.observations[0] = observation (0, 2, 11);

        valid.now = adk::MicrosecondTimePoint (11);
        adk::ClueConstraintModel::PreparedUpdate second;
        assert (successfulLater.preflightUpdate (valid, second).ok ());
        const adk::ClueConstraintSnapshot successfulBefore =
            successfulLater.snapshot ();
        successfulLater.applyPreparedUpdate (first);

        assert (sameSnapshot (successfulBefore, successfulLater.snapshot ()));

        successfulLater.applyPreparedUpdate (second);

        assert (successfulLater.snapshot ().generation ==
                successfulBefore.generation + 1U);
        assert (successfulLater.evidence (0).value ().sourceSequence == 2);

        adk::ClueConstraintModel resetModel (config);

        assert (resetModel.initialize ().ok ());

        valid.observations[0] = observation (0, 1, 10);

        valid.now = adk::MicrosecondTimePoint (10);

        assert (resetModel.preflightUpdate (valid, first).ok ());

        resetModel.reset ();

        const adk::ClueConstraintSnapshot resetBefore = resetModel.snapshot ();

        resetModel.applyPreparedUpdate (first);

        assert (sameSnapshot (resetBefore, resetModel.snapshot ()));

        adk::ClueConstraintModel shutdownModel (config);

        assert (shutdownModel.initialize ().ok ());
        assert (shutdownModel.preflightUpdate (valid, first).ok ());

        shutdownModel.shutdown ();

        const adk::ClueConstraintSnapshot shutdownBefore = shutdownModel.snapshot ();

        shutdownModel.applyPreparedUpdate (first);

        assert (sameSnapshot (shutdownBefore, shutdownModel.snapshot ()));
    }

    void testPreparedProposalInspection ()
    {
        adk::ClueConstraintConfig config = baseConfig (2, 1);

        adk::ClueConstraintModel model (config);

        assert (model.initialize ().ok ());

        adk::ClueConstraintUpdate retained = emptyUpdate (10);
        retained.observationMask           = 1;
        retained.observations[0] =
            observation (0, 1, 10, adk::ClueCategory::Low, adk::ClueQuality::Degraded);
        assert (model.update (retained).ok ());

        adk::ClueConstraintUpdate mixed = emptyUpdate (11);
        mixed.observationMask           = 2;
        mixed.observations[1]           = observation (
            1, 4, 11, adk::ClueCategory::High, adk::ClueQuality::Contradictory,
            adk::Status (adk::StatusCode::HardwareFailure));
        adk::ClueConstraintModel::PreparedUpdate prepared;
        assert (model.preflightUpdate (mixed, prepared).ok ());
        const adk::Result<adk::ClueConstraintSnapshot> proposed =
            model.preparedSnapshot (prepared);
        assert (proposed.ok ());
        assert (proposed.value ().generation == model.snapshot ().generation + 1U);
        const adk::Result<adk::ClueEvidenceSnapshot> proposedRetained =
            model.preparedEvidence (prepared, 0);
        const adk::Result<adk::ClueEvidenceSnapshot> proposedNew =
            model.preparedEvidence (prepared, 1);
        assert (proposedRetained.ok ());
        assert (sameEvidence (proposedRetained.value (), model.evidence (0).value ()));
        assert (proposedNew.ok ());
        assert (proposedNew.value ().category == adk::ClueCategory::High);
        assert (proposedNew.value ().quality == adk::ClueQuality::Contradictory);
        assert (proposedNew.value ().sourceSequence == 4);
        assert (sameSource (proposedNew.value ().source, source (1)));
        assert (proposedNew.value ().observedAt.microseconds () == 11);
        assert (proposedNew.value ().status.error () ==
                adk::StatusCode::HardwareFailure);

        model.applyPreparedUpdate (prepared);

        assert (sameSnapshot (proposed.value (), model.snapshot ()));
        assert (sameEvidence (proposedNew.value (), model.evidence (1).value ()));
        assert (model.preparedEvidence (prepared, 1).error () ==
                adk::StatusCode::InvalidArgument);
        assert (model.preparedSnapshot (prepared).error () ==
                adk::StatusCode::InvalidArgument);

        adk::ClueConstraintConfig packedConfig = baseConfig (1, 1);

        adk::ClueConstraintModel packed (packedConfig);

        assert (packed.initialize ().ok ());
        uint32_t sequence = 1;
        for (uint8_t category = 0; category < 8; ++category)
        {
            for (uint8_t quality = 0; quality < 7; ++quality)
            {
                adk::ClueConstraintUpdate update = emptyUpdate (sequence);
                update.observationMask           = 1;
                update.observations[0]           = observation (
                    0, sequence, sequence, static_cast<adk::ClueCategory> (category),
                    static_cast<adk::ClueQuality> (quality));
                assert (packed.preflightUpdate (update, prepared).ok ());
                const adk::Result<adk::ClueEvidenceSnapshot> proposal =
                    packed.preparedEvidence (prepared, 0);
                assert (proposal.ok ());
                assert (proposal.value ().category ==
                        static_cast<adk::ClueCategory> (category));
                assert (proposal.value ().quality ==
                        static_cast<adk::ClueQuality> (quality));
                packed.applyPreparedUpdate (prepared);

                assert (sameEvidence (proposal.value (), packed.evidence (0).value ()));
                ++sequence;
            }
        }

        adk::ClueConstraintUpdate valid = emptyUpdate (sequence);
        valid.observationMask           = 1;
        valid.observations[0]           = observation (0, sequence, sequence);

        assert (packed.preflightUpdate (valid, prepared).ok ());

        const adk::ClueConstraintSnapshot retainedSnapshot = packed.snapshot ();

        const adk::ClueEvidenceSnapshot retainedEvidence = packed.evidence (0).value ();
        for (uint8_t corruption = 0; corruption < 10; ++corruption)
        {
            adk::ClueConstraintModel::PreparedUpdate invalid = prepared;
            if (corruption == 0)
            {
                invalid.ownerToken = nullptr;
            }
            else if (corruption == 1)
            {
                ++invalid.lifecycleGeneration;
            }
            else if (corruption == 2)
            {
                ++invalid.preparationGeneration;
            }
            else if (corruption == 3)
            {
                ++invalid.proposedSnapshot.generation;
            }
            else if (corruption == 4)
            {
                ++invalid.proposedSnapshot.configurationRevision;
            }
            else if (corruption == 5)
            {
                ++invalid.proposedSnapshot.instanceEpoch;
            }
            else if (corruption == 6)
            {
                ++invalid.proposedSnapshot.satisfiedRuleMask;
            }
            else if (corruption == 7)
            {
                ++invalid.proposedSnapshot.blockedRuleMask;
            }
            else if (corruption == 8)
            {
                invalid.proposedSnapshot.disposition =
                    adk::ClueModelDisposition::InternalFault;
            }
            else
            {
                invalid.proposedSnapshot.status =
                    adk::Status (adk::StatusCode::InternalInvariant);
            }
            const adk::Result<adk::ClueEvidenceSnapshot> evidence =
                packed.preparedEvidence (invalid, 0);
            assert (evidence.error () == adk::StatusCode::InvalidArgument);

            assertCanonicalEvidence (evidence.value ());
            const adk::Result<adk::ClueConstraintSnapshot> snapshot =
                packed.preparedSnapshot (invalid);
            assert (snapshot.error () == adk::StatusCode::InvalidArgument);
            assert (snapshot.value ().configurationRevision == 0);
            assert (snapshot.value ().instanceEpoch == 0);
            assert (snapshot.value ().generation == 0);
            assert (snapshot.value ().satisfiedRuleMask == 0);
            assert (snapshot.value ().blockedRuleMask == 0);
            assert (snapshot.value ().disposition ==
                    adk::ClueModelDisposition::Uninitialized);
            assert (snapshot.value ().status.ok ());

            packed.applyPreparedUpdate (invalid);

            assert (sameSnapshot (retainedSnapshot, packed.snapshot ()));
            assert (sameEvidence (retainedEvidence, packed.evidence (0).value ()));
        }

        adk::ClueConstraintModel foreign (packedConfig);

        assert (foreign.initialize ().ok ());
        const adk::Result<adk::ClueEvidenceSnapshot> foreignEvidence =
            foreign.preparedEvidence (prepared, 0);
        assert (foreignEvidence.error () == adk::StatusCode::InvalidArgument);

        assertCanonicalEvidence (foreignEvidence.value ());
        const adk::Result<adk::ClueConstraintSnapshot> foreignSnapshot =
            foreign.preparedSnapshot (prepared);
        assert (foreignSnapshot.error () == adk::StatusCode::InvalidArgument);

        const adk::Result<adk::ClueEvidenceSnapshot> invalidId =
            packed.preparedEvidence (prepared, 1);
        assert (invalidId.error () == adk::StatusCode::InvalidArgument);

        assertCanonicalEvidence (invalidId.value ());
    }

} // namespace

int main ()
{
    void (*const allTests[])() = {
        testTypeAndLifecycleContract,
        testConfigurationBoundaries,
        testExhaustiveFourNodeDagOracle,
        testAllObservationMasksAndSlotIdentity,
        testEvaluationPrecedence,
        testFreshnessSequenceIdentityAndAtomicity,
        testTimeEdges,
        testInvalidQueriesAndReplay,
        testPrerequisiteEnumAndQualityBoundaries,
        testSourceAtomicityAndPreparedCapability,
        testLifecycleExhaustionAndReconstruction,
        testPreparationGenerationAndLifecycleInvalidation,
        testPreparedProposalInspection};
    (void) allTests;

#if ADK_CLUE_CONSTRAINT_TEST_PART == 0 || ADK_CLUE_CONSTRAINT_TEST_PART == 1
    testTypeAndLifecycleContract ();

    testConfigurationBoundaries ();

    testExhaustiveFourNodeDagOracle ();

    testAllObservationMasksAndSlotIdentity ();

    testEvaluationPrecedence ();

    testFreshnessSequenceIdentityAndAtomicity ();

    testTimeEdges ();
#endif

#if ADK_CLUE_CONSTRAINT_TEST_PART == 0 || ADK_CLUE_CONSTRAINT_TEST_PART == 2
    testInvalidQueriesAndReplay ();

    testPrerequisiteEnumAndQualityBoundaries ();

    testSourceAtomicityAndPreparedCapability ();

    testLifecycleExhaustionAndReconstruction ();

    testPreparationGenerationAndLifecycleInvalidation ();

    testPreparedProposalInspection ();
#endif
}
