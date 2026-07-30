#include "inertial_record_qualification.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        InertialVector zeroVector () noexcept
        {
            return {0, 0, 0};
        }

        InertialRecord zeroRecord () noexcept
        {
            return {0,
                    0,
                    {},
                    zeroVector (),
                    zeroVector (),
                    TimePoint  (),
                    0,
                    false,
                    InertialSaturation::None,
                    Status (),
                    InertialRecordState::NotReady};
        }

        InertialQualificationEvidence emptyEvidence (
            uint16_t revision, uint32_t generation,
            const SourceAxisMapping& mapping) noexcept
        {
            return {0,
                    generation,
                    revision,
                    mapping,
                    InertialQualificationState::Idle,
                    InertialQualificationReason::None,
                    0,
                    0,
                    0,
                    TimePoint  (),
                    TimePoint  (),
                    Duration   (),
                    Duration   (),
                    zeroVector (),
                    zeroVector (),
                    zeroVector (),
                    zeroVector (),
                    zeroVector (),
                    zeroVector (),
                    {0, 0, 0},
                    {0, 0, 0},
                    zeroRecord (),
                    zeroRecord (),
                    Status     ()};
        }

        bool sameSource (const InertialSource& left,
                         const InertialSource& right) noexcept
        {
            return left.kind == right.kind && left.model == right.model &&
                   left.sourceId == right.sourceId &&
                   left.configurationRevision ==
                       right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.accelerationRangeMicroG ==
                       right.accelerationRangeMicroG &&
                   left.angularRateRangeMilliDegreesPerSecond ==
                       right.angularRateRangeMilliDegreesPerSecond;
        }

        bool nonnegative (const InertialVector& value) noexcept
        {
            return value.x >= 0 && value.y >= 0 && value.z >= 0;
        }

        bool validSource (const InertialSource& source) noexcept
        {
            const bool matchingPair =
                (source.kind == InertialSourceKind::SyntheticFixture &&
                 source.model == InertialModel::Synthetic) ||
                (source.kind == InertialSourceKind::Mpu6050Adapter &&
                 source.model == InertialModel::Mpu6050) ||
                (source.kind == InertialSourceKind::Qmi8658Adapter &&
                 source.model == InertialModel::Qmi8658UnknownRevision);
            return matchingPair && source.sourceId != 0 &&
                   source.configurationRevision != 0 &&
                   source.calibrationRevision != 0 &&
                   source.accelerationRangeMicroG != 0 &&
                   source.angularRateRangeMilliDegreesPerSecond != 0 &&
                   source.accelerationRangeMicroG <=
                       static_cast<uint32_t> (INT32_MAX) &&
                   source.angularRateRangeMilliDegreesPerSecond <=
                       static_cast<uint32_t> (INT32_MAX);
        }

        bool accelerationBoundsRepresentable (
            const InertialRecordQualificationConfig& config) noexcept
        {
            const int64_t expected[3] = {
                config.expectedStationaryAccelerationMicroG.x,
                config.expectedStationaryAccelerationMicroG.y,
                config.expectedStationaryAccelerationMicroG.z};
            const int64_t deviation[3] = {
                config.maximumAccelerationDeviationMicroG.x,
                config.maximumAccelerationDeviationMicroG.y,
                config.maximumAccelerationDeviationMicroG.z};
            for (uint8_t index = 0; index < 3; ++index)
            {
                if (expected[index] - deviation[index] < INT32_MIN ||
                    expected[index] + deviation[index] > INT32_MAX)
                {
                    return false;
                }
            }
            return true;
        }

        bool validConfiguration (
            const InertialRecordQualificationConfig& config) noexcept
        {
            const uint32_t maximumAge = config.maximumAge.milliseconds ();

            const uint32_t maximumGap = config.maximumGap.milliseconds ();

            const bool sourceValid = validSource (config.expectedSource);

            const bool mappingValid = validSignedAxisMapping (
                config.sourceToQualificationFrame);
            return config.qualificationRevision != 0 &&
                   config.expectedSchemaRevision != 0 &&
                   config.expectedNormalizationRevision != 0 &&
                   sourceValid && mappingValid &&
                   config.requiredSampleCount >= 2 &&
                   config.requiredSampleCount <= 32 &&
                   maximumAge != 0 && maximumGap != 0 &&
                   maximumAge < halfRange && maximumGap < halfRange &&
                   nonnegative (
                       config.maximumAccelerationDeviationMicroG) &&
                   nonnegative (
                       config.maximumAngularRateMilliDegreesPerSecond) &&
                   accelerationBoundsRepresentable (config);
        }

        bool validRecord (const InertialRecord& record) noexcept
        {
            uint8_t image[InertialRecordCodec::size] = {};
            return InertialRecordCodec ()
                .encode (record,
                         MutableByteSpan {
                             image, InertialRecordCodec::size})
                .ok ();
        }

        bool sameRecord (const InertialRecord& left,
                         const InertialRecord& right) noexcept
        {
            uint8_t leftImage[InertialRecordCodec::size]   = {};
            uint8_t rightImage[InertialRecordCodec::size]  = {};
            InertialRecordCodec codec;
            const Result<uint16_t> leftResult =
                codec.encode (
                    left,
                    MutableByteSpan {
                        leftImage, InertialRecordCodec::size});
            const Result<uint16_t> rightResult =
                codec.encode (
                    right,
                    MutableByteSpan {
                        rightImage, InertialRecordCodec::size});
            if (!leftResult.ok () || !rightResult.ok ())
            {
                return false;
            }
            for (uint8_t index = 0; index < InertialRecordCodec::size; ++index)
            {
                if (leftImage[index] != rightImage[index])
                {
                    return false;
                }
            }
            return true;
        }

        bool mapVector (const SourceAxisMapping& mapping,
                        const InertialVector&     input,
                        InertialVector&           output) noexcept
        {
            return mapSignedAxes (mapping, input.x, input.y, input.z, output.x,
                                  output.y, output.z);
        }

        uint32_t absolute (int32_t value) noexcept
        {
            const int64_t widened = value;
            return static_cast<uint32_t> (widened < 0 ? -widened : widened);
        }

        bool accelerationInWindow (
            const InertialRecordQualificationConfig& config,
            const InertialVector& value) noexcept
        {
            const int64_t xDifference =
                static_cast<int64_t> (value.x) -
                config.expectedStationaryAccelerationMicroG.x;
            const int64_t yDifference =
                static_cast<int64_t> (value.y) -
                config.expectedStationaryAccelerationMicroG.y;
            const int64_t zDifference =
                static_cast<int64_t> (value.z) -
                config.expectedStationaryAccelerationMicroG.z;
            return (xDifference < 0 ? -xDifference : xDifference) <=
                       config.maximumAccelerationDeviationMicroG.x &&
                   (yDifference < 0 ? -yDifference : yDifference) <=
                       config.maximumAccelerationDeviationMicroG.y &&
                   (zDifference < 0 ? -zDifference : zDifference) <=
                       config.maximumAccelerationDeviationMicroG.z;
        }

        bool angularRateInWindow (
            const InertialRecordQualificationConfig& config,
            const InertialVector& value) noexcept
        {
            return absolute (value.x) <=
                       static_cast<uint32_t> (
                           config.maximumAngularRateMilliDegreesPerSecond.x) &&
                   absolute (value.y) <=
                       static_cast<uint32_t> (
                           config.maximumAngularRateMilliDegreesPerSecond.y) &&
                   absolute (value.z) <=
                       static_cast<uint32_t> (
                           config.maximumAngularRateMilliDegreesPerSecond.z);
        }

        void reject (InertialQualificationEvidence& evidence,
                     InertialQualificationReason    reason,
                     const InertialRecord&          record,
                     Status                         status = Status ()) noexcept
        {
            evidence.state          = InertialQualificationState::Rejected;
            evidence.reason         = reason;
            evidence.terminalRecord = record;
            evidence.status         = status;
        }

        int32_t mean (int64_t sum, uint8_t count) noexcept
        {
            return static_cast<int32_t> (sum / count);
        }

        void updateMinimum (InertialVector& target,
                            const InertialVector& value) noexcept
        {
            if (value.x < target.x)
            {
                target.x = value.x;
            }
            if (value.y < target.y)
            {
                target.y = value.y;
            }
            if (value.z < target.z)
            {
                target.z = value.z;
            }
        }

        void updateMaximum (InertialVector& target,
                            const InertialVector& value) noexcept
        {
            if (value.x > target.x)
            {
                target.x = value.x;
            }
            if (value.y > target.y)
            {
                target.y = value.y;
            }
            if (value.z > target.z)
            {
                target.z = value.z;
            }
        }
    } // namespace

    InertialRecordQualificationPolicy::InertialRecordQualificationPolicy (
        const InertialRecordQualificationConfig& config) noexcept
        : config_ (config),
          evidence_ (emptyEvidence (config.qualificationRevision, 0,
                                    config.sourceToQualificationFrame)),
          accelerationSums_ {0, 0, 0},
          angularRateSums_ {0, 0, 0},
          initialized_ {false},
          active_ {false},
          shutdown_ {false},
          lifecycleGeneration_ (0)
    {
    }

    Status InertialRecordQualificationPolicy::initialize (
        TimePoint) noexcept
    {
        if (initialized_ || shutdown_)
        {
            return StatusCode::InvalidArgument;
        }
        if (!validConfiguration (config_))
        {
            return StatusCode::InvalidConfiguration;
        }
        if (config_.expectedSource.kind !=
                InertialSourceKind::SyntheticFixture ||
            config_.expectedSource.model != InertialModel::Synthetic)
        {
            return StatusCode::Unsupported;
        }
        if (lifecycleGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        ++lifecycleGeneration_;
        evidence_ = emptyEvidence (config_.qualificationRevision,
                                   lifecycleGeneration_,
                                   config_.sourceToQualificationFrame);
        initialized_ = true;
        return {};
    }

    Status InertialRecordQualificationPolicy::begin (
        TimePoint, uint32_t attemptId) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (active_ || evidence_.state != InertialQualificationState::Idle ||
            attemptId == 0)
        {
            return StatusCode::InvalidArgument;
        }
        evidence_ = emptyEvidence (config_.qualificationRevision,
                                   lifecycleGeneration_,
                                   config_.sourceToQualificationFrame);
        evidence_.attemptId  = attemptId;
        evidence_.state      = InertialQualificationState::Collecting;
        accelerationSums_[0] = accelerationSums_[1] = accelerationSums_[2] = 0;
        angularRateSums_[0] = angularRateSums_[1] = angularRateSums_[2] = 0;
        active_ = true;
        return {};
    }

    Status InertialRecordQualificationPolicy::observe (
        TimePoint now, const InertialRecord& record) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (!active_)
        {
            return StatusCode::InvalidArgument;
        }
        if (!validRecord (record))
        {
            return StatusCode::InvalidArgument;
        }

        InertialQualificationEvidence candidate = evidence_;
        if (record.schemaRevision != config_.expectedSchemaRevision ||
            record.normalizationRevision !=
                config_.expectedNormalizationRevision ||
            !sameSource (record.source, config_.expectedSource))
        {
            reject (candidate,
                    InertialQualificationReason::ConfigurationMismatch, record);
            evidence_ = candidate;
            active_   = false;
            return {};
        }

        const uint32_t age =
            now.milliseconds () - record.observedAt.milliseconds ();
        if (age >= halfRange)
        {
            reject (candidate,
                    InertialQualificationReason::TimestampDiscontinuity,
                    record);
            evidence_ = candidate;
            active_   = false;
            return {};
        }
        if (age > config_.maximumAge.milliseconds ())
        {
            reject (candidate, InertialQualificationReason::Stale, record);
            evidence_ = candidate;
            active_   = false;
            return {};
        }

        if (candidate.acceptedSampleCount != 0)
        {
            const uint32_t sequenceDelta =
                record.sequence - candidate.lastSequence;
            if (sequenceDelta == 0)
            {
                if (sameRecord (record, candidate.terminalRecord))
                {
                    return {};
                }
                reject (candidate,
                        InertialQualificationReason::SequenceDiscontinuity,
                        record);
                evidence_ = candidate;
                active_   = false;
                return {};
            }
            if (sequenceDelta >= halfRange)
            {
                reject (candidate,
                        InertialQualificationReason::SequenceDiscontinuity,
                        record);
                evidence_ = candidate;
                active_   = false;
                return {};
            }
            if (sequenceDelta != 1)
            {
                reject (candidate,
                        InertialQualificationReason::SequenceDiscontinuity,
                        record);
                evidence_ = candidate;
                active_   = false;
                return {};
            }
            const uint32_t observedAt =
                record.observedAt.milliseconds ();
            const uint32_t lastObservedAt =
                candidate.lastObservedAt.milliseconds ();
            const uint32_t observedGap = observedAt - lastObservedAt;
            if (observedGap == 0 || observedGap >= halfRange ||
                observedGap > config_.maximumGap.milliseconds ())
            {
                reject (candidate,
                        InertialQualificationReason::TimestampDiscontinuity,
                        record);
                evidence_ = candidate;
                active_   = false;
                return {};
            }
            if (observedGap > candidate.maximumObservedGap.milliseconds ())
            {
                candidate.maximumObservedGap = Duration (observedGap);
            }
        }

        if (!record.producerStatus.ok () ||
            record.state == InertialRecordState::SourceFault)
        {
            reject (candidate, InertialQualificationReason::ProducerFault,
                    record);
            evidence_ = candidate;
            active_   = false;
            return {};
        }
        if (!record.dataReady ||
            record.state == InertialRecordState::NotReady)
        {
            reject (candidate, InertialQualificationReason::NotReady, record);
            evidence_ = candidate;
            active_   = false;
            return {};
        }
        if (record.saturation != InertialSaturation::None)
        {
            reject (candidate, InertialQualificationReason::Saturated, record);
            evidence_ = candidate;
            active_   = false;
            return {};
        }

        InertialRecord mapped = record;
        if (!mapVector (config_.sourceToQualificationFrame,
                        record.accelerationMicroG,
                        mapped.accelerationMicroG) ||
            !mapVector (config_.sourceToQualificationFrame,
                        record.angularRateMilliDegreesPerSecond,
                        mapped.angularRateMilliDegreesPerSecond))
        {
            reject (candidate, InertialQualificationReason::ArithmeticOverflow,
                    record, StatusCode::InvalidArgument);
            evidence_ = candidate;
            active_   = false;
            return StatusCode::InvalidArgument;
        }
        if (!accelerationInWindow (config_, mapped.accelerationMicroG))
        {
            reject (
                candidate,
                InertialQualificationReason::AccelerationOutsideWindow, record);
            candidate.mappedRecord = mapped;
            evidence_              = candidate;
            active_                = false;
            return {};
        }
        if (!angularRateInWindow (
                config_, mapped.angularRateMilliDegreesPerSecond))
        {
            reject (
                candidate,
                InertialQualificationReason::AngularRateOutsideWindow, record);
            candidate.mappedRecord = mapped;
            evidence_              = candidate;
            active_                = false;
            return {};
        }

        const InertialVector acceleration = mapped.accelerationMicroG;
        const InertialVector rate =
            mapped.angularRateMilliDegreesPerSecond;
        if (candidate.acceptedSampleCount == 0)
        {
            candidate.firstSequence = record.sequence;
            candidate.firstObservedAt = record.observedAt;
            candidate.minimumAccelerationMicroG = acceleration;
            candidate.maximumAccelerationMicroG = acceleration;
            candidate.minimumAngularRateMilliDegreesPerSecond = rate;
            candidate.maximumAngularRateMilliDegreesPerSecond = rate;
        }
        else
        {
            updateMinimum (candidate.minimumAccelerationMicroG, acceleration);
            updateMaximum (candidate.maximumAccelerationMicroG, acceleration);
            updateMinimum (
                candidate.minimumAngularRateMilliDegreesPerSecond, rate);
            updateMaximum (
                candidate.maximumAngularRateMilliDegreesPerSecond, rate);
        }
        accelerationSums_[0] += acceleration.x;
        accelerationSums_[1] += acceleration.y;
        accelerationSums_[2] += acceleration.z;
        angularRateSums_[0] += rate.x;
        angularRateSums_[1] += rate.y;
        angularRateSums_[2] += rate.z;
        candidate.accelerationSumsMicroG = {
            accelerationSums_[0],
            accelerationSums_[1],
            accelerationSums_[2]};
        candidate.angularRateSumsMilliDegreesPerSecond = {
            angularRateSums_[0],
            angularRateSums_[1],
            angularRateSums_[2]};
        ++candidate.acceptedSampleCount;
        candidate.lastSequence   = record.sequence;
        candidate.lastObservedAt = record.observedAt;
        candidate.terminalRecord = record;
        candidate.mappedRecord   = mapped;
        if (age > candidate.maximumObservedAge.milliseconds ())
        {
            candidate.maximumObservedAge = Duration (age);
        }
        candidate.meanAccelerationMicroG = {
            mean (accelerationSums_[0], candidate.acceptedSampleCount),
            mean (accelerationSums_[1], candidate.acceptedSampleCount),
            mean (accelerationSums_[2], candidate.acceptedSampleCount)};
        candidate.meanAngularRateMilliDegreesPerSecond = {
            mean (angularRateSums_[0], candidate.acceptedSampleCount),
            mean (angularRateSums_[1], candidate.acceptedSampleCount),
            mean (angularRateSums_[2], candidate.acceptedSampleCount)};
        if (candidate.acceptedSampleCount == config_.requiredSampleCount)
        {
            candidate.state  = InertialQualificationState::Qualified;
            candidate.reason = InertialQualificationReason::None;
            active_          = false;
        }
        evidence_ = candidate;
        return {};
    }

    Status InertialRecordQualificationPolicy::reset (TimePoint) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (lifecycleGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        ++lifecycleGeneration_;
        evidence_ = emptyEvidence (config_.qualificationRevision,
                                   lifecycleGeneration_,
                                   config_.sourceToQualificationFrame);
        accelerationSums_[0] = accelerationSums_[1] = accelerationSums_[2] = 0;
        angularRateSums_[0] = angularRateSums_[1] = angularRateSums_[2] = 0;
        active_ = false;
        return {};
    }

    Status InertialRecordQualificationPolicy::shutdown (TimePoint) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        evidence_ = emptyEvidence (config_.qualificationRevision,
                                   lifecycleGeneration_,
                                   config_.sourceToQualificationFrame);
        active_      = false;
        initialized_ = false;
        shutdown_    = true;
        return {};
    }

    Status InertialRecordQualificationPolicy::evidence (
        InertialQualificationEvidence& output) const noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        output = evidence_;
        return {};
    }

    bool InertialRecordQualificationPolicy::initialized () const noexcept
    {
        return initialized_ && !shutdown_;
    }
} // namespace adk
