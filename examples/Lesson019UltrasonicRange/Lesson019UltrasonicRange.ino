#include <Adk.h>

// Mega 2560, USB 5 V: HC-SR04 Trigger D40, Echo D41/TP-E.
// D45, D46, and D47 each drive one LED through its own 220 ohm resistor.

namespace {

    constexpr adk::PinId triggerPin = 40;
    constexpr adk::PinId echoPin    = 41;
    constexpr adk::PinId validPin   = 45;
    constexpr adk::PinId outsidePin = 46;
    constexpr adk::PinId faultPin   = 47;

    const adk::UltrasonicRangerConfig rangeConfig =
    {
        adk::MicrosecondDuration (30000),
        adk::MicrosecondDuration (30000),
        20,
        4000,
        343
    };

    adk::Runtime          runtime;
    adk::DigitalOutput    trigger (runtime.resources (), triggerPin);
    adk::DigitalInput     echo    (runtime.resources (), echoPin);
    adk::MonoLed          valid   (runtime.resources (), validPin);
    adk::MonoLed          outside (runtime.resources (), outsidePin);
    adk::MonoLed          fault   (runtime.resources (), faultPin);
    adk::UltrasonicRanger ranger  (rangeConfig);

    enum struct RangeSignal : uint8_t
    {
        Waiting,
        Valid,
        Outside,
        Fault
    };

    adk::MicrosecondTimePoint lastRequest;
    bool                      running         = false;
    bool                      startupEvidence = true;

    bool        initializeRangeCircuit ();
    bool        observeRange           (adk::MicrosecondTimePoint now);
    RangeSignal decideRangeSignal      ();
    bool        showRangeSignal        (RangeSignal signal);
    bool        requestRange           (adk::MicrosecondTimePoint now);
    bool        measurementComplete    ();
    bool        echoIsHigh             ();
    void        stopSafely             ();
} // namespace

void setup ()
{
    running = initializeRangeCircuit ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::MicrosecondTimePoint now (micros ());

    if (startupEvidence)
    {
        if (now.elapsedSince (lastRequest).microseconds () < 150000U)
        {
            return;
        }

        startupEvidence = false;

        if (!showRangeSignal (RangeSignal::Waiting))
        {
            stopSafely ();
            return;
        }
    }

    if (!observeRange (now))
    {
        stopSafely ();
        return;
    }

    const RangeSignal signal = decideRangeSignal ();

    if (!showRangeSignal (signal))
    {
        stopSafely ();
        return;
    }

    if (now.elapsedSince (lastRequest).microseconds () >= 60000U &&
        measurementComplete ())
    {
        if (!requestRange (now))
        {
            stopSafely ();
        }
    }
}

namespace {

    bool initializeRangeCircuit ()
    {
        if (!trigger.initialize ().ok () ||
            !echo   .initialize ().ok () ||
            !valid  .initialize ().ok () ||
            !outside.initialize ().ok () ||
            !fault  .initialize ().ok () ||
            !ranger .initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        lastRequest = adk::MicrosecondTimePoint (micros ());

        return valid.on ().ok () && outside.on ().ok () && fault.on ().ok ();
    }

    bool observeRange (adk::MicrosecondTimePoint now)
    {
        const adk::RangeState state = ranger.reading ().state;

        if (state == adk::RangeState::Idle ||
            state == adk::RangeState::Valid ||
            state == adk::RangeState::Timeout ||
            state == adk::RangeState::OutOfRange)
        {
            return true;
        }

        return ranger.update (now, echoIsHigh ()).ok ();
    }

    RangeSignal decideRangeSignal ()
    {
        switch (ranger.reading ().state)
        {
            case adk::RangeState::Idle:        return RangeSignal::Waiting;
            case adk::RangeState::AwaitingEcho:return RangeSignal::Waiting;
            case adk::RangeState::Measuring:   return RangeSignal::Waiting;
            case adk::RangeState::Valid:       return RangeSignal::Valid;
            case adk::RangeState::OutOfRange:  return RangeSignal::Outside;
            case adk::RangeState::Timeout:     return RangeSignal::Fault;
        }

        return RangeSignal::Fault;
    }

    bool showRangeSignal (RangeSignal signal)
    {
        const bool validShown =
            (signal == RangeSignal::Valid ? valid.on () : valid.off ()).ok ();
        const bool outsideShown =
            (signal == RangeSignal::Outside ? outside.on () : outside.off ()).ok ();
        const bool faultShown =
            (signal == RangeSignal::Fault ? fault.on () : fault.off ()).ok ();

        return validShown && outsideShown && faultShown;
    }

    bool requestRange (adk::MicrosecondTimePoint now)
    {
        if (!trigger.write (adk::Level::High).ok ())
        {
            return false;
        }

        delayMicroseconds (10);

        if (!trigger.write (adk::Level::Low).ok ())
        {
            return false;
        }

        lastRequest = now;
        const adk::MicrosecondTimePoint started  (micros ());

        const bool                      echoHigh = echoIsHigh ();

        return ranger.startMeasurement (started, echoHigh).ok ();
    }

    bool measurementComplete ()
    {
        const adk::RangeState state = ranger.reading ().state;

        return state == adk::RangeState::Idle       ||
               state == adk::RangeState::Valid      ||
               state == adk::RangeState::Timeout    ||
               state == adk::RangeState::OutOfRange;
    }

    bool echoIsHigh ()
    {
        return echo.sample () == adk::Level::High;
    }

    void stopSafely ()
    {
        ranger.reset     ();
        trigger.write    (adk::Level::Low);
        fault.shutdown   ();
        outside.shutdown ();
        valid.shutdown   ();
        echo.shutdown    ();
        trigger.shutdown ();
        running = false;
    }
} // namespace
