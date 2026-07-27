#include <Adk.h>

// Mega 2560, USB 5 V: a 16x2 HD44780 LCD uses D22-D27 in 4-bit mode.
// D13 is the separate resource-acquisition indicator. LCD VO uses a 10 kOhm
// contrast potentiometer; the backlight follows the identified module rating.

namespace {

    const adk::Hd44780Pins displayPins = {22, 23, 24, 25, 26, 27};
    const adk::Duration    samplePeriod (2000);
    const adk::TimePoint   traceStart   (0);

    const adk::ClimateSample samples[] = {
        {2150, 456, traceStart, adk::ClimateSampleState::Valid},
        {2160, 458, traceStart, adk::ClimateSampleState::Valid},
        {0, 0, traceStart, adk::ClimateSampleState::TransportTimeout},
        {2175, 461, traceStart, adk::ClimateSampleState::Valid}};

    const char* sampleLines[][2] = {{"sample 0001     ", "T=21.50 H=45.6 "},
                                    {"sample 0002     ", "T=21.60 H=45.8 "},
                                    {"sample 0003     ", "SENSOR TIMEOUT  "},
                                    {"sample 0004     ", "T=21.75 H=46.1 "}};

    adk::Runtime        runtime;
    adk::MonoLed        acquisitionLed  (runtime.resources (), LED_BUILTIN, true);
    adk::Hd44780Display display         (runtime.resources (), displayPins);

    adk::TimePoint lastSampleAt (0);
    uint8_t        sampleIndex = 0;
    uint32_t       sequence    = 0;
    bool           started     = false;
    bool           halted      = false;

    bool               acquireDisplayCircuit  ();
    bool               observeSampleCadence   (adk::TimePoint now);
    adk::ClimateSample decideSample           (adk::TimePoint now);
    bool               showSample             (const adk::ClimateSample& sample);
    bool               recordSample           (const adk::ClimateSample& sample);
    void               haltDisplayCircuit     ();
} // namespace

void setup ()
{
    Serial.begin (115200);

    halted = !acquireDisplayCircuit ();
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (!display.update (now).ok ())
    {
        haltDisplayCircuit ();
        return;
    }

    if (!display.ready () || !observeSampleCadence (now))
    {
        return;
    }

    const adk::ClimateSample sample = decideSample (now);

    if (!showSample (sample) || !recordSample (sample))
    {
        haltDisplayCircuit ();
    }
}

namespace {

    bool acquireDisplayCircuit ()
    {
        if (!acquisitionLed.initialize ().ok ())
        {
            return false;
        }

        if (!display.initialize ().ok ())
        {
            acquisitionLed.shutdown ();
            return false;
        }

        return acquisitionLed.on ().ok ();
    }

    bool observeSampleCadence (adk::TimePoint now)
    {
        if (!started)
        {
            lastSampleAt = now;
            started      = true;
            return true;
        }

        if (now.elapsedSince (lastSampleAt).milliseconds () <
            samplePeriod.milliseconds ())
        {
            return false;
        }

        lastSampleAt = now;
        return true;
    }

    adk::ClimateSample decideSample (adk::TimePoint now)
    {
        adk::ClimateSample sample = samples[sampleIndex];
        sample.observedAt         = now;
        return sample;
    }

    bool showSample (const adk::ClimateSample&)
    {
        if (!display.show (0, sampleLines[sampleIndex][0]).ok () ||
            !display.show (1, sampleLines[sampleIndex][1]).ok ())
        {
            return false;
        }

        sampleIndex = static_cast<uint8_t> ((sampleIndex + 1U) %
                                            (sizeof (samples) / sizeof (samples[0])));
        return true;
    }

    bool recordSample (const adk::ClimateSample& sample)
    {
        char record[112] = {};

        if (!adk::formatClimateRecord (sample, ++sequence, record, sizeof (record))
                 .ok ())
        {
            return false;
        }

        Serial.print (record);
        return true;
    }

    void haltDisplayCircuit ()
    {
        acquisitionLed.off      ();
        display.shutdown        ();
        acquisitionLed.shutdown ();
        halted = true;
    }
} // namespace
