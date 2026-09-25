#pragma once

#include "object.h"

namespace adk {

    // A small DC motor on one half of an L293D driver chip. The chip switches
    // the motor's own supply, both ways round, so the motor can run forwards
    // or backwards; PWM on its enable pin sets the speed.
    //
    //   L293D pin 1 (EN1,2)          -> a PWM pin, the enable pin
    //   L293D pin 2 (1A)             -> the forward pin
    //   L293D pin 7 (2A)             -> the backward pin
    //   L293D pins 3 and 6 (1Y, 2Y)  -> the motor's two leads
    //   L293D pin 16 (VCC1)          -> 5 V for the chip's logic, from the Mega
    //                                   or the power module
    //   L293D pin 8 (VCC2)           -> the motor supply: 5 V from a breadboard
    //                                   power module
    //   L293D pins 4, 5, 12 and 13   -> GND, joined to the Mega's GND
    //
    // The chip's other half works the same way: pin 9 (EN3,4) is the enable,
    // pins 10 and 15 (3A, 4A) the directions, and pins 11 and 14 (3Y, 4Y) the
    // motor.
    //
    // Never power a motor from a Mega pin, or put VCC2 on the Mega's 5 V: a
    // starting motor draws far more current than the board can give.
    //
    // Reversing a spinning motor at once would briefly draw about twice its
    // stall current. So when the direction changes, the motor coasts for
    // 100 ms before it is driven the other way, while the sketch carries on.
    struct Motor : Object
    {
        Motor (Pin enable, Pin forward, Pin backward);

        // From -255 (full speed backwards) to 255 (full speed forwards); 0
        // coasts. speed () is the speed asked for, even while a reversal
        // waits. A slow speed may only make the motor hum until it gets going.
        void    speed (int16_t speed);
        int16_t speed () const;

        // Stop quickly by shorting the motor's leads together, or cut the
        // power and let it spin down freely.
        void brake ();
        void coast ();

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void drive       ();
        void stopDriving (bool braking);

        Millis  stoppedAt_;
        int16_t speed_;
        Pin     enable_;
        Pin     forward_;
        Pin     backward_;
        int8_t  spin_;      // 1 or -1 while the motor may still turn that way
        bool    driving_;
        bool    starting_;
    };
}
