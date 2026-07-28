// Mega 2560, USB 5 V: 10 kOhm potentiometer on A0, 220 Ohm LED on D6.
#include <Adk.h>

namespace {

    constexpr adk::PinId potentiometerPin = 54; // Mega A0
    constexpr adk::PinId brightnessPin    = 6;
    constexpr adk::PinId acquisitionPin   = 13;

    constexpr uint32_t sampleIntervalMs = 20;
    constexpr uint32_t experimentDurationMs = 120000;
    constexpr uint32_t acquisitionPulseMs  = 250;
    constexpr uint32_t observationGapMs    = 750;
    constexpr uint32_t shutdownWitnessMs   = 250;

    const adk::LinearCalibrationConfig calibrationConfig =
    {
        80, 940, 0, 255, true
    };

    adk::Runtime           runtime;
    adk::AnalogInput       potentiometer  (runtime.resources (), potentiometerPin);
    adk::PwmOutput         brightnessLed  (runtime.resources (), brightnessPin);
    adk::MonoLed           acquisitionLed (runtime.resources (), acquisitionPin);
#if defined(ADK_LESSON008_INJECT_D6_CONFLICT)
    adk::PwmOutput         acquisitionBlocker (runtime.resources (), brightnessPin);
#endif
    adk::LinearCalibration calibration    (calibrationConfig);
    adk::MovingAverage     average        (8);
    adk::Deadband          brightnessHold (4);

    uint32_t nextSampleMs = 0;
    uint32_t stopAtMs     = 0;
    bool     ready        = false;
    bool     halted       = false;

    bool acquireCircuit ();

    bool showAcquisition ();

    bool observationDue   (uint32_t now);

    bool chooseBrightness (adk::PwmOutput::Duty& brightness);

    void reportStages     (uint16_t raw,
                           uint16_t calibrated,
                           uint16_t smoothed,
                           uint16_t stable);

    void stopSafely ();

    void haltSafely ();

} // namespace

void setup ()
{
    Serial.begin (115200);

#if defined(ADK_LESSON008_INJECT_D6_CONFLICT)
    acquisitionBlocker.initialize ();
#endif

    ready        = acquireCircuit                                               () &&
                   showAcquisition                                              ();
    nextSampleMs = millis                                                       ();
    stopAtMs     = nextSampleMs + experimentDurationMs;

    if (!ready)
    {
        stopSafely ();
#if defined(ADK_LESSON008_INJECT_D6_CONFLICT)
        acquisitionBlocker.shutdown ();
#endif
    }
}

void loop ()
{
    if (halted)
    {
        return;
    }

    if (ready &&
        static_cast<int32_t> (millis () - stopAtMs) >= 0)
    {
        haltSafely ();
        return;
    }

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

    if (!brightnessLed.write (brightness).ok ())
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

        delay (acquisitionPulseMs);

        if (!acquisitionLed.off ().ok ())
        {
            return false;
        }

        delay (observationGapMs);
        return true;
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
        const uint16_t raw = potentiometer.read ();

        const adk::Result<uint16_t> calibrated = calibration.map (raw);

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
        reportStages (raw, calibrated.value (), smoothed.value (), stable);
        return true;
    }

    void reportStages (uint16_t raw,
                       uint16_t calibrated,
                       uint16_t smoothed,
                       uint16_t stable)
    {
        Serial.print   (raw);
        Serial.print   (',');
        Serial.print   (calibrated);
        Serial.print   (',');
        Serial.print   (smoothed);
        Serial.print   (',');
        Serial.println (stable);
    }

    void stopSafely ()
    {
        brightnessLed .shutdown ();
        potentiometer .shutdown ();
        acquisitionLed.shutdown ();
        ready = false;
    }

    void haltSafely ()
    {
        stopSafely ();

        delay (shutdownWitnessMs);
        halted = true;
    }

} // namespace
