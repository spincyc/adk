#include "magnetic_observation.h"

#include "board.h"

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        bool validForwardTime (TimePoint now, TimePoint earlier) noexcept
        {
            return now.elapsedSince (earlier).milliseconds () < halfRange;
        }

        MagneticObservation emptyObservation (MagneticSource source) noexcept
        {
            return {source,
                    0,
                    Level::Low,
                    TimePoint (),
                    source == MagneticSource::LinearAnalog
                        ? MagneticPolarity::Neutral
                        : MagneticPolarity::Unspecified,
                    false,
                    false,
                    false,
                    Duration (),
                    MagneticQuality::Unqualified,
                    StatusCode::NotInitialized};
        }

        bool validPull (Pull pull) noexcept
        {
            return pull == Pull::None || pull == Pull::Up;
        }

        bool validLevel (Level level) noexcept
        {
            return level == Level::Low || level == Level::High;
        }
    } // namespace

    LinearHall::LinearHall (ResourceRegistry&       resources,
                            const LinearHallConfig& config) noexcept
        : config_         (config)
        , input_          (resources, config.pin)
        , snapshot_       (emptyObservation (MagneticSource::LinearAnalog))
        , candidate_      (MagneticPolarity::Neutral)
        , candidateSince_ ()
        , stableSince_    ()
        , lastUpdate_     ()
        , hasCandidate_   (false)
        , hasUpdate_      (false)
        , initialized_    (false)
    {
    }

    LinearHall::~LinearHall () noexcept
    {
        shutdown ();
    }

    Status LinearHall::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (config_.pin))
        {
            snapshot_.status = StatusCode::InvalidPin;
            return snapshot_.status;
        }

        if (!Mega2560Board::supports (config_.pin, PinCapability::AnalogInput))
        {
            snapshot_.status = StatusCode::Unsupported;
            return snapshot_.status;
        }

        if (config_.qualifiedMinimum > config_.negativeActivate ||
            config_.negativeActivate >= config_.negativeRelease ||
            config_.negativeRelease >= config_.positiveRelease ||
            config_.positiveRelease >= config_.positiveActivate ||
            config_.positiveActivate > config_.qualifiedMaximum ||
            config_.qualifiedMaximum > AnalogInput::maximumReading ||
            config_.dwell.milliseconds () >= halfRange)
        {
            snapshot_.status = StatusCode::InvalidConfiguration;
            return snapshot_.status;
        }

        const Status status = input_.initialize ();

        if (!status.ok ())
        {
            snapshot_.status = status;
            return status;
        }

        initialized_     = true;
        hasUpdate_       = false;
        hasCandidate_    = false;
        snapshot_        = emptyObservation (MagneticSource::LinearAnalog);
        snapshot_.status = StatusCode::NotInitialized;
        return StatusCode::Ok;
    }

    void LinearHall::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            snapshot_.status = StatusCode::NotInitialized;
            return;
        }

        if (hasUpdate_ && !validForwardTime (now, lastUpdate_))
        {
            snapshot_.status = StatusCode::InvalidArgument;
            return;
        }

        snapshot_.activationEvent   = false;
        snapshot_.deactivationEvent = false;

        input_.update ();

        const uint16_t        raw = input_.read ();
        const MagneticQuality quality =
            raw < config_.qualifiedMinimum
                ? MagneticQuality::BelowQualifiedRange
                : (raw > config_.qualifiedMaximum ? MagneticQuality::AboveQualifiedRange
                                                  : MagneticQuality::Valid);

        publish (now, raw, quality);
        lastUpdate_ = now;
        hasUpdate_  = true;
    }

    void LinearHall::shutdown () noexcept
    {
        if (initialized_)
        {
            input_.shutdown ();
            initialized_ = false;
        }

        clearCandidate ();
        hasUpdate_                  = false;
        snapshot_.status            = StatusCode::NotInitialized;
        snapshot_.activationEvent   = false;
        snapshot_.deactivationEvent = false;
    }

    MagneticObservation LinearHall::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool LinearHall::initialized () const noexcept
    {
        return initialized_;
    }

    MagneticPolarity LinearHall::classify (uint16_t raw) const noexcept
    {
        if (snapshot_.polarity == reported (MagneticPolarity::Negative) &&
            raw < config_.negativeRelease)
        {
            return MagneticPolarity::Negative;
        }

        if (snapshot_.polarity == reported (MagneticPolarity::Positive) &&
            raw > config_.positiveRelease)
        {
            return MagneticPolarity::Positive;
        }

        if (raw <= config_.negativeActivate)
        {
            return MagneticPolarity::Negative;
        }

        if (raw >= config_.positiveActivate)
        {
            return MagneticPolarity::Positive;
        }

        return MagneticPolarity::Neutral;
    }

    MagneticPolarity LinearHall::reported (MagneticPolarity value) const noexcept
    {
        if (!config_.reversePolarity)
        {
            return value;
        }

        if (value == MagneticPolarity::Negative)
        {
            return MagneticPolarity::Positive;
        }

        if (value == MagneticPolarity::Positive)
        {
            return MagneticPolarity::Negative;
        }

        return value;
    }

    void LinearHall::clearCandidate () noexcept
    {
        hasCandidate_ = false;
        candidate_    = MagneticPolarity::Neutral;
    }

    void LinearHall::publish (TimePoint now, uint16_t raw,
                              MagneticQuality quality) noexcept
    {
        snapshot_.raw        = raw;
        snapshot_.rawLevel   = Level::Low;
        snapshot_.observedAt = now;
        snapshot_.quality    = quality;
        snapshot_.status     = StatusCode::Ok;

        if (quality != MagneticQuality::Valid)
        {
            clearCandidate ();
            snapshot_.stableFor =
                hasUpdate_ ? now.elapsedSince (stableSince_) : Duration ();
            return;
        }

        MagneticPolarity       next    = classify (raw);
        const MagneticPolarity current = snapshot_.polarity;

        if ((current == reported (MagneticPolarity::Negative) &&
             next == MagneticPolarity::Positive) ||
            (current == reported (MagneticPolarity::Positive) &&
             next == MagneticPolarity::Negative))
        {
            next = MagneticPolarity::Neutral;
        }

        next = reported (next);

        if (!hasUpdate_)
        {
            stableSince_ = now;
        }

        if (next == current)
        {
            clearCandidate ();

            snapshot_.stableFor = now.elapsedSince (stableSince_);
            return;
        }

        if (!hasCandidate_ || candidate_ != next)
        {
            candidate_      = next;
            candidateSince_ = now;
            hasCandidate_   = true;
        }

        if (now.elapsedSince (candidateSince_) >= config_.dwell)
        {
            const bool wasActive = snapshot_.active;

            snapshot_.polarity          = candidate_;
            snapshot_.active            = candidate_ != MagneticPolarity::Neutral;
            snapshot_.activationEvent   = !wasActive && snapshot_.active;
            snapshot_.deactivationEvent = wasActive && !snapshot_.active;
            stableSince_                = now;
            snapshot_.stableFor         = Duration ();

            clearCandidate ();
        }
        else
        {
            snapshot_.stableFor = now.elapsedSince (stableSince_);
        }
    }

    MagneticContact::MagneticContact (ResourceRegistry&            resources,
                                      const MagneticContactConfig& config) noexcept
        : config_          (config)
        , input_           (resources, config.pin, config.pull)
        , snapshot_        (emptyObservation (MagneticSource::ContactDigital))
        , candidateActive_ (false)
        , candidateSince_  ()
        , stableSince_     ()
        , lastUpdate_      ()
        , hasCandidate_    (false)
        , hasUpdate_       (false)
        , initialized_     (false)
    {
    }

    MagneticContact::~MagneticContact () noexcept
    {
        shutdown ();
    }

    Status MagneticContact::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (config_.pin))
        {
            snapshot_.status = StatusCode::InvalidPin;
            return snapshot_.status;
        }

        if (!Mega2560Board::supports (config_.pin, PinCapability::DigitalInput))
        {
            snapshot_.status = StatusCode::Unsupported;
            return snapshot_.status;
        }

        if (!validPull (config_.pull) || !validLevel (config_.closedLevel) ||
            config_.dwell.milliseconds () >= halfRange)
        {
            snapshot_.status = StatusCode::InvalidConfiguration;
            return snapshot_.status;
        }

        const Status status = input_.initialize ();

        if (!status.ok ())
        {
            snapshot_.status = status;
            return status;
        }

        initialized_     = true;
        hasUpdate_       = false;
        hasCandidate_    = false;
        snapshot_        = emptyObservation (MagneticSource::ContactDigital);
        snapshot_.status = StatusCode::NotInitialized;
        return StatusCode::Ok;
    }

    void MagneticContact::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            snapshot_.status = StatusCode::NotInitialized;
            return;
        }

        if (hasUpdate_ && !validForwardTime (now, lastUpdate_))
        {
            snapshot_.status = StatusCode::InvalidArgument;
            return;
        }

        snapshot_.activationEvent   = false;
        snapshot_.deactivationEvent = false;

        input_.update ();

        const Level level  = input_.read ();
        const bool  active = level == config_.closedLevel;

        snapshot_.raw        = level == Level::High ? 1 : 0;
        snapshot_.rawLevel   = level;
        snapshot_.observedAt = now;
        snapshot_.polarity   = MagneticPolarity::Unspecified;
        snapshot_.quality    = MagneticQuality::Valid;
        snapshot_.status     = StatusCode::Ok;

        if (!hasUpdate_)
        {
            stableSince_ = now;
        }

        if (active == snapshot_.active)
        {
            clearCandidate ();

            snapshot_.stableFor = now.elapsedSince (stableSince_);
        }
        else
        {
            if (!hasCandidate_ || candidateActive_ != active)
            {
                candidateActive_ = active;
                candidateSince_  = now;
                hasCandidate_    = true;
            }

            if (now.elapsedSince (candidateSince_) >= config_.dwell)
            {
                snapshot_.active            = candidateActive_;
                snapshot_.activationEvent   = snapshot_.active;
                snapshot_.deactivationEvent = !snapshot_.active;
                stableSince_                = now;
                snapshot_.stableFor         = Duration ();

                clearCandidate ();
            }
            else
            {
                snapshot_.stableFor = now.elapsedSince (stableSince_);
            }
        }

        lastUpdate_ = now;
        hasUpdate_  = true;
    }

    void MagneticContact::shutdown () noexcept
    {
        if (initialized_)
        {
            input_.shutdown ();

            initialized_ = false;
        }

        clearCandidate ();
        hasUpdate_                  = false;
        snapshot_.status            = StatusCode::NotInitialized;
        snapshot_.activationEvent   = false;
        snapshot_.deactivationEvent = false;
    }

    MagneticObservation MagneticContact::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool MagneticContact::initialized () const noexcept
    {
        return initialized_;
    }

    void MagneticContact::clearCandidate () noexcept
    {
        hasCandidate_    = false;
        candidateActive_ = false;
    }
} // namespace adk
