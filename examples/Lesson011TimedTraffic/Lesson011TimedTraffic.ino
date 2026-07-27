#include <Adk.h>

// Mega 2560, USB 5 V: D30-D35 vehicle LEDs, D36-D37 pedestrian LEDs,
// and D13 ready LED, each through 330 Ohm; D22 button to GND uses INPUT_PULLUP.

namespace {

    adk::Runtime runtime;

    const adk::ButtonConfig pedestrianButtonConfig (22);
    adk::Button             pedestrianButton       (runtime.resources (),
                                                     pedestrianButtonConfig);

    adk::MonoLed mainRed    (runtime.resources (), 30);
    adk::MonoLed mainYellow (runtime.resources (), 31);
    adk::MonoLed mainGreen  (runtime.resources (), 32);
    adk::MonoLed sideRed    (runtime.resources (), 33);
    adk::MonoLed sideYellow (runtime.resources (), 34);
    adk::MonoLed sideGreen  (runtime.resources (), 35);
    adk::MonoLed walk       (runtime.resources (), 36);
    adk::MonoLed stop       (runtime.resources (), 37);
    adk::MonoLed ready      (runtime.resources (), LED_BUILTIN);

    adk::TrafficConfig  trafficConfig;
    adk::TrafficJunction traffic (trafficConfig);

    bool running = false;

    bool              initializeCircuit  ();
    adk::TrafficInput observeRequest     (adk::TimePoint now);
    adk::Status       decideSignals      (adk::TimePoint now,
                                          const adk::TrafficInput& observation);
    bool              showSignals        (const adk::TrafficSignals& signals);
    void              stopSafely         ();
} // namespace

void setup ()
{
    running = initializeCircuit ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    const adk::TrafficInput observation = observeRequest (now);
    const adk::Status       decision    = decideSignals  (now, observation);
    const bool              signalsShown = showSignals   (traffic.snapshot ().signals);

    if (!signalsShown)
    {
        stopSafely ();
        return;
    }

    if (decision != adk::Status::Ok)
    {
        ready.off ();
    }
}

namespace {

    bool initializeCircuit ()
    {
        if (pedestrianButton.initialize () != adk::Status::Ok ||
            mainRed.initialize          () != adk::Status::Ok ||
            mainYellow.initialize       () != adk::Status::Ok ||
            mainGreen.initialize        () != adk::Status::Ok ||
            sideRed.initialize          () != adk::Status::Ok ||
            sideYellow.initialize       () != adk::Status::Ok ||
            sideGreen.initialize        () != adk::Status::Ok ||
            walk.initialize             () != adk::Status::Ok ||
            stop.initialize             () != adk::Status::Ok ||
            ready.initialize            () != adk::Status::Ok ||
            traffic.initialize          () != adk::Status::Ok)
        {
            stopSafely ();
            return false;
        }

        if (ready.on () != adk::Status::Ok)
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    adk::TrafficInput observeRequest (adk::TimePoint now)
    {
        pedestrianButton.update (now);

        return adk::TrafficInput (pedestrianButton.pressEvent (), true);
    }

    adk::Status decideSignals (adk::TimePoint             now,
                                const adk::TrafficInput& observation)
    {
        return traffic.update (now, observation);
    }

    bool showSignals (const adk::TrafficSignals& signals)
    {
        return mainRed.set    (signals.mainRed)        == adk::Status::Ok &&
               mainYellow.set (signals.mainYellow)     == adk::Status::Ok &&
               mainGreen.set  (signals.mainGreen)      == adk::Status::Ok &&
               sideRed.set    (signals.sideRed)        == adk::Status::Ok &&
               sideYellow.set (signals.sideYellow)     == adk::Status::Ok &&
               sideGreen.set  (signals.sideGreen)      == adk::Status::Ok &&
               walk.set       (signals.pedestrianWalk) == adk::Status::Ok &&
               stop.set       (signals.pedestrianStop) == adk::Status::Ok;
    }

    void stopSafely ()
    {
        ready.shutdown      ();
        stop.shutdown       ();
        walk.shutdown       ();
        sideGreen.shutdown  ();
        sideYellow.shutdown ();
        sideRed.shutdown    ();
        mainGreen.shutdown  ();
        mainYellow.shutdown ();
        mainRed.shutdown    ();

        pedestrianButton.shutdown ();
        running = false;
    }
} // namespace
