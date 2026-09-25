#include "rgb_led.h"

#include <Arduino.h>

namespace adk {

    RgbLed::RgbLed (Pin red, Pin green, Pin blue, Polarity polarity)
        : shown_      (color::off)
        , from_       (color::off)
        , to_         (color::off)
        , fadeStart_  (0)
        , fadeLength_ (0)
        , pins_       {red, green, blue}
        , polarity_   (polarity)
        , starting_   (false)
    {
    }

    void RgbLed::setup ()
    {
        bool dark = polarity_ == ActiveLow;

        if (claimPwm (pins_[0], dark) && claimPwm (pins_[1], dark) && claimPwm (pins_[2], dark))
        {
            write (color::off);
        }
    }

    void RgbLed::show (Color color)
    {
        fadeLength_ = 0;
        write (color);
    }

    void RgbLed::off ()
    {
        show (color::off);
    }

    void RgbLed::fadeTo (Color color, Millis duration)
    {
        from_       = shown_;
        to_         = color;
        fadeLength_ = duration;
        starting_   = true;

        if (duration == 0)
        {
            show (color);
        }
    }

    bool RgbLed::isFading () const
    {
        return fadeLength_ != 0;
    }

    Color RgbLed::color () const
    {
        return shown_;
    }

    void RgbLed::update (Millis now)
    {
        if (fadeLength_ == 0)
        {
            return;
        }

        if (starting_)
        {
            fadeStart_ = now;
            starting_  = false;
        }

        Millis elapsed = now - fadeStart_;

        if (elapsed >= fadeLength_)
        {
            show (to_);
            return;
        }

        write (blend (from_, to_, static_cast<uint16_t> (elapsed * 256 / fadeLength_), 256));
    }

    void RgbLed::stop ()
    {
        off ();
    }

    void RgbLed::write (Color color)
    {
        const uint8_t duties [3] = {color.red, color.green, color.blue};

        for (uint8_t index = 0; index < 3; ++index)
        {
            uint8_t duty = duties[index];
            analogWrite (pins_[index], polarity_ == ActiveHigh ? duty : 255 - duty);
        }

        shown_ = color;
    }
}
