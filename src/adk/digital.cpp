#include "digital.h"

#include <Arduino.h>

namespace adk {

    DigitalOutput::DigitalOutput (Pin pin)
        : pin_  (pin)
        , high_ (false)
    {
    }

    void DigitalOutput::setup ()
    {
        claimOutput (pin_, high_);
    }

    void DigitalOutput::stop ()
    {
        write (false);
    }

    void DigitalOutput::write (bool high)
    {
        digitalWrite (pin_, high ? HIGH : LOW);
        high_ = high;
    }

    void DigitalOutput::toggle ()
    {
        write (!high_);
    }

    bool DigitalOutput::isHigh () const
    {
        return high_;
    }

    Pin DigitalOutput::pin () const
    {
        return pin_;
    }

    DigitalInput::DigitalInput (Pin pin, bool pullUp)
        : pin_    (pin)
        , pullUp_ (pullUp)
    {
    }

    void DigitalInput::setup ()
    {
        claimInput (pin_, pullUp_);
    }

    bool DigitalInput::read () const
    {
        return digitalRead (pin_) == HIGH;
    }

    Pin DigitalInput::pin () const
    {
        return pin_;
    }
}
