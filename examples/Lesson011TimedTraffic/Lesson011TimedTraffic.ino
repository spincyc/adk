#include <Adk.h>

// Mega 2560, USB 5 V: D30-D35 vehicle LEDs and D36-D37 pedestrian LEDs,
// each through 330 Ohm. D13 is the built-in LED; D22 to GND uses INPUT_PULLUP.

namespace {

    adk::Runtime runtime;

    const adk::ButtonConfig pedestrianButtonConfig (22);
    adk::Button             pedestrianButton       (runtime.resources (),
                                                     pedestrianButtonConfig);

    adk::MonoLed mainRed        (runtime.resources (), 30);
    adk::MonoLed mainYellow     (runtime.resources (), 31);
    adk::MonoLed mainGreen      (runtime.resources (), 32);
    adk::MonoLed sideRed        (runtime.resources (), 33);
    adk::MonoLed sideYellow     (runtime.resources (), 34);
    adk::MonoLed sideGreen      (runtime.resources (), 35);
    adk::MonoLed walk           (runtime.resources (), 36);
    adk::MonoLed stop           (runtime.resources (), 37);
    adk::MonoLed acquisitionLed (runtime.resources (), LED_BUILTIN);

    adk::TrafficConfig  trafficConfig;
    adk::TrafficJunction traffic (trafficConfig);

    enum class DemoStage
    {
        Acquiring,
        Separating,
        Running,
        AllRed,
        Halted
    };

    const adk::Duration acquisitionPulseDuration (250);
    const adk::Duration acquisitionSeparator     (750);
    const adk::Duration demoDuration             (120000);
    const adk::Duration allRedDuration           (1000);

    DemoStage      demoStage    = DemoStage::Halted;
    adk::TimePoint stageStarted;

    bool              initializeCircuit  ();
    void              advanceDemo        (adk::TimePoint now);
    adk::TrafficInput observeRequest     (adk::TimePoint now);
    adk::Status       decideSignals      (adk::TimePoint now,
                                          const adk::TrafficInput& observation);
    bool              showSignals        (const adk::TrafficSignals& signals);
    void              setSignal          (adk::MonoLed& signal, bool on,
                                          bool& succeeded);
    void              stopSafely         ();
} // namespace

void setup ()
{
    if (initializeCircuit ())
    {
        stageStarted = adk::TimePoint (millis ());
        demoStage    = DemoStage::Acquiring;
    }
}

void loop ()
{
    if (demoStage == DemoStage::Halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());
    advanceDemo              (now);

    if (demoStage != DemoStage::Running)
    {
        return;
    }

    const adk::TrafficInput observation = observeRequest (now);
    const adk::Status       decision    = decideSignals  (now, observation);
    const bool              signalsShown = showSignals   (traffic.snapshot ().signals);

    if (!signalsShown)
    {
        stopSafely ();
        return;
    }

    if (!decision.ok ())
    {
        acquisitionLed.off ();
    }
}

namespace {

    bool initializeCircuit ()
    {
        if (!pedestrianButton.initialize ().ok () ||
            !mainRed.initialize          ().ok () ||
            !mainYellow.initialize       ().ok () ||
            !mainGreen.initialize        ().ok () ||
            !sideRed.initialize          ().ok () ||
            !sideYellow.initialize       ().ok () ||
            !sideGreen.initialize        ().ok () ||
            !walk.initialize             ().ok () ||
            !stop.initialize             ().ok () ||
            !acquisitionLed.initialize   ().ok () ||
            !traffic.initialize          ().ok ())
        {
            stopSafely ();
            return false;
        }

        if (!acquisitionLed.on ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    void advanceDemo (adk::TimePoint now)
    {
        const adk::Duration elapsed = now.elapsedSince (stageStarted);

        if (demoStage == DemoStage::Acquiring &&
            elapsed >= acquisitionPulseDuration)
        {
            if (!acquisitionLed.off ().ok ())
            {
                stopSafely ();
                return;
            }

            stageStarted = now;
            demoStage    = DemoStage::Separating;
        }
        else if (demoStage == DemoStage::Separating &&
                 elapsed >= acquisitionSeparator)
        {
            stageStarted = now;
            demoStage    = DemoStage::Running;
        }
        else if (demoStage == DemoStage::Running &&
                 elapsed >= demoDuration)
        {
            const adk::TrafficSignals allRed = {
                true, false, false,
                true, false, false,
                true, false
            };

            if (!showSignals (allRed))
            {
                stopSafely ();
                return;
            }

            stageStarted = now;
            demoStage    = DemoStage::AllRed;
        }
        else if (demoStage == DemoStage::AllRed &&
                 elapsed >= allRedDuration)
        {
            stopSafely ();
        }
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
        bool safe = true;

        setSignal (mainGreen,  false, safe);
        setSignal (mainYellow, false, safe);
        setSignal (sideGreen,  false, safe);
        setSignal (sideYellow, false, safe);
        setSignal (walk,       false, safe);
        setSignal (mainRed,     true, safe);
        setSignal (sideRed,     true, safe);
        setSignal (stop,        true, safe);

        if (!safe)
        {
            return false;
        }

        bool shown = true;
        setSignal (mainYellow, signals.mainYellow, shown);
        setSignal (mainGreen,  signals.mainGreen, shown);
        setSignal (sideYellow, signals.sideYellow, shown);
        setSignal (sideGreen,  signals.sideGreen, shown);
        setSignal (walk,       signals.pedestrianWalk, shown);

        if (!shown)
        {
            return false;
        }

        setSignal (mainRed, signals.mainRed, shown);
        setSignal (sideRed, signals.sideRed, shown);
        setSignal (stop,    signals.pedestrianStop, shown);
        return shown;
    }

    void setSignal (adk::MonoLed& signal, bool on, bool& succeeded)
    {
        if (!signal.set (on).ok ())
        {
            succeeded = false;
        }
    }

    void stopSafely ()
    {
        traffic       .shutdown ();
        acquisitionLed.shutdown ();
        stop          .shutdown ();
        walk          .shutdown ();
        sideGreen     .shutdown ();
        sideYellow    .shutdown ();
        sideRed       .shutdown ();
        mainGreen     .shutdown ();
        mainYellow    .shutdown ();
        mainRed       .shutdown ();

        pedestrianButton.shutdown ();
        demoStage = DemoStage::Halted;
    }
} // namespace
