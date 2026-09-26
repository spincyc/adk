#include "mpu6050.h"

#include "i2c.h"

#include <math.h>

namespace adk {

    namespace {

        // The MPU-6050's registers.
        constexpr uint8_t LowPass         = 0x1A;
        constexpr uint8_t GyroRange       = 0x1B;
        constexpr uint8_t AccelRange      = 0x1C;
        constexpr uint8_t FirstReading    = 0x3B;
        constexpr uint8_t PowerManagement = 0x6B;

        // The QMI8658's, and what its WHO_AM_I holds.
        constexpr uint8_t QmiWhoAmI       = 0x00;
        constexpr uint8_t QmiControl1     = 0x02;
        constexpr uint8_t QmiControl2     = 0x03;
        constexpr uint8_t QmiControl3     = 0x04;
        constexpr uint8_t QmiControl5     = 0x06;
        constexpr uint8_t QmiControl7     = 0x08;
        constexpr uint8_t QmiFirstReading = 0x33;
        constexpr uint8_t QmiIdentity     = 0x05;

        constexpr Millis Period           = 20;
        constexpr float  DegreesPerRadian = 57.29578f;

        int16_t bigEndian (const uint8_t* bytes)
        {
            return static_cast<int16_t> ((bytes[0] << 8) | bytes[1]);
        }

        int16_t littleEndian (const uint8_t* bytes)
        {
            return static_cast<int16_t> ((bytes[1] << 8) | bytes[0]);
        }

        // At +-2 g both chips count 16384 per g.
        int16_t milliG (int16_t counts)
        {
            return static_cast<int16_t> (counts * 1000L / 16384);
        }
    }

    Mpu6050::Mpu6050 (uint8_t address)
        : read_            ()
        , rawAcceleration_ ({0, 0, 0})
        , rawRotation_     ({0, 0, 0})
        , rawTemperature_  (0)
        , address_         (address)
        , qmi_             (false)
        , ok_              (false)
        , measured_        (false)
    {
    }

    void Mpu6050::setup ()
    {
        if (i2c::begin ())
        {
            ok_ = configure ();
        }
    }

    Axes Mpu6050::acceleration () const
    {
        return {milliG (rawAcceleration_.x),
                milliG (rawAcceleration_.y),
                milliG (rawAcceleration_.z)};
    }

    // At +-250 degrees per second the MPU-6050 counts 131 per degree per
    // second; at +-256 the QMI8658 counts 128.
    Axes Mpu6050::rotation () const
    {
        int16_t perDegree = qmi_ ? 128 : 131;

        return {static_cast<int16_t> (rawRotation_.x / perDegree),
                static_cast<int16_t> (rawRotation_.y / perDegree),
                static_cast<int16_t> (rawRotation_.z / perDegree)};
    }

    float Mpu6050::pitch () const
    {
        float x = rawAcceleration_.x;
        float y = rawAcceleration_.y;
        float z = rawAcceleration_.z;

        return atan2 (x, sqrt (y * y + z * z)) * DegreesPerRadian;
    }

    float Mpu6050::roll () const
    {
        float y = rawAcceleration_.y;
        float z = rawAcceleration_.z;

        return atan2 (y, z) * DegreesPerRadian;
    }

    float Mpu6050::temperature () const
    {
        return qmi_ ? rawTemperature_ / 256.0f : rawTemperature_ / 340.0f + 36.53f;
    }

    bool Mpu6050::measured () const
    {
        return measured_;
    }

    bool Mpu6050::ok () const
    {
        return ok_;
    }

    // The first reading comes one period after the first update, which gives
    // the chip time to wake.
    void Mpu6050::update (Millis now)
    {
        measured_ = read_.elapsed (now) >= Period;

        if (measured_)
        {
            read_.restart (now);
            ok_ = (ok_ || configure ()) && measure ();
        }
    }

    // The QMI8658's address pin sits where the MPU-6050's AD0 does, but low
    // gives 0x6B and high 0x6A.
    uint8_t Mpu6050::qmiAddress () const
    {
        return address_ == 0x69 ? 0x6A : 0x6B;
    }

    bool Mpu6050::configure ()
    {
        uint8_t identity = 0;

        qmi_ = i2c::read (qmiAddress (), QmiWhoAmI, &identity, 1) && identity == QmiIdentity;

        // The QMI8658 takes its settings one register at a time: registers
        // that count on through a read, low bytes first; +-2 g and +-256
        // degrees per second, 224 readings a second, each filtered to about
        // 30 Hz; then acceleration and rotation switched on.
        if (qmi_)
        {
            uint8_t chip = qmiAddress ();

            return i2c::writeRegister (chip, QmiControl1, 0x40)
                && i2c::writeRegister (chip, QmiControl2, 0x05)
                && i2c::writeRegister (chip, QmiControl3, 0x45)
                && i2c::writeRegister (chip, QmiControl5, 0x77)
                && i2c::writeRegister (chip, QmiControl7, 0x03);
        }

        // The MPU-6050: wake on the internal oscillator; measure +-2 g and
        // +-250 degrees per second; filter both to about 44 Hz, smoothing out
        // vibration for 5 ms of delay.
        return i2c::writeRegister (address_, PowerManagement, 0x00)
            && i2c::writeRegister (address_, AccelRange,      0x00)
            && i2c::writeRegister (address_, GyroRange,       0x00)
            && i2c::writeRegister (address_, LowPass,         0x03);
    }

    // Every reading comes in one transfer, so they all belong to the same
    // moment.
    bool Mpu6050::measure ()
    {
        uint8_t bytes [14];

        // Temperature, then acceleration x, y, z, then rotation x, y, z.
        if (qmi_)
        {
            if (!i2c::read (qmiAddress (), QmiFirstReading, bytes, sizeof bytes))
            {
                return false;
            }

            rawTemperature_  = littleEndian (bytes);
            rawAcceleration_ = {littleEndian (bytes + 2), littleEndian (bytes + 4),
                                littleEndian (bytes + 6)};
            rawRotation_     = {littleEndian (bytes + 8), littleEndian (bytes + 10),
                                littleEndian (bytes + 12)};
            return true;
        }

        // Acceleration x, y, z, then temperature, then rotation x, y, z.
        if (!i2c::read (address_, FirstReading, bytes, sizeof bytes))
        {
            return false;
        }

        rawAcceleration_ = {bigEndian (bytes + 0), bigEndian (bytes + 2), bigEndian (bytes + 4)};
        rawTemperature_  = bigEndian (bytes + 6);
        rawRotation_     = {bigEndian (bytes + 8), bigEndian (bytes + 10), bigEndian (bytes + 12)};
        return true;
    }
}
