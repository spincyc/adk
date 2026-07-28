// E0 copied-sample fixture. This sketch uses only fixed values and memory
// result cells: it owns no sensor, bus, endpoint, live clock, or powered
// circuit. Compilation and synthetic replay are not physical acceptance for
// an MPU6050, QMI8658, or any retail inertial module.
#include <Adk.h>

namespace {

    enum struct DiagnosticCode : uint8_t
    {
        SelfTest    = 0xa5,
        Current     = 1,
        Stale       = 2,
        Saturated   = 3,
        Invalid     = 4,
        ProducerFault = 5
    };

    struct ReplayFrame
    {
        adk::TimePoint      policyTime;
        adk::InertialSample sample;
    };

    const adk::InertialSource syntheticSource = {
        adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel::Synthetic,
        7,
        3,
        5,
        2000000,
        250000000};

    const adk::InertialObservationConfig observationConfig = {
        adk::Duration (20), 1};

    const ReplayFrame replayFrames[] = {
        {adk::TimePoint (100),
         {syntheticSource,
          {0, 0, 1000000},
          {0, 0, 0},
          adk::TimePoint (100),
          1,
          true,
          adk::InertialSaturation::None,
          adk::StatusCode::Ok}},
        {adk::TimePoint (110),
         {syntheticSource,
          {10000, -10000, 999900},
          {100, -100, 50},
          adk::TimePoint (110),
          2,
          true,
          adk::InertialSaturation::None,
          adk::StatusCode::Ok}},
        {adk::TimePoint (115),
         {syntheticSource,
          {10000, -10000, 999900},
          {100, -100, 50},
          adk::TimePoint (110),
          2,
          false,
          adk::InertialSaturation::None,
          adk::StatusCode::Ok}},
        {adk::TimePoint (120),
         {syntheticSource,
          {500000, -250000, 825000},
          {0, 0, 12000},
          adk::TimePoint (120),
          3,
          true,
          adk::InertialSaturation::None,
          adk::StatusCode::Ok}},
        {adk::TimePoint (130),
         {syntheticSource,
          {2000000, 0, 0},
          {0, 0, 0},
          adk::TimePoint (130),
          4,
          true,
          adk::InertialSaturation::Acceleration,
          adk::StatusCode::Ok}},
        {adk::TimePoint (170),
         {syntheticSource,
          {0, 0, 1000000},
          {0, 0, 0},
          adk::TimePoint (140),
          5,
          true,
          adk::InertialSaturation::None,
          adk::StatusCode::Ok}},
        {adk::TimePoint (180),
         {syntheticSource,
          {0, 0, 1000000},
          {0, 0, 0},
          adk::TimePoint (180),
          6,
          true,
          adk::InertialSaturation::None,
          adk::StatusCode::HardwareFailure}}};
    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::InertialObservationPolicy observationPolicy (observationConfig);

    volatile uint8_t  diagnosticCells[replayFrameCount];
    volatile uint8_t  statusCells[replayFrameCount];
    volatile uint8_t  qualityCells[replayFrameCount];
    volatile uint8_t  sourceKindCells[replayFrameCount];
    volatile uint8_t  sourceModelCells[replayFrameCount];
    volatile uint8_t  sourceIdCells[replayFrameCount];
    volatile uint8_t  acceptedDataReadyCells[replayFrameCount];
    volatile uint8_t  latestDataReadyCells[replayFrameCount];
    volatile uint8_t  saturationCells[replayFrameCount];
    volatile uint8_t  producerStatusCells[replayFrameCount];
    volatile uint16_t configurationCells[replayFrameCount];
    volatile uint16_t calibrationCells[replayFrameCount];
    volatile uint16_t freshnessRevisionCells[replayFrameCount];
    volatile uint32_t accelerationRangeCells[replayFrameCount];
    volatile uint32_t angularRateRangeCells[replayFrameCount];
    volatile uint32_t observedAtCells[replayFrameCount];
    volatile uint32_t sequenceCells[replayFrameCount];
    volatile uint32_t ageCells[replayFrameCount];
    volatile uint32_t maximumAgeCells[replayFrameCount];
    volatile uint32_t sequenceGapCells[replayFrameCount];
    volatile int32_t  accelerationXCells[replayFrameCount];
    volatile int32_t  accelerationYCells[replayFrameCount];
    volatile int32_t  accelerationZCells[replayFrameCount];
    volatile int32_t  angularRateXCells[replayFrameCount];
    volatile int32_t  angularRateYCells[replayFrameCount];
    volatile int32_t  angularRateZCells[replayFrameCount];

    void           configureReplayResults ();
    adk::Status    observeCopiedFrame     (uint8_t index);
    DiagnosticCode decideDiagnostic       (
        const adk::InertialObservation& observation);
    void presentObservation (
        uint8_t index, const adk::InertialObservation& observation,
        DiagnosticCode diagnostic);

} // namespace

void setup ()
{
    const adk::Status acquired = observationPolicy.initialize ();

    configureReplayResults ();

    if (!acquired.ok ())
    {
        return;
    }

    for (uint8_t index = 0; index < replayFrameCount; ++index)
    {
        observeCopiedFrame (index);

        const adk::InertialObservation observation =
            observationPolicy.snapshot ();
        const DiagnosticCode diagnostic = decideDiagnostic (observation);

        presentObservation (index, observation, diagnostic);
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
            diagnosticCells[index] =
                static_cast<uint8_t> (DiagnosticCode::SelfTest);
            statusCells[index]  =
                static_cast<uint8_t> (adk::StatusCode::NotInitialized);
            qualityCells[index] =
                static_cast<uint8_t> (adk::InertialSampleQuality::Invalid);
            sourceKindCells[index]        = 0xff;
            sourceModelCells[index]       = 0xff;
            sourceIdCells[index]          = 0xff;
            acceptedDataReadyCells[index] = 0xff;
            latestDataReadyCells[index]   = 0xff;
            saturationCells[index]        = 0xff;
            producerStatusCells[index]    = 0xff;
            configurationCells[index]     = 0xffff;
            calibrationCells[index]       = 0xffff;
            freshnessRevisionCells[index] = 0xffff;
            accelerationRangeCells[index] = 0xffffffffUL;
            angularRateRangeCells[index]  = 0xffffffffUL;
            observedAtCells[index]        = 0xffffffffUL;
            sequenceCells[index]          = 0xffffffffUL;
            ageCells[index]               = 0xffffffffUL;
            maximumAgeCells[index]        = 0xffffffffUL;
            sequenceGapCells[index]       = 0xffffffffUL;
            accelerationXCells[index]     = -2147483647L - 1L;
            accelerationYCells[index]     = -2147483647L - 1L;
            accelerationZCells[index]     = -2147483647L - 1L;
            angularRateXCells[index]      = -2147483647L - 1L;
            angularRateYCells[index]      = -2147483647L - 1L;
            angularRateZCells[index]      = -2147483647L - 1L;
        }
    }

    adk::Status observeCopiedFrame (uint8_t index)
    {
        const ReplayFrame& frame = replayFrames[index];

        return observationPolicy.update (frame.policyTime, frame.sample);
    }

    DiagnosticCode decideDiagnostic (
        const adk::InertialObservation& observation)
    {
        if (!observation.status.ok ())
        {
            return observation.status.error () ==
                           adk::StatusCode::HardwareFailure
                       ? DiagnosticCode::ProducerFault
                       : DiagnosticCode::Invalid;
        }

        if (observation.quality == adk::InertialSampleQuality::Current)
        {
            return DiagnosticCode::Current;
        }
        if (observation.quality == adk::InertialSampleQuality::Stale)
        {
            return DiagnosticCode::Stale;
        }
        if (observation.quality == adk::InertialSampleQuality::Saturated)
        {
            return DiagnosticCode::Saturated;
        }

        return DiagnosticCode::Invalid;
    }

    void presentObservation (
        uint8_t index, const adk::InertialObservation& observation,
        DiagnosticCode diagnostic)
    {
        diagnosticCells[index] =
            static_cast<uint8_t> (diagnostic);
        statusCells[index] =
            static_cast<uint8_t> (observation.status.error ());
        qualityCells[index] = static_cast<uint8_t> (observation.quality);
        sourceKindCells[index] =
            static_cast<uint8_t> (observation.sample.source.kind);
        sourceModelCells[index] =
            static_cast<uint8_t> (observation.sample.source.model);
        sourceIdCells[index] = observation.sample.source.sourceId;
        acceptedDataReadyCells[index] =
            static_cast<uint8_t> (observation.sample.dataReady);
        latestDataReadyCells[index] =
            static_cast<uint8_t> (observation.latestDataReady);
        saturationCells[index] =
            static_cast<uint8_t> (observation.sample.saturation);
        producerStatusCells[index] =
            static_cast<uint8_t> (observation.sample.status.error ());
        configurationCells[index] =
            observation.sample.source.configurationRevision;
        calibrationCells[index] =
            observation.sample.source.calibrationRevision;
        freshnessRevisionCells[index] =
            observation.freshnessContractRevision;
        accelerationRangeCells[index] =
            observation.sample.source.accelerationRangeMicroG;
        angularRateRangeCells[index] =
            observation.sample.source.angularRateRangeMilliDegreesPerSecond;
        observedAtCells[index] =
            observation.sample.observedAt.milliseconds ();
        sequenceCells[index]    = observation.sample.sequence;
        ageCells[index]         =
            observation.age.milliseconds ();
        maximumAgeCells[index]  =
            observation.maximumAge.milliseconds ();
        sequenceGapCells[index] = observation.sequenceGap;
        accelerationXCells[index]  =
            observation.sample.accelerationMicroG.x;
        accelerationYCells[index]  =
            observation.sample.accelerationMicroG.y;
        accelerationZCells[index]  =
            observation.sample.accelerationMicroG.z;
        angularRateXCells[index]   =
            observation.sample.angularRateMilliDegreesPerSecond.x;
        angularRateYCells[index]   =
            observation.sample.angularRateMilliDegreesPerSecond.y;
        angularRateZCells[index]   =
            observation.sample.angularRateMilliDegreesPerSecond.z;
    }

} // namespace
