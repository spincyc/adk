// E0 replay fixture: D30, D31, and D32 each drive an LED through 1 kOhm
// to GND. D30 presents qualified optical state, D31 presents a qualified
// transition, and D32 presents ready evidence. After shutdown, D30/TP-OA,
// D31/TP-OB, and D32/TP-OC are separate safe-state test points; LED-off alone
// does not identify an acquisition fault. No optical sensor is powered or
// electrically identified by this sketch. The copied samples are deterministic
// teaching evidence, not bench acceptance for a retail module.
#include <Adk.h>
#include <optical_observation.h>

namespace {

    constexpr adk::PinId stateEvidencePin      = 30;
    constexpr adk::PinId transitionEvidencePin = 31;
    constexpr adk::PinId readyEvidencePin      = 32;
    constexpr uint32_t   replayStepMs          = 500;

    const adk::ReflectiveObservationConfig reflectiveConfig = {
        1, 4, 100, 900, 800, 200, 300, 500, adk::Duration (40), true};
    const adk::BeamObservationConfig beamConfig = {
        1, 2, adk::Level::Low, adk::Duration (30), adk::Duration (30)};

    struct ReplayFrame
    {
        uint32_t   observedAtMs;
        uint16_t   reflectiveRaw;
        adk::Level beamLevel;
    };

    const ReplayFrame replayFrames[] = {
        {0, 700, adk::Level::High},   {20, 700, adk::Level::High},
        {40, 760, adk::Level::High},  {60, 760, adk::Level::High},
        {80, 760, adk::Level::Low},   {100, 760, adk::Level::Low},
        {120, 760, adk::Level::Low},  {140, 400, adk::Level::High},
        {160, 400, adk::Level::High}, {180, 400, adk::Level::High}};
    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::Runtime                     runtime;
    adk::MonoLed                     stateEvidence      (
        runtime.resources (), stateEvidencePin);
    adk::MonoLed                     transitionEvidence (
        runtime.resources (), transitionEvidencePin);
    adk::MonoLed                     readyEvidence      (
        runtime.resources (), readyEvidencePin);
    adk::ReflectiveObservationPolicy reflectivePolicy  (reflectiveConfig);
    adk::BeamObservationPolicy       beamPolicy        (beamConfig);

    adk::TimePoint lastReplayStep;
    uint8_t        replayFrame = 0;
    bool           running     = false;

    bool acquirePresentation   ();
    void configureReplay       (adk::TimePoint now);
    bool startReplay           ();
    bool observeCopiedEvidence ();
    bool decideOpticalState    (bool& stateActive, bool& transitionActive,
                                bool& observationsReady);
    bool presentOpticalState   (bool stateActive, bool transitionActive,
                                bool observationsReady);
    void stopSafely            ();

} // namespace

void setup ()
{
    const adk::TimePoint now (millis ());

    if (acquirePresentation ())
    {
        configureReplay           (now);
        running = startReplay     ();
    }

    if (!running)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (now.elapsedSince (lastReplayStep).milliseconds () < replayStepMs)
    {
        return;
    }

    bool stateActive       = false;
    bool transitionActive  = false;
    bool observationsReady = false;

    if (!observeCopiedEvidence () ||
        !decideOpticalState    (stateActive, transitionActive, observationsReady) ||
        !presentOpticalState   (stateActive, transitionActive, observationsReady))
    {
        stopSafely ();
        return;
    }

    lastReplayStep = now;
    ++replayFrame;

    if (replayFrame == replayFrameCount)
    {
        reflectivePolicy.reset ();
        beamPolicy.reset       ();
        replayFrame = 0;
    }
}

namespace {

    bool acquirePresentation ()
    {
        if (!stateEvidence.initialize ().ok ())
        {
            return false;
        }

        if (!transitionEvidence.initialize ().ok ())
        {
            stateEvidence.shutdown ();
            return false;
        }

        if (!readyEvidence.initialize ().ok ())
        {
            transitionEvidence.shutdown ();
            stateEvidence.shutdown      ();
            return false;
        }

        return true;
    }

    void configureReplay (adk::TimePoint now)
    {
        lastReplayStep = now;
        replayFrame    = 0;
    }

    bool startReplay ()
    {
        if (!reflectivePolicy.initialize ().ok () || !beamPolicy.initialize ().ok ())
        {
            return false;
        }

        return stateEvidence.off ().ok () && transitionEvidence.off ().ok () &&
               readyEvidence.off ().ok ();
    }

    bool observeCopiedEvidence ()
    {
        const ReplayFrame&          frame = replayFrames[replayFrame];
        const adk::TimePoint        observedAt (frame.observedAtMs);
        const adk::ReflectiveSample reflectiveSample = {
            {reflectiveConfig.sourceId, reflectiveConfig.calibrationRevision,
             observedAt},
            frame.reflectiveRaw,
            adk::StatusCode::Ok};
        const adk::BeamSample beamSample = {
            {beamConfig.sourceId, beamConfig.calibrationRevision, observedAt},
            frame.beamLevel,
            adk::StatusCode::Ok};

        return reflectivePolicy.update (reflectiveSample).ok () &&
               beamPolicy.update (beamSample).ok ();
    }

    bool decideOpticalState (bool& stateActive, bool& transitionActive,
                             bool& observationsReady)
    {
        const adk::ReflectiveObservation reflective = reflectivePolicy.snapshot ();
        const adk::BeamObservation       beam       = beamPolicy.snapshot       ();

        if (!reflective.status.ok () || !beam.status.ok ())
        {
            return false;
        }

        stateActive      = reflective.markerActive || beam.interrupted;
        transitionActive = reflective.activationEvent || reflective.deactivationEvent ||
                           beam.interruptionEvent || beam.restorationEvent;
        observationsReady = reflective.quality == adk::OpticalQuality::Valid &&
                            beam.quality == adk::OpticalQuality::Valid;
        return true;
    }

    bool presentOpticalState (bool stateActive, bool transitionActive,
                              bool observationsReady)
    {
        return stateEvidence.set      (stateActive).ok () &&
               transitionEvidence.set (transitionActive).ok () &&
               readyEvidence.set      (observationsReady).ok ();
    }

    void stopSafely ()
    {
        readyEvidence.shutdown      ();
        transitionEvidence.shutdown ();
        stateEvidence.shutdown      ();
        running = false;
    }

} // namespace
