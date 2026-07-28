// E0 orientation fixture. This sketch replays only synthetic values and
// copies pure light/tone intents into memory result cells. It owns no sensor,
// bus, endpoint, live clock, or powered circuit.
#include <Adk.h>
#include <orientation_presentation.h>

namespace {

    struct ReplayFrame
    {
        int32_t                    rightMicroG;
        int32_t                    forwardMicroG;
        int32_t                    upMicroG;
        int32_t                    angularRateMilliDegreesPerSecond;
        adk::InertialSampleQuality quality;
        adk::StatusCode            status;
        uint16_t                   sensitivityPermille;
        bool                       diagnosticPhase;
    };

    const adk::InertialSource syntheticSource = {
        adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel::Synthetic,
        7,
        3,
        5,
        2000000,
        250000000};

    const adk::OrientationConfig orientationConfig = {{adk::SignedAxis::PositiveX,
                                                       adk::SignedAxis::PositiveY,
                                                       adk::SignedAxis::PositiveZ},
                                                      900000,
                                                      1100000,
                                                      1000,
                                                      5000,
                                                      60000};

    const adk::BalancePresentationConfig presentationConfig = {{0, 1000, 0, false},
                                                               {1000, 0, 0, false},
                                                               {0, 1000, 0, false},
                                                               {0, 0, 1000, false},
                                                               {1000, 1000, 0, false},
                                                               {0, 0, 300, false},
                                                               {0, 0, 700, false},
                                                               {1000, 500, 0, true},
                                                               {1000, 0, 1000, true},
                                                               60000,
                                                               100,
                                                               1000,
                                                               1000,
                                                               20};

    const adk::BalancePresentation selfTestIntent = {adk::BalanceDirection::None,
                                                     {321, 654, 987, false},
                                                     {true, 777, 12},
                                                     adk::StatusCode::Ok};

    adk::InertialObservation
    copiedObservation (int32_t rightMicroG, int32_t forwardMicroG, int32_t upMicroG,
                       int32_t                    angularRateMilliDegreesPerSecond,
                       adk::InertialSampleQuality quality, adk::Status status)
    {
        const adk::InertialSample sample = {syntheticSource,
                                            {rightMicroG, forwardMicroG, upMicroG},
                                            {0, 0, angularRateMilliDegreesPerSecond},
                                            adk::TimePoint (100),
                                            1,
                                            true,
                                            adk::InertialSaturation::None,
                                            status};

        return {sample, quality, true,  adk::Duration (0), adk::Duration (20),
                1,      0,       status};
    }

    const ReplayFrame replayFrames[] = {
        {0, 0, 1000000, 0, adk::InertialSampleQuality::Current, adk::StatusCode::Ok,
         1000, false},
        {0, 500000, 866025, 0, adk::InertialSampleQuality::Current, adk::StatusCode::Ok,
         1000, false},
        {0, -500000, 866025, 0, adk::InertialSampleQuality::Current,
         adk::StatusCode::Ok, 750, false},
        {-500000, 0, 866025, 0, adk::InertialSampleQuality::Current,
         adk::StatusCode::Ok, 500, false},
        {500000, 0, 866025, 0, adk::InertialSampleQuality::Current, adk::StatusCode::Ok,
         250, false},
        {300000, 600000, 741620, 0, adk::InertialSampleQuality::Current,
         adk::StatusCode::Ok, 1000, false},
        {0, 0, 1000000, 1001, adk::InertialSampleQuality::Current, adk::StatusCode::Ok,
         1000, true},
        {0, 900000, 435890, 0, adk::InertialSampleQuality::Current, adk::StatusCode::Ok,
         1000, false},
        {0, 0, 1000000, 0, adk::InertialSampleQuality::Stale, adk::StatusCode::Ok, 1000,
         false}};
    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);
    constexpr uint8_t resultCellCount = replayFrameCount + 1;

    adk::OrientationPolicy         orientationPolicy  (orientationConfig);
    adk::BalancePresentationPolicy presentationPolicy (presentationConfig);

    volatile int32_t  pitchCells[resultCellCount];
    volatile int32_t  rollCells[resultCellCount];
    volatile uint8_t  orientationQualityCells[resultCellCount];
    volatile uint8_t  orientationStatusCells[resultCellCount];
    volatile uint8_t  directionCells[resultCellCount];
    volatile uint16_t redCells[resultCellCount];
    volatile uint16_t greenCells[resultCellCount];
    volatile uint16_t blueCells[resultCellCount];
    volatile uint8_t  lightFaultCells[resultCellCount];
    volatile uint8_t  toneEnabledCells[resultCellCount];
    volatile uint16_t toneFrequencyCells[resultCellCount];
    volatile uint16_t toneDurationCells[resultCellCount];
    volatile uint8_t  presentationStatusCells[resultCellCount];

    void configureReplayResults ();
    void publishSelfTest        ();
    void observePose            (uint8_t index);
    void presentIntent          (
        uint8_t resultIndex, const adk::OrientationEstimate& estimate,
        const adk::BalancePresentation& presentation);

} // namespace

void setup ()
{
    const adk::Status orientationAcquired  = orientationPolicy.initialize  ();
    const adk::Status presentationAcquired = presentationPolicy.initialize ();

    configureReplayResults ();
    publishSelfTest        ();

    if (!orientationAcquired.ok () || !presentationAcquired.ok ())
    {
        return;
    }

    for (uint8_t index = 0; index < replayFrameCount; ++index)
    {
        observePose (index);
    }
}

void loop ()
{
}

namespace {

    void configureReplayResults ()
    {
        for (uint8_t index = 0; index < resultCellCount; ++index)
        {
            pitchCells[index]              = -2147483647L - 1L;
            rollCells[index]               = -2147483647L - 1L;
            orientationQualityCells[index] = 0xff;
            orientationStatusCells[index]  = 0xff;
            directionCells[index]          = 0xff;
            redCells[index]                = 0xffff;
            greenCells[index]              = 0xffff;
            blueCells[index]               = 0xffff;
            lightFaultCells[index]         = 0xff;
            toneEnabledCells[index]        = 0xff;
            toneFrequencyCells[index]      = 0xffff;
            toneDurationCells[index]       = 0xffff;
            presentationStatusCells[index] = 0xff;
        }
    }

    void publishSelfTest ()
    {
        const adk::OrientationEstimate selfTestEstimate = {
            0, 0, adk::OrientationQuality::Invalid, adk::StatusCode::Ok};
        presentIntent (0, selfTestEstimate, selfTestIntent);
    }

    void observePose (uint8_t index)
    {
        const ReplayFrame&             frame       = replayFrames[index];
        const adk::InertialObservation observation = copiedObservation (
            frame.rightMicroG, frame.forwardMicroG, frame.upMicroG,
            frame.angularRateMilliDegreesPerSecond, frame.quality, frame.status);

        orientationPolicy.update (observation);
        const adk::OrientationEstimate estimate =
            orientationPolicy.snapshot ();
        presentationPolicy.update (estimate, frame.sensitivityPermille,
                                   frame.diagnosticPhase);
        presentIntent (index + 1, estimate, presentationPolicy.snapshot ());
    }

    void presentIntent (uint8_t resultIndex, const adk::OrientationEstimate& estimate,
                        const adk::BalancePresentation& presentation)
    {
        pitchCells[resultIndex]              = estimate.pitchMilliDegrees;
        rollCells[resultIndex]               = estimate.rollMilliDegrees;
        orientationQualityCells[resultIndex] = static_cast<uint8_t> (estimate.quality);
        orientationStatusCells[resultIndex] =
            static_cast<uint8_t> (estimate.status.error ());
        directionCells[resultIndex]     = static_cast<uint8_t> (presentation.direction);
        redCells[resultIndex]           = presentation.light.redPermille;
        greenCells[resultIndex]         = presentation.light.greenPermille;
        blueCells[resultIndex]          = presentation.light.bluePermille;
        lightFaultCells[resultIndex]    = presentation.light.fault ? 1 : 0;
        toneEnabledCells[resultIndex]   = presentation.tone.enabled ? 1 : 0;
        toneFrequencyCells[resultIndex] = presentation.tone.frequencyHertz;
        toneDurationCells[resultIndex]  = presentation.tone.durationMilliseconds;
        presentationStatusCells[resultIndex] =
            static_cast<uint8_t> (presentation.status.error ());
    }

} // namespace
