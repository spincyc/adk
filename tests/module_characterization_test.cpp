#include <assert.h>
#include <stdint.h>

#include "module_characterization.h"

namespace {

    adk::ModuleThresholdDescriptor descriptor ()
    {
        return {1,
                0x12345678UL,
                2,
                0x87654321UL,
                3,
                4,
                adk::ModuleChannelTopology::AnalogAndComparator,
                adk::ModuleComparatorOutputStage::OpenDrain,
                adk::ModulePullRequirement::PullUp,
                adk::ModuleDeclaredRail::LogicSupply,
                {3300, 5000},
                {0, 3300},
                {0, 1023},
                adk::ModuleComparatorPolarity::ActiveLow,
                adk::ModuleThresholdControlKind::Potentiometer,
                adk::ModuleThresholdDirection::IncreasingClockwise,
                {adk::ModuleDurationDeclaration::Known, adk::Duration (0)},
                {adk::ModuleDurationDeclaration::Known, adk::Duration (0)}};
    }

    adk::ModuleCharacterizationConfig config (uint8_t required = 3)
    {
        return {5, descriptor (), required, adk::Duration (20), adk::Duration (10)};
    }

    adk::ModuleThresholdFrame frame (uint32_t sequence, uint32_t observedAt,
                                     uint16_t raw, bool asserted)
    {
        const adk::ModuleThresholdDescriptor value = descriptor ();
        return {value.schemaRevision,
                value.descriptorId,
                value.descriptorRevision,
                value.declaredSpecimenReference,
                value.declaredSpecimenRevision,
                value.declaredElectricalEvidenceRevision,
                {7, 8, sequence, adk::TimePoint (observedAt)},
                raw,
                adk::ModuleChannelStatus::Current,
                !asserted,
                adk::ModuleChannelStatus::Current,
                true,
                asserted,
                true,
                true,
                adk::StatusCode::Ok,
                adk::StatusCode::Ok};
    }

    adk::ModuleCharacterizationPoint point (uint32_t legId, uint16_t ordinal,
                                            adk::ModuleCharacterizationLeg leg,
                                            adk::ModuleSweepDirection      direction,
                                            uint32_t sequence, uint32_t observedAt,
                                            uint16_t raw, bool asserted)
    {
        return {11,      12,  legId,
                ordinal, leg, direction,
                7,       8,   frame (sequence, observedAt, raw, asserted)};
    }

    adk::ModuleCharacterizationPoint
    polarizedPoint (uint32_t legId, uint16_t ordinal,
                    adk::ModuleCharacterizationLeg leg,
                    adk::ModuleSweepDirection direction, uint32_t sequence,
                    uint32_t observedAt, uint16_t raw, bool asserted,
                    bool activeHigh)
    {
        adk::ModuleCharacterizationPoint value =
            point (legId, ordinal, leg, direction, sequence, observedAt, raw,
                   asserted);
        value.frame.comparatorLevelHigh =
            activeHigh ? asserted : !asserted;
        return value;
    }

    adk::ModuleCharacterizationEvidence
    evidence (const adk::ModuleCharacterizationPolicy& policy)
    {
        adk::ModuleCharacterizationEvidence value;
        assert (policy.evidence (value).ok ());
        return value;
    }

    void begin (adk::ModuleCharacterizationPolicy& policy)
    {
        assert (policy.initialize (adk::TimePoint (0)).ok ());
        assert (policy.beginSession (adk::TimePoint (0), 11, 12).ok ());
    }

    void beginAscending (adk::ModuleCharacterizationPolicy& policy, uint32_t legId = 21)
    {
        assert (policy
                    .beginLeg (adk::TimePoint (0), legId,
                               adk::ModuleCharacterizationLeg::Ascending,
                               adk::ModuleSweepDirection::Increasing)
                    .ok ());
    }

    void observeAscending (adk::ModuleCharacterizationPolicy& policy)
    {
        assert (policy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2, 400,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (3),
                              point (21, 3, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 102, 3,
                                     1023, true))
                    .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (3)).ok ());
    }

    void observeDescending (adk::ModuleCharacterizationPolicy& policy)
    {
        assert (policy
                    .beginLeg (adk::TimePoint (4), 22,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (4),
                              point (22, 1, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 103, 4,
                                     1023, true))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (5),
                              point (22, 2, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 104, 5, 500,
                                     true))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (6),
                              point (22, 3, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 105, 6, 0,
                                     false))
                    .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (6)).ok ());
    }

    void testConfigurationAndLifecycle ()
    {
        for (uint8_t required = 0; required <= 17; ++required)
        {
            adk::ModuleCharacterizationPolicy policy     (config (required));
            const adk::Status status = policy.initialize (adk::TimePoint (0));
            assert                                       (status.ok () == (required >= 2 && required <= 16));
            assert                                       (status.error () == (required >= 2 && required <= 16
                                            ? adk::StatusCode::Ok
                                            : adk::StatusCode::InvalidConfiguration));
        }

        adk::ModuleCharacterizationConfig invalid = config ();
        invalid.descriptor.channelTopology = adk::ModuleChannelTopology::AnalogOnly;
        adk::ModuleCharacterizationPolicy analogOnly (invalid);
        assert                                       (analogOnly.initialize (adk::TimePoint (0)).error () ==
                adk::StatusCode::InvalidConfiguration);

        invalid = config ();
        invalid.descriptor.comparatorPolarity =
            adk::ModuleComparatorPolarity::Unspecified;
        adk::ModuleCharacterizationPolicy unspecifiedPolarity (invalid);
        assert                                                (unspecifiedPolarity.initialize (adk::TimePoint (0)).error () ==
                adk::StatusCode::InvalidConfiguration);

        invalid            = config               ();
        invalid.maximumAge = adk::Duration        (0);
        adk::ModuleCharacterizationPolicy zeroAge (invalid);
        assert                                    (zeroAge.initialize (adk::TimePoint (0)).error () ==
                adk::StatusCode::InvalidConfiguration);

        invalid            = config               ();
        invalid.maximumGap = adk::Duration        (0);
        adk::ModuleCharacterizationPolicy zeroGap (invalid);
        assert                                    (zeroGap.initialize (adk::TimePoint (0)).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::ModuleCharacterizationPolicy policy (config ());
        assert                                   (policy.beginSession (adk::TimePoint (0), 11, 12).error () ==
                adk::StatusCode::NotInitialized);
        assert (policy.initialize (adk::TimePoint (0)).ok ());
        assert (policy.beginSession (adk::TimePoint (0), 0, 12).error () ==
                adk::StatusCode::InvalidArgument);
        assert (policy.beginSession (adk::TimePoint (0), 11, 0).error () ==
                adk::StatusCode::InvalidArgument);
        assert (policy.beginSession (adk::TimePoint (0), 11, 12).ok ());
        assert (policy
                    .beginLeg (adk::TimePoint (0), 21,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .error () == adk::StatusCode::InvalidArgument);
        assert (policy.shutdown (adk::TimePoint (0)).ok ());
        assert (policy.reset (adk::TimePoint (0)).error () ==
                adk::StatusCode::NotInitialized);
    }

    void testThreeLegCompletion ()
    {
        adk::ModuleCharacterizationPolicy policy (config ());
        begin                                    (policy);
        beginAscending                           (policy);
        observeAscending                         (policy);
        observeDescending                        (policy);
        assert                                   (policy
                    .beginLeg (adk::TimePoint (7), 23,
                               adk::ModuleCharacterizationLeg::Verification,
                               adk::ModuleSweepDirection::Unordered)
                    .ok ());
        assert (
            policy
                .observe (adk::TimePoint (7),
                          point (23, 1, adk::ModuleCharacterizationLeg::Verification,
                                 adk::ModuleSweepDirection::Unordered, 106, 7, 250,
                                 false))
                .ok ());
        assert (policy
                    .observe (
                        adk::TimePoint (8),
                        point          (23, 2, adk::ModuleCharacterizationLeg::Verification,
                               adk::ModuleSweepDirection::Unordered, 107, 8, 550, true))
                    .ok ());
        assert (
            policy
                .observe (adk::TimePoint (9),
                          point (23, 3, adk::ModuleCharacterizationLeg::Verification,
                                 adk::ModuleSweepDirection::Unordered, 108, 9, 450,
                                 false))
                .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (9)).ok ());

        const adk::ModuleCharacterizationEvidence value = evidence (policy);
        assert                                                     (value.state == adk::ModuleCharacterizationState::Complete);
        assert                                                     (value.reason == adk::ModuleCharacterizationReason::None);
        assert                                                     (value.ascendingCount == 3);
        assert                                                     (value.descendingCount == 3);
        assert                                                     (value.verificationCount == 3);
        assert                                                     (value.ascendingBracket.present);
        assert                                                     (value.ascendingBracket.before.frame.analogRaw == 400);
        assert                                                     (value.ascendingBracket.after.frame.analogRaw == 1023);
        assert                                                     (value.descendingBracket.present);
        assert                                                     (value.descendingBracket.before.frame.analogRaw == 500);
        assert                                                     (value.descendingBracket.after.frame.analogRaw == 0);
        assert                                                     (value.guaranteedInactiveInterval.present);
        assert                                                     (value.guaranteedInactiveInterval.lower == 0);
        assert                                                     (value.guaranteedInactiveInterval.upper == 0);
        assert                                                     (value.guaranteedActiveInterval.present);
        assert                                                     (value.guaranteedActiveInterval.lower == 1023);
        assert                                                     (value.guaranteedActiveInterval.upper == 1023);
        assert                                                     (value.ambiguityInterval.present);
        assert                                                     (value.ambiguityInterval.lower == 1);
        assert                                                     (value.ambiguityInterval.upper == 1022);
        assert                                                     (value.relation == adk::ModuleComparatorRelation::Ambiguous);
    }

    void testApiFailuresAreAtomic ()
    {
        adk::ModuleCharacterizationPolicy policy (config ());
        begin                                    (policy);
        beginAscending                           (policy);
        const adk::ModuleCharacterizationPoint first =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 200, false);
        assert                                                      (policy.observe (adk::TimePoint (1), first).ok ());
        const adk::ModuleCharacterizationEvidence before = evidence (policy);

        adk::ModuleCharacterizationPoint invalid = first;
        invalid.controlOrdinal                   = 3;
        invalid.frame.provenance.sequence        = 102;
        invalid.frame.provenance.observedAt      = adk::TimePoint (2);
        assert                                                    (policy.observe (adk::TimePoint (2), invalid).error () ==
                adk::StatusCode::InvalidArgument);
        const adk::ModuleCharacterizationEvidence after = evidence (policy);
        assert                                                     (after.ascendingCount == before.ascendingCount);
        assert                                                     (after.lastWitness.sequence == before.lastWitness.sequence);
        assert                                                     (after.lastWitness.observedAt == before.lastWitness.observedAt);
        assert                                                     (after.state == before.state);
        assert                                                     (after.reason == before.reason);

        assert (policy.finalizeLeg (adk::TimePoint (2)).error () ==
                adk::StatusCode::InvalidArgument);
        const adk::ModuleCharacterizationEvidence finalized = evidence (policy);
        assert                                                         (finalized.ascendingCount == before.ascendingCount);
        assert                                                         (finalized.state == before.state);
    }

    void testIdenticalDuplicateAndChangedDuplicate ()
    {
        adk::ModuleCharacterizationPolicy policy (config ());
        begin                                    (policy);
        beginAscending                           (policy);
        const adk::ModuleCharacterizationPoint first =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 200, false);
        assert (policy.observe (adk::TimePoint (1), first).ok ());
        assert (policy.observe (adk::TimePoint (1), first).ok ());
        assert (evidence (policy).ascendingCount == 1);

        adk::ModuleCharacterizationPoint changed = first;
        changed.frame.analogRaw                  = 201;
        assert (policy.observe (adk::TimePoint (1), changed).error () ==
                adk::StatusCode::InvalidArgument);
        assert (evidence (policy).ascendingCount == 1);
    }

    void testFailurePrecedence ()
    {
        adk::ModuleCharacterizationPolicy producerPolicy (config ());
        begin                                            (producerPolicy);
        beginAscending                                   (producerPolicy);
        adk::ModuleCharacterizationPoint producer =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 200, false);
        producer.frame.analogProducerStatus     = adk::StatusCode::HardwareFailure;
        producer.frame.comparatorProducerStatus = adk::StatusCode::HardwareFailure;
        producer.frame.analogStatus     = adk::ModuleChannelStatus::ProducerFault;
        producer.frame.comparatorStatus = adk::ModuleChannelStatus::ProducerFault;
        assert (producerPolicy.observe (adk::TimePoint (1), producer).ok ());
        assert (evidence (producerPolicy).reason ==
                adk::ModuleCharacterizationReason::ProducerFault);

        adk::ModuleCharacterizationPolicy sequencePolicy (config ());
        begin                                            (sequencePolicy);
        beginAscending                                   (sequencePolicy);
        assert                                           (sequencePolicy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 300,
                                     false))
                    .ok ());
        const adk::ModuleCharacterizationPoint gap =
            point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 102, 2, 200, true);
        assert (sequencePolicy.observe (adk::TimePoint (2), gap).ok ());
        assert (evidence (sequencePolicy).reason ==
                adk::ModuleCharacterizationReason::SequenceDiscontinuity);

        adk::ModuleCharacterizationPolicy warmupPolicy (config ());
        begin                                          (warmupPolicy);
        beginAscending                                 (warmupPolicy);
        adk::ModuleCharacterizationPoint unwarmed =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 200, false);
        unwarmed.frame.declaredWarmupSatisfied = false;
        assert (warmupPolicy.observe (adk::TimePoint (1), unwarmed).ok ());
        assert (evidence (warmupPolicy).reason ==
                adk::ModuleCharacterizationReason::WarmupUnsatisfied);

        adk::ModuleCharacterizationPolicy settlingPolicy (config ());
        begin                                            (settlingPolicy);
        beginAscending                                   (settlingPolicy);
        adk::ModuleCharacterizationPoint unsettled =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 200, false);
        unsettled.frame.declaredSettlingSatisfied = false;
        assert (settlingPolicy.observe (adk::TimePoint (1), unsettled).ok ());
        assert (evidence (settlingPolicy).reason ==
                adk::ModuleCharacterizationReason::SettlingUnsatisfied);
    }

    void testDirectionAndChatter ()
    {
        adk::ModuleCharacterizationPolicy directionPolicy (config ());
        begin                                             (directionPolicy);
        beginAscending                                    (directionPolicy);
        assert                                            (directionPolicy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 400,
                                     false))
                    .ok ());
        assert (directionPolicy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2, 300,
                                     true))
                    .ok ());
        assert (evidence (directionPolicy).reason ==
                adk::ModuleCharacterizationReason::DirectionViolation);

        adk::ModuleCharacterizationPolicy chatterPolicy (config ());
        begin                                           (chatterPolicy);
        beginAscending                                  (chatterPolicy);
        assert                                          (chatterPolicy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 300,
                                     false))
                    .ok ());
        assert (chatterPolicy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2, 400,
                                     true))
                    .ok ());
        assert (chatterPolicy
                    .observe (adk::TimePoint (3),
                              point (21, 3, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 102, 3, 500,
                                     false))
                    .ok ());
        assert (evidence (chatterPolicy).reason ==
                adk::ModuleCharacterizationReason::Chatter);
    }

    void testCapacityBoundary ()
    {
        adk::ModuleCharacterizationPolicy policy (config (2));
        begin                                    (policy);
        beginAscending                           (policy);
        assert                                   (policy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     false))
                    .ok ());
        const adk::ModuleCharacterizationPoint second =
            point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 101, 2, 1023, true);
        assert                                                           (policy.observe (adk::TimePoint (2), second).ok ());
        const adk::ModuleCharacterizationEvidence before = evidence      (policy);
        assert                                                           (policy.observe (adk::TimePoint (2), second).ok ());
        const adk::ModuleCharacterizationEvidence afterReplay = evidence (policy);
        assert                                                           (afterReplay.ascendingCount == before.ascendingCount);
        assert                                                           (afterReplay.lastWitness.sequence == before.lastWitness.sequence);
        assert                                                           (afterReplay.lastWitness.observedAt == before.lastWitness.observedAt);
        assert                                                           (afterReplay.reason == before.reason);

        adk::ModuleCharacterizationPoint changed = second;
        changed.frame.analogRaw                  = 1022;
        assert (policy.observe (adk::TimePoint (2), changed).error () ==
                adk::StatusCode::InvalidArgument);
        const adk::ModuleCharacterizationEvidence afterChanged = evidence (policy);
        assert                                                            (afterChanged.ascendingCount == before.ascendingCount);
        assert                                                            (afterChanged.lastWitness.sequence == before.lastWitness.sequence);
        assert                                                            (afterChanged.lastWitness.observedAt == before.lastWitness.observedAt);
        assert                                                            (afterChanged.reason == before.reason);

        assert (policy
                    .observe (adk::TimePoint (3),
                              point (21, 3, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 102, 3, 700,
                                     true))
                    .error () == adk::StatusCode::CapacityExceeded);
        const adk::ModuleCharacterizationEvidence after = evidence (policy);
        assert                                                     (after.ascendingCount == before.ascendingCount);
        assert                                                     (after.lastWitness.sequence == before.lastWitness.sequence);
        assert                                                     (after.reason == before.reason);
    }

    void testRolloverSequenceIsContiguous ()
    {
        adk::ModuleCharacterizationPolicy policy (config (2));
        assert                                   (policy.initialize (adk::TimePoint (UINT32_MAX - 2)).ok ());
        assert                                   (policy.beginSession (adk::TimePoint (UINT32_MAX - 2), 11, 12).ok ());
        assert                                   (policy
                    .beginLeg (adk::TimePoint (UINT32_MAX - 2), 21,
                               adk::ModuleCharacterizationLeg::Ascending,
                               adk::ModuleSweepDirection::Increasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (UINT32_MAX),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, UINT32_MAX,
                                     UINT32_MAX, 0, false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (0),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 0, 0, 1023,
                                     true))
                    .ok ());
        assert (evidence (policy).ascendingCount == 2);
    }

    void testLearningEndpointPrecedence ()
    {
        adk::ModuleCharacterizationPolicy missingEndpoint (config (2));
        begin                                             (missingEndpoint);
        beginAscending                                    (missingEndpoint);
        assert                                            (missingEndpoint
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 1,
                                     false))
                    .ok ());
        assert (missingEndpoint
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2,
                                     1023, true))
                    .ok ());
        assert (missingEndpoint.finalizeLeg (adk::TimePoint (2)).ok ());
        assert (evidence (missingEndpoint).reason ==
                adk::ModuleCharacterizationReason::DirectionViolation);

        adk::ModuleCharacterizationPolicy lowerRail (config (2));
        begin                                       (lowerRail);
        beginAscending                              (lowerRail);
        assert                                      (lowerRail
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     false))
                    .ok ());
        assert (lowerRail
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2, 0,
                                     false))
                    .ok ());
        assert (lowerRail.finalizeLeg (adk::TimePoint (2)).ok ());
        assert (evidence (lowerRail).reason ==
                adk::ModuleCharacterizationReason::AtLowerRail);
    }

    void testLifecycleChronology ()
    {
        {
            adk::ModuleCharacterizationPolicy policy                    (config (2));
            assert                                                      (policy.initialize (adk::TimePoint (10)).ok ());
            const adk::ModuleCharacterizationEvidence before = evidence (policy);
            assert                                                      (policy.beginSession (adk::TimePoint (9), 11, 12).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (policy.beginSession (adk::TimePoint (10 + 0x80000000UL), 11, 12)
                        .error () == adk::StatusCode::InvalidArgument);
            const adk::ModuleCharacterizationEvidence after = evidence (policy);
            assert                                                     (after.state == before.state);
            assert                                                     (after.sessionId == before.sessionId);
            assert                                                     (policy.beginSession (adk::TimePoint (10), 11, 12).ok ());

            const adk::ModuleCharacterizationEvidence beforeLeg = evidence (policy);
            assert                                                         (policy
                        .beginLeg (adk::TimePoint (9), 21,
                                   adk::ModuleCharacterizationLeg::Ascending,
                                   adk::ModuleSweepDirection::Increasing)
                        .error () == adk::StatusCode::InvalidArgument);
            assert (policy
                        .beginLeg (adk::TimePoint (10 + 0x80000000UL), 21,
                                   adk::ModuleCharacterizationLeg::Ascending,
                                   adk::ModuleSweepDirection::Increasing)
                        .error () == adk::StatusCode::InvalidArgument);
            const adk::ModuleCharacterizationEvidence afterLeg = evidence (policy);
            assert                                                        (afterLeg.state == beforeLeg.state);
            assert                                                        (afterLeg.legId == beforeLeg.legId);
            assert                                                        (policy
                        .beginLeg (adk::TimePoint (10), 21,
                                   adk::ModuleCharacterizationLeg::Ascending,
                                   adk::ModuleSweepDirection::Increasing)
                        .ok ());
        }

        {
            adk::ModuleCharacterizationPolicy policy (config (2));
            assert                                   (policy.initialize (adk::TimePoint (10)).ok ());
            assert                                   (policy.beginSession (adk::TimePoint (10), 11, 12).ok ());
            assert                                   (policy
                        .beginLeg (adk::TimePoint (10), 21,
                                   adk::ModuleCharacterizationLeg::Ascending,
                                   adk::ModuleSweepDirection::Increasing)
                        .ok ());
            assert (
                policy
                    .observe (adk::TimePoint (10),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 10, 0,
                                     false))
                    .ok ());
            assert (
                policy
                    .observe (adk::TimePoint (11),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 11,
                                     1023, true))
                    .ok ());
            const adk::ModuleCharacterizationEvidence before = evidence (policy);
            assert                                                      (policy.finalizeLeg (adk::TimePoint (10)).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (policy.finalizeLeg (adk::TimePoint (11 + 0x80000000UL)).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (evidence (policy).ascendingCount == before.ascendingCount);
            assert (policy.finalizeLeg (adk::TimePoint (11)).ok ());
        }

        {
            adk::ModuleCharacterizationPolicy policy                    (config (2));
            assert                                                      (policy.initialize (adk::TimePoint (10)).ok ());
            const adk::ModuleCharacterizationEvidence before = evidence (policy);
            assert                                                      (policy.reset (adk::TimePoint (9)).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (policy.reset (adk::TimePoint (10 + 0x80000000UL)).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (evidence (policy).lifecycleGeneration ==
                    before.lifecycleGeneration);
            assert (policy.reset (adk::TimePoint (10)).ok ());
        }

        {
            adk::ModuleCharacterizationPolicy policy                    (config (2));
            assert                                                      (policy.initialize (adk::TimePoint (10)).ok ());
            const adk::ModuleCharacterizationEvidence before = evidence (policy);
            assert                                                      (policy.shutdown (adk::TimePoint (9)).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (policy.shutdown (adk::TimePoint (10 + 0x80000000UL)).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (evidence (policy).state == before.state);
            assert (policy.shutdown (adk::TimePoint (10)).ok ());
            assert (policy.initialize (adk::TimePoint (10)).error () ==
                    adk::StatusCode::NotInitialized);

            adk::ModuleCharacterizationPolicy replacement (config (2));
            assert                                        (replacement.initialize (adk::TimePoint (10)).ok ());
        }
    }

    void testNoTransitionAndUpperRailOutcomes ()
    {
        for (uint8_t asserted = 0; asserted <= 1; ++asserted)
        {
            adk::ModuleCharacterizationPolicy policy (config (2));
            begin                                    (policy);
            beginAscending                           (policy);
            assert                                   (
                policy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     asserted != 0))
                    .ok ());
            assert (
                policy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2,
                                     1023, asserted != 0))
                    .ok ());
            assert (policy.finalizeLeg (adk::TimePoint (2)).ok ());
            assert (evidence (policy).reason ==
                    (asserted
                         ? adk::ModuleCharacterizationReason::NoObservedTransitionActive
                         : adk::ModuleCharacterizationReason::
                               NoObservedTransitionInactive));
        }

        adk::ModuleCharacterizationPolicy upper (config (2));
        begin                                   (upper);
        beginAscending                          (upper);
        assert                                  (upper
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1,
                                     1023, true))
                    .ok ());
        assert (upper
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2,
                                     1023, true))
                    .ok ());
        assert (upper.finalizeLeg (adk::TimePoint (2)).ok ());
        assert (evidence (upper).reason ==
                adk::ModuleCharacterizationReason::AtUpperRail);
    }

    void testMaximumPointCapacity ()
    {
        adk::ModuleCharacterizationPolicy policy (config (16));
        begin                                    (policy);
        beginAscending                           (policy);
        for (uint8_t index = 0; index < 16; ++index)
        {
            const uint16_t raw =
                index == 15 ? 1023 : static_cast<uint16_t> (index * 64);
            assert (policy
                        .observe (adk::TimePoint (index + 1),
                                  point (21, static_cast<uint16_t> (index + 1),
                                         adk::ModuleCharacterizationLeg::Ascending,
                                         adk::ModuleSweepDirection::Increasing,
                                         static_cast<uint32_t> (100 + index), index + 1,
                                         raw, index >= 8))
                        .ok ());
        }
        assert (evidence (policy).ascendingCount == 16);
        assert (policy
                    .observe (adk::TimePoint (17),
                              point (21, 17, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 116, 17,
                                     1023, true))
                    .error () == adk::StatusCode::CapacityExceeded);
        assert (evidence (policy).ascendingCount == 16);
    }

    void testStaleTimestampAndCorrelationFailures ()
    {
        adk::ModuleCharacterizationPolicy stale (config ());
        begin                                   (stale);
        beginAscending                          (stale);
        assert                                  (stale
                    .observe (adk::TimePoint (22),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     false))
                    .ok ());
        assert (evidence (stale).reason == adk::ModuleCharacterizationReason::Stale);

        adk::ModuleCharacterizationPolicy gap (config ());
        begin                                 (gap);
        beginAscending                        (gap);
        assert                                (gap.observe (adk::TimePoint (1),
                             point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                    adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                    false))
                    .ok ());
        assert (gap.observe (adk::TimePoint (12),
                             point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                    adk::ModuleSweepDirection::Increasing, 101, 12, 500,
                                    false))
                    .ok ());
        assert (evidence (gap).reason ==
                adk::ModuleCharacterizationReason::TimestampDiscontinuity);

        adk::ModuleCharacterizationPolicy drift                     (config ());
        begin                                                       (drift);
        beginAscending                                              (drift);
        const adk::ModuleCharacterizationEvidence before = evidence (drift);
        adk::ModuleCharacterizationPoint          wrong =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 0, false);
        wrong.sourceId = 9;
        assert (drift.observe (adk::TimePoint (1), wrong).error () ==
                adk::StatusCode::InvalidArgument);
        wrong = point (22, 1, adk::ModuleCharacterizationLeg::Ascending,
                       adk::ModuleSweepDirection::Increasing, 100, 1, 0, false);
        assert (drift.observe (adk::TimePoint (1), wrong).error () ==
                adk::StatusCode::InvalidArgument);
        wrong = point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                       adk::ModuleSweepDirection::Increasing, 100, 1, 0, false);
        wrong.frame.descriptorRevision++;
        assert (drift.observe (adk::TimePoint (1), wrong).error () ==
                adk::StatusCode::InvalidArgument);
        const adk::ModuleCharacterizationEvidence after = evidence (drift);
        assert                                                     (after.ascendingCount == before.ascendingCount);
        assert                                                     (after.state == before.state);
        assert                                                     (after.reason == before.reason);
    }

    void testActiveHighSymmetry ()
    {
        adk::ModuleCharacterizationConfig value = config (2);
        value.descriptor.comparatorPolarity = adk::ModuleComparatorPolarity::ActiveHigh;
        adk::ModuleCharacterizationPolicy policy (value);
        begin                                    (policy);
        beginAscending                           (policy);
        adk::ModuleCharacterizationPoint low =
            point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 100, 1, 0, false);
        low.frame.comparatorLevelHigh = false;
        assert (policy.observe (adk::TimePoint (1), low).ok ());
        adk::ModuleCharacterizationPoint high =
            point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                   adk::ModuleSweepDirection::Increasing, 101, 2, 1023, true);
        high.frame.comparatorLevelHigh = true;
        assert (policy.observe (adk::TimePoint (2), high).ok ());
        assert (policy.finalizeLeg (adk::TimePoint (2)).ok ());
        assert (evidence (policy).ascendingBracket.present);
    }

    void testOrientationMismatch ()
    {
        adk::ModuleCharacterizationPolicy policy (config ());
        begin                                    (policy);
        beginAscending                           (policy);
        observeAscending                         (policy);
        assert                                   (policy
                    .beginLeg (adk::TimePoint (4), 22,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (4),
                              point (22, 1, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 103, 4,
                                     1023, false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (5),
                              point (22, 2, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 104, 5, 500,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (6),
                              point (22, 3, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 105, 6, 0,
                                     true))
                    .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (6)).ok ());
        assert (evidence (policy).reason ==
                adk::ModuleCharacterizationReason::TransitionOrientationMismatch);
    }

    void testVerificationRelationOutcomes ()
    {
        adk::ModuleCharacterizationPolicy consistent (config ());
        begin                                        (consistent);
        beginAscending                               (consistent);
        observeAscending                             (consistent);
        observeDescending                            (consistent);
        assert                                       (consistent
                    .beginLeg (adk::TimePoint (7), 23,
                               adk::ModuleCharacterizationLeg::Verification,
                               adk::ModuleSweepDirection::Unordered)
                    .ok ());
        const uint16_t raws[]       = {0, 1023, 0};
        const bool     assertions[] = {false, true, false};
        for (uint8_t index = 0; index < 3; ++index)
        {
            assert (consistent
                        .observe (adk::TimePoint (7 + index),
                                  point (23, static_cast<uint16_t> (index + 1),
                                         adk::ModuleCharacterizationLeg::Verification,
                                         adk::ModuleSweepDirection::Unordered,
                                         static_cast<uint32_t> (106 + index), 7 + index,
                                         raws[index], assertions[index]))
                        .ok ());
        }
        assert (consistent.finalizeLeg (adk::TimePoint (9)).ok ());
        assert (evidence (consistent).relation ==
                adk::ModuleComparatorRelation::Consistent);

        adk::ModuleCharacterizationPolicy disagrees (config ());
        begin                                       (disagrees);
        beginAscending                              (disagrees);
        observeAscending                            (disagrees);
        observeDescending                           (disagrees);
        assert                                      (disagrees
                    .beginLeg (adk::TimePoint (7), 23,
                               adk::ModuleCharacterizationLeg::Verification,
                               adk::ModuleSweepDirection::Unordered)
                    .ok ());
        assert (
            disagrees
                .observe (adk::TimePoint (7),
                          point (23, 1, adk::ModuleCharacterizationLeg::Verification,
                                 adk::ModuleSweepDirection::Unordered, 106, 7, 0, true))
                .ok ());
        const adk::ModuleCharacterizationEvidence result = evidence (disagrees);
        assert                                                      (result.reason ==
                adk::ModuleCharacterizationReason::AnalogComparatorDisagreement);
        assert (result.relation == adk::ModuleComparatorRelation::Disagrees);
    }

    void testEqualCodeChatterAndTerminalImmutability ()
    {
        adk::ModuleCharacterizationPolicy policy (config ());
        begin                                    (policy);
        beginAscending                           (policy);
        assert                                   (policy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2, 0,
                                     true))
                    .ok ());
        const adk::ModuleCharacterizationEvidence rejected = evidence (policy);
        assert                                                        (rejected.reason == adk::ModuleCharacterizationReason::Chatter);
        assert                                                        (policy
                    .observe (adk::TimePoint (3),
                              point (21, 3, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 102, 3,
                                     1023, false))
                    .error () == adk::StatusCode::InvalidArgument);
        const adk::ModuleCharacterizationEvidence after = evidence (policy);
        assert                                                     (after.reason == rejected.reason);
        assert                                                     (after.ascendingCount == rejected.ascendingCount);
        assert                                                     (after.lastWitness.sequence == rejected.lastWitness.sequence);
    }

    void testEvidenceOutputCanaries ()
    {
        struct GuardedEvidence
        {
            uint32_t                            before;
            adk::ModuleCharacterizationEvidence value;
            uint32_t                            after;
        };

        adk::ModuleCharacterizationPolicy policy (config ());
        begin                                    (policy);
        GuardedEvidence guarded;
        guarded.before = 0x55aa33ccUL;
        guarded.after  = 0xaa55cc33UL;
        assert (policy.evidence (guarded.value).ok ());
        assert (guarded.before == 0x55aa33ccUL);
        assert (guarded.after == 0xaa55cc33UL);
        assert (guarded.value.state == adk::ModuleCharacterizationState::Idle);
    }

    void prepareDescending (adk::ModuleCharacterizationPolicy& policy)
    {
        begin            (policy);
        beginAscending   (policy);
        observeAscending (policy);
        assert           (policy
                    .beginLeg (adk::TimePoint (4), 22,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .ok ());
    }

    void requireCrossLegRejection (uint32_t now, uint32_t sequence, uint32_t observedAt,
                                   adk::ModuleCharacterizationReason reason)
    {
        adk::ModuleCharacterizationPolicy policy (config ());
        prepareDescending                        (policy);
        assert                                   (policy
                    .observe (adk::TimePoint (now),
                              point (22, 1, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, sequence,
                                     observedAt, 1023, true))
                    .ok ());
        const adk::ModuleCharacterizationEvidence value = evidence (policy);
        assert                                                     (value.state == adk::ModuleCharacterizationState::Rejected);
        assert                                                     (value.reason == reason);
        assert                                                     (value.descendingCount == 0);
        assert                                                     (!value.lastWitness.present || value.lastWitness.sequence == 102);
    }

    void testCrossLegContinuity ()
    {
        requireCrossLegRejection (
            4, 104, 4, adk::ModuleCharacterizationReason::SequenceDiscontinuity);
        requireCrossLegRejection (
            4, 101, 4, adk::ModuleCharacterizationReason::SequenceDiscontinuity);
        requireCrossLegRejection (
            4, 102 + 0x80000000UL, 4,
            adk::ModuleCharacterizationReason::SequenceDiscontinuity);
        requireCrossLegRejection (
            14, 103, 14, adk::ModuleCharacterizationReason::TimestampDiscontinuity);
        requireCrossLegRejection (
            4, 103, 2, adk::ModuleCharacterizationReason::TimestampDiscontinuity);
        requireCrossLegRejection (
            3 + 0x80000000UL, 103, 3 + 0x80000000UL,
            adk::ModuleCharacterizationReason::TimestampDiscontinuity);

        adk::ModuleCharacterizationPolicy nowRegression             (config ());
        prepareDescending                                           (nowRegression);
        const adk::ModuleCharacterizationEvidence before = evidence (nowRegression);
        assert                                                      (nowRegression
                    .observe (adk::TimePoint (3),
                              point (22, 1, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 103, 4,
                                     1023, true))
                    .error () == adk::StatusCode::InvalidArgument);
        const adk::ModuleCharacterizationEvidence after = evidence (nowRegression);
        assert                                                     (after.state == before.state);
        assert                                                     (after.reason == before.reason);
        assert                                                     (after.descendingCount == before.descendingCount);
        assert                                                     (after.lastWitness.sequence == before.lastWitness.sequence);
    }

    void testTransitionPositionPolarityMatrix ()
    {
        for (uint8_t polarity = 0; polarity < 2; ++polarity)
        {
            for (uint8_t transition = 1; transition <= 2; ++transition)
            {
                adk::ModuleCharacterizationConfig value = config ();
                value.descriptor.comparatorPolarity =
                    polarity == 0 ? adk::ModuleComparatorPolarity::ActiveLow
                                  : adk::ModuleComparatorPolarity::ActiveHigh;
                adk::ModuleCharacterizationPolicy policy (value);
                begin                                    (policy);
                beginAscending                           (policy);
                const uint16_t raws[] = {0, 512, 1023};
                for (uint8_t index = 0; index < 3; ++index)
                {
                    const bool                       asserted = index >= transition;
                    adk::ModuleCharacterizationPoint sample =
                        point (21, static_cast<uint16_t> (index + 1),
                               adk::ModuleCharacterizationLeg::Ascending,
                               adk::ModuleSweepDirection::Increasing,
                               static_cast<uint32_t> (100 + index), index + 1,
                               raws[index], asserted);
                    sample.frame.comparatorLevelHigh =
                        polarity == 0 ? !asserted : asserted;
                    assert (policy.observe (adk::TimePoint (index + 1), sample).ok ());
                }
                assert (policy.finalizeLeg (adk::TimePoint (3)).ok ());
                const adk::ModuleTransitionBracket bracket =
                    evidence (policy).ascendingBracket;
                assert (bracket.present);
                assert (bracket.before.frame.analogRaw == raws[transition - 1]);
                assert (bracket.after.frame.analogRaw == raws[transition]);
                assert (!bracket.before.frame.comparatorAsserted);
                assert (bracket.after.frame.comparatorAsserted);
            }
        }
    }

    uint16_t ascendingRaw (uint8_t index, uint8_t count)
    {
        return static_cast<uint16_t> ((static_cast<uint32_t> (index) * 1023U) /
                                      static_cast<uint32_t> (count - 1));
    }

    uint16_t descendingRaw (uint8_t index, uint8_t count)
    {
        return static_cast<uint16_t> (1023U - (static_cast<uint32_t> (index) * 1023U) /
                                                  static_cast<uint32_t> (count - 1));
    }

    void testExhaustiveTransitionAndIntervalMatrix ()
    {
        for (uint8_t count = 2; count <= 16; ++count)
        {
            for (uint8_t polarity = 0; polarity < 2; ++polarity)
            {
                for (uint8_t ascendingTransition = 1; ascendingTransition < count;
                     ++ascendingTransition)
                {
                    for (uint8_t descendingTransition = 1; descendingTransition < count;
                         ++descendingTransition)
                    {
                        adk::ModuleCharacterizationConfig value = config (count);
                        value.descriptor.comparatorPolarity =
                            polarity == 0 ? adk::ModuleComparatorPolarity::ActiveLow
                                          : adk::ModuleComparatorPolarity::ActiveHigh;
                        adk::ModuleCharacterizationPolicy policy (value);
                        begin                                    (policy);
                        beginAscending                           (policy);
                        uint32_t sequence = 100;
                        uint32_t tick     = 1;
                        for (uint8_t index = 0; index < count; ++index)
                        {
                            const bool asserted = index >= ascendingTransition;
                            adk::ModuleCharacterizationPoint sample = point (
                                21, static_cast<uint16_t> (index + 1),
                                adk::ModuleCharacterizationLeg::Ascending,
                                adk::ModuleSweepDirection::Increasing, sequence++, tick,
                                ascendingRaw (index, count), asserted);
                            sample.frame.comparatorLevelHigh =
                                polarity == 0 ? !asserted : asserted;
                            assert (
                                policy.observe (adk::TimePoint (tick++), sample).ok ());
                        }
                        assert (policy.finalizeLeg (adk::TimePoint (tick - 1)).ok ());
                        assert (
                            policy
                                .beginLeg (adk::TimePoint (tick), 22,
                                           adk::ModuleCharacterizationLeg::Descending,
                                           adk::ModuleSweepDirection::Decreasing)
                                .ok ());
                        for (uint8_t index = 0; index < count; ++index)
                        {
                            const bool asserted = index < descendingTransition;
                            adk::ModuleCharacterizationPoint sample = point (
                                22, static_cast<uint16_t> (index + 1),
                                adk::ModuleCharacterizationLeg::Descending,
                                adk::ModuleSweepDirection::Decreasing, sequence++, tick,
                                descendingRaw (index, count), asserted);
                            sample.frame.comparatorLevelHigh =
                                polarity == 0 ? !asserted : asserted;
                            assert (
                                policy.observe (adk::TimePoint (tick++), sample).ok ());
                        }
                        assert (policy.finalizeLeg (adk::TimePoint (tick - 1)).ok ());

                        const adk::ModuleCharacterizationEvidence result =
                            evidence (policy);
                        assert (result.reason ==
                                adk::ModuleCharacterizationReason::None);
                        assert (result.ascendingBracket.present);
                        assert (result.descendingBracket.present);
                        assert (result.ascendingBracket.before.frame.analogRaw ==
                                ascendingRaw (ascendingTransition - 1, count));
                        assert (result.ascendingBracket.after.frame.analogRaw ==
                                ascendingRaw (ascendingTransition, count));
                        assert (result.descendingBracket.before.frame.analogRaw ==
                                descendingRaw (descendingTransition - 1, count));
                        assert (result.descendingBracket.after.frame.analogRaw ==
                                descendingRaw (descendingTransition, count));

                        const uint16_t lowProved =
                            result.ascendingBracket.before.frame.analogRaw <
                                    result.descendingBracket.after.frame.analogRaw
                                ? result.ascendingBracket.before.frame.analogRaw
                                : result.descendingBracket.after.frame.analogRaw;
                        const uint16_t highProved =
                            result.ascendingBracket.after.frame.analogRaw >
                                    result.descendingBracket.before.frame.analogRaw
                                ? result.ascendingBracket.after.frame.analogRaw
                                : result.descendingBracket.before.frame.analogRaw;
                        assert (result.guaranteedInactiveInterval.present);
                        assert (result.guaranteedInactiveInterval.lower == 0);
                        assert (result.guaranteedInactiveInterval.upper == lowProved);
                        assert (result.guaranteedActiveInterval.present);
                        assert (result.guaranteedActiveInterval.lower == highProved);
                        assert (result.guaranteedActiveInterval.upper == 1023);
                        const bool ambiguityPresent =
                            static_cast<uint32_t> (lowProved) + 1U <=
                            static_cast<uint32_t> (highProved) - 1U;
                        assert (result.ambiguityInterval.present == ambiguityPresent);
                        if (ambiguityPresent)
                        {
                            assert (result.ambiguityInterval.lower == lowProved + 1);
                            assert (result.ambiguityInterval.upper == highProved - 1);
                        }
                    }
                }
            }
        }
    }

    void requireInverseOrientationIntervalAssignment (bool activeHigh)
    {
        adk::ModuleCharacterizationConfig value = config (2);
        value.descriptor.comparatorPolarity =
            activeHigh ? adk::ModuleComparatorPolarity::ActiveHigh
                       : adk::ModuleComparatorPolarity::ActiveLow;
        adk::ModuleCharacterizationPolicy policy (value);
        begin                                    (policy);
        beginAscending                           (policy);
        assert                                   (policy
                    .observe (adk::TimePoint (1),
                              polarizedPoint (
                                  21, 1,
                                  adk::ModuleCharacterizationLeg::Ascending,
                                  adk::ModuleSweepDirection::Increasing, 100,
                                  1, 0, true, activeHigh))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (2),
                              polarizedPoint (
                                  21, 2,
                                  adk::ModuleCharacterizationLeg::Ascending,
                                  adk::ModuleSweepDirection::Increasing, 101,
                                  2, 1023, false, activeHigh))
                    .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (2)).ok ());
        assert (policy
                    .beginLeg (adk::TimePoint (3), 22,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (3),
                              polarizedPoint (
                                  22, 1,
                                  adk::ModuleCharacterizationLeg::Descending,
                                  adk::ModuleSweepDirection::Decreasing, 102,
                                  3, 1023, false, activeHigh))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (4),
                              polarizedPoint (
                                  22, 2,
                                  adk::ModuleCharacterizationLeg::Descending,
                                  adk::ModuleSweepDirection::Decreasing, 103,
                                  4, 0, true, activeHigh))
                    .ok ());
        assert                                                      (policy.finalizeLeg (adk::TimePoint (4)).ok ());
        const adk::ModuleCharacterizationEvidence result = evidence (policy);
        assert                                                      (result.reason == adk::ModuleCharacterizationReason::None);
        assert                                                      (result.guaranteedActiveInterval.present);
        assert                                                      (result.guaranteedActiveInterval.lower == 0);
        assert                                                      (result.guaranteedActiveInterval.upper == 0);
        assert                                                      (result.guaranteedInactiveInterval.present);
        assert                                                      (result.guaranteedInactiveInterval.lower == 1023);
        assert                                                      (result.guaranteedInactiveInterval.upper == 1023);
        assert                                                      (result.ambiguityInterval.present);
        assert                                                      (result.ambiguityInterval.lower == 1);
        assert                                                      (result.ambiguityInterval.upper == 1022);
    }

    void testInverseOrientationIntervalAssignment ()
    {
        requireInverseOrientationIntervalAssignment (false);
        requireInverseOrientationIntervalAssignment (true);
    }

    void requireTouchingBracketsHaveNoAmbiguity (bool activeHigh)
    {
        adk::ModuleCharacterizationConfig value = config (4);
        value.descriptor.comparatorPolarity =
            activeHigh ? adk::ModuleComparatorPolarity::ActiveHigh
                       : adk::ModuleComparatorPolarity::ActiveLow;
        adk::ModuleCharacterizationPolicy policy (value);
        begin                                    (policy);
        beginAscending                           (policy);
        const uint16_t ascendingRaws[] = {0, 500, 501, 1023};
        for (uint8_t index = 0; index < 4; ++index)
        {
            assert (policy
                        .observe (adk::TimePoint (index + 1),
                                  polarizedPoint (
                                      21,
                                      static_cast<uint16_t> (index + 1),
                                      adk::ModuleCharacterizationLeg::Ascending,
                                      adk::ModuleSweepDirection::Increasing,
                                      static_cast<uint32_t> (100 + index),
                                      index + 1, ascendingRaws[index],
                                      index >= 2, activeHigh))
                        .ok ());
        }
        assert (policy.finalizeLeg (adk::TimePoint (4)).ok ());
        assert (policy
                    .beginLeg (adk::TimePoint (5), 22,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .ok ());
        const uint16_t descendingRaws[] = {1023, 501, 500, 0};
        for (uint8_t index = 0; index < 4; ++index)
        {
            assert (policy
                        .observe (adk::TimePoint (index + 5),
                                  polarizedPoint (
                                      22,
                                      static_cast<uint16_t> (index + 1),
                                      adk::ModuleCharacterizationLeg::Descending,
                                      adk::ModuleSweepDirection::Decreasing,
                                      static_cast<uint32_t> (104 + index),
                                      index + 5, descendingRaws[index],
                                      index < 2, activeHigh))
                        .ok ());
        }
        assert                                                      (policy.finalizeLeg (adk::TimePoint (8)).ok ());
        const adk::ModuleCharacterizationEvidence result = evidence (policy);
        assert                                                      (result.reason == adk::ModuleCharacterizationReason::None);
        assert                                                      (result.guaranteedInactiveInterval.present);
        assert                                                      (result.guaranteedInactiveInterval.lower == 0);
        assert                                                      (result.guaranteedInactiveInterval.upper == 500);
        assert                                                      (result.guaranteedActiveInterval.present);
        assert                                                      (result.guaranteedActiveInterval.lower == 501);
        assert                                                      (result.guaranteedActiveInterval.upper == 1023);
        assert                                                      (!result.ambiguityInterval.present);
    }

    void testTouchingBracketsHaveNoAmbiguity ()
    {
        requireTouchingBracketsHaveNoAmbiguity (false);
        requireTouchingBracketsHaveNoAmbiguity (true);
    }

#if defined(ADK_TESTING)
    void testLifecycleGenerationExhaustion ()
    {
        adk::ModuleCharacterizationPolicy initializePolicy (config ());
        initializePolicy.seedLifecycleGenerationForTest    (UINT32_MAX);
        assert                                             (initializePolicy.initialize (adk::TimePoint (10)).error () ==
                adk::StatusCode::CapacityExceeded);
        initializePolicy.seedLifecycleGenerationForTest (UINT32_MAX - 1);
        assert                                          (initializePolicy.initialize (adk::TimePoint (10)).ok ());
        assert                                          (evidence (initializePolicy).lifecycleGeneration == UINT32_MAX);

        adk::ModuleCharacterizationPolicy resetPolicy               (config ());
        assert                                                      (resetPolicy.initialize (adk::TimePoint (10)).ok ());
        resetPolicy.seedLifecycleGenerationForTest                  (UINT32_MAX);
        const adk::ModuleCharacterizationEvidence before = evidence (resetPolicy);
        assert                                                      (resetPolicy.reset (adk::TimePoint (10)).error () ==
                adk::StatusCode::CapacityExceeded);
        const adk::ModuleCharacterizationEvidence after = evidence (resetPolicy);
        assert                                                     (after.lifecycleGeneration == before.lifecycleGeneration);
        assert                                                     (after.state == before.state);
        assert                                                     (after.reason == before.reason);
        assert                                                     (after.sessionId == before.sessionId);
        assert                                                     (after.runId == before.runId);
    }
#endif

} // namespace

int main ()
{
    testConfigurationAndLifecycle               ();
    testThreeLegCompletion                      ();
    testApiFailuresAreAtomic                    ();
    testIdenticalDuplicateAndChangedDuplicate   ();
    testFailurePrecedence                       ();
    testDirectionAndChatter                     ();
    testCapacityBoundary                        ();
    testRolloverSequenceIsContiguous            ();
    testLearningEndpointPrecedence              ();
    testLifecycleChronology                     ();
    testNoTransitionAndUpperRailOutcomes        ();
    testMaximumPointCapacity                    ();
    testStaleTimestampAndCorrelationFailures    ();
    testActiveHighSymmetry                      ();
    testOrientationMismatch                     ();
    testVerificationRelationOutcomes            ();
    testEqualCodeChatterAndTerminalImmutability ();
    testEvidenceOutputCanaries                  ();
    testCrossLegContinuity                      ();
    testTransitionPositionPolarityMatrix        ();
    testExhaustiveTransitionAndIntervalMatrix   ();
    testInverseOrientationIntervalAssignment    ();
    testTouchingBracketsHaveNoAmbiguity         ();
#if defined(ADK_TESTING)
    testLifecycleGenerationExhaustion ();
#endif
}
