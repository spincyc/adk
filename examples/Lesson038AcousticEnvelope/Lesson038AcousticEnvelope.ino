// Reference fixture: SparkFun SEN-12642 Sound Detector, silkscreen V10.
// With USB as the only 5 V source, connect VCC to Mega 5V, GND to Mega GND,
// ENVELOPE to A0/TP-E, and active-high GATE to D22/TP-T. Leave AUDIO open.
// The Mega and detector must always share the same powered/unpowered state.
// D30, D31, and D32 each drive an LED through 1 kOhm to GND.
//
// The checked-in default uses a deterministic copied trace, so no detector is
// required. Set useReferenceModule only for the exact V10 reference fixture
// after unpowered identity and wiring checks. Physical acceptance remains open.
#include <Adk.h>
#include <acoustic_envelope.h>

namespace {

    constexpr bool       useReferenceModule = false;
    constexpr adk::PinId envelopePin        = 54; // Mega A0.
    constexpr adk::PinId gatePin            = 22;
    constexpr uint32_t   fixtureStepMs       = 20;
    constexpr uint32_t   presentationStepMs  = 250;
    constexpr uint32_t   evidenceDurationMs  = 120000;

    struct FixtureFrame
    {
        uint16_t envelope;
        bool     gate;
    };

    const FixtureFrame fixture[] = {
        {512, false}, {512, false}, {513, false}, {512, false},
        {511, false}, {512, false}, {600, true},  {650, true},
        {548, false}, {530, false}, {520, false}, {514, false},
        {512, false}, {512, false}, {512, false}, {512, false}};

    constexpr uint8_t fixtureCount =
        static_cast<uint8_t> (sizeof (fixture) / sizeof (fixture[0]));

    const adk::AcousticEnvelopeConfig envelopeConfig (
        true, adk::Level::High, 16, 80, 30, 3,
        adk::Duration (100), adk::Duration (300), adk::Duration (40),
        adk::Duration (200));

    adk::Runtime runtime;

    adk::AnalogInput      envelopeInput (runtime.resources (), envelopePin);
    adk::DigitalInput     gateInput     (
        runtime.resources (), gatePin, adk::Pull::None);
    adk::AcousticEnvelope envelope      (envelopeConfig);
    adk::MonoLed          eventEvidence (runtime.resources (), 30);
    adk::MonoLed          gateEvidence  (runtime.resources (), 31);
    adk::MonoLed          readyFault    (runtime.resources (), 32);

    uint32_t fixtureTimeMs = 0;
    uint32_t nextStepAtMs  = 0;
    uint32_t startedAtMs   = 0;
    uint8_t  fixtureIndex  = 0;
    bool     running       = false;

    bool                acquireEvidencePanel     ();
    bool                configureEnvelope        ();
    bool                startLesson              ();
    adk::AcousticSample observeAcousticEvidence  ();
    bool                decideEnvelope           (const adk::AcousticSample& sample);
    bool                actuateEvidence          ();
    void                stopSafely               ();

} // namespace

void setup ()
{
    if (acquireEvidencePanel () && configureEnvelope ())
    {
        running = startLesson ();
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

    const uint32_t nowMs = millis ();

    if (nowMs - startedAtMs >= evidenceDurationMs)
    {
        stopSafely ();
        return;
    }

    if (nowMs - nextStepAtMs < presentationStepMs)
    {
        return;
    }

    nextStepAtMs += presentationStepMs;

    const adk::AcousticSample sample = observeAcousticEvidence ();

    if (!decideEnvelope (sample) || !actuateEvidence ())
    {
        stopSafely ();
    }
}

namespace {

    bool acquireEvidencePanel ()
    {
        if (useReferenceModule && !envelopeInput.initialize ().ok ())
        {
            return false;
        }

        if (useReferenceModule && !gateInput.initialize ().ok ())
        {
            envelopeInput.shutdown ();
            return false;
        }

        if (!eventEvidence.initialize ().ok ())
        {
            gateInput.shutdown ();

            envelopeInput.shutdown ();

            return false;
        }

        if (!gateEvidence.initialize ().ok ())
        {
            eventEvidence.shutdown ();

            gateInput.shutdown ();

            envelopeInput.shutdown ();

            return false;
        }

        if (!readyFault.initialize ().ok ())
        {
            gateEvidence.shutdown ();

            eventEvidence.shutdown ();

            gateInput.shutdown ();

            envelopeInput.shutdown ();

            return false;
        }

        return true;
    }

    bool configureEnvelope ()
    {
        if (!envelope.initialize ().ok ())
        {
            return false;
        }

        if (useReferenceModule)
        {
            analogReference (DEFAULT);
        }

        if (!eventEvidence.off ().ok () || !gateEvidence.off ().ok ())
        {
            return false;
        }

        // D32 lights only after every acquisition and initialization succeeds.
        return readyFault.on ().ok ();
    }

    bool startLesson ()
    {
        fixtureIndex  = 0;
        fixtureTimeMs = 0;
        startedAtMs   = millis ();
        nextStepAtMs  = startedAtMs;
        return true;
    }

    adk::AcousticSample observeAcousticEvidence ()
    {
        uint16_t raw  = 0;
        bool     gate = false;
        uint32_t observedAtMs;

        if (useReferenceModule)
        {
            envelopeInput.update ();

            gateInput.update ();

            raw  = envelopeInput.sample ();

            gate = gateInput.sample () == adk::Level::High;

            observedAtMs = millis () - startedAtMs;
        }
        else
        {
            raw  = fixture[fixtureIndex].envelope;
            gate = fixture[fixtureIndex].gate;
            fixtureIndex =
                static_cast<uint8_t> ((fixtureIndex + 1U) % fixtureCount);

            observedAtMs = fixtureTimeMs;
            fixtureTimeMs += fixtureStepMs;
        }

        const adk::AcousticSample sample = {
            adk::TimePoint (observedAtMs),
            raw,
            true,
            gate ? adk::Level::High : adk::Level::Low,
            adk::StatusCode::Ok,
            adk::StatusCode::Ok};

        return sample;
    }

    bool decideEnvelope (const adk::AcousticSample& sample)
    {
        return envelope.update (sample).ok ();
    }

    bool actuateEvidence ()
    {
        const adk::AcousticObservation observation = envelope.snapshot ();
        const bool healthy = observation.phase != adk::AcousticPhase::Fault;

        return eventEvidence.set   (observation.eventCompleted).ok () &&
               gateEvidence.set    (observation.rawThresholdActive).ok () &&
               readyFault.set      (healthy).ok ();
    }

    void stopSafely ()
    {
        running = false;

        readyFault.shutdown ();

        gateEvidence.shutdown ();

        eventEvidence.shutdown ();

        gateInput.shutdown ();

        envelopeInput.shutdown ();

        envelope.reset ();
    }

} // namespace
