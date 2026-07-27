#pragma once

#include "analog_input.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct MoistureSampleState : uint8_t
    {
        Unavailable,
        Valid,
        InputBelowRange,
        InputAboveRange,
        Stale
    };

    struct MoistureCalibration
    {
        AnalogInput::Reading dryReading;
        AnalogInput::Reading wetReading;
        AnalogInput::Reading faultMargin;
    };

    struct MoistureSample
    {
        uint16_t             moisturePermille;
        AnalogInput::Reading rawReading;
        TimePoint            observedAt;
        MoistureSampleState  state;
    };

    struct MoistureSensor
    {
        MoistureSensor  (AnalogInput&               input,
                         const MoistureCalibration& calibration) noexcept;
        ~MoistureSensor () noexcept;

        MoistureSensor& operator= (const MoistureSensor&) = delete;
        MoistureSensor  (const MoistureSensor&)           = delete;
        MoistureSensor& operator= (MoistureSensor&&)      = delete;
        MoistureSensor  (MoistureSensor&&)                = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now) noexcept;

        MoistureSample sample      (TimePoint now,
                                    Duration  staleAfter) const noexcept;
        bool           initialized () const noexcept;

      private:
        bool validCalibration () const noexcept;
        void deriveSample     (AnalogInput::Reading reading,
                               TimePoint            now) noexcept;

        AnalogInput*        input_;
        MoistureCalibration calibration_;
        MoistureSample      sample_;
        bool                initialized_;
    };
}
