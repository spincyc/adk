// E0 copied-evidence fixture. This sketch replays supplied time and complete
// synthetic acquisition cycles into named memory result cells. It owns no
// probe, ADC channel, excitation supply, pin, clock, or powered circuit.
#include <Adk.h>
#include <resistive_probe_observation.h>

namespace {

    struct ReplayFrame
    {
        uint32_t                  now;
        adk::ResistiveProbeSample sample;
        adk::ProbeQuality         expectedQuality;
        adk::StatusCode           expectedStatus;
    };

    struct ProbeResultCell
    {
        uint32_t observedAt;
        uint32_t age;
        uint32_t sequence;
        uint16_t energizedRaw;
        uint16_t dischargedRaw;
        uint16_t normalizedPermille;
        uint16_t cycleDutyPermille;
        uint8_t  sourceId;
        uint8_t  quality;
        uint8_t  status;
        uint8_t  excitationObservedOff;
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

    const adk::ResistiveProbeConfig probeConfig = {
        1023, 900, 300, 20, 25, 350, 700, adk::Duration (100), adk::Duration (5), 100};

    const ReplayFrame replayFrames[] = {
        {100,
         {61, 1, 1, 1, adk::TimePoint (100), 900, 5, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::Ok},
         adk::ProbeQuality::Dry,
         adk::StatusCode::Ok},
        {110,
         {61, 1, 1, 2, adk::TimePoint (110), 650, 4, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::Ok},
         adk::ProbeQuality::Damp,
         adk::StatusCode::Ok},
        {120,
         {61, 1, 1, 3, adk::TimePoint (120), 350, 3, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::Ok},
         adk::ProbeQuality::Wet,
         adk::StatusCode::Ok},
        {130,
         {61, 1, 1, 4, adk::TimePoint (130), 500, 80, adk::Duration (2),
          adk::Duration (100), false, adk::StatusCode::Ok},
         adk::ProbeQuality::ExcitationFault,
         adk::StatusCode::Ok},
        {140,
         {61, 1, 1, 5, adk::TimePoint (140), 1023, 2, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::Ok},
         adk::ProbeQuality::Saturated,
         adk::StatusCode::Ok},
        {145,
         {61, 1, 1, 6, adk::TimePoint (145), 10, 2, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::Ok},
         adk::ProbeQuality::Disconnected,
         adk::StatusCode::Ok},
        {260,
         {61, 1, 1, 7, adk::TimePoint (150), 900, 2, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::Ok},
         adk::ProbeQuality::Stale,
         adk::StatusCode::Ok},
        {270,
         {61, 1, 1, 8, adk::TimePoint (270), 700, 2, adk::Duration (2),
          adk::Duration (100), true, adk::StatusCode::HardwareFailure},
         adk::ProbeQuality::ProducerFault,
         adk::StatusCode::HardwareFailure}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::ResistiveProbeObservationPolicy probePolicy (probeConfig);

    volatile ProbeResultCell  probeResultCells[replayFrameCount];
    volatile ProbeResultCell  initializedResultCell;
    volatile ProbeResultCell  resetResultCell;
    volatile ReplayResultCell replayResultCell;

    uint8_t replayIndex;

    // clang-format off
    adk::Status        acquireSyntheticFixture ();
    void               configureProbeReplay    ();
    adk::Status        startProbeObservation   ();
    bool               predictUnqualified      (
        const adk::ResistiveProbeObservation& observation);
    const ReplayFrame& observeCopiedCycle      ();
    bool               decideProbeObservation  (
        const ReplayFrame& frame,
        const adk::ResistiveProbeObservation& observation);
    void               presentProbeObservation (
        volatile ProbeResultCell& cell,
        const adk::ResistiveProbeObservation& observation,
        bool prediction);
    void               finishReplay            ();
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

    configureProbeReplay ();

    const adk::Status initializeStatus = startProbeObservation ();
    replayResultCell.initializeStatus =
        static_cast<uint8_t> (initializeStatus.error ());

    if (!initializeStatus.ok ())
    {
        finishReplay ();
        return;
    }

    const adk::ResistiveProbeObservation initializedObservation =
        probePolicy.snapshot ();
    const bool initializedPrediction =
        predictUnqualified (initializedObservation);
    presentProbeObservation (
        initializedResultCell, initializedObservation, initializedPrediction);

    probePolicy.reset ();
    const adk::ResistiveProbeObservation resetObservation =
        probePolicy.snapshot ();
    const bool resetPrediction = predictUnqualified (resetObservation);

    presentProbeObservation (resetResultCell, resetObservation, resetPrediction);

    replayResultCell.predictionsPass =
        initializedPrediction && resetPrediction ? 1 : 0;
}

void loop ()
{
    if (replayResultCell.complete != 0)
    {
        return;
    }

    const ReplayFrame& frame = observeCopiedCycle ();
    const adk::Status  updateStatus =
        probePolicy.update (adk::TimePoint (frame.now), frame.sample);

    const adk::ResistiveProbeObservation observation = probePolicy.snapshot ();

    const bool prediction = updateStatus.error () == frame.expectedStatus &&
                            decideProbeObservation (frame, observation);

    presentProbeObservation (probeResultCells[replayIndex], observation, prediction);

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
        return replayFrameCount == 8 ? adk::StatusCode::Ok
                                     : adk::StatusCode::InternalInvariant;
    }

    void configureProbeReplay ()
    {
        replayIndex                       = 0;
        replayResultCell.initializeStatus = 0xff;
        replayResultCell.completedFrames  = 0;
        replayResultCell.predictionsPass  = 1;
        replayResultCell.terminalStatus =
            static_cast<uint8_t> (adk::StatusCode::NotInitialized);
        replayResultCell.terminalAssertionPass = 0;
        replayResultCell.complete         = 0;
    }

    adk::Status startProbeObservation ()
    {
        return probePolicy.initialize ();
    }

    bool predictUnqualified (
        const adk::ResistiveProbeObservation& observation)
    {
        const adk::Duration emptyDuration;

        return observation.sample.sourceId == 0 &&
               observation.sample.configurationRevision == 0 &&
               observation.sample.calibrationRevision == 0 &&
               observation.sample.sequence == 0 &&
               observation.sample.observedAt.milliseconds () == 0 &&
               observation.sample.energizedRaw == 0 &&
               observation.sample.dischargedRaw == 0 &&
               observation.sample.excitationOnTime == emptyDuration &&
               observation.sample.cycleTime == emptyDuration &&
               !observation.sample.excitationObservedOffAfterSample &&
               observation.sample.status.error () ==
                   adk::StatusCode::NotInitialized &&
               observation.normalizedPermille == 0 &&
               observation.observedCycleDutyPermille == 0 &&
               observation.quality == adk::ProbeQuality::Unqualified &&
               observation.age == emptyDuration &&
               observation.status == adk::StatusCode::NotInitialized;
    }

    const ReplayFrame& observeCopiedCycle ()
    {
        return replayFrames[replayIndex];
    }

    bool decideProbeObservation (const ReplayFrame&                    frame,
                                 const adk::ResistiveProbeObservation& observation)
    {
        const uint16_t expectedNormalized =
            frame.sample.energizedRaw >= probeConfig.dryReference
                ? 0
                : frame.sample.energizedRaw <= probeConfig.wetReference
                      ? 1000
                      : static_cast<uint16_t> (
                            (static_cast<uint32_t> (
                                 probeConfig.dryReference -
                                 frame.sample.energizedRaw) *
                             1000UL) /
                            static_cast<uint32_t> (
                                probeConfig.dryReference -
                                probeConfig.wetReference));
        const uint16_t expectedDuty = static_cast<uint16_t> (
            (static_cast<uint64_t> (
                 frame.sample.excitationOnTime.milliseconds ()) *
             1000ULL) /
            frame.sample.cycleTime.milliseconds ());

        return observation.quality == frame.expectedQuality &&
               observation.status.error () == frame.expectedStatus &&
               observation.sample.sourceId == frame.sample.sourceId &&
               observation.sample.configurationRevision ==
                   frame.sample.configurationRevision &&
               observation.sample.calibrationRevision ==
                   frame.sample.calibrationRevision &&
               observation.sample.sequence == frame.sample.sequence &&
               observation.sample.observedAt.milliseconds () ==
                   frame.sample.observedAt.milliseconds () &&
               observation.sample.energizedRaw == frame.sample.energizedRaw &&
               observation.sample.dischargedRaw == frame.sample.dischargedRaw &&
               observation.sample.excitationOnTime ==
                   frame.sample.excitationOnTime &&
               observation.sample.cycleTime == frame.sample.cycleTime &&
               observation.sample.excitationObservedOffAfterSample ==
                   frame.sample.excitationObservedOffAfterSample &&
               observation.sample.status == frame.sample.status &&
               observation.normalizedPermille == expectedNormalized &&
               observation.observedCycleDutyPermille == expectedDuty &&
               observation.age.milliseconds () ==
                   frame.now - frame.sample.observedAt.milliseconds ();
    }

    void presentProbeObservation (volatile ProbeResultCell&             cell,
                                  const adk::ResistiveProbeObservation& observation,
                                  bool                                  prediction)
    {
        cell.observedAt = observation.sample.observedAt.milliseconds ();

        cell.age                 = observation.age.milliseconds ();
        cell.sequence            = observation.sample.sequence;
        cell.energizedRaw        = observation.sample.energizedRaw;
        cell.dischargedRaw       = observation.sample.dischargedRaw;
        cell.normalizedPermille  = observation.normalizedPermille;
        cell.cycleDutyPermille =
            observation.observedCycleDutyPermille;
        cell.sourceId = observation.sample.sourceId;
        cell.quality  = static_cast<uint8_t> (observation.quality);
        cell.status   = static_cast<uint8_t> (observation.status.error ());
        cell.excitationObservedOff =
            observation.sample.excitationObservedOffAfterSample ? 1 : 0;
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

        replayResultCell.terminalStatus =
            static_cast<uint8_t> (terminalStatus);
        const bool terminalAssertion = terminalStatus == adk::StatusCode::Ok;
        replayResultCell.terminalAssertionPass = terminalAssertion ? 1 : 0;
        replayResultCell.complete               = 1;
    }

} // namespace
