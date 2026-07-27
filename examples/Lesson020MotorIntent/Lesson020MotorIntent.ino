// Mega 2560, USB logic only: D22/D23/D6 each drive an LED through 1 kOhm.
// This sketch simulates H-bridge inputs. Do not connect a motor or motor supply.
#include <Adk.h>
#include <motor_intent.h>
#include <power_domain.h>

namespace {

    constexpr adk::PinId forwardEvidencePin = 22;
    constexpr adk::PinId reverseEvidencePin = 23;
    constexpr adk::PinId enableEvidencePin  = 6;

    const adk::MotorIntentConfig motorConfig (adk::Duration (250), 180);

    adk::Runtime runtime;

    adk::MonoLed   forwardEvidence     (runtime.resources (), forwardEvidencePin);
    adk::MonoLed   reverseEvidence     (runtime.resources (), reverseEvidencePin);
    adk::PwmOutput enableEvidence      (runtime.resources (), enableEvidencePin);
    adk::MonoLed   acquisitionEvidence (runtime.resources (), LED_BUILTIN);

    adk::ExternalPowerDomainGate commandGate;
    adk::MotorIntent             motorIntent (motorConfig, commandGate);

    adk::TimePoint    scriptStarted;
    adk::MotorCommand observedRequest;
    uint8_t           scriptStep = 0;
    bool              running    = false;
    bool              requestDue = false;

    bool initializeSimulation ();

    void observeOperatorRequest (adk::TimePoint now);
    bool decideMotorIntent      (adk::TimePoint now);
    bool actuateMotorEvidence   ();

    bool writeMotorEvidence (const adk::MotorCommand& applied);
    void stopSafely         ();

} // namespace

void setup ()
{
    running = initializeSimulation ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    observeOperatorRequest (now);

    if (!decideMotorIntent (now) || !actuateMotorEvidence ())
    {
        stopSafely ();
    }
}

namespace {

    bool initializeSimulation ()
    {
        if (!forwardEvidence.initialize ().ok () ||
            !reverseEvidence.initialize     ().ok () ||
            !enableEvidence.initialize      ().ok () ||
            !acquisitionEvidence.initialize ().ok () ||
            !motorIntent.initialize         ().ok ())
        {
            stopSafely ();
            return false;
        }

        commandGate.admit ();

        scriptStarted = adk::TimePoint (millis ());

        if (!acquisitionEvidence.on ().ok () || !actuateMotorEvidence ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    void observeOperatorRequest (adk::TimePoint now)
    {
        const uint32_t elapsed = now.elapsedSince (scriptStarted).milliseconds ();

        requestDue = false;

        if (scriptStep == 0 && elapsed >= 1000)
        {
            observedRequest = adk::MotorCommand (adk::MotorDirection::Forward, 96);
            requestDue      = true;
            ++scriptStep;
        }
        else if (scriptStep == 1 && elapsed >= 3000)
        {
            observedRequest = adk::MotorCommand (adk::MotorDirection::Reverse, 160);
            requestDue      = true;
            ++scriptStep;
        }
        else if (scriptStep == 2 && elapsed >= 5000)
        {
            observedRequest = adk::MotorCommand ();
            requestDue      = true;
            ++scriptStep;
        }
    }

    bool decideMotorIntent (adk::TimePoint now)
    {
        if (requestDue && !motorIntent.command (observedRequest, now).ok ())
        {
            return false;
        }

        return motorIntent.update (now).ok ();
    }

    bool actuateMotorEvidence ()
    {
        const adk::MotorIntentSnapshot decision = motorIntent.snapshot ();

        if (!decision.status.ok ())
        {
            return false;
        }

        return writeMotorEvidence (decision.applied);
    }

    bool writeMotorEvidence (const adk::MotorCommand& applied)
    {
        if (!enableEvidence.write (0).ok ())
        {
            return false;
        }

        const bool forward = applied.direction == adk::MotorDirection::Forward;
        const bool reverse = applied.direction == adk::MotorDirection::Reverse;

        if (!forwardEvidence.set (forward).ok () ||
            !reverseEvidence.set (reverse).ok ())
        {
            return false;
        }

        return enableEvidence.write (applied.duty).ok ();
    }

    void stopSafely ()
    {
        motorIntent.stop     ();
        writeMotorEvidence   (adk::MotorCommand ());
        commandGate.revoke   ();
        motorIntent.shutdown ();

        acquisitionEvidence.shutdown ();
        enableEvidence     .shutdown ();
        reverseEvidence    .shutdown ();
        forwardEvidence    .shutdown ();

        running = false;
    }

} // namespace
