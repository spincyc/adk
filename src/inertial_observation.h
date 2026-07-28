#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct InertialSourceKind : uint8_t
    {
        SyntheticFixture,
        Mpu6050Adapter,
        Qmi8658Adapter
    };

    enum struct InertialModel : uint8_t
    {
        Synthetic,
        Mpu6050,
        Qmi8658UnknownRevision
    };

    enum struct InertialSaturation : uint8_t
    {
        None         = 0,
        Acceleration = 1,
        AngularRate  = 2,
        Both         = 3
    };

    enum struct InertialSampleQuality : uint8_t
    {
        Invalid,
        Current,
        Stale,
        Saturated
    };

    struct InertialSource
    {
        InertialSourceKind kind;
        InertialModel      model;
        uint8_t            sourceId;
        uint16_t           configurationRevision;
        uint16_t           calibrationRevision;
        uint32_t           accelerationRangeMicroG;
        uint32_t           angularRateRangeMilliDegreesPerSecond;
    };

    struct InertialVector
    {
        int32_t x;
        int32_t y;
        int32_t z;
    };

    struct InertialSample
    {
        InertialSource     source;
        InertialVector     accelerationMicroG;
        InertialVector     angularRateMilliDegreesPerSecond;
        TimePoint          observedAt;
        uint32_t           sequence;
        bool               dataReady;
        InertialSaturation saturation;
        Status             status;
    };

    struct InertialObservationConfig
    {
        Duration maximumAge;
        uint16_t freshnessContractRevision;
    };

    struct InertialObservation
    {
        InertialSample        sample;
        InertialSampleQuality quality;
        Duration              age;
        Duration              maximumAge;
        uint16_t              freshnessContractRevision;
        uint32_t              sequenceGap;
        Status                status;
    };

    // Pure copied-sample policy; it owns no transport, endpoint, or clock.
    struct InertialObservationPolicy
    {
        explicit InertialObservationPolicy (
            const InertialObservationConfig& config) noexcept;

        InertialObservationPolicy (const InertialObservationPolicy&) = delete;
        InertialObservationPolicy&
        operator= (const InertialObservationPolicy&) = delete;
        InertialObservationPolicy (InertialObservationPolicy&&) = delete;
        InertialObservationPolicy&
        operator= (InertialObservationPolicy&&) = delete;

        Status              initialize  () noexcept;
        void                reset       () noexcept;
        Status              update      (TimePoint             now,
                                         const InertialSample& sample) noexcept;
        InertialObservation snapshot    () const noexcept;
        bool                initialized () const noexcept;

      private:
        InertialObservationConfig config_;
        InertialObservation       observation_;
        TimePoint                 lastUpdateAt_;
        bool                      initialized_;
        bool                      hasSample_;
        bool                      hasUpdate_;
    };
} // namespace adk
