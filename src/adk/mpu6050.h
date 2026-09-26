#pragma once

#include "timing.h"

namespace adk {

    // Three readings, one along each axis printed on a module.
    struct Axes
    {
        int16_t x;
        int16_t y;
        int16_t z;
    };

    // A GY-521 module: an MPU-6050 accelerometer and gyroscope, read every
    // 20 ms. It goes on the I2C bus:
    //
    //   VCC -> 5 V (the module has its own 3.3 V regulator), GND -> GND,
    //   SCL -> pin 21, SDA -> pin 20
    //
    // With AD0 unconnected the chip answers at I2C address 0x68. A DS1307
    // clock answers there too, so beside one wire AD0 -> 3.3 V and declare
    // Mpu6050 {0x69}. XDA, XCL and INT stay unconnected.
    //
    // Some kits come with a board marked ICM40607&QMI8658 in its place,
    // carrying a QMI8658 chip. Wire its 5V, GND, SCL and SDA the same way and
    // leave RST, SWDIO, 3V3 and SWCLK unconnected. Mpu6050 looks for that
    // chip first, at 0x6B (or at 0x6A for Mpu6050 {0x69}), and reads it
    // through the same calls in the same units. Its axes should follow the
    // board's printed arrows as the MPU-6050's do; that is not yet checked
    // on a real board.
    //
    // Lay the module flat, chip up, with its X arrow pointing forward. At
    // rest the axis pointing up reads +1000 milli-g, so flat gives z = 1000.
    // pitch () is positive with the front raised, and roll () positive with
    // the side the Y arrow points to raised. Both come from gravity alone,
    // so they are only right while the module is not being shaken.
    //
    // A missing chip is not a pin fault: the sketch runs and ok () is false.
    // A chip that stops answering is set up again once it answers, since it
    // wakes from a power glitch asleep. The MPU-6050's WHO_AM_I register is
    // not checked, because clones answer with other values than a genuine
    // chip's 0x68.
    struct Mpu6050 : Object
    {
        explicit Mpu6050 (uint8_t address = 0x68);

        // From the latest good reading: acceleration in milli-g, up to 2000
        // either way, and rotation in degrees per second, up to about 250.
        Axes acceleration () const;
        Axes rotation     () const;

        // Tilt in degrees: pitch from -90 to 90, roll from -180 to 180.
        float pitch () const;
        float roll  () const;

        // The chip's own temperature, in degrees Celsius.
        float temperature () const;

        // A reading finished in this update, good or not: an event, every
        // 20 ms.
        bool measured () const;

        // The chip answered the last time it was asked.
        bool ok () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        uint8_t qmiAddress () const;
        bool    configure  ();
        bool    measure    ();

        StartTime read_;
        Axes      rawAcceleration_;
        Axes      rawRotation_;
        int16_t   rawTemperature_;
        uint8_t   address_;
        bool      qmi_;        // the chip is a QMI8658, not an MPU-6050
        bool      ok_;
        bool      measured_;
    };
}
