#include "inertial_observation.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        InertialObservation emptyObservation () noexcept
        {
            return {{{InertialSourceKind::SyntheticFixture,
                      InertialModel::Synthetic,
                      0,
                      0,
                      0,
                      0,
                      0},
                     {0, 0, 0},
                     {0, 0, 0},
                     TimePoint (),
                     0,
                     false,
                     InertialSaturation::None,
                     StatusCode::NotInitialized},
                    InertialSampleQuality::Invalid,
                    false,
                    Duration (),
                    Duration (),
                    0,
                    0,
                    StatusCode::NotInitialized};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validSaturation (InertialSaturation saturation) noexcept
        {
            return saturation >= InertialSaturation::None &&
                   saturation <= InertialSaturation::Both;
        }

        bool sameSource (const InertialSource& left,
                         const InertialSource& right) noexcept
        {
            return left.kind == right.kind && left.model == right.model &&
                   left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.accelerationRangeMicroG ==
                       right.accelerationRangeMicroG &&
                   left.angularRateRangeMilliDegreesPerSecond ==
                       right.angularRateRangeMilliDegreesPerSecond;
        }

        bool sameVector (const InertialVector& left,
                         const InertialVector& right) noexcept
        {
            return left.x == right.x && left.y == right.y && left.z == right.z;
        }

        bool sameSample (const InertialSample& left,
                         const InertialSample& right) noexcept
        {
            return sameSource (left.source, right.source) &&
                   sameVector (left.accelerationMicroG,
                               right.accelerationMicroG) &&
                   sameVector (left.angularRateMilliDegreesPerSecond,
                               right.angularRateMilliDegreesPerSecond) &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence &&
                   left.dataReady == right.dataReady &&
                   left.saturation == right.saturation &&
                   left.status == right.status;
        }

        bool sameReadinessPayload (const InertialSample& poll,
                                   const InertialSample& accepted) noexcept
        {
            return sameSource (poll.source, accepted.source) &&
                   sameVector (poll.accelerationMicroG,
                               accepted.accelerationMicroG) &&
                   sameVector (poll.angularRateMilliDegreesPerSecond,
                               accepted.angularRateMilliDegreesPerSecond) &&
                   poll.observedAt == accepted.observedAt &&
                   poll.sequence == accepted.sequence &&
                   accepted.dataReady && !poll.dataReady &&
                   poll.saturation == accepted.saturation &&
                   poll.status == accepted.status;
        }

        uint32_t magnitude (int32_t value) noexcept
        {
            const int64_t widened = value;

            return static_cast<uint32_t> (widened < 0 ? -widened : widened);
        }

        bool anyBeyond (const InertialVector& vector, uint32_t range) noexcept
        {
            return magnitude (vector.x) > range || magnitude (vector.y) > range ||
                   magnitude (vector.z) > range;
        }

        bool anyAt (const InertialVector& vector, uint32_t range) noexcept
        {
            return magnitude (vector.x) == range ||
                   magnitude (vector.y) == range ||
                   magnitude (vector.z) == range;
        }

        InertialSaturation measuredSaturation (
            const InertialSample& sample) noexcept
        {
            const bool acceleration =
                anyAt (sample.accelerationMicroG,
                       sample.source.accelerationRangeMicroG);
            const bool angularRate =
                anyAt (sample.angularRateMilliDegreesPerSecond,
                       sample.source.angularRateRangeMilliDegreesPerSecond);

            return static_cast<InertialSaturation> (
                (acceleration ? 1U : 0U) | (angularRate ? 2U : 0U));
        }

        Status validateSource (const InertialSource& source) noexcept
        {
            const bool recognizedKind =
                source.kind >= InertialSourceKind::SyntheticFixture &&
                source.kind <= InertialSourceKind::Qmi8658Adapter;
            const bool recognizedModel =
                source.model >= InertialModel::Synthetic &&
                source.model <= InertialModel::Qmi8658UnknownRevision;

            if (!recognizedKind || !recognizedModel || source.sourceId == 0 ||
                source.configurationRevision == 0 ||
                source.calibrationRevision == 0 ||
                source.accelerationRangeMicroG == 0 ||
                source.angularRateRangeMilliDegreesPerSecond == 0 ||
                source.accelerationRangeMicroG >
                    static_cast<uint32_t> (INT32_MAX) ||
                source.angularRateRangeMilliDegreesPerSecond >
                    static_cast<uint32_t> (INT32_MAX))
            {
                return StatusCode::InvalidArgument;
            }

            if (source.kind == InertialSourceKind::SyntheticFixture &&
                source.model == InertialModel::Synthetic)
            {
                return StatusCode::Ok;
            }

            if ((source.kind == InertialSourceKind::Mpu6050Adapter &&
                 source.model == InertialModel::Mpu6050) ||
                (source.kind == InertialSourceKind::Qmi8658Adapter &&
                 source.model == InertialModel::Qmi8658UnknownRevision))
            {
                return StatusCode::Unsupported;
            }

            return StatusCode::InvalidArgument;
        }
    } // namespace

    InertialObservationPolicy::InertialObservationPolicy (
        const InertialObservationConfig& config) noexcept
        : config_       (config),
          observation_  (emptyObservation ()),
          lastUpdateAt_ (),
          initialized_  (false),
          hasSample_    (false),
          hasUpdate_    (false)
    {
    }

    Status InertialObservationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (config_.maximumAge.milliseconds () == 0 ||
            config_.maximumAge.milliseconds () >= halfRange ||
            config_.freshnessContractRevision == 0)
        {
            observation_.status = StatusCode::InvalidConfiguration;
            return observation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void InertialObservationPolicy::reset () noexcept
    {
        observation_ = emptyObservation ();

        lastUpdateAt_ = TimePoint ();
        hasSample_    = false;
        hasUpdate_    = false;

        if (initialized_)
        {
            observation_.maximumAge = config_.maximumAge;
            observation_.freshnessContractRevision =
                config_.freshnessContractRevision;
        }
    }

    Status InertialObservationPolicy::update (
        TimePoint now, const InertialSample& sample) noexcept
    {
        observation_.latestDataReady = false;

        if (!initialized_)
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = StatusCode::NotInitialized;
            return observation_.status;
        }

        const uint32_t sampleAge = now.elapsedSince (sample.observedAt).milliseconds ();

        if (sampleAge >= halfRange ||
            (hasUpdate_ &&
             now.elapsedSince (lastUpdateAt_).milliseconds () >= halfRange))
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = StatusCode::InvalidArgument;
            return observation_.status;
        }

        if (!validStatus (sample.status))
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = StatusCode::InvalidArgument;
            return observation_.status;
        }

        if (!sample.status.ok ())
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = sample.status;
            return observation_.status;
        }

        const Status sourceStatus = validateSource (sample.source);

        if (!sourceStatus.ok ())
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = sourceStatus;
            return observation_.status;
        }

        if (!validSaturation (sample.saturation))
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = StatusCode::InvalidArgument;
            return observation_.status;
        }

        if (!sample.dataReady)
        {
            if (!hasSample_ ||
                !sameReadinessPayload (sample, observation_.sample))
            {
                observation_.quality = InertialSampleQuality::Invalid;
                observation_.status  = StatusCode::InvalidArgument;
                return observation_.status;
            }

            const uint32_t acceptedAge =
                now.elapsedSince (observation_.sample.observedAt).milliseconds ();

            if (acceptedAge >= halfRange)
            {
                observation_.quality = InertialSampleQuality::Invalid;
                observation_.status  = StatusCode::InvalidArgument;
                return observation_.status;
            }

            observation_.quality         = InertialSampleQuality::Stale;
            observation_.latestDataReady = false;
            observation_.age             = Duration (acceptedAge);
            observation_.sequenceGap     = 0;
            observation_.status          = StatusCode::Ok;
            lastUpdateAt_                = now;
            hasUpdate_                   = true;
            return observation_.status;
        }

        const bool sameDomain =
            hasSample_ && sameSource (sample.source, observation_.sample.source);
        uint32_t sequenceGap = 0;

        if (sameDomain)
        {
            const uint32_t sequenceDelta =
                sample.sequence - observation_.sample.sequence;

            if (sequenceDelta == 0)
            {
                if (!sameSample (sample, observation_.sample))
                {
                    observation_.quality = InertialSampleQuality::Invalid;
                    observation_.status  = StatusCode::InvalidArgument;
                    return observation_.status;
                }

                observation_.age             = Duration (sampleAge);
                observation_.latestDataReady = true;
                observation_.sequenceGap     = 0;

                if (!observation_.sample.status.ok ())
                {
                    observation_.quality = InertialSampleQuality::Invalid;
                    observation_.status  = observation_.sample.status;
                }
                else if (observation_.sample.saturation !=
                         InertialSaturation::None)
                {
                    observation_.quality = InertialSampleQuality::Saturated;
                    observation_.status  = StatusCode::Ok;
                }
                else if (!observation_.sample.dataReady ||
                         sampleAge > config_.maximumAge.milliseconds ())
                {
                    observation_.quality = InertialSampleQuality::Stale;
                    observation_.status  = StatusCode::Ok;
                }
                else
                {
                    observation_.quality = InertialSampleQuality::Current;
                    observation_.status  = StatusCode::Ok;
                }

                lastUpdateAt_ = now;
                hasUpdate_    = true;
                return observation_.status;
            }

            if (sequenceDelta >= halfRange)
            {
                observation_.quality = InertialSampleQuality::Invalid;
                observation_.status  = StatusCode::InvalidArgument;
                return observation_.status;
            }

            sequenceGap = sequenceDelta - 1U;
        }

        if (anyBeyond (sample.accelerationMicroG,
                       sample.source.accelerationRangeMicroG) ||
            anyBeyond (sample.angularRateMilliDegreesPerSecond,
                       sample.source.angularRateRangeMilliDegreesPerSecond) ||
            measuredSaturation (sample) != sample.saturation)
        {
            observation_.quality = InertialSampleQuality::Invalid;
            observation_.status  = StatusCode::InvalidArgument;
            return observation_.status;
        }

        observation_.sample          = sample;
        observation_.latestDataReady = true;
        observation_.age             = Duration (sampleAge);
        observation_.sequenceGap     = sequenceGap;
        observation_.status          = StatusCode::Ok;

        if (sample.saturation != InertialSaturation::None)
        {
            observation_.quality = InertialSampleQuality::Saturated;
        }
        else if (!sample.dataReady ||
                 sampleAge > config_.maximumAge.milliseconds ())
        {
            observation_.quality = InertialSampleQuality::Stale;
        }
        else
        {
            observation_.quality = InertialSampleQuality::Current;
        }

        hasSample_    = true;
        lastUpdateAt_ = now;
        hasUpdate_    = true;
        return observation_.status;
    }

    InertialObservation InertialObservationPolicy::snapshot () const noexcept
    {
        return observation_;
    }

    bool InertialObservationPolicy::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
