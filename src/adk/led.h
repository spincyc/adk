#pragma once

#include "on_off_pin.h"
#include "timing.h"

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
        // toggle () or set () takes over. Asking again for the same blink
        // changes nothing, so blink () can be called from every pass of
        // loop (); a new period starts afresh.
        void blink (Millis period);

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        OnOffPin  light_;
        StartTime flash_;
        Millis    period_;
    };
}
