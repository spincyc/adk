// E0 logical-step fixture. This sketch replays copied commands and stores
// four-bit drive intent in memory. It owns no pin, endpoint, timer, interrupt,
// supply, driver, motor, stop switch, or physical-position evidence.
#include <Adk.h>

namespace {

    struct ReplayFrame
    {
        adk::TimePoint      policyTime;
        adk::StepperCommand command;
        bool                independentStop;
    };

    const adk::StepperSequenceConfig sequenceConfig = {
        adk::Duration (10), adk::Duration (40), adk::Duration (100), -8, 8, false};

    const ReplayFrame replayFrames[] = {
        {adk::TimePoint (0),
         {1, adk::TimePoint (0), adk::StepDirection::Forward, 4, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (10),
         {1, adk::TimePoint (0), adk::StepDirection::Forward, 4, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (20),
         {1, adk::TimePoint (0), adk::StepDirection::Forward, 4, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (20),
         {2, adk::TimePoint (20), adk::StepDirection::Reverse, 2, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (30),
         {2, adk::TimePoint (20), adk::StepDirection::Reverse, 2, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (35),
         {3, adk::TimePoint (35), adk::StepDirection::Stopped, 0, adk::Duration (10),
          true, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (40),
         {4, adk::TimePoint (40), adk::StepDirection::Forward, 2, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (50),
         {4, adk::TimePoint (40), adk::StepDirection::Forward, 2, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (60),
         {4, adk::TimePoint (40), adk::StepDirection::Forward, 2, adk::Duration (10),
          false, adk::StatusCode::Ok},
         false},
        {adk::TimePoint (65),
         {5, adk::TimePoint (65), adk::StepDirection::Stopped, 0, adk::Duration (10),
          false, adk::StatusCode::Ok},
         true},
        {adk::TimePoint (70),
         {6, adk::TimePoint (70), adk::StepDirection::Forward, 1, adk::Duration (10),
          false, adk::StatusCode::HardwareFailure},
         false}};
    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    const uint8_t expectedDriveIntentPattern[] = {0x00, 0x01, 0x03, 0x03, 0x01, 0x00,
                                                  0x00, 0x01, 0x00, 0x00, 0x00};
    const adk::StepSequencePhase expectedPhasePattern[] = {
        adk::StepSequencePhase::Moving,   adk::StepSequencePhase::Moving,
        adk::StepSequencePhase::Moving,   adk::StepSequencePhase::Moving,
        adk::StepSequencePhase::Moving,   adk::StepSequencePhase::Cancelled,
        adk::StepSequencePhase::Moving,   adk::StepSequencePhase::Moving,
        adk::StepSequencePhase::Complete, adk::StepSequencePhase::Cancelled,
        adk::StepSequencePhase::Fault};
    const int32_t expectedLogicalPositionPattern[] = {0, 1, 2, 2, 1, 1, 1, 2, 3, 3, 3};
    const adk::StatusCode expectedOperationStatusPattern[] = {
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::Ok,
        adk::StatusCode::HardwareFailure};

    adk::BoundedStepperSequence sequence (sequenceConfig);

    volatile uint8_t  fixtureValidationStatusCell;
    volatile uint8_t  initializationStatusCell;
    volatile uint8_t  startStatusCell;
    volatile uint8_t  replayActiveCell;
    volatile uint8_t  completedReplayFramesCell;
    volatile uint32_t commandIdCells[replayFrameCount];
    volatile uint32_t issuedAtCells[replayFrameCount];
    volatile uint8_t  requestedDirectionCells[replayFrameCount];
    volatile uint32_t requestedStepCells[replayFrameCount];
    volatile uint32_t requestedIntervalCells[replayFrameCount];
    volatile uint8_t  cancelCells[replayFrameCount];
    volatile uint8_t  commandStatusCells[replayFrameCount];
    volatile uint8_t  operationStatusCells[replayFrameCount];
    volatile uint8_t  phaseCells[replayFrameCount];
    volatile uint8_t  dispositionCells[replayFrameCount];
    volatile uint8_t  driveDirectionCells[replayFrameCount];
    volatile int32_t  logicalPositionCells[replayFrameCount];
    volatile uint32_t requestedStepsCells[replayFrameCount];
    volatile uint32_t completedStepCells[replayFrameCount];
    volatile uint8_t  driveIntentCells[replayFrameCount];
    volatile uint8_t  hasDeadlineCells[replayFrameCount];
    volatile uint32_t phaseSinceCells[replayFrameCount];
    volatile uint32_t nextStepAtCells[replayFrameCount];
    volatile uint8_t  sequenceStatusCells[replayFrameCount];
    volatile uint8_t  expectedDriveIntentCells[replayFrameCount];
    volatile uint8_t  expectedPhaseCells[replayFrameCount];
    volatile int32_t  expectedLogicalPositionCells[replayFrameCount];
    volatile uint8_t  predictionPassCells[replayFrameCount];

    uint8_t replayIndex;
    bool    replayStarted;
    // clang-format off
    adk::Status        validateReplayFixture  ();
    void               configureReplayResults ();
    adk::Status        startReplay            ();
    const ReplayFrame& observeCopiedCommand   (uint8_t index);
    adk::Status        decideSequence         (const ReplayFrame& frame);
    void               presentDriveIntent     (uint8_t index,
                                               adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    configureReplayResults ();

    const adk::Status fixtureStatus = validateReplayFixture ();

    fixtureValidationStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    const adk::Status initializationStatus = sequence.initialize ();

    initializationStatusCell = static_cast<uint8_t> (initializationStatus.error ());
    const adk::Status startStatus =
        initializationStatus.ok () ? startReplay () : initializationStatus;
    startStatusCell = static_cast<uint8_t> (startStatus.error ());
}

void loop ()
{
    if (!replayStarted)
    {
        return;
    }

    const ReplayFrame& frame = observeCopiedCommand (replayIndex);

    const adk::Status operationStatus = decideSequence (frame);

    presentDriveIntent (replayIndex, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;

    if (replayIndex == replayFrameCount)
    {
        replayStarted    = false;
        replayActiveCell = 0;
    }
}

namespace {

    adk::Status validateReplayFixture ()
    {
        const uint8_t drivePredictionCount = sizeof (expectedDriveIntentPattern) /
                                             sizeof (expectedDriveIntentPattern[0]);
        const uint8_t phasePredictionCount =
            sizeof (expectedPhasePattern) / sizeof (expectedPhasePattern[0]);
        const uint8_t positionPredictionCount =
            sizeof (expectedLogicalPositionPattern) /
            sizeof (expectedLogicalPositionPattern[0]);
        const uint8_t statusPredictionCount =
            sizeof (expectedOperationStatusPattern) /
            sizeof (expectedOperationStatusPattern[0]);

        if (drivePredictionCount != replayFrameCount ||
            phasePredictionCount != replayFrameCount ||
            positionPredictionCount != replayFrameCount ||
            statusPredictionCount != replayFrameCount)
        {
            return adk::StatusCode::InvalidConfiguration;
        }

        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            if (replayFrames[index].command.commandId == 0 ||
                expectedDriveIntentPattern[index] > 0x0f ||
                expectedLogicalPositionPattern[index] <
                    sequenceConfig.minimumLogicalPosition ||
                expectedLogicalPositionPattern[index] >
                    sequenceConfig.maximumLogicalPosition)
            {
                return adk::StatusCode::InvalidConfiguration;
            }
        }

        return adk::StatusCode::Ok;
    }

    void configureReplayResults ()
    {
        fixtureValidationStatusCell =
            static_cast<uint8_t> (adk::StatusCode::NotInitialized);
        initializationStatusCell =
            static_cast<uint8_t> (adk::StatusCode::NotInitialized);
        startStatusCell  = static_cast<uint8_t> (adk::StatusCode::NotInitialized);
        replayActiveCell = 0;
        completedReplayFramesCell = 0;
        replayIndex               = 0;
        replayStarted             = false;

        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            commandIdCells[index]           = 0xffffffffUL;
            issuedAtCells[index]            = 0xffffffffUL;
            requestedDirectionCells[index]  = 0xff;
            requestedStepCells[index]       = 0xffffffffUL;
            requestedIntervalCells[index]   = 0xffffffffUL;
            cancelCells[index]              = 0xff;
            commandStatusCells[index]       = 0xff;
            operationStatusCells[index]     = 0xff;
            phaseCells[index]               = 0xff;
            dispositionCells[index]         = 0xff;
            driveDirectionCells[index]      = 0xff;
            logicalPositionCells[index]     = -2147483647L - 1L;
            requestedStepsCells[index]      = 0xffffffffUL;
            completedStepCells[index]       = 0xffffffffUL;
            driveIntentCells[index]         = 0xff;
            hasDeadlineCells[index]         = 0xff;
            phaseSinceCells[index]          = 0xffffffffUL;
            nextStepAtCells[index]          = 0xffffffffUL;
            sequenceStatusCells[index]      = 0xff;
            expectedDriveIntentCells[index] = expectedDriveIntentPattern[index];
            expectedPhaseCells[index] =
                static_cast<uint8_t> (expectedPhasePattern[index]);
            expectedLogicalPositionCells[index] = expectedLogicalPositionPattern[index];
            predictionPassCells[index]          = 0xff;
        }
    }

    adk::Status startReplay ()
    {
        replayIndex               = 0;
        completedReplayFramesCell = 0;
        replayStarted             = true;
        replayActiveCell          = 1;
        return adk::StatusCode::Ok;
    }

    const ReplayFrame& observeCopiedCommand (uint8_t index)
    {
        const ReplayFrame& frame = replayFrames[index];

        commandIdCells[index]          = frame.command.commandId;
        issuedAtCells[index]           = frame.command.issuedAt.milliseconds ();
        requestedDirectionCells[index] = static_cast<uint8_t> (frame.command.direction);
        requestedStepCells[index]      = frame.command.stepCount;
        requestedIntervalCells[index]  = frame.command.stepInterval.milliseconds ();
        cancelCells[index]             = frame.command.cancel ? 1 : 0;
        commandStatusCells[index] =
            static_cast<uint8_t> (frame.command.status.error ());

        return frame;
    }

    adk::Status decideSequence (const ReplayFrame& frame)
    {
        if (frame.independentStop)
        {
            return sequence.stop (frame.policyTime);
        }

        adk::StepperSequencePreview candidate;
        const adk::Status           previewStatus =
            sequence.preview (frame.policyTime, frame.command, candidate);
        if (!sequence.canCommit (candidate))
        {
            return previewStatus;
        }

        const adk::Status commitStatus = sequence.commit (candidate);

        return commitStatus.ok () ? previewStatus : commitStatus;
    }

    void presentDriveIntent (uint8_t index, adk::Status operationStatus)
    {
        const adk::StepperSequenceSnapshot snapshot = sequence.snapshot ();

        operationStatusCells[index] = static_cast<uint8_t> (operationStatus.error ());
        phaseCells[index]           = static_cast<uint8_t> (snapshot.phase);
        dispositionCells[index]     = static_cast<uint8_t> (snapshot.disposition);
        driveDirectionCells[index]  = static_cast<uint8_t> (snapshot.direction);
        logicalPositionCells[index] = snapshot.logicalPosition;
        requestedStepsCells[index]  = snapshot.requestedSteps;
        completedStepCells[index]   = snapshot.completedSteps;
        driveIntentCells[index]     = snapshot.coilIntent;
        hasDeadlineCells[index]     = snapshot.hasDeadline ? 1 : 0;
        phaseSinceCells[index]      = snapshot.phaseSince.milliseconds ();

        nextStepAtCells[index] = snapshot.nextStepAt.milliseconds ();

        sequenceStatusCells[index] = static_cast<uint8_t> (snapshot.status.error ());
        predictionPassCells[index] =
            snapshot.coilIntent == expectedDriveIntentPattern[index] &&
                    snapshot.phase == expectedPhasePattern[index] &&
                    snapshot.logicalPosition == expectedLogicalPositionPattern[index] &&
                    operationStatus.error () == expectedOperationStatusPattern[index]
                ? 1
                : 0;
    }

} // namespace
