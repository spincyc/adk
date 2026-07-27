#include "dht11_sensor.h"

#include <Arduino.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <vector>

namespace {

    struct Pulse
    {
        adk::Level level;
        uint16_t   duration;
    };

    struct ScriptedTransport final : adk::Dht11Transport
    {
        std::vector<Pulse> pulses;
        size_t             pulseIndex    = 0;
        uint16_t           pulseAge      = 0;
        uint32_t           time          = 0;
        adk::Status        driveStatus   = adk::Status::Ok;
        adk::Status        releaseStatus = adk::Status::Ok;
        int                readFailureAt = -1;
        int                reads         = 0;
        int                drives        = 0;
        int                releases      = 0;

        adk::Status driveLow (adk::PinId) noexcept override
        {
            ++drives;
            return driveStatus;
        }

        adk::Status release (adk::PinId) noexcept override
        {
            ++releases;
            pulseIndex = 0;
            pulseAge   = 0;
            return releaseStatus;
        }

        adk::Result<adk::Level> read (adk::PinId) noexcept override
        {
            if (reads++ == readFailureAt)
            {
                return {adk::Status::HardwareFailure, adk::Level::Low};
            }

            const adk::Level level = pulseIndex < pulses.size ()
                                         ? pulses[pulseIndex].level
                                         : adk::Level::High;
            return {adk::Status::Ok, level};
        }

        void waitMicroseconds (uint16_t duration) noexcept override
        {
            time += duration;

            while (duration != 0 && pulseIndex < pulses.size ())
            {
                const uint16_t remaining =
                    static_cast<uint16_t> (pulses[pulseIndex].duration - pulseAge);
                const uint16_t consumed = duration < remaining ? duration : remaining;

                pulseAge = static_cast<uint16_t> (pulseAge + consumed);
                duration = static_cast<uint16_t> (duration - consumed);

                if (pulseAge == pulses[pulseIndex].duration)
                {
                    ++pulseIndex;
                    pulseAge = 0;
                }
            }
        }

        uint32_t microseconds () const noexcept override
        {
            return time;
        }
    };

    void appendFrame (ScriptedTransport& transport,
                      const uint8_t (&bytes)[5],
                      uint16_t zeroWidth    = 27,
                      uint16_t oneWidth     = 70,
                      uint16_t responseLow  = 80,
                      uint16_t responseHigh = 80,
                      uint16_t bitLow       = 50)
    {
        transport.pulses.push_back ({adk::Level::High, 20});
        transport.pulses.push_back ({adk::Level::Low, responseLow});
        transport.pulses.push_back ({adk::Level::High, responseHigh});

        for (uint8_t byte : bytes)
        {
            for (uint8_t mask = 0x80U; mask != 0; mask >>= 1U)
            {
                transport.pulses.push_back ({adk::Level::Low, bitLow});
                transport.pulses.push_back (
                    {adk::Level::High, (byte & mask) != 0 ? oneWidth : zeroWidth});
            }
        }

        transport.pulses.push_back ({adk::Level::Low, 50});
    }

    void resetArduinoAt (uint32_t milliseconds)
    {
        adk::test::arduino::reset     ();
        adk::test::arduino::setTimeUs (static_cast<uint64_t> (milliseconds) * 1000U);
    }

    void testLifecycleAndScheduling ()
    {
        resetArduinoAt (100);
        adk::ResourceRegistry resources;
        ScriptedTransport     transport;
        const uint8_t         frame[5] = {45, 3, 23, 4, 75};
        appendFrame             (transport, frame);
        adk::Dht11Sensor sensor (resources, 22, transport);

        assert (sensor.initialize () == adk::Status::Ok);
        assert (sensor.initialize () == adk::Status::Ok);
        assert (adk::test::arduino::mode (22) == INPUT);
        assert (sensor.update (adk::TimePoint (100)) == adk::Status::Ok);
        assert (sensor.update (adk::TimePoint (1099)) == adk::Status::Ok);
        assert (transport.drives == 0);
        assert (sensor.update (adk::TimePoint (1100)) == adk::Status::Ok);
        assert (transport.drives == 1);

        const adk::ClimateSample sample =
            sensor.sample (adk::TimePoint (1100), adk::Duration (100));
        assert (sample.temperatureCentiCelsius == 2340);
        assert (sample.humidityPermille == 453);
        assert (sample.state == adk::ClimateSampleState::Valid);

        assert          (sensor.update (adk::TimePoint (2099)) == adk::Status::Ok);
        assert          (transport.drives == 1);
        sensor.shutdown ();
        sensor.shutdown ();
        assert          (!sensor.initialized ());
        assert          (adk::test::arduino::mode (22) == INPUT);
        assert          (!resources.claimed ({adk::ResourceKind::Pin, 22}));
    }

    void testClaimsAndDestruction ()
    {
        resetArduinoAt (0);
        adk::ResourceRegistry resources;
        ScriptedTransport     transport;
        adk::ResourceClaim    busy;

        assert (resources.claim ({adk::ResourceKind::Pin, 22}, busy) ==
                adk::Status::Ok);
        adk::Dht11Sensor blocked (resources, 22, transport);
        assert                   (blocked.initialize () == adk::Status::ResourceBusy);
        assert                   (adk::test::arduino::trace ().empty ());
        busy.release             ();

        {
            adk::Dht11Sensor sensor (resources, 22, transport);
            assert                  (sensor.initialize () == adk::Status::Ok);
        }

        assert (!resources.claimed ({adk::ResourceKind::Pin, 22}));

        adk::Dht11Sensor invalid (resources, 70, transport);
        assert                   (invalid.initialize () == adk::Status::InvalidPin);
    }

    void testTimeoutChecksumAndRecovery ()
    {
        resetArduinoAt (0);
        adk::ResourceRegistry resources;
        ScriptedTransport     transport;
        adk::Dht11Sensor      sensor (resources, 22, transport);
        assert                       (sensor.initialize () == adk::Status::Ok);
        assert                       (sensor.update (adk::TimePoint (0)) ==
                adk::Status::Ok);

        assert (sensor.update (adk::TimePoint (1000)) == adk::Status::HardwareFailure);
        assert (sensor.sample (adk::TimePoint (1000), adk::Duration (1000)).state ==
                adk::ClimateSampleState::TransportTimeout);
        assert (adk::test::arduino::mode (22) == INPUT);

        const uint8_t bad[5] = {40, 0, 20, 0, 61};
        transport.pulses.clear ();
        appendFrame            (transport, bad);
        assert                 (sensor.update (adk::TimePoint (2000)) == adk::Status::HardwareFailure);
        assert                 (sensor.sample (adk::TimePoint (2000), adk::Duration (1000)).state ==
                adk::ClimateSampleState::ChecksumFailure);

        const uint8_t good[5] = {40, 0, 20, 0, 60};
        transport.pulses.clear ();
        appendFrame            (transport, good);
        assert                 (sensor.update (adk::TimePoint (3000)) == adk::Status::Ok);
        assert                 (sensor.sample (adk::TimePoint (3000), adk::Duration (1000)).state ==
                adk::ClimateSampleState::Valid);
    }

    void testPulseAndOperationFailures ()
    {
        resetArduinoAt (0);
        adk::ResourceRegistry resources;
        ScriptedTransport     transport;
        const uint8_t         frame[5] = {50, 0, 25, 0, 75};
        appendFrame             (transport, frame, 47, 70);
        adk::Dht11Sensor sensor (resources, 22, transport);
        assert                  (sensor.initialize () == adk::Status::Ok);
        assert                  (sensor.update (adk::TimePoint (0)) ==
                adk::Status::Ok);
        assert                  (sensor.update (adk::TimePoint (1000)) == adk::Status::HardwareFailure);
        assert                  (adk::test::arduino::mode (22) == INPUT);

        ScriptedTransport releaseFailure;
        releaseFailure.releaseStatus = adk::Status::HardwareFailure;
        adk::Dht11Sensor releaseSensor (resources, 23, releaseFailure);
        assert                         (releaseSensor.initialize () == adk::Status::Ok);
        assert                         (releaseSensor.update (adk::TimePoint (0)) ==
                adk::Status::Ok);
        assert                         (releaseSensor.update (adk::TimePoint (1000)) ==
                adk::Status::HardwareFailure);
        assert (adk::test::arduino::mode (23) == INPUT);

        ScriptedTransport readFailure;
        appendFrame (readFailure, frame);
        readFailure.readFailureAt = 10;
        adk::Dht11Sensor readSensor (resources, 24, readFailure);
        assert                      (readSensor.initialize () == adk::Status::Ok);
        assert                      (readSensor.update (adk::TimePoint (0)) ==
                adk::Status::Ok);
        assert                      (readSensor.update (adk::TimePoint (1000)) ==
                adk::Status::HardwareFailure);
        assert (adk::test::arduino::mode (24) == INPUT);
    }

    void testRangeStaleAndRollover ()
    {
        resetArduinoAt (0xfffffc17U);
        adk::ResourceRegistry resources;
        ScriptedTransport     transport;
        const uint8_t         humid[5] = {100, 1, 20, 0, 121};
        appendFrame             (transport, humid);
        adk::Dht11Sensor sensor (resources, 22, transport);
        assert                  (sensor.initialize () == adk::Status::Ok);
        assert                  (sensor.update (adk::TimePoint (0xfffffc17U)) ==
                adk::Status::Ok);
        assert                  (sensor.update (adk::TimePoint (0xffffffffU)) ==
                adk::Status::InvalidArgument);

        resetArduinoAt (0);
        ScriptedTransport validTransport;
        const uint8_t     valid[5] = {50, 0, 25, 0, 75};
        appendFrame (validTransport, valid);
        adk::ResourceRegistry secondResources;
        adk::Dht11Sensor      validSensor (secondResources, 22, validTransport);
        assert                            (validSensor.initialize () == adk::Status::Ok);
        assert                            (validSensor.update (adk::TimePoint (0)) ==
                adk::Status::Ok);
        assert                            (validSensor.update (adk::TimePoint (1000)) == adk::Status::Ok);
        assert                            (validSensor.sample (adk::TimePoint (1100), adk::Duration (100)).state ==
                adk::ClimateSampleState::Valid);
        assert (validSensor.sample (adk::TimePoint (1101), adk::Duration (100)).state ==
                adk::ClimateSampleState::Stale);
    }
} // namespace

static_assert (!std::is_copy_constructible<adk::Dht11Sensor>::value,
               "Dht11Sensor owns one pin");
static_assert (!std::is_move_constructible<adk::Dht11Sensor>::value,
               "Dht11Sensor has a stable address");

int main ()
{
    testLifecycleAndScheduling     ();
    testClaimsAndDestruction       ();
    testTimeoutChecksumAndRecovery ();
    testPulseAndOperationFailures  ();
    testRangeStaleAndRollover      ();
}
