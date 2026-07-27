// Mega 2560, USB only: D38/D40/D41 use 220R LEDs; D12 uses a 1k fault LED.
// D13 is the built-in request pulse. No relay or powered load is connected.
#include <inert_load_interlock.h>
#include <inert_load_panel.h>
#include <mono_led.h>
#include <power_domain.h>
#include <pump_output.h>
#include <runtime.h>

namespace {

    constexpr adk::PinId faultEvidencePin  = 12;
    constexpr adk::PinId pumpEvidencePin   = 38;
    constexpr adk::PinId fanEvidencePin    = 40;
    constexpr adk::PinId heaterEvidencePin = 41;

    constexpr uint32_t selectionIntervalMilliseconds = 2000;
    constexpr uint32_t requestPulseMilliseconds      = 200;

    adk::Runtime                 runtime;
    adk::ExternalPowerDomainGate admission;
    adk::InertLoadInterlock      interlock (admission);

    adk::MonoLed requestEvidence (runtime.resources (), LED_BUILTIN);
    adk::MonoLed faultEvidence   (runtime.resources (), faultEvidencePin);

    adk::IndicatorPump  fanIndicator    (runtime.resources (), fanEvidencePin);
    adk::IndicatorPump  pumpIndicator   (runtime.resources (), pumpEvidencePin);
    adk::IndicatorPump  heaterIndicator (runtime.resources (), heaterEvidencePin);
    adk::InertLoadPanel panel           (fanIndicator, pumpIndicator, heaterIndicator);

    adk::SimulatedLoad requestedLoad  = adk::SimulatedLoad::None;
    uint32_t           lastSelection  = 0;
    uint32_t           requestStarted = 0;
    bool               requestVisible = false;
    bool               faultTrial     = false;
    bool               running        = false;

    bool acquireCircuit       ();
    bool clearRequestEvidence (uint32_t now);
    bool requestNextIntent    (uint32_t now);
    bool applyAdmittedIntent  ();
    void observeFailure       (adk::Status status);
    void stopSafely           ();

} // namespace

void setup ()
{
    running = acquireCircuit ();

    if (!running)
    {
        stopSafely ();
        return;
    }

    admission.admit ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const uint32_t now = millis ();

    if (!clearRequestEvidence (now))
    {
        stopSafely ();
        return;
    }

    if (static_cast<uint32_t> (now - lastSelection) < selectionIntervalMilliseconds)
    {
        return;
    }

    if (!requestNextIntent (now) || !applyAdmittedIntent ())
    {
        stopSafely ();
    }
}

namespace {

    bool acquireCircuit ()
    {
        const adk::Status requestStatus = requestEvidence.initialize ();

        if (!requestStatus.ok ())
        {
            return false;
        }

        const adk::Status faultStatus = faultEvidence.initialize ();

        if (!faultStatus.ok ())
        {
            observeFailure (faultStatus);
            return false;
        }

        const adk::Status interlockStatus = interlock.initialize ();

        if (!interlockStatus.ok ())
        {
            observeFailure (interlockStatus);
            return false;
        }

        const adk::Status panelStatus = panel.initialize ();

        if (!panelStatus.ok ())
        {
            observeFailure (panelStatus);
            return false;
        }

        return true;
    }

    bool clearRequestEvidence (uint32_t now)
    {
        if (!requestVisible ||
            static_cast<uint32_t> (now - requestStarted) < requestPulseMilliseconds)
        {
            return true;
        }

        const adk::Status status = requestEvidence.off ();

        requestVisible = false;

        if (!status.ok ())
        {
            observeFailure (status);
            return false;
        }

        return true;
    }

    bool requestNextIntent (uint32_t now)
    {
        switch (requestedLoad)
        {
            case adk::SimulatedLoad::None:
                requestedLoad = adk::SimulatedLoad::Fan;
                break;
            case adk::SimulatedLoad::Fan:
                requestedLoad = adk::SimulatedLoad::Pump;
                break;
            case adk::SimulatedLoad::Pump:
                requestedLoad = adk::SimulatedLoad::Heater;
                break;
            case adk::SimulatedLoad::Heater:
                requestedLoad = adk::SimulatedLoad::None;
                faultTrial    = true;
                break;
        }

        if (faultTrial && requestedLoad == adk::SimulatedLoad::Fan)
        {
            admission.revoke ();
        }

        const adk::Status requestStatus = requestEvidence.on      ();
        const adk::Status selectStatus  = interlock.select        (requestedLoad);

        lastSelection  = now;
        requestStarted = now;
        requestVisible = requestStatus.ok ();

        if (!requestStatus.ok ())
        {
            observeFailure (requestStatus);
            return false;
        }

        if (!selectStatus.ok ())
        {
            observeFailure (selectStatus);
        }

        return true;
    }

    bool applyAdmittedIntent ()
    {
        const adk::Status            updateStatus = interlock.update   ();
        const adk::InertLoadSnapshot snapshot     = interlock.snapshot ();
        const adk::Status            status       = panel.select       (snapshot.active);

        if (!status.ok ())
        {
            observeFailure (status);
            return false;
        }

        if (!updateStatus.ok ())
        {
            observeFailure (updateStatus);
            return false;
        }

        return snapshot.status.ok ();
    }

    void observeFailure (adk::Status status)
    {
        (void)status;
        faultEvidence.on ();
    }

    void stopSafely ()
    {
        admission.revoke   ();
        interlock.shutdown ();
        panel.shutdown     ();

        requestEvidence.off      ();
        requestEvidence.shutdown ();

        if (!faultEvidence.on ().ok ())
        {
            faultEvidence.shutdown ();
        }

        running = false;
    }
} // namespace
