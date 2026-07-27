// Mega 2560, USB 5 V: 10 kOhm potentiometer on A0, 220 Ohm LED on D6.
#include <Adk.h>

namespace {

    constexpr adk::PinId potentiometerPin = 54; // Mega A0
    constexpr adk::PinId brightnessPin    = 6;
    constexpr adk::PinId diagnosticPin    = 13;

    constexpr uint32_t sampleIntervalMs = 20;

    const adk::LinearCalibrationConfig calibrationConfig =
    {
        80, 940, 0, 255, true
    };

    adk::Runtime           runtime;
    adk::AnalogInput       potentiometer  (runtime.resources (), potentiometerPin);
    adk::PwmOutput         brightnessLed  (runtime.resources (), brightnessPin);
    adk::MonoLed           diagnosticLed  (runtime.resources (), diagnosticPin);
    adk::LinearCalibration calibration    (calibrationConfig);
    adk::MovingAverage     average        (8);
    adk::Deadband          brightnessHold (4);

    uint32_t nextSampleMs = 0;
    bool     ready        = false;

    bool acquireCircuit ();

    bool showReady ();

    bool observationDue   (uint32_t now);

    bool chooseBrightness (adk::PwmOutput::Duty& brightness);

    void stopSafely ();

} // namespace

void setup ()
{
    ready        = acquireCircuit                                               () &&
                   showReady                                                    ();
    nextSampleMs = millis                                                       ();

    if (!ready)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!ready || !observationDue (millis ()))
    {
        return;
    }

    potentiometer.update                                                      ();

    adk::PwmOutput::Duty brightness = 0;
    if (!chooseBrightness                                                      (brightness))
    {
        stopSafely ();
        return;
    }

    if (brightnessLed.write (brightness) != adk::Status::Ok)
    {
        stopSafely ();
    }
}

namespace {

    bool acquireCircuit ()
    {
        if (!calibration.valid () || !average.valid ())
        {
            return false;
        }

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

    bool observationDue (uint32_t now)
    {
        if (static_cast<int32_t> (now - nextSampleMs) < 0)
        {
            return false;
        }

        nextSampleMs += sampleIntervalMs;
        return true;
    }

    bool chooseBrightness (adk::PwmOutput::Duty& brightness)
    {
        const adk::Result<uint16_t> calibrated = calibration.map              (
            potentiometer.read ());
        if (!calibrated.ok ())
        {
            return false;
        }

        const adk::Result<uint16_t> smoothed = average.addSample               (
            calibrated.value ());
        if (!smoothed.ok ())
        {
            return false;
        }

        const uint16_t stable = brightnessHold.addSample (smoothed.value ());
        brightness            = static_cast<adk::PwmOutput::Duty> (stable);
        return true;
    }

    void stopSafely ()
    {
        brightnessLed.shutdown ();
        potentiometer.shutdown ();
        diagnosticLed.shutdown ();
        ready = false;
    }

} // namespace
