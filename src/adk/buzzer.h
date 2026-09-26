#pragma once

#include "on_off_pin.h"
#include "timing.h"

namespace adk {

    // An active buzzer: it makes its own tone whenever its pin is high. It
    // is the sealed one, often with a sticker on top; the one with a green
    // board showing underneath is a passive buzzer, which is a Speaker.
    //
    //   + (the longer leg) -> the pin
    //   the other leg      -> GND
    //
    // It draws up to about 30 mA: within a Mega pin's 40 mA, but give it a
    // pin to itself.
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
