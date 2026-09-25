#pragma once

#include "digital.h"

namespace adk {

    // An LED wired from the pin through a resistor (220 or 330 ohm) to GND.
    struct Led : Object
    {
        Led (Pin pin, Polarity polarity = ActiveHigh);

        void on     ();
        void off    ();
        void toggle ();
        void set    (bool lit);
        bool isOn   () const;
        Pin  pin    () const;

        // Flash on and off, a whole period per flash, until on (), off (),
        // toggle () or set () takes over.
        void blink (Millis period);

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void show (bool lit);

        Millis   period_;
        Millis   toggledAt_;
        Pin      pin_;
        Polarity polarity_;
        bool     lit_;
        bool     starting_;
    };
}
