#include "motor_intent.h"

#include "power_domain.h"

namespace adk {

    namespace {

        constexpr uint32_t maximumForwardElapsed = 0x7FFFFFFFUL;
    }

    MotorIntentConfig::MotorIntentConfig (Duration reversalDeadTime,
                              uint8_t  maximumDuty) noexcept
        : reversalDeadTime (reversalDeadTime)
        , maximumDuty      (maximumDuty)
    {
    }

    MotorCommand::MotorCommand (MotorDirection direction,
                                uint8_t        duty) noexcept
        : direction (direction)
        , duty      (duty)
    {
    }

    MotorIntent::MotorIntent (const MotorIntentConfig& config,
                              const PowerDomain& power) noexcept
        : config_              (config)
        , power_               (&power)
        , requested_           ()
        , applied_             ()
        , directionBeforeWait_ (MotorDirection::Stopped)
        , phase_               (MotorIntentPhase::Inactive)
        , status_              ()
        , phaseSince_          ()
        , lastUpdate_          ()
        , initialized_         (false)
        , hasLastUpdate_       (false)
        , hasDeadline_         (false)
        , transitionCount_     (0)
    {
    }

    Status MotorIntent::initialize () noexcept
    {
        if (initialized_)
        {
            return status_;
        }

        if (!configValid ())
        {
            status_ = StatusCode::InvalidArgument;
            return status_;
        }

        requested_           = MotorCommand ();
        applied_             = MotorCommand ();
        directionBeforeWait_ = MotorDirection::Stopped;
        phase_               = MotorIntentPhase::Inactive;
        status_              = StatusCode::Ok;
        phaseSince_          = TimePoint ();
        lastUpdate_          = TimePoint ();
        initialized_         = true;
        hasLastUpdate_       = false;
        hasDeadline_         = false;
        transitionCount_     = 0;
        return status_;
    }

    void MotorIntent::shutdown () noexcept
    {
        requested_           = MotorCommand ();
        applied_             = MotorCommand ();
        directionBeforeWait_ = MotorDirection::Stopped;
        phase_               = MotorIntentPhase::Inactive;
        status_              = StatusCode::Ok;
        phaseSince_          = TimePoint ();
        lastUpdate_          = TimePoint ();
        initialized_         = false;
        hasLastUpdate_       = false;
        hasDeadline_         = false;
        transitionCount_     = 0;
    }

    Status MotorIntent::command (const MotorCommand& command,
                                 TimePoint           now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!status_.ok ())
        {
            return status_;
        }

        if (!commandValid (command))
        {
            return StatusCode::InvalidArgument;
        }

        if (!timeValid (now))
        {
            return enterFault (StatusCode::InvalidArgument, now);
        }

        lastUpdate_    = now;
        hasLastUpdate_ = true;

        if (command.direction == MotorDirection::Stopped)
        {
            requested_ = command;
            enterStopped (now);
            return status_;
        }

        if (!power_->commandAdmitted ())
        {
            requested_ = command;
            return enterFault (StatusCode::HardwareFailure, now);
        }

        if (phase_ == MotorIntentPhase::WaitingForDeadTime)
        {
            requested_ = command;

            if (command.direction == directionBeforeWait_)
            {
                apply (command, MotorIntentPhase::Running, now);
            }

            return status_;
        }

        requested_ = command;

        if (applied_.direction == MotorDirection::Stopped ||
            applied_.direction == command.direction)
        {
            apply (command, MotorIntentPhase::Running, now);
            return status_;
        }

        directionBeforeWait_ = applied_.direction;
        applied_             = MotorCommand ();
        phase_               = MotorIntentPhase::WaitingForDeadTime;
        phaseSince_          = now;
        hasDeadline_         = true;
        ++transitionCount_;
        return status_;
    }

    Status MotorIntent::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!status_.ok ())
        {
            return status_;
        }

        if (!timeValid (now))
        {
            return enterFault (StatusCode::InvalidArgument, now);
        }

        lastUpdate_    = now;
        hasLastUpdate_ = true;

        if (phase_ == MotorIntentPhase::Running ||
            phase_ == MotorIntentPhase::WaitingForDeadTime)
        {
            if (!power_->commandAdmitted ())
            {
                return enterFault (StatusCode::HardwareFailure, now);
            }
        }

        if (phase_ == MotorIntentPhase::WaitingForDeadTime && deadlineDue (now))
        {
            apply (requested_, MotorIntentPhase::Running, now);
        }

        return status_;
    }

    Status MotorIntent::stop () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        requested_   = MotorCommand ();
        applied_     = MotorCommand ();
        hasDeadline_ = false;

        if (phase_ != MotorIntentPhase::Fault && phase_ != MotorIntentPhase::Inactive)
        {
            phase_ = MotorIntentPhase::Inactive;
            ++transitionCount_;
        }

        return status_;
    }

    MotorIntentSnapshot MotorIntent::snapshot () const noexcept
    {
        const uint32_t deadlineMilliseconds =
            phaseSince_.milliseconds              () +
            config_.reversalDeadTime.milliseconds ();
        const TimePoint nextDeadline (deadlineMilliseconds);

        return
        {
            requested_,
            applied_,
            phase_,
            status_,
            phaseSince_,
            nextDeadline,
            hasDeadline_,
            transitionCount_
        };
    }

    bool MotorIntent::initialized () const noexcept
    {
        return initialized_;
    }

    bool MotorIntent::configValid () const noexcept
    {
        return config_.reversalDeadTime.milliseconds () > 0 &&
               config_.reversalDeadTime.milliseconds () <=
                   maximumForwardElapsed &&
               config_.maximumDuty > 0;
    }

    bool MotorIntent::commandValid (const MotorCommand& command) const noexcept
    {
        if (command.direction == MotorDirection::Stopped)
        {
            return command.duty == 0;
        }

        return (command.direction == MotorDirection::Forward ||
                command.direction == MotorDirection::Reverse) &&
               command.duty > 0 &&
               command.duty <= config_.maximumDuty;
    }

    bool MotorIntent::timeValid (TimePoint now) const noexcept
    {
        return !hasLastUpdate_ ||
               now.elapsedSince (lastUpdate_).milliseconds () <=
                   maximumForwardElapsed;
    }

    bool MotorIntent::deadlineDue (TimePoint now) const noexcept
    {
        return now.elapsedSince (phaseSince_) >= config_.reversalDeadTime;
    }

    Status MotorIntent::enterFault (Status status, TimePoint now) noexcept
    {
        applied_     = MotorCommand ();
        phase_       = MotorIntentPhase::Fault;
        status_      = status;
        phaseSince_  = now;
        hasDeadline_ = false;
        ++transitionCount_;
        return status_;
    }

    void MotorIntent::apply (const MotorCommand& command,
                             MotorIntentPhase         phase,
                             TimePoint          now) noexcept
    {
        const bool phaseChanged = phase_ != phase;

        applied_             = command;
        directionBeforeWait_ = MotorDirection::Stopped;
        phase_               = phase;
        hasDeadline_         = false;

        if (phaseChanged)
        {
            phaseSince_ = now;
            ++transitionCount_;
        }
    }

    void MotorIntent::enterStopped (TimePoint now) noexcept
    {
        apply (MotorCommand (), MotorIntentPhase::Inactive, now);
    }
} // namespace adk
