#include "relay.h"

namespace adk {

    Relay::Relay (Pin pin, Polarity polarity)
        : coil_ (pin, polarity)
    {
    }

    void Relay::setup ()
    {
        coil_.claim ();
    }

    void Relay::on ()
    {
        coil_.set (true);
    }

    void Relay::off ()
    {
        coil_.set (false);
    }

    void Relay::toggle ()
    {
        coil_.set (!coil_.isOn ());
    }

    bool Relay::isOn () const
    {
        return coil_.isOn ();
    }

    void Relay::stop ()
    {
        off ();
    }
}
