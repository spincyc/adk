#include "pulse_input.h"

namespace adk {

    static constexpr uint32_t maximumUnambiguousDuration = 0x7fffffffu;

    MicrosecondDuration::MicrosecondDuration (Raw microseconds) noexcept
        : microseconds_ (microseconds)
    {
    }

    MicrosecondDuration::Raw MicrosecondDuration::microseconds () const noexcept
    {
        return microseconds_;
    }

    MicrosecondTimePoint::MicrosecondTimePoint (Raw microseconds) noexcept
        : microseconds_ (microseconds)
    {
    }

    MicrosecondTimePoint::Raw MicrosecondTimePoint::microseconds () const noexcept
    {
        return microseconds_;
    }

    MicrosecondDuration
    MicrosecondTimePoint::elapsedSince (MicrosecondTimePoint earlier) const noexcept
    {
        return MicrosecondDuration (microseconds_ - earlier.microseconds_);
    }

    PulseInput::PulseInput (const PulseInputConfig& config) noexcept
        : config_ (config), state_ (PulseInputState::Idle), phaseStarted_ (),
          pulseStarted_ (), highDuration_ (), inputHigh_ (false), initialized_ (false)
    {
    }

    Status PulseInput::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validConfig ())
        {
            return StatusCode::InvalidArgument;
        }

        reset ();
        initialized_ = true;
        return StatusCode::Ok;
    }

    void PulseInput::reset () noexcept
    {
        state_        = PulseInputState::Idle;
        phaseStarted_ = MicrosecondTimePoint ();
        pulseStarted_ = MicrosecondTimePoint ();
        highDuration_ = MicrosecondDuration  ();
        inputHigh_    = false;
    }

    Status PulseInput::arm (MicrosecondTimePoint now, bool inputHigh) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        state_ =
            inputHigh ? PulseInputState::AwaitingLow : PulseInputState::AwaitingRise;
        phaseStarted_ = now;
        pulseStarted_ = now;
        highDuration_ = MicrosecondDuration ();
        inputHigh_    = inputHigh;
        return StatusCode::Ok;
    }

    Status PulseInput::update (MicrosecondTimePoint now, bool inputHigh) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        inputHigh_ = inputHigh;

        if (state_ == PulseInputState::AwaitingLow)
        {
            if (!inputHigh)
            {
                state_        = PulseInputState::AwaitingRise;
                phaseStarted_ = now;
            }
            else if (exceeded (now, phaseStarted_, config_.edgeTimeout))
            {
                state_ = PulseInputState::Timeout;
            }
        }
        else if (state_ == PulseInputState::AwaitingRise)
        {
            if (inputHigh)
            {
                state_        = PulseInputState::MeasuringHigh;
                pulseStarted_ = now;
            }
            else if (exceeded (now, phaseStarted_, config_.edgeTimeout))
            {
                state_ = PulseInputState::Timeout;
            }
        }
        else if (state_ == PulseInputState::MeasuringHigh)
        {
            const MicrosecondDuration duration = now.elapsedSince (pulseStarted_);

            if (!inputHigh)
            {
                highDuration_ = duration;
                state_        = PulseInputState::Complete;
            }
            else if (duration.microseconds () > config_.maximumPulse.microseconds ())
            {
                state_ = PulseInputState::Timeout;
            }
        }

        return StatusCode::Ok;
    }

    PulseInputSnapshot PulseInput::snapshot () const noexcept
    {
        const PulseInputSnapshot result = {state_, highDuration_, inputHigh_,
                                           state_ == PulseInputState::Complete,
                                           state_ == PulseInputState::Timeout};
        return result;
    }

    bool PulseInput::initialized () const noexcept
    {
        return initialized_;
    }

    bool PulseInput::validConfig () const noexcept
    {
        const uint32_t edgeTimeout  = config_.edgeTimeout.microseconds  ();
        const uint32_t maximumPulse = config_.maximumPulse.microseconds ();

        return edgeTimeout > 0 && maximumPulse > 0 &&
               edgeTimeout <= maximumUnambiguousDuration &&
               maximumPulse <= maximumUnambiguousDuration;
    }

    bool PulseInput::exceeded (MicrosecondTimePoint now, MicrosecondTimePoint earlier,
                               MicrosecondDuration limit) const noexcept
    {
        return now.elapsedSince (earlier).microseconds () > limit.microseconds ();
    }
} // namespace adk
