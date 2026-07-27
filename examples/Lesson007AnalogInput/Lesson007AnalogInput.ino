// Mega 2560, USB 5 V: 10 kOhm potentiometer on A0, 220 Ohm LED on D6.
#include <Adk.h>

namespace {

    constexpr adk::PinId potentiometerPin = 54; // Mega A0
    constexpr adk::PinId brightnessPin    = 6;
    constexpr adk::PinId diagnosticPin    = 13;

    adk::Runtime     runtime;
    adk::AnalogInput potentiometer (runtime.resources (), potentiometerPin);
    adk::PwmOutput   brightnessLed (runtime.resources (), brightnessPin);
    adk::MonoLed     diagnosticLed (runtime.resources (), diagnosticPin);

    bool ready = false;

    bool acquireCircuit ();

    bool showReady ();

    adk::PwmOutput::Duty chooseBrightness (adk::AnalogInput::Reading position);

    void stopSafely ();

} // namespace

void setup ()
{
    ready = acquireCircuit () && showReady ();

    if (!ready)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    potentiometer.update                                                      ();
    const adk::AnalogInput::Reading position   = potentiometer.read           ();
    const adk::PwmOutput::Duty      brightness = chooseBrightness             (position);

    if (brightnessLed.write (brightness) != adk::Status::Ok)
    {
        stopSafely ();
    }
}

namespace {

    bool acquireCircuit ()
    {
        if (diagnosticLed.initialize () != adk::Status::Ok)
        {
            return false;
        }

        if (potentiometer.initialize () != adk::Status::Ok)
        {
            diagnosticLed.shutdown ();
            return false;
        }

        if (brightnessLed.initialize () != adk::Status::Ok)
        {
            potentiometer.shutdown ();
            diagnosticLed.shutdown ();
            return false;
        }

        return true;
    }

    bool showReady ()
    {
        return diagnosticLed.on () == adk::Status::Ok;
    }

    adk::PwmOutput::Duty chooseBrightness (adk::AnalogInput::Reading position)
    {
        const uint32_t scaled = static_cast<uint32_t> (position) * 255u + 511u;
        return static_cast<adk::PwmOutput::Duty> (scaled /
                                                  adk::AnalogInput::maximumReading);
    }

    void stopSafely ()
    {
        brightnessLed.shutdown ();
        potentiometer.shutdown ();
        diagnosticLed.shutdown ();
        ready = false;
    }

} // namespace
