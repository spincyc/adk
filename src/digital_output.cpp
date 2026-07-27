#include "digital_output.h"

#include <Arduino.h>

namespace adk {

    DigitalOutput::DigitalOutput (ResourceRegistry& resources,
                                  PinId            pin,
                                  Level            initial) noexcept
        : resources_   (&resources)
        , claim_       ()
        , pin_         (pin)
        , initial_     (initial)
        , level_       (initial)
        , initialized_ (false)
    {
    }

    DigitalOutput::~DigitalOutput () noexcept
    {
        shutdown ();
    }

    Status DigitalOutput::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (pin_ >= NUM_DIGITAL_PINS)
        {
            return StatusCode::InvalidPin;
        }

        Status status =
            resources_->claim ({ResourceKind::Pin, pin_}, claim_);

        if (!status.ok ())
        {
            return status;
        }

        digitalWrite (pin_, initial_ == Level::High ? HIGH : LOW);
        pinMode      (pin_, OUTPUT);

        level_       = initial_;
        initialized_ = true;
        return StatusCode::Ok;
    }

    void DigitalOutput::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        pinMode        (pin_, INPUT);
        claim_.release ();

        initialized_ = false;
    }

    Status DigitalOutput::write (Level level) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        digitalWrite (pin_, level == Level::High ? HIGH : LOW);
        level_ = level;
        return StatusCode::Ok;
    }

    PinId DigitalOutput::pin () const noexcept
    {
        return pin_;
    }

    Level DigitalOutput::level () const noexcept
    {
        return level_;
    }

    bool DigitalOutput::initialized () const noexcept
    {
        return initialized_;
    }
}
