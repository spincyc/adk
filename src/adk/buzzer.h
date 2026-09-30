#pragma once

#include "on_off_pin.h"
#include "timing.h"

namespace adk {

    // An active buzzer: it makes its own tone whenever its pin is high. It
    // is the sealed one, often with a sticker on top; the one with a green
    // board showing underneath is a passive buzzer, which is a Speaker.
    //
    // Use a transistor driver: the buzzer can need 30 mA, above a Mega
    // pin's recommended 20 mA. The course uses an S8050 with its verified
    // E-B-C pin order, viewed from the marked flat face with legs down:
    //
    //   + (the longer leg) -> 5 V rail
    //   the other leg      -> S8050 collector; emitter -> GND
    //   the pin            -> 1k ohm -> base; base -> 10k ohm -> GND
    //   1N4007 diode       -> across buzzer, banded end toward +
    //
    // The pin switches only the base current. Do not connect the buzzer
    // directly to it, even for short beeps. Check the actual transistor's
    // marking and supplier pin diagram; other TO-92 parts may differ.
    struct Buzzer : Object
    {
        Buzzer (Pin pin, Polarity polarity = ActiveHigh);

        void on   ();
        void off  ();
        bool isOn () const;

        // Sound for a duration, while the sketch carries on. Asking again
        // for a beep of the same length while one sounds changes nothing, so
        // beep () can be called from every pass of loop (); a different
        // length starts afresh.
        void beep (Millis duration);

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        OnOffPin  sound_;
        StartTime beep_;
        Millis    length_;
    };
}
