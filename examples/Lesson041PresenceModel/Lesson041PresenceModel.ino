// The copied sensor-evidence replay is E0. Running it with the documented E1
// presentation uses D30 and D31, each driving an LED through 1 kOhm to GND.
// TP-PRESENCE is D31 measured relative to Mega GND. No PIR, optical, or
// ultrasonic module is connected or powered. D30 indicates acquisition only
// after both LEDs and the policy initialize; D31 provides separate electrical
// safe-state and replay evidence. Compilation is not physical acceptance.
#include <Adk.h>

namespace {

    constexpr uint8_t pirBit =
        static_cast<uint8_t> (adk::PresenceSourceBit::Pir);
    constexpr uint8_t beamBit =
        static_cast<uint8_t> (adk::PresenceSourceBit::Beam);
    constexpr uint8_t guardBit =
        static_cast<uint8_t> (adk::PresenceSourceBit::FinishGuard);
    constexpr uint8_t rangeBit =
        static_cast<uint8_t> (adk::PresenceSourceBit::Range);

    const adk::PresenceModelConfig presenceConfig = {
        static_cast<uint8_t> (pirBit | beamBit | guardBit | rangeBit),
        static_cast<uint8_t> (pirBit | beamBit | rangeBit),
        adk::Duration (100),
        adk::Duration (100),
        adk::Duration (100),
        adk::Duration (100),
        adk::Duration (1000),
        adk::Duration (50),
        100,
        500};

    adk::PresenceModel presence (presenceConfig);
    adk::Runtime       runtime;
    adk::MonoLed       acquisitionLed (runtime.resources (), 30);
    adk::MonoLed       presenceLed    (runtime.resources (), 31);

    volatile uint8_t  evidenceCell = 0;
    volatile uint8_t  qualityCell  = 0;
    volatile uint16_t rangeCell    = 0;

    uint8_t replayIndex = 0;
    bool    replaying   = false;

    adk::PresenceInput copiedFrame       (uint8_t index);
    void               observeReplay     (adk::PresenceInput& input);
    bool               decidePresence    (const adk::PresenceInput& input);
    bool               presentEvidence   ();
    void               stopReplaySafely  ();

} // namespace

void setup ()
{
    const bool presentationAcquired =
        acquisitionLed.initialize ().ok () && presenceLed.initialize ().ok ();

    const bool safeStateSelected = presentationAcquired && presenceLed.off ().ok ();

    const bool policyReady = safeStateSelected && presence.initialize ().ok ();

    const bool acquisitionShown = policyReady && acquisitionLed.on ().ok ();

    if (acquisitionShown)
    {
        evidenceCell = 0;
        qualityCell  = static_cast<uint8_t> (adk::PresenceQuality::Unqualified);
        rangeCell    = 0;
        replaying    = true;
    }
    else
    {
        stopReplaySafely ();
    }
}

void loop ()
{
    if (!replaying)
    {
        return;
    }

    adk::PresenceInput input;

    observeReplay (input);

    const bool updated = decidePresence (input);

    if (!updated)
    {
        stopReplaySafely ();
        return;
    }
    if (!presentEvidence ())
    {
        stopReplaySafely ();
        return;
    }

    ++replayIndex;
    if (replayIndex == 4)
    {
        replaying = false;
    }
}

namespace {

    adk::PirObservation copiedPir (uint32_t now, adk::PirPhase phase)
    {
        return {1,
                adk::TimePoint (now),
                phase == adk::PirPhase::Motion ? adk::Level::High : adk::Level::Low,
                phase,
                false,
                false,
                adk::Duration (20),
                adk::Status   ()};
    }

    adk::BeamObservation copiedBeam (uint32_t now, bool interrupted,
                                     bool interruptionEvent, bool restorationEvent)
    {
        return {{2, 7, adk::TimePoint (now)},
                interrupted ? adk::Level::Low : adk::Level::High,
                interrupted,
                interruptionEvent,
                restorationEvent,
                adk::Duration (20),
                adk::OpticalQuality::Valid,
                adk::Status ()};
    }

    adk::ReflectiveObservation copiedGuard (uint32_t now, bool active,
                                             bool activationEvent,
                                             bool deactivationEvent)
    {
        return {{3, 4, adk::TimePoint (now)},
                static_cast<uint16_t> (active ? 700 : 200),
                100,
                900,
                static_cast<uint16_t> (active ? 750 : 125),
                active,
                activationEvent,
                deactivationEvent,
                adk::Duration (20),
                adk::OpticalQuality::Valid,
                adk::Status ()};
    }

    adk::TimedRangeEvidence copiedRange (uint32_t now, uint16_t distanceMm)
    {
        return {4,
                adk::TimePoint            (now - 1U),
                adk::TimePoint            (now),
                adk::MicrosecondTimePoint (1000U + now),
                adk::MicrosecondDuration  (600),
                {adk::RangeState::Valid, distanceMm,
                 adk::MicrosecondDuration (500), true},
                adk::Status ()};
    }

    adk::PresenceInput copiedFrame (uint8_t index)
    {
        const uint32_t now         = (static_cast<uint32_t> (index) + 1U) * 100U;
        const bool     active      = index == 2;
        const bool     interrupted = index == 2;
        const bool     entry       = index == 2;
        const bool     exit        = index == 3;
        const auto     phase =
            active ? adk::PirPhase::Motion : adk::PirPhase::ReadyClear;
        const uint16_t distance = active ? 250 : 800;

        return {adk::TimePoint (now),
                {true, copiedPir   (now, phase)},
                {true, copiedBeam  (now, interrupted, entry, exit)},
                {true, copiedGuard (now, active, entry, exit)},
                {true, copiedRange (now, distance)}};
    }

    void observeReplay (adk::PresenceInput& input)
    {
        input = copiedFrame (replayIndex);
    }

    bool decidePresence (const adk::PresenceInput& input)
    {
        return presence.update (input).ok ();
    }

    bool presentEvidence ()
    {
        const adk::PresenceSnapshot snapshot = presence.snapshot ();

        evidenceCell = static_cast<uint8_t> (
            (snapshot.pirEligible ? 1U : 0U) |
            (snapshot.beam.active ? 1U << 1 : 0U) |
            (snapshot.finishGuard.active ? 1U << 2 : 0U) |
            (snapshot.range.approachValid ? 1U << 3 : 0U) |
            (snapshot.passageEvent ? 1U << 4 : 0U) |
            (snapshot.disagreement ? 1U << 5 : 0U));
        qualityCell = static_cast<uint8_t> (snapshot.quality);
        rangeCell   = snapshot.range.evidence.reading.distanceMm;
        return presenceLed.set (snapshot.pirEligible || snapshot.beam.active ||
                                snapshot.finishGuard.active ||
                                snapshot.range.approachValid ||
                                snapshot.passageEvent)
            .ok ();
    }

    void stopReplaySafely ()
    {
        replaying   = false;
        evidenceCell = 0;
        qualityCell  = static_cast<uint8_t> (adk::PresenceQuality::SourceFault);
        rangeCell    = 0;

        presence.reset ();

        presenceLed.shutdown ();

        acquisitionLed.shutdown ();
    }

} // namespace
