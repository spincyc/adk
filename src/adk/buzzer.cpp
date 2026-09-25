#include "buzzer.h"

#include <Arduino.h>

namespace adk {

    Buzzer::Buzzer (Pin pin, Polarity polarity)
        : beepStart_  (0)
        , beepLength_ (0)
        , pin_        (pin)
        , polarity_   (polarity)
        , on_         (false)
        , starting_   (false)
    {
    }

    void Buzzer::setup ()
    {
        claimOutput (pin_, polarity_ == ActiveLow);
    }

    void Buzzer::on ()
    {
        beepLength_ = 0;
        sound (true);
    }

    void Buzzer::off ()
    {
        beepLength_ = 0;
        sound (false);
    }

    void Buzzer::beep (Millis duration)
    {
        beepLength_ = duration;
        starting_   = true;
        sound (duration != 0);
    }

    bool Buzzer::isOn () const
    {
        return on_;
    }

    void Buzzer::update (Millis now)
    {
        if (beepLength_ == 0)
        {
            return;
        }

        if (starting_)
        {
            beepStart_ = now;
            starting_  = false;
        }

        if (now - beepStart_ >= beepLength_)
        {
            off ();
        }
    }

    void Buzzer::stop ()
    {
        off ();
    }

    void Buzzer::sound (bool on)
    {
        digitalWrite (pin_, (on == (polarity_ == ActiveHigh)) ? HIGH : LOW);
        on_ = on;
    }
}
