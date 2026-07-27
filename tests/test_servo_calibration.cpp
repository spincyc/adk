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
            crc ^= static_cast<uint16_t> (bytes[index]) << 8;

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
    testInvalidSavePreservesRecord ();

    std::cout << "All ADK servo calibration tests passed.\n";
}
