// Mega 2560, USB 5 V only. After unpowered label and ground checks, connect
// joystick VRx/X/HOR to A0, VRy/Y/VER to A1, and active-low SW/KEY/SEL to D22.
// Connect a common-cathode RGB LED at D5/D6/D7 through one 330 Ohm resistor
// per die. TP-X is A0 and TP-Y is A1. Bench acceptance remains open.
#include <Adk.h>
#include <analog_joystick.h>

namespace {

    constexpr adk::PinId xAxisPin       = 54; // A0 on the Mega 2560.
    constexpr adk::PinId yAxisPin       = 55; // A1 on the Mega 2560.
    constexpr adk::PinId selectPin      = 22;
    constexpr uint32_t   readyPulseMs   = 750;
    constexpr uint32_t   selectPulseMs  = 200;
    constexpr uint8_t    minimumPreview = 16;

    const adk::JoystickAxisConfig xAxisConfig      (xAxisPin, 512, 0, 1023, 48);
    const adk::JoystickAxisConfig yAxisConfig      (yAxisPin, 512, 0, 1023, 48);
    const adk::ButtonConfig       selectConfig     (selectPin);
    const adk::AnalogJoystickConfig joystickConfig (
        xAxisConfig, yAxisConfig, selectConfig);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};

    adk::Runtime        runtime;
    adk::AnalogJoystick joystick   (runtime.resources (), joystickConfig);
    adk::RgbLed         previewLed (
        runtime.resources (), redChannel, greenChannel, blueChannel);

    adk::TimePoint startedAt;
    adk::TimePoint selectPulseStartedAt;
    bool           selectPulseActive = false;
    bool           running = false;

    bool                         acquireJoystickCircuit ();
    void                         configurePreview       (adk::TimePoint now);
    bool                         startJoystickLesson    ();
    adk::AnalogJoystickSnapshot observeJoystick         (adk::TimePoint now);
    adk::Rgb                     decidePreview          (
        adk::TimePoint now, const adk::AnalogJoystickSnapshot& observation);
    bool                         actuatePreview         (const adk::Rgb& color);
    uint8_t                      yBrightness            (int16_t position);
    void                         stopSafely             ();

} // namespace

void setup ()
{
    const adk::TimePoint now (millis ());

    if (acquireJoystickCircuit ())
    {
        configurePreview    (now);

        running = startJoystickLesson ();
    }

    if (!running)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    const adk::AnalogJoystickSnapshot observation = observeJoystick (now);

    if (!observation.status.ok ())
    {
        stopSafely ();
        return;
    }

    const adk::Rgb preview = decidePreview (now, observation);

    if (!actuatePreview (preview))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireJoystickCircuit ()
    {
        if (!joystick.initialize ().ok ())
        {
            return false;
        }

        if (!previewLed.initialize ().ok ())
        {
            joystick.shutdown ();
            return false;
        }

        return true;
    }

    void configurePreview (adk::TimePoint now)
    {
        startedAt = now;
    }

    bool startJoystickLesson ()
    {
        return previewLed.set (adk::Rgb (0, 0, 64)).ok ();
    }

    adk::AnalogJoystickSnapshot observeJoystick (adk::TimePoint now)
    {
        joystick.update (now);

        return joystick.snapshot ();
    }

    adk::Rgb decidePreview (
        adk::TimePoint now, const adk::AnalogJoystickSnapshot& observation)
    {
        if (now.elapsedSince (startedAt).milliseconds () < readyPulseMs)
        {
            return adk::Rgb (0, 0, 64);
        }

        if (observation.selectEvent)
        {
            selectPulseStartedAt = now;
            selectPulseActive    = true;
        }

        if (selectPulseActive)
        {
            if (now.elapsedSince (selectPulseStartedAt).milliseconds () <
                selectPulseMs)
            {
                return adk::Rgb (96, 96, 96);
            }

            selectPulseActive = false;
        }

        const uint8_t brightness = yBrightness (observation.y.position);

        if (observation.x.position < 0)
        {
            return adk::Rgb (brightness, 0, 0);
        }

        if (observation.x.position > 0)
        {
            return adk::Rgb (0, brightness, 0);
        }

        return adk::Rgb (0, 0, brightness);
    }

    bool actuatePreview (const adk::Rgb& color)
    {
        return previewLed.set (color).ok ();
    }

    uint8_t yBrightness (int16_t position)
    {
        const int32_t shifted = static_cast<int32_t> (position) -
                                adk::AnalogJoystick::minimumPosition;
        const int32_t span = adk::AnalogJoystick::maximumPosition -
                             adk::AnalogJoystick::minimumPosition;

        return static_cast<uint8_t> (
            minimumPreview +
            shifted * (255 - minimumPreview) / span);
    }

    void stopSafely ()
    {
        running = false;
        previewLed.shutdown ();
        joystick.shutdown   ();
    }
} // namespace
