#include <Adk.h>

#include <stdio.h>

// Mega 2560, USB 5 V: D22 is DHT11 DATA. D30-D35 drive an independently
// identified parallel LCD1602 in 4-bit mode. D5/D6/D7 drive a
// common-cathode RGB LED through one 330 Ohm resistor per channel.
// D13 gives a short acquisition blink. The station rests after two minutes.

namespace {

    constexpr adk::PinId dataPin = 22;

    const adk::Hd44780Pins displayPins = {30, 31, 32, 33, 34, 35};

    const adk::Rgb      startingColor    (0, 0, 48);
    const adk::Rgb      healthyColor     (0, 48, 0);
    const adk::Rgb      staleColor       (48, 20, 0);
    const adk::Rgb      faultColor       (64, 0, 0);
    const adk::Rgb      offColor         (0, 0, 0);
    const adk::Duration acquisitionPulse (250);
    const adk::Duration introTime        (1500);
    const adk::Duration runTime          (120000);
    const adk::Duration farewellTime     (1500);

    adk::Runtime runtime;

    adk::MonoLed     acquisitionLed (runtime.resources (), LED_BUILTIN, true);
    adk::Dht11Sensor climateSensor  (runtime.resources (), dataPin);

    adk::EnvironmentalStationConfig stationConfig;
    adk::EnvironmentalStation station (climateSensor, stationConfig);
    adk::Hd44780Display       display (runtime.resources (), displayPins);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusLed (runtime.resources (), redChannel, greenChannel, blueChannel);

    enum struct RunPhase : uint8_t
    {
        Intro,
        Running,
        Farewell,
        Halted
    };

    adk::TimePoint acquiredAt (0);
    adk::TimePoint farewellAt (0);
    RunPhase       runPhase = RunPhase::Halted;
    bool           acquisitionLit = false;

    bool        acquireStation       ();
    adk::Status observeEnvironment   (adk::TimePoint now);
    void        presentEnvironment   (const adk::EnvironmentalSnapshot& evidence);
    bool        showRecord           (const adk::EnvironmentalRecord& record);
    bool        showHealth           (adk::EnvironmentalHealth health,
                                      adk::TimePoint          now);
    const char* healthName           (adk::EnvironmentalHealth health);
    bool        pulseOn              (adk::TimePoint now,
                                      uint8_t        pulseCount);
    adk::Rgb    healthyClimateColor  (const adk::ClimateSample& sample);
    bool        updateLifecycle      (adk::TimePoint now);
    bool        beginFarewell        (adk::TimePoint now);
    void        haltStation          ();
} // namespace

void setup ()
{
    Serial.begin (115200);

    runPhase = acquireStation () ? RunPhase::Intro : RunPhase::Halted;
}

void loop ()
{
    if (runPhase == RunPhase::Halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (!updateLifecycle (now))
    {
        haltStation ();
        return;
    }

    if (runPhase != RunPhase::Running)
    {
        return;
    }

    const adk::Status                status   = observeEnvironment (now);
    const adk::EnvironmentalSnapshot evidence = station.snapshot   ();

    presentEnvironment (evidence);

    const bool        healthShown   = showHealth       (evidence.record.health, now);
    const adk::Status displayStatus = display.update   (now);
    const bool        displayReady  = displayStatus.ok ();
    const bool        stationUsable = status.ok        ();

    if (!healthShown ||
        !displayReady ||
        (!stationUsable &&
         evidence.record.health == adk::EnvironmentalHealth::TimingFault))
    {
        haltStation ();
    }
}

namespace {

    bool acquireStation ()
    {
        if (!acquisitionLed.initialize ().ok ())
        {
            return false;
        }

        if (!station.initialize ().ok ())
        {
            acquisitionLed.shutdown ();
            return false;
        }

        if (!display.initialize ().ok ())
        {
            station.shutdown        ();
            acquisitionLed.shutdown ();
            return false;
        }

        if (!statusLed.initialize ().ok ())
        {
            display.shutdown        ();
            station.shutdown        ();
            acquisitionLed.shutdown ();
            return false;
        }

        if (!display.show  (0, "STATION STARTING").ok () ||
            !display.show      (1, "WAIT FOR RECORD").ok () ||
            !statusLed.set     (startingColor).ok () ||
            !acquisitionLed.on ().ok ())
        {
            haltStation ();
            return false;
        }

        acquiredAt     = adk::TimePoint (millis ());
        acquisitionLit = true;
        return true;
    }

    adk::Status observeEnvironment (adk::TimePoint now)
    {
        return station.update (now);
    }

    void presentEnvironment (const adk::EnvironmentalSnapshot& evidence)
    {
        if (!evidence.recordReady)
        {
            return;
        }

        if (!showRecord (evidence.record))
        {
            haltStation ();
            return;
        }

        char record[96];

        const adk::Status formatStatus =
            adk::formatClimateRecord (evidence.record.sample,
                                      evidence.record.sequence,
                                      record,
                                      sizeof (record));

        if (formatStatus.ok ())
        {
            Serial.print (record);
        }
    }

    bool showRecord (const adk::EnvironmentalRecord& record)
    {
        char     healthLine[17];
        char     sampleLine[17];
        int32_t  temperature = record.sample.temperatureCentiCelsius;
        char     sign        = '+';

        if (temperature < 0)
        {
            sign        = '-';
            temperature = -temperature;
        }

        const int healthLength = snprintf (
            healthLine,
            sizeof (healthLine),
            "%-7s #%06lu",
            healthName (record.health),
            static_cast<unsigned long> (record.sequence % 1000000U));
        const int sampleLength = snprintf (
            sampleLine,
            sizeof (sampleLine),
            "T%c%02ld.%02ld H%04u",
            sign,
            static_cast<long> (temperature / 100),
            static_cast<long> (temperature % 100),
            record.sample.humidityPermille);

        return healthLength >= 0 && healthLength <= 16 &&
               sampleLength >= 0 && sampleLength <= 16 &&
               display.show (0, healthLine).ok () &&
               display.show (1, sampleLine).ok ();
    }

    bool showHealth (adk::EnvironmentalHealth health, adk::TimePoint now)
    {
        switch (health)
        {
            case adk::EnvironmentalHealth::Starting:
                return statusLed.set (
                    pulseOn (now, 1) ? startingColor : offColor).ok ();

            case adk::EnvironmentalHealth::Healthy:
                return statusLed.set (
                    healthyClimateColor (station.snapshot ().record.sample)).ok ();

            case adk::EnvironmentalHealth::Stale:
                return statusLed.set (
                    pulseOn (now, 2) ? staleColor : offColor).ok ();

            case adk::EnvironmentalHealth::SensorFault:
                return statusLed.set (
                    pulseOn (now, 2) ? faultColor : offColor).ok ();

            case adk::EnvironmentalHealth::TimingFault:
                return statusLed.set (
                    pulseOn (now, 3) ? faultColor : offColor).ok ();
        }

        return false;
    }

    const char* healthName (adk::EnvironmentalHealth health)
    {
        switch (health)
        {
            case adk::EnvironmentalHealth::Starting: return "START";
            case adk::EnvironmentalHealth::Healthy: return "HEALTHY";
            case adk::EnvironmentalHealth::SensorFault: return "SENSOR";
            case adk::EnvironmentalHealth::Stale: return "STALE";
            case adk::EnvironmentalHealth::TimingFault: return "TIMING";
        }

        return "UNKNOWN";
    }

    bool pulseOn (adk::TimePoint now, uint8_t pulseCount)
    {
        const uint32_t phase = now.milliseconds () % 2400U;

        return phase < static_cast<uint32_t> (pulseCount) * 400U &&
               (phase % 400U) < 200U;
    }

    adk::Rgb healthyClimateColor (const adk::ClimateSample& sample)
    {
        if (sample.humidityPermille >= 700)
        {
            return adk::Rgb (40, 24, 0);
        }

        if (sample.humidityPermille >= 550)
        {
            return adk::Rgb (0, 24, 48);
        }

        return healthyColor;
    }

    bool updateLifecycle (adk::TimePoint now)
    {
        if (acquisitionLit &&
            now.elapsedSince (acquiredAt).milliseconds () >=
                acquisitionPulse.milliseconds ())
        {
            if (!acquisitionLed.off ().ok ())
            {
                return false;
            }
            acquisitionLit = false;
        }

        if (runPhase == RunPhase::Intro)
        {
            if (!display.update (now).ok ())
            {
                return false;
            }

            if (display.ready () &&
                now.elapsedSince (acquiredAt).milliseconds () >=
                    introTime.milliseconds ())
            {
                runPhase = RunPhase::Running;
            }

            return true;
        }

        if (runPhase == RunPhase::Running &&
            now.elapsedSince (acquiredAt).milliseconds () >=
                runTime.milliseconds ())
        {
            return beginFarewell (now);
        }

        if (runPhase == RunPhase::Farewell)
        {
            if (!display.update (now).ok ())
            {
                return false;
            }

            if (now.elapsedSince (farewellAt).milliseconds () >=
                farewellTime.milliseconds ())
            {
                haltStation ();
            }
        }

        return true;
    }

    bool beginFarewell (adk::TimePoint now)
    {
        if (!display.show (0, "STATION RESTING ").ok () ||
            !display.show  (1, "PRESS RESET     ").ok () ||
            !statusLed.set (startingColor).ok ())
        {
            return false;
        }

        farewellAt = now;
        runPhase   = RunPhase::Farewell;
        return true;
    }

    void haltStation ()
    {
        acquisitionLed.off      ();
        statusLed.off           ();
        statusLed.shutdown      ();
        display.shutdown        ();
        station.shutdown        ();
        acquisitionLed.shutdown ();
        runPhase = RunPhase::Halted;
    }
} // namespace
