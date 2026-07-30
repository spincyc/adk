#pragma once

#include "inertial_record.h"
#include "signed_axis_mapping.h"

#include <stdint.h>

namespace adk {

    using SourceAxisMapping = SignedAxisMapping;

    enum struct InertialQualificationState : uint8_t
    {
        Idle,
        Collecting,
        Qualified,
        Rejected
    };

    enum struct InertialQualificationReason : uint8_t
    {
        None,
        ConfigurationMismatch,
        ProducerFault,
        NotReady,
        Saturated,
        Stale,
        SequenceDiscontinuity,
        TimestampDiscontinuity,
        AccelerationOutsideWindow,
        AngularRateOutsideWindow,
        ArithmeticOverflow,
        Disordered  = SequenceDiscontinuity,
        SequenceGap = SequenceDiscontinuity
    };

    struct InertialRecordQualificationConfig
    {
        uint16_t          qualificationRevision;
        uint16_t          expectedSchemaRevision;
        uint16_t          expectedNormalizationRevision;
        InertialSource    expectedSource;
        SourceAxisMapping sourceToQualificationFrame;
        uint8_t           requiredSampleCount;
        Duration          maximumAge;
        Duration          maximumGap;
        InertialVector    expectedStationaryAccelerationMicroG;
        InertialVector    maximumAccelerationDeviationMicroG;
        InertialVector    maximumAngularRateMilliDegreesPerSecond;
    };

    struct InertialWideVector
    {
        int64_t x;
        int64_t y;
        int64_t z;
    };

    struct InertialQualificationEvidence
    {
        uint32_t                    attemptId;
        uint32_t                    lifecycleGeneration;
        uint16_t                    qualificationRevision;
        SourceAxisMapping           sourceToQualificationFrame;
        InertialQualificationState  state;
        InertialQualificationReason reason;
        uint8_t                     acceptedSampleCount;
        uint32_t                    firstSequence;
        uint32_t                    lastSequence;
        TimePoint                   firstObservedAt;
        TimePoint                   lastObservedAt;
        Duration                    maximumObservedAge;
        Duration                    maximumObservedGap;
        InertialVector              meanAccelerationMicroG;
        InertialVector              meanAngularRateMilliDegreesPerSecond;
        InertialVector              minimumAccelerationMicroG;
        InertialVector              maximumAccelerationMicroG;
        InertialVector              minimumAngularRateMilliDegreesPerSecond;
        InertialVector              maximumAngularRateMilliDegreesPerSecond;
        InertialWideVector          accelerationSumsMicroG;
        InertialWideVector          angularRateSumsMilliDegreesPerSecond;
        InertialRecord              terminalRecord;
        InertialRecord              mappedRecord;
        Status                      status;
    };

    struct InertialRecordQualificationPolicy
    {
        explicit InertialRecordQualificationPolicy (
            const InertialRecordQualificationConfig& config) noexcept;

        InertialRecordQualificationPolicy (
            const InertialRecordQualificationPolicy&) = delete;
        InertialRecordQualificationPolicy&
        operator= (const InertialRecordQualificationPolicy&) = delete;
        InertialRecordQualificationPolicy (
            InertialRecordQualificationPolicy&&) = delete;
        InertialRecordQualificationPolicy&
        operator= (InertialRecordQualificationPolicy&&) = delete;

        Status initialize (TimePoint now) noexcept;
        Status begin      (TimePoint now, uint32_t attemptId) noexcept;
        Status observe    (TimePoint now, const InertialRecord& record) noexcept;
        Status reset      (TimePoint now) noexcept;
        Status shutdown   (TimePoint now) noexcept;
        Status evidence   (InertialQualificationEvidence& output) const noexcept;

        bool initialized () const noexcept;

      private:
        InertialRecordQualificationConfig config_;
        InertialQualificationEvidence     evidence_;
        int64_t                           accelerationSums_[3];
        int64_t                           angularRateSums_[3];
        bool                              initialized_;
        bool                              active_;
        bool                              shutdown_;
        uint32_t                          lifecycleGeneration_;
    };
} // namespace adk
