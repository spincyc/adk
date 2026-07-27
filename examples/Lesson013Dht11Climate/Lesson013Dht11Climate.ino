#include <Adk.h>

// Mega 2560, USB 5 V: D22 is DHT11 DATA. D5/D6/D7 drive a
// common-cathode RGB LED through one 330 Ohm resistor per channel.

namespace {

    constexpr adk::PinId dataPin = 22;

    const adk::Rgb waitingColor (0, 0, 48);
    const adk::Rgb validColor   (0, 48, 0);
    const adk::Rgb faultColor   (64, 0, 0);
    const adk::Rgb staleColor   (48, 20, 0);
    const adk::Rgb offColor     (0, 0, 0);

    const adk::Duration freshFor (5000);

    adk::Runtime runtime;

    adk::Dht11Sensor climateSensor (runtime.resources (), dataPin);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusLed (runtime.resources (), redChannel, greenChannel, blueChannel);

    bool halted = false;

    bool               acquireClimateCircuit ();
    adk::Status        observeClimate        (adk::TimePoint now);
    adk::ClimateSample decideClimateState    (adk::TimePoint now);
    bool               showClimateState      (const adk::ClimateSample& observation,
                                              adk::TimePoint            now);
    adk::Rgb           faultPattern          (adk::ClimateSampleState state,
                                              adk::TimePoint           now);
    bool               pulseOn               (adk::TimePoint now,
                                              uint8_t        pulseCount);
    void               haltClimateCircuit    ();
} // namespace

void setup ()
{
    halted = !acquireClimateCircuit ();
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    const adk::Status        status      = observeClimate     (now);
    const adk::ClimateSample observation = decideClimateState (now);

    if ((!status.ok () &&
         observation.state == adk::ClimateSampleState::Unavailable) ||
        !showClimateState (observation, now))
    {
        haltClimateCircuit ();
    }
}

namespace {

    bool acquireClimateCircuit ()
    {
        if (!climateSensor.initialize ().ok ())
        {
            return false;
        }

        if (!statusLed.initialize ().ok ())
        {
            climateSensor.shutdown ();
            return false;
        }

        if (!statusLed.set (waitingColor).ok ())
        {
            haltClimateCircuit ();
            return false;
        }

        return true;
    }

    adk::Status observeClimate (adk::TimePoint now)
    {
        return climateSensor.update (now);
    }

    adk::ClimateSample decideClimateState (adk::TimePoint now)
    {
        return climateSensor.sample (now, freshFor);
    }

    bool showClimateState (const adk::ClimateSample& observation, adk::TimePoint now)
    {
        switch (observation.state)
        {
            case adk::ClimateSampleState::Unavailable:
                return statusLed.set (pulseOn (now, 1) ? waitingColor : offColor).ok ();

            case adk::ClimateSampleState::Valid:
                return statusLed.set (validColor).ok ();

            case adk::ClimateSampleState::Stale:
                return statusLed.set (staleColor).ok ();

            case adk::ClimateSampleState::TransportTimeout:
            case adk::ClimateSampleState::ChecksumFailure:
            case adk::ClimateSampleState::TemperatureOutOfRange:
            case adk::ClimateSampleState::HumidityOutOfRange:
            case adk::ClimateSampleState::InvalidLimits:
            case adk::ClimateSampleState::InvalidTiming:
                return statusLed.set (faultPattern (observation.state, now)).ok ();
        }

        return false;
    }

    adk::Rgb faultPattern (adk::ClimateSampleState state, adk::TimePoint now)
    {
        uint8_t pulseCount = 3;

        if (state == adk::ClimateSampleState::TransportTimeout)
        {
            pulseCount = 2;
        }

        if ((state == adk::ClimateSampleState::TemperatureOutOfRange ||
             state == adk::ClimateSampleState::HumidityOutOfRange) &&
            (now.milliseconds () % 2400U) >= 1600U)
        {
            return staleColor;
        }

        return pulseOn (now, pulseCount) ? faultColor : offColor;
    }

    bool pulseOn (adk::TimePoint now, uint8_t pulseCount)
    {
        const uint32_t phase = now.milliseconds () % 2400U;

        return phase < static_cast<uint32_t> (pulseCount) * 400U &&
               (phase % 400U) < 200U;
    }

    void haltClimateCircuit ()
    {
        statusLed.off          ();
        statusLed.shutdown     ();
        climateSensor.shutdown ();
        halted = true;
    }
} // namespace
