#include "check.h"

#include <adk/ds18b20.h>

#include <Arduino.h>

#include <algorithm>
#include <vector>

namespace {

    using Bytes = std::vector<uint8_t>;

    const uint16_t PowerOn = 0x0550;

    // The Dallas CRC-8 again, written differently, so the test does not take
    // the code under test on trust.
    uint8_t crc8 (const uint8_t* bytes, int length)
    {
        uint8_t crc = 0;

        for (int index = 0; index < length; ++index)
        {
            for (int bit = 0; bit < 8; ++bit)
            {
                bool mix = ((crc ^ (bytes[index] >> bit)) & 1) != 0;
                crc      = static_cast<uint8_t> ((crc >> 1) ^ (mix ? 0x8C : 0));
            }
        }

        return crc;
    }

    // A DS18B20 on a pin, played through the fake core's hooks and as strict
    // as its datasheet allows. The master's digitalWrite (LOW) starts a slot
    // or a reset, and the master samples the line in each; a pin that is
    // still an output then is the master writing 0. The sensor shows its
    // presence pulse only 60-75 us after a reset's release, and holds a 0 bit
    // only until 15 us into a slot, so a master that samples late misses them.
    //
    // pinMode () costs no time on the host, so tests charge 3 us for each
    // digitalWrite () and digitalRead (), about what each costs on the Mega,
    // and together about what the Mega's pinMode () calls add.
    struct Sensor
    {
        explicit Sensor (adk::Pin data)
            : pin (data)
        {
            load (PowerOn);

            arduino::onDigitalWrite = [this] (uint8_t line, uint8_t level) { fall (line, level); };
            arduino::onDigitalRead  = [this] (uint8_t line) { return sample (line); };
        }

        ~Sensor ()
        {
            arduino::onDigitalWrite = nullptr;
            arduino::onDigitalRead  = nullptr;
        }

        Sensor            (const Sensor&) = delete;
        Sensor& operator= (const Sensor&) = delete;

        // What the next Read Scratchpad returns: the temperature in
        // sixteenths of a degree, the power-up settings, and the CRC.
        void load (uint16_t sixteenths)
        {
            const uint8_t bytes [8] = {uint8_t (sixteenths), uint8_t (sixteenths >> 8),
                                       0x4B, 0x46, 0x7F, 0xFF, 0x0C, 0x10};

            memcpy (scratchpad, bytes, sizeof bytes);
            scratchpad[8] = crc8 (scratchpad, 8);
        }

        void fall (uint8_t written, uint8_t value)
        {
            if (written != pin)
            {
                return;
            }

            unsigned long now = arduino::now ();

            // Only ever pull the line low, and only once it has been let go.
            faults += (value != LOW || arduino::pin (pin).mode == OUTPUT) ? 1 : 0;

            if (afterReset)
            {
                shortestReset = std::min (shortestReset, now - fellAt);
            }
            else if (fellAt != 0)
            {
                shortestSlot = std::min (shortestSlot, now - fellAt);
            }

            fellAt     = now;
            afterReset = false;
        }

        int sample (uint8_t read)
        {
            if (read != pin)
            {
                return arduino::pin (read).input;
            }

            unsigned long elapsed  = arduino::now () - fellAt;
            bool          released = arduino::pin (pin).mode != OUTPUT;

            if (elapsed >= 480)
            {
                return presence (elapsed - 480, released);
            }

            slowestSample = std::max (slowestSample, elapsed);

            if (stuckLow)
            {
                return LOW;
            }

            if (!released)
            {
                receive (false);
                return LOW;
            }

            if (phase == Phase::Sending)
            {
                bool bit = (scratchpad[sent / 8] >> (sent % 8)) & 1;
                phase    = (++sent == 72) ? Phase::Idle : phase;
                return (bit || elapsed > 15) ? HIGH : LOW;
            }

            receive (true);
            return HIGH;
        }

        int presence (unsigned long afterRelease, bool released)
        {
            faults      += released ? 0 : 1;
            presenceAt   = afterRelease;
            afterReset   = true;
            phase        = connected ? Phase::Rom : Phase::Idle;
            bits         = 0;
            ++resets;

            bool pulsing = connected && afterRelease >= 60 && afterRelease <= 75;
            return (pulsing || stuckLow) ? LOW : HIGH;
        }

        void receive (bool bit)
        {
            if (phase == Phase::Idle || phase == Phase::Sending)
            {
                ++faults;
                return;
            }

            shift = static_cast<uint8_t> ((shift >> 1) | (bit ? 0x80 : 0));

            if (++bits < 8)
            {
                return;
            }

            bits = 0;
            received.push_back (shift);

            if (phase == Phase::Rom)
            {
                faults += shift == 0xCC ? 0 : 1;
                phase   = shift == 0xCC ? Phase::Function : Phase::Idle;
            }
            else
            {
                faults += (shift == 0x44 || shift == 0xBE) ? 0 : 1;
                phase   = shift == 0xBE ? Phase::Sending : Phase::Idle;
                sent    = 0;
            }
        }

        enum struct Phase
        {
            Idle,
            Rom,
            Function,
            Sending
        };

        adk::Pin pin;
        bool     connected = true;
        bool     stuckLow  = false;
        uint8_t  scratchpad [9];

        Bytes         received;
        int           resets        = 0;
        int           faults        = 0;
        unsigned long presenceAt    = 0;
        unsigned long slowestSample = 0;
        unsigned long shortestSlot  = 1000000;
        unsigned long shortestReset = 1000000;

        Phase         phase      = Phase::Idle;
        unsigned long fellAt     = 0;
        bool          afterReset = false;
        uint8_t       shift      = 0;
        int           bits       = 0;
        int           sent       = 0;
    };

    // Update once a millisecond over [from, to) and list when readings came.
    std::vector<adk::Millis> readingsBetween (const adk::Ds18b20& thermometer, adk::Millis from,
                                              adk::Millis to)
    {
        std::vector<adk::Millis> times;

        for (adk::Millis now = from; now < to; ++now)
        {
            adk::update (now);

            if (thermometer.measured ())
            {
                times.push_back (now);
            }
        }

        return times;
    }
}

TEST (ds18b20ClaimsItsPinWithoutThePullUp)
{
    adk::Ds18b20 thermometer {7};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (7).mode == INPUT);
    CHECK (!thermometer.ok ());
    CHECK (!thermometer.measured ());
}

TEST (ds18b20StartsAConversionOnTheFirstUpdate)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    adk::update (0);

    CHECK (sensor.received == (Bytes {0xCC, 0x44}));
    CHECK (sensor.resets == 1);
    CHECK (sensor.faults == 0);
    CHECK (arduino::pin (7).mode == INPUT);
    CHECK (!thermometer.measured ());
}

TEST (ds18b20KeepsTo1WireTiming)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.load (0x0191);
    adk::update (0);
    adk::update (750);

    CHECK (thermometer.ok ());
    CHECK (sensor.faults == 0);
    CHECK (sensor.presenceAt >= 60 && sensor.presenceAt <= 75);
    CHECK (sensor.shortestReset >= 960);
    CHECK (sensor.slowestSample <= 15);
    CHECK (sensor.shortestSlot >= 61);
}

TEST (ds18b20ReadsTheTemperature750msLater)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.load (0x0191);
    CHECK (sensor.scratchpad[8] == 0x70);

    adk::update (0);
    adk::update (749);
    CHECK (!thermometer.measured ());
    CHECK (sensor.received == (Bytes {0xCC, 0x44}));

    adk::update (750);
    CHECK (thermometer.measured ());
    CHECK (thermometer.ok ());
    CHECK (thermometer.celsius () == 25.0625f);
    CHECK (sensor.received == (Bytes {0xCC, 0x44, 0xCC, 0xBE, 0xCC, 0x44}));

    adk::update (751);
    CHECK (!thermometer.measured ());
    CHECK (thermometer.ok ());
}

TEST (ds18b20ReadsBelowZero)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.load (0xFF5E);
    CHECK (sensor.scratchpad[8] == 0x6A);

    adk::update (0);
    adk::update (750);

    CHECK (thermometer.ok ());
    CHECK (thermometer.celsius () == -10.125f);
}

TEST (ds18b20MeasuresEvery750ms)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.load (0x0191);

    CHECK (readingsBetween (thermometer, 0, 3000) == (std::vector<adk::Millis> {750, 1500, 2250}));
    CHECK (sensor.resets == 7);
    CHECK (sensor.faults == 0);
}

TEST (ds18b20KeepsTheLastReadingOnACrcError)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.load (0x0191);
    adk::update (0);
    adk::update (750);
    CHECK (thermometer.celsius () == 25.0625f);

    sensor.load (0x0200);
    sensor.scratchpad[8] ^= 0x01;
    adk::update (1500);
    CHECK (thermometer.measured ());
    CHECK (!thermometer.ok ());
    CHECK (thermometer.celsius () == 25.0625f);

    sensor.load (0x0200);
    adk::update (2250);
    CHECK (thermometer.measured ());
    CHECK (thermometer.ok ());
    CHECK (thermometer.celsius () == 32.0f);
}

TEST (ds18b20WithoutASensorFailsAReadingEvery750msUntilOneAnswers)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.connected = false;
    sensor.load (0x0191);

    CHECK (readingsBetween (thermometer, 0, 1600) == (std::vector<adk::Millis> {0, 750, 1500}));
    CHECK (!thermometer.ok ());
    CHECK (thermometer.celsius () == 0.0f);
    CHECK (sensor.received.empty ());
    CHECK (sensor.resets == 3);

    sensor.connected = true;
    CHECK (readingsBetween (thermometer, 1600, 3100) == (std::vector<adk::Millis> {3000}));
    CHECK (thermometer.ok ());
}

TEST (ds18b20KeepsTheLastReadingWhenTheSensorGoes)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.load (0x0191);
    adk::update (0);
    adk::update (750);

    sensor.connected = false;
    adk::update (1500);

    CHECK (thermometer.measured ());
    CHECK (!thermometer.ok ());
    CHECK (thermometer.celsius () == 25.0625f);
}

TEST (ds18b20SkipsAFirstReadingOf85Degrees)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    adk::update (0);
    adk::update (750);
    CHECK (thermometer.measured ());
    CHECK (!thermometer.ok ());

    adk::update (1500);
    CHECK (thermometer.measured ());
    CHECK (thermometer.ok ());
    CHECK (thermometer.celsius () == 85.0f);
}

TEST (ds18b20RejectsALineHeldLow)
{
    arduino::setCallCost (3);
    Sensor       sensor      {7};
    adk::Ds18b20 thermometer {7};

    adk::setup ();
    sensor.stuckLow = true;

    CHECK (readingsBetween (thermometer, 0, 1600) == (std::vector<adk::Millis> {750, 1500}));
    CHECK (!thermometer.ok ());
}
