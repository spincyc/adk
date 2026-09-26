#pragma once

#include "digital.h"

namespace adk {

    // A pin that switches something on and off, whichever level means on:
    // the pin inside an Led, a Buzzer and a Relay.
    struct OnOffPin
    {
        OnOffPin (Pin pin, Polarity polarity);

        // Claim the pin as an output, switched off. From the part's setup ().
        bool claim ();

        void set  (bool on);
        bool isOn () const;
        Pin  pin  () const;

      private:
        Pin      pin_;
        Polarity polarity_;
        bool     on_;
    };
}
