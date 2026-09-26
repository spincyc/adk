#include "buzzer.h"

namespace adk {

    Buzzer::Buzzer (Pin pin, Polarity polarity)
        : sound_  (pin, polarity)
        , beep_   ()
        , length_ (0)
    {
    }

    void Buzzer::setup ()
    {
        sound_.claim ();
    }

    void Buzzer::on ()
    {
        length_ = 0;
        sound_.set (true);
    }

    void Buzzer::off ()
    {
        length_ = 0;
        sound_.set (false);
    }

    bool Buzzer::isOn () const
    {
        return sound_.isOn ();
    }

    void Buzzer::beep (Millis duration)
    {
        if (duration == 0)
        {
            off ();
            return;
        }

        if (duration == length_)
        {
            return;
        }

        length_ = duration;
        beep_.restart ();
        sound_.set (true);
    }

    void Buzzer::update (Millis now)
    {
        if (length_ != 0 && beep_.elapsed (now) >= length_)
        {
            off ();
        }
    }

    void Buzzer::stop ()
    {
        off ();
    }
}
