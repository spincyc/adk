#include <assert.h>
#include <type_traits>
#include <stdint.h>

#include "module_threshold_descriptor.h"

namespace {

    bool sameDescriptor (const adk::ModuleThresholdDescriptor& left,
                         const adk::ModuleThresholdDescriptor& right)
    {
        return left.schemaRevision == right.schemaRevision &&
               left.descriptorId == right.descriptorId &&
               left.descriptorRevision == right.descriptorRevision &&
               left.declaredSpecimenReference ==
                   right.declaredSpecimenReference &&
               left.declaredSpecimenRevision ==
                   right.declaredSpecimenRevision &&
               left.declaredElectricalEvidenceRevision ==
                   right.declaredElectricalEvidenceRevision &&
               left.channelTopology == right.channelTopology &&
               left.comparatorOutputStage == right.comparatorOutputStage &&
               left.pullRequirement == right.pullRequirement &&
               left.declaredPullRail == right.declaredPullRail &&
               left.declaredSupplyMillivolts.minimum ==
                   right.declaredSupplyMillivolts.minimum &&
               left.declaredSupplyMillivolts.maximum ==
                   right.declaredSupplyMillivolts.maximum &&
               left.declaredSignalMillivolts.minimum ==
                   right.declaredSignalMillivolts.minimum &&
               left.declaredSignalMillivolts.maximum ==
                   right.declaredSignalMillivolts.maximum &&
               left.rawDomain.minimum == right.rawDomain.minimum &&
               left.rawDomain.maximum == right.rawDomain.maximum &&
               left.comparatorPolarity == right.comparatorPolarity &&
               left.thresholdControlKind == right.thresholdControlKind &&
               left.thresholdDirection == right.thresholdDirection &&
               left.warmup.declaration == right.warmup.declaration &&
               left.warmup.value == right.warmup.value &&
               left.settling.declaration == right.settling.declaration &&
               left.settling.value == right.settling.value;
    }

    bool sameFrame (const adk::ModuleThresholdFrame& left,
                    const adk::ModuleThresholdFrame& right)
    {
        return left.schemaRevision == right.schemaRevision &&
               left.descriptorId == right.descriptorId &&
               left.descriptorRevision == right.descriptorRevision &&
               left.declaredSpecimenReference ==
                   right.declaredSpecimenReference &&
               left.declaredSpecimenRevision ==
                   right.declaredSpecimenRevision &&
               left.declaredElectricalEvidenceRevision ==
                   right.declaredElectricalEvidenceRevision &&
               left.provenance.sourceId == right.provenance.sourceId &&
               left.provenance.sourceConfigurationRevision ==
                   right.provenance.sourceConfigurationRevision &&
               left.provenance.sequence == right.provenance.sequence &&
               left.provenance.observedAt == right.provenance.observedAt &&
               left.analogRaw == right.analogRaw &&
               left.analogStatus == right.analogStatus &&
               left.comparatorLevelHigh == right.comparatorLevelHigh &&
               left.comparatorStatus == right.comparatorStatus &&
               left.comparatorPresent == right.comparatorPresent &&
               left.comparatorAsserted == right.comparatorAsserted &&
               left.declaredWarmupSatisfied ==
                   right.declaredWarmupSatisfied &&
               left.declaredSettlingSatisfied ==
                   right.declaredSettlingSatisfied &&
               left.analogProducerStatus == right.analogProducerStatus &&
               left.comparatorProducerStatus ==
                   right.comparatorProducerStatus;
    }

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
                {adk::ModuleDurationDeclaration::Known, adk::Duration (100)},
                {adk::ModuleDurationDeclaration::Known, adk::Duration (20)}};
    }

    adk::ModuleThresholdFrame frame (
        const adk::ModuleThresholdDescriptor& value)
    {
        return {value.schemaRevision,
                value.descriptorId,
                value.descriptorRevision,
                value.declaredSpecimenReference,
                value.declaredSpecimenRevision,
                value.declaredElectricalEvidenceRevision,
                {7, 8, 0, adk::TimePoint (0xffffffffUL)},
                512,
                adk::ModuleChannelStatus::Current,
                false,
                adk::ModuleChannelStatus::Current,
                true,
                true,
                true,
                true,
                adk::StatusCode::Ok,
                adk::StatusCode::Ok};
    }

    void testEveryDescriptorEnumEncoding ()
    {
        for (uint16_t value = 0; value <= UINT8_MAX; ++value)
        {
            adk::ModuleThresholdDescriptor candidate = descriptor ();
            candidate.channelTopology =
                static_cast<adk::ModuleChannelTopology> (value);
            if (value == 0)
            {
                candidate.comparatorOutputStage =
                    adk::ModuleComparatorOutputStage::Unspecified;
                candidate.pullRequirement =
                    adk::ModulePullRequirement::Unspecified;
                candidate.declaredPullRail =
                    adk::ModuleDeclaredRail::Unspecified;
                candidate.comparatorPolarity =
                    adk::ModuleComparatorPolarity::Unspecified;
                candidate.thresholdControlKind =
                    adk::ModuleThresholdControlKind::Unspecified;
                candidate.thresholdDirection =
                    adk::ModuleThresholdDirection::Unspecified;
            }
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 2));

            candidate = descriptor ();
            candidate.comparatorOutputStage =
                static_cast<adk::ModuleComparatorOutputStage> (value);
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 3));

            candidate = descriptor ();
            candidate.pullRequirement =
                static_cast<adk::ModulePullRequirement> (value);
            if (value <= 1)
            {
                candidate.declaredPullRail =
                    adk::ModuleDeclaredRail::Unspecified;
            }
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 3));

            candidate = descriptor ();
            candidate.declaredPullRail =
                static_cast<adk::ModuleDeclaredRail> (value);
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value >= 1 && value <= 3));

            candidate = descriptor ();
            candidate.comparatorPolarity =
                static_cast<adk::ModuleComparatorPolarity> (value);
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 2));

            candidate = descriptor ();
            candidate.thresholdControlKind =
                static_cast<adk::ModuleThresholdControlKind> (value);
            if (value != 2)
            {
                candidate.thresholdDirection =
                    adk::ModuleThresholdDirection::Unspecified;
            }
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 2));

            candidate = descriptor ();
            candidate.thresholdDirection =
                static_cast<adk::ModuleThresholdDirection> (value);
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 2));

            candidate = descriptor ();
            candidate.warmup.declaration =
                static_cast<adk::ModuleDurationDeclaration> (value);
            if (value == 1)
            {
                candidate.warmup.value = adk::Duration (0);
            }
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 1));

            candidate = descriptor ();
            candidate.settling.declaration =
                static_cast<adk::ModuleDurationDeclaration> (value);
            if (value == 1)
            {
                candidate.settling.value = adk::Duration (0);
            }
            assert (adk::validateModuleThresholdDescriptor (candidate).ok () ==
                    (value <= 1));
        }
    }

    void testCompleteDescriptorCrossProduct ()
    {
        for (uint8_t topology = 0; topology < 3; ++topology)
        {
            for (uint8_t stage = 0; stage < 4; ++stage)
            {
                for (uint8_t pull = 0; pull < 4; ++pull)
                {
                    for (uint8_t rail = 0; rail < 4; ++rail)
                    {
                        for (uint8_t polarity = 0; polarity < 3; ++polarity)
                        {
                            for (uint8_t control = 0; control < 3; ++control)
                            {
                                for (uint8_t direction = 0; direction < 3;
                                     ++direction)
                                {
                                    adk::ModuleThresholdDescriptor candidate =
                                        descriptor ();
                                    candidate.channelTopology =
                                        static_cast<adk::ModuleChannelTopology> (
                                            topology);
                                    candidate.comparatorOutputStage =
                                        static_cast<
                                            adk::ModuleComparatorOutputStage> (
                                            stage);
                                    candidate.pullRequirement =
                                        static_cast<
                                            adk::ModulePullRequirement> (pull);
                                    candidate.declaredPullRail =
                                        static_cast<adk::ModuleDeclaredRail> (
                                            rail);
                                    candidate.comparatorPolarity =
                                        static_cast<
                                            adk::ModuleComparatorPolarity> (
                                            polarity);
                                    candidate.thresholdControlKind =
                                        static_cast<
                                            adk::ModuleThresholdControlKind> (
                                            control);
                                    candidate.thresholdDirection =
                                        static_cast<
                                            adk::ModuleThresholdDirection> (
                                            direction);

                                    const bool comparator = topology != 0;
                                    const bool declaresPull = pull >= 2;
                                    const bool pullCanonical =
                                        declaresPull ? rail != 0 : rail == 0;
                                    const bool comparatorCanonical =
                                        comparator
                                            ? pullCanonical &&
                                                  (control == 2 ||
                                                   direction == 0)
                                            : stage == 0 && pull == 0 &&
                                                  rail == 0 && polarity == 0 &&
                                                  control == 0 &&
                                                  direction == 0;
                                    assert (
                                        adk::validateModuleThresholdDescriptor (
                                            candidate)
                                            .ok () == comparatorCanonical);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void testIdentityRangeAndDurationBoundaries ()
    {
        for (uint8_t field = 0; field < 6; ++field)
        {
            adk::ModuleThresholdDescriptor candidate = descriptor ();
            switch (field)
            {
                case 0: candidate.schemaRevision = 0; break;
                case 1: candidate.descriptorId = 0; break;
                case 2: candidate.descriptorRevision = 0; break;
                case 3: candidate.declaredSpecimenReference = 0; break;
                case 4: candidate.declaredSpecimenRevision = 0; break;
                case 5:
                    candidate.declaredElectricalEvidenceRevision = 0;
                    break;
            }
            assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                    adk::StatusCode::InvalidConfiguration);
        }

        adk::ModuleThresholdDescriptor candidate = descriptor ();
        candidate.declaredSupplyMillivolts = {5000, 5000};
        candidate.declaredSignalMillivolts = {0, 0};
        candidate.rawDomain = {UINT16_MAX, UINT16_MAX};
        assert (adk::validateModuleThresholdDescriptor (candidate).ok ());

        candidate = descriptor ();
        candidate.declaredSupplyMillivolts = {2, 1};
        assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                adk::StatusCode::InvalidConfiguration);
        candidate = descriptor ();
        candidate.declaredSignalMillivolts = {2, 1};
        assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                adk::StatusCode::InvalidConfiguration);
        candidate = descriptor ();
        candidate.rawDomain = {2, 1};
        assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                adk::StatusCode::InvalidConfiguration);

        const uint32_t durationValues[] = {0, 1, 0x7fffffffUL, 0xffffffffUL};
        for (uint8_t index = 0; index < 4; ++index)
        {
            candidate = descriptor ();

            candidate.warmup.value = adk::Duration (durationValues[index]);

            candidate.settling.value = adk::Duration (durationValues[index]);

            assert (adk::validateModuleThresholdDescriptor (candidate).ok ());
        }

        candidate = descriptor ();
        candidate.warmup = {adk::ModuleDurationDeclaration::Unknown,
                            adk::Duration (0)};
        candidate.settling = candidate.warmup;

        assert (adk::validateModuleThresholdDescriptor (candidate).ok ());

        const adk::Result<bool> complete =
            adk::moduleDescriptorDeclarationsComplete (candidate);
        assert (complete.ok ());
        assert (!complete.value ());

        candidate.warmup.value = adk::Duration (1);

        assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testIdentityExtremaAndIndependentDurations ()
    {
        adk::ModuleThresholdDescriptor candidate = descriptor ();
        candidate.schemaRevision = 1;
        candidate.descriptorId = 1;
        candidate.descriptorRevision = 1;
        candidate.declaredSpecimenReference = 1;
        candidate.declaredSpecimenRevision = 1;
        candidate.declaredElectricalEvidenceRevision = 1;
        assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                adk::StatusCode::Ok);

        candidate.schemaRevision = UINT16_MAX;
        candidate.descriptorId = UINT32_MAX;
        candidate.descriptorRevision = UINT16_MAX;
        candidate.declaredSpecimenReference = UINT32_MAX;
        candidate.declaredSpecimenRevision = UINT16_MAX;
        candidate.declaredElectricalEvidenceRevision = UINT16_MAX;
        assert (adk::validateModuleThresholdDescriptor (candidate).error () ==
                adk::StatusCode::Ok);

        for (uint8_t warmup = 0; warmup < 2; ++warmup)
        {
            for (uint8_t settling = 0; settling < 2; ++settling)
            {
                candidate = descriptor ();
                candidate.warmup = {
                    static_cast<adk::ModuleDurationDeclaration> (warmup),
                    adk::Duration (warmup == 0 ? 1 : 0)};
                candidate.settling = {
                    static_cast<adk::ModuleDurationDeclaration> (settling),
                    adk::Duration (settling == 0 ? UINT32_MAX : 0)};
                assert (
                    adk::validateModuleThresholdDescriptor (candidate).error () ==
                    adk::StatusCode::Ok);
                const adk::Result<bool> complete =
                    adk::moduleDescriptorDeclarationsComplete (candidate);
                assert (complete.error () == adk::StatusCode::Ok);
                assert (complete.value () ==
                        (warmup == 0 && settling == 0));

                adk::ModuleThresholdFrame observation = frame (candidate);
                observation.declaredWarmupSatisfied = warmup == 0;
                observation.declaredSettlingSatisfied = settling == 0;
                assert (adk::validateModuleThresholdFrame (
                            candidate, observation)
                            .error () == adk::StatusCode::Ok);

                if (warmup != 0)
                {
                    observation.declaredWarmupSatisfied = true;
                    assert (adk::validateModuleThresholdFrame (
                                candidate, observation)
                                .error () == adk::StatusCode::InvalidArgument);
                }
                if (settling != 0)
                {
                    observation = frame (candidate);
                    observation.declaredWarmupSatisfied = false;
                    observation.declaredSettlingSatisfied = true;
                    assert (adk::validateModuleThresholdFrame (
                                candidate, observation)
                                .error () == adk::StatusCode::InvalidArgument);
                }
            }
        }
    }

    void testComparatorAssertionAndCompleteness ()
    {
        adk::ModuleThresholdDescriptor candidate = descriptor ();
        for (uint8_t polarity = 1; polarity <= 2; ++polarity)
        {
            candidate.comparatorPolarity =
                static_cast<adk::ModuleComparatorPolarity> (polarity);
            for (uint8_t level = 0; level <= 1; ++level)
            {
                const adk::Result<bool> result =
                    adk::moduleComparatorAsserted (candidate, level != 0);
                assert (result.ok ());
                assert (result.value () ==
                        ((polarity == 1) == (level != 0)));
            }
        }

        candidate.comparatorPolarity =
            adk::ModuleComparatorPolarity::Unspecified;
        assert (adk::moduleComparatorAsserted (candidate, false).error () ==
                adk::StatusCode::InvalidConfiguration);

        candidate = descriptor ();

        const adk::Result<bool> complete =
            adk::moduleDescriptorDeclarationsComplete (candidate);
        assert (complete.ok ());
        assert (complete.value ());
        for (uint8_t field = 0; field < 6; ++field)
        {
            adk::ModuleThresholdDescriptor incomplete = descriptor ();
            switch (field)
            {
                case 0:
                    incomplete.comparatorOutputStage =
                        adk::ModuleComparatorOutputStage::Unspecified;
                    break;
                case 1:
                    incomplete.pullRequirement =
                        adk::ModulePullRequirement::Unspecified;
                    incomplete.declaredPullRail =
                        adk::ModuleDeclaredRail::Unspecified;
                    break;
                case 2:
                    incomplete.comparatorPolarity =
                        adk::ModuleComparatorPolarity::Unspecified;
                    break;
                case 3:
                    incomplete.thresholdControlKind =
                        adk::ModuleThresholdControlKind::Unspecified;
                    incomplete.thresholdDirection =
                        adk::ModuleThresholdDirection::Unspecified;
                    break;
                case 4:
                    incomplete.thresholdDirection =
                        adk::ModuleThresholdDirection::Unspecified;
                    break;
                case 5:
                    incomplete.warmup = {
                        adk::ModuleDurationDeclaration::Unknown,
                        adk::Duration (0)};
                    break;
            }
            const adk::Result<bool> result =
                adk::moduleDescriptorDeclarationsComplete (incomplete);
            assert (result.ok ());
            assert (!result.value ());
        }
    }

    void makeCanonicalForTopology (
        adk::ModuleThresholdDescriptor& candidate,
        adk::ModuleThresholdFrame&      observation,
        adk::ModuleChannelTopology      topology)
    {
        candidate.channelTopology = topology;
        if (topology == adk::ModuleChannelTopology::AnalogOnly)
        {
            candidate.comparatorOutputStage =
                adk::ModuleComparatorOutputStage::Unspecified;
            candidate.pullRequirement =
                adk::ModulePullRequirement::Unspecified;
            candidate.declaredPullRail = adk::ModuleDeclaredRail::Unspecified;
            candidate.comparatorPolarity =
                adk::ModuleComparatorPolarity::Unspecified;
            candidate.thresholdControlKind =
                adk::ModuleThresholdControlKind::Unspecified;
            candidate.thresholdDirection =
                adk::ModuleThresholdDirection::Unspecified;
            observation.comparatorLevelHigh = false;
            observation.comparatorStatus =
                adk::ModuleChannelStatus::NotPresent;
            observation.comparatorPresent = false;
            observation.comparatorAsserted = false;
        }
        else if (topology == adk::ModuleChannelTopology::ComparatorOnly)
        {
            observation.analogRaw = 0;
            observation.analogStatus = adk::ModuleChannelStatus::NotPresent;
        }
    }

    void testCanonicalFramesAndChannelStatuses ()
    {
        for (uint8_t topology = 0; topology < 3; ++topology)
        {
            adk::ModuleThresholdDescriptor candidate = descriptor ();

            adk::ModuleThresholdFrame      observation = frame (candidate);

            makeCanonicalForTopology (
                candidate, observation,
                static_cast<adk::ModuleChannelTopology> (topology));
            assert (adk::validateModuleThresholdFrame (
                        candidate, observation)
                        .ok ());
        }

        for (uint8_t channel = 0; channel < 2; ++channel)
        {
            for (uint8_t status = 0; status < 4; ++status)
            {
                for (uint8_t producer = 0;
                     producer <= static_cast<uint8_t> (
                                     adk::StatusCode::HardwareFailure);
                     ++producer)
                {
                    adk::ModuleThresholdDescriptor candidate = descriptor ();

                    adk::ModuleThresholdFrame observation = frame (candidate);
                    const adk::ModuleChannelStatus channelStatus =
                        static_cast<adk::ModuleChannelStatus> (status);
                    const adk::Status producerStatus (
                        static_cast<adk::StatusCode> (producer));
                    if (channel == 0)
                    {
                        observation.analogStatus = channelStatus;
                        observation.analogProducerStatus = producerStatus;
                    }
                    else
                    {
                        observation.comparatorStatus = channelStatus;
                        observation.comparatorProducerStatus = producerStatus;
                    }
                    const bool valid =
                        status != 0 &&
                        ((status == 3) == (producer != 0));
                    assert (
                        adk::validateModuleThresholdFrame (
                            candidate, observation)
                            .ok () == valid);
                }
            }
        }

        for (uint16_t value = 4; value <= UINT8_MAX; ++value)
        {
            adk::ModuleThresholdDescriptor candidate = descriptor ();

            adk::ModuleThresholdFrame observation = frame (candidate);
            observation.analogStatus =
                static_cast<adk::ModuleChannelStatus> (value);
            assert (adk::validateModuleThresholdFrame (
                        candidate, observation)
                        .error () == adk::StatusCode::InvalidArgument);

            observation = frame (candidate);
            observation.comparatorStatus =
                static_cast<adk::ModuleChannelStatus> (value);
            assert (adk::validateModuleThresholdFrame (
                        candidate, observation)
                        .error () == adk::StatusCode::InvalidArgument);
        }

        for (uint16_t value = 11; value <= UINT8_MAX; ++value)
        {
            adk::ModuleThresholdDescriptor candidate = descriptor ();

            adk::ModuleThresholdFrame observation = frame (candidate);
            observation.analogProducerStatus =
                adk::Status (static_cast<adk::StatusCode> (value));
            assert (adk::validateModuleThresholdFrame (
                        candidate, observation)
                        .error () == adk::StatusCode::InvalidArgument);

            observation = frame (candidate);
            observation.comparatorProducerStatus =
                adk::Status (static_cast<adk::StatusCode> (value));
            assert (adk::validateModuleThresholdFrame (
                        candidate, observation)
                        .error () == adk::StatusCode::InvalidArgument);
        }
    }

    void testAbsentChannelsRequireCanonicalFields ()
    {
        adk::ModuleThresholdDescriptor analogOnly = descriptor ();

        adk::ModuleThresholdFrame analogFrame = frame (analogOnly);

        makeCanonicalForTopology (
            analogOnly, analogFrame,
            adk::ModuleChannelTopology::AnalogOnly);
        for (uint8_t field = 0; field < 5; ++field)
        {
            adk::ModuleThresholdFrame candidate = analogFrame;
            switch (field)
            {
                case 0: candidate.comparatorPresent = true; break;
                case 1:
                    candidate.comparatorStatus =
                        adk::ModuleChannelStatus::Current;
                    break;
                case 2: candidate.comparatorLevelHigh = true; break;
                case 3: candidate.comparatorAsserted = true; break;
                case 4:
                    candidate.comparatorProducerStatus =
                        adk::StatusCode::HardwareFailure;
                    break;
            }
            assert (adk::validateModuleThresholdFrame (
                        analogOnly, candidate)
                        .error () == adk::StatusCode::InvalidArgument);
        }

        adk::ModuleThresholdDescriptor comparatorOnly = descriptor ();

        adk::ModuleThresholdFrame comparatorFrame = frame (comparatorOnly);

        makeCanonicalForTopology (
            comparatorOnly, comparatorFrame,
            adk::ModuleChannelTopology::ComparatorOnly);
        for (uint8_t field = 0; field < 3; ++field)
        {
            adk::ModuleThresholdFrame candidate = comparatorFrame;
            switch (field)
            {
                case 0: candidate.analogRaw = 1; break;
                case 1:
                    candidate.analogStatus =
                        adk::ModuleChannelStatus::Current;
                    break;
                case 2:
                    candidate.analogProducerStatus =
                        adk::StatusCode::HardwareFailure;
                    break;
            }
            assert (adk::validateModuleThresholdFrame (
                        comparatorOnly, candidate)
                        .error () == adk::StatusCode::InvalidArgument);
        }
    }

    void testFrameCorrelationCanonicalityAndNonmutation ()
    {
        const adk::ModuleThresholdDescriptor originalDescriptor =
            descriptor ();
        const adk::ModuleThresholdFrame originalFrame =
            frame (originalDescriptor);

        for (uint8_t field = 0; field < 8; ++field)
        {
            adk::ModuleThresholdFrame candidate = originalFrame;
            switch (field)
            {
                case 0: ++candidate.schemaRevision; break;
                case 1: ++candidate.descriptorId; break;
                case 2: ++candidate.descriptorRevision; break;
                case 3: ++candidate.declaredSpecimenReference; break;
                case 4: ++candidate.declaredSpecimenRevision; break;
                case 5:
                    ++candidate.declaredElectricalEvidenceRevision;
                    break;
                case 6: candidate.provenance.sourceId = 0; break;
                case 7:
                    candidate.provenance.sourceConfigurationRevision = 0;
                    break;
            }
            assert (adk::validateModuleThresholdFrame (
                        originalDescriptor, candidate)
                        .error () == adk::StatusCode::InvalidArgument);
        }

        adk::ModuleThresholdDescriptor unknownDuration =
            originalDescriptor;
        unknownDuration.warmup = {
            adk::ModuleDurationDeclaration::Unknown, adk::Duration (0)};
        unknownDuration.settling = unknownDuration.warmup;
        adk::ModuleThresholdFrame candidate = frame (unknownDuration);
        candidate.declaredWarmupSatisfied = false;
        candidate.declaredSettlingSatisfied = false;
        assert (adk::validateModuleThresholdFrame (
                    unknownDuration, candidate)
                    .ok ());
        candidate.declaredWarmupSatisfied = true;
        assert (adk::validateModuleThresholdFrame (
                    unknownDuration, candidate)
                    .error () == adk::StatusCode::InvalidArgument);

        assert (originalDescriptor.descriptorId == 0x12345678UL);
        assert (originalDescriptor.declaredSpecimenReference ==
                0x87654321UL);
        assert (originalFrame.provenance.sequence == 0);
        assert (originalFrame.provenance.observedAt ==
                adk::TimePoint (0xffffffffUL));
    }

    void testFrameRailsCollisionsReplayAndNonmutation ()
    {
        const adk::ModuleThresholdDescriptor original = descriptor ();
        for (uint8_t boundary = 0; boundary < 2; ++boundary)
        {
            adk::ModuleThresholdFrame observation = frame (original);
            observation.analogRaw =
                boundary == 0 ? original.rawDomain.minimum
                              : original.rawDomain.maximum;
            const adk::ModuleThresholdFrame before = observation;
            assert (adk::validateModuleThresholdFrame (
                        original, observation)
                        .error () == adk::StatusCode::Ok);
            assert (sameFrame (observation, before));
        }

        adk::ModuleThresholdDescriptor narrowed = original;
        narrowed.rawDomain = {10, 20};
        for (uint8_t boundary = 0; boundary < 2; ++boundary)
        {
            adk::ModuleThresholdFrame observation = frame (narrowed);
            observation.analogRaw = boundary == 0 ? 9 : 21;
            const adk::ModuleThresholdFrame before = observation;
            assert (adk::validateModuleThresholdFrame (
                        narrowed, observation)
                        .error () == adk::StatusCode::InvalidArgument);
            assert (sameFrame (observation, before));
        }

        adk::ModuleThresholdFrame collision = frame (original);
        collision.descriptorRevision = 0;
        collision.analogStatus = adk::ModuleChannelStatus::ProducerFault;
        collision.analogProducerStatus = adk::StatusCode::HardwareFailure;
        assert (adk::validateModuleThresholdFrame (
                    original, collision)
                    .error () == adk::StatusCode::InvalidArgument);

        collision = frame (original);
        collision.analogStatus = adk::ModuleChannelStatus::ProducerFault;
        collision.analogProducerStatus = adk::StatusCode::HardwareFailure;
        collision.comparatorStatus =
            adk::ModuleChannelStatus::ProducerFault;
        collision.comparatorProducerStatus =
            adk::StatusCode::ResourceBusy;
        assert (adk::validateModuleThresholdFrame (
                    original, collision)
                    .error () == adk::StatusCode::Ok);

        for (uint8_t sourceId = 1; sourceId != 0; ++sourceId)
        {
            adk::ModuleThresholdFrame observation = frame (original);
            observation.provenance.sourceId = sourceId;
            observation.provenance.sourceConfigurationRevision =
                sourceId == 1 ? 1 : UINT16_MAX;
            observation.provenance.sequence =
                sourceId == 1 ? 0 : UINT32_MAX;
            assert (adk::validateModuleThresholdFrame (
                        original, observation)
                        .error () == adk::StatusCode::Ok);
            if (sourceId == UINT8_MAX)
            {
                break;
            }
        }

        adk::ModuleThresholdDescriptor alias = original;
        ++alias.declaredSpecimenReference;
        ++alias.declaredElectricalEvidenceRevision;
        adk::ModuleThresholdFrame aliasFrame = frame (alias);

        assert (adk::validateModuleThresholdFrame (
                    alias, aliasFrame)
                    .error () == adk::StatusCode::Ok);
        assert (adk::validateModuleThresholdFrame (
                    original, aliasFrame)
                    .error () == adk::StatusCode::InvalidArgument);

        const adk::ModuleThresholdDescriptor descriptorBefore = original;
        const adk::ModuleThresholdFrame frameBefore = frame (original);
        adk::ModuleThresholdFrame replay = frameBefore;
        for (uint8_t repeat = 0; repeat < 3; ++repeat)
        {
            assert (adk::validateModuleThresholdDescriptor (original).error () ==
                    adk::StatusCode::Ok);
            assert (adk::validateModuleThresholdFrame (
                        original, replay)
                        .error () == adk::StatusCode::Ok);
            assert (sameDescriptor (original, descriptorBefore));
            assert (sameFrame (replay, frameBefore));
        }
    }

    void testStructuralResourceFreedom ()
    {
        static_assert (
            std::is_standard_layout<adk::ModuleThresholdDescriptor>::value,
            "descriptor remains a caller-owned value");
        static_assert (
            std::is_standard_layout<adk::ModuleThresholdFrame>::value,
            "frame remains a caller-owned value");
        static_assert (sizeof (adk::ModuleThresholdDescriptor) <= 96,
                       "descriptor hard size limit");
    }
} // namespace

int main ()
{
    testEveryDescriptorEnumEncoding                    ();
    testCompleteDescriptorCrossProduct                 ();
    testIdentityRangeAndDurationBoundaries             ();
    testIdentityExtremaAndIndependentDurations         ();
    testComparatorAssertionAndCompleteness             ();
    testCanonicalFramesAndChannelStatuses              ();
    testAbsentChannelsRequireCanonicalFields           ();
    testFrameCorrelationCanonicalityAndNonmutation     ();
    testFrameRailsCollisionsReplayAndNonmutation       ();
    testStructuralResourceFreedom                      ();
}
