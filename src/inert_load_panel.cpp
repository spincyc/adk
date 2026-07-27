#include "inert_load_panel.h"

#include "inert_load_interlock.h"

namespace adk {

    InertLoadPanel::InertLoadPanel (PumpOutput& fanIndicator, PumpOutput& pumpIndicator,
                                    PumpOutput& heaterIndicator) noexcept
        : fan_ (&fanIndicator), pump_ (&pumpIndicator), heater_ (&heaterIndicator),
          active_ (SimulatedLoad::None), status_ (), initialized_ (false)
    {
    }

    InertLoadPanel::~InertLoadPanel () noexcept
    {
        shutdown ();
    }

    Status InertLoadPanel::initialize () noexcept
    {
        if (initialized_)
        {
            return status_;
        }

        if (fan_->initialized () || pump_->initialized () || heater_->initialized ())
        {
            status_ = StatusCode::InvalidArgument;
            return status_;
        }

        Status status = fan_->initialize ();

        if (!status.ok ())
        {
            status_ = status;
            return status_;
        }

        status = pump_->initialize ();

        if (!status.ok ())
        {
            fan_->shutdown ();
            status_ = status;
            return status_;
        }

        status = heater_->initialize ();

        if (!status.ok ())
        {
            pump_->shutdown ();
            fan_->shutdown  ();
            status_ = status;
            return status_;
        }

        status = fan_->setState (PumpState::Off);

        if (status.ok ())
        {
            status = pump_->setState (PumpState::Off);
        }

        if (status.ok ())
        {
            status = heater_->setState (PumpState::Off);
        }

        if (!status.ok ())
        {
            bestEffortOff     ();
            heater_->shutdown ();
            pump_->shutdown   ();
            fan_->shutdown    ();
            status_ = StatusCode::HardwareFailure;
            return status_;
        }

        active_      = SimulatedLoad::None;
        status_      = StatusCode::Ok;
        initialized_ = true;
        return status_;
    }

    void InertLoadPanel::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        bestEffortOff     ();
        heater_->shutdown ();
        pump_->shutdown   ();
        fan_->shutdown    ();
        active_      = SimulatedLoad::None;
        status_      = StatusCode::Ok;
        initialized_ = false;
    }

    Status InertLoadPanel::select (SimulatedLoad load) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!validLoad (load))
        {
            return StatusCode::InvalidArgument;
        }

        if (load == active_)
        {
            return status_;
        }

        PumpOutput* previous = outputFor (active_);

        if (previous != nullptr && !previous->setState (PumpState::Off).ok ())
        {
            bestEffortOff ();
            active_ = SimulatedLoad::None;
            status_ = StatusCode::HardwareFailure;
            return status_;
        }

        PumpOutput* next = outputFor (load);

        if (next != nullptr && !next->setState (PumpState::On).ok ())
        {
            bestEffortOff ();
            active_ = SimulatedLoad::None;
            status_ = StatusCode::HardwareFailure;
            return status_;
        }

        active_ = load;
        status_ = StatusCode::Ok;
        return status_;
    }

    SimulatedLoadSnapshot InertLoadPanel::snapshot () const noexcept
    {
        return {active_, status_};
    }

    bool InertLoadPanel::initialized () const noexcept
    {
        return initialized_;
    }

    PumpOutput* InertLoadPanel::outputFor (SimulatedLoad load) noexcept
    {
        switch (load)
        {
            case SimulatedLoad::Fan: return fan_;
            case SimulatedLoad::Pump: return pump_;
            case SimulatedLoad::Heater: return heater_;
            case SimulatedLoad::None: return nullptr;
        }

        return nullptr;
    }

    bool InertLoadPanel::validLoad (SimulatedLoad load) const noexcept
    {
        return load == SimulatedLoad::None || load == SimulatedLoad::Fan ||
               load == SimulatedLoad::Pump || load == SimulatedLoad::Heater;
    }

    void InertLoadPanel::bestEffortOff () noexcept
    {
        fan_->setState    (PumpState::Off);
        pump_->setState   (PumpState::Off);
        heater_->setState (PumpState::Off);
    }

    PanelPumpOutput::PanelPumpOutput (InertLoadPanel& panel) noexcept
        : panel_       (&panel)
        , initialized_ (false)
    {
    }

    PanelPumpOutput::~PanelPumpOutput () noexcept
    {
        shutdown ();
    }

    Status PanelPumpOutput::initialize () noexcept
    {
        if (initialized_ && panel_->initialized ())
        {
            return StatusCode::Ok;
        }

        initialized_ = false;

        if (!panel_->initialized ())
        {
            return StatusCode::NotInitialized;
        }

        const Status status = panel_->select (SimulatedLoad::None);

        if (status.ok ())
        {
            initialized_ = true;
        }

        return status;
    }

    void PanelPumpOutput::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        panel_->select (SimulatedLoad::None);
        initialized_ = false;
    }

    Status PanelPumpOutput::setState (PumpState state) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (state != PumpState::Off && state != PumpState::On)
        {
            return StatusCode::InvalidArgument;
        }

        return panel_->select (state == PumpState::On ? SimulatedLoad::Pump
                                                      : SimulatedLoad::None);
    }

    PumpState PanelPumpOutput::state () const noexcept
    {
        return initialized_ &&
                       panel_->snapshot ().active == SimulatedLoad::Pump
                   ? PumpState::On
                   : PumpState::Off;
    }

    bool PanelPumpOutput::initialized () const noexcept
    {
        return initialized_ && panel_->initialized ();
    }
} // namespace adk
