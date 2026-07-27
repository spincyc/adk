#include "pump_output.h"

namespace adk {

    PumpOutput::~PumpOutput () noexcept = default;

    IndicatorPump::IndicatorPump (ResourceRegistry& resources,
                                  PinId             indicatorPin) noexcept
        : output_ (resources, indicatorPin, Level::Low), state_ (PumpState::Off)
    {
    }

    IndicatorPump::~IndicatorPump () noexcept
    {
        shutdown ();
    }

    Status IndicatorPump::initialize () noexcept
    {
        if (output_.initialized ())
        {
            return StatusCode::Ok;
        }

        const Status status = output_.initialize ();

        if (!status.ok ())
        {
            state_ = PumpState::Off;
            return status;
        }

        state_ = PumpState::Off;
        return status;
    }

    void IndicatorPump::shutdown () noexcept
    {
        if (output_.initialized ())
        {
            output_.write (Level::Low);
        }

        output_.shutdown ();
        state_ = PumpState::Off;
    }

    Status IndicatorPump::setState (PumpState state) noexcept
    {
        if (state != PumpState::Off && state != PumpState::On)
        {
            return StatusCode::InvalidArgument;
        }

        const Status status =
            output_.write (state == PumpState::On ? Level::High : Level::Low);

        if (status.ok ())
        {
            state_ = state;
        }

        return status;
    }

    PumpState IndicatorPump::state () const noexcept
    {
        return state_;
    }

    bool IndicatorPump::initialized () const noexcept
    {
        return output_.initialized ();
    }
} // namespace adk
