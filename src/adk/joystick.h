#pragma once

#include "object.h"

namespace adk {

    // The two-axis joystick module: a thumb stick on two potentiometers.
    // Wire VRx to one analog pin, VRy to another, +5V to 5 V and GND to GND.
    // Its push switch, SW, is a separate adk::Button on a digital pin.
    //
    // Held with its pins on the left, x () is positive to the right and y ()
    // positive upwards. Held another way round, or with VRx and VRy swapped,
    // the axes swap or change sign. Where the stick rests at setup () is its
    // centre, so leave it alone while the sketch starts.
    struct Joystick : Object
    {
        enum Direction : uint8_t
        {
            Center,
            Up,
            Down,
            Left,
            Right
        };

        Joystick (Pin x, Pin y);

        // How far the stick is pushed, from -100 to 100 per cent of its
        // travel. Within 10 of the centre it reads 0, since a released stick
        // never quite returns to the same place.
        int8_t x () const;
        int8_t y () const;

        // Where the stick is pushed: the axis pushed furthest, once past 50.
        // It holds until the stick falls back below 30, so a stick resting
        // near halfway does not flicker in and out.
        Direction direction () const;

        // The direction just entered in this update, else Center: an event,
        // for moving through a game or a menu one push at a time.
        Direction moved () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        int8_t    reach     (Direction direction) const;
        Direction strongest () const;

        uint16_t  xCenter_;
        uint16_t  yCenter_;
        Pin       xPin_;
        Pin       yPin_;
        int8_t    x_;
        int8_t    y_;
        Direction direction_;
        Direction moved_;
    };
}
