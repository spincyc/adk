#include "watering_controller.h"

namespace adk {

    namespace {

        constexpr uint32_t maximumForwardElapsed = 0x7FFFFFFFUL;
    }

    WateringController::WateringController (const WateringConfig& config,
                                            PumpOutput&           pump) noexcept
        : config_ (config), pump_ (&pump), state_ (WateringState::Starting),
          reason_       (WateringReason::None), requested_ (PumpState::Off),
          applied_      (PumpState::Off), stateSince_ (), initialized_ (false),
          hasStateTime_ (false)
    {
    }

    WateringController::~WateringController () noexcept
    {
        shutdown ();
    }

    Status WateringController::initialize () noexcept
    {
        if (initialized_)
        {
            return state_ == WateringState::OutputFault
                       ? Status (StatusCode::HardwareFailure)
                       : Status ();
        }

        if (!configValid ())
        {
            return StatusCode::InvalidArgument;
        }

        const Status status = pump_->initialize ();

        if (!status.ok ())
        {
            pump_->shutdown ();
            return status;
        }

        const Status offStatus = pump_->setState (PumpState::Off);

        if (!offStatus.ok ())
        {
            pump_->shutdown ();
            return offStatus;
        }

        state_        = WateringState::Starting;
        reason_       = WateringReason::None;
        requested_    = PumpState::Off;
        applied_      = PumpState::Off;
        stateSince_   = TimePoint ();
        initialized_  = true;
        hasStateTime_ = false;
        return StatusCode::Ok;
    }

    void WateringController::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        pump_->setState (PumpState::Off);
        pump_->shutdown ();
        state_        = WateringState::Starting;
        reason_       = WateringReason::Shutdown;
        requested_    = PumpState::Off;
        applied_      = PumpState::Off;
        stateSince_   = TimePoint ();
        initialized_  = false;
        hasStateTime_ = false;
    }

    Status WateringController::decide (TimePoint now, const MoistureSample& sample,
                                       bool wateringAllowed) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (state_ == WateringState::OutputFault)
        {
            requested_ = PumpState::Off;
            return StatusCode::HardwareFailure;
        }

        if (!sampleValid (sample))
        {
            enter (WateringState::SensorFault, WateringReason::InvalidSample,
                   PumpState::Off, now);
            return StatusCode::Ok;
        }

        if (!wateringAllowed)
        {
            enter (WateringState::Idle, WateringReason::OperatorInhibit, PumpState::Off,
                   now);
            return StatusCode::Ok;
        }

        if (!hasStateTime_)
        {
            enter (WateringState::Idle, WateringReason::None, PumpState::Off, now);
            return StatusCode::Ok;
        }

        if (state_ == WateringState::SensorFault)
        {
            enter (WateringState::Idle, WateringReason::None, PumpState::Off, now);
            return StatusCode::Ok;
        }

        if (state_ == WateringState::Watering)
        {
            if (elapsed (now, config_.maximumOnTime))
            {
                enter (WateringState::LockedOut, WateringReason::MaximumOnTime,
                       PumpState::Off, now);
            }
            else if (sample.moisturePermille >= config_.stopAtPermille)
            {
                enter (WateringState::Idle, WateringReason::WetThreshold,
                       PumpState::Off, now);
            }

            return StatusCode::Ok;
        }

        if (state_ == WateringState::LockedOut)
        {
            if (elapsed (now, config_.minimumOffTime))
            {
                state_     = WateringState::Idle;
                reason_    = WateringReason::MinimumOffTime;
                requested_ = PumpState::Off;
            }

            return StatusCode::Ok;
        }

        if (sample.moisturePermille < config_.startBelowPermille)
        {
            if (elapsed (now, config_.minimumOffTime))
            {
                enter (WateringState::Watering, WateringReason::DryThreshold,
                       PumpState::On, now);
            }
            else
            {
                reason_ = WateringReason::MinimumOffTime;
            }
        }

        return StatusCode::Ok;
    }

    Status WateringController::actuate () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (state_ == WateringState::OutputFault)
        {
            return StatusCode::HardwareFailure;
        }

        if (requested_ == applied_)
        {
            return StatusCode::Ok;
        }

        const Status status = pump_->setState (requested_);

        if (!status.ok ())
        {
            return enterOutputFault ();
        }

        applied_ = requested_;
        return StatusCode::Ok;
    }

    WateringSnapshot WateringController::snapshot () const noexcept
    {
        return {state_, reason_, requested_, stateSince_};
    }

    bool WateringController::initialized () const noexcept
    {
        return initialized_;
    }

    bool WateringController::configValid () const noexcept
    {
        return config_.startBelowPermille < config_.stopAtPermille &&
               config_.stopAtPermille <= 1000 &&
               config_.maximumOnTime.milliseconds  () != 0 &&
               config_.minimumOffTime.milliseconds () != 0 &&
               config_.maximumOnTime.milliseconds  () <= maximumForwardElapsed &&
               config_.minimumOffTime.milliseconds () <= maximumForwardElapsed;
    }

    bool WateringController::sampleValid (const MoistureSample& sample) const noexcept
    {
        return sample.state == MoistureSampleState::Valid &&
               sample.moisturePermille <= 1000;
    }

    bool WateringController::elapsed (TimePoint now, Duration interval) const noexcept
    {
        return now.elapsedSince (stateSince_) >= interval;
    }

    void WateringController::enter (WateringState state, WateringReason reason,
                                    PumpState pump, TimePoint now) noexcept
    {
        if (state_ != state)
        {
            stateSince_ = now;
        }

        state_        = state;
        reason_       = reason;
        requested_    = pump;
        hasStateTime_ = true;
    }

    Status WateringController::enterOutputFault () noexcept
    {
        pump_->setState (PumpState::Off);
        state_     = WateringState::OutputFault;
        reason_    = WateringReason::OutputFailure;
        requested_ = PumpState::Off;
        applied_   = PumpState::Off;
        return StatusCode::HardwareFailure;
    }
} // namespace adk
