#include "check.h"

#include <Arduino.h>
#include <adk/dht11.h>

#include <vector>

namespace {

    const adk::Pin Data = 7;

    // How long each part of a reply lasts, in microseconds.
    struct Timing
    {
        unsigned long wait;     // after the start signal, before the answer
        unsigned long low;      // the answer: low, then high
        unsigned long high;
        unsigned long bitLow;   // before every bit
        unsigned long zero;     // the high of a 0
        unsigned long one;      // the high of a 1
    };

    const Timing typical = {30, 80, 80, 50, 27, 70};
    const Timing fast    = {15, 75, 75, 45, 22, 66};
    const Timing slow    = {45, 90, 95, 58, 30, 76};

    struct Level
    {
        int           level;
        unsigned long us;
    };

    // Five bytes as the sensor sends them, ending with their checksum.
    std::vector<uint8_t> reading (uint8_t humidity, uint8_t humidityTenths, uint8_t degrees,
                                  uint8_t degreeTenths)
    {
        int sum = humidity + humidityTenths + degrees + degreeTenths;
        return {humidity, humidityTenths, degrees, degreeTenths, static_cast<uint8_t> (sum)};
    }

    // Plays a DHT11 on the data pin. The start signal readies it; from the
    // next read of the line it plays its reply against the clock, then lets
    // the line go high.
    struct Sensor
    {
        Sensor ()
            : began       (0)
            , signalledAt (0)
            , modeAtLow   (0)
            , ready       (false)
            , playing     (false)
            , released    (false)
        {
            arduino::setCallCost (4);
            arduino::onDigitalWrite = [this] (uint8_t pin, uint8_t value) { written (pin, value); };
            arduino::onDigitalRead  = [this] (uint8_t pin) { return read (pin); };
        }

        // Answer the next start signal with these bytes, or only their
        // first bits, as if the sensor stopped partway.
        void answer (const std::vector<uint8_t>& bytes, Timing timing = typical, int bits = 40)
        {
            reply = {{HIGH, timing.wait}, {LOW, timing.low}, {HIGH, timing.high}};

            for (int bit = 0; bit < bits; ++bit)
            {
                bool one = bytes[static_cast<size_t> (bit / 8)] & (0x80 >> (bit % 8));

                reply.push_back ({LOW, timing.bitLow});
                reply.push_back ({HIGH, one ? timing.one : timing.zero});
            }

            if (bits == 40)
            {
                reply.push_back ({LOW, 50});
            }
        }

        void written (uint8_t pin, uint8_t value)
        {
            if (pin == Data && value == LOW)
            {
                signalledAt = arduino::now ();
                modeAtLow   = arduino::pin (Data).mode;
                ready       = true;
                playing     = false;
            }
        }

        int read (uint8_t pin)
        {
            if (pin != Data || !ready)
            {
                return HIGH;
            }

            if (!playing)
            {
                playing  = true;
                began    = arduino::now ();
                released = arduino::pin (Data).mode == INPUT_PULLUP;
            }

            unsigned long elapsed = arduino::now () - began;

            for (const Level& part : reply)
            {
                if (elapsed < part.us)
                {
                    return part.level;
                }

                elapsed -= part.us;
            }

            return HIGH;
        }

        std::vector<Level> reply;
        unsigned long      began;
        unsigned long      signalledAt;
        uint8_t            modeAtLow;
        bool               ready;
        bool               playing;
        bool               released;
    };

    // The update that starts a reading, and the one that receives it.
    void readOnce (adk::Millis at)
    {
        adk::update (at);
        adk::update (at + 20);
    }
}

TEST (dht11ClaimsItsPinWithThePullUp)
{
    adk::Dht11 dht11 {Data};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (Data).mode == INPUT_PULLUP);
    CHECK (!dht11.ok ());
    CHECK (!dht11.measured ());
}

TEST (dht11SharingItsPinHalts)
{
    adk::Dht11 dht11 {Data};
    adk::Led   led   {Data};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == Data);
}

TEST (dht11ReadsTemperatureAndHumidity)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    sensor.answer (reading (45, 0, 23, 6));

    adk::update (0);
    adk::update (1000);
    CHECK (!dht11.measured ());

    unsigned long before = arduino::now ();
    adk::update (1020);

    CHECK (dht11.measured ());
    CHECK (dht11.ok ());
    CHECK (dht11.humidity () == 45.0f);
    CHECK (dht11.temperature () == 23.6f);
    CHECK (sensor.released);
    CHECK (arduino::now () - before < 5000);
}

TEST (dht11SignalsAndReadsOnSchedule)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    sensor.answer (reading (45, 0, 23, 6));

    // The first reading starts a second after the first update.
    adk::update (100);
    adk::update (1099);
    CHECK (arduino::pin (Data).mode == INPUT_PULLUP);

    adk::update (1100);
    CHECK (arduino::pin (Data).mode == OUTPUT);
    CHECK (arduino::pin (Data).output == LOW);
    CHECK (sensor.modeAtLow == INPUT_PULLUP);

    adk::update (1119);
    CHECK (arduino::pin (Data).mode == OUTPUT);
    CHECK (!dht11.measured ());

    adk::update (1120);
    CHECK (arduino::pin (Data).mode == INPUT_PULLUP);
    CHECK (dht11.measured ());
    CHECK (dht11.ok ());

    adk::update (1121);
    CHECK (!dht11.measured ());

    // Then one every two seconds.
    adk::update (3099);
    CHECK (arduino::pin (Data).mode == INPUT_PULLUP);

    adk::update (3100);
    CHECK (arduino::pin (Data).mode == OUTPUT);

    adk::update (3120);
    CHECK (dht11.measured ());
    CHECK (dht11.ok ());
}

TEST (dht11WithoutASensorFailsQuickly)
{
    adk::Dht11 dht11 {Data};

    arduino::setCallCost (4);
    adk::setup ();
    adk::update (0);
    adk::update (1000);

    unsigned long before = arduino::now ();
    adk::update (1020);

    CHECK (dht11.measured ());
    CHECK (!dht11.ok ());
    CHECK (dht11.temperature () == 0.0f);
    CHECK (arduino::now () - before < 200);
}

TEST (dht11WithTheLineStuckLowFailsQuickly)
{
    adk::Dht11 dht11 {Data};

    arduino::setCallCost (4);
    arduino::onDigitalRead = [] (uint8_t) { return LOW; };

    adk::setup ();
    adk::update (0);
    adk::update (1000);

    unsigned long before = arduino::now ();
    adk::update (1020);

    CHECK (dht11.measured ());
    CHECK (!dht11.ok ());
    CHECK (arduino::now () - before < 250);
}

TEST (dht11SensorStoppingPartwayFails)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    sensor.answer (reading (45, 0, 23, 6), typical, 20);

    adk::update (0);
    readOnce (1000);

    CHECK (dht11.measured ());
    CHECK (!dht11.ok ());
}

TEST (dht11BadChecksumKeepsTheLastGoodReading)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    adk::update (0);

    sensor.answer (reading (45, 0, 23, 6));
    readOnce (1000);
    CHECK (dht11.ok ());

    std::vector<uint8_t> garbled = reading (50, 0, 30, 0);
    garbled[4] ^= 0x01;
    sensor.answer (garbled);
    readOnce (3000);

    CHECK (dht11.measured ());
    CHECK (!dht11.ok ());
    CHECK (dht11.humidity () == 45.0f);
    CHECK (dht11.temperature () == 23.6f);

    sensor.answer (reading (50, 0, 30, 0));
    readOnce (5000);

    CHECK (dht11.ok ());
    CHECK (dht11.humidity () == 50.0f);
    CHECK (dht11.temperature () == 30.0f);
}

TEST (dht11ReadsTemperaturesBelowZero)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    sensor.answer (reading (60, 0, 5, 0x83));
    adk::update (0);
    readOnce (1000);

    CHECK (dht11.ok ());
    CHECK (dht11.temperature () == -5.3f);
    CHECK (dht11.humidity () == 60.0f);
}

TEST (dht11ReadsFastAndSlowSensors)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    adk::update (0);

    sensor.answer (reading (33, 0, 21, 9), fast);
    readOnce (1000);
    CHECK (dht11.ok ());
    CHECK (dht11.temperature () == 21.9f);

    sensor.answer (reading (87, 0, 49, 1), slow);
    readOnce (3000);
    CHECK (dht11.ok ());
    CHECK (dht11.humidity () == 87.0f);
    CHECK (dht11.temperature () == 49.1f);
}

TEST (dht11StopLetsGoOfTheLine)
{
    adk::Dht11 dht11 {Data};
    Sensor     sensor;

    adk::setup ();
    sensor.answer (reading (45, 0, 23, 6));
    adk::update (0);
    adk::update (1000);
    CHECK (arduino::pin (Data).mode == OUTPUT);

    adk::stop ();
    CHECK (arduino::pin (Data).mode == INPUT_PULLUP);

    adk::update (1020);
    CHECK (!dht11.measured ());

    adk::update (2999);
    CHECK (arduino::pin (Data).mode == INPUT_PULLUP);

    readOnce (3000);
    CHECK (dht11.measured ());
    CHECK (dht11.ok ());
}
