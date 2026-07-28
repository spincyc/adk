// Mega 2560, USB 5 V only after exact specimen qualification. Connect the
// qualified Linear Hall output to A0/TP-H and the qualified contact output to
// D22/TP-R. Connect D30, D31, and D32 to LEDs through separate 1 kOhm
// resistors. The sketch makes no claim about an unidentified module pinout,
// output topology, or magnetic stimulus. Bench acceptance remains open.
#include <Adk.h>
#include <magnetic_observation.h>

namespace {

    constexpr adk::PinId hallPin              = 54; // A0 on the Mega 2560.
    constexpr adk::PinId contactPin           = 22;
    constexpr adk::PinId rangeEvidencePin     = 30;
    constexpr adk::PinId contactEvidencePin   = 31;
    constexpr adk::PinId readyFaultPin         = 32;
    constexpr uint32_t   observationDurationMs = 120000;

    const adk::LinearHallConfig hallConfig = {
        hallPin, 100, 900, 300, 400, 600, 700, adk::Duration (20), false};
    const adk::MagneticContactConfig contactConfig = {
        contactPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)};

    adk::Runtime         runtime;
    adk::LinearHall      hall            (runtime.resources (), hallConfig);
    adk::MagneticContact contact         (runtime.resources (), contactConfig);
    adk::MonoLed         rangeEvidence   (
        runtime.resources (), rangeEvidencePin);
    adk::MonoLed         contactEvidence (
        runtime.resources (), contactEvidencePin);
    adk::MonoLed         readyFault      (runtime.resources (), readyFaultPin);

    adk::TimePoint startedAt;
    bool           running = false;

    bool acquireObservationCircuit ();
    void configureObservation      (adk::TimePoint now);
    bool startObservation          ();
    void observeMagneticInputs     (adk::TimePoint now);
    bool decideEvidence            ();
    bool actuateEvidence           ();
    void stopSafely                ();

} // namespace

void setup ()
{
    const adk::TimePoint now (millis ());

    if (acquireObservationCircuit ())
    {
        configureObservation (now);

        running = startObservation ();
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

    if (now.elapsedSince (startedAt).milliseconds () >= observationDurationMs)
    {
        stopSafely ();
        return;
    }

    observeMagneticInputs (now);

    if (!decideEvidence () || !actuateEvidence ())
    {
        stopSafely ();
    }
}

namespace {

    bool acquireObservationCircuit ()
    {
        if (!hall.initialize ().ok ())
        {
            return false;
        }

        if (!contact.initialize ().ok ())
        {
            hall.shutdown ();
            return false;
        }

        if (!rangeEvidence.initialize ().ok ())
        {
            contact.shutdown ();
            hall.shutdown    ();
            return false;
        }

        if (!contactEvidence.initialize ().ok ())
        {
            rangeEvidence.shutdown ();
            contact.shutdown       ();
            hall.shutdown          ();
            return false;
        }

        if (!readyFault.initialize ().ok ())
        {
            contactEvidence.shutdown ();
            rangeEvidence.shutdown   ();
            contact.shutdown         ();
            hall.shutdown            ();
            return false;
        }

        return true;
    }

    void configureObservation (adk::TimePoint now)
    {
        startedAt = now;
    }

    bool startObservation ()
    {
        return rangeEvidence.off   ().ok ()
            && contactEvidence.off ().ok ()
            && readyFault.on       ().ok ();
    }

    void observeMagneticInputs (adk::TimePoint now)
    {
        hall.update    (now);
        contact.update (now);
    }

    bool decideEvidence ()
    {
        return hall.snapshot    ().status.ok ()
            && contact.snapshot ().status.ok ();
    }

    bool actuateEvidence ()
    {
        const adk::MagneticObservation hallObservation    = hall.snapshot    ();
        const adk::MagneticObservation contactObservation = contact.snapshot ();

        const bool rangeIsQualified =
            hallObservation.quality == adk::MagneticQuality::Valid;

        return rangeEvidence.set   (rangeIsQualified).ok ()
            && contactEvidence.set (contactObservation.active).ok ()

            && readyFault.set       (rangeIsQualified).ok ();
    }

    void stopSafely ()
    {
        readyFault.shutdown       ();

        contactEvidence.shutdown ();

        rangeEvidence.shutdown   ();

        contact.shutdown         ();

        hall.shutdown            ();
        running = false;
    }

} // namespace
