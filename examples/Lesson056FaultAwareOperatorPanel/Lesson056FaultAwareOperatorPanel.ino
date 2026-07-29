// E0 operator-panel fixture. This sketch replays copied controls, stop
// evidence, presentation evidence, and a caller-owned audit image. It owns no
// keypad, display, storage endpoint, pin, clock, or durable storage.
#include <Adk.h>

#include <new>

namespace {

    struct RecoveryCell
    {
        uint8_t failureAccepted;
        uint8_t recoveredDiagnostic;
        uint8_t acknowledgmentPrepared;
    };

    struct CollisionCell
    {
        uint8_t updateStatus;
        uint8_t stopped;
        uint8_t presentationMode;
        uint8_t candidateInvalidated;
        uint8_t selectedCellPreserved;
    };

    struct AuditCell
    {
        uint32_t operationId;
        uint32_t recordSequence;
        uint8_t  kind;
        uint8_t  slot;
        uint8_t  slotState;
        uint8_t  disposition;
    };

    struct ReleaseCell
    {
        uint8_t deassertStatus;
        uint8_t auditStatus;
        uint8_t stoppedAfterRelease;
        uint8_t presentationMode;
    };

    struct RestartCell
    {
        uint8_t initializeStatus;
        uint8_t reconcileStatus;
        uint8_t stopped;
        uint8_t auditDisposition;
    };

    struct ReplayCell
    {
        uint8_t initializeStatus;
        uint8_t bootstrapStatus;
        uint8_t navigationStatus;
        uint8_t collisionStatus;
        uint8_t stopAuditStatus;
        uint8_t releaseStatus;
        uint8_t restartStatus;
        uint8_t complete;
    };

    constexpr adk::OperatorSourceIdentity    controlSource = {11, 7, 101};
    constexpr adk::OperatorSourceIdentity    stopSource    = {12, 7, 102};
    const adk::FaultAwareOperatorPanelConfig panelConfig   = {
        7, 41, adk::MicrosecondDuration (100), 12, controlSource, stopSource};

    adk::PanelAuditImage              auditImage     = {};
    adk::FaultAwareOperatorPanelInput inputFrame     = {};
    adk::PanelAcknowledgePreview      acknowledgment = {};
    adk::PanelAuditPreview            auditCandidate = {};
    alignas (adk::Result<adk::PanelAcknowledgePreview>) uint8_t
        acknowledgmentResultStorage[sizeof (adk::Result<adk::PanelAcknowledgePreview>)];
    adk::FaultAwareOperatorPanel panel (panelConfig);

    volatile RecoveryCell  recoveryCell  = {0, 0xff, 0};
    volatile CollisionCell collisionCell = {0xff, 0, 0xff, 0, 0};
    volatile AuditCell     auditCell     = {0, 0, 0xff, 0xff, 0xff, 0xff};
    volatile ReleaseCell   releaseCell   = {0xff, 0xff, 1, 0xff};
    volatile RestartCell   restartCell   = {0xff, 0xff, 1, 0xff};
    volatile ReplayCell    replayCell = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0};

    bool replayActive = false;

    // clang-format off
    void        loadImageFrame                  (uint32_t now);
    void        loadControl                     (uint8_t mask, uint32_t sequence,
                                                 uint32_t observedAt);
    void        loadStop                        (bool asserted, uint32_t sequence,
                                                 uint32_t observedAt);
    adk::Status bootstrapAuditImage             ();
    adk::Status navigatePanel                   ();
    adk::Status observePresentationRecovery     () __attribute__ ((noinline));
    adk::Status decideDiagnosticAcknowledgment  () __attribute__ ((noinline));
    adk::Status actuateStopCollision            () __attribute__ ((noinline));
    uint32_t    observePanelGeneration          () __attribute__ ((noinline));
    uint8_t     observeSelectedCell             () __attribute__ ((noinline));
    void        copyCanonicalAuditImage         () __attribute__ ((noinline));
    void        publishCommittedAudit           (uint8_t slotIndex)
                                                 __attribute__ ((noinline));
    void        publishCollision                (
                    adk::Status status, uint8_t beforeSelectedCell)
                    __attribute__ ((noinline));
    adk::Status decideStopAudit                 (
                    uint32_t operationId, adk::PanelAuditKind kind, uint32_t now)
                    __attribute__ ((noinline));
    adk::Status observeStopRelease              () __attribute__ ((noinline));
    void        publishRelease                  (adk::Status status);
    adk::Status reconcileRestart                () __attribute__ ((noinline));
    adk::Status commitAudit                     (
                    const adk::PanelAuditPreview& preview, uint32_t now)
                    __attribute__ ((noinline));
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status initialized = panel.initialize ();

    replayCell.initializeStatus = static_cast<uint8_t> (initialized.error ());

    replayActive = initialized.ok ();
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    adk::Status status = bootstrapAuditImage ();

    replayCell.bootstrapStatus = static_cast<uint8_t> (status.error ());

    if (status.ok ())
    {
        status = navigatePanel ();
    }
    replayCell.navigationStatus = static_cast<uint8_t> (status.error ());

    // Observe a failed presentation followed by its exact recovery.
    if (status.ok ())
    {
        status = observePresentationRecovery ();
    }
    // Decide whether that recoverable diagnostic may be acknowledged.
    if (status.ok ())
    {
        status = decideDiagnosticAcknowledgment ();
    }
    // Actuate one atomic collision: stop, acknowledgment, and another
    // presentation failure. Stop dominates and invalidates the capability.
    if (status.ok ())
    {
        status = actuateStopCollision ();
    }
    replayCell.collisionStatus = static_cast<uint8_t> (status.error ());

    if (status.ok ())
    {
        status = decideStopAudit (102, adk::PanelAuditKind::StopAsserted, 41);
    }
    if (status.ok ())
    {
        status = commitAudit (auditCandidate, 42);
    }
    replayCell.stopAuditStatus = static_cast<uint8_t> (status.error ());

    if (status.ok ())
    {
        status = observeStopRelease ();
    }
    if (status.ok ())
    {
        status = decideStopAudit (103, adk::PanelAuditKind::StopReleased, 51);
    }
    if (status.ok ())
    {
        status = commitAudit (auditCandidate, 52);
    }
    publishRelease (status);

    replayCell.releaseStatus = static_cast<uint8_t> (status.error ());

    if (status.ok ())
    {
        status = reconcileRestart ();
    }
    replayCell.restartStatus = static_cast<uint8_t> (status.error ());

    replayCell.complete = status.ok () ? UINT8_C (1) : UINT8_C (0);
    replayActive        = false;
}

namespace {

    void loadImageFrame (uint32_t now)
    {
        inputFrame                   = {};
        inputFrame.now               = adk::MicrosecondTimePoint (now);
        inputFrame.auditImagePresent = true;
        inputFrame.auditImage        = auditImage;
    }

    void loadControl (uint8_t mask, uint32_t sequence, uint32_t observedAt)
    {
        inputFrame.controlPresent = true;
        inputFrame.control = {mask, controlSource, sequence,
                              adk::MicrosecondTimePoint (observedAt), adk::Status ()};
    }

    void loadStop (bool asserted, uint32_t sequence, uint32_t observedAt)
    {
        inputFrame.stopPresent = true;
        inputFrame.stop = {asserted, stopSource, sequence,
                           adk::MicrosecondTimePoint (observedAt), adk::Status ()};
    }

    adk::Status bootstrapAuditImage ()
    {
        loadImageFrame (10);

        return panel.update (inputFrame);
    }

    adk::Status navigatePanel ()
    {
        loadImageFrame (20);

        loadControl (UINT8_C (2), 1, 20);

        return panel.update (inputFrame);
    }

    adk::Status observePresentationRecovery ()
    {
        const adk::FaultAwareOperatorPanelSnapshot before = panel.snapshot ();

        loadImageFrame (30);
        inputFrame.presentationPresent = true;
        inputFrame.presentation = {before.generation, adk::MicrosecondTimePoint (30),
                                   adk::StatusCode::HardwareFailure};
        adk::Status status      = panel.update (inputFrame);

        recoveryCell.failureAccepted = status.ok () ? UINT8_C (1) : UINT8_C (0);

        if (status.ok ())
        {
            loadImageFrame (31);
            inputFrame.presentationPresent = true;
            inputFrame.presentation = {before.generation + 1,
                                       adk::MicrosecondTimePoint (31), adk::Status ()};
            status                  = panel.update (inputFrame);
        }

        const adk::FaultAwareOperatorPanelSnapshot recovered = panel.snapshot ();
        recoveryCell.recoveredDiagnostic = static_cast<uint8_t> (recovered.diagnostic);
        return status;
    }

    adk::Status decideDiagnosticAcknowledgment ()
    {
        using AcknowledgeResult = adk::Result<adk::PanelAcknowledgePreview>;
        AcknowledgeResult* result =
            new (acknowledgmentResultStorage) AcknowledgeResult (
                panel.prepareAcknowledge (101, adk::MicrosecondTimePoint (32)));
        recoveryCell.acknowledgmentPrepared = result->ok () ? UINT8_C (1) : UINT8_C (0);

        if (!result->ok ())
        {
            return result->status ();
        }

        acknowledgment                                   = result->value ();
        auditImage.slots[acknowledgment.audit.slotIndex] = acknowledgment.audit.record;
        return adk::Status ();
    }

    adk::Status actuateStopCollision ()
    {
        const uint32_t beforeGeneration = observePanelGeneration ();

        const uint8_t beforeSelectedCell = observeSelectedCell ();

        loadImageFrame (40);

        loadStop (true, 1, 40);

        loadControl (UINT8_C (2), 2, 40);
        inputFrame.acknowledgePresent  = true;
        inputFrame.acknowledge         = acknowledgment;
        inputFrame.presentationPresent = true;
        inputFrame.presentation = {beforeGeneration, adk::MicrosecondTimePoint (40),
                                   adk::StatusCode::HardwareFailure};

        const adk::Status status = panel.update (inputFrame);

        copyCanonicalAuditImage ();

        publishCollision (status, beforeSelectedCell);
        return status;
    }

    uint32_t observePanelGeneration ()
    {
        return panel.snapshot ().generation;
    }

    uint8_t observeSelectedCell ()
    {
        return panel.snapshot ().selectedCell;
    }

    void copyCanonicalAuditImage ()
    {
        auditImage = panel.canonicalAuditImage ();
    }

    void publishCollision (adk::Status status, uint8_t beforeSelectedCell)
    {
        const adk::FaultAwareOperatorPanelSnapshot after = panel.snapshot ();

        collisionCell.updateStatus = static_cast<uint8_t> (status.error ());

        collisionCell.stopped          = after.stopped ? UINT8_C (1) : UINT8_C (0);
        collisionCell.presentationMode = static_cast<uint8_t> (after.presentation.mode);
        collisionCell.candidateInvalidated =
            panel.canAcknowledgeAudit (acknowledgment.audit) ? UINT8_C (0)
                                                             : UINT8_C (1);
        collisionCell.selectedCellPreserved =
            after.selectedCell == beforeSelectedCell ? UINT8_C (1) : UINT8_C (0);
    }

    adk::Status decideStopAudit (uint32_t operationId, adk::PanelAuditKind kind,
                                 uint32_t now)
    {
        const adk::Result<adk::PanelAuditPreview> prepared =
            panel.prepareAudit (operationId, kind, adk::MicrosecondTimePoint (now));
        if (!prepared.ok ())
        {
            return prepared.status ();
        }
        auditCandidate = prepared.value ();

        return adk::Status ();
    }

    adk::Status observeStopRelease ()
    {
        loadImageFrame (50);

        loadStop (false, 2, 50);

        const adk::Status status = panel.update (inputFrame);

        releaseCell.deassertStatus = static_cast<uint8_t> (status.error ());
        return status;
    }

    void publishRelease (adk::Status status)
    {
        const adk::FaultAwareOperatorPanelSnapshot released = panel.snapshot ();

        releaseCell.auditStatus = static_cast<uint8_t> (status.error ());

        releaseCell.stoppedAfterRelease = released.stopped ? UINT8_C (1) : UINT8_C (0);
        releaseCell.presentationMode =
            static_cast<uint8_t> (released.presentation.mode);
    }

    adk::Status reconcileRestart ()
    {
        panel.shutdown ();

        adk::Status status = panel.initialize ();

        restartCell.initializeStatus = static_cast<uint8_t> (status.error ());

        if (status.ok ())
        {
            loadImageFrame (60);

            status = panel.update (inputFrame);
        }

        const adk::FaultAwareOperatorPanelSnapshot restarted = panel.snapshot ();

        restartCell.reconcileStatus = static_cast<uint8_t> (status.error ());

        restartCell.stopped = restarted.stopped ? UINT8_C (1) : UINT8_C (0);
        restartCell.auditDisposition =
            static_cast<uint8_t> (restarted.auditDisposition);
        return status;
    }

    adk::Status commitAudit (const adk::PanelAuditPreview& preview, uint32_t now)
    {
        auditImage.slots[preview.slotIndex] = preview.record;
        loadImageFrame (now);
        inputFrame.auditAcknowledgePresent = true;
        inputFrame.auditAcknowledge        = preview;
        const adk::Status status           = panel.update (inputFrame);

        if (status.ok ())
        {
            copyCanonicalAuditImage ();

            publishCommittedAudit (preview.slotIndex);
        }
        return status;
    }

    void publishCommittedAudit (uint8_t slotIndex)
    {
        const adk::PanelAuditRecord& committed = auditImage.slots[slotIndex];
        auditCell.operationId                  = committed.operationId;
        auditCell.recordSequence               = committed.recordSequence;
        auditCell.kind                         = static_cast<uint8_t> (committed.kind);
        auditCell.slot                         = slotIndex;
        auditCell.slotState                    = static_cast<uint8_t> (committed.state);
        auditCell.disposition =
            static_cast<uint8_t> (panel.snapshot ().auditDisposition);
    }

} // namespace
