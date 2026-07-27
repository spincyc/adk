#include <Adk.h>

namespace {

    adk::Runtime runtime;

    const adk::ButtonConfig  triggerConfig (22, adk::Pull::Up, adk::Level::Low,
                                            adk::Duration (20));
    adk::ReactionTimerConfig reactionConfig;

    adk::Button triggerButton (runtime.resources (), triggerConfig);

    adk::ReactionTimer reactionTimer (reactionConfig);

    adk::MonoLed cueLed (runtime.resources (), LED_BUILTIN);

    bool running = false;

    bool initializeProject ();

    void observeButton (adk::TimePoint now);

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

        if (!ready)
        {
            stopSafely ();
        }

        return ready;
    }

    void observeButton (adk::TimePoint now)
    {
        triggerButton.update (now);
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
        cueLed.shutdown ();

        triggerButton.shutdown ();
        running = false;
    }
} // namespace
