// E0 copied-clue fixture. This sketch replays clue cards through fixed DAGs
// and stores evidence-wall intent in named memory cells. It owns no endpoint,
// pin, clock, display, storage medium, actuator, or powered circuit.
#include <Adk.h>

#include <new>

namespace {

    enum struct ReplayStage : uint8_t
    {
        FourRuleProgress,
        FourRuleSolved,
        TwelveRuleSolved,
        StaleEvidence,
        ContradictoryEvidence
    };

    struct ReplayFrame
    {
        ReplayStage               stage;
        uint32_t                  now;
        uint16_t                  observationMask;
        uint8_t                   evidenceClueId;
        adk::ClueModelDisposition expectedDisposition;
        uint16_t                  expectedSatisfiedMask;
        uint16_t                  expectedBlockedMask;
    };

    struct EvidenceWallResultCell
    {
        uint32_t generation;
        uint32_t sourceSessionEpoch;
        uint32_t sourceSequence;
        uint32_t observedAt;
        uint16_t sourceId;
        uint16_t sourceConfigurationRevision;
        uint16_t satisfiedRuleMask;
        uint16_t blockedRuleMask;
        uint8_t  category;
        uint8_t  quality;
        uint8_t  disposition;
        uint8_t  operationStatus;
        uint8_t  firstRuleDisposition;
        uint8_t  finalRuleDisposition;
        uint8_t  predictionPass;
    };

    const ReplayFrame replayFrames[] = {
        {ReplayStage::FourRuleProgress, 100, 0x0001, 0,
         adk::ClueModelDisposition::Incomplete, 0x0001, 0x000e},
        {ReplayStage::FourRuleSolved, 110, 0x000f, 3, adk::ClueModelDisposition::Solved,
         0x000f, 0x0000},
        {ReplayStage::TwelveRuleSolved, 120, 0x0fff, 11,
         adk::ClueModelDisposition::Solved, 0x0fff, 0x0000},
        {ReplayStage::StaleEvidence, 250, 0x0fff, 0,
         adk::ClueModelDisposition::StaleEvidence, 0x0000, 0x0fff},
        {ReplayStage::ContradictoryEvidence, 260, 0x0fff, 5,
         adk::ClueModelDisposition::ContradictoryEvidence, 0x001f, 0x0fe0}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::ClueConstraintConfig configWorkspace;
    alignas (adk::ClueConstraintModel) uint8_t
        modelStorage[sizeof (adk::ClueConstraintModel)];
    adk::ClueConstraintModel* clueModel;

    volatile EvidenceWallResultCell evidenceWallCells[replayFrameCount];
    volatile uint8_t                fixtureStatusCell;
    volatile uint8_t                initializeStatusCell;
    volatile uint8_t                cycleRejectionStatusCell;
    volatile uint8_t                cycleRejectionDispositionCell;
    volatile uint8_t                completedReplayFramesCell;
    volatile uint8_t                replayActiveCell;

    uint32_t clueSequences[12];
    uint8_t  replayIndex;
    uint8_t  activeRuleCount;
    bool     replayActive;

    // clang-format off
    adk::ClueSourceIdentity   clueCardSource            (uint8_t clueId);
    void                      configureDag              (uint8_t count,
                                                        bool    addCycle);
    void                      constructConfiguredModel  ();
    void                      destroyConfiguredModel    ();
    adk::Status               acquireClueCardFixture    ();
    void                      configureClueCardReplay   ();
    adk::Status               startFourRuleReplay       ();
    adk::Status               startTwelveRuleReplay     ();
    adk::ClueConstraintUpdate observeCopiedClueCards    (const ReplayFrame& frame);
    adk::Status               decideClueProgress        (
        const adk::ClueConstraintUpdate& update);
    void                      presentEvidenceWallIntent (
        uint8_t index, const ReplayFrame& frame, adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireClueCardFixture ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    configureClueCardReplay ();

    const adk::Status initializationStatus = startFourRuleReplay ();

    initializeStatusCell = static_cast<uint8_t> (initializationStatus.error ());

    if (!initializationStatus.ok ())
    {
        return;
    }
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame& frame = replayFrames[replayIndex];

    const adk::ClueConstraintUpdate update = observeCopiedClueCards (frame);

    const adk::Status operationStatus = decideClueProgress (update);

    presentEvidenceWallIntent (replayIndex, frame, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;

    if (replayIndex == 2)
    {
        const adk::Status transitionStatus = startTwelveRuleReplay ();

        if (!transitionStatus.ok ())
        {
            replayActive     = false;
            replayActiveCell = 0;
            return;
        }
    }

    replayActive     = replayIndex < replayFrameCount;
    replayActiveCell = replayActive ? 1 : 0;
}

namespace {

    adk::ClueSourceIdentity clueCardSource (uint8_t clueId)
    {
        return {static_cast<uint16_t> (550U + clueId), 55,
                static_cast<uint32_t> (55000UL + clueId)};
    }

    void configureDag (uint8_t count, bool addCycle)
    {
        configWorkspace.configurationRevision = 55;
        configWorkspace.instanceEpoch         = count == 4 ? 55004 : 55012;
        configWorkspace.maximumEvidenceAge    = adk::MicrosecondDuration (100);
        configWorkspace.clueCount             = count;
        configWorkspace.ruleCount             = count;

        for (uint8_t index = 0; index < 12; ++index)
        {
            configWorkspace.expectedSources[index]         = {0, 0, 0};
            configWorkspace.rules[index].ruleId            = 0;
            configWorkspace.rules[index].termCount         = 0;
            configWorkspace.rules[index].prerequisiteCount = 0;

            for (uint8_t slot = 0; slot < 4; ++slot)
            {
                configWorkspace.rules[index].terms[slot] = {
                    0, adk::ClueTermRelation::Equals, adk::ClueCategory::Absent};
                configWorkspace.rules[index].prerequisiteRuleIds[slot] = 0;
            }
        }

        for (uint8_t ruleId = 0; ruleId < count; ++ruleId)
        {
            configWorkspace.expectedSources[ruleId] = clueCardSource (ruleId);

            adk::ClueRuleDefinition& rule = configWorkspace.rules[ruleId];

            rule.ruleId    = ruleId;
            rule.termCount = 1;
            rule.terms[0]  = {ruleId, adk::ClueTermRelation::Equals,
                              adk::ClueCategory::Match};
        }

        configWorkspace.rules[1].prerequisiteCount      = 1;
        configWorkspace.rules[1].prerequisiteRuleIds[0] = 0;
        configWorkspace.rules[2].prerequisiteCount      = 1;
        configWorkspace.rules[2].prerequisiteRuleIds[0] = 0;
        configWorkspace.rules[3].prerequisiteCount      = 2;
        configWorkspace.rules[3].prerequisiteRuleIds[0] = 1;
        configWorkspace.rules[3].prerequisiteRuleIds[1] = 2;

        for (uint8_t ruleId = 4; ruleId < count; ++ruleId)
        {
            configWorkspace.rules[ruleId].prerequisiteCount = 1;
            configWorkspace.rules[ruleId].prerequisiteRuleIds[0] =
                static_cast<uint8_t> (ruleId - 1);
        }

        if (addCycle)
        {
            configWorkspace.rules[0].prerequisiteCount      = 1;
            configWorkspace.rules[0].prerequisiteRuleIds[0] = 3;
        }
    }

    void constructConfiguredModel ()
    {
        clueModel = new (modelStorage) adk::ClueConstraintModel (configWorkspace);
    }

    void destroyConfiguredModel ()
    {
        clueModel->~ClueConstraintModel ();
        clueModel = nullptr;
    }

    adk::Status acquireClueCardFixture ()
    {
        configureDag (4, true);

        constructConfiguredModel ();

        const adk::Status cycleStatus = clueModel->initialize ();

        cycleRejectionStatusCell = static_cast<uint8_t> (cycleStatus.error ());
        cycleRejectionDispositionCell =
            static_cast<uint8_t> (clueModel->snapshot ().disposition);
        const bool rejected =
            cycleStatus.error () == adk::StatusCode::InvalidConfiguration &&

            clueModel->snapshot ().disposition ==
                adk::ClueModelDisposition::InvalidConfiguration;

        destroyConfiguredModel ();

        return rejected ? adk::Status () : adk::StatusCode::InternalInvariant;
    }

    void configureClueCardReplay ()
    {
        initializeStatusCell      = 0xff;
        completedReplayFramesCell = 0;
        replayActiveCell          = 0;
        replayIndex               = 0;
        replayActive              = false;

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            clueSequences[clueId] = 0;
        }
    }

    adk::Status startFourRuleReplay ()
    {
        configureDag (4, false);

        constructConfiguredModel ();
        activeRuleCount = 4;

        const adk::Status status = clueModel->initialize ();

        replayActive     = status.ok ();
        replayActiveCell = replayActive ? 1 : 0;
        return status;
    }

    adk::Status startTwelveRuleReplay ()
    {
        destroyConfiguredModel ();

        configureDag (12, false);

        constructConfiguredModel ();
        activeRuleCount = 12;

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            clueSequences[clueId] = 0;
        }
        return clueModel->initialize ();
    }

    adk::ClueConstraintUpdate observeCopiedClueCards (const ReplayFrame& frame)
    {
        adk::ClueConstraintUpdate update = {};

        update.now             = adk::MicrosecondTimePoint (frame.now);
        update.observationMask = frame.observationMask;

        for (uint8_t clueId = 0; clueId < activeRuleCount; ++clueId)
        {
            if ((frame.observationMask & (UINT16_C (1) << clueId)) == 0)
            {
                continue;
            }

            ++clueSequences[clueId];
            const bool staleStage = frame.stage == ReplayStage::StaleEvidence;
            const bool contradiction =
                frame.stage == ReplayStage::ContradictoryEvidence && clueId == 5;

            update.observations[clueId] = {
                clueId,
                adk::ClueCategory::Match,
                contradiction ? adk::ClueQuality::Contradictory
                              : adk::ClueQuality::Qualified,
                clueCardSource (clueId),
                clueSequences[clueId],
                adk::MicrosecondTimePoint (staleStage ? 120 : frame.now),

                adk::Status ()};
        }
        return update;
    }

    adk::Status decideClueProgress (const adk::ClueConstraintUpdate& update)
    {
        return clueModel->update (update);
    }

    void presentEvidenceWallIntent (uint8_t index, const ReplayFrame& frame,
                                    adk::Status operationStatus)
    {
        const adk::ClueConstraintSnapshot snapshot = clueModel->snapshot ();

        const adk::Result<adk::ClueRuleSnapshot> firstRule = clueModel->rule (0);
        const adk::Result<adk::ClueRuleSnapshot> finalRule =
            clueModel->rule (static_cast<uint8_t> (activeRuleCount - 1));
        const adk::Result<adk::ClueEvidenceSnapshot> evidence =
            clueModel->evidence (frame.evidenceClueId);

        EvidenceWallResultCell cell = {};
        cell.generation             = snapshot.generation;
        cell.satisfiedRuleMask      = snapshot.satisfiedRuleMask;
        cell.blockedRuleMask        = snapshot.blockedRuleMask;
        cell.disposition            = static_cast<uint8_t> (snapshot.disposition);
        cell.operationStatus        = static_cast<uint8_t> (operationStatus.error ());
        cell.firstRuleDisposition =
            firstRule.ok () ? static_cast<uint8_t> (firstRule.value ().disposition)
                            : 0xff;
        cell.finalRuleDisposition =
            finalRule.ok () ? static_cast<uint8_t> (finalRule.value ().disposition)
                            : 0xff;

        if (evidence.ok ())
        {
            cell.sourceId = evidence.value ().source.sourceId;
            cell.sourceConfigurationRevision =
                evidence.value ().source.configurationRevision;
            cell.sourceSessionEpoch = evidence.value ().source.sessionEpoch;

            cell.sourceSequence = evidence.value ().sourceSequence;

            cell.observedAt = evidence.value ().observedAt.microseconds ();

            cell.category = static_cast<uint8_t> (evidence.value ().category);

            cell.quality = static_cast<uint8_t> (evidence.value ().quality);
        }

        cell.predictionPass =
            operationStatus.ok () && evidence.ok () &&
                    snapshot.disposition == frame.expectedDisposition &&
                    snapshot.satisfiedRuleMask == frame.expectedSatisfiedMask &&
                    snapshot.blockedRuleMask == frame.expectedBlockedMask
                ? 1
                : 0;

        evidenceWallCells[index].generation         = cell.generation;
        evidenceWallCells[index].sourceSessionEpoch = cell.sourceSessionEpoch;
        evidenceWallCells[index].sourceSequence     = cell.sourceSequence;
        evidenceWallCells[index].observedAt         = cell.observedAt;
        evidenceWallCells[index].sourceId           = cell.sourceId;
        evidenceWallCells[index].sourceConfigurationRevision =
            cell.sourceConfigurationRevision;
        evidenceWallCells[index].satisfiedRuleMask    = cell.satisfiedRuleMask;
        evidenceWallCells[index].blockedRuleMask      = cell.blockedRuleMask;
        evidenceWallCells[index].category             = cell.category;
        evidenceWallCells[index].quality              = cell.quality;
        evidenceWallCells[index].disposition          = cell.disposition;
        evidenceWallCells[index].operationStatus      = cell.operationStatus;
        evidenceWallCells[index].firstRuleDisposition = cell.firstRuleDisposition;
        evidenceWallCells[index].finalRuleDisposition = cell.finalRuleDisposition;
        evidenceWallCells[index].predictionPass       = cell.predictionPass;
    }

} // namespace
