// E0 multiplexed-digit fixture. This sketch replays supplied time and copies
// ordered blank/load/select intent into named memory cells. It owns no display,
// pin, timer, interrupt, driver, or powered circuit.
#include <Adk.h>
#include <multiplexed_digit_policy.h>

namespace {

    enum struct ReplayAction : uint8_t
    {
        Commit42,
        Refresh,
        Commit1203,
        CommitOverflow,
        ResetAndRecover,
        Shutdown,
        PostShutdownRefresh
    };

    struct ReplayFrame
    {
        ReplayAction               action;
        uint32_t                   now;
        uint32_t                   value;
        uint32_t                   sourceSequence;
        uint32_t                   expectedGeneration;
        uint8_t                    expectedDigit;
        uint8_t                    expectedSegments;
        adk::Status                expectedStatus;
        adk::MultiplexedDigitFault expectedFault;
    };

    struct DigitIntentCell
    {
        uint32_t now;
        uint32_t frameGeneration;
        uint32_t sourceSequence;
        uint8_t  operationStatus;
        uint8_t  refreshStatus;
        uint8_t  fault;
        uint8_t  emitted;
        uint8_t  digitIndex;
        uint8_t  segmentLevels[3];
        uint8_t  digitSelectLevels[3];
        uint8_t  predictionPass;
    };

    struct CommitIntentCell
    {
        uint32_t generation;
        uint32_t sourceSequence;
        uint8_t  glyphs[4];
        uint8_t  decimalMask;
        uint8_t  overflow;
        uint8_t  predictionPass;
    };

    struct InitialSnapshotCell
    {
        uint32_t ownerToken;
        uint32_t constructionLifecycle;
        uint32_t initializedLifecycle;
        uint16_t configurationRevision;
        uint8_t  constructionStatus;
        uint8_t  constructionInitialized;
        uint8_t  initializedStatus;
        uint8_t  initialized;
        uint8_t  activeGlyphs[4];
        uint8_t  activeDecimalMask;
        uint8_t  activeOverflow;
        uint8_t  pending;
        uint8_t  fault;
        uint8_t  predictionPass;
    };

    const ReplayFrame replayFrames[] = {
        {ReplayAction::Commit42, 100, 42, 58001, 0, 0, 0, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 100, 0, 0, 1, 0, 0x00, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 101, 0, 0, 1, 1, 0x00, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 102, 0, 0, 1, 2, 0x66, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Commit1203, 102, 1203, 58002, 0, 0, 0, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 103, 0, 0, 1, 3, 0xdb, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 104, 0, 0, 2, 0, 0x06, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::CommitOverflow, 104, 10000, 58003, 0, 0, 0, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 105, 0, 0, 2, 1, 0x5b, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 106, 0, 0, 2, 2, 0x3f, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 107, 0, 0, 2, 3, 0x4f, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 108, 0, 0, 3, 0, 0x40, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 112, 0, 0, 3, 1, 0x40, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Refresh, 117, 0, 0, 0, 0, 0x00, adk::StatusCode::Timeout,
         adk::MultiplexedDigitFault::RefreshLost},
        {ReplayAction::ResetAndRecover, 200, 7, 58004, 1, 0, 0x00, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::Shutdown, 201, 0, 0, 0, 0, 0x00, adk::StatusCode::Ok,
         adk::MultiplexedDigitFault::None},
        {ReplayAction::PostShutdownRefresh, 202, 0, 0, 0, 0, 0x00,
         adk::StatusCode::NotInitialized, adk::MultiplexedDigitFault::None}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    const adk::MultiplexedDigitConfig
        digitConfig (580, 58, adk::SevenSegmentPolarity::CommonCathode,
                     adk::DigitSelectPolarity::ActiveHigh);
    adk::MultiplexedDigitPolicy digitPolicy (digitConfig);

    constexpr uint8_t recordedIntentCount = 8;

    volatile DigitIntentCell     resultCells[recordedIntentCount];
    volatile CommitIntentCell    commitCells[3];
    volatile InitialSnapshotCell initialSnapshotCell;
    volatile uint8_t             fixtureStatusCell;
    volatile uint8_t             initializeStatusCell;
    volatile uint8_t             replayActiveCell;
    volatile uint8_t             completedReplayFramesCell;
    volatile uint8_t             allPredictionsPassCell;

    uint8_t replayIndex;
    bool    replayActive;

    // clang-format off
    adk::Status        acquireReplayFixture   ();
    void               configureReplay        ();
    adk::Status        startDigitReplay       ();
    bool               blankFrame             (const adk::MultiplexedDigitFrame& frame);
    bool               expectedPendingFrame   (
        ReplayAction action, const adk::MultiplexedDigitFrame& frame);
    uint8_t            commitCellIndex        (ReplayAction action);
    uint8_t            resultCellIndex        (uint8_t replayFrameIndex);
    const ReplayFrame& observeReplayStage     ();
    adk::Status        decideDigitIntent      (
        const ReplayFrame& frame, adk::MultiplexedDigitTransaction& transaction,
        adk::Status& refreshStatus);
    void presentDigitIntent (
        uint8_t index, const ReplayFrame& frame, adk::Status operationStatus,
        adk::Status refreshStatus,
        const adk::MultiplexedDigitTransaction& transaction);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireReplayFixture ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    configureReplay ();

    const adk::Status initializeStatus = startDigitReplay ();

    initializeStatusCell = static_cast<uint8_t> (initializeStatus.error ());
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame& frame = observeReplayStage ();

    adk::MultiplexedDigitTransaction transaction = {
        0,
        0,
        0,
        0,
        0,
        adk::TimePoint (),
        0,
        {adk::MultiplexedDigitStage::BlankSelects,
         adk::MultiplexedDigitStage::LoadSegments,
         adk::MultiplexedDigitStage::SelectDigit},
        {0, 0, 0},
        {0, 0, 0},
        adk::MultiplexedDigitFault::None,
        false};
    adk::Status       refreshStatus;
    const adk::Status operationStatus =
        decideDigitIntent (frame, transaction, refreshStatus);

    presentDigitIntent (replayIndex, frame, operationStatus, refreshStatus,
                        transaction);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;
    replayActive              = replayIndex < replayFrameCount;
    replayActiveCell          = replayActive ? UINT8_C (1) : UINT8_C (0);
}

namespace {

    adk::Status acquireReplayFixture ()
    {
        return replayFrameCount == 17 ? adk::StatusCode::Ok
                                      : adk::StatusCode::InternalInvariant;
    }

    void configureReplay ()
    {
        replayIndex               = 0;
        replayActive              = false;
        initializeStatusCell      = 0xff;
        replayActiveCell          = 0;
        completedReplayFramesCell = 0;
        allPredictionsPassCell    = 1;
    }

    adk::Status startDigitReplay ()
    {
        const adk::MultiplexedDigitSnapshot construction = digitPolicy.snapshot ();

        initialSnapshotCell.ownerToken            = digitConfig.ownerToken;
        initialSnapshotCell.configurationRevision = digitConfig.configurationRevision;
        initialSnapshotCell.constructionLifecycle = construction.lifecycleGeneration;
        initialSnapshotCell.constructionStatus =
            static_cast<uint8_t> (construction.status.error ());
        initialSnapshotCell.constructionInitialized =
            construction.initialized ? UINT8_C (1) : UINT8_C (0);

        const adk::Status status = digitPolicy.initialize (adk::TimePoint (100));

        const adk::MultiplexedDigitSnapshot initialized = digitPolicy.snapshot ();

        initialSnapshotCell.initializedLifecycle = initialized.lifecycleGeneration;
        initialSnapshotCell.initializedStatus =
            static_cast<uint8_t> (initialized.status.error ());
        initialSnapshotCell.initialized =
            initialized.initialized ? UINT8_C (1) : UINT8_C (0);
        initialSnapshotCell.activeDecimalMask = initialized.activeFrame.decimalMask;
        initialSnapshotCell.activeOverflow =
            initialized.activeFrame.overflow ? UINT8_C (1) : UINT8_C (0);
        initialSnapshotCell.pending = initialized.pending ? UINT8_C (1) : UINT8_C (0);
        initialSnapshotCell.fault   = static_cast<uint8_t> (initialized.fault);

        bool glyphsBlank = true;
        for (uint8_t index = 0; index < 4; ++index)
        {
            initialSnapshotCell.activeGlyphs[index] =
                static_cast<uint8_t> (initialized.activeFrame.glyphs[index]);
            glyphsBlank = glyphsBlank && initialized.activeFrame.glyphs[index] ==
                                             adk::SevenSegmentGlyph::Blank;
        }

        const bool initializeSucceeded = status.ok ();

        const bool initializedStatusOk = initialized.status.ok ();

        initialSnapshotCell.predictionPass =
            construction.status.error () == adk::StatusCode::NotInitialized &&
                    !construction.initialized &&
                    construction.lifecycleGeneration == 0 && initializeSucceeded &&
                    initializedStatusOk && initialized.initialized &&
                    initialized.lifecycleGeneration == 1 && glyphsBlank &&
                    initialized.activeFrame.decimalMask == 0 &&
                    !initialized.activeFrame.overflow && !initialized.pending &&
                    initialized.fault == adk::MultiplexedDigitFault::None
                ? UINT8_C (1)
                : UINT8_C (0);

        if (status.ok ())
        {
            replayActive     = true;
            replayActiveCell = 1;
        }

        return status;
    }

    const ReplayFrame& observeReplayStage ()
    {
        return replayFrames[replayIndex];
    }

    bool blankFrame (const adk::MultiplexedDigitFrame& frame)
    {
        return frame.glyphs[0] == adk::SevenSegmentGlyph::Blank &&
               frame.glyphs[1] == adk::SevenSegmentGlyph::Blank &&
               frame.glyphs[2] == adk::SevenSegmentGlyph::Blank &&
               frame.glyphs[3] == adk::SevenSegmentGlyph::Blank &&
               frame.decimalMask == 0 && frame.sourceSnapshotSequence == 0 &&
               frame.generation == 0 && !frame.overflow;
    }

    bool expectedPendingFrame (ReplayAction                      action,
                               const adk::MultiplexedDigitFrame& frame)
    {
        if (action == ReplayAction::Commit42)
        {
            return frame.glyphs[0] == adk::SevenSegmentGlyph::Blank &&
                   frame.glyphs[1] == adk::SevenSegmentGlyph::Blank &&
                   frame.glyphs[2] == adk::SevenSegmentGlyph::Four &&
                   frame.glyphs[3] == adk::SevenSegmentGlyph::Two &&
                   frame.decimalMask == 1 && frame.sourceSnapshotSequence == 58001 &&
                   frame.generation == 1 && !frame.overflow;
        }
        if (action == ReplayAction::Commit1203)
        {
            return frame.glyphs[0] == adk::SevenSegmentGlyph::One &&
                   frame.glyphs[1] == adk::SevenSegmentGlyph::Two &&
                   frame.glyphs[2] == adk::SevenSegmentGlyph::Zero &&
                   frame.glyphs[3] == adk::SevenSegmentGlyph::Three &&
                   frame.decimalMask == 0 && frame.sourceSnapshotSequence == 58002 &&
                   frame.generation == 2 && !frame.overflow;
        }
        return frame.glyphs[0] == adk::SevenSegmentGlyph::Dash &&
               frame.glyphs[1] == adk::SevenSegmentGlyph::Dash &&
               frame.glyphs[2] == adk::SevenSegmentGlyph::Dash &&
               frame.glyphs[3] == adk::SevenSegmentGlyph::Dash &&
               frame.decimalMask == 0 && frame.sourceSnapshotSequence == 58003 &&
               frame.generation == 3 && frame.overflow;
    }

    uint8_t commitCellIndex (ReplayAction action)
    {
        if (action == ReplayAction::Commit42)
        {
            return 0;
        }
        if (action == ReplayAction::Commit1203)
        {
            return 1;
        }
        return 2;
    }

    uint8_t resultCellIndex (uint8_t replayFrameIndex)
    {
        switch (replayFrameIndex)
        {
            case 1: return 0;
            case 5: return 1;
            case 6: return 2;
            case 11: return 3;
            case 13: return 4;
            case 14: return 5;
            case 15: return 6;
            case 16: return 7;
            default: return 0xff;
        }
    }

    adk::Status decideDigitIntent (const ReplayFrame&                frame,
                                   adk::MultiplexedDigitTransaction& transaction,
                                   adk::Status&                      refreshStatus)
    {
        refreshStatus = adk::StatusCode::Ok;

        if (frame.action == ReplayAction::Shutdown)
        {
            digitPolicy.shutdown ();
            return adk::StatusCode::Ok;
        }

        if (frame.action == ReplayAction::ResetAndRecover)
        {
            digitPolicy.reset (adk::TimePoint (frame.now));
        }

        if (frame.action == ReplayAction::Commit42 ||
            frame.action == ReplayAction::Commit1203 ||
            frame.action == ReplayAction::CommitOverflow ||
            frame.action == ReplayAction::ResetAndRecover)
        {
            adk::MultiplexedDigitPreview candidate;
            const uint8_t                decimalMask =
                frame.action == ReplayAction::Commit42 ? UINT8_C (1) : UINT8_C (0);
            adk::Status status = digitPolicy.preview (frame.value, false, decimalMask,
                                                      frame.sourceSequence, candidate);

            if (status.ok ())
            {
                status = digitPolicy.commit (candidate);
            }
            if (!status.ok () || frame.action != ReplayAction::ResetAndRecover)
            {
                return status;
            }
        }

        const adk::Result<adk::MultiplexedDigitTransaction> result =
            digitPolicy.refresh (adk::TimePoint (frame.now));

        transaction = result.value ();

        refreshStatus = result.status ();

        return result.status ();
    }

    void presentDigitIntent (uint8_t index, const ReplayFrame& frame,
                             adk::Status operationStatus, adk::Status refreshStatus,
                             const adk::MultiplexedDigitTransaction& transaction)
    {
        const adk::MultiplexedDigitSnapshot snapshot = digitPolicy.snapshot ();

        const bool    commitOnly   = frame.action == ReplayAction::Commit42 ||
                                     frame.action == ReplayAction::Commit1203 ||
                                     frame.action == ReplayAction::CommitOverflow;
        const bool    shutdownOnly = frame.action == ReplayAction::Shutdown;
        const bool    postShutdown = frame.action == ReplayAction::PostShutdownRefresh;
        const uint8_t expectedSelect =
            frame.expectedFault == adk::MultiplexedDigitFault::None
                ? static_cast<uint8_t> (1U << frame.expectedDigit)
                : UINT8_C (0);
        const bool ordered =
            commitOnly || shutdownOnly || postShutdown ||
            (transaction.stages[0] == adk::MultiplexedDigitStage::BlankSelects &&
             transaction.stages[1] == adk::MultiplexedDigitStage::LoadSegments &&
             transaction.stages[2] == adk::MultiplexedDigitStage::SelectDigit &&
             transaction.digitSelectLevels[0] == 0 &&
             transaction.digitSelectLevels[1] == 0 &&
             transaction.digitSelectLevels[2] == expectedSelect);
        const uint32_t expectedLifecycle = index >= 14 ? UINT32_C (2) : UINT32_C (1);
        const bool identityMatches = digitConfig.ownerToken == 580 &&
                                     digitConfig.configurationRevision == 58 &&
                                     snapshot.lifecycleGeneration == expectedLifecycle;
        const bool transactionIdentityMatches =
            commitOnly || shutdownOnly ||
            (transaction.ownerToken == 580 && transaction.configurationRevision == 58 &&
             transaction.lifecycleGeneration == expectedLifecycle);
        const bool commitMatches =
            snapshot.pending &&
            expectedPendingFrame (frame.action, snapshot.pendingFrame);

        const bool activeFrameBlank = blankFrame (snapshot.activeFrame);

        const bool pendingFrameBlank = blankFrame (snapshot.pendingFrame);

        const bool shutdownMatches =
            !snapshot.initialized && !snapshot.pending && activeFrameBlank &&
            pendingFrameBlank &&
            snapshot.status.error () == adk::StatusCode::NotInitialized &&
            !transaction.emitted;
        const bool postShutdownMatches =
            shutdownMatches && transaction.segmentLevels[0] == 0 &&
            transaction.segmentLevels[1] == 0 && transaction.segmentLevels[2] == 0 &&
            transaction.digitSelectLevels[0] == 0 &&
            transaction.digitSelectLevels[1] == 0 &&
            transaction.digitSelectLevels[2] == 0;
        const bool refreshMatches =
            transaction.frameGeneration == frame.expectedGeneration &&
            transaction.digitIndex == frame.expectedDigit &&
            transaction.segmentLevels[2] == frame.expectedSegments &&
            transaction.fault == frame.expectedFault && transaction.emitted;
        const bool valueMatches =
            (commitOnly && commitMatches) || (shutdownOnly && shutdownMatches) ||
            (postShutdown && postShutdownMatches) ||
            (!commitOnly && !shutdownOnly && !postShutdown && refreshMatches);

        const bool predictionPass = operationStatus == frame.expectedStatus &&
                                    ordered && identityMatches &&
                                    transactionIdentityMatches && valueMatches;

        if (commitOnly)
        {
            volatile CommitIntentCell& cell =
                commitCells[commitCellIndex (frame.action)];

            cell.generation     = snapshot.pendingFrame.generation;
            cell.sourceSequence = snapshot.pendingFrame.sourceSnapshotSequence;
            cell.decimalMask    = snapshot.pendingFrame.decimalMask;
            cell.overflow = snapshot.pendingFrame.overflow ? UINT8_C (1) : UINT8_C (0);
            for (uint8_t digit = 0; digit < 4; ++digit)
            {
                cell.glyphs[digit] =
                    static_cast<uint8_t> (snapshot.pendingFrame.glyphs[digit]);
            }
            cell.predictionPass = predictionPass ? UINT8_C (1) : UINT8_C (0);
        }

        const uint8_t resultIndex = resultCellIndex (index);
        if (resultIndex != 0xff)
        {
            volatile DigitIntentCell& cell = resultCells[resultIndex];

            cell.now             = frame.now;
            cell.frameGeneration = transaction.frameGeneration;
            cell.sourceSequence  = transaction.sourceSnapshotSequence;
            cell.operationStatus = static_cast<uint8_t> (operationStatus.error ());

            cell.refreshStatus = static_cast<uint8_t> (refreshStatus.error ());
            cell.fault         = static_cast<uint8_t> (transaction.fault);
            cell.emitted       = transaction.emitted ? UINT8_C (1) : UINT8_C (0);
            cell.digitIndex    = transaction.digitIndex;
            for (uint8_t phase = 0; phase < 3; ++phase)
            {
                cell.segmentLevels[phase]     = transaction.segmentLevels[phase];
                cell.digitSelectLevels[phase] = transaction.digitSelectLevels[phase];
            }
            cell.predictionPass = predictionPass ? UINT8_C (1) : UINT8_C (0);
        }

        if (!predictionPass)
        {
            allPredictionsPassCell = 0;
        }
    }

} // namespace
