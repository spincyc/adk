#include "analog.h"

#include <Arduino.h>

namespace adk {

    AnalogInput::AnalogInput (Pin pin)
        : pin_ (pin)
    {
    }

    void AnalogInput::setup ()
    {
        claimAnalog (pin_);
    }

    uint16_t AnalogInput::read () const
    {
        return static_cast<uint16_t> (analogRead (pin_));
    }

    long AnalogInput::read (long low, long high) const
    {
        return low + (high - low) * static_cast<long> (read ()) / 1023;
    }

    Pin AnalogInput::pin () const
    {
        return pin_;
    }

    PwmOutput::PwmOutput (Pin pin)
        : pin_  (pin)
        , duty_ (0)
    {
    }

    void PwmOutput::setup ()
    {
        claimPwm (pin_);
    }

    void PwmOutput::stop ()
    {
        write (0);
    }

    void PwmOutput::write (int duty)
    {
        duty_ = static_cast<uint8_t> (constrain (duty, 0, 255));
        analogWrite (pin_, duty_);
    }

    uint8_t PwmOutput::duty () const
    {
        return duty_;
    }

    Pin PwmOutput::pin () const
    {
        return pin_;
    }

    Smoother::Smoother (uint8_t shift)
        : scaled_ (0)
        , shift_  (shift)
        , primed_ (false)
    {
    }

    uint16_t Smoother::add (uint16_t sample)
    {
        // The first sample is taken as is, so the value starts where it is.
        if (!primed_)
        {
            scaled_ = static_cast<uint32_t> (sample) << shift_;
            primed_ = true;
        }
        else
        {
            scaled_ = scaled_ - (scaled_ >> shift_) + sample;
        }

        return value ();
    }

    uint16_t Smoother::value () const
    {
        return static_cast<uint16_t> (scaled_ >> shift_);
    }
}
