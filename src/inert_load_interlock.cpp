#include "inert_load_interlock.h"

#include "power_domain.h"

namespace adk {

    InertLoadInterlock::InertLoadInterlock (const PowerDomain& power) noexcept
        : power_ (&power), requested_ (SimulatedLoad::None),
          active_ (SimulatedLoad::None), status_ (), initialized_ (false)
    {
    }

    InertLoadInterlock::~InertLoadInterlock () noexcept
    {
        shutdown ();
    }

    Status InertLoadInterlock::initialize () noexcept
    {
        if (initialized_)
        {
            return status_;
        }

        requested_   = SimulatedLoad::None;
        active_      = SimulatedLoad::None;
        status_      = StatusCode::Ok;
        initialized_ = true;
        return status_;
    }

    void InertLoadInterlock::shutdown () noexcept
    {
        requested_   = SimulatedLoad::None;
        active_      = SimulatedLoad::None;
        status_      = StatusCode::Ok;
        initialized_ = false;
    }

    Status InertLoadInterlock::select (SimulatedLoad load) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!validLoad (load))
        {
            return StatusCode::InvalidArgument;
        }

        requested_ = load;

        if (load == SimulatedLoad::None)
        {
            active_ = SimulatedLoad::None;
            return status_;
        }

        if (!status_.ok ())
        {
            active_ = SimulatedLoad::None;
            return status_;
        }

        if (!power_->commandAdmitted ())
        {
            active_ = SimulatedLoad::None;
            status_ = StatusCode::HardwareFailure;
            return status_;
        }

        active_ = load;
        return status_;
    }

    Status InertLoadInterlock::update () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (active_ != SimulatedLoad::None && !power_->commandAdmitted ())
        {
            active_ = SimulatedLoad::None;
            status_ = StatusCode::HardwareFailure;
        }

        return status_;
    }

    InertLoadSnapshot InertLoadInterlock::snapshot () const noexcept
    {
        return {requested_, active_, status_};
    }

    bool InertLoadInterlock::initialized () const noexcept
    {
        return initialized_;
    }

    bool InertLoadInterlock::validLoad (SimulatedLoad load) const noexcept
    {
        return load == SimulatedLoad::None || load == SimulatedLoad::Fan ||
               load == SimulatedLoad::Pump || load == SimulatedLoad::Heater;
    }
} // namespace adk
