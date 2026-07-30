#include "qualified_motion_recorder.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);
        constexpr uint32_t hashBasis = UINT32_C (2166136261);
        constexpr uint32_t hashPrime = UINT32_C (16777619);

        void hashByte (uint32_t& hash, uint8_t value) noexcept
        {
            hash = (hash ^ value) * hashPrime;
        }

        void hash32 (uint32_t& hash, uint32_t value) noexcept
        {
            for (int8_t shift = 24; shift >= 0; shift -= 8)
            {
                hashByte (
                    hash, static_cast<uint8_t> (value >> shift));
            }
        }

        void hash64 (uint32_t& hash, uint64_t value) noexcept
        {
            for (int8_t shift = 56; shift >= 0; shift -= 8)
            {
                hashByte (
                    hash, static_cast<uint8_t> (value >> shift));
            }
        }

        void hashVector (uint32_t& hash,
                         const InertialVector& value) noexcept
        {
            hash32 (hash, static_cast<uint32_t> (value.x));
            hash32 (hash, static_cast<uint32_t> (value.y));
            hash32 (hash, static_cast<uint32_t> (value.z));
        }

        void hashWideVector (
            uint32_t& hash, const InertialWideVector& value) noexcept
        {
            hash64 (hash, static_cast<uint64_t> (value.x));
            hash64 (hash, static_cast<uint64_t> (value.y));
            hash64 (hash, static_cast<uint64_t> (value.z));
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
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

        bool validRecord (const InertialRecord& record) noexcept
        {
            uint8_t image[InertialRecordCodec::size] = {};
            return InertialRecordCodec ()
                .encode (record,
                         MutableByteSpan {
                             image, InertialRecordCodec::size})
                .ok ();
        }

        bool zeroVector (const InertialVector& value) noexcept
        {
            return value.x == 0 && value.y == 0 && value.z == 0;
        }

        bool withinRange (const InertialVector& value,
                          uint32_t range) noexcept
        {
            const int64_t limit = range;
            return value.x >= -limit && value.x <= limit &&
                   value.y >= -limit && value.y <= limit &&
                   value.z >= -limit && value.z <= limit;
        }

        int32_t component (const InertialVector& value,
                           uint8_t axis) noexcept
        {
            return axis == 0 ? value.x :
                   axis == 1 ? value.y :
                               value.z;
        }

        int64_t component (const InertialWideVector& value,
                           uint8_t axis) noexcept
        {
            return axis == 0 ? value.x :
                   axis == 1 ? value.y :
                               value.z;
        }

        bool validRecorderRecord (
            const InertialRecord& record) noexcept
        {
            if (!validStatus (record.producerStatus) ||
                record.state > InertialRecordState::SourceFault ||
                record.saturation > InertialSaturation::Both)
            {
                return false;
            }
            if (record.state == InertialRecordState::Recorded)
            {
                const bool producerHealthy =
                    record.producerStatus.ok ();
                return record.dataReady && producerHealthy &&
                       withinRange (
                           record.accelerationMicroG,
                           record.source.accelerationRangeMicroG) &&
                       withinRange (
                           record.angularRateMilliDegreesPerSecond,
                           record.source
                               .angularRateRangeMilliDegreesPerSecond);
            }
            const bool producerHealthy =
                record.producerStatus.ok ();
            return !record.dataReady &&
                   record.saturation == InertialSaturation::None &&
                   zeroVector (record.accelerationMicroG) &&
                   zeroVector (
                       record.angularRateMilliDegreesPerSecond) &&
                   (record.state == InertialRecordState::SourceFault ?
                        !producerHealthy :
                        producerHealthy);
        }

        bool sameRecord (const InertialRecord& left,
                         const InertialRecord& right) noexcept
        {
            uint8_t leftImage[InertialRecordCodec::size]  = {};
            uint8_t rightImage[InertialRecordCodec::size] = {};
            InertialRecordCodec codec;
            const Result<uint16_t> leftEncoded =
                codec.encode (left,
                              MutableByteSpan {
                                  leftImage, InertialRecordCodec::size});
            const Result<uint16_t> rightEncoded =
                codec.encode (right,
                              MutableByteSpan {
                                  rightImage, InertialRecordCodec::size});
            if (!leftEncoded.ok () || !rightEncoded.ok ())
            {
                return false;
            }
            for (uint8_t index = 0; index < InertialRecordCodec::size;
                 ++index)
            {
                if (leftImage[index] != rightImage[index])
                {
                    return false;
                }
            }
            return true;
        }

        bool mapRecord (const SourceAxisMapping& mapping,
                        const InertialRecord& input,
                        InertialRecord& output) noexcept;

        bool validConfiguration (const MotionRecorderConfig& config) noexcept
        {
            const uint32_t maximumAge =
                config.maximumRecordAge.milliseconds ();
            const uint32_t minimumDuration =
                config.minimumStepDuration.milliseconds ();
            return config.recordSchemaRevision != 0 &&
                   config.normalizationRevision != 0 &&
                   config.qualificationRevision != 0 &&
                   config.recorderRevision != 0 &&
                   config.maximumRecordCount != 0 &&
                   config.traceToken != 0 &&
                   maximumAge != 0 && maximumAge < halfRange &&
                   minimumDuration != 0 && minimumDuration < halfRange &&
                   config.expectedSource.kind ==
                       InertialSourceKind::SyntheticFixture &&
                   config.expectedSource.model == InertialModel::Synthetic &&
                   config.expectedSource.sourceId != 0 &&
                   validateOrientationConfig (config.orientation).ok ();
        }

        bool evidenceMatchesConfiguration (
            const MotionRecorderConfig&             config,
            const InertialQualificationEvidence& evidence) noexcept
        {
            const InertialRecord& terminal = evidence.terminalRecord;
            const InertialRecord& mapped   = evidence.mappedRecord;
            InertialRecord expectedMapped;
            if (!mapRecord (
                    evidence.sourceToQualificationFrame,
                    terminal, expectedMapped) ||
                !sameRecord (expectedMapped, mapped))
            {
                return false;
            }
            const uint32_t sequenceSpan =
                evidence.lastSequence - evidence.firstSequence;
            const uint32_t lastObservedAt =
                evidence.lastObservedAt.milliseconds ();
            const uint32_t firstObservedAt =
                evidence.firstObservedAt.milliseconds ();
            const uint32_t timeSpan =
                lastObservedAt - firstObservedAt;
            if (evidence.acceptedSampleCount < 2 ||
                sequenceSpan !=
                    static_cast<uint32_t> (
                        evidence.acceptedSampleCount - 1) ||
                timeSpan == 0 || timeSpan >= halfRange ||
                evidence.maximumObservedGap.milliseconds () == 0 ||
                evidence.maximumObservedGap.milliseconds () > timeSpan ||
                evidence.maximumObservedAge.milliseconds () >=
                    halfRange ||
                evidence.lastSequence != terminal.sequence ||
                evidence.lastObservedAt.milliseconds () !=
                    terminal.observedAt.milliseconds ())
            {
                return false;
            }
            for (uint8_t axis = 0; axis < 3; ++axis)
            {
                const int64_t count = evidence.acceptedSampleCount;
                const int32_t accelerationMinimum = component (
                    evidence.minimumAccelerationMicroG, axis);
                const int32_t accelerationMaximum = component (
                    evidence.maximumAccelerationMicroG, axis);
                const int32_t angularRateMinimum = component (
                    evidence.minimumAngularRateMilliDegreesPerSecond,
                    axis);
                const int32_t angularRateMaximum = component (
                    evidence.maximumAngularRateMilliDegreesPerSecond,
                    axis);
                const int64_t accelerationSum = component (
                    evidence.accelerationSumsMicroG, axis);
                const int64_t angularRateSum = component (
                    evidence.angularRateSumsMilliDegreesPerSecond,
                    axis);
                if (accelerationMinimum > accelerationMaximum ||
                    angularRateMinimum > angularRateMaximum ||
                    component (
                        evidence.meanAccelerationMicroG, axis) !=
                        accelerationSum / count ||
                    component (
                        evidence.meanAngularRateMilliDegreesPerSecond,
                        axis) != angularRateSum / count ||
                    accelerationSum < accelerationMinimum * count ||
                    accelerationSum > accelerationMaximum * count ||
                    angularRateSum < angularRateMinimum * count ||
                    angularRateSum > angularRateMaximum * count ||
                    component (mapped.accelerationMicroG, axis) <
                        accelerationMinimum ||
                    component (mapped.accelerationMicroG, axis) >
                        accelerationMaximum ||
                    component (
                        mapped.angularRateMilliDegreesPerSecond,
                        axis) < angularRateMinimum ||
                    component (
                        mapped.angularRateMilliDegreesPerSecond,
                        axis) > angularRateMaximum)
                {
                    return false;
                }
            }
            const bool terminalHealthy =
                terminal.producerStatus.ok ();
            const bool mappedHealthy =
                mapped.producerStatus.ok ();
            const bool statusesHealthy =
                terminalHealthy && mappedHealthy;
            return evidence.state == InertialQualificationState::Qualified &&
                   evidence.reason == InertialQualificationReason::None &&
                   evidence.status.ok () && evidence.attemptId != 0 &&
                   evidence.lifecycleGeneration != 0 &&
                   evidence.qualificationRevision ==
                       config.qualificationRevision &&
                   validSignedAxisMapping (
                       evidence.sourceToQualificationFrame) &&
                   validRecord (terminal) && validRecord (mapped) &&
                   terminal.state == InertialRecordState::Recorded &&
                   mapped.state == InertialRecordState::Recorded &&
                   terminal.schemaRevision ==
                       config.recordSchemaRevision &&
                   mapped.schemaRevision == config.recordSchemaRevision &&
                   terminal.normalizationRevision ==
                       config.normalizationRevision &&
                   mapped.normalizationRevision ==
                       config.normalizationRevision &&
                   sameSource (terminal.source, config.expectedSource) &&
                   sameSource (mapped.source, config.expectedSource) &&
                   terminal.sequence == mapped.sequence &&
                   terminal.observedAt.milliseconds () ==
                       mapped.observedAt.milliseconds () &&
                   terminal.dataReady && mapped.dataReady &&
                   terminal.saturation == InertialSaturation::None &&
                   mapped.saturation == InertialSaturation::None &&
                   statusesHealthy;
        }

        MotionDisplayToken tokenForStep (MotionScriptStep step) noexcept
        {
            switch (step)
            {
                case MotionScriptStep::Rest:
                    return MotionDisplayToken::HoldRest;
                case MotionScriptStep::TiltForward:
                    return MotionDisplayToken::TiltForward;
                case MotionScriptStep::TiltBack:
                    return MotionDisplayToken::TiltBack;
                case MotionScriptStep::TiltLeft:
                    return MotionDisplayToken::TiltLeft;
                case MotionScriptStep::TiltRight:
                    return MotionDisplayToken::TiltRight;
                case MotionScriptStep::ReturnToRest:
                    return MotionDisplayToken::ReturnToRest;
            }
            return MotionDisplayToken::Fault;
        }

        MotionPresentationIntent presentation (
            MotionDisplayToken token, MotionRecorderHealth health) noexcept
        {
            MotionPresentationIntent output {
                token, health, 0, 0, 0, false, 0, 0};
            switch (health)
            {
                case MotionRecorderHealth::Ready:
                    output.rgbGreen = 96;
                    break;
                case MotionRecorderHealth::Recording:
                    output.rgbBlue = 96;
                    break;
                case MotionRecorderHealth::Complete:
                    output.rgbGreen = 96;
                    output.rgbBlue  = 32;
                    break;
                case MotionRecorderHealth::Unknown:
                    output.rgbBlue = 24;
                    break;
                default:
                    output.rgbRed = 96;
                    break;
            }
            return output;
        }

        void put16 (uint8_t* output, uint16_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value >> 8);
            output[1] = static_cast<uint8_t> (value);
        }

        void put32 (uint8_t* output, uint32_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value >> 24);
            output[1] = static_cast<uint8_t> (value >> 16);
            output[2] = static_cast<uint8_t> (value >> 8);
            output[3] = static_cast<uint8_t> (value);
        }

        uint16_t get16 (const uint8_t* input) noexcept
        {
            return static_cast<uint16_t> (
                (static_cast<uint16_t> (input[0]) << 8) | input[1]);
        }

        uint32_t get32 (const uint8_t* input) noexcept
        {
            return (static_cast<uint32_t> (input[0]) << 24) |
                   (static_cast<uint32_t> (input[1]) << 16) |
                   (static_cast<uint32_t> (input[2]) << 8) |
                   input[3];
        }

        uint16_t crc16 (const uint8_t* bytes, uint16_t count) noexcept
        {
            uint16_t crc = UINT16_C (0xffff);
            for (uint16_t index = 0; index < count; ++index)
            {
                crc ^= static_cast<uint16_t> (
                    static_cast<uint16_t> (bytes[index]) << 8);
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & UINT16_C (0x8000)) != 0
                              ? static_cast<uint16_t> (
                                    (crc << 1) ^ UINT16_C (0x1021))
                              : static_cast<uint16_t> (crc << 1);
                }
            }
            return crc;
        }

        Status buildImage (
            const MotionRecorderConfig&             config,
            uint32_t                                lifecycleGeneration,
            uint32_t                                sessionId,
            uint16_t                                ordinal,
            MotionScriptStep                        step,
            MotionRecorderHealth                    health,
            const InertialQualificationEvidence& evidence,
            uint32_t                                qualificationDigest,
            const InertialRecord&                   record,
            const OrientationEstimate&              orientation,
            MotionRecordImage&                      output) noexcept
        {
            MotionRecordImage candidate {};
            for (uint16_t index = 0; index < MotionRecordImage::capacity;
                 ++index)
            {
                candidate.bytes[index] = 0;
            }
            candidate.bytes[0] = 'Q';
            candidate.bytes[1] = 'M';
            candidate.bytes[2] = 'R';
            candidate.bytes[3] = '1';
            candidate.bytes[4] = 1;
            candidate.bytes[5] =
                static_cast<uint8_t> (MotionRecordImage::capacity);
            put16 (&candidate.bytes[6], config.recorderRevision);
            put16 (&candidate.bytes[8], record.schemaRevision);
            put16 (&candidate.bytes[10], record.normalizationRevision);
            put16 (&candidate.bytes[12],
                   evidence.qualificationRevision);
            put32 (&candidate.bytes[14], lifecycleGeneration);
            put32 (&candidate.bytes[18], sessionId);
            put16 (&candidate.bytes[22], ordinal);
            candidate.bytes[24] = static_cast<uint8_t> (step);
            candidate.bytes[25] = static_cast<uint8_t> (health);
            candidate.bytes[26] =
                static_cast<uint8_t> (record.source.kind);
            candidate.bytes[27] =
                static_cast<uint8_t> (record.source.model);
            candidate.bytes[28] = record.source.sourceId;
            put16 (&candidate.bytes[29],
                   record.source.configurationRevision);
            put16 (&candidate.bytes[31],
                   record.source.calibrationRevision);
            put32 (&candidate.bytes[33],
                   record.source.accelerationRangeMicroG);
            put32 (
                &candidate.bytes[37],
                record.source.angularRateRangeMilliDegreesPerSecond);
            candidate.bytes[41] = static_cast<uint8_t> (
                evidence.sourceToQualificationFrame.x);
            candidate.bytes[42] = static_cast<uint8_t> (
                evidence.sourceToQualificationFrame.y);
            candidate.bytes[43] = static_cast<uint8_t> (
                evidence.sourceToQualificationFrame.z);
            put32 (&candidate.bytes[44],
                   evidence.lifecycleGeneration);
            put32 (&candidate.bytes[48], evidence.attemptId);
            put32 (&candidate.bytes[52], record.sequence);
            put32 (&candidate.bytes[56],
                   record.observedAt.milliseconds ());
            put32 (&candidate.bytes[60],
                   static_cast<uint32_t> (
                       record.accelerationMicroG.x));
            put32 (&candidate.bytes[64],
                   static_cast<uint32_t> (
                       record.accelerationMicroG.y));
            put32 (&candidate.bytes[68],
                   static_cast<uint32_t> (
                       record.accelerationMicroG.z));
            put32 (&candidate.bytes[72],
                   static_cast<uint32_t> (
                       record.angularRateMilliDegreesPerSecond.x));
            put32 (&candidate.bytes[76],
                   static_cast<uint32_t> (
                       record.angularRateMilliDegreesPerSecond.y));
            put32 (&candidate.bytes[80],
                   static_cast<uint32_t> (
                       record.angularRateMilliDegreesPerSecond.z));
            put32 (&candidate.bytes[84],
                   static_cast<uint32_t> (
                       orientation.pitchMilliDegrees));
            put32 (&candidate.bytes[88],
                   static_cast<uint32_t> (
                       orientation.rollMilliDegrees));
            candidate.bytes[92] =
                static_cast<uint8_t> (orientation.quality);
            candidate.bytes[93] =
                static_cast<uint8_t> (orientation.status.error ());
            candidate.bytes[94] =
                static_cast<uint8_t> (record.state);
            candidate.bytes[95] =
                static_cast<uint8_t> (record.saturation);
            candidate.bytes[96] = record.dataReady ? 1 : 0;
            candidate.bytes[97] =
                static_cast<uint8_t> (record.producerStatus.error ());
            put32 (&candidate.bytes[98], qualificationDigest);
            put32 (&candidate.bytes[102],
                   motionRecordDigest (record));
            put32 (&candidate.bytes[106], config.traceToken);
            put16 (&candidate.bytes[126],
                   crc16 (candidate.bytes,
                          MotionRecordImage::capacity - 2));
            output           = candidate;
            return {};
        }

        InertialObservation asObservation (
            const MotionRecorderConfig& config, TimePoint now,
            const InertialRecord& record) noexcept
        {
            InertialSample sample {
                record.source,
                record.accelerationMicroG,
                record.angularRateMilliDegreesPerSecond,
                record.observedAt,
                record.sequence,
                record.dataReady,
                record.saturation,
                record.producerStatus};
            return {sample,
                    InertialSampleQuality::Current,
                    record.dataReady,
                    Duration (now.milliseconds () -
                              record.observedAt.milliseconds ()),
                    config.maximumRecordAge,
                    config.normalizationRevision,
                    0,
                    record.producerStatus};
        }

        int16_t tenthsDegree (int32_t milliDegrees) noexcept
        {
            const int32_t value = milliDegrees / 100;
            if (value > INT16_MAX)
            {
                return INT16_MAX;
            }
            if (value < INT16_MIN)
            {
                return INT16_MIN;
            }
            return static_cast<int16_t> (value);
        }

        InertialRecord emptyRecord () noexcept
        {
            return {0,
                    0,
                    {},
                    {0, 0, 0},
                    {0, 0, 0},
                    TimePoint (),
                    0,
                    false,
                    InertialSaturation::None,
                    Status (),
                    InertialRecordState::NotReady};
        }

        InertialQualificationEvidence emptyQualification () noexcept
        {
            return {0,
                    0,
                    0,
                    {SignedAxis::PositiveX, SignedAxis::PositiveY,
                     SignedAxis::PositiveZ},
                    InertialQualificationState::Idle,
                    InertialQualificationReason::None,
                    0,
                    0,
                    0,
                    TimePoint (),

                    TimePoint (),

                    Duration (),

                    Duration (),
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    emptyRecord (),

                    emptyRecord (),

                    Status ()};
        }

        MotionRecorderResult emptyResult () noexcept
        {
            return {0,
                    0,
                    MotionRecorderMode::Inert,
                    MotionRecorderHealth::Unknown,
                    MotionScriptStep::Rest,
                    0,
                    0,
                    emptyRecord (),

                    emptyQualification (),
                    {MotionDisplayToken::QualifySource,
                     MotionRecorderHealth::Unknown,
                     0,
                     0,
                     0,
                     false,
                     0,
                     0},
                    false,
                    Status ()};
        }

        MotionRecorderControl emptyControl () noexcept
        {
            return {0,
                    0,
                    TimePoint (),
                    0,
                    0,
                    0,
                    0,
                    0,
                    MotionRecorderCommand::None,
                    Status ()};
        }

        void clearVector (InertialVector& value) noexcept
        {
            value.x = 0;
            value.y = 0;
            value.z = 0;
        }

        void clearRecord (InertialRecord& record) noexcept
        {
            record.schemaRevision        = 0;
            record.normalizationRevision = 0;
            record.source                = {};
            clearVector (record.accelerationMicroG);
            clearVector (
                record.angularRateMilliDegreesPerSecond);
            record.observedAt      = TimePoint ();
            record.sequence        = 0;
            record.dataReady       = false;
            record.saturation      = InertialSaturation::None;
            record.producerStatus  = Status ();
            record.state           = InertialRecordState::NotReady;
        }

        void clearQualification (
            InertialQualificationEvidence& evidence) noexcept
        {
            evidence.attemptId             = 0;
            evidence.lifecycleGeneration   = 0;
            evidence.qualificationRevision = 0;
            evidence.sourceToQualificationFrame = {
                SignedAxis::PositiveX, SignedAxis::PositiveY,
                SignedAxis::PositiveZ};
            evidence.state  = InertialQualificationState::Idle;
            evidence.reason = InertialQualificationReason::None;
            evidence.acceptedSampleCount = 0;
            evidence.firstSequence       = 0;
            evidence.lastSequence        = 0;
            evidence.firstObservedAt     = TimePoint ();

            evidence.lastObservedAt      = TimePoint ();

            evidence.maximumObservedAge  = Duration ();

            evidence.maximumObservedGap  = Duration ();

            clearVector (evidence.meanAccelerationMicroG);

            clearVector (
                evidence.meanAngularRateMilliDegreesPerSecond);

            clearVector (evidence.minimumAccelerationMicroG);

            clearVector (evidence.maximumAccelerationMicroG);

            clearVector (
                evidence.minimumAngularRateMilliDegreesPerSecond);

            clearVector (
                evidence.maximumAngularRateMilliDegreesPerSecond);
            evidence.accelerationSumsMicroG = {0, 0, 0};
            evidence.angularRateSumsMilliDegreesPerSecond =
                {0, 0, 0};
            clearRecord (evidence.terminalRecord);

            clearRecord (evidence.mappedRecord);

            evidence.status = Status ();
        }

        void clearResult (MotionRecorderResult& result) noexcept
        {
            result.sessionId           = 0;
            result.lifecycleGeneration = 0;
            result.mode                = MotionRecorderMode::Inert;
            result.health              = MotionRecorderHealth::Unknown;
            result.scriptStep          = MotionScriptStep::Rest;
            result.recordCount         = 0;
            result.recordCapacity      = 0;
            clearRecord (result.latestRecord);

            clearQualification (result.qualification);
            result.presentation = {
                MotionDisplayToken::QualifySource,
                MotionRecorderHealth::Unknown,
                0,
                0,
                0,
                false,
                0,
                0};
            result.exportRequested = false;
            result.status          = Status ();
        }

        void clearControl (MotionRecorderControl& control) noexcept
        {
            control.sourceId = 0;
            control.sequence = 0;
            control.observedAt = TimePoint ();
            control.qualificationRevision = 0;
            control.qualificationLifecycleGeneration = 0;
            control.qualificationAttemptId = 0;
            control.qualificationDigest = 0;
            control.traceToken = 0;
            control.command    = MotionRecorderCommand::None;
            control.status     = Status ();
        }

        bool sameControl (const MotionRecorderControl& left,
                          const MotionRecorderControl& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.sequence == right.sequence &&
                   left.observedAt.milliseconds () ==
                       right.observedAt.milliseconds () &&
                   left.qualificationRevision ==
                       right.qualificationRevision &&
                   left.qualificationLifecycleGeneration ==
                       right.qualificationLifecycleGeneration &&
                   left.qualificationAttemptId ==
                       right.qualificationAttemptId &&
                   left.qualificationDigest ==
                       right.qualificationDigest &&
                   left.traceToken == right.traceToken &&
                   left.command == right.command &&
                   left.status == right.status;
        }

        bool mapRecord (const SourceAxisMapping& mapping,
                        const InertialRecord& input,
                        InertialRecord& output) noexcept
        {
            output = input;
            return mapSignedAxes (
                       mapping, input.accelerationMicroG.x,
                       input.accelerationMicroG.y,
                       input.accelerationMicroG.z,
                       output.accelerationMicroG.x,
                       output.accelerationMicroG.y,
                       output.accelerationMicroG.z) &&
                   mapSignedAxes (
                       mapping,
                       input.angularRateMilliDegreesPerSecond.x,
                       input.angularRateMilliDegreesPerSecond.y,
                       input.angularRateMilliDegreesPerSecond.z,
                       output.angularRateMilliDegreesPerSecond.x,
                       output.angularRateMilliDegreesPerSecond.y,
                       output.angularRateMilliDegreesPerSecond.z);
        }
    } // namespace

    uint32_t motionRecordDigest (const InertialRecord& record) noexcept
    {
        uint32_t hash = hashBasis;
        hash32 (hash, record.schemaRevision);
        hash32 (hash, record.normalizationRevision);

        hashByte (hash, static_cast<uint8_t> (record.source.kind));
        hashByte (hash, static_cast<uint8_t> (record.source.model));
        hashByte (hash, record.source.sourceId);

        hash32 (hash, record.source.configurationRevision);
        hash32 (hash, record.source.calibrationRevision);
        hash32 (hash, record.source.accelerationRangeMicroG);
        hash32 (
            hash,
            record.source.angularRateRangeMilliDegreesPerSecond);
        hashVector (hash, record.accelerationMicroG);
        hashVector (
            hash, record.angularRateMilliDegreesPerSecond);

        hash32 (hash, record.observedAt.milliseconds ());
        hash32 (hash, record.sequence);

        hashByte (hash, record.dataReady ? 1 : 0);
        hashByte (hash, static_cast<uint8_t> (record.saturation));
        hashByte (
            hash,
            static_cast<uint8_t> (record.producerStatus.error ()));
        hashByte (hash, static_cast<uint8_t> (record.state));
        return hash == 0 ? 1 : hash;
    }

    uint32_t motionQualificationDigest (
        const InertialQualificationEvidence& evidence) noexcept
    {
        uint32_t hash = hashBasis;
        const uint32_t firstObservedAt =
            evidence.firstObservedAt.milliseconds ();
        const uint32_t lastObservedAt =
            evidence.lastObservedAt.milliseconds ();
        const uint32_t maximumObservedAge =
            evidence.maximumObservedAge.milliseconds ();
        const uint32_t maximumObservedGap =
            evidence.maximumObservedGap.milliseconds ();
        hash32 (hash, evidence.attemptId);
        hash32 (hash, evidence.lifecycleGeneration);
        hash32 (hash, evidence.qualificationRevision);

        hashByte (
            hash,
            static_cast<uint8_t> (
                evidence.sourceToQualificationFrame.x));
        hashByte (
            hash,
            static_cast<uint8_t> (
                evidence.sourceToQualificationFrame.y));
        hashByte (
            hash,
            static_cast<uint8_t> (
                evidence.sourceToQualificationFrame.z));
        hashByte (hash, static_cast<uint8_t> (evidence.state));
        hashByte (hash, static_cast<uint8_t> (evidence.reason));
        hashByte (hash, evidence.acceptedSampleCount);

        hash32 (hash, evidence.firstSequence);
        hash32 (hash, evidence.lastSequence);
        hash32 (hash, firstObservedAt);
        hash32 (hash, lastObservedAt);
        hash32 (hash, maximumObservedAge);
        hash32 (hash, maximumObservedGap);

        hashVector (hash, evidence.meanAccelerationMicroG);
        hashVector (
            hash, evidence.meanAngularRateMilliDegreesPerSecond);
        hashVector (hash, evidence.minimumAccelerationMicroG);
        hashVector (hash, evidence.maximumAccelerationMicroG);
        hashVector (
            hash,
            evidence.minimumAngularRateMilliDegreesPerSecond);
        hashVector (
            hash,
            evidence.maximumAngularRateMilliDegreesPerSecond);
        hashWideVector (hash, evidence.accelerationSumsMicroG);
        hashWideVector (
            hash, evidence.angularRateSumsMilliDegreesPerSecond);
        hash32 (hash, motionRecordDigest (evidence.terminalRecord));

        hash32 (hash, motionRecordDigest (evidence.mappedRecord));

        hashByte (hash, static_cast<uint8_t> (evidence.status.error ()));
        return hash == 0 ? 1 : hash;
    }

    MotionRecordValidity MotionRecordCodec::decode (
        const MotionRecordImage& image,
        DecodedMotionRecord& output) const noexcept
    {
        if (image.bytes[0] != 'Q' || image.bytes[1] != 'M' ||
            image.bytes[2] != 'R' || image.bytes[3] != '1' ||
            image.bytes[4] != version)
        {
            return MotionRecordValidity::BadFraming;
        }
        if (image.bytes[5] != MotionRecordImage::capacity)
        {
            return MotionRecordValidity::BadLength;
        }
        if (get16 (&image.bytes[126]) !=
            crc16 (image.bytes, MotionRecordImage::capacity - 2))
        {
            return MotionRecordValidity::BadIntegrity;
        }
        for (uint8_t index = 110; index < 126; ++index)
        {
            if (image.bytes[index] != 0)
            {
                return MotionRecordValidity::BadSemanticValue;
            }
        }

        DecodedMotionRecord candidate;
        candidate.recorderRevision = get16 (&image.bytes[6]);

        candidate.lifecycleGeneration = get32 (&image.bytes[14]);

        candidate.sessionId            = get32 (&image.bytes[18]);

        candidate.ordinal              = get16 (&image.bytes[22]);
        candidate.scriptStep =
            static_cast<MotionScriptStep> (image.bytes[24]);
        candidate.health =
            static_cast<MotionRecorderHealth> (image.bytes[25]);
        candidate.qualificationRevision = get16 (&image.bytes[12]);
        candidate.qualificationLifecycleGeneration =
            get32 (&image.bytes[44]);
        candidate.qualificationAttemptId = get32 (&image.bytes[48]);

        candidate.qualificationDigest = get32 (&image.bytes[98]);

        candidate.recordDigest        = get32 (&image.bytes[102]);

        candidate.traceToken          = get32 (&image.bytes[106]);
        candidate.sourceToQualificationFrame = {
            static_cast<SignedAxis> (image.bytes[41]),
            static_cast<SignedAxis> (image.bytes[42]),
            static_cast<SignedAxis> (image.bytes[43])};

        InertialSource source {
            static_cast<InertialSourceKind> (image.bytes[26]),
            static_cast<InertialModel> (image.bytes[27]),
            image.bytes[28],
            get16 (&image.bytes[29]),
            get16 (&image.bytes[31]),
            get32 (&image.bytes[33]),
            get32 (&image.bytes[37])};
        candidate.mappedRecord = {
            get16 (&image.bytes[8]),
            get16 (&image.bytes[10]),
            source,
            {static_cast<int32_t> (get32 (&image.bytes[60])),
             static_cast<int32_t> (get32 (&image.bytes[64])),
             static_cast<int32_t> (get32 (&image.bytes[68]))},
            {static_cast<int32_t> (get32 (&image.bytes[72])),
             static_cast<int32_t> (get32 (&image.bytes[76])),
             static_cast<int32_t> (get32 (&image.bytes[80]))},
            TimePoint (get32 (&image.bytes[56])),

            get32 (&image.bytes[52]),
            image.bytes[96] != 0,
            static_cast<InertialSaturation> (image.bytes[95]),
            static_cast<StatusCode> (image.bytes[97]),
            static_cast<InertialRecordState> (image.bytes[94])};
        candidate.orientation = {
            static_cast<int32_t> (get32 (&image.bytes[84])),
            static_cast<int32_t> (get32 (&image.bytes[88])),
            static_cast<OrientationQuality> (image.bytes[92]),
            static_cast<StatusCode> (image.bytes[93])};

        if (candidate.recorderRevision == 0 ||
            candidate.lifecycleGeneration == 0 ||
            candidate.sessionId == 0 ||
            candidate.scriptStep > MotionScriptStep::ReturnToRest ||
            candidate.health > MotionRecorderHealth::CapacityExhausted ||
            candidate.qualificationRevision == 0 ||
            candidate.qualificationLifecycleGeneration == 0 ||
            candidate.qualificationAttemptId == 0 ||
            candidate.qualificationDigest == 0 ||
            candidate.recordDigest == 0 ||
            candidate.traceToken == 0 ||
            !validSignedAxisMapping (
                candidate.sourceToQualificationFrame) ||
            image.bytes[96] > 1 ||
            !validRecord (candidate.mappedRecord) ||
            candidate.mappedRecord.state !=
                InertialRecordState::Recorded ||
            candidate.recordDigest !=
                motionRecordDigest (candidate.mappedRecord) ||
            candidate.health != MotionRecorderHealth::Recording ||
            candidate.mappedRecord.saturation !=
                InertialSaturation::None ||
            !candidate.mappedRecord.dataReady ||
            !candidate.mappedRecord.producerStatus.ok () ||
            candidate.orientation.quality >
                OrientationQuality::BeyondPresentationRange ||
            candidate.orientation.quality == OrientationQuality::Invalid ||
            !candidate.orientation.status.ok ())
        {
            return MotionRecordValidity::BadSemanticValue;
        }
        if (!validStatus (candidate.orientation.status))
        {
            return MotionRecordValidity::BadSemanticValue;
        }
        output = candidate;
        return MotionRecordValidity::Valid;
    }

    QualifiedMotionRecorder::QualifiedMotionRecorder (
        const MotionRecorderConfig& config) noexcept
        : config_ (config),
          recordCapacity_ (0),

          result_ (emptyResult ()),

          orientation_ (config.orientation),

          stepStartedAt_ (),

          lastSessionId_ (0),

          lastRecordSequence_ (0),

          lastRecordObservedAt_ (),

          lastAcceptedControl_ (emptyControl ()),

          qualificationDigest_ (0),

          lifecycleGeneration_ (0),

          initialized_ (false),

          shutdown_ (false),

          hasRecord_ (false)
    {
        result_.mode         = MotionRecorderMode::Inert;
        result_.health       = MotionRecorderHealth::Unknown;
        result_.scriptStep   = MotionScriptStep::Rest;
        result_.presentation = presentation (
            MotionDisplayToken::QualifySource,
            MotionRecorderHealth::Unknown);
    }

    Status QualifiedMotionRecorder::initialize (
        TimePoint now, uint16_t recordCapacity) noexcept
    {
        if (initialized_ || shutdown_)
        {
            return StatusCode::InvalidArgument;
        }
        if (!validConfiguration (config_))
        {
            return StatusCode::InvalidConfiguration;
        }
        if (recordCapacity == 0 ||
            recordCapacity != config_.maximumRecordCount)
        {
            return StatusCode::InvalidArgument;
        }
        if (lifecycleGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        const Status orientationStatus =
            orientation_.initialize ();
        if (!orientationStatus.ok ())
        {
            return orientationStatus;
        }
        recordCapacity_ = config_.maximumRecordCount;
        ++lifecycleGeneration_;
        initialized_           = true;
        result_.mode           = MotionRecorderMode::AwaitingQualification;
        result_.health         = MotionRecorderHealth::Unknown;
        result_.recordCapacity = recordCapacity_;
        result_.lifecycleGeneration = lifecycleGeneration_;
        result_.presentation   = presentation (
            MotionDisplayToken::QualifySource,
            MotionRecorderHealth::Unknown);
        result_.status = {};
        stepStartedAt_ = now;
        return {};
    }

    Status QualifiedMotionRecorder::qualify (
        TimePoint now,
        const InertialQualificationEvidence& evidence) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (result_.mode != MotionRecorderMode::AwaitingQualification)
        {
            return StatusCode::InvalidArgument;
        }
        if (!evidenceMatchesConfiguration (config_, evidence))
        {
            return StatusCode::InvalidArgument;
        }
        const uint32_t observedAt =
            evidence.terminalRecord.observedAt.milliseconds ();
        const uint32_t age = now.milliseconds () - observedAt;
        if (age >= halfRange ||
            age > config_.maximumRecordAge.milliseconds ())
        {
            return StatusCode::InvalidArgument;
        }
        result_.qualification     = evidence;
        qualificationDigest_ =
            motionQualificationDigest (evidence);
        result_.latestRecord      = evidence.terminalRecord;
        result_.mode              = MotionRecorderMode::Ready;
        result_.health            = MotionRecorderHealth::Ready;
        result_.presentation      = presentation (
            MotionDisplayToken::ReadyToRecord,
            MotionRecorderHealth::Ready);
        result_.status = {};
        return {};
    }

    Status QualifiedMotionRecorder::begin (
        TimePoint now, uint32_t sessionId) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        const uint32_t sessionDelta = sessionId - lastSessionId_;
        if (result_.mode != MotionRecorderMode::Ready || sessionId == 0 ||
            (lastSessionId_ != 0 &&
             (sessionDelta == 0 || sessionDelta >= halfRange)))
        {
            return StatusCode::InvalidArgument;
        }
        result_.sessionId       = sessionId;
        lastSessionId_          = sessionId;
        result_.mode            = MotionRecorderMode::Recording;
        result_.health          = MotionRecorderHealth::Recording;
        result_.scriptStep      = MotionScriptStep::Rest;
        result_.recordCount     = 0;
        result_.exportRequested = false;
        result_.presentation    = presentation (
            MotionDisplayToken::HoldRest,
            MotionRecorderHealth::Recording);
        result_.status      = {};
        stepStartedAt_      = now;
        hasRecord_          = false;
        lastAcceptedControl_ = emptyControl ();

        orientation_.reset ();
        return {};
    }

    Status QualifiedMotionRecorder::update (
        TimePoint now, const InertialRecord& record,
        const MotionRecorderControl& control,
        MotionRecordImage* records, uint16_t recordCapacity) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (control.command == MotionRecorderCommand::Reset)
        {
            return resetState (now);
        }
        if (result_.mode != MotionRecorderMode::Recording)
        {
            return StatusCode::InvalidArgument;
        }
        if (records == nullptr || recordCapacity != recordCapacity_ ||
            control.traceToken != config_.traceToken)
        {
            return StatusCode::InvalidArgument;
        }
        if (!validStatus (control.status) || !control.status.ok () ||
            control.sourceId != record.source.sourceId ||
            control.sequence != record.sequence ||
            control.observedAt.milliseconds () !=
                record.observedAt.milliseconds () ||
            control.qualificationRevision !=
                result_.qualification.qualificationRevision ||
            control.qualificationLifecycleGeneration !=
                result_.qualification.lifecycleGeneration ||
            control.qualificationAttemptId !=
                result_.qualification.attemptId ||
            control.qualificationDigest != qualificationDigest_ ||
            control.command > MotionRecorderCommand::AcknowledgeExport ||
            !validRecorderRecord (record))
        {
            return StatusCode::InvalidArgument;
        }
        if (hasRecord_ && record.sequence == lastRecordSequence_)
        {
            return sameRecord (record, result_.latestRecord) &&
                           sameControl (control, lastAcceptedControl_) ?
                       Status () :
                       Status (StatusCode::InvalidArgument);
        }
        if (control.command ==
                MotionRecorderCommand::AcknowledgeExport &&
            !result_.exportRequested)
        {
            return StatusCode::InvalidArgument;
        }
        if (record.schemaRevision != config_.recordSchemaRevision ||
            record.normalizationRevision !=
                config_.normalizationRevision ||
            !sameSource (record.source, config_.expectedSource))
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t age =
            now.milliseconds () - record.observedAt.milliseconds ();
        if (age >= halfRange ||
            age > config_.maximumRecordAge.milliseconds ())
        {
            result_.latestRecord = record;
            result_.status       = StatusCode::Timeout;
            result_.mode         = MotionRecorderMode::Fault;
            result_.health       = MotionRecorderHealth::Stale;
            result_.presentation = presentation (
                MotionDisplayToken::Fault,
                MotionRecorderHealth::Stale);
            return result_.status;
        }
        if (!record.producerStatus.ok () ||
            record.state == InertialRecordState::SourceFault ||
            !record.dataReady)
        {
            result_.latestRecord = record;
            const bool producerHealthy =
                record.producerStatus.ok ();
            result_.status = producerHealthy ?
                                 Status (StatusCode::HardwareFailure) :
                                 record.producerStatus;
            result_.mode         = MotionRecorderMode::Fault;
            result_.health       = MotionRecorderHealth::SourceFault;
            result_.presentation = presentation (
                MotionDisplayToken::Fault,
                MotionRecorderHealth::SourceFault);
            return result_.status;
        }
        if (record.saturation != InertialSaturation::None)
        {
            result_.latestRecord = record;
            result_.status       = StatusCode::InvalidArgument;
            result_.mode         = MotionRecorderMode::Fault;
            result_.health       = MotionRecorderHealth::Saturated;
            result_.presentation = presentation (
                MotionDisplayToken::Fault,
                MotionRecorderHealth::Saturated);
            return result_.status;
        }
        if (hasRecord_)
        {
            const uint32_t delta = record.sequence - lastRecordSequence_;
            const uint32_t observedAt =
                record.observedAt.milliseconds ();
            const uint32_t lastObservedAt =
                lastRecordObservedAt_.milliseconds ();
            const uint32_t timeDelta = observedAt - lastObservedAt;
            if (delta >= halfRange || timeDelta == 0 ||
                timeDelta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
        }
        else
        {
            const uint32_t sequenceDelta =
                record.sequence -
                result_.qualification.terminalRecord.sequence;
            const uint32_t timeDelta =
                record.observedAt.milliseconds () -
                result_.qualification.terminalRecord.observedAt
                    .milliseconds ();
            if (sequenceDelta == 0 || sequenceDelta >= halfRange ||
                timeDelta == 0 || timeDelta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
        }
        if (result_.recordCount >= recordCapacity_)
        {
            result_.latestRecord = record;
            result_.status       = StatusCode::CapacityExceeded;
            result_.mode         = MotionRecorderMode::Fault;
            result_.health       =
                MotionRecorderHealth::CapacityExhausted;
            result_.presentation = presentation (
                MotionDisplayToken::Fault,
                MotionRecorderHealth::CapacityExhausted);
            return result_.status;
        }

        InertialRecord mappedRecord;
        if (!mapRecord (
                result_.qualification.sourceToQualificationFrame,
                record, mappedRecord))
        {
            return StatusCode::InvalidArgument;
        }
        PreparedOrientationEstimate prepared;
        const Status orientationStatus = orientation_.preview (
            asObservation (config_, now, mappedRecord), prepared);
        if (!orientationStatus.ok () ||
            !orientation_.canCommit (prepared))
        {
            return orientationStatus.ok () ? StatusCode::InternalInvariant :
                                             orientationStatus;
        }

        if (control.command == MotionRecorderCommand::Advance &&
            now.milliseconds () - stepStartedAt_.milliseconds () <
                config_.minimumStepDuration.milliseconds ())
        {
            return StatusCode::InvalidArgument;
        }
        MotionRecordImage candidate;
        const Status imageStatus = buildImage (
            config_, lifecycleGeneration_, result_.sessionId,
            result_.recordCount,
            result_.scriptStep, MotionRecorderHealth::Recording,
            result_.qualification, qualificationDigest_,
            mappedRecord, prepared.result (), candidate);
        if (!imageStatus.ok ())
        {
            return imageStatus;
        }

        const Status commitStatus =
            orientation_.commit (prepared);
        if (!commitStatus.ok ())
        {
            return commitStatus;
        }
        records[result_.recordCount] = candidate;
        ++result_.recordCount;
        result_.latestRecord       = record;
        lastRecordSequence_        = record.sequence;
        lastRecordObservedAt_      = record.observedAt;
        lastAcceptedControl_       = control;
        hasRecord_                 = true;
        result_.presentation       = presentation (
            tokenForStep (result_.scriptStep),
            MotionRecorderHealth::Recording);
        result_.presentation.orientationValid =
            prepared.result ().quality == OrientationQuality::Level ||
            prepared.result ().quality == OrientationQuality::Tilted;
        result_.presentation.pitchTenthsDegree =
            tenthsDegree (prepared.result ().pitchMilliDegrees);
        result_.presentation.rollTenthsDegree =
            tenthsDegree (prepared.result ().rollMilliDegrees);

        if (control.command == MotionRecorderCommand::Advance)
        {
            if (result_.scriptStep != MotionScriptStep::ReturnToRest)
            {
                result_.scriptStep = static_cast<MotionScriptStep> (
                    static_cast<uint8_t> (result_.scriptStep) + 1);
                stepStartedAt_     = now;
                result_.presentation.token =
                    tokenForStep (result_.scriptStep);
            }
            else
            {
                result_.mode   = MotionRecorderMode::Complete;
                result_.health = MotionRecorderHealth::Complete;
                result_.presentation = presentation (
                    MotionDisplayToken::RecordingComplete,
                    MotionRecorderHealth::Complete);
            }
        }
        if (control.command == MotionRecorderCommand::RequestExport)
        {
            result_.exportRequested = true;
        }
        else if (control.command ==
                 MotionRecorderCommand::AcknowledgeExport)
        {
            result_.exportRequested = false;
        }
        result_.status     = {};
        return {};
    }

    Status QualifiedMotionRecorder::acknowledgeExport (TimePoint) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (!result_.exportRequested)
        {
            return StatusCode::InvalidArgument;
        }
        result_.exportRequested = false;
        return {};
    }

    Status QualifiedMotionRecorder::reset (TimePoint now) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        return resetState (now);
    }

    Status QualifiedMotionRecorder::resetState (TimePoint now) noexcept
    {
        if (lifecycleGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        ++lifecycleGeneration_;
        clearResult (result_);
        result_.mode           = MotionRecorderMode::AwaitingQualification;
        result_.health         = MotionRecorderHealth::Unknown;
        result_.scriptStep     = MotionScriptStep::Rest;
        result_.recordCapacity = recordCapacity_;
        result_.lifecycleGeneration = lifecycleGeneration_;
        result_.presentation   = presentation (
            MotionDisplayToken::QualifySource,
            MotionRecorderHealth::Unknown);
        orientation_.reset ();
        stepStartedAt_     = now;
        hasRecord_         = false;
        lastRecordObservedAt_ = TimePoint ();

        clearControl (lastAcceptedControl_);
        qualificationDigest_  = 0;
        return {};
    }

    Status QualifiedMotionRecorder::shutdown (TimePoint) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (shutdown_)
        {
            return {};
        }
        shutdown_            = true;
        result_.mode         = MotionRecorderMode::Shutdown;
        result_.health       = MotionRecorderHealth::Unknown;
        result_.presentation = presentation (
            MotionDisplayToken::QualifySource,
            MotionRecorderHealth::Unknown);
        result_.exportRequested = false;
        return {};
    }

    Status QualifiedMotionRecorder::result (
        MotionRecorderResult& output) const noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        output = result_;
        return {};
    }

    bool QualifiedMotionRecorder::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
