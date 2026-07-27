#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    struct ServoCalibrationConfig
    {
        uint16_t minimumPosition;
        uint16_t maximumPosition;
        uint16_t minimumPulseUs;
        uint16_t maximumPulseUs;
    };

    struct ServoCalibration
    {
        explicit ServoCalibration (const ServoCalibrationConfig& config) noexcept;

        bool             valid    ()                  const noexcept;
        Result<uint16_t> pulseFor (uint16_t position) const noexcept;

      private:
        ServoCalibrationConfig config_;
        bool                   valid_;
    };

    enum struct BoundedServoIntent : uint8_t
    {
        Inactive,
        Position
    };

    struct BoundedServoConfig
    {
        ServoCalibrationConfig calibration;
        uint16_t               safePosition;
    };

    struct BoundedServoSnapshot
    {
        BoundedServoIntent intent;
        Status      status;
        uint16_t    position;
        uint16_t    pulseUs;
    };

    struct BoundedServo
    {
        explicit BoundedServo (const BoundedServoConfig& config) noexcept;

        Status initialize () noexcept;
        Status command    (uint16_t position) noexcept;
        void   shutdown   () noexcept;

        BoundedServoSnapshot snapshot () const noexcept;

      private:
        BoundedServoConfig config_;
        ServoCalibration  calibration_;
        BoundedServoIntent       intent_;
        Status            status_;
        uint16_t          position_;
        uint16_t          pulseUs_;
        bool              initialized_;
    };
}
