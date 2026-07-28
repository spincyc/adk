#include "contact_dynamics.h"

#include <limits.h>

// clang-format off
namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        bool validLevel (Level level) noexcept
        {
            return level == Level::Low || level == Level::High;
        }

        bool validDuration (Duration duration) noexcept
        {
            return duration.milliseconds () != 0 &&
                   duration.milliseconds () < halfRange;
        }

        bool sameSample (const ContactSample& left, const ContactSample& right) noexcept
        {
            return left.observedAt == right.observedAt &&
                   left.rawLevel == right.rawLevel && left.status == right.status;
        }

        uint32_t incrementSaturating (uint32_t value) noexcept
        {
            return value == UINT32_MAX ? value : value + 1U;
        }

        ContactObservation emptyObservation () noexcept
        {
            return {TimePoint (),
                    Level::Low,
                    false,
                    false,
                    false,
                    false,
                    Duration (),
                    Duration (),
                    0,
                    0,
                    ContactDisposition::None,
                    ContactQuality::Unqualified,
                    StatusCode::NotInitialized};
        }
    } // namespace

    ContactDynamicsConfig::ContactDynamicsConfig (Level    activeLevelValue,
                                                  Duration qualifyValue,
                                                  Duration releaseValue,
                                                  Duration refractoryValue,
                                                  Duration stuckActiveValue) noexcept
        : activeLevel (activeLevelValue), qualify (qualifyValue),
          release     (releaseValue), refractory (refractoryValue),
          stuckActive (stuckActiveValue)
    {
    }

    ContactDynamics::ContactDynamics (const ContactDynamicsConfig& config) noexcept
        : config_ (config), observation_ (emptyObservation ()), lastSample_ (),
          sourceFaultStatus_ (StatusCode::Ok),
          candidateSince_    (), acceptedAt_ (), releaseSince_ (), lastUpdate_ (),
          initialized_       (false), hasLastSample_ (false),
          candidateActive_   (false), releaseCandidate_ (false),
          acceptedPulseOpen_ (false), stuckActive_ (false), sourceFaulted_ (false)
    {
    }

    Status ContactDynamics::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validLevel (config_.activeLevel) || !validDuration (config_.qualify) ||
            !validDuration (config_.release) || !validDuration (config_.refractory) ||
            !validDuration (config_.stuckActive))
        {
            observation_.status = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void ContactDynamics::reset () noexcept
    {
        observation_         = emptyObservation ();
        lastSample_          = ContactSample    ();
        sourceFaultStatus_   = StatusCode::Ok;
        candidateSince_      = TimePoint        ();
        acceptedAt_          = TimePoint        ();
        releaseSince_        = TimePoint        ();
        lastUpdate_          = TimePoint        ();
        hasLastSample_       = false;
        candidateActive_     = false;
        releaseCandidate_    = false;
        acceptedPulseOpen_   = false;
        stuckActive_         = false;
        sourceFaulted_       = false;

        if (initialized_)
        {
            observation_.status = StatusCode::Ok;
        }
    }

    Status ContactDynamics::update (const ContactSample& sample) noexcept
    {
        if (!initialized_)
        {
            observation_.status = StatusCode::NotInitialized;
            return observation_.status;
        }

        if (hasLastSample_)
        {
            const Duration elapsed = sample.observedAt.elapsedSince (lastUpdate_);

            if (elapsed.milliseconds () >= halfRange)
            {
                publishTimingFault (sample);
                return observation_.status;
            }

            if (elapsed == Duration ())
            {
                if (sameSample (sample, lastSample_))
                {
                    return observation_.status;
                }

                publishTimingFault (sample);
                return observation_.status;
            }
        }

        if (!validLevel (sample.rawLevel))
        {
            observation_.attackEvent  = false;
            observation_.releaseEvent = false;
            observation_.disposition  = ContactDisposition::None;
            observation_.observedAt   = sample.observedAt;
            observation_.quality      = ContactQuality::SourceFault;
            observation_.status       = StatusCode::InvalidArgument;
            sourceFaultStatus_        = observation_.status;
            sourceFaulted_            = true;
            hasLastSample_            = true;
            lastSample_               = sample;
            lastUpdate_               = sample.observedAt;
            return observation_.status;
        }

        observation_.attackEvent  = false;
        observation_.releaseEvent = false;
        observation_.disposition  = ContactDisposition::None;
        observation_.observedAt   = sample.observedAt;
        observation_.rawLevel     = sample.rawLevel;
        observation_.rawActive    = sample.rawLevel == config_.activeLevel;

        if (sourceFaulted_)
        {
            observation_.quality = ContactQuality::SourceFault;
            observation_.status  = sourceFaultStatus_;
            hasLastSample_       = true;
            lastSample_          = sample;
            lastUpdate_          = sample.observedAt;
            return observation_.status;
        }

        if (!sample.status.ok ())
        {
            observation_.quality = ContactQuality::SourceFault;
            observation_.status  = sample.status;
            sourceFaultStatus_   = sample.status;
            sourceFaulted_       = true;
            hasLastSample_       = true;
            lastSample_          = sample;
            lastUpdate_          = sample.observedAt;
            return observation_.status;
        }

        hasLastSample_       = true;
        lastSample_          = sample;
        lastUpdate_          = sample.observedAt;
        observation_.quality = stuckActive_ ? ContactQuality::StuckActive
                                             : ContactQuality::Valid;
        observation_.status  = StatusCode::Ok;

        if (!observation_.qualifiedActive)
        {
            releaseCandidate_ = false;

            if (!observation_.rawActive)
            {
                candidateActive_ = false;
            }
            else if (!candidateActive_)
            {
                candidateActive_ = true;
                candidateSince_  = sample.observedAt;
            }
        }
        else
        {
            candidateActive_ = false;

            if (observation_.rawActive)
            {
                releaseCandidate_ = false;
            }
            else if (!releaseCandidate_)
            {
                releaseCandidate_ = true;
                releaseSince_     = sample.observedAt;
            }
        }

        updateRefractory (sample.observedAt);

        if (observation_.qualifiedActive && releaseCandidate_ &&
            sample.observedAt.elapsedSince (releaseSince_) >= config_.release)
        {
            observation_.qualifiedActive = false;
            observation_.releaseEvent    = true;
            releaseCandidate_            = false;
            stuckActive_                 = false;
            observation_.quality         = ContactQuality::Valid;

            if (acceptedPulseOpen_)
            {
                observation_.qualifiedPulseWidth =
                    sample.observedAt.elapsedSince (acceptedAt_);
                acceptedPulseOpen_ = false;
            }

            return observation_.status;
        }

        if (observation_.qualifiedActive && acceptedPulseOpen_ &&
            sample.observedAt.elapsedSince (acceptedAt_) >= config_.stuckActive)
        {
            observation_.quality = ContactQuality::StuckActive;
            stuckActive_         = true;
            return observation_.status;
        }

        if (!observation_.qualifiedActive && candidateActive_ &&
            sample.observedAt.elapsedSince (candidateSince_) >= config_.qualify)
        {
            observation_.qualifiedActive = true;
            observation_.attackEvent     = true;
            candidateActive_             = false;

            if (observation_.refractoryRemaining != Duration ())
            {
                observation_.suppressedCount =
                    incrementSaturating (observation_.suppressedCount);
                observation_.disposition =
                    ContactDisposition::SuppressedDuringRefractory;
                return observation_.status;
            }

            acceptedAt_        = sample.observedAt;
            acceptedPulseOpen_ = true;
            observation_.acceptedCount =
                incrementSaturating (observation_.acceptedCount);
            observation_.disposition = ContactDisposition::Accepted;
            updateRefractory (sample.observedAt);
        }

        return observation_.status;
    }

    bool ContactDynamics::initialized () const noexcept
    {
        return initialized_;
    }

    ContactObservation ContactDynamics::snapshot () const noexcept
    {
        return observation_;
    }

    void ContactDynamics::publishTimingFault (const ContactSample& sample) noexcept
    {
        observation_.attackEvent  = false;
        observation_.releaseEvent = false;
        observation_.disposition  = ContactDisposition::None;
        observation_.observedAt   = sample.observedAt;
        observation_.rawLevel     = sample.rawLevel;
        observation_.rawActive    = sample.rawLevel == config_.activeLevel;
        observation_.quality      = ContactQuality::TimingFault;
        observation_.status       = StatusCode::InvalidArgument;
    }

    void ContactDynamics::updateRefractory (TimePoint now) noexcept
    {
        if (observation_.acceptedCount == 0)
        {
            observation_.refractoryRemaining = Duration ();
            return;
        }

        const Duration elapsed = now.elapsedSince (acceptedAt_);

        if (elapsed >= config_.refractory)
        {
            observation_.refractoryRemaining = Duration ();
            return;
        }

        observation_.refractoryRemaining =
            Duration (config_.refractory.milliseconds () - elapsed.milliseconds ());
    }
} // namespace adk
// clang-format on
