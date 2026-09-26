#include "on_off_pin.h"

#include <Arduino.h>

namespace adk {

    OnOffPin::OnOffPin (Pin pin, Polarity polarity)
        : pin_      (pin)
        , polarity_ (polarity)
        , on_       (false)
    {
    }

    bool OnOffPin::claim ()
    {
        on_ = false;
        return claimOutput (pin_, polarity_ == ActiveLow);
    }

    void OnOffPin::set (bool on)
    {
        digitalWrite (pin_, (on == (polarity_ == ActiveHigh)) ? HIGH : LOW);
        on_ = on;
    }

    bool OnOffPin::isOn () const
    {
        return on_;
    }

    Pin OnOffPin::pin () const
    {
        return pin_;
    }
}
