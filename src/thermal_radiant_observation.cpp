#include "thermal_radiant_observation.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        ThermalRadiantEnvelope emptyEnvelope () noexcept
        {
            const ConvertedThermalSample thermistor = {
                0, 0, 0, 0, TimePoint (), 0, 0, false, StatusCode::NotInitialized};
            const CategoricalThresholdSample categorical = {0,
                                                            0,
                                                            0,
                                                            0,
                                                            TimePoint (),
                                                            0,
                                                            ThresholdState::Below,
                                                            false,
                                                            StatusCode::NotInitialized};
            return {thermistor, categorical, categorical};
        }

        ThermalRadiantObservation emptyThermalRadiantObservation () noexcept
        {
            return {emptyEnvelope (),
                    ThermalQuality::Unqualified,
                    RadiantQuality::Unqualified,
                    Duration (),
                    Duration (),
                    Duration (),
                    false,
                    false,
                    StatusCode::NotInitialized};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validState (ThresholdState state) noexcept
        {
            return state == ThresholdState::Below || state == ThresholdState::AtOrAbove;
        }

        bool sameSample (const ConvertedThermalSample& left,
                         const ConvertedThermalSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt &&
                   left.milliCelsius == right.milliCelsius &&
                   left.uncertaintyMilliCelsius == right.uncertaintyMilliCelsius &&
                   left.saturated == right.saturated && left.status == right.status;
        }

        bool sameSample (const CategoricalThresholdSample& left,
                         const CategoricalThresholdSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt && left.raw == right.raw &&
                   left.state == right.state && left.saturated == right.saturated &&
                   left.status == right.status;
        }

        bool sameEnvelope (const ThermalRadiantEnvelope& left,
                           const ThermalRadiantEnvelope& right) noexcept
        {
            return sameSample (left.thermistor, right.thermistor) &&
                   sameSample (left.digitalTemperature, right.digitalTemperature) &&
                   sameSample (left.radiant, right.radiant);
        }

        bool sameDomain (const ConvertedThermalSample& left,
                         const ConvertedThermalSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision;
        }

        bool sameDomain (const CategoricalThresholdSample& left,
                         const CategoricalThresholdSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision;
        }

        template <typename Sample>
        bool validOrder (const Sample& sample, const Sample& accepted,
                         bool exhausted) noexcept
        {
            if (!sameDomain (sample, accepted))
            {
                return false;
            }

            const uint32_t sequenceDelta = sample.sequence - accepted.sequence;
            if (sequenceDelta == 0)
            {
                return sameSample (sample, accepted);
            }

            return !exhausted && sequenceDelta < halfRange &&
                   sample.observedAt != accepted.observedAt &&
                   sample.observedAt.elapsedSince (accepted.observedAt)
                           .milliseconds () < halfRange;
        }

        ThermalQuality classifyThermistor (const ConvertedThermalSample& sample,
                                           const ThermalRadiantConfig& config) noexcept
        {
            const int64_t upper = static_cast<int64_t> (sample.milliCelsius) +
                                  sample.uncertaintyMilliCelsius;

            if (upper >= config.alarmMilliCelsius)
            {
                return ThermalQuality::Alarm;
            }

            if (upper >= config.warningMilliCelsius)
            {
                return ThermalQuality::Warning;
            }

            return ThermalQuality::Normal;
        }
    } // namespace

    ThermalRadiantObservationPolicy::ThermalRadiantObservationPolicy (
        const ThermalRadiantConfig& config) noexcept
        : config_                      (config),
          observation_                 (emptyThermalRadiantObservation ()),
          lastUpdateAt_                (),
          radiantActiveSince_          (),
          initialized_                 (false),
          hasEnvelope_                 (false),
          hasUpdate_                   (false),
          radiantCandidateActive_      (false),
          radiantCandidateSustained_   (false),
          thermistorSequenceExhausted_ (false),
          digitalSequenceExhausted_    (false),
          radiantSequenceExhausted_    (false)
    {
    }

    Status ThermalRadiantObservationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const uint32_t maximumAge = config_.maximumAge              .milliseconds ();
        const uint32_t pulse      = config_.radiantPulseMaximum     .milliseconds ();
        const uint32_t sustained  = config_.radiantSustainedMinimum .milliseconds ();

        if (config_.warningMilliCelsius >= config_.alarmMilliCelsius ||
            maximumAge == 0 || maximumAge >= halfRange || pulse == 0 ||
            pulse >= sustained || sustained >= halfRange)
        {
            observation_.status = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void ThermalRadiantObservationPolicy::reset () noexcept
    {
        observation_                 = emptyThermalRadiantObservation ();
        lastUpdateAt_                = TimePoint                      ();
        radiantActiveSince_          = TimePoint                      ();
        hasEnvelope_                 = false;
        hasUpdate_                   = false;
        radiantCandidateActive_      = false;
        radiantCandidateSustained_   = false;
        thermistorSequenceExhausted_ = false;
        digitalSequenceExhausted_    = false;
        radiantSequenceExhausted_    = false;
    }

    Status ThermalRadiantObservationPolicy::update (
        TimePoint now, const ThermalRadiantEnvelope& envelope) noexcept
    {
        if (!initialized_)
        {
            observation_.thermalQuality = ThermalQuality::Unqualified;
            observation_.radiantQuality = RadiantQuality::Unqualified;
            observation_.status         = StatusCode::NotInitialized;
            return observation_.status;
        }

        const uint32_t thermistorAge =
            now.elapsedSince (envelope.thermistor.observedAt).milliseconds ();
        const uint32_t digitalAge =
            now.elapsedSince (envelope.digitalTemperature.observedAt).milliseconds ();
        const uint32_t radiantAge =
            now.elapsedSince (envelope.radiant.observedAt).milliseconds ();
        if (thermistorAge >= halfRange || digitalAge >= halfRange ||
            radiantAge >= halfRange ||
            (hasUpdate_ &&
             now.elapsedSince (lastUpdateAt_).milliseconds () >= halfRange) ||
            !validStatus (envelope.thermistor.status) ||
            !validStatus (envelope.digitalTemperature.status) ||
            !validStatus (envelope.radiant.status) ||
            !validState  (envelope.digitalTemperature.state) ||
            !validState  (envelope.radiant.state) ||
            envelope.thermistor.sourceId == 0 ||
            envelope.digitalTemperature.sourceId == 0 ||
            envelope.radiant.sourceId == 0 ||
            envelope.thermistor.sourceId == envelope.digitalTemperature.sourceId ||
            envelope.thermistor.sourceId == envelope.radiant.sourceId ||
            envelope.digitalTemperature.sourceId == envelope.radiant.sourceId ||
            envelope.thermistor.configurationRevision == 0 ||
            envelope.digitalTemperature.configurationRevision == 0 ||
            envelope.radiant.configurationRevision == 0 ||
            envelope.thermistor.calibrationRevision == 0 ||
            envelope.digitalTemperature.calibrationRevision == 0 ||
            envelope.radiant.calibrationRevision == 0 ||
            envelope.thermistor.sequence == 0 ||
            envelope.digitalTemperature.sequence == 0 || envelope.radiant.sequence == 0)
        {
            return StatusCode::InvalidArgument;
        }

        const bool duplicate =
            hasEnvelope_ && sameEnvelope (envelope, observation_.envelope);
        if (duplicate)
        {
            return observation_.status;
        }

        if (hasEnvelope_ &&
            (!validOrder (envelope.thermistor, observation_.envelope.thermistor,
                          thermistorSequenceExhausted_) ||
             !validOrder (envelope.digitalTemperature,
                          observation_.envelope.digitalTemperature,
                          digitalSequenceExhausted_) ||
             !validOrder (envelope.radiant, observation_.envelope.radiant,
                          radiantSequenceExhausted_)))
        {
            const bool exhaustedChange =
                (thermistorSequenceExhausted_ &&
                 !sameSample (envelope.thermistor, observation_.envelope.thermistor)) ||
                (digitalSequenceExhausted_ &&
                 !sameSample (envelope.digitalTemperature,
                              observation_.envelope.digitalTemperature)) ||
                (radiantSequenceExhausted_ &&
                 !sameSample (envelope.radiant, observation_.envelope.radiant));
            return exhaustedChange ? StatusCode::CapacityExceeded
                                   : StatusCode::InvalidArgument;
        }

        if (hasUpdate_ && now == lastUpdateAt_)
        {
            return StatusCode::InvalidArgument;
        }

        ThermalRadiantObservation next               = observation_;
        bool                      candidateActive    = radiantCandidateActive_;
        bool                      candidateSustained = radiantCandidateSustained_;
        TimePoint                 activeSince        = radiantActiveSince_;
        const bool radiantAdvanced =
            !hasEnvelope_ ||
            !sameSample (envelope.radiant, observation_.envelope.radiant);

        next.envelope              = envelope;
        next.thermistorAge         = Duration (thermistorAge);
        next.digitalTemperatureAge = Duration (digitalAge);
        next.radiantAge            = Duration (radiantAge);
        next.thermalHazard         = false;
        next.radiantHazard         = false;
        next.status                = StatusCode::Ok;

        const uint32_t allowedAge = config_.maximumAge.milliseconds ();

        const bool     thermistorUsable = envelope.thermistor.status.ok () &&
                                      !envelope.thermistor.saturated &&
                                      thermistorAge <= allowedAge;

        const bool digitalUsable = envelope.digitalTemperature.status.ok () &&
                                   !envelope.digitalTemperature.saturated &&
                                   digitalAge <= allowedAge;
        const ThermalQuality thermistorQuality =
            classifyThermistor (envelope.thermistor, config_);

        if (!envelope.thermistor.status.ok () ||
            !envelope.digitalTemperature.status.ok ())
        {
            next.thermalQuality = ThermalQuality::ProducerFault;
            next.status         = !envelope.thermistor.status.ok ()
                                      ? envelope.thermistor.status
                                      : envelope.digitalTemperature.status;
        }
        else if (envelope.thermistor.saturated || envelope.digitalTemperature.saturated)
        {
            next.thermalQuality = ThermalQuality::Saturated;
        }
        else if (thermistorAge > config_.maximumAge.milliseconds () ||
                 digitalAge > config_.maximumAge.milliseconds ())
        {
            next.thermalQuality = ThermalQuality::Stale;
        }
        else if (envelope.digitalTemperature.state == ThresholdState::AtOrAbove)
        {
            next.thermalQuality = ThermalQuality::Alarm;
        }
        else if (thermistorQuality == ThermalQuality::Warning ||
                 thermistorQuality == ThermalQuality::Alarm)
        {
            next.thermalQuality = ThermalQuality::Disagreement;
        }
        else
        {
            next.thermalQuality = ThermalQuality::Normal;
        }

        next.thermalHazard =
            (thermistorUsable && thermistorQuality == ThermalQuality::Alarm) ||
            (digitalUsable &&
             envelope.digitalTemperature.state == ThresholdState::AtOrAbove);

        if (!envelope.radiant.status.ok ())
        {
            next.radiantQuality = RadiantQuality::ProducerFault;
            candidateActive     = false;
            candidateSustained  = false;
            if (next.status.ok ())
            {
                next.status = envelope.radiant.status;
            }
        }
        else if (envelope.radiant.saturated)
        {
            next.radiantQuality = RadiantQuality::SaturatedAmbient;
            candidateActive     = false;
            candidateSustained  = false;
        }
        else if (radiantAge > config_.maximumAge.milliseconds ())
        {
            next.radiantQuality = RadiantQuality::Stale;
            candidateActive     = false;
            candidateSustained  = false;
        }
        else if (!radiantAdvanced)
        {
            next.radiantQuality = observation_.radiantQuality;
            next.radiantHazard  = observation_.radiantHazard;
        }
        else if (envelope.radiant.state == ThresholdState::AtOrAbove)
        {
            if (!candidateActive)
            {
                candidateActive    = true;
                candidateSustained = false;
                activeSince        = envelope.radiant.observedAt;
            }

            const Duration elapsed =
                envelope.radiant.observedAt.elapsedSince (activeSince);
            if (elapsed >= config_.radiantSustainedMinimum)
            {
                candidateSustained = true;
            }
            next.radiantQuality = candidateSustained
                                      ? RadiantQuality::Sustained
                                      : RadiantQuality::AbruptChange;
            next.radiantHazard  = true;
        }
        else if (candidateActive)
        {
            const Duration elapsed =
                envelope.radiant.observedAt.elapsedSince (activeSince);
            next.radiantQuality = !candidateSustained &&
                                          elapsed <= config_.radiantPulseMaximum
                                      ? RadiantQuality::AbruptChange
                                      : RadiantQuality::Sustained;
            next.radiantHazard  = next.radiantQuality == RadiantQuality::Sustained;
            candidateActive     = false;
            candidateSustained  = false;
        }
        else
        {
            next.radiantQuality = RadiantQuality::Quiet;
        }

        observation_                 = next;
        lastUpdateAt_                = now;
        radiantActiveSince_          = activeSince;
        radiantCandidateActive_      = candidateActive;
        radiantCandidateSustained_   = candidateSustained;
        hasEnvelope_                 = true;
        hasUpdate_                   = true;
        thermistorSequenceExhausted_ = envelope.thermistor.sequence == UINT32_MAX;
        digitalSequenceExhausted_ = envelope.digitalTemperature.sequence == UINT32_MAX;
        radiantSequenceExhausted_ = envelope.radiant.sequence == UINT32_MAX;
        return observation_.status;
    }

    ThermalRadiantObservation
    ThermalRadiantObservationPolicy::snapshot () const noexcept
    {
        return observation_;
    }

    bool ThermalRadiantObservationPolicy::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
