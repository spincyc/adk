#include "servo_configuration.h"

#include <string.h>

namespace adk {

    namespace {
        const uint8_t magicFirst  = 0x41;
        const uint8_t magicSecond = 0x53;

        void write16 (uint8_t* output, uint16_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value);
            output[1] = static_cast<uint8_t> (value >> 8);
        }

        void write32 (uint8_t* output, uint32_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value);
            output[1] = static_cast<uint8_t> (value >> 8);
            output[2] = static_cast<uint8_t> (value >> 16);
            output[3] = static_cast<uint8_t> (value >> 24);
        }

        uint16_t read16 (const uint8_t* input) noexcept
        {
            return static_cast<uint16_t> (
                static_cast<uint16_t> (input[0]) |
                static_cast<uint16_t> (
                    static_cast<uint16_t> (input[1]) << 8));
        }

        uint32_t read32 (const uint8_t* input) noexcept
        {
            return static_cast<uint32_t> (input[0]) |
                   (static_cast<uint32_t> (input[1]) << 8) |
                   (static_cast<uint32_t> (input[2]) << 16) |
                   (static_cast<uint32_t> (input[3]) << 24);
        }

        uint16_t checksum (const uint8_t* bytes, size_t size) noexcept
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

        ServoConfiguration emptyConfiguration () noexcept
        {
            const ServoCalibrationConfig calibration = {0, 1, 1000, 2000};
            return {{calibration, 0}, 0};
        }
    }

    ServoConfigurationRecord::ServoConfigurationRecord () noexcept
        : bytes_ {}
        , empty_ (true)
    {
    }

    void ServoConfigurationRecord::clear () noexcept
    {
        memset (bytes_, 0, sizeof (bytes_));
        empty_ = true;
    }

    bool ServoConfigurationRecord::empty () const noexcept
    {
        return empty_;
    }

    Status ServoConfigurationRecord::save (
        const ServoConfiguration& config) noexcept
    {
        const ServoCalibration calibration (config.servo.calibration);

        if (!calibration.valid () ||
            !calibration.pulseFor (config.servo.safePosition).ok ())
        {
            return StatusCode::InvalidArgument;
        }

        clear ();
        bytes_[0] = magicFirst;
        bytes_[1] = magicSecond;
        bytes_[2] = FormatVersion;
        bytes_[3] = static_cast<uint8_t> (EncodedSize);
        write32 (&bytes_[4], config.generation);
        write16 (&bytes_[8], config.servo.calibration.minimumPosition);
        write16 (&bytes_[10], config.servo.calibration.maximumPosition);
        write16 (&bytes_[12], config.servo.calibration.minimumPulseUs);
        write16 (&bytes_[14], config.servo.calibration.maximumPulseUs);
        write16 (&bytes_[16], config.servo.safePosition);
        write16 (&bytes_[18], checksum (bytes_, 18));
        empty_ = false;
        return StatusCode::Ok;
    }

    Result<ServoConfiguration> ServoConfigurationRecord::load () const noexcept
    {
        ServoConfiguration result = emptyConfiguration ();

        if (empty_)
        {
            return {StatusCode::NotInitialized, result};
        }

        if (bytes_[0] != magicFirst || bytes_[1] != magicSecond)
        {
            return {StatusCode::HardwareFailure, result};
        }

        if (bytes_[2] != FormatVersion)
        {
            return {StatusCode::Unsupported, result};
        }

        if (bytes_[3] != EncodedSize ||
            read16 (&bytes_[18]) != checksum (bytes_, 18))
        {
            return {StatusCode::HardwareFailure, result};
        }

        result.generation                         = read32 (&bytes_[4]);
        result.servo.calibration.minimumPosition = read16  (&bytes_[8]);
        result.servo.calibration.maximumPosition = read16  (&bytes_[10]);
        result.servo.calibration.minimumPulseUs  = read16  (&bytes_[12]);
        result.servo.calibration.maximumPulseUs  = read16  (&bytes_[14]);
        result.servo.safePosition                 = read16 (&bytes_[16]);

        const ServoCalibration calibration (result.servo.calibration);

        if (!calibration.valid () ||
            !calibration.pulseFor (result.servo.safePosition).ok ())
        {
            return {StatusCode::HardwareFailure, emptyConfiguration ()};
        }

        return {StatusCode::Ok, result};
    }

    Status ServoConfigurationRecord::import (
        const uint8_t* bytes,
        size_t         size) noexcept
    {
        if (bytes == nullptr)
        {
            return StatusCode::InvalidArgument;
        }

        if (size != EncodedSize)
        {
            return StatusCode::CapacityExceeded;
        }

        memcpy (bytes_, bytes, EncodedSize);
        memset (bytes_ + EncodedSize, 0, Capacity - EncodedSize);
        empty_ = false;
        return StatusCode::Ok;
    }

    Status ServoConfigurationRecord::exportTo (
        uint8_t* bytes,
        size_t   capacity) const noexcept
    {
        if (bytes == nullptr)
        {
            return StatusCode::InvalidArgument;
        }

        if (empty_)
        {
            return StatusCode::NotInitialized;
        }

        if (capacity < EncodedSize)
        {
            return StatusCode::CapacityExceeded;
        }

        memcpy (bytes, bytes_, EncodedSize);
        return StatusCode::Ok;
    }
}
