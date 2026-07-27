#include <Adk.h>

// Mega 2560, USB 5 V: A0 LDR/10 kOhm divider; D9 white LED and
// D5/D6/D7 common-cathode RGB LED, each LED die through 330 Ohm.

namespace {

    constexpr adk::PinId lightSensorPin = 54; // A0 on the Mega 2560.
    constexpr adk::PinId lampPin        = 9;

    constexpr uint16_t sensorFaultLow  = 7;
    constexpr uint16_t sensorFaultHigh = 1016;

    const adk::Rgb readyColor  (0, 0, 48);
    const adk::Rgb activeColor (0, 48, 0);
    const adk::Rgb faultColor  (64, 0, 0);

    adk::Runtime runtime;

    adk::AnalogInput lightSensor (runtime.resources (), lightSensorPin);
    adk::PwmOutput   lampOutput  (runtime.resources (), lampPin);

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

    bool halted = false;

    bool                 initializeCircuit ();
    adk::NightLightInput observeLight      ();
    adk::Status          decideLighting    (const adk::NightLightInput& observation);
    bool                 actuateLighting   (const adk::NightLightSnapshot& decision);
    void                 stopSafely        ();
} // namespace

void setup ()
{
    halted = !initializeCircuit ();
}

void loop ()
{
    if (halted)
    {
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

    adk::NightLightInput observeLight ()
    {
        lightSensor.update                         ();
        const uint16_t rawLight = lightSensor.read ();

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
        lampOutput.shutdown  ();
        statusLed.shutdown   ();
        lightSensor.shutdown ();
        halted = true;
    }
} // namespace
