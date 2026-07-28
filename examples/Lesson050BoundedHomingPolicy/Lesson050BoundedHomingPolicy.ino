// E0 bounded-homing fixture. This sketch replays copied synthetic home and
// stop evidence and stores semantic intent in memory. It owns no endpoint,
// pin, timer, interrupt, sensor, driver, motor, or physical-position evidence.
#include <Adk.h>
#include <bounded_homing_policy.h>

namespace {

    struct ReplayFrame
    {
        uint32_t           now;
        uint32_t           sequence;
        bool               homeActive;
        bool               stopActive;
        adk::HomingCommand command;
        adk::HomingPhase   expectedPhase;
        bool               expectedPositionKnown;
        int8_t             expectedStepDirection;
        bool               expectedStopIntent;
    };

    struct HomingResultCell
    {
        uint32_t acceptedFrameSequence;
        uint32_t homeEpoch;
        int32_t  logicalPosition;
        uint16_t homingSteps;
        int8_t   requestedStepDirection;
        uint8_t  phase;
        uint8_t  fault;
        uint8_t  positionKnown;
        uint8_t  stepRequested;
        uint8_t  stopIntent;
        uint8_t  operationStatus;
        uint8_t  policyStatus;
        uint8_t  predictionPass;
    };

    const adk::CarouselSource homeSource = {adk::CarouselSourceKind::SyntheticHome, 1,
                                            1};
    const adk::CarouselSource stopSource = {adk::CarouselSourceKind::SyntheticStop, 1,
                                            1};

    const adk::BoundedHomingConfig homingConfig = {-4,
                                                   4,
                                                   0,
                                                   1,
                                                   2,
                                                   4,
                                                   adk::Duration (50),
                                                   adk::Duration (100),
                                                   adk::Duration (10),
                                                   adk::Duration (20),
                                                   adk::Duration (5)};

    const ReplayFrame replayFrames[] = {
        {0,
         1,
         false,
         false,
         {true, false, 0},
         adk::HomingPhase::SeekingHome,
         false,
         1,
         false},
        {10,
         2,
         false,
         false,
         {false, false, 0},
         adk::HomingPhase::SeekingHome,
         false,
         1,
         false},
        {20, 3, true, false, {false, false, 0}, adk::HomingPhase::Homed, true, 0, true},
        {30,
         4,
         true,
         false,
         {false, true, 2},
         adk::HomingPhase::Moving,
         true,
         1,
         false},
        {40,
         5,
         true,
         false,
         {false, false, 0},
         adk::HomingPhase::Homed,
         true,
         1,
         false},
        {50,
         6,
         true,
         false,
         {false, false, 0},
         adk::HomingPhase::Homed,
         true,
         0,
         false},
        {60,
         7,
         true,
         true,
         {false, true, -2},
         adk::HomingPhase::Stopped,
         true,
         0,
         true},
        {70,
         8,
         false,
         false,
         {true, false, 0},
         adk::HomingPhase::SeekingHome,
         false,
         1,
         false},
        {80,
         9,
         true,
         false,
         {false, false, 0},
         adk::HomingPhase::Homed,
         true,
         0,
         true}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::BoundedHomingPolicy homingPolicy (homingConfig);

    volatile HomingResultCell resultCells[replayFrameCount];
    volatile uint8_t          fixtureStatusCell;
    volatile uint8_t          initializeStatusCell;
    volatile uint8_t          replayActiveCell;
    volatile uint8_t          completedReplayFramesCell;
    volatile int32_t          minimumExcursionCell;
    volatile int32_t          maximumExcursionCell;

    uint8_t replayIndex;
    bool    replayActive;

    // clang-format off
    adk::Status       configureReplay         ();
    adk::HomingInput  observeCopiedEvidence   (const ReplayFrame& frame);
    adk::Status       decideHomingIntent      (const ReplayFrame& frame,
                                               const adk::HomingInput& input);
    void              presentHomingIntent     (uint8_t index,
                                               const ReplayFrame& frame,
                                               adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = configureReplay ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    const adk::Status initializationStatus = homingPolicy.initialize ();

    initializeStatusCell = static_cast<uint8_t> (initializationStatus.error ());

    if (!initializationStatus.ok ())
    {
        return;
    }

    const adk::HomingExcursionBounds bounds = homingPolicy.excursionBounds ();
    minimumExcursionCell                    = bounds.minimum;
    maximumExcursionCell                    = bounds.maximum;
    replayIndex                             = 0;
    replayActive                            = true;
    replayActiveCell                        = 1;
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame&     frame = replayFrames[replayIndex];
    const adk::HomingInput input = observeCopiedEvidence (frame);

    const adk::Status operationStatus = decideHomingIntent (frame, input);

    presentHomingIntent (replayIndex, frame, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;
    replayActive              = replayIndex < replayFrameCount;
    replayActiveCell          = replayActive ? 1 : 0;
}

namespace {

    adk::Status configureReplay ()
    {
        fixtureStatusCell         = 0xff;
        initializeStatusCell      = 0xff;
        replayActiveCell          = 0;
        completedReplayFramesCell = 0;
        minimumExcursionCell      = 0;
        maximumExcursionCell      = 0;
        replayIndex               = 0;
        replayActive              = false;

        if (replayFrameCount == 0)
        {
            return adk::StatusCode::InvalidConfiguration;
        }

        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            if (replayFrames[index].sequence == 0)
            {
                return adk::StatusCode::InvalidConfiguration;
            }
        }

        return adk::StatusCode::Ok;
    }

    adk::HomingInput observeCopiedEvidence (const ReplayFrame& frame)
    {
        const adk::TimePoint            observedAt (frame.now);
        const adk::CopiedBinaryEvidence home = {
            homeSource, observedAt, frame.sequence,     frame.homeActive,
            true,       1,          adk::StatusCode::Ok};
        const adk::CopiedBinaryEvidence stop = {
            stopSource, observedAt, frame.sequence,     frame.stopActive,
            true,       1,          adk::StatusCode::Ok};

        return {observedAt, frame.sequence, home, stop};
    }

    adk::Status decideHomingIntent (const ReplayFrame&      frame,
                                    const adk::HomingInput& input)
    {
        adk::HomingPreview candidate;
        const adk::Status  previewStatus = homingPolicy.preview (
            adk::TimePoint (frame.now), input, frame.command, candidate);
        if (!previewStatus.ok ())
        {
            return previewStatus;
        }
        if (!homingPolicy.canCommit (candidate))
        {
            return adk::StatusCode::InternalInvariant;
        }
        return homingPolicy.commit (candidate);
    }

    void presentHomingIntent (uint8_t index, const ReplayFrame& frame,
                              adk::Status operationStatus)
    {
        const adk::HomingSnapshot  snapshot = homingPolicy.snapshot ();
        volatile HomingResultCell& result   = resultCells[index];

        result.acceptedFrameSequence  = snapshot.acceptedFrameSequence;
        result.homeEpoch              = snapshot.homeEpoch;
        result.logicalPosition        = snapshot.logicalPosition;
        result.homingSteps            = snapshot.homingSteps;
        result.requestedStepDirection = snapshot.requestedStepDirection;
        result.phase                  = static_cast<uint8_t> (snapshot.phase);
        result.fault                  = static_cast<uint8_t> (snapshot.fault);
        result.positionKnown          = snapshot.positionKnown ? 1 : 0;
        result.stepRequested          = snapshot.stepRequested ? 1 : 0;
        result.stopIntent             = snapshot.stopIntent ? 1 : 0;
        result.operationStatus        = static_cast<uint8_t> (operationStatus.error ());
        result.policyStatus           = static_cast<uint8_t> (snapshot.status.error ());
        result.predictionPass =
            operationStatus.ok () && snapshot.phase == frame.expectedPhase &&
                    snapshot.positionKnown == frame.expectedPositionKnown &&
                    snapshot.requestedStepDirection == frame.expectedStepDirection &&
                    snapshot.stopIntent == frame.expectedStopIntent
                ? 1
                : 0;
    }

} // namespace
