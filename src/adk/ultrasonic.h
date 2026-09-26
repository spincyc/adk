#pragma once

#include "timing.h"

namespace adk {

    // An HC-SR04 ultrasonic distance sensor. Wire VCC to 5 V, Trig to the
    // trigger pin, Echo to the echo pin, and GND to GND.
    //
    // It pings every 60 ms, the shortest cycle its datasheet allows, so the
    // echoes of one ping have died away before the next. Timing the echo
    // blocks update () for up to 25 ms once per ping, so it sees no further
    // than about 4 m.
    struct Ultrasonic : Object
    {
        Ultrasonic (Pin trigger, Pin echo);

        // Centimeters to the nearest object, from the latest reading, or 0
        // when that reading is not ok ().
        uint16_t distance () const;

        // A ping finished in this update, echo or not: an event. The first
        // comes 60 ms after the first update.
        bool measured () const;

        // Whether the latest ping's echo came back. Without one, nothing is
        // within about 4 m (or closer than 2 cm), or the sensor is not
        // answering.
        bool ok () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        void ping ();

        StartTime pinged_;
        uint16_t  distance_;
        Pin       trigger_;
        Pin       echo_;
        bool      measured_;
    };
}
