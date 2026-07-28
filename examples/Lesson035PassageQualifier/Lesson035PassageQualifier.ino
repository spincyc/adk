// Mega 2560, USB 5 V only. D30, D31, and D32 each drive an LED through
// a separate 1 kOhm resistor to GND. TP-A and TP-B are deterministic software
// fixture boundaries, not electrical sensor inputs. Do not connect a Linear
// Hall or Magnetic Spring specimen until its exact pinout, supply, output
// topology, logic levels, polarity, and retained magnetic stimulus have been
// qualified. This sketch proves passage policy only; physical acceptance
// remains open.
#include <Adk.h>
#include <passage_qualifier.h>

namespace {

    constexpr adk::PinId acceptedEvidencePin  = 30;
    constexpr adk::PinId directionEvidencePin = 31;
    constexpr adk::PinId readyFaultPin        = 32;
    constexpr uint32_t   qualifierStepMs      = 10;
    constexpr uint32_t   presentationStepMs   = 250;
    constexpr uint32_t   evidenceDurationMs   = 120000;

    struct FixtureFrame
    {
        bool    boundaryA;
        bool    boundaryB;
        int32_t position;
    };

    const FixtureFrame fixture[] = {
        {false, false, 0}, {true, false, 0},  {true, false, 0}, {true, false, 0},
        {true, true, 1},   {true, true, 2},   {true, true, 3},  {false, false, 3},
        {false, false, 3}, {false, false, 3}, {false, true, 3}, {false, true, 3},
        {false, true, 3},  {true, true, 2},   {true, true, 1},  {true, true, 0},
        {false, false, 0}, {false, false, 0}, {false, false, 0}};

    constexpr uint8_t fixtureCount =
        static_cast<uint8_t> (sizeof (fixture) / sizeof (fixture[0]));

    const adk::PassageQualifierConfig qualifierConfig = {
        adk::Duration (20), adk::Duration (80), adk::Duration (20)};

    adk::Runtime runtime;

    adk::PassageQualifier qualifier (qualifierConfig);

    adk::MonoLed acceptedEvidence (runtime.resources (), acceptedEvidencePin);

    adk::MonoLed directionEvidence (runtime.resources (), directionEvidencePin);

    adk::MonoLed readyFault (runtime.resources (), readyFaultPin);

    adk::PassageInput    observedPassage;
    adk::PassageSnapshot decidedPassage;
    uint32_t             fixtureTimeMs = 0;
    uint32_t             nextStepAtMs  = 0;
    uint32_t             startedAtMs   = 0;
    uint8_t              fixtureIndex  = 0;
    bool                 running       = false;

    bool acquireEvidencePanel ();

    bool configureQualifier ();

    bool startFixture ();

    void observeFixture ();

    bool decidePassage ();

    bool actuateEvidence ();

    adk::MagneticObservation qualifiedObservation (bool           active,
                                                   adk::TimePoint observedAt);

    void stopSafely ();

} // namespace

void setup ()
{
    if (acquireEvidencePanel () && configureQualifier ())
    {
        running = startFixture ();
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

    observeFixture ();

    if (!decidePassage () || !actuateEvidence ())
    {
        stopSafely ();
    }
}

namespace {

    bool acquireEvidencePanel ()
    {
        if (!acceptedEvidence.initialize ().ok ())
        {
            return false;
        }

        if (!directionEvidence.initialize ().ok ())
        {
            acceptedEvidence.shutdown ();
            return false;
        }

        if (!readyFault.initialize ().ok ())
        {
            directionEvidence.shutdown ();

            acceptedEvidence.shutdown ();
            return false;
        }

        return true;
    }

    bool configureQualifier ()
    {
        if (!qualifier.initialize ().ok ())
        {
            return false;
        }

        return acceptedEvidence.off ().ok () && directionEvidence.off ().ok () &&
               readyFault.on ().ok ();
    }

    bool startFixture ()
    {
        fixtureIndex  = 0;
        fixtureTimeMs = 0;

        startedAtMs  = millis ();
        nextStepAtMs = startedAtMs;
        return true;
    }

    void observeFixture ()
    {
        const FixtureFrame& frame = fixture[fixtureIndex];

        const adk::TimePoint observedAt (fixtureTimeMs);

        observedPassage = {observedAt,
                           qualifiedObservation (frame.boundaryA, observedAt),
                           qualifiedObservation (frame.boundaryB, observedAt),
                           true,
                           frame.position,
                           adk::StatusCode::Ok};

        fixtureTimeMs += qualifierStepMs;

        fixtureIndex = static_cast<uint8_t> ((fixtureIndex + 1U) % fixtureCount);
    }

    bool decidePassage ()
    {
        qualifier.update (observedPassage);

        decidedPassage = qualifier.snapshot ();

        return decidedPassage.status.ok () ||
               decidedPassage.status.error () == adk::StatusCode::Timeout;
    }

    bool actuateEvidence ()
    {
        const bool accepted =
            decidedPassage.hasRecord &&
            decidedPassage.record.disposition == adk::PassageDisposition::Accepted;
        const bool aToB =
            accepted && decidedPassage.record.direction == adk::PassageDirection::AToB;
        const bool healthy = decidedPassage.phase != adk::PassagePhase::Fault;

        return acceptedEvidence.set (accepted).ok () &&
               directionEvidence.set (aToB).ok () && readyFault.set (healthy).ok ();
    }

    adk::MagneticObservation qualifiedObservation (bool           active,
                                                   adk::TimePoint observedAt)
    {
        return {adk::MagneticSource::ContactDigital,
                static_cast<uint16_t> (active ? 1 : 0),
                active ? adk::Level::High : adk::Level::Low,
                observedAt,
                adk::MagneticPolarity::Unspecified,
                false,
                false,
                active,
                adk::Duration (),
                adk::MagneticQuality::Valid,
                adk::StatusCode::Ok};
    }

    void stopSafely ()
    {
        running = false;

        readyFault.shutdown ();

        directionEvidence.shutdown ();

        acceptedEvidence.shutdown ();

        qualifier.reset ();
    }

} // namespace
