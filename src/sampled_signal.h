#pragma once

#include <stdint.h>

#include "status.h"

namespace adk {

    struct LinearCalibrationConfig
    {
        uint16_t observedMinimum;
        uint16_t observedMaximum;
        uint16_t mappedAtMinimum;
        uint16_t mappedAtMaximum;
        bool     clamp;
    };

    struct LinearCalibration
    {
        explicit LinearCalibration (const LinearCalibrationConfig& config) noexcept;

        bool             valid ()                const noexcept;
        Result<uint16_t> map   (uint16_t sample) const noexcept;

      private:
        LinearCalibrationConfig config_;
        bool                    valid_;
    };

    struct MovingAverage
    {
        static const uint8_t maximumWindowSize = 32;

        explicit MovingAverage (uint8_t windowSize) noexcept;

        void             reset       ()                        noexcept;
        bool             valid       ()                  const noexcept;
        bool             hasValue    ()                  const noexcept;
        uint8_t          sampleCount ()                  const noexcept;
        uint8_t          windowSize  ()                  const noexcept;
        Result<uint16_t> addSample   (uint16_t sample)          noexcept;
        Result<uint16_t> value       ()                  const noexcept;

      private:
        uint16_t samples_[maximumWindowSize];
        uint32_t sum_;
        uint8_t  windowSize_;
        uint8_t  sampleCount_;
        uint8_t  nextIndex_;
    };

    struct Deadband
    {
        explicit Deadband (uint16_t width) noexcept;

        void             reset     ()                        noexcept;
        bool             hasValue  ()                  const noexcept;
        uint16_t         width     ()                  const noexcept;
        uint16_t         addSample (uint16_t sample)          noexcept;
        Result<uint16_t> value     ()                  const noexcept;

      private:
        uint16_t width_;
        uint16_t value_;
        bool     hasValue_;
    };
}
