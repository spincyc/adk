// E0 interaction fixture. This sketch replays only copied values and stores
// interaction intent in memory. It owns no endpoint, pin, ADC, timer, bus,
// live clock, or powered circuit, and must not run as hardware evidence.
#include <Adk.h>
#include <interaction_intent_policy.h>

namespace {

    struct ReplayFrame
    {
        uint32_t        now;
        uint32_t        contactObservedAt;
        uint32_t        contactSequence;
        adk::Level      contactLevel;
        adk::StatusCode contactStatus;
        uint32_t        directionalObservedAt;
        uint32_t        directionalSequence;
        int16_t         xPermille;
        int16_t         yPermille;
        bool            directionalSaturated;
        adk::StatusCode directionalStatus;
    };

    struct IntentCell
    {
        uint32_t contactSequence;
        uint32_t directionalSequence;
        uint8_t  direction;
        uint16_t magnitudePermille;
        uint8_t  touchActive;
        uint8_t  touchEvent;
        uint8_t  touchReleaseEvent;
        uint8_t  directionEvent;
        uint8_t  quality;
        uint32_t contactAge;
        uint32_t directionalAge;
        uint8_t  directionalSaturated;
        uint8_t  contactQuality;
        uint8_t  contactStatus;
        uint8_t  directionalStatus;
        uint8_t  status;
        uint8_t  available;
    };

    const adk::InteractionSource contactSource = {
        adk::InteractionSourceKind::SyntheticFixture, 1, 1};
    const adk::InteractionSource directionalSource = {
        adk::InteractionSourceKind::SyntheticFixture, 2, 1};

    const adk::InteractionIntentConfig interactionConfig = {
        {adk::Level::High, adk::Duration (8), adk::Duration (8),
         adk::Duration (80), adk::Duration (2000)},
        adk::Duration (100),
        adk::Duration (100),
        300,
        200};

    const ReplayFrame replayFrames[] = {
        {0, 0, 1, adk::Level::Low, adk::StatusCode::Ok, 0, 1, 0, 0, false,
         adk::StatusCode::Ok},
        {1, 1, 2, adk::Level::High, adk::StatusCode::Ok, 1, 2, 0, 0, false,
         adk::StatusCode::Ok},
        {9, 9, 3, adk::Level::High, adk::StatusCode::Ok, 9, 3, 600, 0, false,
         adk::StatusCode::Ok},
        {10, 10, 4, adk::Level::High, adk::StatusCode::Ok, 10, 4, 600, 600, false,
         adk::StatusCode::Ok},
        {11, 11, 5, adk::Level::High, adk::StatusCode::Ok, 11, 5, 1000, 1000, true,
         adk::StatusCode::Ok},
        {12, 12, 6, adk::Level::Low, adk::StatusCode::Ok, 12, 6, 100, 100, false,
         adk::StatusCode::Ok},
        {20, 20, 7, adk::Level::Low, adk::StatusCode::Ok, 20, 7, 0, 0, false,
         adk::StatusCode::Ok},
        {21, 21, 8, adk::Level::Low, adk::StatusCode::Ok, 21, 8, 0, 0, false,
         adk::StatusCode::HardwareFailure}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::InteractionIntentPolicy interaction (interactionConfig);

    volatile IntentCell resultCells[replayFrameCount];
    volatile uint8_t    fixtureTableValidCell = 0;
    volatile uint8_t    initializeStatusCell  = 0xff;
    volatile uint8_t    updateStatusCells[replayFrameCount];

    uint8_t replayIndex  = 0;
    bool    replayActive = false;

    bool        configureReplay ();
    void        startReplay     (adk::Status acquired, bool fixturesValid);
    ReplayFrame observeEvidence ();
    adk::Status decideIntent    (const ReplayFrame& frame);
    void        actuateMirror   (uint8_t index, adk::Status updateStatus);

} // namespace

void setup ()
{
    const adk::Status acquired      = interaction.initialize ();
    const bool        fixturesValid = configureReplay ();

    startReplay (acquired, fixturesValid);
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame frame        = observeEvidence ();
    const adk::Status updateStatus = decideIntent (frame);

    actuateMirror (replayIndex, updateStatus);

    ++replayIndex;
    replayActive = replayIndex < replayFrameCount;
}

namespace {

    bool configureReplay ()
    {
        bool valid = replayFrameCount != 0;

        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            const ReplayFrame& frame = replayFrames[index];
            valid = valid && frame.contactSequence != 0 &&
                    frame.directionalSequence != 0 &&
                    frame.contactObservedAt <= frame.now &&
                    frame.directionalObservedAt <= frame.now &&
                    frame.xPermille >= -1000 && frame.xPermille <= 1000 &&
                    frame.yPermille >= -1000 && frame.yPermille <= 1000;

            resultCells[index] = {0,    0,    0xff, 0xffff, 0xff, 0xff,
                                  0xff, 0xff, 0xff, 0xffffffffUL,
                                  0xffffffffUL, 0xff, 0xff, 0xff,
                                  0xff, 0xff, 0};
            updateStatusCells[index] = 0xff;
        }

        fixtureTableValidCell = valid ? 1 : 0;
        return valid;
    }

    void startReplay (adk::Status acquired, bool fixturesValid)
    {
        initializeStatusCell = static_cast<uint8_t> (acquired.error ());
        replayIndex          = 0;
        replayActive         = acquired.ok () && fixturesValid;
    }

    ReplayFrame observeEvidence ()
    {
        return replayFrames[replayIndex];
    }

    adk::Status decideIntent (const ReplayFrame& frame)
    {
        const adk::ContactSample contact = {
            adk::TimePoint (frame.contactObservedAt), frame.contactLevel,
            frame.contactStatus};
        const adk::DirectionalEvidence directional = {
            directionalSource,
            adk::TimePoint (frame.directionalObservedAt),
            frame.directionalSequence,
            frame.xPermille,
            frame.yPermille,
            frame.directionalSaturated,
            frame.directionalStatus};

        adk::InteractionIntentPreview candidate;
        const adk::Status previewed =
            interaction.preview (adk::TimePoint (frame.now), contactSource,
                                 frame.contactSequence, contact, directional, candidate);
        if (!previewed.ok ())
        {
            return previewed;
        }
        if (!interaction.canCommit (candidate))
        {
            return adk::StatusCode::InvalidArgument;
        }

        return interaction.commit (candidate);
    }

    void actuateMirror (uint8_t index, adk::Status updateStatus)
    {
        updateStatusCells[index] = static_cast<uint8_t> (updateStatus.error ());
        if (!updateStatus.ok ())
        {
            return;
        }

        const adk::InteractionIntent intent = interaction.snapshot ();
        resultCells[index] = {
            intent.contactSequence,
            intent.directionalSequence,
            static_cast<uint8_t> (intent.direction),
            intent.magnitudePermille,
            intent.touchActive ? 1 : 0,
            intent.touchEvent ? 1 : 0,
            intent.touchReleaseEvent ? 1 : 0,
            intent.directionEvent ? 1 : 0,
            static_cast<uint8_t> (intent.quality),
            intent.contactAge.milliseconds (),
            intent.directionalAge.milliseconds (),
            intent.directionalSaturated ? 1 : 0,
            static_cast<uint8_t> (intent.contactQuality),
            static_cast<uint8_t> (intent.contactStatus.error ()),
            static_cast<uint8_t> (intent.directionalStatus.error ()),
            static_cast<uint8_t> (intent.status.error ()),
            1};
    }

} // namespace
