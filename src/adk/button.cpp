#include "button.h"

#include <Arduino.h>

namespace adk {

    Switch::Switch (Pin pin, Polarity polarity, uint8_t debounce)
        : debouncer_   ()
        , pin_         (pin)
        , debounce_    (debounce)
        , polarity_    (polarity)
        , activated_   (false)
        , deactivated_ (false)
    {
    }

    void Switch::setup ()
    {
        if (claimInput (pin_, polarity_ == ActiveLow))
        {
            debouncer_ = Debouncer (read ());
        }
    }

    void Switch::update (Millis now)
    {
        bool changed = debouncer_.sample (read (), now, debounce_);

        activated_   = changed && debouncer_.stable ();
        deactivated_ = changed && !debouncer_.stable ();
    }

    bool Switch::read () const
    {
        return (digitalRead (pin_) == HIGH) == (polarity_ == ActiveHigh);
    }

    bool Switch::isActive () const
    {
        return debouncer_.stable ();
    }

    bool Switch::activated () const
    {
        return activated_;
    }

    bool Switch::deactivated () const
    {
        return deactivated_;
    }

    Pin Switch::pin () const
    {
        return pin_;
    }

    Button::Button (Pin pin, uint8_t debounce)
        : Switch (pin, ActiveLow, debounce)
    {
    }

    bool Button::isPressed () const
    {
        return isActive ();
    }

    bool Button::wasPressed () const
    {
        return activated ();
    }

    bool Button::wasReleased () const
    {
        return deactivated ();
    }
}
