#pragma once

#include "board.h"

#include <stdint.h>

namespace adk {

    using Millis = uint32_t;

    // Every device or behavior in a sketch is an Object. Declaring one
    // registers it, in declaration order. adk::setup () starts them all,
    // adk::update () advances them all with one shared timestamp, and
    // adk::stop () puts them all in a safe state.
    struct Object
    {
        Object            (const Object&) = delete;
        Object& operator= (const Object&) = delete;

      protected:
        Object  ();
        ~Object ();

        // Claim and configure pins. Runs once, from adk::setup ().
        virtual void setup ();

        // Advance time-driven behavior. Runs on every adk::update ().
        virtual void update (Millis now);

        // Enter the safe state: outputs off, sound and motion stopped.
        virtual void stop ();

      private:
        friend bool start  ();
        friend void update (Millis now);
        friend void stop   ();

        Object* next_;
    };

    // Start every object. If a pin is used twice or cannot do what an object
    // needs, halt and blink the pin number on the built-in LED.
    void setup ();

    // As setup (), and first print what went wrong, for example to Serial.
    // Start Serial first: Serial.begin (9600); adk::setup (Serial);
    void setup (Print& log);

    // Start every object and report whether every claim succeeded.
    bool start ();

    // Advance every object. Call it at the top of loop ().
    void update ();
    void update (Millis now);

    // Like delay (), but every object keeps updating while it waits.
    void wait (Millis duration);

    // Put every object in its safe state.
    void stop ();
}
