// Mega 2560, USB logic: button D22; marker LEDs D30-D32 with 220 Ohm.
// Servo signal is D44/OC5C. Keep servo positive disconnected for the TP-S check.
#include <Adk.h>

namespace {

    constexpr uint16_t positions[] = {500, 0, 500, 1000};
    constexpr uint8_t  positionCount =
        static_cast<uint8_t> (sizeof (positions) / sizeof (positions[0]));

    const adk::ButtonConfig commandButtonConfig (22);

    const adk::BoundedServoConfig servoConfig = {{0, 1000, 1000, 2000}, 500};

    adk::Runtime runtime;

    adk::Button  commandButton (runtime.resources (), commandButtonConfig);
    adk::MonoLed lowMarker     (runtime.resources (), 30);
    adk::MonoLed middleMarker  (runtime.resources (), 31);
    adk::MonoLed highMarker    (runtime.resources (), 32);
    adk::MonoLed logicReady    (runtime.resources (), LED_BUILTIN);

    adk::MonoLed* positionMarkers[] = {&lowMarker, &middleMarker, &highMarker};

    adk::ExternalPowerDomainGate commandGate;
    adk::MegaTimer5ServoPulseIo  pulseIo;
    adk::ServoOutput  pulseOutput (runtime.resources (), pulseIo, commandGate,
                                   adk::MegaTimer5ServoPulseIo::signalPin,
                                   servoConfig.calibration.minimumPulseUs,
                                   servoConfig.calibration.maximumPulseUs,
                                   adk::MegaTimer5ServoPulseIo::timer);
    adk::BoundedServo servo (servoConfig);

    uint8_t nextPosition = 0;
    bool    running      = false;

    bool validateConfigurationRecord ();

    bool initializeBench ();

    bool startSafeWaveform ();

    bool commandRequested (adk::TimePoint now);

    bool commandPosition (uint16_t position);

    bool showCommandedPosition (uint16_t position);

    void stopLogic ();

} // namespace

void setup ()
{
    running = initializeBench ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (!commandRequested (now))
    {
        return;
    }

    const uint16_t position = positions[nextPosition];

    if (!commandPosition (position))
    {
        stopLogic ();
        return;
    }

    nextPosition = static_cast<uint8_t> ((nextPosition + 1) % positionCount);
}

namespace {

    bool initializeBench ()
    {
        if (!validateConfigurationRecord ())
        {
            return false;
        }

        if (!commandButton.initialize ().ok ())
        {
            return false;
        }

        for (uint8_t index = 0; index < 3; ++index)
        {
            if (!positionMarkers[index]->initialize ().ok ())
            {
                stopLogic ();
                return false;
            }
        }

        if (!logicReady.initialize ().ok () || !servo.initialize ().ok () ||
            !pulseOutput.initialize ().ok ())
        {
            stopLogic ();
            return false;
        }

        if (!startSafeWaveform () || !logicReady.on ().ok ())
        {
            stopLogic ();
            return false;
        }

        return true;
    }

    bool validateConfigurationRecord ()
    {
        adk::ServoConfigurationRecord record;
        const adk::ServoConfiguration saved = {servoConfig, 1};

        if (!record.save (saved).ok ())
        {
            return false;
        }

        const adk::Result<adk::ServoConfiguration> loaded = record.load ();

        if (!loaded.ok ())
        {
            return false;
        }

        const adk::ServoConfiguration& decoded = loaded.value ();

        return decoded.generation == saved.generation &&
               decoded.servo.calibration.minimumPosition ==
                   servoConfig.calibration.minimumPosition &&
               decoded.servo.calibration.maximumPosition ==
                   servoConfig.calibration.maximumPosition &&
               decoded.servo.calibration.minimumPulseUs ==
                   servoConfig.calibration.minimumPulseUs &&
               decoded.servo.calibration.maximumPulseUs ==
                   servoConfig.calibration.maximumPulseUs &&
               decoded.servo.safePosition == servoConfig.safePosition;
    }

    bool startSafeWaveform ()
    {
        commandGate.admit ();

        if (!servo.command (servoConfig.safePosition).ok ())
        {
            commandGate.revoke ();
            return false;
        }

        const adk::BoundedServoSnapshot safe = servo.snapshot ();

        if (!pulseOutput.writePulse (safe.pulseUs).ok ())
        {
            commandGate.revoke ();
            return false;
        }

        return showCommandedPosition (safe.position);
    }

    bool commandRequested (adk::TimePoint now)
    {
        commandButton.update            (now);
        return commandButton.pressEvent ();
    }

    bool commandPosition (uint16_t position)
    {
        commandGate.admit ();

        if (!servo.command (position).ok ())
        {
            commandGate.revoke ();
            return false;
        }

        const adk::BoundedServoSnapshot command = servo.snapshot ();

        if (!pulseOutput.writePulse (command.pulseUs).ok ())
        {
            commandGate.revoke ();
            return false;
        }

        return showCommandedPosition (command.position);
    }

    bool showCommandedPosition (uint16_t position)
    {
        const uint8_t marker = position < 250 ? 0 : (position > 750 ? 2 : 1);
        bool          ready  = true;

        for (uint8_t index = 0; index < 3; ++index)
        {
            ready = positionMarkers[index]->set (index == marker).ok () && ready;
        }

        return ready;
    }

    void stopLogic ()
    {
        commandGate.revoke      ();
        servo.shutdown          ();
        pulseOutput.shutdown    ();

        logicReady.shutdown ();

        for (uint8_t index = 3; index > 0; --index)
        {
            positionMarkers[index - 1]->shutdown ();
        }

        commandButton.shutdown ();
        running = false;
    }

} // namespace
