#include "relay.h"

#include <Arduino.h>

namespace adk {

    Relay::Relay (Pin pin, Polarity polarity)
        : pin_      (pin)
        , polarity_ (polarity)
        , on_       (false)
    {
    }

    void Relay::setup ()
    {
        claimOutput (pin_, polarity_ == ActiveLow);
    }

    void Relay::on ()
    {
        set (true);
    }

    void Relay::off ()
    {
        set (false);
    }

    void Relay::toggle ()
    {
        set (!on_);
    }

    bool Relay::isOn () const
    {
        return on_;
    }

    void Relay::stop ()
    {
        off ();
    }

    void Relay::set (bool on)
    {
        digitalWrite (pin_, (on == (polarity_ == ActiveHigh)) ? HIGH : LOW);
        on_ = on;
    }
}
