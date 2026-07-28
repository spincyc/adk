#include <Adk.h>

#ifndef ADK_LESSON009_ACCEPTANCE_TELEMETRY
#define ADK_LESSON009_ACCEPTANCE_TELEMETRY 0
#endif

#if ADK_LESSON009_ACCEPTANCE_TELEMETRY != 0 && \
    ADK_LESSON009_ACCEPTANCE_TELEMETRY != 1
#error "ADK_LESSON009_ACCEPTANCE_TELEMETRY must be 0 or 1"
#endif

// Mega 2560, USB 5 V: A0 LDR/10 kOhm divider; D9 white LED and
// D5/D6/D7 common-cathode RGB LED, each LED die through 330 Ohm.

namespace {

    constexpr adk::PinId lightSensorPin = 54; // A0 on the Mega 2560.
    constexpr adk::PinId lampPin        = 9;
    constexpr adk::PinId acquisitionPin = LED_BUILTIN;

    constexpr uint16_t sensorFaultLow  = 7;
    constexpr uint16_t sensorFaultHigh = 1016;
    constexpr uint32_t acquisitionPulseMilliseconds = 250u;
    constexpr uint32_t acquisitionGapMilliseconds   = 750u;
    constexpr uint32_t runDurationMilliseconds      = 120000u;
    constexpr uint32_t inactiveSettleMilliseconds   = 250u;
    constexpr uint32_t telemetryIntervalMilliseconds = 250u;

    const adk::Rgb readyColor    (0, 0, 48);
    const adk::Rgb activeColor   (0, 48, 0);
    const adk::Rgb faultColor    (64, 0, 0);
    const adk::Rgb inactiveColor (0, 0, 0);

    adk::Runtime runtime;

    adk::AnalogInput lightSensor    (runtime.resources (), lightSensorPin);
    adk::PwmOutput   lampOutput     (runtime.resources (), lampPin);
    adk::MonoLed     acquisitionLed (runtime.resources (), acquisitionPin);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusLed (runtime.resources (), redChannel, greenChannel, blueChannel);

    const adk::LinearCalibrationConfig calibrationConfig =
    {
        sensorFaultLow,
        sensorFaultHigh,
        0,
        1000,
        true
    };

    adk::LinearCalibration lightCalibration (calibrationConfig);
    adk::MovingAverage     lightAverage     (8);
    adk::Deadband          lightDeadband    (8);

    adk::NightLightConfig nightLightConfig;
    adk::NightLight       nightLight (nightLightConfig);

    bool     halted          = false;
    uint32_t runStartedAt    = 0;
    uint32_t nextTelemetryAt = 0;

    bool                 initializeCircuit ();
    bool                 showAcquisition   ();
    adk::NightLightInput observeLight      ();
    adk::Status          decideLighting    (const adk::NightLightInput& observation);
    bool                 actuateLighting   (const adk::NightLightSnapshot& decision);
    void                 stopSafely        ();
    void                 reportRawSample   (uint16_t rawLight);
} // namespace

void setup ()
{
#if ADK_LESSON009_ACCEPTANCE_TELEMETRY
    Serial.begin (115200);
#endif

    halted = !initializeCircuit () || !showAcquisition ();

    if (halted)
    {
        stopSafely ();
        return;
    }

    runStartedAt = millis ();
    nextTelemetryAt = runStartedAt;
}

void loop ()
{
    if (halted)
    {
        return;
    }

    if (static_cast<uint32_t> (millis () - runStartedAt) >=
        runDurationMilliseconds)
    {
        stopSafely ();
        return;
    }

    const adk::NightLightInput observation = observeLight ();

    const adk::Status             decisionStatus = decideLighting      (observation);
    const adk::NightLightSnapshot decision       = nightLight.snapshot ();

    if (!decisionStatus.ok () &&
        decision.diagnostic != adk::NightLightDiagnostic::SensorFault)
    {
        stopSafely ();
        return;
    }

    if (!actuateLighting (decision))
    {
        stopSafely ();
    }
}

namespace {

    bool initializeCircuit ()
    {
        if (!acquisitionLed.initialize ().ok ())
        {
            return false;
        }

        if (!lightSensor.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        if (!lampOutput.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        if (!statusLed.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        if (!nightLight.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        if (!statusLed.set (readyColor).ok ())
        {
            stopSafely ();
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

        delay (acquisitionPulseMilliseconds);

        if (!acquisitionLed.off ().ok ())
        {
            return false;
        }

        delay (acquisitionGapMilliseconds);
        return true;
    }

    adk::NightLightInput observeLight ()
    {
        lightSensor.update                          ();
        const uint16_t rawLight = lightSensor.read  ();
        reportRawSample                             (rawLight);

        if (rawLight <= sensorFaultLow)
        {
            return adk::NightLightInput (0, adk::LightSampleState::BelowRange);
        }

        if (rawLight >= sensorFaultHigh)
        {
            return adk::NightLightInput (1000, adk::LightSampleState::AboveRange);
        }

        const adk::Result<uint16_t> calibrated = lightCalibration.map (rawLight);

        if (!calibrated.ok ())
        {
            return adk::NightLightInput (0, adk::LightSampleState::Stale);
        }

        const adk::Result<uint16_t> averaged = lightAverage.addSample (calibrated.value ());

        if (!averaged.ok ())
        {
            return adk::NightLightInput (0, adk::LightSampleState::Stale);
        }

        return adk::NightLightInput (lightDeadband.addSample (averaged.value ()));
    }

    adk::Status decideLighting (const adk::NightLightInput& observation)
    {
        return nightLight.update (observation);
    }

    bool actuateLighting (const adk::NightLightSnapshot& decision)
    {
        if (!lampOutput.write (decision.outputDuty).ok ())
        {
            return false;
        }

        switch (decision.diagnostic)
        {
            case adk::NightLightDiagnostic::Ready:
                return statusLed.set (readyColor).ok ();

            case adk::NightLightDiagnostic::Active:
                return statusLed.set (activeColor).ok ();

            case adk::NightLightDiagnostic::SensorFault:
                return statusLed.set (faultColor).ok ();
        }

        return false;
    }

    void stopSafely ()
    {
        lampOutput.write (0);
        statusLed.set    (inactiveColor);
        delay            (inactiveSettleMilliseconds);

        statusLed.shutdown      ();
        lampOutput.shutdown     ();
        lightSensor.shutdown    ();
        acquisitionLed.shutdown ();
        halted = true;
    }

    void reportRawSample (uint16_t rawLight)
    {
#if ADK_LESSON009_ACCEPTANCE_TELEMETRY
        const uint32_t now = millis ();

        if (static_cast<int32_t> (now - nextTelemetryAt) < 0)
        {
            return;
        }

        Serial.print   ("raw=");
        Serial.println (rawLight);
        nextTelemetryAt = now + telemetryIntervalMilliseconds;
#else
        static_cast<void> (rawLight);
#endif
    }
} // namespace
