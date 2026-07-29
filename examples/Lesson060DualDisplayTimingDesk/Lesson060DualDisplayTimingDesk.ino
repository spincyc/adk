// E0 dual-display timing-desk fixture. This sketch replays copied controls,
// supplied time, logical frame receipts, and recording-transport receipts. It
// owns no button, display, timer, SPI wire, pin, interrupt, or powered circuit.
#include <Adk.h>
#include <dual_display_timing_desk.h>

namespace {

    struct StopwatchResultCell
    {
        uint32_t generation;
        uint32_t elapsed;
        uint32_t lapElapsed;
        uint8_t  state;
        uint8_t  qualification;
        uint8_t  lapVisible;
    };

    struct DigitPresentationResultCell
    {
        uint32_t sourceGeneration;
        uint32_t digest;
        uint8_t  glyphs[4];
        uint8_t  decimalMask;
        uint8_t  refreshDisposition;
        uint8_t  transactionPresent;
    };

    struct MatrixPresentationResultCell
    {
        uint32_t sourceGeneration;
        uint32_t digest;
        uint8_t  rows[8];
        uint8_t  progressStep;
        uint8_t  transportDisposition;
        uint8_t  commandPresent;
    };

    struct DisagreementResultCell
    {
        uint32_t expectedGeneration;
        uint32_t digitGeneration;
        uint32_t matrixGeneration;
        uint8_t  disposition;
        uint8_t  faultOwner;
        uint8_t  digitBlankRequested;
        uint8_t  matrixBlankRequested;
    };

    struct SelfTestResultCell
    {
        uint8_t stage;
        uint8_t digitAccepted;
        uint8_t matrixAccepted;
        uint8_t timePresentationAllowed;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t initializeStatus;
        uint8_t updateStatus;
        uint8_t completedStages;
        uint8_t predictionsPass;
        uint8_t complete;
    };

    enum struct ReplayStage : uint8_t
    {
        Qualify,
        Start,
        Advance,
        Lap,
        LapHeld,
        BadReceipt,
        ResetRecovery,
        Shutdown,
        Complete
    };

    constexpr adk::TimingDeskControlIdentity startPauseIdentity = {601, 60, 60001};
    constexpr adk::TimingDeskControlIdentity lapIdentity        = {602, 60, 60002};
    constexpr adk::TimingDeskControlIdentity resetIdentity      = {603, 60, 60003};

    const adk::MultiplexedDigitConfig
        digitConfig (6058, 60, adk::SevenSegmentPolarity::CommonCathode,
                     adk::DigitSelectPolarity::ActiveHigh);
    const adk::Max7219PresentationConfig
        matrixConfig (6059, 60, adk::Max7219Orientation::Identity, 1);
    const adk::DualDisplayTimingDeskConfig deskConfig (6060, 60, digitConfig,
                                                       matrixConfig, startPauseIdentity,
                                                       lapIdentity, resetIdentity);
    adk::DualDisplayTimingDesk             timingDesk (deskConfig);

    volatile StopwatchResultCell          stopwatchResultCell;
    volatile DigitPresentationResultCell  digitPresentationResultCell;
    volatile MatrixPresentationResultCell matrixPresentationResultCell;
    volatile DisagreementResultCell       disagreementResultCell;
    volatile SelfTestResultCell           selfTestResultCell;
    volatile ReplayResultCell             replayResultCell;

    ReplayStage                        replayStage;
    uint32_t                           suppliedNow;
    uint32_t                           controlSequence;
    uint32_t                           digitLifecycle;
    uint32_t                           matrixLifecycle;
    uint32_t                           digitReadyGeneration;
    uint32_t                           matrixReadyGeneration;
    uint16_t                           serviceCount;
    uint8_t                            observedSelfTestStage;
    adk::Max7219Command                priorMatrixCommand;
    bool                               haveMatrixCommand;
    adk::DualDisplayTimingDeskSnapshot workingBefore;
    adk::DualDisplayTimingDeskSnapshot workingAfter;
    adk::DualDisplayEnvelope           workingEnvelope;
    adk::DualDisplayTimingDeskResult   workingResult;

    // clang-format off
    adk::Status acquireTimingDeskFixture ();

    void configureTimingDeskReplay ();

    adk::Status startDisplaySelfTests ();

    adk::TimingDeskControlEvidence copiedControl (
        const adk::TimingDeskControlIdentity& identity, bool pressed);
    adk::DigitFrameReceipt copiedDigitReceipt (
        const adk::DualDisplayTimingDeskSnapshot& snapshot);
    adk::MatrixFrameReceipt copiedMatrixReceipt (
        const adk::DualDisplayTimingDeskSnapshot& snapshot);
    adk::Max7219Receipt copiedTransportReceipt ();

    adk::DualDisplayEnvelope observeDeskFrame ();

    bool updateTimingDesk ();

    bool decideTimingPresentation (
        const adk::DualDisplayTimingDeskSnapshot& before,
        const adk::DualDisplayTimingDeskResult& result,
        const adk::DualDisplayTimingDeskSnapshot& after);
    void actuatePresentationIntent (
        const adk::DualDisplayTimingDeskResult& result,
        const adk::DualDisplayTimingDeskSnapshot& snapshot, bool prediction);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireTimingDeskFixture ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    configureTimingDeskReplay ();

    const adk::Status initializeStatus = startDisplaySelfTests ();
    replayResultCell.initializeStatus =
        static_cast<uint8_t> (initializeStatus.error ());
}

void loop ()
{
    if (replayStage == ReplayStage::Shutdown)
    {
        timingDesk.shutdown ();

        workingAfter = timingDesk.snapshot ();

        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && !workingAfter.initialized ? 1 : 0;
        replayResultCell.complete = 1;
        replayStage               = ReplayStage::Complete;
        return;
    }

    if (replayStage == ReplayStage::Complete)
    {
        return;
    }

    ++serviceCount;
    if (serviceCount > 600)
    {
        replayResultCell.predictionsPass = 0;
        replayStage                      = ReplayStage::Shutdown;
        return;
    }

    workingBefore = timingDesk.snapshot ();

    workingEnvelope = observeDeskFrame ();

    if (!updateTimingDesk ())
    {
        return;
    }
    workingAfter = timingDesk.snapshot ();
    const bool prediction =
        decideTimingPresentation (workingBefore, workingResult, workingAfter);

    actuatePresentationIntent (workingResult, workingAfter, prediction);
}

namespace {

    adk::Status acquireTimingDeskFixture ()
    {
        const bool identitiesDistinct =
            startPauseIdentity.sourceId != lapIdentity.sourceId &&
            startPauseIdentity.sourceId != resetIdentity.sourceId &&
            lapIdentity.sourceId != resetIdentity.sourceId;
        return identitiesDistinct ? adk::StatusCode::Ok
                                  : adk::StatusCode::InvalidConfiguration;
    }

    void configureTimingDeskReplay ()
    {
        replayStage                      = ReplayStage::Qualify;
        suppliedNow                      = 1000;
        controlSequence                  = 0;
        digitLifecycle                   = 1;
        matrixLifecycle                  = 1;
        digitReadyGeneration             = 0;
        matrixReadyGeneration            = 0;
        serviceCount                     = 0;
        observedSelfTestStage            = 0xff;
        haveMatrixCommand                = false;
        replayResultCell.updateStatus    = 0xff;
        replayResultCell.completedStages = 0;
        replayResultCell.predictionsPass = 1;
        replayResultCell.complete        = 0;
    }

    adk::Status startDisplaySelfTests ()
    {
        return timingDesk.initialize (adk::TimePoint (suppliedNow));
    }

    adk::TimingDeskControlEvidence
    copiedControl (const adk::TimingDeskControlIdentity& identity, bool pressed)
    {
        return {identity,
                controlSequence,
                adk::TimePoint (suppliedNow),
                adk::StatusCode::Ok,
                pressed,
                pressed};
    }

    adk::DigitFrameReceipt
    copiedDigitReceipt (const adk::DualDisplayTimingDeskSnapshot& snapshot)
    {
        return {digitConfig.ownerToken,
                digitLifecycle,
                digitConfig.configurationRevision,
                snapshot.presentationGeneration,
                snapshot.presentationGeneration,
                snapshot.digitFrame,
                snapshot.digitDigest,
                adk::TimePoint (suppliedNow),
                adk::StatusCode::Ok,
                false};
    }

    adk::MatrixFrameReceipt
    copiedMatrixReceipt (const adk::DualDisplayTimingDeskSnapshot& snapshot)
    {
        return {matrixConfig.ownerToken,
                matrixLifecycle,
                matrixConfig.configurationRevision,
                snapshot.presentationGeneration,
                snapshot.presentationGeneration,
                snapshot.matrixFrame,
                snapshot.matrixDigest,
                adk::TimePoint (suppliedNow),
                adk::StatusCode::Ok,
                false};
    }

    adk::Max7219Receipt copiedTransportReceipt ()
    {
        return {priorMatrixCommand.ownerToken,
                priorMatrixCommand.lifecycleGeneration,
                priorMatrixCommand.configurationRevision,
                priorMatrixCommand.presentationGeneration,
                priorMatrixCommand.operationIndex,
                priorMatrixCommand.registerAddress,
                priorMatrixCommand.data,
                priorMatrixCommand.operation,
                2,
                true,
                adk::TimePoint (suppliedNow),
                adk::StatusCode::Ok};
    }

    adk::DualDisplayEnvelope observeDeskFrame ()
    {
        const adk::DualDisplayTimingDeskSnapshot snapshot = timingDesk.snapshot ();
        static adk::DigitFrameReceipt            digitReceipt;
        static adk::MatrixFrameReceipt           matrixReceipt;
        static adk::Max7219Receipt               transportReceipt;

        const bool presentationPending =
            snapshot.presentationGeneration != 0 &&
            (snapshot.presentationDisposition ==
                 adk::TimingDeskPresentationDisposition::Pending ||
             snapshot.presentationDisposition ==
                 adk::TimingDeskPresentationDisposition::SelfTest);
        const bool pressStart = replayStage == ReplayStage::Start;
        const bool pressLap   = replayStage == ReplayStage::Lap;
        const bool pressReset = replayStage == ReplayStage::ResetRecovery;

        ++controlSequence;
        ++suppliedNow;

        const adk::DigitFrameReceipt*  digitPointer     = nullptr;
        const adk::MatrixFrameReceipt* matrixPointer    = nullptr;
        const adk::Max7219Receipt*     transportPointer = nullptr;

        if (presentationPending &&
            digitReadyGeneration == snapshot.presentationGeneration)
        {
            digitReceipt = copiedDigitReceipt (snapshot);
            digitPointer = &digitReceipt;
        }
        if (presentationPending &&
            matrixReadyGeneration == snapshot.presentationGeneration)
        {
            matrixReceipt = copiedMatrixReceipt (snapshot);
            if (replayStage == ReplayStage::BadReceipt)
            {
                matrixReceipt.reportedDigest ^= UINT32_C (1);
            }
            matrixPointer = &matrixReceipt;
        }
        if (haveMatrixCommand)
        {
            transportReceipt  = copiedTransportReceipt ();
            transportPointer  = &transportReceipt;
            haveMatrixCommand = false;
        }

        return {adk::TimePoint (suppliedNow),
                copiedControl (startPauseIdentity, pressStart),
                copiedControl (lapIdentity, pressLap),
                copiedControl (resetIdentity, pressReset),
                digitPointer,
                matrixPointer,
                transportPointer};
    }

    bool updateTimingDesk ()
    {
        const adk::Result<adk::DualDisplayTimingDeskResult> decided =
            timingDesk.update (workingEnvelope);
        replayResultCell.updateStatus =
            static_cast<uint8_t> (decided.status ().error ());
        workingResult = decided.value ();

        if (!decided.ok ())
        {
            const bool expectedDisagreement =
                replayStage == ReplayStage::BadReceipt &&
                decided.error () == adk::StatusCode::HardwareFailure;
            replayResultCell.predictionsPass =
                replayResultCell.predictionsPass && expectedDisagreement ? 1 : 0;
            if (expectedDisagreement)
            {
                workingAfter = timingDesk.snapshot ();
                const bool faultCaptured =
                    workingAfter.faultOwner ==
                        adk::TimingDeskFaultOwner::MatrixDisplay &&
                    workingAfter.presentationDisposition ==
                        adk::TimingDeskPresentationDisposition::Disagreement &&
                    workingAfter.qualification == adk::TimingDeskQualification::Fault &&
                    workingResult.digitBlankRequested &&
                    workingResult.matrixBlankRequested;
                replayResultCell.predictionsPass =
                    replayResultCell.predictionsPass && faultCaptured ? 1 : 0;
                actuatePresentationIntent (workingResult, workingAfter, faultCaptured);
                replayStage = ReplayStage::ResetRecovery;
                ++replayResultCell.completedStages;
            }
            return false;
        }
        return true;
    }

    bool decideTimingPresentation (const adk::DualDisplayTimingDeskSnapshot& before,
                                   const adk::DualDisplayTimingDeskResult&   result,
                                   const adk::DualDisplayTimingDeskSnapshot& after)
    {
        bool prediction = result.controlStatus.ok ();

        if (result.digitTransactionPresent)
        {
            digitLifecycle = result.digitTransaction.lifecycleGeneration;
            if (result.digitTransaction.frameGeneration ==
                    after.presentationGeneration &&
                result.digitTransaction.digitIndex == 0)
            {
                digitReadyGeneration = after.presentationGeneration;
            }
        }
        if (result.matrixCommandPresent)
        {
            matrixLifecycle    = result.matrixCommand.lifecycleGeneration;
            priorMatrixCommand = result.matrixCommand;
            haveMatrixCommand  = true;
        }
        if (workingEnvelope.transportReceipt != nullptr &&
            workingEnvelope.transportReceipt->operation ==
                adk::Max7219Operation::SubmitRow &&
            workingEnvelope.transportReceipt->registerAddress == 8)
        {
            matrixReadyGeneration =
                workingEnvelope.transportReceipt->presentationGeneration;
        }

        if (replayStage == ReplayStage::Qualify &&
            after.qualification == adk::TimingDeskQualification::Ready)
        {
            replayStage = ReplayStage::Start;
            ++replayResultCell.completedStages;
        }
        else if (replayStage == ReplayStage::Qualify &&
                 after.selfTestStage != observedSelfTestStage)
        {
            observedSelfTestStage = after.selfTestStage;
            ++replayResultCell.completedStages;
        }
        else if (replayStage == ReplayStage::Start)
        {
            prediction = prediction &&
                         after.stopwatchState == adk::TimingDeskStopwatchState::Running;
            replayStage = ReplayStage::Advance;
            ++replayResultCell.completedStages;
        }
        else if (replayStage == ReplayStage::Advance &&
                 after.elapsed.milliseconds () >= 250)
        {
            replayStage = ReplayStage::Lap;
            ++replayResultCell.completedStages;
        }
        else if (replayStage == ReplayStage::Lap)
        {
            prediction  = prediction && after.lapVisible;
            replayStage = ReplayStage::LapHeld;
            ++replayResultCell.completedStages;
        }
        else if (replayStage == ReplayStage::LapHeld)
        {
            prediction =
                prediction && after.lapVisible && after.elapsed >= before.elapsed;
            replayStage = ReplayStage::BadReceipt;
            ++replayResultCell.completedStages;
        }
        else if (replayStage == ReplayStage::ResetRecovery)
        {
            prediction =
                prediction && after.elapsed == adk::Duration (0) &&
                after.qualification == adk::TimingDeskQualification::Configuring;
            replayStage = ReplayStage::Shutdown;
            ++replayResultCell.completedStages;
        }

        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
        return prediction;
    }

    void actuatePresentationIntent (const adk::DualDisplayTimingDeskResult&   result,
                                    const adk::DualDisplayTimingDeskSnapshot& snapshot,
                                    bool)
    {
        stopwatchResultCell.generation = snapshot.presentationGeneration;
        stopwatchResultCell.elapsed    = snapshot.elapsed.milliseconds ();

        stopwatchResultCell.lapElapsed = snapshot.lapElapsed.milliseconds ();
        stopwatchResultCell.state      = static_cast<uint8_t> (snapshot.stopwatchState);
        stopwatchResultCell.qualification =
            static_cast<uint8_t> (snapshot.qualification);
        stopwatchResultCell.lapVisible = snapshot.lapVisible ? 1 : 0;

        digitPresentationResultCell.sourceGeneration =
            snapshot.digitFrame.sourceSnapshotSequence;
        digitPresentationResultCell.digest = snapshot.digitDigest;
        for (uint8_t index = 0; index < 4; ++index)
        {
            digitPresentationResultCell.glyphs[index] =
                static_cast<uint8_t> (snapshot.digitFrame.glyphs[index]);
        }
        digitPresentationResultCell.decimalMask = snapshot.digitFrame.decimalMask;
        digitPresentationResultCell.refreshDisposition =
            static_cast<uint8_t> (result.presentationDisposition);
        digitPresentationResultCell.transactionPresent =
            result.digitTransactionPresent ? 1 : 0;

        matrixPresentationResultCell.sourceGeneration =
            snapshot.matrixFrame.sourceSnapshotSequence;
        matrixPresentationResultCell.digest = snapshot.matrixDigest;
        for (uint8_t row = 0; row < 8; ++row)
        {
            matrixPresentationResultCell.rows[row] = snapshot.matrixFrame.rows[row];
        }
        matrixPresentationResultCell.progressStep = snapshot.selfTestStage;
        matrixPresentationResultCell.transportDisposition =
            static_cast<uint8_t> (result.presentationDisposition);
        matrixPresentationResultCell.commandPresent =
            result.matrixCommandPresent ? 1 : 0;

        const uint8_t disagreement =
            static_cast<uint8_t> (adk::TimingDeskPresentationDisposition::Disagreement);
        if (snapshot.presentationDisposition ==
                adk::TimingDeskPresentationDisposition::Disagreement ||
            disagreementResultCell.disposition != disagreement)
        {
            disagreementResultCell.expectedGeneration = snapshot.presentationGeneration;
            disagreementResultCell.digitGeneration =
                snapshot.digitAccepted ? snapshot.presentationGeneration : 0;
            disagreementResultCell.matrixGeneration =
                snapshot.matrixAccepted ? snapshot.presentationGeneration : 0;
            disagreementResultCell.disposition =
                static_cast<uint8_t> (snapshot.presentationDisposition);
            disagreementResultCell.faultOwner =
                static_cast<uint8_t> (snapshot.faultOwner);
            disagreementResultCell.digitBlankRequested =
                result.digitBlankRequested ? 1 : 0;
            disagreementResultCell.matrixBlankRequested =
                result.matrixBlankRequested ? 1 : 0;
        }

        selfTestResultCell.stage          = snapshot.selfTestStage;
        selfTestResultCell.digitAccepted  = snapshot.digitAccepted ? 1 : 0;
        selfTestResultCell.matrixAccepted = snapshot.matrixAccepted ? 1 : 0;
        selfTestResultCell.timePresentationAllowed =
            snapshot.qualification == adk::TimingDeskQualification::Ready ? 1 : 0;
    }

} // namespace
