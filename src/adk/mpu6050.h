#pragma once

#include "object.h"

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
    // Lay the module flat, chip up, with its X arrow pointing forward. At
    // rest the axis pointing up reads +1000 milli-g, so flat gives z = 1000.
    // pitch () is positive with the front raised, and roll () positive with
    // the side the Y arrow points to raised. Both come from gravity alone,
    // so they are only right while the module is not being shaken.
    //
    // A missing chip is not a pin fault: the sketch runs and ok () is false.
    // A chip that stops answering is set up again once it answers, since it
    // wakes from a power glitch asleep. The WHO_AM_I register is not checked,
    // because clones answer with other values than a genuine chip's 0x68.
    struct Mpu6050 : Object
    {
        explicit Mpu6050 (uint8_t address = 0x68);

        // From the latest reading.
        Axes   acceleration () const;   // milli-g, up to 2000 either way
        Axes   rotation     () const;   // degrees per second, up to 250
        float  pitch        () const;   // degrees, -90 to 90
        float  roll         () const;   // degrees, -180 to 180
        float  temperature  () const;   // degrees Celsius, of the chip

        // The chip answered the last time it was asked.
        bool ok () const;

        // A new reading arrived in this update.
        bool measured () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool configure ();
        bool measure   ();

        Millis  readAt_;
        Axes    rawAcceleration_;
        Axes    rawRotation_;
        int16_t rawTemperature_;
        uint8_t address_;
        bool    ok_;
        bool    measured_;
        bool    starting_;
    };
}
