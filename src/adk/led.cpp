#include "led.h"

namespace adk {

    Led::Led (Pin pin, Polarity polarity)
        : light_  (pin, polarity)
        , flash_  ()
        , period_ (0)
    {
    }

    void Led::setup ()
    {
        light_.claim ();
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
        set (!light_.isOn ());
    }

    void Led::set (bool lit)
    {
        period_ = 0;
        light_.set (lit);
    }

    bool Led::isOn () const
    {
        return light_.isOn ();
    }

    Pin Led::pin () const
    {
        return light_.pin ();
    }

    void Led::blink (Millis period)
    {
        if (period == period_)
        {
            return;
        }

        period_ = period;
        flash_.restart ();
        light_.set (true);
    }

    void Led::update (Millis now)
    {
        if (period_ != 0 && flash_.elapsed (now) >= period_ / 2)
        {
            flash_.restart (now);
            light_.set (!light_.isOn ());
        }
    }

    void Led::stop ()
    {
        off ();
    }
}
