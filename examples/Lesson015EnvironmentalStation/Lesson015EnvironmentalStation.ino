#include <Adk.h>

#include <stdio.h>

// Mega 2560, USB 5 V: D22 is DHT11 DATA. D30-D35 drive a 16x2 LCD
// in 4-bit mode. D5/D6/D7 drive a common-cathode RGB LED through
// one 330 Ohm resistor per channel. TP13 is the D22 DATA test point.

namespace {

    constexpr adk::PinId dataPin = 22;

    const adk::Hd44780Pins displayPins = {30, 31, 32, 33, 34, 35};

    const adk::Rgb startingColor (0, 0, 48);
    const adk::Rgb healthyColor  (0, 48, 0);
    const adk::Rgb staleColor    (48, 20, 0);
    const adk::Rgb faultColor    (64, 0, 0);
    const adk::Rgb offColor      (0, 0, 0);

    adk::Runtime runtime;

    adk::Dht11Sensor climateSensor (runtime.resources (), dataPin);

    adk::EnvironmentalStationConfig stationConfig;
    adk::EnvironmentalStation station (climateSensor, stationConfig);
    adk::Hd44780Display       display (runtime.resources (), displayPins);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusLed (runtime.resources (), redChannel, greenChannel, blueChannel);

    bool halted = false;

    bool        acquireStation       ();
    adk::Status observeEnvironment   (adk::TimePoint now);
    void        presentEnvironment   (const adk::EnvironmentalSnapshot& evidence);
    bool        showRecord           (const adk::EnvironmentalRecord& record);
    bool        showHealth           (adk::EnvironmentalHealth health,
                                      adk::TimePoint          now);
    const char* healthName           (adk::EnvironmentalHealth health);
    bool        pulseOn              (adk::TimePoint now,
                                      uint8_t        pulseCount);
    void        haltStation          ();
} // namespace

void setup ()
{
    Serial.begin (115200);

    halted = !acquireStation ();
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

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
        if (!station.initialize ().ok ())
        {
            return false;
        }

        if (!display.initialize ().ok ())
        {
            station.shutdown ();
            return false;
        }

        if (!statusLed.initialize ().ok ())
        {
            display.shutdown ();
            station.shutdown ();
            return false;
        }

        if (!display.show  (0, "STATION STARTING").ok () ||
            !display.show  (1, "WAIT FOR RECORD").ok () ||
            !statusLed.set (startingColor).ok ())
        {
            haltStation ();
            return false;
        }

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
                return statusLed.set (healthyColor).ok ();

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

    void haltStation ()
    {
        statusLed.off      ();
        statusLed.shutdown ();
        display.shutdown   ();
        station.shutdown   ();
        halted = true;
    }
} // namespace
