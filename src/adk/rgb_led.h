#pragma once

#include "color.h"
#include "digital.h"

namespace adk {

    // An RGB LED on three PWM pins, each colour leg through its own resistor.
    // A common-cathode LED (long leg to GND) is active high; a common-anode
    // LED (long leg to 5 V) is active low.
    struct RgbLed : Object
    {
        RgbLed (Pin red, Pin green, Pin blue, Polarity polarity = ActiveHigh);

        void  show     (Color color);
        void  off      ();
        void  fadeTo   (Color color, Millis duration);
        bool  isFading () const;
        Color color    () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void write (Color color);

        Color    shown_;
        Color    from_;
        Color    to_;
        Millis   fadeStart_;
        Millis   fadeLength_;
        Pin      pins_ [3];
        Polarity polarity_;
        bool     starting_;
    };
}
