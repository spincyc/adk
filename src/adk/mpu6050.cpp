#include "mpu6050.h"

#include "i2c.h"

#include <math.h>

namespace adk {

    namespace {

        const uint8_t LowPass         = 0x1A;
        const uint8_t GyroRange       = 0x1B;
        const uint8_t AccelRange      = 0x1C;
        const uint8_t FirstReading    = 0x3B;
        const uint8_t PowerManagement = 0x6B;

        const Millis Period           = 20;
        const float  DegreesPerRadian = 57.29578f;

        // Readings are big-endian, high byte first.
        int16_t wordAt (const uint8_t* bytes)
        {
            return static_cast<int16_t> ((bytes[0] << 8) | bytes[1]);
        }

        // At +-2 g the chip counts 16384 per g.
        int16_t milliG (int16_t counts)
        {
            return static_cast<int16_t> (counts * 1000L / 16384);
        }

        // At +-250 degrees per second it counts 131 per degree per second.
        int16_t degreesPerSecond (int16_t counts)
        {
            return static_cast<int16_t> (counts / 131);
        }
    }

    Mpu6050::Mpu6050 (uint8_t address)
        : readAt_          (0)
        , rawAcceleration_ ({0, 0, 0})
        , rawRotation_     ({0, 0, 0})
        , rawTemperature_  (0)
        , address_         (address)
        , ok_              (false)
        , measured_        (false)
        , starting_        (true)
    {
    }

    void Mpu6050::setup ()
    {
        if (i2c::begin ())
        {
            ok_ = configure ();
        }
    }

    Vector Mpu6050::acceleration () const
    {
        return {milliG (rawAcceleration_.x),
                milliG (rawAcceleration_.y),
                milliG (rawAcceleration_.z)};
    }

    Vector Mpu6050::rotation () const
    {
        return {degreesPerSecond (rawRotation_.x),
                degreesPerSecond (rawRotation_.y),
                degreesPerSecond (rawRotation_.z)};
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
        return rawTemperature_ / 340.0f + 36.53f;
    }

    bool Mpu6050::ok () const
    {
        return ok_;
    }

    bool Mpu6050::measured () const
    {
        return measured_;
    }

    void Mpu6050::update (Millis now)
    {
        measured_ = false;

        // The first reading comes one period after the first update, which
        // gives the chip time to wake.
        if (starting_)
        {
            readAt_   = now;
            starting_ = false;
            return;
        }

        if (now - readAt_ < Period)
        {
            return;
        }

        readAt_   = now;
        ok_       = (ok_ || configure ()) && measure ();
        measured_ = ok_;
    }

    bool Mpu6050::configure ()
    {
        // Wake on the internal oscillator; measure +-2 g and +-250 degrees
        // per second; filter both to about 44 Hz, smoothing out vibration
        // for 5 ms of delay.
        return i2c::writeRegister (address_, PowerManagement, 0x00)
            && i2c::writeRegister (address_, AccelRange,      0x00)
            && i2c::writeRegister (address_, GyroRange,       0x00)
            && i2c::writeRegister (address_, LowPass,         0x03);
    }

    bool Mpu6050::measure ()
    {
        // Acceleration x, y, z, then temperature, then rotation x, y, z, in
        // one transfer so they all belong to the same moment.
        uint8_t bytes [14];

        if (!i2c::read (address_, FirstReading, bytes, sizeof bytes))
        {
            return false;
        }

        rawAcceleration_ = {wordAt (bytes + 0), wordAt (bytes + 2), wordAt (bytes + 4)};
        rawTemperature_  = wordAt (bytes + 6);
        rawRotation_     = {wordAt (bytes + 8), wordAt (bytes + 10), wordAt (bytes + 12)};
        return true;
    }
}
