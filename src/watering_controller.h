#pragma once

#include "moisture_sensor.h"
#include "pump_output.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct WateringConfig
    {
        uint16_t startBelowPermille;
        uint16_t stopAtPermille;
        Duration maximumOnTime;
        Duration minimumOffTime;
    };

    enum struct WateringState : uint8_t
    {
        Starting,
        Idle,
        Watering,
        LockedOut,
        SensorFault,
        OutputFault
    };

    enum struct WateringReason : uint8_t
    {
        None,
        DryThreshold,
        WetThreshold,
        MaximumOnTime,
        MinimumOffTime,
        OperatorInhibit,
        InvalidSample,
        OutputFailure,
        Shutdown
    };

    struct WateringSnapshot
    {
        WateringState  state;
        WateringReason reason;
        PumpState      requestedPump;
        TimePoint      stateSince;
    };

    struct WateringController
    {
        WateringController  (const WateringConfig& config, PumpOutput& pump) noexcept;
        ~WateringController () noexcept;

        WateringController (const WateringController&)            = delete;
        WateringController& operator= (const WateringController&) = delete;
        WateringController (WateringController&&)                 = delete;
        WateringController& operator= (WateringController&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status decide     (TimePoint now, const MoistureSample& sample,
                       bool wateringAllowed) noexcept;
        Status actuate () noexcept;

        WateringSnapshot snapshot    () const noexcept;
        bool             initialized () const noexcept;

      private:
        bool   configValid () const noexcept;
        bool   sampleValid (const MoistureSample& sample) const noexcept;
        bool   elapsed     (TimePoint now, Duration interval) const noexcept;
        void   enter       (WateringState state, WateringReason reason, PumpState pump,
                      TimePoint now) noexcept;
        Status enterOutputFault () noexcept;

        WateringConfig config_;
        PumpOutput*    pump_;
        WateringState  state_;
        WateringReason reason_;
        PumpState      requested_;
        PumpState      applied_;
        TimePoint      stateSince_;
        bool           initialized_;
        bool           hasStateTime_;
    };
} // namespace adk
