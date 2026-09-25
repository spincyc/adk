#pragma once

#include "object.h"

namespace adk {

    // The KY-040 rotary encoder module: a knob that clicks through detents
    // as it turns. Wire CLK to one pin, DT to another, + to 5 V and GND to
    // GND. Its push switch, SW, is a separate adk::Button on a third pin.
    //
    // Both contacts are read on every update and decoded as a Gray code, so
    // a bouncing contact steps back and forth and cancels itself out. Loops
    // that call adk::update () or adk::wait () read about a thousand times a
    // second, plenty for a hand-turned knob; long blocking work elsewhere,
    // such as delay (), loses the steps turned meanwhile.
    struct RotaryEncoder : Object
    {
        // The KY-040's contacts step four times per detent; some encoders
        // step twice or once.
        RotaryEncoder (Pin clk, Pin dt, uint8_t stepsPerDetent = 4);

        // Detents turned since setup, clockwise positive.
        long position () const;

        // Detents turned in this update: an event, usually -1, 0 or +1.
        int8_t turned () const;

        // Count from here as position, without moving the knob.
        void reset (long position = 0);

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        uint8_t read () const;

        long    position_;
        Pin     clk_;
        Pin     dt_;
        uint8_t stepsPerDetent_;
        uint8_t contacts_;
        int8_t  steps_;
        int8_t  turned_;
    };
}
