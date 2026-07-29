#include "resistive_probe_observation.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        ResistiveProbeObservation emptyObservation () noexcept
        {
            return {{0, 0, 0, 0, TimePoint (), 0, 0, Duration (), Duration (), false,
                     StatusCode::NotInitialized},
                    0,
                    0,
                    ProbeQuality::Unqualified,
                    Duration (),
                    StatusCode::NotInitialized};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool sameSample (const ResistiveProbeSample& left,
                         const ResistiveProbeSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt &&
                   left.energizedRaw == right.energizedRaw &&
                   left.dischargedRaw == right.dischargedRaw &&
                   left.excitationOnTime == right.excitationOnTime &&
                   left.cycleTime == right.cycleTime &&
                   left.excitationObservedOffAfterSample ==
                       right.excitationObservedOffAfterSample &&
                   left.status == right.status;
        }

        bool sameDomain (const ResistiveProbeSample& left,
                         const ResistiveProbeSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision;
        }

        uint16_t normalizedPermille (uint16_t                    raw,
                                     const ResistiveProbeConfig& config) noexcept
        {
            const uint16_t dry = config.dryReference;
            const uint16_t wet = config.wetReference;

            if (wet > dry)
            {
                if (raw <= dry)
                {
                    return 0;
                }

                if (raw >= wet)
                {
                    return 1000;
                }

                return static_cast<uint16_t> (
                    (static_cast<uint32_t> (raw - dry) * 1000UL) /
                    static_cast<uint32_t> (wet - dry));
            }

            if (raw >= dry)
            {
                return 0;
            }

            if (raw <= wet)
            {
                return 1000;
            }

            return static_cast<uint16_t> ((static_cast<uint32_t> (dry - raw) * 1000UL) /
                                          static_cast<uint32_t> (dry - wet));
        }

        ProbeQuality classify (uint16_t                    normalized,
                               const ResistiveProbeConfig& config) noexcept
        {
            if (normalized < config.dampThresholdPermille)
            {
                return ProbeQuality::Dry;
            }

            if (normalized < config.wetThresholdPermille)
            {
                return ProbeQuality::Damp;
            }

            return ProbeQuality::Wet;
        }
    } // namespace

    ResistiveProbeObservationPolicy::ResistiveProbeObservationPolicy (
        const ResistiveProbeConfig& config) noexcept
        : config_ (config), observation_ (emptyObservation ()), lastUpdateAt_ (),
          initialized_       (false), hasSample_ (false), hasUpdate_ (false),
          sequenceExhausted_ (false)
    {
    }

    Status ResistiveProbeObservationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const uint32_t maximumAge = config_.maximumAge.milliseconds ();
        const uint32_t maximumExcitationOnTime =
            config_.maximumExcitationOnTime.milliseconds ();

        if (config_.adcMaximum == 0 || config_.dryReference > config_.adcMaximum ||
            config_.wetReference > config_.adcMaximum ||
            config_.dryReference == config_.wetReference ||
            config_.disconnectedMaximum > config_.adcMaximum ||
            config_.dischargedMaximum > config_.adcMaximum ||
            config_.dampThresholdPermille > config_.wetThresholdPermille ||
            config_.wetThresholdPermille > 1000 || maximumAge == 0 ||
            maximumAge >= halfRange || maximumExcitationOnTime == 0 ||
            maximumExcitationOnTime >= halfRange || config_.maximumDutyPermille > 1000)
        {
            observation_.status = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void ResistiveProbeObservationPolicy::reset () noexcept
    {
        observation_ = emptyObservation ();

        lastUpdateAt_      = TimePoint ();
        hasSample_         = false;
        hasUpdate_         = false;
        sequenceExhausted_ = false;
    }

    Status ResistiveProbeObservationPolicy::update (
        TimePoint now, const ResistiveProbeSample& sample) noexcept
    {
        if (!initialized_)
        {
            observation_.quality = ProbeQuality::Unqualified;
            observation_.status  = StatusCode::NotInitialized;
            return observation_.status;
        }

        const uint32_t age = now.elapsedSince (sample.observedAt).milliseconds ();
        const bool     repeatedUpdateTime =
            hasUpdate_ && now == lastUpdateAt_ &&
            (!hasSample_ || !sameSample (sample, observation_.sample));

        if (age >= halfRange ||
            (hasUpdate_ &&
             now.elapsedSince (lastUpdateAt_).milliseconds () >= halfRange) ||
            repeatedUpdateTime || !validStatus (sample.status) ||
            sample.sourceId == 0 || sample.configurationRevision == 0 ||
            sample.calibrationRevision == 0 || sample.sequence == 0 ||
            sample.energizedRaw > config_.adcMaximum ||
            sample.dischargedRaw > config_.adcMaximum ||
            sample.cycleTime.milliseconds () == 0 ||
            sample.cycleTime.milliseconds () >= halfRange ||
            sample.excitationOnTime > sample.cycleTime)
        {
            return StatusCode::InvalidArgument;
        }

        if (hasSample_)
        {
            if (sequenceExhausted_)
            {
                return StatusCode::CapacityExceeded;
            }

            if (!sameDomain (sample, observation_.sample))
            {
                return StatusCode::InvalidArgument;
            }

            const uint32_t sequenceDelta =
                sample.sequence - observation_.sample.sequence;

            if (sequenceDelta == 0)
            {
                if (!sameSample (sample, observation_.sample))
                {
                    return StatusCode::InvalidArgument;
                }
            }
            else if (sequenceDelta >= halfRange ||
                     sample.observedAt == observation_.sample.observedAt ||
                     sample.observedAt.elapsedSince (observation_.sample.observedAt)
                             .milliseconds () >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
        }

        const uint16_t duty = static_cast<uint16_t> (
            (static_cast<uint64_t> (sample.excitationOnTime.milliseconds ()) *
             1000ULL) /
            sample.cycleTime.milliseconds ());
        const uint16_t normalized = normalizedPermille (sample.energizedRaw, config_);

        observation_.sample                    = sample;
        observation_.normalizedPermille        = normalized;
        observation_.observedCycleDutyPermille = duty;
        observation_.age                       = Duration (age);
        observation_.status                    = StatusCode::Ok;

        if (!sample.status.ok ())
        {
            observation_.quality = ProbeQuality::ProducerFault;
            observation_.status  = sample.status;
        }
        else if (!sample.excitationObservedOffAfterSample ||
                 sample.dischargedRaw > config_.dischargedMaximum ||
                 sample.excitationOnTime > config_.maximumExcitationOnTime ||
                 duty > config_.maximumDutyPermille)
        {
            observation_.quality = ProbeQuality::ExcitationFault;
        }
        else if (sample.energizedRaw == config_.adcMaximum)
        {
            observation_.quality = ProbeQuality::Saturated;
        }
        else if (sample.energizedRaw <= config_.disconnectedMaximum &&
                 sample.dischargedRaw <= config_.disconnectedMaximum)
        {
            observation_.quality = ProbeQuality::Disconnected;
        }
        else if (age > config_.maximumAge.milliseconds ())
        {
            observation_.quality = ProbeQuality::Stale;
        }
        else
        {
            observation_.quality = classify (normalized, config_);
        }

        hasSample_         = true;
        lastUpdateAt_      = now;
        hasUpdate_         = true;
        sequenceExhausted_ = sample.sequence == UINT32_MAX;
        return observation_.status;
    }

    ResistiveProbeObservation
    ResistiveProbeObservationPolicy::snapshot () const noexcept
    {
        return observation_;
    }

    bool ResistiveProbeObservationPolicy::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
