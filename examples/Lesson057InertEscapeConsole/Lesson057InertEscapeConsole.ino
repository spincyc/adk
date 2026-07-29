// E0 escape-console fixture. This sketch replays copied clue, operator, stop,
// presentation, and audit values. It owns no input, display, storage endpoint,
// lamp, sounder, latch, pin, timer, bus, or powered circuit.
#include <Adk.h>

#include <new>
#include <string.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        Bootstrap,
        Incomplete,
        StaleClue,
        FamiliesEligible,
        PresentationFailure,
        PresentationRecovered,
        DiagnosticAcknowledged,
        Confirmation,
        Solved,
        StopCollision,
        StopAssertedAudit,
        StopReleaseObserved,
        StopReleasedAudit,
        Contradiction,
        TornRestart,
        Recovered,
        InvalidChord
    };

    struct ConsoleResultCell
    {
        uint32_t generation;
        uint32_t operationId;
        uint32_t diagnosticGeneration;
        uint32_t presentationGeneration;
        uint16_t satisfiedRuleMask;
        uint8_t  stage;
        uint8_t  updateStatus;
        uint8_t  disposition;
        uint8_t  auditDisposition;
        uint8_t  latchIntent;
        uint8_t  lampIntent;
        uint8_t  presentationMode;
        uint8_t  diagnostic;
        uint8_t  stopped;
        uint8_t  predictionPass;
    };

    struct FixtureResultCell
    {
        uint32_t policyDigest;
        uint8_t  initializeStatus;
        uint8_t  solvePrepared;
        uint8_t  solveCommitted;
        uint8_t  stopDominated;
        uint8_t  releaseStayedInactive;
        uint8_t  restartRejectedTornImage;
        uint8_t  recoverySolvePrepared;
        uint8_t  complete;
    };

    struct FamilyResultCell
    {
        uint8_t family;
        uint8_t firstClueId;
        uint8_t secondClueId;
        uint8_t complete;
        uint8_t weakestQuality;
    };

    union ReplayResultStorage
    {
        alignas (adk::Result<adk::EscapeConsolePreview>)
            uint8_t solve[sizeof (adk::Result<adk::EscapeConsolePreview>)];
        alignas (adk::Result<adk::PanelAcknowledgePreview>) uint8_t
            acknowledge[sizeof (adk::Result<adk::PanelAcknowledgePreview>)];
    };

    adk::EscapeConsoleConfig configWorkspace;
    alignas (adk::InertEscapeConsole) uint8_t
        consoleStorage[sizeof (adk::InertEscapeConsole)];
    ReplayResultStorage      replayResultStorage;
    adk::InertEscapeConsole* console;

    adk::PanelAuditImage         auditImage;
    adk::PanelAuditImage         recoveryImage;
    adk::EscapeConsoleUpdate     updateFrame;
    adk::EscapeConsolePreview    solvePreview;
    adk::PanelAcknowledgePreview diagnosticAcknowledgment;

    volatile ConsoleResultCell resultCells[17];
    volatile FamilyResultCell  familyResultCells[6];
    volatile FixtureResultCell fixtureResult = {0, 0xff, 0, 0, 0, 0, 0, 0, 0};

    uint32_t clueSequences[12];
    uint32_t controlSequence;
    uint32_t stopSequence;
    uint8_t  replayStage;
    bool     replayActive;
    bool     replayPredictionsPass;

    // clang-format off
    void                    hashByte                (uint32_t& hash, uint8_t value);
    void                    hash16                  (uint32_t& hash, uint16_t value);
    void                    hash32                  (uint32_t& hash, uint32_t value);
    void                    hashClueSource          (
                                uint32_t& hash,
                                const adk::ClueSourceIdentity& source);
    void                    hashOperatorSource      (
                                uint32_t& hash,
                                const adk::OperatorSourceIdentity& source);
    adk::ClueSourceIdentity clueSource              (uint8_t clueId);
    void                    acquireCopiedFixture    ();
    void                    configureFixture        ();
    uint32_t                calculatePolicyDigest   (
                                const adk::EscapeConsoleConfig& config);
    adk::Status             startConsoleFixture    ();

    void                    loadEnvelope            (uint32_t now);

    void                    loadClues               (uint16_t mask, uint32_t now,
                                                    adk::ClueQuality quality);
    void                    loadControl             (uint8_t mask, uint32_t now);
    void                    loadStop                (bool asserted, uint32_t now);
    adk::Status             observeConsole          (ReplayStage stage);
    adk::Status             decideConsolePolicy     (ReplayStage stage);
    adk::Status             preparePanelAudit       (
                                uint32_t operationId,
                                adk::PanelAuditKind kind, uint32_t now);
    adk::Status             prepareTornRestart      (uint32_t now);
    adk::Status             prepareDiagnosticAck    (uint32_t now);
    void                    captureAuditImage       () __attribute__ ((noinline));

    bool                    predictionMatches       (
                                ReplayStage stage,
                                const adk::EscapeConsoleSnapshot& snapshot,
                                const adk::FaultAwareOperatorPanelSnapshot& panel);
    void                    actuateInertIntent       (ReplayStage stage,
                                                     adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    acquireCopiedFixture ();

    configureFixture ();

    const adk::Status initialized = startConsoleFixture ();

    fixtureResult.initializeStatus = static_cast<uint8_t> (initialized.error ());

    replayActive = initialized.ok ();
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayStage stage = static_cast<ReplayStage> (replayStage);

    const adk::Status observed = observeConsole (stage);

    const adk::Status decided = observed.ok () ? decideConsolePolicy (stage) : observed;

    const adk::Status updated = decided.ok () ? console->update (updateFrame) : decided;

    captureAuditImage ();

    actuateInertIntent (stage, updated);

    ++replayStage;
    replayActive = replayStage <= static_cast<uint8_t> (ReplayStage::InvalidChord);

    if (!replayActive)
    {
        fixtureResult.complete =
            updated.ok () && replayPredictionsPass ? UINT8_C (1) : UINT8_C (0);
    }
}

namespace {

    adk::ClueSourceIdentity clueSource (uint8_t clueId)
    {
        return {static_cast<uint16_t> (570U + clueId), 57,
                static_cast<uint32_t> (57000UL + clueId)};
    }

    void acquireCopiedFixture ()
    {
        fixtureResult.policyDigest             = 0;
        fixtureResult.initializeStatus         = 0xff;
        fixtureResult.solvePrepared            = 0;
        fixtureResult.solveCommitted           = 0;
        fixtureResult.stopDominated            = 0;
        fixtureResult.releaseStayedInactive    = 0;
        fixtureResult.restartRejectedTornImage = 0;
        fixtureResult.recoverySolvePrepared    = 0;
        fixtureResult.complete                 = 0;

        for (uint8_t family = 0; family < 6; ++family)
        {
            familyResultCells[family].family         = family;
            familyResultCells[family].firstClueId    = 0;
            familyResultCells[family].secondClueId   = 0;
            familyResultCells[family].complete       = 0;
            familyResultCells[family].weakestQuality = 0xff;
        }
    }

    void hashByte (uint32_t& hash, uint8_t value)
    {
        hash = (hash ^ value) * UINT32_C (0x01000193);
    }

    void hash16 (uint32_t& hash, uint16_t value)
    {
        hashByte (hash, static_cast<uint8_t> (value));
        hashByte (hash, static_cast<uint8_t> (value >> 8));
    }

    void hash32 (uint32_t& hash, uint32_t value)
    {
        for (uint8_t index = 0; index < 4; ++index)
        {
            hashByte (hash, static_cast<uint8_t> (value >> (index * 8)));
        }
    }

    void hashClueSource (uint32_t& hash, const adk::ClueSourceIdentity& source)
    {
        hash16 (hash, source.sourceId);
        hash16 (hash, source.configurationRevision);
        hash32 (hash, source.sessionEpoch);
    }

    void hashOperatorSource (uint32_t& hash, const adk::OperatorSourceIdentity& source)
    {
        hash16 (hash, source.sourceId);
        hash16 (hash, source.configurationRevision);
        hash32 (hash, source.sessionEpoch);
    }

    void configureFixture ()
    {
        configWorkspace = adk::EscapeConsoleConfig ();

        configWorkspace.configurationRevision = 57;
        configWorkspace.instanceEpoch         = 57001;

        adk::ClueConstraintConfig& clue = configWorkspace.clueModel;

        clue.configurationRevision = 55;
        clue.instanceEpoch         = 55057;
        clue.maximumEvidenceAge    = adk::MicrosecondDuration (100);
        clue.clueCount             = 12;
        clue.ruleCount             = 12;

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            clue.expectedSources[clueId] = clueSource (clueId);

            adk::ClueRuleDefinition& rule = clue.rules[clueId];

            rule.ruleId            = clueId;
            rule.termCount         = 1;
            rule.terms[0]          = {clueId, adk::ClueTermRelation::Equals,
                                      adk::ClueCategory::Match};
            rule.prerequisiteCount = clueId == 0 ? 0 : 1;
            rule.prerequisiteRuleIds[0] =
                clueId == 0 ? 0 : static_cast<uint8_t> (clueId - 1);
        }

        const adk::OperatorSourceIdentity controlSource = {71, 56, 57002};
        const adk::OperatorSourceIdentity stopSource    = {72, 56, 57003};

        configWorkspace.panel = {56, 56057,         adk::MicrosecondDuration (100),
                                 12, controlSource, stopSource};

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            configWorkspace.clueFamilies[clueId] =
                static_cast<adk::EscapeClueFamily> (clueId / 2);
            clueSequences[clueId] = 0;
        }

        configWorkspace.policyDigest = calculatePolicyDigest (configWorkspace);
        fixtureResult.policyDigest   = configWorkspace.policyDigest;

        auditImage = adk::PanelAuditImage ();

        recoveryImage = adk::PanelAuditImage ();

        updateFrame = adk::EscapeConsoleUpdate ();

        solvePreview          = adk::EscapeConsolePreview ();
        controlSequence       = 0;
        stopSequence          = 0;
        replayStage           = 0;
        replayActive          = false;
        replayPredictionsPass = true;
    }

    uint32_t calculatePolicyDigest (const adk::EscapeConsoleConfig& config)
    {
        uint32_t   hash     = UINT32_C (0x811c9dc5);
        const char domain[] = "ADK.ESCAPE.POLICY.V1";

        for (uint8_t index = 0; index < sizeof (domain); ++index)
        {
            hashByte (hash, static_cast<uint8_t> (domain[index]));
        }

        hash16 (hash, config.configurationRevision);

        hash32 (hash, config.instanceEpoch);

        const adk::ClueConstraintConfig& clue = config.clueModel;

        hash16 (hash, clue.configurationRevision);

        hash32 (hash, clue.instanceEpoch);

        hash32 (hash, clue.maximumEvidenceAge.microseconds ());

        hashByte (hash, clue.clueCount);

        hashByte (hash, clue.ruleCount);

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            hashClueSource (hash, clue.expectedSources[clueId]);
        }

        for (uint8_t ruleId = 0; ruleId < 12; ++ruleId)
        {
            const adk::ClueRuleDefinition& rule = clue.rules[ruleId];

            hashByte (hash, rule.ruleId);
            hashByte (hash, rule.termCount);
            for (uint8_t termIndex = 0; termIndex < 4; ++termIndex)
            {
                hashByte (hash, rule.terms[termIndex].clueId);
                hashByte (hash, static_cast<uint8_t> (rule.terms[termIndex].relation));
                hashByte (hash, static_cast<uint8_t> (rule.terms[termIndex].category));
            }
            hashByte (hash, rule.prerequisiteCount);
            for (uint8_t prerequisiteIndex = 0; prerequisiteIndex < 4;
                 ++prerequisiteIndex)
            {
                hashByte (hash, rule.prerequisiteRuleIds[prerequisiteIndex]);
            }
        }

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            hashByte (hash, static_cast<uint8_t> (config.clueFamilies[clueId]));
        }

        const adk::FaultAwareOperatorPanelConfig& panel = config.panel;

        hash16 (hash, panel.configurationRevision);

        hash32 (hash, panel.instanceEpoch);

        hash32 (hash, panel.maximumInputAge.microseconds ());

        hashByte (hash, panel.selectableCellCount);

        hashOperatorSource (hash, panel.controlSource);

        hashOperatorSource (hash, panel.stopSource);

        return hash;
    }

    adk::Status startConsoleFixture ()
    {
        console = new (consoleStorage) adk::InertEscapeConsole (configWorkspace);

        return console->initialize ();
    }

    void loadEnvelope (uint32_t now)
    {
        memset (static_cast<void*> (&updateFrame), 0, sizeof (updateFrame));

        updateFrame.now               = adk::MicrosecondTimePoint (now);
        updateFrame.auditImagePresent = true;
        updateFrame.auditImage        = auditImage;
    }

    void loadClues (uint16_t mask, uint32_t now, adk::ClueQuality quality)
    {
        updateFrame.clueUpdatePresent          = true;
        updateFrame.clueUpdate.now             = adk::MicrosecondTimePoint (now);
        updateFrame.clueUpdate.observationMask = mask;

        for (uint8_t clueId = 0; clueId < 12; ++clueId)
        {
            if ((mask & (UINT16_C (1) << clueId)) == 0)
            {
                continue;
            }

            ++clueSequences[clueId];
            updateFrame.clueUpdate.observations[clueId] = {
                clueId,
                adk::ClueCategory::Match,
                quality,
                clueSource (clueId),
                clueSequences[clueId],
                adk::MicrosecondTimePoint (now),

                adk::Status ()};
        }
    }

    void loadControl (uint8_t mask, uint32_t now)
    {
        ++controlSequence;
        updateFrame.controlPresent = true;
        updateFrame.control        = {mask, configWorkspace.panel.controlSource,
                                      controlSequence, adk::MicrosecondTimePoint (now),

                                      adk::Status ()};
    }

    void loadStop (bool asserted, uint32_t now)
    {
        ++stopSequence;
        updateFrame.stopPresent = true;
        updateFrame.stop = {asserted, configWorkspace.panel.stopSource, stopSequence,
                            adk::MicrosecondTimePoint (now), adk::Status ()};
    }

    adk::Status observeConsole (ReplayStage stage)
    {
        const uint32_t now = UINT32_C (10) + static_cast<uint8_t> (stage) * 10U;

        loadEnvelope (now);

        switch (stage)
        {
            case ReplayStage::Bootstrap:
                loadClues (UINT16_C (0), now, adk::ClueQuality::Qualified);
                break;
            case ReplayStage::Incomplete:
                loadClues (UINT16_C (0x003f), now, adk::ClueQuality::Qualified);
                break;
            case ReplayStage::StaleClue:
                loadClues (UINT16_C (0x0001), now, adk::ClueQuality::Stale);
                break;
            case ReplayStage::PresentationFailure:
            case ReplayStage::PresentationRecovered:
            {
                const adk::FaultAwareOperatorPanelSnapshot panel =
                    console->panelSnapshot ();
                updateFrame.presentationPresent = true;
                updateFrame.presentation        = {
                    panel.generation, adk::MicrosecondTimePoint (now),
                    stage == ReplayStage::PresentationFailure
                        ? adk::Status (adk::StatusCode::HardwareFailure)
                        : adk::Status ()};
                break;
            }
            case ReplayStage::FamiliesEligible:
                loadClues (UINT16_C (0x0fff), now, adk::ClueQuality::Qualified);
                break;
            case ReplayStage::Confirmation:
            {
                loadControl (UINT8_C (4), now);

                loadStop (false, now);
                break;
            }
            case ReplayStage::StopCollision:
            {
                loadStop (true, now);
                break;
            }
            case ReplayStage::StopReleaseObserved:
            {
                loadStop (false, now);
                break;
            }
            case ReplayStage::Contradiction:
                loadClues (UINT16_C (0x0001), now, adk::ClueQuality::Contradictory);
                break;
            case ReplayStage::InvalidChord: loadControl (UINT8_C (3), now); break;
            default: break;
        }

        return adk::Status ();
    }

    adk::Status decideConsolePolicy (ReplayStage stage)
    {
        if (stage == ReplayStage::InvalidChord)
        {
            adk::Result<adk::EscapeConsolePreview>* prepared =
                new (replayResultStorage.solve) adk::Result<adk::EscapeConsolePreview> (
                    console->prepareSolve (57005, updateFrame.now));
            fixtureResult.recoverySolvePrepared =
                prepared->ok () ? UINT8_C (1) : UINT8_C (0);
            const adk::Status status = prepared->status ();

            prepared->~Result ();

            if (!status.ok ())
            {
                return status;
            }
        }

        if (stage == ReplayStage::DiagnosticAcknowledged)
        {
            return prepareDiagnosticAck (updateFrame.now.microseconds ());
        }

        if (stage == ReplayStage::Solved || stage == ReplayStage::StopCollision)
        {
            const uint32_t operationId =
                stage == ReplayStage::Solved ? UINT32_C (57001) : UINT32_C (57002);
            adk::Result<adk::EscapeConsolePreview>* prepared =
                new (replayResultStorage.solve) adk::Result<adk::EscapeConsolePreview> (
                    console->prepareSolve (operationId, updateFrame.now));
            fixtureResult.solvePrepared = prepared->ok () ? UINT8_C (1) : UINT8_C (0);

            if (!prepared->ok ())
            {
                const adk::Status status = prepared->status ();

                prepared->~Result ();
                return status;
            }

            solvePreview = prepared->value ();

            prepared->~Result ();
            auditImage.slots[solvePreview.audit.slotIndex] = solvePreview.audit.record;
            updateFrame.auditImage                         = auditImage;
            updateFrame.auditAcknowledgePresent            = true;
            updateFrame.auditAcknowledge                   = solvePreview.audit;
            updateFrame.solvePreviewPresent                = true;
            updateFrame.solvePreview                       = solvePreview;
        }

        if (stage == ReplayStage::StopAssertedAudit)
        {
            return preparePanelAudit (57003, adk::PanelAuditKind::StopAsserted,
                                      updateFrame.now.microseconds ());
        }

        if (stage == ReplayStage::StopReleasedAudit)
        {
            return preparePanelAudit (57004, adk::PanelAuditKind::StopReleased,
                                      updateFrame.now.microseconds ());
        }

        if (stage == ReplayStage::TornRestart)
        {
            return prepareTornRestart (updateFrame.now.microseconds ());
        }

        if (stage == ReplayStage::Recovered)
        {
            auditImage             = recoveryImage;
            updateFrame.auditImage = auditImage;

            loadClues (UINT16_C (0x0fff), updateFrame.now.microseconds (),
                       adk::ClueQuality::Qualified);
        }

        return adk::Status ();
    }

    adk::Status preparePanelAudit (uint32_t operationId, adk::PanelAuditKind kind,
                                   uint32_t now)
    {
        const adk::Result<adk::PanelAuditPreview> prepared =
            console->preparePanelAudit (operationId, kind,
                                        adk::MicrosecondTimePoint (now));
        if (!prepared.ok ())
        {
            return prepared.status ();
        }

        const adk::PanelAuditPreview preview = prepared.value ();

        auditImage.slots[preview.slotIndex] = preview.record;

        updateFrame.auditImage              = auditImage;
        updateFrame.auditAcknowledgePresent = true;
        updateFrame.auditAcknowledge        = preview;

        return adk::Status ();
    }

    adk::Status prepareTornRestart (uint32_t now)
    {
        recoveryImage = auditImage;

        console->shutdown ();

        console->~InertEscapeConsole ();

        console = new (consoleStorage) adk::InertEscapeConsole (configWorkspace);

        const adk::Status status = console->initialize ();

        if (!status.ok ())
        {
            return status;
        }

        auditImage = recoveryImage;
        auditImage.slots[0].checksum ^= UINT32_C (1);

        updateFrame.auditImage = auditImage;
        loadClues (UINT16_C (0), now, adk::ClueQuality::Qualified);

        return status;
    }

    adk::Status prepareDiagnosticAck (uint32_t now)
    {
        adk::Result<adk::PanelAcknowledgePreview>* prepared =
            new (replayResultStorage.acknowledge)
                adk::Result<adk::PanelAcknowledgePreview> (
                    console->preparePanelAcknowledge (57000,
                                                      adk::MicrosecondTimePoint (now)));
        if (!prepared->ok ())
        {
            const adk::Status status = prepared->status ();

            prepared->~Result ();
            return status;
        }

        diagnosticAcknowledgment = prepared->value ();

        prepared->~Result ();

        auditImage.slots[diagnosticAcknowledgment.audit.slotIndex] =
            diagnosticAcknowledgment.audit.record;
        updateFrame.auditImage         = auditImage;
        updateFrame.acknowledgePresent = true;
        updateFrame.acknowledge        = diagnosticAcknowledgment;

        return adk::Status ();
    }

    void __attribute__ ((noinline)) captureAuditImage ()
    {
        auditImage = console->canonicalAuditImage ();
    }

    bool predictionMatches (ReplayStage                                 stage,
                            const adk::EscapeConsoleSnapshot&           snapshot,
                            const adk::FaultAwareOperatorPanelSnapshot& panel)
    {
        adk::EscapeConsoleDisposition expected =
            adk::EscapeConsoleDisposition::AuditPending;
        adk::PanelAuditDisposition expectedAudit = adk::PanelAuditDisposition::Ready;
        uint8_t                    expectedCompleteFamilies = 6;
        bool                       expectedStopped          = false;

        if (stage <= ReplayStage::PresentationRecovered)
        {
            expectedAudit = adk::PanelAuditDisposition::PrepareRequired;
        }
        if (stage == ReplayStage::Bootstrap)
        {
            expectedCompleteFamilies = 0;
        }
        else if (stage == ReplayStage::Incomplete)
        {
            expectedCompleteFamilies = 3;
        }
        else if (stage == ReplayStage::StaleClue)
        {
            expectedCompleteFamilies = 2;
        }
        else if (stage == ReplayStage::Contradiction)
        {
            expectedCompleteFamilies = 5;
        }
        else if (stage == ReplayStage::TornRestart)
        {
            expectedAudit            = adk::PanelAuditDisposition::Corrupt;
            expectedCompleteFamilies = 0;
        }

        switch (stage)
        {
            case ReplayStage::DiagnosticAcknowledged:
            case ReplayStage::Confirmation:
                expected = adk::EscapeConsoleDisposition::AwaitingOperator;
                break;
            case ReplayStage::StaleClue:
                expected = adk::EscapeConsoleDisposition::StaleEvidence;
                break;
            case ReplayStage::Solved:
                expected = adk::EscapeConsoleDisposition::Solved;
                break;
            case ReplayStage::StopCollision:
            case ReplayStage::StopAssertedAudit:
            case ReplayStage::StopReleaseObserved:
                expected        = adk::EscapeConsoleDisposition::Stopped;
                expectedStopped = true;
                break;
            case ReplayStage::StopReleasedAudit:
            case ReplayStage::Recovered:
                expected = adk::EscapeConsoleDisposition::AwaitingOperator;
                break;
            case ReplayStage::Contradiction:
                expected = adk::EscapeConsoleDisposition::ContradictoryEvidence;
                break;
            case ReplayStage::TornRestart:
                expected = adk::EscapeConsoleDisposition::InternalFault;
                break;
            case ReplayStage::InvalidChord:
                expected = adk::EscapeConsoleDisposition::InvalidOperatorChord;
                break;
            default: break;
        }

        const bool latchExpected =
            stage == ReplayStage::Solved
                ? snapshot.latchIntent ==
                      adk::EscapeLatchIntent::RequestDemonstrationRelease
                : snapshot.latchIntent == adk::EscapeLatchIntent::Inactive;

        uint8_t completeFamilies = 0;
        bool    familyMapMatches = true;
        for (uint8_t family = 0; family < 6; ++family)
        {
            completeFamilies = static_cast<uint8_t> (
                completeFamilies +
                (snapshot.families[family].complete ? UINT8_C (1) : UINT8_C (0)));
            familyMapMatches = familyMapMatches &&
                               snapshot.families[family].family ==
                                   static_cast<adk::EscapeClueFamily> (family) &&
                               snapshot.families[family].firstClueId == family * 2 &&
                               snapshot.families[family].secondClueId == family * 2 + 1;
        }

        return snapshot.disposition == expected &&
               snapshot.auditDisposition == expectedAudit &&
               completeFamilies == expectedCompleteFamilies && familyMapMatches &&
               panel.stopped == expectedStopped && latchExpected &&
               (stage != ReplayStage::Recovered ||
                panel.diagnostic == adk::PanelDiagnostic::None);
    }

    void actuateInertIntent (ReplayStage stage, adk::Status status)
    {
        const adk::EscapeConsoleSnapshot snapshot = console->snapshot ();

        const adk::ClueConstraintSnapshot clues = console->clueSnapshot ();

        const adk::FaultAwareOperatorPanelSnapshot panel = console->panelSnapshot ();
        ConsoleResultCell&                         result =
            const_cast<ConsoleResultCell&> (resultCells[static_cast<uint8_t> (stage)]);

        result.generation             = snapshot.generation;
        result.operationId            = snapshot.operationId;
        result.diagnosticGeneration   = panel.diagnosticGeneration;
        result.presentationGeneration = snapshot.presentation.diagnosticGeneration;
        result.satisfiedRuleMask      = clues.satisfiedRuleMask;
        result.stage                  = static_cast<uint8_t> (stage);
        result.updateStatus           = static_cast<uint8_t> (status.error ());
        result.disposition            = static_cast<uint8_t> (snapshot.disposition);
        result.auditDisposition = static_cast<uint8_t> (snapshot.auditDisposition);
        result.latchIntent      = static_cast<uint8_t> (snapshot.latchIntent);
        result.lampIntent       = static_cast<uint8_t> (snapshot.lampIntent);
        result.presentationMode = static_cast<uint8_t> (snapshot.presentation.mode);
        result.diagnostic       = static_cast<uint8_t> (panel.diagnostic);

        result.stopped = panel.stopped ? UINT8_C (1) : UINT8_C (0);

        const bool predictionPass = predictionMatches (stage, snapshot, panel);

        result.predictionPass = predictionPass ? UINT8_C (1) : UINT8_C (0);
        replayPredictionsPass = replayPredictionsPass && predictionPass;

        if (stage == ReplayStage::Solved)
        {
            fixtureResult.solveCommitted =
                snapshot.latchIntent ==
                        adk::EscapeLatchIntent::RequestDemonstrationRelease
                    ? UINT8_C (1)
                    : UINT8_C (0);
        }
        else if (stage == ReplayStage::StopCollision)
        {
            fixtureResult.stopDominated =
                snapshot.latchIntent == adk::EscapeLatchIntent::Inactive ? UINT8_C (1)
                                                                         : UINT8_C (0);
        }
        else if (stage == ReplayStage::StopReleasedAudit)
        {
            fixtureResult.releaseStayedInactive =
                snapshot.latchIntent == adk::EscapeLatchIntent::Inactive &&
                        !panel.stopped
                    ? UINT8_C (1)
                    : UINT8_C (0);
        }
        else if (stage == ReplayStage::TornRestart)
        {
            fixtureResult.restartRejectedTornImage =
                snapshot.disposition == adk::EscapeConsoleDisposition::InternalFault &&
                        snapshot.auditDisposition == adk::PanelAuditDisposition::Corrupt
                    ? UINT8_C (1)
                    : UINT8_C (0);
        }

        for (uint8_t family = 0; family < 6; ++family)
        {
            familyResultCells[family].family =
                static_cast<uint8_t> (snapshot.families[family].family);
            familyResultCells[family].firstClueId =
                snapshot.families[family].firstClueId;
            familyResultCells[family].secondClueId =
                snapshot.families[family].secondClueId;
            familyResultCells[family].complete =
                snapshot.families[family].complete ? UINT8_C (1) : UINT8_C (0);
            familyResultCells[family].weakestQuality =
                static_cast<uint8_t> (snapshot.families[family].weakestQuality);
        }
    }

} // namespace
