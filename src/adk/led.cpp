#include "led.h"

#include <Arduino.h>

namespace adk {

    Led::Led (Pin pin, Polarity polarity)
        : period_    (0)
        , toggledAt_ (0)
        , pin_       (pin)
        , polarity_  (polarity)
        , lit_       (false)
        , starting_  (false)
    {
    }

    void Led::setup ()
    {
        claimOutput (pin_, polarity_ == ActiveLow);
    }

    void Led::on ()
    {
        set (true);
    }

    void Led::off ()
    {
        set (false);
    }

    void Led::toggle ()
    {
        set (!lit_);
    }

    void Led::set (bool lit)
    {
        period_ = 0;
        show (lit);
    }

    bool Led::isOn () const
    {
        return lit_;
    }

    Pin Led::pin () const
    {
        return pin_;
    }

    void Led::blink (Millis period)
    {
        // Asking again for the same blink changes nothing, so blink () can be
        // called from every pass of loop ().
        if (period == period_)
        {
            return;
        }

        period_   = period;
        starting_ = true;
        show (true);
    }

    void Led::update (Millis now)
    {
        if (period_ == 0)
        {
            return;
        }

        if (starting_)
        {
            toggledAt_ = now;
            starting_  = false;
        }

        if (now - toggledAt_ >= period_ / 2)
        {
            toggledAt_ = now;
            show (!lit_);
        }
    }

    void Led::stop ()
    {
        off ();
    }

    void Led::show (bool lit)
    {
        digitalWrite (pin_, (lit == (polarity_ == ActiveHigh)) ? HIGH : LOW);
        lit_ = lit;
    }
}
