// Mega 2560 reference fixture. Use only a visibly identified C&K/Littelfuse
// RB-220-07A R. Connect Mega 5 V through an external 10 kOhm resistor to
// TP-C/D22, and connect the switch from TP-C to GND. D30, D31, and D32 each
// drive an LED through 1 kOhm to GND: raw closure, a short accepted-attack
// witness, and ready/fault evidence. The switch terminals are nonpolar, but its
// installed closure orientation still requires incoming inspection. USB power
// and physical acceptance remain open.
#include <Adk.h>
#include <contact_dynamics.h>

namespace {

    constexpr adk::PinId contactPin            = 22;
    constexpr adk::PinId rawEvidencePin        = 30;
    constexpr adk::PinId acceptedEvidencePin   = 31;
    constexpr adk::PinId readyFaultEvidencePin = 32;
    constexpr uint32_t   attackWitnessMs       = 150;
    constexpr uint32_t   observationDurationMs = 120000;

    const adk::ContactDynamicsConfig
        dynamicsConfig (adk::Level::Low, adk::Duration (30), adk::Duration (20),
                        adk::Duration (80), adk::Duration (2000));

    adk::Runtime      runtime;
    adk::DigitalInput    contactInput       (runtime.resources (), contactPin,
                                             adk::Pull::None);
    adk::ContactDynamics dynamics           (dynamicsConfig);
    adk::MonoLed         rawEvidence        (runtime.resources (), rawEvidencePin);
    adk::MonoLed         acceptedEvidence   (runtime.resources (), acceptedEvidencePin);
    adk::MonoLed         readyFaultEvidence (runtime.resources (), readyFaultEvidencePin);

    adk::TimePoint     startedAt;
    adk::TimePoint     attackObservedAt;
    adk::ContactSample pendingSample     = {adk::TimePoint (), adk::Level::High,
                                            adk::StatusCode::Ok};
    bool               attackWitnessOpen = false;
    bool               rawActive         = false;
    bool               ready             = false;
    bool               running           = false;

    bool acquireContactCircuit   ();
    void configureContact        (adk::TimePoint now);
    bool startObservation        ();
    bool observeContact          (adk::TimePoint now);
    bool qualifyDynamics         ();
    bool presentContactEvidence  (adk::TimePoint now);
    bool actuateEvidence         ();
    void stopSafely              ();

} // namespace

void setup ()
{
    const adk::TimePoint now (millis ());

    if (acquireContactCircuit ())
    {
        configureContact (now);

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

    if (now.elapsedSince (startedAt).milliseconds () >= observationDurationMs ||
        !observeContact         (now) || !qualifyDynamics () ||
        !presentContactEvidence (now) || !actuateEvidence ())
    {
        stopSafely ();
    }
}

namespace {

    bool acquireContactCircuit ()
    {
        if (!contactInput.initialize       ().ok () || !dynamics.initialize ().ok () ||
            !rawEvidence.initialize        ().ok () ||
            !acceptedEvidence.initialize   ().ok () ||
            !readyFaultEvidence.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    void configureContact (adk::TimePoint now)
    {
        startedAt         = now;
        attackObservedAt  = now;
        attackWitnessOpen = false;
    }

    bool startObservation ()
    {
        return rawEvidence.off ().ok () && acceptedEvidence.off ().ok () &&
               readyFaultEvidence.on ().ok ();
    }

    bool observeContact (adk::TimePoint now)
    {
        contactInput.update ();

        pendingSample = {now, contactInput.read (), adk::StatusCode::Ok};
        return true;
    }

    bool qualifyDynamics ()
    {
        return dynamics.update (pendingSample).ok ();
    }

    bool presentContactEvidence (adk::TimePoint now)
    {
        const adk::ContactObservation observation = dynamics.snapshot ();

        if (!observation.status.ok ())
        {
            return false;
        }

        if (observation.attackEvent &&
            observation.disposition == adk::ContactDisposition::Accepted)
        {
            attackObservedAt  = now;
            attackWitnessOpen = true;
        }
        else if (attackWitnessOpen &&
                 now.elapsedSince (attackObservedAt).milliseconds () >= attackWitnessMs)
        {
            attackWitnessOpen = false;
        }

        rawActive = observation.rawActive;
        ready     = observation.quality == adk::ContactQuality::Valid;
        return true;
    }

    bool actuateEvidence ()
    {
        return rawEvidence.set        (rawActive).ok () &&
               acceptedEvidence.set   (attackWitnessOpen).ok () &&
               readyFaultEvidence.set (ready).ok ();
    }

    void stopSafely ()
    {
        readyFaultEvidence.shutdown ();
        acceptedEvidence.shutdown   ();
        rawEvidence.shutdown        ();
        contactInput.shutdown       ();
        running = false;
    }

} // namespace
