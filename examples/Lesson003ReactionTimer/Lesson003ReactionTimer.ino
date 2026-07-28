// Mega 2560, USB 5 V: D8 drives the cue LED through 330 Ohm to GND.
// D22 connects a momentary button to GND; D13 is acquisition evidence.
// TP-CUE=D8, TP-BUTTON=D22, and TP-ACQ=D13, all measured relative to GND.
#include <Adk.h>

namespace {

    adk::Runtime runtime;

    const uint32_t acquisitionPulseMs = 250;
    const uint32_t shutdownLowMs       = 250;
    const uint32_t shutdownHoldMs      = 3000;

    const adk::ButtonConfig  triggerConfig (22, adk::Pull::Up, adk::Level::Low,
                                            adk::Duration (20));
    adk::ReactionTimerConfig reactionConfig;

    adk::Button triggerButton (runtime.resources (), triggerConfig);

    adk::ReactionTimer reactionTimer (reactionConfig);

    adk::MonoLed cueLed         (runtime.resources (), 8);
    adk::MonoLed acquisitionLed (runtime.resources (), LED_BUILTIN);

    bool           running       = false;
    bool           stopHoldActive = false;
    adk::TimePoint stopHoldStarted;

    bool initializeProject ();

    void observeButton (adk::TimePoint now);

    bool shutdownRequested (adk::TimePoint now);

    adk::Status decide (adk::TimePoint now);

    adk::Status showTimerState ();

    void stopSafely ();
} // namespace

void setup ()
{
    running = initializeProject ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    observeButton (now);

    if (shutdownRequested (now))
    {
        stopSafely ();
        return;
    }

    const adk::Status decision = decide (now);

    if (!decision.ok ())
    {
        stopSafely ();
        return;
    }

    const adk::Status output = showTimerState ();

    if (!output.ok ())
    {
        stopSafely ();
    }
}

namespace {

    bool initializeProject ()
    {
        bool ready = triggerButton.initialize ().ok ();

        ready = reactionTimer.initialize ().ok () && ready;

        ready = cueLed.initialize ().ok () && ready;

        ready = acquisitionLed.initialize ().ok () && ready;

        if (!ready)
        {
            stopSafely ();
            return false;
        }

        if (!acquisitionLed.on ().ok ())
        {
            stopSafely ();
            return false;
        }

        delay (acquisitionPulseMs);

        if (!acquisitionLed.off ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    void observeButton (adk::TimePoint now)
    {
        triggerButton.update (now);
    }

    bool shutdownRequested (adk::TimePoint now)
    {
        if (!triggerButton.pressed ())
        {
            stopHoldActive = false;
            return false;
        }

        if (!stopHoldActive)
        {
            stopHoldActive  = true;
            stopHoldStarted = now;
            return false;
        }

        return now.elapsedSince (stopHoldStarted).milliseconds () >= shutdownHoldMs;
    }

    adk::Status decide (adk::TimePoint now)
    {
        return reactionTimer.update (now, triggerButton);
    }

    adk::Status showTimerState ()
    {
        // The cue LED also exposes timer state without Serial.
        return cueLed.set (reactionTimer.snapshot ().ledOn);
    }

    void stopSafely ()
    {
        cueLed.off         ();
        acquisitionLed.off ();
        delay              (shutdownLowMs);

        acquisitionLed.shutdown ();
        cueLed        .shutdown ();

        triggerButton.shutdown ();
        running        = false;
        stopHoldActive = false;
    }
} // namespace
