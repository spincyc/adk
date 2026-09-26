#pragma once

#include "object.h"

namespace adk {

    // The KY-040 rotary encoder module: a knob that clicks through detents
    // as it turns. Wire CLK to one pin, DT to another, + to 5 V and GND to
    // GND. Its push switch, SW, is a separate adk::Button on a third pin.
    //
    // Both contacts are read on every update and decoded as a Gray code, so
    // a bouncing contact steps back and forth and cancels itself out, and a
    // detent counts when the knob comes to rest in it. Loops that call
    // adk::update () or adk::wait () read about a thousand times a second,
    // plenty for a hand-turned knob, which takes several milliseconds a
    // step even spun fast. A read that misses a step still counts the
    // detent; long blocking work elsewhere, such as delay (), loses the
    // detents turned meanwhile. Reading by polling, rather than from an
    // interrupt, lets CLK and DT go on any two pins.
    struct RotaryEncoder : Object
    {
        // The KY-040's contacts step four times per detent; some encoders
        // step twice or once.
        RotaryEncoder (Pin clk, Pin dt, uint8_t stepsPerDetent = 4);

        // Detents turned since setup, clockwise positive.
        long position () const;

        // Detents turned in this update, clockwise positive: an event. It
        // is -1, 0 or +1 unless the loop was too slow to see the knob rest
        // between two clicks, when it counts both.
        int8_t turned () const;

        // Count from here as position, without moving the knob.
        void reset (long position = 0);

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool    isResting (uint8_t contacts) const;
        uint8_t read      () const;

        long    position_;
        Pin     clk_;
        Pin     dt_;
        uint8_t stepsPerDetent_;
        uint8_t contacts_;
        int8_t  steps_;
        int8_t  turned_;
    };
}
