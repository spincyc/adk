#include "simon.h"

namespace adk {

    constexpr uint8_t Simon::cueCount;
    constexpr uint8_t Simon::sequenceCapacity;

    static constexpr uint32_t maximumDuration = 0x7fffffffu;
    static constexpr uint32_t defaultNonzeroSeed = 0x6d2b79f5u;
    static constexpr uint8_t validCueMask = 0x0fu;

    CueSource::~CueSource () noexcept = default;

    SimonInput::SimonInput (uint8_t active,
                            uint8_t pressed,
                            uint8_t released,
                            bool    start) noexcept
        : activeMask   (active)
        , pressedMask  (pressed)
        , releasedMask (released)
        , startEvent   (start)
    {
    }

    FixedCueSource::FixedCueSource (const CueId* cues, size_t count) noexcept
        : cues_  (cues)
        , count_ (count)
        , index_ (0)
    {
    }

    Status FixedCueSource::reset () noexcept
    {
        index_ = 0;
        return cues_ != nullptr && count_ > 0 ? Status::Ok
                                             : Status::InvalidArgument;
    }

    Status FixedCueSource::next (CueId& cue) noexcept
    {
        if (cues_ == nullptr || index_ >= count_)
        {
            return Status::CapacityExceeded;
        }

        cue = cues_[index_++];
        return static_cast<uint8_t> (cue) < Simon::cueCount
                   ? Status::Ok
                   : Status::InvalidArgument;
    }

    SimonAlgorithm FixedCueSource::algorithmVersion () const noexcept
    {
        return SimonAlgorithm::Fixed;
    }

    uint32_t FixedCueSource::seed () const noexcept
    {
        return 0;
    }

    XorShift32CueSource::XorShift32CueSource (uint32_t seed) noexcept
        : initialSeed_ (seed == 0 ? defaultNonzeroSeed : seed)
        , state_       (initialSeed_)
    {
    }

    Status XorShift32CueSource::reset () noexcept
    {
        state_ = initialSeed_;
        return Status::Ok;
    }

    Status XorShift32CueSource::next (CueId& cue) noexcept
    {
        state_ ^= state_ << 13u;
        state_ ^= state_ >> 17u;
        state_ ^= state_ << 5u;
        cue = static_cast<CueId> (state_ & 0x03u);
        return Status::Ok;
    }

    SimonAlgorithm XorShift32CueSource::algorithmVersion () const noexcept
    {
        return SimonAlgorithm::XorShift32V1;
    }

    uint32_t XorShift32CueSource::seed () const noexcept
    {
        return initialSeed_;
    }

    Simon::Simon (const SimonConfig& config, CueSource& source) noexcept
        : config_            (config)
        , source_            (&source)
        , sequence_          {}
        , phase_             (SimonPhase::Idle)
        , outcome_           (SimonOutcome::None)
        , status_            (Status::NotInitialized)
        , phaseSince_        (TimePoint (0))
        , lastUpdate_        (TimePoint (0))
        , observedCue_       (CueId::One)
        , sequenceLength_    (0)
        , playbackIndex_     (0)
        , playerIndex_       (0)
        , initialized_       (false)
        , hasLastUpdate_     (false)
        , hasObservedCue_    (false)
    {
    }

    Status Simon::initialize () noexcept
    {
        initialized_ = false;

        if (!configValid ())
        {
            status_ = Status::InvalidArgument;
            return status_;
        }

        status_ = source_->reset ();

        if (status_ != Status::Ok)
        {
            return status_;
        }

        sequenceLength_ = 0;
        status_         = growSequence (config_.startingLength);

        if (status_ != Status::Ok)
        {
            outcome_ = SimonOutcome::SourceFailure;
            return status_;
        }

        phase_          = SimonPhase::Idle;
        outcome_        = SimonOutcome::None;
        phaseSince_     = TimePoint (0);
        lastUpdate_     = TimePoint (0);
        observedCue_    = CueId::One;
        playbackIndex_  = 0;
        playerIndex_    = 0;
        initialized_    = true;
        hasLastUpdate_  = false;
        hasObservedCue_ = false;

        return Status::Ok;
    }

    Status Simon::update (TimePoint now, const SimonInput& input) noexcept
    {
        if (!initialized_)
        {
            return Status::NotInitialized;
        }

        if (!inputValid (input) || !timeValid (now))
        {
            status_ = Status::InvalidArgument;
            return status_;
        }

        status_        = Status::Ok;
        lastUpdate_    = now;
        hasLastUpdate_ = true;

        switch (phase_)
        {
            case SimonPhase::Idle:
                if (input.startEvent)
                {
                    outcome_        = SimonOutcome::None;
                    playerIndex_    = 0;
                    playbackIndex_  = 0;
                    hasObservedCue_ = false;
                    startPlayback (now);
                }
                break;

            case SimonPhase::PlaybackOn:
                if (deadlineDue (now, config_.cueOnDuration))
                {
                    enter (SimonPhase::PlaybackGap, now);
                }
                break;

            case SimonPhase::PlaybackGap:
                if (deadlineDue (now, config_.cueGapDuration))
                {
                    ++playbackIndex_;

                    if (playbackIndex_ < sequenceLength_)
                    {
                        enter (SimonPhase::PlaybackOn, now);
                    }
                    else
                    {
                        playerIndex_ = 0;
                        enter (SimonPhase::AwaitPress, now);
                    }
                }
                break;

            case SimonPhase::AwaitPress:
                if (deadlineDue (now, config_.inputTimeout))
                {
                    fail (SimonOutcome::Timeout, now);
                }
                else if (input.pressedMask != 0)
                {
                    if (!oneCue (input.pressedMask) || !oneCue (input.activeMask))
                    {
                        fail (SimonOutcome::InvalidInput, now, input.pressedMask);
                    }
                    else
                    {
                        observedCue_    = cueFromMask (input.pressedMask);
                        hasObservedCue_ = true;

                        if (observedCue_ != cueAt (playerIndex_))
                        {
                            fail (SimonOutcome::Mismatch, now, input.pressedMask);
                        }
                        else
                        {
                            enter (SimonPhase::AwaitRelease, now);
                        }
                    }
                }
                break;

            case SimonPhase::AwaitRelease:
                if (input.activeMask == 0)
                {
                    ++playerIndex_;

                    if (playerIndex_ >= sequenceLength_)
                    {
                        outcome_ = sequenceLength_ >= config_.maximumLength
                                       ? SimonOutcome::GameComplete
                                       : SimonOutcome::RoundComplete;
                        enter (sequenceLength_ >= config_.maximumLength
                                   ? SimonPhase::GameSuccess
                                   : SimonPhase::RoundSuccess,
                               now);
                    }
                    else
                    {
                        enter (SimonPhase::AwaitPress, now);
                    }
                }
                break;

            case SimonPhase::RoundSuccess:
                if (deadlineDue (now, config_.resultDuration))
                {
                    uint16_t target = static_cast<uint16_t> (sequenceLength_) +
                                      config_.growthPerRound;

                    if (target > config_.maximumLength)
                    {
                        target = config_.maximumLength;
                    }

                    status_ = growSequence (static_cast<uint8_t> (target));

                    if (status_ == Status::Ok)
                    {
                        outcome_ = SimonOutcome::None;
                        startPlayback (now);
                    }
                    else
                    {
                        outcome_ = SimonOutcome::SourceFailure;
                        enter (SimonPhase::GameFailure, now);
                    }
                }
                break;

            case SimonPhase::GameSuccess:
            case SimonPhase::GameFailure:
                if (input.startEvent)
                {
                    status_ = initialize ();

                    if (status_ == Status::Ok)
                    {
                        lastUpdate_    = now;
                        hasLastUpdate_ = true;
                        startPlayback (now);
                    }
                }
                break;
        }

        return status_;
    }

    SimonSnapshot Simon::snapshot () const noexcept
    {
        SimonSnapshot result = {
            phase_,
            outcome_,
            status_,
            CueId::One,
            CueId::One,
            observedCue_,
            TimePoint (0),
            0,
            sequenceLength_,
            playbackIndex_,
            playerIndex_,
            phase_ == SimonPhase::AwaitPress,
            false,
            false,
            hasObservedCue_,
            false
        };

        if (phase_ == SimonPhase::PlaybackOn)
        {
            result.displayedCue    = cueAt   (playbackIndex_);
            result.ledMask         = cueMask (result.displayedCue);
            result.hasDisplayedCue = true;
        }

        if (phase_ == SimonPhase::AwaitPress ||
            phase_ == SimonPhase::AwaitRelease)
        {
            result.expectedCue    = cueAt (playerIndex_);
            result.hasExpectedCue = true;
        }

        Duration duration;

        switch (phase_)
        {
            case SimonPhase::PlaybackOn:
                duration           = config_.cueOnDuration;
                result.hasDeadline = true;
                break;

            case SimonPhase::PlaybackGap:
                duration           = config_.cueGapDuration;
                result.hasDeadline = true;
                break;

            case SimonPhase::AwaitPress:
                duration           = config_.inputTimeout;
                result.hasDeadline = true;
                break;

            case SimonPhase::RoundSuccess:
                duration           = config_.resultDuration;
                result.hasDeadline = true;
                break;

            default:
                break;
        }

        if (result.hasDeadline)
        {
            result.nextDeadline = TimePoint (
                phaseSince_.milliseconds () + duration.milliseconds ());
        }

        return result;
    }

    SimonAlgorithm Simon::algorithmVersion () const noexcept
    {
        return source_->algorithmVersion ();
    }

    uint32_t Simon::seed () const noexcept
    {
        return source_->seed ();
    }

    bool Simon::configValid () const noexcept
    {
        const uint32_t on = config_.cueOnDuration.milliseconds ();

        const uint32_t gap = config_.cueGapDuration.milliseconds ();

        const uint32_t timeout = config_.inputTimeout.milliseconds ();

        const uint32_t result = config_.resultDuration.milliseconds ();

        return on > 0 && on <= maximumDuration && gap > 0 &&
               gap <= maximumDuration && timeout > 0 &&
               timeout <= maximumDuration && result > 0 &&
               result <= maximumDuration && config_.startingLength > 0 &&
               config_.growthPerRound > 0 &&
               config_.startingLength <= config_.maximumLength &&
               config_.maximumLength <= sequenceCapacity;
    }

    bool Simon::inputValid (const SimonInput& input) const noexcept
    {
        const uint8_t all = static_cast<uint8_t> (
            input.activeMask | input.pressedMask | input.releasedMask);

        return (all & static_cast<uint8_t> (~validCueMask)) == 0 &&
               (input.pressedMask & input.releasedMask) == 0 &&
               (input.pressedMask & input.activeMask) == input.pressedMask &&
               (input.releasedMask & input.activeMask) == 0;
    }

    bool Simon::timeValid (TimePoint now) const noexcept
    {
        return !hasLastUpdate_ || now == lastUpdate_ ||
               now.elapsedSince (lastUpdate_).milliseconds () <= maximumDuration;
    }

    bool Simon::deadlineDue (TimePoint now, Duration duration) const noexcept
    {
        return now.elapsedSince (phaseSince_) >= duration;
    }

    Status Simon::growSequence (uint8_t targetLength) noexcept
    {
        while (sequenceLength_ < targetLength)
        {
            CueId cue = CueId::One;
            const Status status = source_->next (cue);

            if (status != Status::Ok)
            {
                return status;
            }

            if (static_cast<uint8_t> (cue) >= cueCount)
            {
                return Status::InvalidArgument;
            }

            sequence_[sequenceLength_++] = cue;
        }

        return Status::Ok;
    }

    void Simon::startPlayback (TimePoint now) noexcept
    {
        playbackIndex_ = 0;
        playerIndex_   = 0;
        enter (SimonPhase::PlaybackOn, now);
    }

    void Simon::fail (SimonOutcome outcome,
                      TimePoint     now,
                      uint8_t       observedMask) noexcept
    {
        outcome_        = outcome;
        hasObservedCue_ = oneCue (observedMask);

        if (hasObservedCue_)
        {
            observedCue_ = cueFromMask (observedMask);
        }

        enter (SimonPhase::GameFailure, now);
    }

    void Simon::enter (SimonPhase phase, TimePoint now) noexcept
    {
        phase_      = phase;
        phaseSince_ = now;
    }

    CueId Simon::cueAt (uint8_t index) const noexcept
    {
        return sequence_[index];
    }

    uint8_t Simon::cueMask (CueId cue) const noexcept
    {
        return static_cast<uint8_t> (1u << static_cast<uint8_t> (cue));
    }

    CueId Simon::cueFromMask (uint8_t mask) const noexcept
    {
        uint8_t index = 0;

        while ((mask & 1u) == 0u)
        {
            mask >>= 1u;
            ++index;
        }

        return static_cast<CueId> (index);
    }

    bool Simon::oneCue (uint8_t mask) const noexcept
    {
        return mask != 0u && (mask & static_cast<uint8_t> (mask - 1u)) == 0u;
    }
}
