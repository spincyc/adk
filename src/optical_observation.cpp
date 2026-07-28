#include "optical_observation.h"

namespace adk {
    namespace {
        constexpr uint16_t adcMaximum      = 1023;
        constexpr uint16_t permilleMaximum = 1000;
        constexpr uint32_t halfRange       = 0x80000000UL;

        bool validLevel (Level level) noexcept
        {
            return level == Level::Low || level == Level::High;
        }

        bool sameProvenance (const OpticalProvenance& left,
                             const OpticalProvenance& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.observedAt == right.observedAt;
        }

        bool sameSample (const ReflectiveSample& left,
                         const ReflectiveSample& right) noexcept
        {
            return sameProvenance (left.provenance, right.provenance) &&
                   left.raw == right.raw && left.status == right.status;
        }

        bool sameSample (const BeamSample& left, const BeamSample& right) noexcept
        {
            return sameProvenance (left.provenance, right.provenance) &&
                   left.rawLevel == right.rawLevel && left.status == right.status;
        }

        OpticalProvenance emptyProvenance () noexcept
        {
            return {0, 0, TimePoint ()};
        }

        ReflectiveObservation emptyReflectiveObservation () noexcept
        {
            return {emptyProvenance (),
                    0,
                    0,
                    0,
                    0,
                    false,
                    false,
                    false,
                    Duration (),
                    OpticalQuality::Unqualified,
                    StatusCode::NotInitialized};
        }

        BeamObservation emptyBeamObservation () noexcept
        {
            return {emptyProvenance (),
                    Level::Low,
                    false,
                    false,
                    false,
                    Duration (),
                    OpticalQuality::Unqualified,
                    StatusCode::NotInitialized};
        }

        uint16_t normalize (uint16_t raw, uint16_t dark, uint16_t light) noexcept
        {
            if (light > dark)
            {
                if (raw <= dark)
                {
                    return 0;
                }

                if (raw >= light)
                {
                    return permilleMaximum;
                }

                return static_cast<uint16_t> (
                    (static_cast<uint32_t> (raw - dark) * permilleMaximum) /
                    (light - dark));
            }

            if (raw >= dark)
            {
                return 0;
            }

            if (raw <= light)
            {
                return permilleMaximum;
            }

            return static_cast<uint16_t> (
                (static_cast<uint32_t> (dark - raw) * permilleMaximum) /
                (dark - light));
        }

        Duration stableDuration (TimePoint now, TimePoint since) noexcept
        {
            const uint32_t elapsed = now.elapsedSince (since).milliseconds ();

            return Duration (elapsed >= halfRange ? halfRange - 1U : elapsed);
        }
    } // namespace

    ReflectiveObservationPolicy::ReflectiveObservationPolicy (
        const ReflectiveObservationConfig& config) noexcept
        : config_          (config),
          observation_     (emptyReflectiveObservation ()),
          lastSample_      (),
          candidateSince_  (),
          stableSince_     (),
          initialized_     (false),
          hasSample_       (false),
          hasCandidate_    (false),
          candidateActive_ (false),
          faulted_         (false)
    {
    }

    Status ReflectiveObservationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const bool validThresholds =
            config_.activatePermille <= permilleMaximum &&
            config_.releasePermille <= permilleMaximum &&
            (config_.darkerIsActive
                 ? config_.activatePermille < config_.releasePermille
                 : config_.activatePermille > config_.releasePermille);

        if (config_.qualifiedMinimum > config_.qualifiedMaximum ||
            config_.qualifiedMaximum > adcMaximum ||
            config_.darkReference < config_.qualifiedMinimum ||
            config_.darkReference > config_.qualifiedMaximum ||
            config_.lightReference < config_.qualifiedMinimum ||
            config_.lightReference > config_.qualifiedMaximum ||
            config_.dwell.milliseconds () >= halfRange || !validThresholds)
        {
            observation_.status = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        if (config_.darkReference == config_.lightReference)
        {
            observation_.quality = OpticalQuality::DegenerateCalibration;
            observation_.status  = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void ReflectiveObservationPolicy::reset () noexcept
    {
        observation_     = emptyReflectiveObservation ();
        lastSample_      = ReflectiveSample           ();
        candidateSince_  = TimePoint                  ();
        stableSince_     = TimePoint                  ();
        hasSample_       = false;
        hasCandidate_    = false;
        candidateActive_ = false;
        faulted_         = false;

        if (initialized_)
        {
            observation_.darkReference  = config_.darkReference;
            observation_.lightReference = config_.lightReference;
            observation_.status         = StatusCode::Ok;
        }
    }

    Status ReflectiveObservationPolicy::update (const ReflectiveSample& sample) noexcept
    {
        if (!initialized_)
        {
            observation_.status = StatusCode::NotInitialized;
            return observation_.status;
        }

        if (faulted_)
        {
            return observation_.status;
        }

        if (hasSample_)
        {
            const Duration elapsed = sample.provenance.observedAt.elapsedSince (
                lastSample_.provenance.observedAt);

            if (elapsed.milliseconds () == 0)
            {
                if (sameSample (sample, lastSample_))
                {
                    return observation_.status;
                }

                observation_.quality = OpticalQuality::TimingFault;
                observation_.status  = StatusCode::InvalidArgument;
                faulted_             = true;
                return observation_.status;
            }

            if (elapsed.milliseconds () >= halfRange)
            {
                observation_.quality = OpticalQuality::TimingFault;
                observation_.status  = StatusCode::InvalidArgument;
                faulted_             = true;
                return observation_.status;
            }
        }

        if (!hasSample_)
        {
            stableSince_ = sample.provenance.observedAt;
        }

        observation_.activationEvent   = false;
        observation_.deactivationEvent = false;
        observation_.provenance        = sample.provenance;
        observation_.raw               = sample.raw;

        if (sample.provenance.sourceId != config_.sourceId ||
            sample.provenance.calibrationRevision != config_.calibrationRevision ||
            sample.raw > adcMaximum)
        {
            hasCandidate_        = false;
            observation_.quality = OpticalQuality::SourceFault;
            observation_.status  = StatusCode::InvalidArgument;
            faulted_             = true;
        }
        else if (!sample.status.ok ())
        {
            hasCandidate_        = false;
            observation_.quality = OpticalQuality::SourceFault;
            observation_.status  = sample.status;
            faulted_             = true;
        }
        else
        {
            observation_.normalizedPermille =
                normalize (sample.raw, config_.darkReference, config_.lightReference);
            observation_.status = StatusCode::Ok;

            if (sample.raw < config_.qualifiedMinimum)
            {
                hasCandidate_        = false;
                observation_.quality = OpticalQuality::BelowQualifiedRange;
            }
            else if (sample.raw > config_.qualifiedMaximum)
            {
                hasCandidate_        = false;
                observation_.quality = OpticalQuality::AboveQualifiedRange;
            }
            else
            {
                observation_.quality = OpticalQuality::Valid;

                bool nextActive = observation_.markerActive;

                if (config_.darkerIsActive)
                {
                    nextActive =
                        observation_.markerActive
                            ? observation_.normalizedPermille < config_.releasePermille
                            : observation_.normalizedPermille <=
                                  config_.activatePermille;
                }
                else
                {
                    nextActive =
                        observation_.markerActive
                            ? observation_.normalizedPermille > config_.releasePermille
                            : observation_.normalizedPermille >=
                                  config_.activatePermille;
                }

                if (nextActive == observation_.markerActive)
                {
                    hasCandidate_ = false;
                }
                else
                {
                    if (!hasCandidate_ || candidateActive_ != nextActive)
                    {
                        hasCandidate_    = true;
                        candidateActive_ = nextActive;
                        candidateSince_  = sample.provenance.observedAt;
                    }

                    if (sample.provenance.observedAt.elapsedSince (candidateSince_) >=
                        config_.dwell)
                    {
                        observation_.markerActive      = candidateActive_;
                        observation_.activationEvent   = candidateActive_;
                        observation_.deactivationEvent = !candidateActive_;
                        stableSince_                   = sample.provenance.observedAt;
                        hasCandidate_                  = false;
                    }
                }
            }
        }

        observation_.darkReference  = config_.darkReference;
        observation_.lightReference = config_.lightReference;
        observation_.stableFor =
            hasSample_ ? stableDuration (sample.provenance.observedAt, stableSince_)
                       : Duration ();
        lastSample_ = sample;
        hasSample_  = true;
        return observation_.status;
    }

    ReflectiveObservation ReflectiveObservationPolicy::snapshot () const noexcept
    {
        return observation_;
    }

    bool ReflectiveObservationPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    BeamObservationPolicy::BeamObservationPolicy (
        const BeamObservationConfig& config) noexcept
        : config_               (config),
          observation_          (emptyBeamObservation ()),
          lastSample_           (),
          candidateSince_       (),
          stableSince_          (),
          initialized_          (false),
          hasSample_            (false),
          hasCandidate_         (false),
          candidateInterrupted_ (false),
          faulted_              (false)
    {
    }

    Status BeamObservationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validLevel                            (config_.interruptedLevel) ||
            config_.interruptDwell.milliseconds () >= halfRange ||
            config_.restoreDwell.milliseconds   () >= halfRange)
        {
            observation_.status = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void BeamObservationPolicy::reset () noexcept
    {
        observation_          = emptyBeamObservation ();
        lastSample_           = BeamSample           ();
        candidateSince_       = TimePoint            ();
        stableSince_          = TimePoint            ();
        hasSample_            = false;
        hasCandidate_         = false;
        candidateInterrupted_ = false;
        faulted_              = false;

        if (initialized_)
        {
            observation_.status = StatusCode::Ok;
        }
    }

    Status BeamObservationPolicy::update (const BeamSample& sample) noexcept
    {
        if (!initialized_)
        {
            observation_.status = StatusCode::NotInitialized;
            return observation_.status;
        }

        if (faulted_)
        {
            return observation_.status;
        }

        if (hasSample_)
        {
            const Duration elapsed = sample.provenance.observedAt.elapsedSince (
                lastSample_.provenance.observedAt);

            if (elapsed.milliseconds () == 0)
            {
                if (sameSample (sample, lastSample_))
                {
                    return observation_.status;
                }

                observation_.quality = OpticalQuality::TimingFault;
                observation_.status  = StatusCode::InvalidArgument;
                faulted_             = true;
                return observation_.status;
            }

            if (elapsed.milliseconds () >= halfRange)
            {
                observation_.quality = OpticalQuality::TimingFault;
                observation_.status  = StatusCode::InvalidArgument;
                faulted_             = true;
                return observation_.status;
            }
        }

        if (!hasSample_)
        {
            stableSince_ = sample.provenance.observedAt;
        }

        observation_.interruptionEvent = false;
        observation_.restorationEvent  = false;
        observation_.provenance        = sample.provenance;
        observation_.rawLevel          = sample.rawLevel;

        if (sample.provenance.sourceId != config_.sourceId ||
            sample.provenance.calibrationRevision != config_.calibrationRevision ||
            !validLevel (sample.rawLevel))
        {
            hasCandidate_        = false;
            observation_.quality = OpticalQuality::SourceFault;
            observation_.status  = StatusCode::InvalidArgument;
            faulted_             = true;
        }
        else if (!sample.status.ok ())
        {
            hasCandidate_        = false;
            observation_.quality = OpticalQuality::SourceFault;
            observation_.status  = sample.status;
            faulted_             = true;
        }
        else
        {
            const bool interrupted = sample.rawLevel == config_.interruptedLevel;

            observation_.quality = OpticalQuality::Valid;
            observation_.status  = StatusCode::Ok;

            if (interrupted == observation_.interrupted)
            {
                hasCandidate_ = false;
            }
            else
            {
                if (!hasCandidate_ || candidateInterrupted_ != interrupted)
                {
                    hasCandidate_         = true;
                    candidateInterrupted_ = interrupted;
                    candidateSince_       = sample.provenance.observedAt;
                }

                const Duration dwell = candidateInterrupted_ ? config_.interruptDwell
                                                             : config_.restoreDwell;

                if (sample.provenance.observedAt.elapsedSince (candidateSince_) >=
                    dwell)
                {
                    observation_.interrupted       = candidateInterrupted_;
                    observation_.interruptionEvent = candidateInterrupted_;
                    observation_.restorationEvent  = !candidateInterrupted_;
                    stableSince_                   = sample.provenance.observedAt;
                    hasCandidate_                  = false;
                }
            }
        }

        observation_.stableFor =
            hasSample_ ? stableDuration (sample.provenance.observedAt, stableSince_)
                       : Duration ();
        lastSample_ = sample;
        hasSample_  = true;
        return observation_.status;
    }

    BeamObservation BeamObservationPolicy::snapshot () const noexcept
    {
        return observation_;
    }

    bool BeamObservationPolicy::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
