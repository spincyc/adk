#include "pwm_output.h"

#include "board.h"

#include <Arduino.h>

namespace adk {

    PwmOutput::PwmOutput (ResourceRegistry& resources,
                          PinId            pin,
                          Duty             initialDuty) noexcept
        : resources_   (&resources)
        , pinClaim_    ()
        , timerClaim_  ()
        , pin_         (pin)
        , initialDuty_ (initialDuty)
        , duty_        (initialDuty)
        , initialized_ (false)
    {
    }

    PwmOutput::~PwmOutput () noexcept
    {
        shutdown ();
    }

    Status PwmOutput::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (pin_))
        {
            return StatusCode::InvalidPin;
        }

        if (!Mega2560Board::supports (pin_, PinCapability::PwmOutput))
        {
            return StatusCode::Unsupported;
        }

        uint8_t timer = 0;
        Status  status = Mega2560Board::pwmTimer (pin_, timer);

        if (!status.ok ())
        {
            return status;
        }

        status = resources_->claim (
            {ResourceKind::Pin, pin_},
            pinClaim_);

        if (!status.ok ())
        {
            return status;
        }

        status = resources_->claimShared (
            {ResourceKind::Timer, timer},
            timerClaim_);

        if (!status.ok ())
        {
            pinClaim_.release ();
            return status;
        }

        analogWrite  (pin_, initialDuty_);
        duty_        = initialDuty_;
        initialized_ = true;
        return StatusCode::Ok;
    }

    void PwmOutput::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        analogWrite    (pin_, 0);
        pinMode        (pin_, INPUT);

        timerClaim_.release ();
        pinClaim_  .release ();

        duty_        = 0;
        initialized_ = false;
    }

    Status PwmOutput::write (Duty duty) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        analogWrite (pin_, duty);
        duty_ = duty;
        return StatusCode::Ok;
    }

    PinId PwmOutput::pin () const noexcept
    {
        return pin_;
    }

    PwmOutput::Duty PwmOutput::duty () const noexcept
    {
        return duty_;
    }

    bool PwmOutput::initialized () const noexcept
    {
        return initialized_;
    }
}
