#include "threshold_input.h"

namespace adk {

    ThresholdInput::ThresholdInput (const ThresholdInputConfig& config) noexcept
        : config_      (config)
        , observation_ { 0,
                         ThresholdFault::InvalidConfiguration,
                         false,
                         false,
                         false }
    {
        reset ();
    }

    void ThresholdInput::reset () noexcept
    {
        observation_.raw     = 0;
        observation_.fault   = configValid ()
            ? ThresholdFault::SourceFailure
            : ThresholdFault::InvalidConfiguration;
        observation_.active  = false;
        observation_.changed = false;
        observation_.valid   = false;
    }

    bool ThresholdInput::validConfig () const noexcept
    {
        return configValid ();
    }

    ThresholdObservation ThresholdInput::update (uint16_t       raw,
                                                 ThresholdFault sourceFault) noexcept
    {
        const ThresholdFault fault = classify (raw, sourceFault);

        observation_.raw     = raw;
        observation_.fault   = fault;
        observation_.changed = false;
        observation_.valid   = fault == ThresholdFault::None;

        if (!observation_.valid)
        {
            return observation_;
        }

        const bool previousActive = observation_.active;
        observation_.active  = nextActive (raw);
        observation_.changed = observation_.active != previousActive;

        return observation_;
    }

    ThresholdObservation ThresholdInput::observation () const noexcept
    {
        return observation_;
    }

    bool ThresholdInput::configValid () const noexcept
    {
        if (config_.validMinimum > config_.validMaximum)
        {
            return false;
        }

        if (config_.activateAt < config_.validMinimum
            || config_.activateAt > config_.validMaximum
            || config_.deactivateAt < config_.validMinimum
            || config_.deactivateAt > config_.validMaximum)
        {
            return false;
        }

        if (config_.direction == ThresholdDirection::Rising)
        {
            return config_.deactivateAt < config_.activateAt;
        }

        return config_.activateAt < config_.deactivateAt;
    }

    ThresholdFault ThresholdInput::classify (uint16_t       raw,
                                             ThresholdFault sourceFault) const noexcept
    {
        if (!configValid ())
        {
            return ThresholdFault::InvalidConfiguration;
        }
        if (sourceFault != ThresholdFault::None)
        {
            return sourceFault;
        }
        if (raw < config_.validMinimum)
        {
            return ThresholdFault::BelowRange;
        }
        if (raw > config_.validMaximum)
        {
            return ThresholdFault::AboveRange;
        }

        return ThresholdFault::None;
    }

    bool ThresholdInput::nextActive (uint16_t raw) const noexcept
    {
        if (config_.direction == ThresholdDirection::Rising)
        {
            return observation_.active
                ? raw >= config_.deactivateAt
                : raw >= config_.activateAt;
        }

        return observation_.active
            ? raw <= config_.deactivateAt
            : raw <= config_.activateAt;
    }
}
