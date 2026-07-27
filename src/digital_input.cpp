#include "digital_input.h"

#include <Arduino.h>

namespace adk {

    DigitalInput::DigitalInput (ResourceRegistry& resources,
                                PinId             pin,
                                Pull              pull) noexcept
        : resources_   (&resources)
        , claim_       ()
        , pin_         (pin)
        , pull_        (pull)
        , level_       (Level::Low)
        , initialized_ (false)
    {
    }

    DigitalInput::~DigitalInput () noexcept
    {
        shutdown ();
    }

    Status DigitalInput::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (pin_ >= NUM_DIGITAL_PINS)
        {
            return StatusCode::InvalidPin;
        }

        const ResourceId resource = {ResourceKind::Pin, pin_};
        const Status     status   = resources_->claim (resource, claim_);

        if (!status.ok ())
        {
            return status;
        }

        pinMode (pin_, pull_ == Pull::Up ? INPUT_PULLUP : INPUT);

        initialized_ = true;
        update ();
        return StatusCode::Ok;
    }

    void DigitalInput::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        pinMode        (pin_, INPUT);
        claim_.release ();
        initialized_ = false;
    }

    void DigitalInput::update () noexcept
    {
        if (initialized_)
        {
            level_ = sample ();
        }
    }

    Level DigitalInput::sample () const noexcept
    {
        if (!initialized_)
        {
            return level_;
        }

        return digitalRead (pin_) == HIGH ? Level::High : Level::Low;
    }

    Level DigitalInput::read () const noexcept
    {
        return level_;
    }

    PinId DigitalInput::pin () const noexcept
    {
        return pin_;
    }

    Pull DigitalInput::pull () const noexcept
    {
        return pull_;
    }

    bool DigitalInput::initialized () const noexcept
    {
        return initialized_;
    }
}
