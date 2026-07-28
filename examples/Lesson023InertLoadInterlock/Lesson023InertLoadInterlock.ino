// Mega 2560, USB only: D38/D40/D41 and D12 drive resistor-limited LEDs.
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

    constexpr uint32_t requestPulseMilliseconds = 180;
    constexpr uint32_t shutdownAtMilliseconds   = 15000;
    constexpr uint32_t inactiveHoldMilliseconds = 250;

    constexpr uint32_t stepTimesMilliseconds[] =
    {
        300, 1800, 3300, 4800, 6300, 8300, 9800, 11300, 12800
    };

    adk::Runtime            runtime;
    adk::ExternalPowerDomainGate admission;
    adk::InertLoadInterlock interlock (admission);

    adk::MonoLed requestEvidence (runtime.resources (), LED_BUILTIN);
    adk::MonoLed faultEvidence   (runtime.resources (), faultEvidencePin);

    adk::IndicatorPump  fanIndicator    (runtime.resources (), fanEvidencePin);
    adk::IndicatorPump  pumpIndicator   (runtime.resources (), pumpEvidencePin);
    adk::IndicatorPump  heaterIndicator (runtime.resources (), heaterEvidencePin);
    adk::InertLoadPanel panel           (fanIndicator, pumpIndicator, heaterIndicator);

    uint32_t startedAt        = 0;
    uint32_t requestStartedAt = 0;
    uint32_t shutdownStartedAt = 0;
    uint8_t  nextStep         = 0;
    bool     requestVisible   = false;
    bool     running          = false;
    bool     stopping         = false;

    bool acquireCircuit       ();
    bool clearRequestEvidence (uint32_t now);
    bool playStep             (uint8_t step, uint32_t now);
    bool selectVisibleIntent  (adk::SimulatedLoad load, uint32_t now);
    bool showFailClosed       (uint32_t now);
    bool recoverExplicitly    ();
    void observeFailure       ();
    void beginShutdown        (uint32_t now);
    void finishShutdown       ();

} // namespace

void setup ()
{
    running   = acquireCircuit ();
    startedAt = millis         ();

    if (!running)
    {
        beginShutdown (startedAt);
        return;
    }

    admission.admit ();
}

void loop ()
{
    const uint32_t now = millis ();

    if (stopping)
    {
        if (static_cast<uint32_t> (now - shutdownStartedAt) >=
            inactiveHoldMilliseconds)
        {
            finishShutdown ();
        }

        return;
    }

    if (!running)
    {
        return;
    }

    if (static_cast<uint32_t> (now - startedAt) >= shutdownAtMilliseconds)
    {
        beginShutdown (now);
        return;
    }

    if (!clearRequestEvidence (now))
    {
        beginShutdown (now);
        return;
    }

    if (nextStep >= sizeof (stepTimesMilliseconds) /
                    sizeof (stepTimesMilliseconds[0]) ||
        static_cast<uint32_t> (now - startedAt) <
            stepTimesMilliseconds[nextStep])
    {
        return;
    }

    if (!playStep (nextStep, now))
    {
        observeFailure ();
        beginShutdown  (now);
        return;
    }

    ++nextStep;
}

namespace {

    bool acquireCircuit ()
    {
        if (!requestEvidence.initialize ().ok ())
        {
            return false;
        }

        if (!faultEvidence.initialize ().ok () ||
            !interlock.initialize      ().ok () ||
            !panel.initialize          ().ok ())
        {
            return false;
        }

        return true;
    }

    bool clearRequestEvidence (uint32_t now)
    {
        if (!requestVisible ||
            static_cast<uint32_t> (now - requestStartedAt) <
                requestPulseMilliseconds)
        {
            return true;
        }

        requestVisible = false;
        return requestEvidence.off ().ok ();
    }

    bool playStep (uint8_t step, uint32_t now)
    {
        switch (step)
        {
            case 0:
                return selectVisibleIntent (adk::SimulatedLoad::Fan, now);
            case 1:
            case 6:
                return selectVisibleIntent (adk::SimulatedLoad::Pump, now);
            case 2:
            case 7:
                return selectVisibleIntent (adk::SimulatedLoad::Heater, now);
            case 3:
            case 8:
                return selectVisibleIntent (adk::SimulatedLoad::None, now);
            case 4:
                return showFailClosed (now);
            case 5:
                return recoverExplicitly () &&
                       selectVisibleIntent (adk::SimulatedLoad::Fan, now);
            default:
                return false;
        }
    }

    bool selectVisibleIntent (adk::SimulatedLoad load, uint32_t now)
    {
        const adk::Status requestStatus = requestEvidence.on ();

        requestVisible   = requestStatus.ok ();
        requestStartedAt = now;

        if (!requestStatus.ok () || !interlock.select (load).ok ())
        {
            return false;
        }

        const adk::Status            updateStatus = interlock.update   ();
        const adk::InertLoadSnapshot snapshot     = interlock.snapshot ();

        return updateStatus.ok      () &&
               snapshot.status.ok   () &&
               panel.select         (snapshot.active).ok ();
    }

    bool showFailClosed (uint32_t now)
    {
        admission.revoke ();

        const adk::Status requestStatus = requestEvidence.on ();

        requestVisible   = requestStatus.ok ();
        requestStartedAt = now;

        if (!requestStatus.ok ())
        {
            return false;
        }

        const adk::Status            selectStatus = interlock.select   (
            adk::SimulatedLoad::Fan);
        const adk::Status            updateStatus = interlock.update   ();
        const adk::InertLoadSnapshot snapshot     = interlock.snapshot ();

        if (selectStatus.ok () ||
            selectStatus.error    () != adk::StatusCode::HardwareFailure ||
            updateStatus.ok       () ||
            updateStatus.error    () != adk::StatusCode::HardwareFailure ||
            snapshot.status.ok    () ||
            snapshot.status.error () != adk::StatusCode::HardwareFailure ||
            snapshot.active != adk::SimulatedLoad::None ||
            !panel.select         (snapshot.active).ok () ||
            !faultEvidence.on     ().ok ())
        {
            return false;
        }

        return true;
    }

    bool recoverExplicitly ()
    {
        panel.shutdown     ();
        interlock.shutdown ();
        admission.admit    ();

        return interlock.initialize   ().ok () &&
               panel.initialize       ().ok () &&
               faultEvidence.off      ().ok ();
    }

    void observeFailure ()
    {
        faultEvidence.on ();
    }

    void beginShutdown (uint32_t now)
    {
        admission.revoke      ();
        interlock.shutdown    ();
        panel.shutdown        ();
        requestEvidence.off   ();
        faultEvidence.off     ();

        shutdownStartedAt = now;
        running           = false;
        stopping          = true;
    }

    void finishShutdown ()
    {
        requestEvidence.shutdown ();
        faultEvidence.shutdown   ();
        stopping = false;
    }

} // namespace
