// Mega 2560, USB 5 V: 10 kOhm potentiometer on A0, 220 Ohm LED on D6.
#include <Adk.h>

namespace {

    constexpr adk::PinId potentiometerPin = 54; // Mega A0
    constexpr adk::PinId brightnessPin    = 6;
    constexpr adk::PinId acquisitionPin   = 13;
    constexpr uint32_t   acquisitionOnMs  = 250;
    constexpr uint32_t   acquisitionGapMs = 750;
    constexpr uint32_t   runDurationMs    = 120000;

    adk::Runtime     runtime;
    adk::AnalogInput potentiometer  (runtime.resources (), potentiometerPin);
    adk::PwmOutput   brightnessLed  (runtime.resources (), brightnessPin);
    adk::MonoLed     acquisitionLed (runtime.resources (), acquisitionPin);

    bool     ready       = false;
    uint32_t startedAtMs = 0;

    bool acquireCircuit ();

    bool showAcquisition ();

    adk::PwmOutput::Duty chooseBrightness (adk::AnalogInput::Reading position);

    void recordTransformation (adk::AnalogInput::Reading position,
                               adk::PwmOutput::Duty      brightness);

    void stopSafely ();

} // namespace

void setup ()
{
    Serial.begin (115200);

    ready = acquireCircuit () && showAcquisition ();

    if (!ready)
    {
        stopSafely ();
        return;
    }

    startedAtMs = millis ();
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    const uint32_t now = millis ();
    if (static_cast<uint32_t> (now - startedAtMs) >= runDurationMs)
    {
        stopSafely ();
        return;
    }

    potentiometer.update                                                      ();
    const adk::AnalogInput::Reading position   = potentiometer.read           ();
    const adk::PwmOutput::Duty      brightness = chooseBrightness             (position);

    if (!brightnessLed.write (brightness).ok ())
    {
        stopSafely ();
        return;
    }

    recordTransformation (position, brightness);
}

namespace {

    bool acquireCircuit ()
    {
        if (!acquisitionLed.initialize ().ok ())
        {
            return false;
        }

        if (!potentiometer.initialize ().ok ())
        {
            acquisitionLed.shutdown ();
            return false;
        }

        if (!brightnessLed.initialize ().ok ())
        {
            potentiometer .shutdown ();
            acquisitionLed.shutdown ();
            return false;
        }

        return true;
    }

    bool showAcquisition ()
    {
        if (!acquisitionLed.on ().ok ())
        {
            return false;
        }

        delay                       (acquisitionOnMs);
        if (!acquisitionLed.off     ().ok ())
        {
            return false;
        }

        delay (acquisitionGapMs);
        return true;
    }

    adk::PwmOutput::Duty chooseBrightness (adk::AnalogInput::Reading position)
    {
        const uint32_t scaled = static_cast<uint32_t> (position) * 255u + 511u;
        return static_cast<adk::PwmOutput::Duty> (scaled /
                                                  adk::AnalogInput::maximumReading);
    }

    void recordTransformation (adk::AnalogInput::Reading position,
                               adk::PwmOutput::Duty      brightness)
    {
        static adk::AnalogInput::Reading previousPosition = 0;
        static bool                      firstRecord       = true;

        if (!firstRecord && position == previousPosition)
        {
            return;
        }

        Serial.print   ("raw=");
        Serial.print   (position);
        Serial.print   (",duty=");
        Serial.println (brightness);

        previousPosition = position;
        firstRecord       = false;
    }

    void stopSafely ()
    {
        brightnessLed .shutdown ();
        potentiometer .shutdown ();
        acquisitionLed.shutdown ();
        ready = false;
    }

} // namespace
