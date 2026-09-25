#pragma once

#include "object.h"

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

        // Centimetres to the nearest object, from the latest reading.
        uint16_t distance () const;

        // False when nothing answered within range; distance () is then 0.
        bool hasEcho () const;

        // A new reading arrived in this update. The first comes with the
        // first update.
        bool measured () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        void ping ();

        Millis   pingedAt_;
        uint16_t distance_;
        Pin      trigger_;
        Pin      echo_;
        bool     measured_;
        bool     starting_;
    };
}
