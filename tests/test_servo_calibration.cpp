#include <servo_calibration.h>
#include <servo_configuration.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireStatus (
        adk::Status     status,
        adk::StatusCode expected,
        const char*     message)
    {
        require (status.error () == expected, message);
    }

    adk::ServoCalibrationConfig calibrationConfig ()
    {
        return {0, 180, 1000, 2000};
    }

    adk::ServoConfiguration configuration ()
    {
        return {{calibrationConfig (), 90}, 7};
    }

    uint16_t checksum (const uint8_t* bytes, size_t size)
    {
        uint16_t crc = 0xffff;

        for (size_t index = 0; index < size; ++index)
        {
            const uint16_t next = static_cast<uint16_t> (
                static_cast<uint16_t> (bytes[index]) << 8);
            crc ^= next;

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                crc = (crc & 0x8000)
                    ? static_cast<uint16_t> ((crc << 1) ^ 0x1021)
                    : static_cast<uint16_t> (crc << 1);
            }
        }

        return crc;
    }

    void write16 (uint8_t* bytes, uint16_t value)
    {
        bytes[0] = static_cast<uint8_t> (value);
        bytes[1] = static_cast<uint8_t> (value >> 8);
    }

    void testCalibration ()
    {
        const adk::ServoCalibration calibration (calibrationConfig ());

        require (calibration.valid (), "calibration valid");

        const adk::Result<uint16_t> minimum = calibration.pulseFor (0);
        const adk::Result<uint16_t> middle  = calibration.pulseFor (90);
        const adk::Result<uint16_t> maximum = calibration.pulseFor (180);

        require (minimum.ok () && minimum.value () == 1000, "minimum mapping");
        require (middle.ok () && middle.value () == 1500, "middle mapping");
        require (maximum.ok () && maximum.value () == 2000, "maximum mapping");
        require (
            calibration.pulseFor (181).error () ==
                adk::StatusCode::InvalidArgument,
            "position above range");

    }

    void testInvalidCalibration ()
    {
        const adk::ServoCalibrationConfig invalid[] = {
            {10, 10, 1000, 2000},
            {20, 10, 1000, 2000},
            {0, 180, 543, 2000},
            {0, 180, 1000, 2401},
            {0, 180, 1500, 1500},
            {0, 180, 2000, 1000}
        };

        for (const adk::ServoCalibrationConfig& config : invalid)
        {
            const adk::ServoCalibration calibration (config);

            require (!calibration.valid (), "invalid calibration rejected");
            require (
                calibration.pulseFor (10).error () ==
                    adk::StatusCode::InvalidArgument,
                "invalid calibration cannot map");
        }
    }

    void testBoundedServo ()
    {
        const adk::BoundedServoConfig config = {calibrationConfig (), 90};

        adk::BoundedServo             servo  (config);

        require (
            servo.snapshot ().status.error () ==
                adk::StatusCode::NotInitialized,
            "servo constructed inert");
        requireStatus (
            servo.command (90),
            adk::StatusCode::NotInitialized,
            "command before initialize");
        requireStatus (
            servo.initialize (),
            adk::StatusCode::Ok,
            "servo initialize");

        adk::BoundedServoSnapshot snapshot = servo.snapshot ();

        require (snapshot.intent == adk::BoundedServoIntent::Inactive, "safe intent");
        require (snapshot.position == 90, "safe position");
        require (snapshot.pulseUs == 1500, "safe pulse");

        requireStatus (
            servo.command (180),
            adk::StatusCode::Ok,
            "bounded command");
        snapshot = servo.snapshot ();

        require       (snapshot.intent == adk::BoundedServoIntent::Position, "position intent");
        require       (snapshot.position == 180, "commanded position");
        require       (snapshot.pulseUs == 2000, "commanded pulse");
        requireStatus (
            servo.initialize (),
            adk::StatusCode::Ok,
            "initialize remains idempotent");
        require (
            servo.snapshot ().position == 180,
            "reinitialize preserves active command");

        requireStatus (
            servo.command (181),
            adk::StatusCode::InvalidArgument,
            "out of range rejected");
        snapshot = servo.snapshot ();

        require (snapshot.intent == adk::BoundedServoIntent::Position, "intent preserved");
        require (snapshot.position == 180, "position preserved");
        require (snapshot.pulseUs == 2000, "pulse preserved");
        require (
            snapshot.status.error () == adk::StatusCode::InvalidArgument,
            "rejection observable");

        servo.shutdown ();

        snapshot = servo.snapshot ();

        require (snapshot.intent == adk::BoundedServoIntent::Inactive, "shutdown inactive");
        require (snapshot.position == 90, "shutdown restores safe position");
        require (snapshot.pulseUs == 0, "shutdown clears pulse intent");
        require (
            snapshot.status.error () == adk::StatusCode::NotInitialized,
            "shutdown status");
        requireStatus (
            servo.initialize (),
            adk::StatusCode::Ok,
            "servo restart");
    }

    void testInvalidSafePosition ()
    {
        const adk::BoundedServoConfig config = {calibrationConfig (), 181};

        adk::BoundedServo             servo  (config);

        requireStatus (
            servo.initialize (),
            adk::StatusCode::InvalidArgument,
            "invalid safe position");
        require (
            servo.snapshot ().intent == adk::BoundedServoIntent::Inactive,
            "invalid servo remains inactive");
    }

    void testConfigurationRoundTrip ()
    {
        adk::ServoConfigurationRecord record;

        require (record.empty (), "record starts empty");
        require (
            record.load ().error () == adk::StatusCode::NotInitialized,
            "empty load");
        requireStatus (
            record.save (configuration ()),
            adk::StatusCode::Ok,
            "save configuration");
        require (!record.empty (), "saved record not empty");

        const adk::Result<adk::ServoConfiguration> loaded = record.load ();

        require (loaded.ok (), "load saved configuration");
        require (loaded.value ().generation == 7, "generation round trip");
        require (
            loaded.value ().servo.safePosition == 90,
            "safe position round trip");
        require (
            loaded.value ().servo.calibration.minimumPosition == 0,
            "minimum position round trip");
        require (
            loaded.value ().servo.calibration.maximumPosition == 180,
            "maximum position round trip");
        require (
            loaded.value ().servo.calibration.minimumPulseUs == 1000,
            "minimum pulse round trip");
        require (
            loaded.value ().servo.calibration.maximumPulseUs == 2000,
            "maximum pulse round trip");

        record.clear ();

        require (record.empty (), "clear empties record");
    }

    void testPersistenceReplay ()
    {
        adk::ServoConfigurationRecord first;
        adk::ServoConfigurationRecord second;
        uint8_t firstBytes[adk::ServoConfigurationRecord::EncodedSize] = {};
        uint8_t secondBytes[adk::ServoConfigurationRecord::EncodedSize] = {};

        requireStatus (
            first.save (configuration ()),
            adk::StatusCode::Ok,
            "first deterministic save");
        requireStatus (
            first.exportTo (firstBytes, sizeof (firstBytes)),
            adk::StatusCode::Ok,
            "export saved record");
        requireStatus (
            second.import (firstBytes, sizeof (firstBytes)),
            adk::StatusCode::Ok,
            "import power-cycle record");
        require (second.load ().ok (), "imported record loads");

        requireStatus (
            second.exportTo (secondBytes, sizeof (secondBytes)),
            adk::StatusCode::Ok,
            "re-export record");
        require (
            std::memcmp (firstBytes, secondBytes, sizeof (firstBytes)) == 0,
            "record bytes deterministic");
    }

    void testPersistenceFailures ()
    {
        adk::ServoConfigurationRecord record;
        uint8_t bytes[adk::ServoConfigurationRecord::EncodedSize] = {};

        requireStatus (
            record.exportTo (bytes, sizeof (bytes)),
            adk::StatusCode::NotInitialized,
            "empty export");
        requireStatus (
            record.import (nullptr, sizeof (bytes)),
            adk::StatusCode::InvalidArgument,
            "null import");
        requireStatus (
            record.import (bytes, sizeof (bytes) - 1),
            adk::StatusCode::CapacityExceeded,
            "short import");
        requireStatus (
            record.save (configuration ()),
            adk::StatusCode::Ok,
            "failure fixture save");
        requireStatus (
            record.exportTo (nullptr, sizeof (bytes)),
            adk::StatusCode::InvalidArgument,
            "null export");
        requireStatus (
            record.exportTo (bytes, sizeof (bytes) - 1),
            adk::StatusCode::CapacityExceeded,
            "small export");
        requireStatus (
            record.exportTo (bytes, sizeof (bytes)),
            adk::StatusCode::Ok,
            "failure fixture export");

        bytes[2] = adk::ServoConfigurationRecord::FormatVersion + 1;
        requireStatus (
            record.import (bytes, sizeof (bytes)),
            adk::StatusCode::Ok,
            "unknown version import");
        require (
            record.load ().error () == adk::StatusCode::Unsupported,
            "unknown version rejected");

        bytes[2] = adk::ServoConfigurationRecord::FormatVersion;
        bytes[8] ^= 1;
        requireStatus (
            record.import (bytes, sizeof (bytes)),
            adk::StatusCode::Ok,
            "corrupt record import");
        require (
            record.load ().error () == adk::StatusCode::HardwareFailure,
            "checksum corruption rejected");
    }

    void testSemanticCorruption ()
    {
        adk::ServoConfigurationRecord record;
        uint8_t bytes[adk::ServoConfigurationRecord::EncodedSize] = {};

        requireStatus (
            record.save (configuration ()),
            adk::StatusCode::Ok,
            "semantic fixture save");
        requireStatus (
            record.exportTo (bytes, sizeof (bytes)),
            adk::StatusCode::Ok,
            "semantic fixture export");

        write16 (&bytes[16], 181);
        write16 (&bytes[18], checksum (bytes, 18));

        requireStatus (
            record.import (bytes, sizeof (bytes)),
            adk::StatusCode::Ok,
            "semantic corruption import");
        require (
            record.load ().error () == adk::StatusCode::HardwareFailure,
            "semantic corruption rejected");
    }

    void testEveryEncodedByteCorruption ()
    {
        adk::ServoConfigurationRecord source;
        uint8_t golden[adk::ServoConfigurationRecord::EncodedSize] = {};

        requireStatus (
            source.save (configuration ()),
            adk::StatusCode::Ok,
            "byte corruption fixture save");
        requireStatus (
            source.exportTo (golden, sizeof (golden)),
            adk::StatusCode::Ok,
            "byte corruption fixture export");

        for (size_t index = 0; index < sizeof (golden); ++index)
        {
            adk::ServoConfigurationRecord record;
            uint8_t corrupted[sizeof (golden)];

            std::memcpy (corrupted, golden, sizeof (corrupted));
            corrupted[index] ^= 1;

            requireStatus (
                record.import (corrupted, sizeof (corrupted)),
                adk::StatusCode::Ok,
                "corrupt bytes staged");

            const adk::StatusCode expected =
                index == 2
                    ? adk::StatusCode::Unsupported
                    : adk::StatusCode::HardwareFailure;

            require (
                record.load ().error () == expected,
                "every byte corruption diagnosed");
        }
    }

    void testErasedAndOversizedRecords ()
    {
        uint8_t erasedLow[adk::ServoConfigurationRecord::EncodedSize] = {};
        uint8_t erasedHigh[adk::ServoConfigurationRecord::EncodedSize];
        uint8_t oversized[adk::ServoConfigurationRecord::EncodedSize + 1] = {};
        adk::ServoConfigurationRecord record;

        std::memset (erasedHigh, 0xff, sizeof (erasedHigh));

        requireStatus (
            record.import (erasedLow, sizeof (erasedLow)),
            adk::StatusCode::Ok,
            "zero-erased bytes staged");
        require (
            record.load ().error () == adk::StatusCode::HardwareFailure,
            "zero-erased record rejected");
        requireStatus (
            record.import (erasedHigh, sizeof (erasedHigh)),
            adk::StatusCode::Ok,
            "one-erased bytes staged");
        require (
            record.load ().error () == adk::StatusCode::HardwareFailure,
            "one-erased record rejected");
        requireStatus (
            record.import (oversized, sizeof (oversized)),
            adk::StatusCode::CapacityExceeded,
            "oversized record rejected");
    }

    void testGenerationEndpoints ()
    {
        adk::ServoConfigurationRecord record;
        adk::ServoConfiguration       config = configuration ();

        config.generation = UINT32_MAX;
        requireStatus (
            record.save (config),
            adk::StatusCode::Ok,
            "maximum generation save");
        require (
            record.load ().value ().generation == UINT32_MAX,
            "maximum generation round trip");

        config.generation = 0;
        requireStatus (
            record.save (config),
            adk::StatusCode::Ok,
            "wrapped generation save");
        require (
            record.load ().value ().generation == 0,
            "wrapped generation round trip");
    }

    void testCorruptImportReplay ()
    {
        adk::ServoConfigurationRecord source;
        adk::ServoConfigurationRecord first;
        adk::ServoConfigurationRecord second;
        uint8_t corrupted[adk::ServoConfigurationRecord::EncodedSize] = {};
        uint8_t exported[adk::ServoConfigurationRecord::EncodedSize] = {};

        requireStatus (
            source.save (configuration ()),
            adk::StatusCode::Ok,
            "corrupt replay fixture save");
        requireStatus (
            source.exportTo (corrupted, sizeof (corrupted)),
            adk::StatusCode::Ok,
            "corrupt replay fixture export");
        corrupted[12] ^= 1;

        requireStatus (
            first.save (configuration ()),
            adk::StatusCode::Ok,
            "prior valid record");
        requireStatus (
            first.import (corrupted, sizeof (corrupted)),
            adk::StatusCode::Ok,
            "corrupt import replaces prior bytes");
        requireStatus (
            second.import (corrupted, sizeof (corrupted)),
            adk::StatusCode::Ok,
            "corrupt replay import");

        const adk::Result<adk::ServoConfiguration> firstLoad  = first.load  ();
        const adk::Result<adk::ServoConfiguration> secondLoad = second.load ();

        require (
            firstLoad.error () == adk::StatusCode::HardwareFailure,
            "replaced record fails validation");
        require (
            secondLoad.error () == firstLoad.error (),
            "corrupt replay status deterministic");
        require (
            secondLoad.value ().generation == firstLoad.value ().generation,
            "corrupt replay fallback deterministic");
        requireStatus (
            first.exportTo (exported, sizeof (exported)),
            adk::StatusCode::Ok,
            "staged corrupt bytes export");
        require (
            std::memcmp (exported, corrupted, sizeof (exported)) == 0,
            "import stages before validation");
    }

    void testInvalidSavePreservesRecord ()
    {
        adk::ServoConfigurationRecord record;
        adk::ServoConfiguration       invalid = configuration ();

        requireStatus (
            record.save (configuration ()),
            adk::StatusCode::Ok,
            "preservation fixture");
        invalid.servo.safePosition = 181;
        requireStatus (
            record.save (invalid),
            adk::StatusCode::InvalidArgument,
            "invalid save rejected");
        require (record.load ().ok (), "prior record preserved");
        require (
            record.load ().value ().servo.safePosition == 90,
            "preserved record unchanged");
    }
}

int main ()
{
    testCalibration                ();
    testInvalidCalibration         ();
    testBoundedServo               ();
    testInvalidSafePosition        ();
    testConfigurationRoundTrip     ();
    testPersistenceReplay          ();
    testPersistenceFailures        ();
    testSemanticCorruption         ();
    testEveryEncodedByteCorruption ();
    testErasedAndOversizedRecords  ();
    testGenerationEndpoints        ();
    testCorruptImportReplay        ();
    testInvalidSavePreservesRecord ();

    std::cout << "All ADK servo calibration tests passed.\n";
}
