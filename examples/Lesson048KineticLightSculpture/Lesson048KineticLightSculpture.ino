// E0 kinetic-sculpture fixture. This sketch replays copied observations and
// stores intent in memory. It owns no pin, endpoint, timer, interrupt, supply,
// input device, light, driver, motor, stop switch, or physical-position proof.
#include <Adk.h>

namespace {

    enum struct ReplayControl : uint8_t
    {
        Update,
        Restart
    };

    struct ReplayFrame
    {
        ReplayControl       control;
        adk::SculptureInput input;
    };

    struct EvidenceCell
    {
        uint8_t operationStatus;
        uint8_t phase;
        uint8_t projectStatus;
        uint8_t interactionStatus;
        uint8_t motionStatus;
        uint8_t stopStatus;
        uint8_t pendingAuthorization;
        uint8_t terminalDisposition;
        uint8_t coilIntent;
        uint8_t lightIntent;
        uint8_t mirrorExact;
        uint8_t predictionPass;
    };

    const adk::InteractionSource touchSource = {
        adk::InteractionSourceKind::SyntheticFixture, 1, 1};
    const adk::InteractionSource directionalSource = {
        adk::InteractionSourceKind::SyntheticFixture, 2, 1};
    const adk::InteractionSource stopSource = {
        adk::InteractionSourceKind::SyntheticFixture, 3, 1};

    const adk::InteractionIntentConfig interactionConfig = {
        {adk::Level::High, adk::Duration (8), adk::Duration (8), adk::Duration (80),
         adk::Duration (2000)},
        adk::Duration (100),
        adk::Duration (100),
        300,
        200};
    const adk::StepperSequenceConfig sequenceConfig = {
        adk::Duration (60), adk::Duration (250), adk::Duration (2000), -256, 256,
        false};

#define SCULPTURE_INPUT(now, frame, touch_sequence, direction_sequence, stop_sequence, \
                        touch_level, x, direction_status, stop_active)                 \
    {adk::TimePoint (now),                                                             \
     frame,                                                                            \
     stopSource,                                                                       \
     adk::TimePoint (now),                                                             \
     stop_sequence,                                                                    \
     stop_active,                                                                      \
     adk::ContactQuality::Valid,                                                       \
     adk::StatusCode::Ok,                                                              \
     touchSource,                                                                      \
     touch_sequence,                                                                   \
     {adk::TimePoint (now), touch_level, adk::StatusCode::Ok},                         \
     {directionalSource,                                                              \
      adk::TimePoint (now), direction_sequence, x, 0, false,                          \
      direction_status},                                                               \
     adk::StatusCode::Ok}

    const ReplayFrame replayFrames[] = {
        {ReplayControl::Update, SCULPTURE_INPUT (0, 1, 1, 1, 1, adk::Level::Low, 1000,
                                                 adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (1, 2, 2, 2, 2, adk::Level::High, 1000,
                                                 adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (9, 3, 3, 3, 3, adk::Level::High, 1000,
                                                 adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (10, 4, 4, 4, 4, adk::Level::High, 1000,
                                                 adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (90, 5, 5, 5, 5, adk::Level::High, 1000,
                                                 adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (91, 6, 6, 6, 6, adk::Level::High, 1000,
                                                 adk::StatusCode::Ok, true)},
        {ReplayControl::Restart, SCULPTURE_INPUT (92, 7, 7, 7, 7, adk::Level::Low, 1000,
                                                  adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (93, 8, 8, 8, 8, adk::Level::High, 1000,
                                                 adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (101, 9, 9, 9, 9, adk::Level::High,
                                                 1000, adk::StatusCode::Ok, false)},
        {ReplayControl::Update, SCULPTURE_INPUT (102, 10, 10, 10, 10, adk::Level::High,
                                                 1000, adk::StatusCode::Ok, false)},
        {ReplayControl::Update,
         SCULPTURE_INPUT (103, 11, 11, 11, 11, adk::Level::High, 1000,
                          adk::StatusCode::HardwareFailure, false)},
        {ReplayControl::Restart, SCULPTURE_INPUT (104, 12, 12, 12, 12, adk::Level::Low,
                                                  0, adk::StatusCode::Ok, false)}};
#undef SCULPTURE_INPUT

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    const adk::SculpturePhase expectedPhase[] = {
        adk::SculpturePhase::Ready,   adk::SculpturePhase::Ready,
        adk::SculpturePhase::Preview, adk::SculpturePhase::Running,
        adk::SculpturePhase::Running, adk::SculpturePhase::Stopped,
        adk::SculpturePhase::Ready,   adk::SculpturePhase::Ready,
        adk::SculpturePhase::Preview, adk::SculpturePhase::Running,
        adk::SculpturePhase::Fault,   adk::SculpturePhase::Ready};
    const adk::StatusCode expectedStatus[]  = {adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::Ok,
                                               adk::StatusCode::HardwareFailure,
                                               adk::StatusCode::Ok};
    const uint8_t         expectedPending[] = {0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0};
    const adk::AuthorizationDisposition expectedTerminal[] = {
        adk::AuthorizationDisposition::None, adk::AuthorizationDisposition::None,
        adk::AuthorizationDisposition::None, adk::AuthorizationDisposition::Accepted,
        adk::AuthorizationDisposition::None, adk::AuthorizationDisposition::None,
        adk::AuthorizationDisposition::None, adk::AuthorizationDisposition::None,
        adk::AuthorizationDisposition::None, adk::AuthorizationDisposition::Accepted,
        adk::AuthorizationDisposition::None, adk::AuthorizationDisposition::None};
    const uint8_t expectedCoilIntent[] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};

    adk::KineticLightSculpture sculpture (interactionConfig, sequenceConfig,
                                          adk::Duration (250), adk::Duration (40));

    volatile uint8_t      fixtureValidationStatusCell;
    volatile uint8_t      initializationStatusCell;
    volatile uint8_t      startStatusCell;
    volatile uint8_t      replayActiveCell;
    volatile uint8_t      completedReplayFramesCell;
    volatile EvidenceCell evidenceCells[replayFrameCount];

    uint8_t replayIndex;
    bool    replayStarted;

    adk::Status validateReplayFixture ();

    void               configureReplayResults ();

    adk::Status startReplay ();

    const ReplayFrame& observeCopiedEvidence   (uint8_t index);

    adk::Status        decideSculpture         (const ReplayFrame& frame);

    void               presentIntentEvidence  (uint8_t index,
                                               adk::Status operationStatus);

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

    const adk::Status initializationStatus = sculpture.initialize ();

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

    const ReplayFrame& frame           = observeCopiedEvidence (replayIndex);

    const adk::Status  operationStatus = decideSculpture (frame);

    presentIntentEvidence (replayIndex, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;
    if (replayIndex == replayFrameCount)
    {
        sculpture.shutdown ();
        replayStarted    = false;
        replayActiveCell = 0;
    }
}

namespace {

    adk::Status validateReplayFixture ()
    {
        if (sizeof (expectedPhase) / sizeof (expectedPhase[0]) != replayFrameCount ||
            sizeof (expectedStatus) / sizeof (expectedStatus[0]) != replayFrameCount ||
            sizeof (expectedPending) / sizeof (expectedPending[0]) !=
                replayFrameCount ||
            sizeof (expectedTerminal) / sizeof (expectedTerminal[0]) !=
                replayFrameCount ||
            sizeof (expectedCoilIntent) / sizeof (expectedCoilIntent[0]) !=
                replayFrameCount)
        {
            return adk::StatusCode::InvalidConfiguration;
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
            evidenceCells[index].operationStatus      = 0xff;
            evidenceCells[index].phase                = 0xff;
            evidenceCells[index].projectStatus        = 0xff;
            evidenceCells[index].interactionStatus    = 0xff;
            evidenceCells[index].motionStatus         = 0xff;
            evidenceCells[index].stopStatus           = 0xff;
            evidenceCells[index].pendingAuthorization = 0xff;
            evidenceCells[index].terminalDisposition  = 0xff;
            evidenceCells[index].coilIntent           = 0xff;
            evidenceCells[index].lightIntent          = 0xff;
            evidenceCells[index].mirrorExact          = 0xff;
            evidenceCells[index].predictionPass       = 0xff;
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

    const ReplayFrame& observeCopiedEvidence (uint8_t index)
    {
        return replayFrames[index];
    }

    adk::Status decideSculpture (const ReplayFrame& frame)
    {
        if (frame.control == ReplayControl::Restart)
        {
            sculpture.shutdown ();

            const adk::Status restartStatus = sculpture.initialize ();

            return restartStatus.ok () ? sculpture.update (frame.input) : restartStatus;
        }
        return sculpture.update (frame.input);
    }

    void presentIntentEvidence (uint8_t index, adk::Status operationStatus)
    {
        const adk::SculptureSnapshot        snapshot = sculpture.snapshot ();
        const adk::AuthorizationDisposition terminalDisposition =
            snapshot.hasLastTerminalAuthorization
                ? snapshot.lastTerminalAuthorization.disposition
                : adk::AuthorizationDisposition::None;
        const bool mirrorExact =
            snapshot.motion.coilIntent == snapshot.lights.shiftRegisterBits;
        const bool predictionPass =
            operationStatus.error () == expectedStatus[index] &&
            snapshot.phase == expectedPhase[index] &&
            snapshot.status.error () == expectedStatus[index] &&
            snapshot.hasPendingAuthorization == (expectedPending[index] != 0) &&
            terminalDisposition == expectedTerminal[index] &&
            snapshot.motion.coilIntent == expectedCoilIntent[index] && mirrorExact;

        evidenceCells[index].operationStatus =
            static_cast<uint8_t> (operationStatus.error ());
        evidenceCells[index].phase = static_cast<uint8_t> (snapshot.phase);
        evidenceCells[index].projectStatus =
            static_cast<uint8_t> (snapshot.status.error ());
        evidenceCells[index].interactionStatus =
            static_cast<uint8_t> (snapshot.interactionStatus.error ());
        evidenceCells[index].motionStatus =
            static_cast<uint8_t> (snapshot.motionStatus.error ());
        evidenceCells[index].stopStatus =
            static_cast<uint8_t> (snapshot.stopStatus.error ());
        evidenceCells[index].pendingAuthorization =
            snapshot.hasPendingAuthorization ? 1 : 0;
        evidenceCells[index].terminalDisposition =
            static_cast<uint8_t> (terminalDisposition);
        evidenceCells[index].coilIntent     = snapshot.motion.coilIntent;
        evidenceCells[index].lightIntent    = snapshot.lights.shiftRegisterBits;
        evidenceCells[index].mirrorExact    = mirrorExact ? 1 : 0;
        evidenceCells[index].predictionPass = predictionPass ? 1 : 0;
    }

} // namespace
