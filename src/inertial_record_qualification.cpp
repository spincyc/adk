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

        void clearEvidence (InertialQualificationEvidence& evidence,
                            uint16_t revision, uint32_t generation,
                            const SourceAxisMapping& mapping) noexcept
        {
            evidence.attemptId                  = 0;
            evidence.lifecycleGeneration        = generation;
            evidence.qualificationRevision      = revision;
            evidence.sourceToQualificationFrame = mapping;
            evidence.state      = InertialQualificationState::Idle;
            evidence.reason     = InertialQualificationReason::None;
            evidence.acceptedSampleCount = 0;
            evidence.firstSequence       = 0;
            evidence.lastSequence        = 0;
            evidence.firstObservedAt     = TimePoint {};
            evidence.lastObservedAt      = TimePoint {};
            evidence.maximumObservedAge  = Duration {};
            evidence.maximumObservedGap  = Duration {};
            evidence.meanAccelerationMicroG = {0, 0, 0};
            evidence.meanAngularRateMilliDegreesPerSecond = {0, 0, 0};
            evidence.minimumAccelerationMicroG = {0, 0, 0};
            evidence.maximumAccelerationMicroG = {0, 0, 0};
            evidence.minimumAngularRateMilliDegreesPerSecond = {0, 0, 0};
            evidence.maximumAngularRateMilliDegreesPerSecond = {0, 0, 0};
            evidence.accelerationSumsMicroG = {0, 0, 0};
            evidence.angularRateSumsMilliDegreesPerSecond = {0, 0, 0};
            evidence.terminalRecord = zeroRecord ();
            evidence.mappedRecord   = zeroRecord ();
            evidence.status         = Status {};
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

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        uint32_t magnitude (int32_t value) noexcept
        {
            const int64_t widened = value;
            return static_cast<uint32_t> (widened < 0 ? -widened : widened);
        }

        bool withinRange (const InertialVector& value,
                          uint32_t range) noexcept
        {
            return magnitude (value.x) <= range &&
                   magnitude (value.y) <= range &&
                   magnitude (value.z) <= range;
        }

        InertialSaturation measuredSaturation (
            const InertialRecord& record) noexcept
        {
            const bool acceleration =
                magnitude (record.accelerationMicroG.x) ==
                    record.source.accelerationRangeMicroG ||
                magnitude (record.accelerationMicroG.y) ==
                    record.source.accelerationRangeMicroG ||
                magnitude (record.accelerationMicroG.z) ==
                    record.source.accelerationRangeMicroG;
            const bool angularRate =
                magnitude (record.angularRateMilliDegreesPerSecond.x) ==
                    record.source.angularRateRangeMilliDegreesPerSecond ||
                magnitude (record.angularRateMilliDegreesPerSecond.y) ==
                    record.source.angularRateRangeMilliDegreesPerSecond ||
                magnitude (record.angularRateMilliDegreesPerSecond.z) ==
                    record.source.angularRateRangeMilliDegreesPerSecond;
            return static_cast<InertialSaturation> (
                (acceleration ? 1U : 0U) | (angularRate ? 2U : 0U));
        }

        bool validRecord (const InertialRecord& record) noexcept
        {
            const bool validState =
                record.state == InertialRecordState::Recorded ||
                record.state == InertialRecordState::NotReady ||
                record.state == InertialRecordState::SourceFault;
            const bool validSaturation =
                record.saturation >= InertialSaturation::None &&
                record.saturation <= InertialSaturation::Both;
            if (record.schemaRevision == 0 ||
                record.normalizationRevision == 0 ||
                !validSource (record.source) || !validStatus (record.producerStatus) ||
                !validState || !validSaturation ||
                !withinRange (record.accelerationMicroG,
                              record.source.accelerationRangeMicroG) ||
                !withinRange (
                    record.angularRateMilliDegreesPerSecond,
                    record.source.angularRateRangeMilliDegreesPerSecond) ||
                measuredSaturation (record) != record.saturation)
            {
                return false;
            }
            if (record.state == InertialRecordState::SourceFault)
            {
                return !record.producerStatus.ok ();
            }
            return record.producerStatus.ok () &&
                   record.dataReady ==
                       (record.state == InertialRecordState::Recorded);
        }

        bool sameRecord (const InertialRecord& left,
                         const InertialRecord& right) noexcept
        {
            return left.schemaRevision == right.schemaRevision &&
                   left.normalizationRevision ==
                       right.normalizationRevision &&
                   sameSource (left.source, right.source) &&
                   left.accelerationMicroG.x == right.accelerationMicroG.x &&
                   left.accelerationMicroG.y == right.accelerationMicroG.y &&
                   left.accelerationMicroG.z == right.accelerationMicroG.z &&
                   left.angularRateMilliDegreesPerSecond.x ==
                       right.angularRateMilliDegreesPerSecond.x &&
                   left.angularRateMilliDegreesPerSecond.y ==
                       right.angularRateMilliDegreesPerSecond.y &&
                   left.angularRateMilliDegreesPerSecond.z ==
                       right.angularRateMilliDegreesPerSecond.z &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence &&
                   left.dataReady == right.dataReady &&
                   left.saturation == right.saturation &&
                   left.producerStatus == right.producerStatus &&
                   left.state == right.state;
        }

        bool mapAxis (SignedAxis axis, const InertialVector& input,
                      int32_t& output) noexcept
        {
            int32_t value = 0;
            switch (axis)
            {
                case SignedAxis::PositiveX:
                case SignedAxis::NegativeX: value = input.x; break;
                case SignedAxis::PositiveY:
                case SignedAxis::NegativeY: value = input.y; break;
                case SignedAxis::PositiveZ:
                case SignedAxis::NegativeZ: value = input.z; break;
            }
            if (signedAxisSign (axis) < 0)
            {
                if (value == INT32_MIN)
                {
                    return false;
                }
                value = -value;
            }
            output = value;
            return true;
        }

        bool mapVector (const SourceAxisMapping& mapping,
                        const InertialVector& input,
                        InertialVector& output) noexcept
        {
            return mapAxis (mapping.x, input, output.x) &&
                   mapAxis (mapping.y, input, output.y) &&
                   mapAxis (mapping.z, input, output.z);
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
          accelerationSums_ {0, 0, 0},
          angularRateSums_ {0, 0, 0},
          initialized_ {false},
          active_ {false},
          shutdown_ {false},
          lifecycleGeneration_ (0)
    {
        clearEvidence (evidence_, config.qualificationRevision, 0,
                       config.sourceToQualificationFrame);
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
        clearEvidence (evidence_, config_.qualificationRevision,
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
        clearEvidence (evidence_, config_.qualificationRevision,
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

        if (record.schemaRevision != config_.expectedSchemaRevision ||
            record.normalizationRevision !=
                config_.expectedNormalizationRevision ||
            !sameSource (record.source, config_.expectedSource))
        {
            reject (evidence_,
                    InertialQualificationReason::ConfigurationMismatch, record);
            active_   = false;
            return {};
        }

        const uint32_t age =
            now.milliseconds () - record.observedAt.milliseconds ();
        if (age >= halfRange)
        {
            reject (evidence_,
                    InertialQualificationReason::TimestampDiscontinuity,
                    record);
            active_   = false;
            return {};
        }
        if (age > config_.maximumAge.milliseconds ())
        {
            reject (evidence_, InertialQualificationReason::Stale, record);
            active_   = false;
            return {};
        }

        if (evidence_.acceptedSampleCount != 0)
        {
            const uint32_t sequenceDelta =
                record.sequence - evidence_.lastSequence;
            if (sequenceDelta == 0)
            {
                if (sameRecord (record, evidence_.terminalRecord))
                {
                    return {};
                }
                reject (evidence_,
                        InertialQualificationReason::SequenceDiscontinuity,
                        record);
                active_   = false;
                return {};
            }
            if (sequenceDelta >= halfRange)
            {
                reject (evidence_,
                        InertialQualificationReason::SequenceDiscontinuity,
                        record);
                active_   = false;
                return {};
            }
            if (sequenceDelta != 1)
            {
                reject (evidence_,
                        InertialQualificationReason::SequenceDiscontinuity,
                        record);
                active_   = false;
                return {};
            }
            const uint32_t observedAt =
                record.observedAt.milliseconds ();
            const uint32_t lastObservedAt =
                evidence_.lastObservedAt.milliseconds ();
            const uint32_t observedGap = observedAt - lastObservedAt;
            if (observedGap == 0 || observedGap >= halfRange ||
                observedGap > config_.maximumGap.milliseconds ())
            {
                reject (evidence_,
                        InertialQualificationReason::TimestampDiscontinuity,
                        record);
                active_   = false;
                return {};
            }
            if (observedGap > evidence_.maximumObservedGap.milliseconds ())
            {
                evidence_.maximumObservedGap = Duration (observedGap);
            }
        }

        if (!record.producerStatus.ok () ||
            record.state == InertialRecordState::SourceFault)
        {
            reject (evidence_, InertialQualificationReason::ProducerFault,
                    record);
            active_   = false;
            return {};
        }
        if (!record.dataReady ||
            record.state == InertialRecordState::NotReady)
        {
            reject (evidence_, InertialQualificationReason::NotReady, record);
            active_   = false;
            return {};
        }
        if (record.saturation != InertialSaturation::None)
        {
            reject (evidence_, InertialQualificationReason::Saturated, record);
            active_   = false;
            return {};
        }

        InertialVector mappedAcceleration;
        InertialVector mappedAngularRate;
        if (!mapVector (config_.sourceToQualificationFrame,
                        record.accelerationMicroG,
                        mappedAcceleration) ||
            !mapVector (config_.sourceToQualificationFrame,
                        record.angularRateMilliDegreesPerSecond,
                        mappedAngularRate))
        {
            reject (evidence_,
                    InertialQualificationReason::ArithmeticOverflow,
                    record, StatusCode::InvalidArgument);
            active_   = false;
            return StatusCode::InvalidArgument;
        }
        if (!accelerationInWindow (config_, mappedAcceleration))
        {
            reject (
                evidence_,
                InertialQualificationReason::AccelerationOutsideWindow, record);
            evidence_.mappedRecord                    = record;
            evidence_.mappedRecord.accelerationMicroG = mappedAcceleration;
            evidence_.mappedRecord.angularRateMilliDegreesPerSecond =
                mappedAngularRate;
            active_                = false;
            return {};
        }
        if (!angularRateInWindow (
                config_, mappedAngularRate))
        {
            reject (
                evidence_,
                InertialQualificationReason::AngularRateOutsideWindow, record);
            evidence_.mappedRecord                    = record;
            evidence_.mappedRecord.accelerationMicroG = mappedAcceleration;
            evidence_.mappedRecord.angularRateMilliDegreesPerSecond =
                mappedAngularRate;
            active_                = false;
            return {};
        }

        if (evidence_.acceptedSampleCount == 0)
        {
            evidence_.firstSequence = record.sequence;
            evidence_.firstObservedAt = record.observedAt;
            evidence_.minimumAccelerationMicroG = mappedAcceleration;
            evidence_.maximumAccelerationMicroG = mappedAcceleration;
            evidence_.minimumAngularRateMilliDegreesPerSecond =
                mappedAngularRate;
            evidence_.maximumAngularRateMilliDegreesPerSecond =
                mappedAngularRate;
        }
        else
        {
            updateMinimum (evidence_.minimumAccelerationMicroG,
                           mappedAcceleration);
            updateMaximum (evidence_.maximumAccelerationMicroG,
                           mappedAcceleration);
            updateMinimum (
                evidence_.minimumAngularRateMilliDegreesPerSecond,
                mappedAngularRate);
            updateMaximum (
                evidence_.maximumAngularRateMilliDegreesPerSecond,
                mappedAngularRate);
        }
        accelerationSums_[0] += mappedAcceleration.x;
        accelerationSums_[1] += mappedAcceleration.y;
        accelerationSums_[2] += mappedAcceleration.z;
        angularRateSums_[0] += mappedAngularRate.x;
        angularRateSums_[1] += mappedAngularRate.y;
        angularRateSums_[2] += mappedAngularRate.z;
        evidence_.accelerationSumsMicroG = {
            accelerationSums_[0],
            accelerationSums_[1],
            accelerationSums_[2]};
        evidence_.angularRateSumsMilliDegreesPerSecond = {
            angularRateSums_[0],
            angularRateSums_[1],
            angularRateSums_[2]};
        ++evidence_.acceptedSampleCount;
        evidence_.lastSequence   = record.sequence;
        evidence_.lastObservedAt = record.observedAt;
        evidence_.terminalRecord = record;
        evidence_.mappedRecord                    = record;
        evidence_.mappedRecord.accelerationMicroG = mappedAcceleration;
        evidence_.mappedRecord.angularRateMilliDegreesPerSecond =
            mappedAngularRate;
        if (age > evidence_.maximumObservedAge.milliseconds ())
        {
            evidence_.maximumObservedAge = Duration (age);
        }
        evidence_.meanAccelerationMicroG = {
            mean (accelerationSums_[0], evidence_.acceptedSampleCount),
            mean (accelerationSums_[1], evidence_.acceptedSampleCount),
            mean (accelerationSums_[2], evidence_.acceptedSampleCount)};
        evidence_.meanAngularRateMilliDegreesPerSecond = {
            mean (angularRateSums_[0], evidence_.acceptedSampleCount),
            mean (angularRateSums_[1], evidence_.acceptedSampleCount),
            mean (angularRateSums_[2], evidence_.acceptedSampleCount)};
        if (evidence_.acceptedSampleCount == config_.requiredSampleCount)
        {
            evidence_.state  = InertialQualificationState::Qualified;
            evidence_.reason = InertialQualificationReason::None;
            active_          = false;
        }
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
        clearEvidence (evidence_, config_.qualificationRevision,
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
        clearEvidence (evidence_, config_.qualificationRevision,
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
