// E0 copied-characterization replay. This sketch streams supplied frames and
// supplied time through the policy. It owns no module, pin, ADC, comparator,
// clock, display, storage, stimulus, or powered circuit.
#include <Adk.h>
#include <module_characterization.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        Ascending,
        Descending,
        Verification,
        Evidence,
        Shutdown,
        Complete
    };

    struct LegResultCell
    {
        uint32_t legId;
        uint16_t sweepStartRaw;
        uint16_t sweepEndRaw;
        uint16_t beforeRaw;
        uint16_t afterRaw;
        uint8_t  acceptedPoints;
        uint8_t  bracketPresent;
        uint8_t  status;
        uint8_t  predictionPass;
    };

    struct CharacterizationResultCell
    {
        uint16_t inactiveLower;
        uint16_t inactiveUpper;
        uint16_t activeLower;
        uint16_t activeUpper;
        uint16_t ambiguityLower;
        uint16_t ambiguityUpper;
        uint8_t  state;
        uint8_t  reason;
        uint8_t  relation;
        uint8_t  ascendingCount;
        uint8_t  descendingCount;
        uint8_t  verificationCount;
        uint8_t  allIntervalsPresent;
        uint8_t  predictionPass;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t initializeStatus;
        uint8_t sessionStatus;
        uint8_t completedLegs;
        uint8_t predictionsPass;
        uint8_t complete;
    };

    constexpr uint32_t sessionId                    = 71001UL;
    constexpr uint32_t runId                        = 71002UL;
    constexpr uint8_t  sourceId                     = 71;
    constexpr uint16_t sourceConfigurationRevision  = 1;
    constexpr uint8_t  requiredPointsPerLeg         = 4;

    const adk::ModuleThresholdDescriptor descriptor = {
        1,
        71001UL,
        1,
        710001UL,
        1,
        1,
        adk::ModuleChannelTopology::AnalogAndComparator,
        adk::ModuleComparatorOutputStage::PushPull,
        adk::ModulePullRequirement::None,
        adk::ModuleDeclaredRail::Unspecified,
        {3000, 5000},
        {0, 5000},
        {0, 1023},
        adk::ModuleComparatorPolarity::ActiveHigh,
        adk::ModuleThresholdControlKind::Potentiometer,
        adk::ModuleThresholdDirection::IncreasingClockwise,
        {adk::ModuleDurationDeclaration::Known, adk::Duration (100)},
        {adk::ModuleDurationDeclaration::Known, adk::Duration (10)}};

    const adk::ModuleCharacterizationConfig characterizationConfig = {
        1,
        descriptor,
        requiredPointsPerLeg,
        adk::Duration (5),
        adk::Duration (5)};

    const uint16_t ascendingRaw[4]       = {0, 500, 700, 1023};
    const bool     ascendingAsserted[4]  = {false, false, true, true};
    const uint16_t descendingRaw[4]      = {1023, 650, 450, 0};
    const bool     descendingAsserted[4] = {true, true, false, false};
    const uint16_t verificationRaw[4]    = {300, 550, 800, 900};
    const bool     verificationAsserted[4] = {false, false, true, true};

    adk::ModuleCharacterizationPolicy characterization (
        characterizationConfig);
    adk::ModuleCharacterizationPoint    replayPoint;
    adk::ModuleCharacterizationEvidence replayEvidence;

    volatile LegResultCell              legResultCells[3];
    volatile CharacterizationResultCell characterizationResultCell;
    volatile ReplayResultCell           replayResultCell;

    ReplayStage replayStage;
    uint32_t    suppliedNow;
    uint32_t    nextSequence;

    // clang-format off
    adk::Status acquireCopiedDescriptor  ();
    void        configureCopiedReplay    ();
    adk::Status startCopiedReplay        ();

    void observeCopiedPoint (
        uint32_t legId, adk::ModuleCharacterizationLeg leg,
        adk::ModuleSweepDirection direction, uint16_t ordinal,
        uint16_t analogRaw, bool comparatorAsserted);
    adk::Status replayLeg (
        uint8_t resultIndex, uint32_t legId,
        adk::ModuleCharacterizationLeg leg,
        adk::ModuleSweepDirection direction, const uint16_t analogRaw[4],
        const bool comparatorAsserted[4]);

    bool decideLegResult (
        uint8_t resultIndex, uint32_t legId,
        const adk::ModuleCharacterizationEvidence& evidence,
        adk::Status status);
    void actuateLegResult (
        uint8_t resultIndex, uint32_t legId,
        const adk::ModuleCharacterizationEvidence& evidence,
        adk::Status status, bool prediction);
    bool decideFinalResult (
        const adk::ModuleCharacterizationEvidence& evidence);
    void actuateFinalResult (
        const adk::ModuleCharacterizationEvidence& evidence,
        bool prediction);
    void finishReplay (adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedDescriptor ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        finishReplay (fixtureStatus);
        return;
    }

    configureCopiedReplay ();

    const adk::Status initializeStatus = startCopiedReplay ();
    replayResultCell.initializeStatus =
        static_cast<uint8_t> (initializeStatus.error ());
    if (!initializeStatus.ok ())
    {
        finishReplay (initializeStatus);
    }
}

void loop ()
{
    if (replayStage == ReplayStage::Complete)
    {
        return;
    }

    adk::Status status = adk::StatusCode::Ok;
    if (replayStage == ReplayStage::Ascending)
    {
        status = replayLeg (0, 1, adk::ModuleCharacterizationLeg::Ascending,
                            adk::ModuleSweepDirection::Increasing,
                            ascendingRaw, ascendingAsserted);
    }
    else if (replayStage == ReplayStage::Descending)
    {
        status = replayLeg (1, 2, adk::ModuleCharacterizationLeg::Descending,
                            adk::ModuleSweepDirection::Decreasing,
                            descendingRaw, descendingAsserted);
    }
    else if (replayStage == ReplayStage::Verification)
    {
        status = replayLeg (2, 3, adk::ModuleCharacterizationLeg::Verification,
                            adk::ModuleSweepDirection::Unordered,
                            verificationRaw, verificationAsserted);
    }
    else if (replayStage == ReplayStage::Evidence)
    {
        status = characterization.evidence (replayEvidence);

        const bool prediction =
            status.ok () && decideFinalResult (replayEvidence);

        actuateFinalResult (replayEvidence, prediction);
    }
    else
    {
        status = characterization.shutdown (adk::TimePoint (++suppliedNow));

        finishReplay (status);
        return;
    }

    if (!status.ok ())
    {
        finishReplay (status);
        return;
    }
    replayStage =
        static_cast<ReplayStage> (static_cast<uint8_t> (replayStage) + 1U);
}

namespace {

    adk::Status acquireCopiedDescriptor ()
    {
        const adk::Status descriptorStatus =
            adk::validateModuleThresholdDescriptor (descriptor);
        if (!descriptorStatus.ok ())
        {
            return descriptorStatus;
        }

        const adk::Result<bool> declarations =
            adk::moduleDescriptorDeclarationsComplete (descriptor);
        if (!declarations.ok ())
        {
            return declarations.status ();
        }
        return declarations.value () ? adk::StatusCode::Ok :
                                       adk::StatusCode::InvalidConfiguration;
    }

    void configureCopiedReplay ()
    {
        replayStage                         = ReplayStage::Ascending;
        suppliedNow                         = 100;
        nextSequence                        = 1;
        replayResultCell.initializeStatus   = 0;
        replayResultCell.sessionStatus      = 0;
        replayResultCell.completedLegs      = 0;
        replayResultCell.predictionsPass    = 1;
        replayResultCell.complete           = 0;
        characterizationResultCell.predictionPass = 0;
    }

    adk::Status startCopiedReplay ()
    {
        adk::Status status =
            characterization.initialize (adk::TimePoint (suppliedNow));
        if (!status.ok ())
        {
            return status;
        }

        status = characterization.beginSession (
            adk::TimePoint (++suppliedNow), sessionId, runId);
        replayResultCell.sessionStatus =
            static_cast<uint8_t> (status.error ());
        return status;
    }

    void observeCopiedPoint (
        uint32_t legId, adk::ModuleCharacterizationLeg leg,
        adk::ModuleSweepDirection direction, uint16_t ordinal,
        uint16_t analogRaw, bool comparatorAsserted)
    {
        ++suppliedNow;
        const adk::ModuleThresholdFrame frame = {
            descriptor.schemaRevision,
            descriptor.descriptorId,
            descriptor.descriptorRevision,
            descriptor.declaredSpecimenReference,
            descriptor.declaredSpecimenRevision,
            descriptor.declaredElectricalEvidenceRevision,
            {sourceId, sourceConfigurationRevision, nextSequence++,
             adk::TimePoint (suppliedNow)},
            analogRaw,
            adk::ModuleChannelStatus::Current,
            comparatorAsserted,
            adk::ModuleChannelStatus::Current,
            true,
            comparatorAsserted,
            true,
            true,
            adk::StatusCode::Ok,
            adk::StatusCode::Ok};

        replayPoint = {sessionId,
                       runId,
                       legId,
                       ordinal,
                       leg,
                       direction,
                       sourceId,
                       sourceConfigurationRevision,
                       frame};
    }

    adk::Status replayLeg (
        uint8_t resultIndex, uint32_t legId,
        adk::ModuleCharacterizationLeg leg,
        adk::ModuleSweepDirection direction, const uint16_t analogRaw[4],
        const bool comparatorAsserted[4])
    {
        adk::Status status = characterization.beginLeg (
            adk::TimePoint (++suppliedNow), legId, leg, direction);
        for (uint16_t index = 0; status.ok () && index < requiredPointsPerLeg;
             ++index)
        {
            observeCopiedPoint (
                legId, leg, direction, static_cast<uint16_t> (index + 1U),
                analogRaw[index], comparatorAsserted[index]);
            status = characterization.observe (adk::TimePoint (suppliedNow),
                                               replayPoint);
        }
        if (status.ok ())
        {
            status =
                characterization.finalizeLeg (adk::TimePoint (++suppliedNow));
        }

        const adk::Status evidenceStatus =
            characterization.evidence (replayEvidence);

        if (status.ok () && !evidenceStatus.ok ())
        {
            status = evidenceStatus;
        }
        const bool prediction =
            decideLegResult (resultIndex, legId, replayEvidence, status);
        actuateLegResult (
            resultIndex, legId, replayEvidence, status, prediction);
        legResultCells[resultIndex].sweepStartRaw = analogRaw[0];
        legResultCells[resultIndex].sweepEndRaw =
            analogRaw[requiredPointsPerLeg - 1U];
        return status;
    }

    bool decideLegResult (
        uint8_t resultIndex, uint32_t legId,
        const adk::ModuleCharacterizationEvidence& evidence,
        adk::Status status)
    {
        const uint8_t expectedCount =
            resultIndex == 0 ? evidence.ascendingCount :
            resultIndex == 1 ? evidence.descendingCount :
                               evidence.verificationCount;
        const bool bracketExpected = resultIndex < 2;
        const bool bracketPresent =
            resultIndex == 0 ? evidence.ascendingBracket.present :
            resultIndex == 1 ? evidence.descendingBracket.present :
                               false;
        return status.ok () && expectedCount == requiredPointsPerLeg &&
               bracketPresent == bracketExpected &&
               evidence.sessionId == sessionId && evidence.runId == runId &&
               evidence.legId == legId;
    }

    void actuateLegResult (
        uint8_t resultIndex, uint32_t legId,
        const adk::ModuleCharacterizationEvidence& evidence,
        adk::Status status, bool prediction)
    {
        const adk::ModuleTransitionBracket& bracket =
            resultIndex == 0 ? evidence.ascendingBracket :
                               evidence.descendingBracket;
        legResultCells[resultIndex].legId = legId;
        legResultCells[resultIndex].acceptedPoints =
            resultIndex == 0 ? evidence.ascendingCount :
            resultIndex == 1 ? evidence.descendingCount :
                               evidence.verificationCount;
        legResultCells[resultIndex].bracketPresent =
            resultIndex < 2 && bracket.present ? 1 : 0;
        legResultCells[resultIndex].beforeRaw =
            resultIndex < 2 && bracket.present ? bracket.before.frame.analogRaw : 0;
        legResultCells[resultIndex].afterRaw =
            resultIndex < 2 && bracket.present ? bracket.after.frame.analogRaw : 0;
        legResultCells[resultIndex].status =
            static_cast<uint8_t> (status.error ());
        legResultCells[resultIndex].predictionPass = prediction ? 1 : 0;
        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
        ++replayResultCell.completedLegs;
    }

    bool decideFinalResult (
        const adk::ModuleCharacterizationEvidence& evidence)
    {
        return evidence.state == adk::ModuleCharacterizationState::Complete &&
               evidence.reason == adk::ModuleCharacterizationReason::None &&
               evidence.relation == adk::ModuleComparatorRelation::Ambiguous &&
               evidence.ascendingCount == requiredPointsPerLeg &&
               evidence.descendingCount == requiredPointsPerLeg &&
               evidence.verificationCount == requiredPointsPerLeg &&
               evidence.guaranteedInactiveInterval.present &&
               evidence.guaranteedActiveInterval.present &&
               evidence.ambiguityInterval.present;
    }

    void actuateFinalResult (
        const adk::ModuleCharacterizationEvidence& evidence,
        bool prediction)
    {
        characterizationResultCell.inactiveLower =
            evidence.guaranteedInactiveInterval.lower;
        characterizationResultCell.inactiveUpper =
            evidence.guaranteedInactiveInterval.upper;
        characterizationResultCell.activeLower =
            evidence.guaranteedActiveInterval.lower;
        characterizationResultCell.activeUpper =
            evidence.guaranteedActiveInterval.upper;
        characterizationResultCell.ambiguityLower =
            evidence.ambiguityInterval.lower;
        characterizationResultCell.ambiguityUpper =
            evidence.ambiguityInterval.upper;
        characterizationResultCell.state =
            static_cast<uint8_t> (evidence.state);
        characterizationResultCell.reason =
            static_cast<uint8_t> (evidence.reason);
        characterizationResultCell.relation =
            static_cast<uint8_t> (evidence.relation);
        characterizationResultCell.ascendingCount = evidence.ascendingCount;
        characterizationResultCell.descendingCount = evidence.descendingCount;
        characterizationResultCell.verificationCount =
            evidence.verificationCount;
        characterizationResultCell.allIntervalsPresent =
            evidence.guaranteedInactiveInterval.present &&
                    evidence.guaranteedActiveInterval.present &&
                    evidence.ambiguityInterval.present ?
                1 :
                0;
        characterizationResultCell.predictionPass = prediction ? 1 : 0;
        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
    }

    void finishReplay (adk::Status status)
    {
        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && status.ok () ? 1 : 0;
        replayResultCell.complete = 1;
        replayStage               = ReplayStage::Complete;
    }

} // namespace
