#pragma once

#include "color.h"
#include "digital.h"
#include "timing.h"

namespace adk {

    // An RGB LED on three PWM pins, each color leg through its own resistor.
    // A common-cathode LED (long leg to GND) is active high; a common-anode
    // LED (long leg to 5 V) is active low.
    struct RgbLed : Object
    {
        RgbLed (Pin red, Pin green, Pin blue, Polarity polarity = ActiveHigh);

        void  show     (Color color);
        void  off      ();
        bool  isFading () const;
        Color color    () const;

        // Glide from the color shown now to another, arriving after
        // duration, while the sketch carries on. Asking again for the color
        // it is already fading to, or already shows, changes nothing, so
        // fadeTo () can be called from every pass of loop (); a different
        // color starts afresh from wherever the fade has got to.
        void fadeTo (Color color, Millis duration);

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void write (Color color);

        Color     shown_;
        Color     from_;
        Color     to_;
        StartTime fade_;
        Millis    fadeLength_;
        Pin       pins_ [3];
        Polarity  polarity_;
    };
}
