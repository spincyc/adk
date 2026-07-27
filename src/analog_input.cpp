#include "analog_input.h"

#include "board.h"

#include <Arduino.h>

namespace adk {

    constexpr AnalogInput::Reading AnalogInput::maximumReading;

    AnalogInput::AnalogInput (ResourceRegistry& resources,
                              PinId             pin) noexcept
        : resources_   (&resources)
        , claim_       ()
        , pin_         (pin)
        , reading_     (0)
        , initialized_ (false)
    {
    }

    AnalogInput::~AnalogInput () noexcept
    {
        shutdown ();
    }

    Status AnalogInput::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (pin_))
        {
            return StatusCode::InvalidPin;
        }

        if (!Mega2560Board::supports (pin_, PinCapability::AnalogInput))
        {
            return StatusCode::Unsupported;
        }

        const Status status = resources_->claim (
            {ResourceKind::Pin, pin_},
            claim_);

        if (!status.ok ())
        {
            return status;
        }

        pinMode      (pin_, INPUT);
        initialized_ = true;
        update       ();
        return StatusCode::Ok;
    }

    void AnalogInput::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        pinMode        (pin_, INPUT);
        claim_.release ();
        initialized_ = false;
    }

    void AnalogInput::update () noexcept
    {
        if (initialized_)
        {
            reading_ = sample ();
        }
    }

    AnalogInput::Reading AnalogInput::sample () const noexcept
    {
        if (!initialized_)
        {
            return reading_;
        }

        const int reading = analogRead (pin_);

        if (reading <= 0)
        {
            return 0;
        }

        if (reading >= maximumReading)
        {
            return maximumReading;
        }

        return static_cast<Reading> (reading);
    }

    AnalogInput::Reading AnalogInput::read () const noexcept
    {
        return reading_;
    }

    PinId AnalogInput::pin () const noexcept
    {
        return pin_;
    }

    bool AnalogInput::initialized () const noexcept
    {
        return initialized_;
    }
}
