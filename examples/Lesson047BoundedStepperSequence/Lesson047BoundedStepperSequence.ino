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
         true}};
    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::BoundedStepperSequence sequence (sequenceConfig);

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
    volatile uint32_t completedStepCells[replayFrameCount];
    volatile uint8_t  driveIntentCells[replayFrameCount];
    volatile uint8_t  hasDeadlineCells[replayFrameCount];
    volatile uint32_t nextStepAtCells[replayFrameCount];
    volatile uint8_t  sequenceStatusCells[replayFrameCount];

    void        configureReplayResults ();
    void        replayCopiedCommand (uint8_t index);
    adk::Status decideSequence (const ReplayFrame& frame);
    void        presentDriveIntent (uint8_t index, adk::Status operationStatus);

} // namespace

void setup ()
{
    configureReplayResults ();

    if (!sequence.initialize ().ok ())
    {
        return;
    }

    for (uint8_t index = 0; index < replayFrameCount; ++index)
    {
        replayCopiedCommand (index);
    }
}

void loop ()
{
}

namespace {

    void configureReplayResults ()
    {
        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            commandIdCells[index]          = 0xffffffffUL;
            issuedAtCells[index]           = 0xffffffffUL;
            requestedDirectionCells[index] = 0xff;
            requestedStepCells[index]      = 0xffffffffUL;
            requestedIntervalCells[index]  = 0xffffffffUL;
            cancelCells[index]             = 0xff;
            commandStatusCells[index]      = 0xff;
            operationStatusCells[index]    = 0xff;
            phaseCells[index]              = 0xff;
            dispositionCells[index]        = 0xff;
            driveDirectionCells[index]     = 0xff;
            logicalPositionCells[index]    = -2147483647L - 1L;
            completedStepCells[index]      = 0xffffffffUL;
            driveIntentCells[index]        = 0xff;
            hasDeadlineCells[index]        = 0xff;
            nextStepAtCells[index]         = 0xffffffffUL;
            sequenceStatusCells[index]     = 0xff;
        }
    }

    void replayCopiedCommand (uint8_t index)
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

        const adk::Status operationStatus = decideSequence (frame);

        presentDriveIntent (index, operationStatus);
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

        return sequence.commit (candidate);
    }

    void presentDriveIntent (uint8_t index, adk::Status operationStatus)
    {
        const adk::StepperSequenceSnapshot snapshot = sequence.snapshot ();

        operationStatusCells[index] = static_cast<uint8_t> (operationStatus.error ());
        phaseCells[index]           = static_cast<uint8_t> (snapshot.phase);
        dispositionCells[index]     = static_cast<uint8_t> (snapshot.disposition);
        driveDirectionCells[index]  = static_cast<uint8_t> (snapshot.direction);
        logicalPositionCells[index] = snapshot.logicalPosition;
        completedStepCells[index]   = snapshot.completedSteps;
        driveIntentCells[index]     = snapshot.coilIntent;
        hasDeadlineCells[index]     = snapshot.hasDeadline ? 1 : 0;
        nextStepAtCells[index]      = snapshot.nextStepAt.milliseconds ();
        sequenceStatusCells[index]  = static_cast<uint8_t> (snapshot.status.error ());
    }

} // namespace
