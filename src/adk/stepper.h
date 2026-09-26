#pragma once

#include "object.h"

namespace adk {

    // A 28BYJ-48 5 V geared stepper motor on its ULN2003 driver board. The
    // motor turns in small exact steps, so it can go to a position and come
    // back without a sensor. The board's four LEDs show which coils are on.
    //
    //   IN1-IN4 -> the four pins given to Stepper, in the same order
    //   +       -> 5 V from a supply good for 300 mA, such as a breadboard
    //              power module, with its GND joined to the Mega's GND
    //   -       -> GND
    //   motor   -> the board's white socket (the plug only fits one way)
    //
    // The coils draw up to about 200 mA. Never power them from the Mega's
    // 5 V pin.
    //
    // Positions count half-steps: 4096 nominally make one turn of the output
    // shaft. The gearbox is really about 63.68:1 rather than 64:1, so a true
    // turn is about 4076 half-steps, and 4096 steps overshoot by about 1.8
    // degrees. Forward (positive) steps are meant to turn the shaft clockwise,
    // seen from the shaft end; this is not yet checked on a real motor, so
    // watch which way yours goes.
    struct Stepper : Object
    {
        static constexpr uint16_t StepsPerRevolution = 4096;

        Stepper (Pin in1, Pin in2, Pin in3, Pin in4);

        // Move this many half-steps on from where the current move ends, or
        // to a position counted from where the motor was at adk::setup ().
        // Asking again for the position it is already moving to changes
        // nothing, so moveTo () can be called from every pass of loop ();
        // step () counts on each time.
        void step   (long steps);
        void moveTo (long position);

        long position () const;
        bool isMoving () const;

        // Half-steps a second, from 1 up to 500, which it starts at. A
        // 28BYJ-48 is only sure to start, stop and turn back without missing
        // a step below about 600 half-steps a second; faster, it can stall
        // and only hum.
        void speed (uint16_t stepsPerSecond);

        // Keep the coils powered when the motor is still, so it resists
        // being turned. By default they are released at the end of each move,
        // because they would get warm, and the gearbox holds the shaft anyway.
        void hold (bool on);

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void energize ();
        void release  ();

        long     position_;
        long     target_;
        Millis   then_;
        uint16_t credit_;
        uint16_t speed_;
        Pin      pins_ [4];
        bool     holding_;
        bool     energized_;
        bool     resting_;
    };
}
