#include "reaction_timer.h"

#include "button.h"

namespace adk {

    static constexpr uint32_t maximumDuration    = 0x7fffffffu;
    static constexpr uint32_t readyPulsePeriod   = 500u;
    static constexpr uint32_t readyPulseOn       = 250u;
    static constexpr uint32_t successPulsePeriod = 250u;
    static constexpr uint32_t successPulseOn     = 125u;

    static ButtonObservation observe (const Button& button) noexcept
    {
        ButtonObservation observation;

        observation.pressed      = button.pressed      ();
        observation.pressEvent   = button.pressEvent   ();
        observation.releaseEvent = button.releaseEvent ();

        return observation;
    }

    ReactionTimer::ReactionTimer (const ReactionTimerConfig& config) noexcept
        : config_          (config)
        , state_           (ReactionState::Idle)
        , outcome_         (ReactionOutcome::None)
        , status_          (StatusCode::NotInitialized)
        , stateSince_      (TimePoint (0))
        , cueTime_         (TimePoint (0))
        , lastUpdate_      (TimePoint (0))
        , reactionTime_    (Duration (0))
        , initialized_     (false)
        , hasLastUpdate_   (false)
        , hasCueTime_      (false)
        , hasReactionTime_ (false)
    {
    }

    Status ReactionTimer::initialize () noexcept
    {
        if (!configValid ())
        {
            status_      = StatusCode::InvalidArgument;
            initialized_ = false;
            return StatusCode::InvalidArgument;
        }

        state_           = ReactionState::Idle;
        outcome_         = ReactionOutcome::None;
        status_          = StatusCode::Ok;
        stateSince_      = TimePoint (0);
        cueTime_         = TimePoint (0);
        lastUpdate_      = TimePoint (0);
        reactionTime_    = Duration  (0);
        initialized_     = true;
        hasLastUpdate_   = false;
        hasCueTime_      = false;
        hasReactionTime_ = false;

        return StatusCode::Ok;
    }

    Status ReactionTimer::update (TimePoint               now,
                                  const ButtonObservation& button) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!timeValid (now))
        {
            status_ = StatusCode::InvalidArgument;
            return StatusCode::InvalidArgument;
        }

        status_        = StatusCode::Ok;
        lastUpdate_    = now;
        hasLastUpdate_ = true;

        switch (state_)
        {
        case ReactionState::Idle:
            if (button.pressEvent)
            {
                outcome_         = ReactionOutcome::None;
                hasCueTime_      = false;
                hasReactionTime_ = false;
                enter (ReactionState::AwaitRelease, now);
            }
            break;

        case ReactionState::AwaitRelease:
            if (button.releaseEvent || !button.pressed)
            {
                enter (ReactionState::Ready, now);
            }
            break;

        case ReactionState::Ready:
            if (deadlineDue (now, config_.readyDuration))
            {
                enter (ReactionState::Wait, now);
            }
            break;

        case ReactionState::Wait:
            if (button.pressEvent)
            {
                outcome_ = ReactionOutcome::PrematurePress;
                enter (ReactionState::Failure, now);
            } else if (deadlineDue (now, config_.waitDuration))
            {
                cueTime_    = now;
                hasCueTime_ = true;
                enter (ReactionState::Cue, now);
            }
            break;

        case ReactionState::Cue:
            if (deadlineDue (now, config_.responseTimeout))
            {
                outcome_ = ReactionOutcome::Timeout;
                enter (ReactionState::Failure, now);
            } else if (button.pressEvent)
            {
                outcome_         = ReactionOutcome::Success;
                reactionTime_    = now.elapsedSince (cueTime_);
                hasReactionTime_ = true;
                enter (ReactionState::Success, now);
            }
            break;

        case ReactionState::Success:
        case ReactionState::Failure:
            if (deadlineDue (now, config_.resultDuration))
            {
                enter (ReactionState::Idle, now);
            }
            break;
        }

        return StatusCode::Ok;
    }

    Status ReactionTimer::update (TimePoint now, const Button& button) noexcept
    {
        return update (now, observe (button));
    }

    ReactionTimerSnapshot ReactionTimer::snapshot () const noexcept
    {
        ReactionTimerSnapshot result = {
            state_,      outcome_,        ReactionLedPattern::Off,      status_,
            cueTime_,    reactionTime_,   state_ == ReactionState::Cue, false,
            hasCueTime_, hasReactionTime_};

        switch (state_)
        {
        case ReactionState::Ready:
            result.ledPattern = ReactionLedPattern::ReadyPulse;
            break;

        case ReactionState::Cue: result.ledPattern = ReactionLedPattern::Cue; break;

        case ReactionState::Success:
            result.ledPattern = ReactionLedPattern::SuccessPulse;
            break;

        case ReactionState::Failure:
            result.ledPattern = ReactionLedPattern::FailurePulse;
            break;

        default: break;
        }

        result.ledOn = outputActive (lastUpdate_);
        return result;
    }

    bool ReactionTimer::configValid () const noexcept
    {
        const Duration::Raw ready = config_.readyDuration.milliseconds ();

        const Duration::Raw wait = config_.waitDuration.milliseconds ();

        const Duration::Raw response = config_.responseTimeout.milliseconds ();

        const Duration::Raw result = config_.resultDuration.milliseconds ();

        return ready > 0u && ready <= maximumDuration && wait > 0u &&
               wait <= maximumDuration && response > 0u &&
               response <= maximumDuration && result > 0u && result <= maximumDuration;
    }

    bool ReactionTimer::deadlineDue (TimePoint now, Duration duration) const noexcept
    {
        return now.elapsedSince (stateSince_) >= duration;
    }

    bool ReactionTimer::timeValid (TimePoint now) const noexcept
    {
        if (!hasLastUpdate_ || now == lastUpdate_)
        {
            return true;
        }

        return now.elapsedSince (lastUpdate_).milliseconds () <= maximumDuration;
    }

    bool ReactionTimer::outputActive (TimePoint now) const noexcept
    {
        const uint32_t elapsed = now.elapsedSince (stateSince_).milliseconds ();

        switch (state_)
        {
        case ReactionState::Ready: return elapsed % readyPulsePeriod < readyPulseOn;

        case ReactionState::Cue: return true;

        case ReactionState::Success:
            return elapsed < successPulsePeriod * 2u &&
                   elapsed % successPulsePeriod < successPulseOn;

        case ReactionState::Failure: return true;

        default: return false;
        }
    }

    void ReactionTimer::enter (ReactionState state, TimePoint now) noexcept
    {
        state_      = state;
        stateSince_ = now;
    }
}
