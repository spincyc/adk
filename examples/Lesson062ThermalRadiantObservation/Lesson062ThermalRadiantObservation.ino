// E0 copied-evidence fixture. This sketch replays supplied time and complete
// synthetic source envelopes into named memory result cells. It owns no
// thermistor, threshold module, radiant source, pin, transport, or clock.
#include <Adk.h>
#include <thermal_radiant_observation.h>

namespace {

    enum struct ThermistorBand : uint8_t
    {
        Normal,
        Warning,
        Alarm
    };

    struct ReplayFrame
    {
        uint32_t                    now;
        adk::ThermalRadiantEnvelope envelope;
        ThermistorBand              expectedThermistorBand;
        adk::ThermalQuality         expectedThermalQuality;
        adk::RadiantQuality         expectedRadiantQuality;
        bool                        expectedThermalHazard;
        bool                        expectedRadiantHazard;
        adk::StatusCode             expectedStatus;
    };

    struct ObservationResultCell
    {
        int32_t  thermistorMilliCelsius;
        uint32_t thermistorUncertainty;
        uint32_t thermistorAge;
        uint32_t digitalTemperatureAge;
        uint32_t radiantAge;
        uint16_t digitalTemperatureRaw;
        uint16_t radiantRaw;
        uint8_t  thermistorBand;
        uint8_t  digitalTemperatureState;
        uint8_t  radiantState;
        uint8_t  thermalQuality;
        uint8_t  radiantQuality;
        uint8_t  thermalHazard;
        uint8_t  radiantHazard;
        uint8_t  status;
        uint8_t  predictionPass;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t initializeStatus;
        uint8_t completedFrames;
        uint8_t predictionsPass;
        uint8_t terminalStatus;
        uint8_t terminalAssertionPass;
        uint8_t complete;
    };

    const adk::ThermalRadiantConfig observationConfig = {
        25000, 30000, adk::Duration (100), adk::Duration (10), adk::Duration (20)};

    const ReplayFrame replayFrames[] = {
        {100,
         {{11, 1, 1, 1, adk::TimePoint (100), 22000, 1000, false, adk::StatusCode::Ok},
          {12, 1, 1, 1, adk::TimePoint (100), 300, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok},
          {13, 1, 1, 1, adk::TimePoint (100), 100, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok}},
         ThermistorBand::Normal,
         adk::ThermalQuality::Normal,
         adk::RadiantQuality::Quiet,
         false,
         false,
         adk::StatusCode::Ok},
        {110,
         {{11, 1, 1, 2, adk::TimePoint (110), 24000, 1000, false, adk::StatusCode::Ok},
          {12, 1, 1, 2, adk::TimePoint (110), 320, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok},
          {13, 1, 1, 2, adk::TimePoint (110), 700, adk::ThresholdState::AtOrAbove,
           false, adk::StatusCode::Ok}},
         ThermistorBand::Warning,
         adk::ThermalQuality::Disagreement,
         adk::RadiantQuality::AbruptChange,
         false,
         true,
         adk::StatusCode::Ok},
        {130,
         {{11, 1, 1, 3, adk::TimePoint (130), 29500, 500, false, adk::StatusCode::Ok},
          {12, 1, 1, 3, adk::TimePoint (130), 800, adk::ThresholdState::AtOrAbove,
           false, adk::StatusCode::Ok},
          {13, 1, 1, 3, adk::TimePoint (130), 720, adk::ThresholdState::AtOrAbove,
           false, adk::StatusCode::Ok}},
         ThermistorBand::Alarm,
         adk::ThermalQuality::Alarm,
         adk::RadiantQuality::Sustained,
         true,
         true,
         adk::StatusCode::Ok},
        {135,
         {{11, 1, 1, 4, adk::TimePoint (135), 31000, 100, false, adk::StatusCode::Ok},
          {12, 1, 1, 4, adk::TimePoint (135), 350, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok},
          {13, 1, 1, 4, adk::TimePoint (135), 120, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok}},
         ThermistorBand::Alarm,
         adk::ThermalQuality::Disagreement,
         adk::RadiantQuality::Sustained,
         true,
         true,
         adk::StatusCode::Ok},
        {145,
         {{11, 1, 1, 5, adk::TimePoint (145), 22000, 1000, false, adk::StatusCode::Ok},
          {12, 1, 1, 5, adk::TimePoint (145), 300, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok},
          {13, 1, 1, 5, adk::TimePoint (145), 700, adk::ThresholdState::AtOrAbove,
           false, adk::StatusCode::Ok}},
         ThermistorBand::Normal,
         adk::ThermalQuality::Normal,
         adk::RadiantQuality::AbruptChange,
         false,
         true,
         adk::StatusCode::Ok},
        {155,
         {{11, 1, 1, 6, adk::TimePoint (155), 22000, 1000, false, adk::StatusCode::Ok},
          {12, 1, 1, 6, adk::TimePoint (155), 300, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok},
          {13, 1, 1, 6, adk::TimePoint (155), 100, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok}},
         ThermistorBand::Normal,
         adk::ThermalQuality::Normal,
         adk::RadiantQuality::AbruptChange,
         false,
         false,
         adk::StatusCode::Ok},
        {165,
         {{11, 1, 1, 7, adk::TimePoint (165), 22000, 1000, true, adk::StatusCode::Ok},
          {12, 1, 1, 7, adk::TimePoint (165), 1023, adk::ThresholdState::AtOrAbove,
           true, adk::StatusCode::Ok},
          {13, 1, 1, 7, adk::TimePoint (165), 1023, adk::ThresholdState::AtOrAbove,
           true, adk::StatusCode::Ok}},
         ThermistorBand::Normal,
         adk::ThermalQuality::Saturated,
         adk::RadiantQuality::SaturatedAmbient,
         false,
         false,
         adk::StatusCode::Ok},
        {271,
         {{11, 1, 1, 8, adk::TimePoint (170), 22000, 1000, false, adk::StatusCode::Ok},
          {12, 1, 1, 8, adk::TimePoint (170), 300, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok},
          {13, 1, 1, 8, adk::TimePoint (170), 100, adk::ThresholdState::Below, false,
           adk::StatusCode::Ok}},
         ThermistorBand::Normal,
         adk::ThermalQuality::Stale,
         adk::RadiantQuality::Stale,
         false,
         false,
         adk::StatusCode::Ok},
        {280,
         {{11, 1, 1, 9, adk::TimePoint (280), 22000, 1000, false,
           adk::StatusCode::HardwareFailure},
          {12, 1, 1, 9, adk::TimePoint (280), 300, adk::ThresholdState::Below, false,
           adk::StatusCode::HardwareFailure},
          {13, 1, 1, 9, adk::TimePoint (280), 100, adk::ThresholdState::Below, false,
           adk::StatusCode::HardwareFailure}},
         ThermistorBand::Normal,
         adk::ThermalQuality::ProducerFault,
         adk::RadiantQuality::ProducerFault,
         false,
         false,
         adk::StatusCode::HardwareFailure}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::ThermalRadiantObservationPolicy observationPolicy (observationConfig);

    volatile ObservationResultCell observationResultCells[replayFrameCount];
    volatile ObservationResultCell initializedResultCell;
    volatile ObservationResultCell resetResultCell;
    volatile ReplayResultCell      replayResultCell;

    uint8_t replayIndex;

    // clang-format off
    adk::Status        acquireSyntheticFixture     ();
    void               configureObservationReplay  ();
    adk::Status        startObservationPolicy      ();
    bool               predictUnqualified          (
        const adk::ThermalRadiantObservation& observation);
    ThermistorBand     classifyThermistor           (
        const adk::ConvertedThermalSample& sample);
    const ReplayFrame& observeCopiedEnvelope       ();
    bool               decideObservation           (
        const ReplayFrame& frame,
        const adk::ThermalRadiantObservation& observation);
    void               presentObservation          (
        volatile ObservationResultCell& cell,
        const adk::ThermalRadiantObservation& observation,
        bool prediction);
    void               finishReplay                ();
    // clang-format on

} // namespace

void setup ()
{
    replayResultCell.initializeStatus =
        static_cast<uint8_t> (adk::StatusCode::NotInitialized);

    const adk::Status fixtureStatus = acquireSyntheticFixture ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        finishReplay ();
        return;
    }

    configureObservationReplay ();

    const adk::Status initializeStatus = startObservationPolicy ();
    replayResultCell.initializeStatus =
        static_cast<uint8_t> (initializeStatus.error ());
    if (!initializeStatus.ok ())
    {
        finishReplay ();
        return;
    }

    const adk::ThermalRadiantObservation initializedObservation =
        observationPolicy.snapshot ();
    const bool initializedPrediction = predictUnqualified (initializedObservation);

    presentObservation (initializedResultCell, initializedObservation,
                        initializedPrediction);

    observationPolicy.reset ();
    const adk::ThermalRadiantObservation resetObservation =
        observationPolicy.snapshot ();
    const bool resetPrediction = predictUnqualified (resetObservation);

    presentObservation (resetResultCell, resetObservation, resetPrediction);

    replayResultCell.predictionsPass = initializedPrediction && resetPrediction ? 1 : 0;
}

void loop ()
{
    if (replayResultCell.complete != 0)
    {
        return;
    }

    const ReplayFrame& frame = observeCopiedEnvelope ();
    const adk::Status  updateStatus =
        observationPolicy.update (adk::TimePoint (frame.now), frame.envelope);
    const adk::ThermalRadiantObservation observation = observationPolicy.snapshot ();

    const bool prediction = updateStatus.error () == frame.expectedStatus &&
                            decideObservation (frame, observation);

    presentObservation (observationResultCells[replayIndex], observation, prediction);

    replayResultCell.predictionsPass =
        replayResultCell.predictionsPass && prediction ? 1 : 0;
    ++replayIndex;
    replayResultCell.completedFrames = replayIndex;
    if (replayIndex == replayFrameCount)
    {
        finishReplay ();
    }
}

namespace {

    adk::Status acquireSyntheticFixture ()
    {
        return replayFrameCount == 9 ? adk::StatusCode::Ok
                                     : adk::StatusCode::InternalInvariant;
    }

    void configureObservationReplay ()
    {
        replayIndex                       = 0;
        replayResultCell.initializeStatus = 0xff;
        replayResultCell.completedFrames  = 0;
        replayResultCell.predictionsPass  = 1;
        replayResultCell.terminalStatus =
            static_cast<uint8_t> (adk::StatusCode::NotInitialized);
        replayResultCell.terminalAssertionPass = 0;
        replayResultCell.complete              = 0;
    }

    adk::Status startObservationPolicy ()
    {
        return observationPolicy.initialize ();
    }

    bool predictUnqualified (const adk::ThermalRadiantObservation& observation)
    {
        const adk::Duration emptyDuration;

        return observation.envelope.thermistor.sourceId == 0 &&
               observation.envelope.digitalTemperature.sourceId == 0 &&
               observation.envelope.radiant.sourceId == 0 &&
               observation.thermalQuality == adk::ThermalQuality::Unqualified &&
               observation.radiantQuality == adk::RadiantQuality::Unqualified &&
               observation.thermistorAge == emptyDuration &&
               observation.digitalTemperatureAge == emptyDuration &&
               observation.radiantAge == emptyDuration && !observation.thermalHazard &&
               !observation.radiantHazard &&
               observation.status == adk::StatusCode::NotInitialized;
    }

    ThermistorBand classifyThermistor (const adk::ConvertedThermalSample& sample)
    {
        const int64_t upper =
            static_cast<int64_t> (sample.milliCelsius) + sample.uncertaintyMilliCelsius;

        if (upper >= observationConfig.alarmMilliCelsius)
        {
            return ThermistorBand::Alarm;
        }

        return upper >= observationConfig.warningMilliCelsius ? ThermistorBand::Warning
                                                              : ThermistorBand::Normal;
    }

    const ReplayFrame& observeCopiedEnvelope ()
    {
        return replayFrames[replayIndex];
    }

    bool decideObservation (const ReplayFrame&                    frame,
                            const adk::ThermalRadiantObservation& observation)
    {
        const ThermistorBand thermistorBand =
            classifyThermistor (observation.envelope.thermistor);

        return observation.envelope.thermistor.sourceId ==
                   frame.envelope.thermistor.sourceId &&
               observation.envelope.thermistor.configurationRevision ==
                   frame.envelope.thermistor.configurationRevision &&
               observation.envelope.thermistor.calibrationRevision ==
                   frame.envelope.thermistor.calibrationRevision &&
               observation.envelope.thermistor.sequence ==
                   frame.envelope.thermistor.sequence &&
               observation.envelope.thermistor.observedAt ==
                   frame.envelope.thermistor.observedAt &&
               observation.envelope.thermistor.milliCelsius ==
                   frame.envelope.thermistor.milliCelsius &&
               observation.envelope.thermistor.uncertaintyMilliCelsius ==
                   frame.envelope.thermistor.uncertaintyMilliCelsius &&
               observation.envelope.thermistor.saturated ==
                   frame.envelope.thermistor.saturated &&
               observation.envelope.thermistor.status ==
                   frame.envelope.thermistor.status &&
               observation.envelope.digitalTemperature.sourceId ==
                   frame.envelope.digitalTemperature.sourceId &&
               observation.envelope.digitalTemperature.configurationRevision ==
                   frame.envelope.digitalTemperature.configurationRevision &&
               observation.envelope.digitalTemperature.calibrationRevision ==
                   frame.envelope.digitalTemperature.calibrationRevision &&
               observation.envelope.digitalTemperature.sequence ==
                   frame.envelope.digitalTemperature.sequence &&
               observation.envelope.digitalTemperature.observedAt ==
                   frame.envelope.digitalTemperature.observedAt &&
               observation.envelope.digitalTemperature.raw ==
                   frame.envelope.digitalTemperature.raw &&
               observation.envelope.digitalTemperature.state ==
                   frame.envelope.digitalTemperature.state &&
               observation.envelope.digitalTemperature.saturated ==
                   frame.envelope.digitalTemperature.saturated &&
               observation.envelope.digitalTemperature.status ==
                   frame.envelope.digitalTemperature.status &&
               observation.envelope.radiant.sourceId ==
                   frame.envelope.radiant.sourceId &&
               observation.envelope.radiant.configurationRevision ==
                   frame.envelope.radiant.configurationRevision &&
               observation.envelope.radiant.calibrationRevision ==
                   frame.envelope.radiant.calibrationRevision &&
               observation.envelope.radiant.sequence ==
                   frame.envelope.radiant.sequence &&
               observation.envelope.radiant.observedAt ==
                   frame.envelope.radiant.observedAt &&
               observation.envelope.radiant.raw == frame.envelope.radiant.raw &&
               observation.envelope.radiant.state == frame.envelope.radiant.state &&
               observation.envelope.radiant.saturated ==
                   frame.envelope.radiant.saturated &&
               observation.envelope.radiant.status == frame.envelope.radiant.status &&
               thermistorBand == frame.expectedThermistorBand &&
               observation.thermalQuality == frame.expectedThermalQuality &&
               observation.radiantQuality == frame.expectedRadiantQuality &&
               observation.thermalHazard == frame.expectedThermalHazard &&
               observation.radiantHazard == frame.expectedRadiantHazard &&
               observation.thermistorAge.milliseconds () ==
                   frame.now - frame.envelope.thermistor.observedAt.milliseconds () &&
               observation.digitalTemperatureAge.milliseconds () ==
                   frame.now -
                       frame.envelope.digitalTemperature.observedAt.milliseconds () &&
               observation.radiantAge.milliseconds () ==
                   frame.now - frame.envelope.radiant.observedAt.milliseconds () &&
               observation.status.error () == frame.expectedStatus;
    }

    void presentObservation (volatile ObservationResultCell&       cell,
                             const adk::ThermalRadiantObservation& observation,
                             bool                                  prediction)
    {
        cell.thermistorMilliCelsius = observation.envelope.thermistor.milliCelsius;
        cell.thermistorUncertainty =
            observation.envelope.thermistor.uncertaintyMilliCelsius;
        cell.thermistorAge = observation.thermistorAge.milliseconds ();

        cell.digitalTemperatureAge = observation.digitalTemperatureAge.milliseconds ();

        cell.radiantAge            = observation.radiantAge.milliseconds ();
        cell.digitalTemperatureRaw = observation.envelope.digitalTemperature.raw;
        cell.radiantRaw            = observation.envelope.radiant.raw;
        cell.thermistorBand =
            static_cast<uint8_t> (classifyThermistor (observation.envelope.thermistor));
        cell.digitalTemperatureState =
            static_cast<uint8_t> (observation.envelope.digitalTemperature.state);
        cell.radiantState   = static_cast<uint8_t> (observation.envelope.radiant.state);
        cell.thermalQuality = static_cast<uint8_t> (observation.thermalQuality);
        cell.radiantQuality = static_cast<uint8_t> (observation.radiantQuality);
        cell.thermalHazard  = observation.thermalHazard ? 1 : 0;
        cell.radiantHazard  = observation.radiantHazard ? 1 : 0;
        cell.status         = static_cast<uint8_t> (observation.status.error ());
        cell.predictionPass = prediction ? 1 : 0;
    }

    void finishReplay ()
    {
        adk::StatusCode terminalStatus = adk::StatusCode::InternalInvariant;

        if (replayResultCell.fixtureStatus !=
            static_cast<uint8_t> (adk::StatusCode::Ok))
        {
            terminalStatus =
                static_cast<adk::StatusCode> (replayResultCell.fixtureStatus);
        }
        else if (replayResultCell.initializeStatus !=
                 static_cast<uint8_t> (adk::StatusCode::Ok))
        {
            terminalStatus =
                static_cast<adk::StatusCode> (replayResultCell.initializeStatus);
        }
        else if (replayResultCell.completedFrames == replayFrameCount &&
                 replayResultCell.predictionsPass != 0)
        {
            terminalStatus = adk::StatusCode::Ok;
        }

        replayResultCell.terminalStatus = static_cast<uint8_t> (terminalStatus);
        replayResultCell.terminalAssertionPass =
            terminalStatus == adk::StatusCode::Ok ? 1 : 0;
        replayResultCell.complete = 1;
    }

} // namespace
