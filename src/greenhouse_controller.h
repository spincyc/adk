#pragma once

#include "character_display.h"
#include "moisture_sensor.h"
#include "record_sink.h"
#include "status.h"
#include "time.h"
#include "watering_controller.h"

#include <stdint.h>

namespace adk {

    struct GreenhouseConfig
    {
        Duration sampleInterval;
        Duration displayInterval;
        Duration recordInterval;
        Duration staleAfter;
    };

    enum struct GreenhouseMode : uint8_t
    {
        Starting,
        Monitoring,
        Watering,
        Inhibited,
        SensorFault,
        OutputFault,
        DisplayFault,
        RecordFault,
        MultipleFaults
    };

    struct GreenhouseInput
    {
        bool wateringAllowed;
    };

    struct GreenhouseSnapshot
    {
        MoistureSample   moisture;
        WateringSnapshot watering;
        GreenhouseMode   mode;
        Status           sensorStatus;
        Status           outputStatus;
        Status           displayStatus;
        Status           recordStatus;
        uint32_t         recordSequence;
    };

    struct GreenhouseController
    {
        GreenhouseController  (const GreenhouseConfig& config,
                               MoistureSensor&         moisture,
                               WateringController&     watering,
                               CharacterDisplay&       display,
                               RecordSink&             records) noexcept;
        ~GreenhouseController () noexcept;

        GreenhouseController            (const GreenhouseController&) = delete;
        GreenhouseController& operator= (const GreenhouseController&) = delete;
        GreenhouseController            (GreenhouseController&&)      = delete;
        GreenhouseController& operator= (GreenhouseController&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status observe    (TimePoint now) noexcept;
        Status decide     (TimePoint              now,
                           const GreenhouseInput& input) noexcept;
        Status actuate    (TimePoint now) noexcept;
        Status present    (TimePoint now) noexcept;
        Status record     (TimePoint now) noexcept;

        GreenhouseSnapshot snapshot    () const noexcept;
        bool               initialized () const noexcept;

      private:
        bool   configValid     () const noexcept;
        bool   due             (TimePoint now, TimePoint epoch,
                                Duration interval) const noexcept;
        void   anchorSchedules (TimePoint now) noexcept;
        void   refreshMode     () noexcept;
        Status prepareRecord   (TimePoint now) noexcept;
        Status prepareView     (TimePoint now) noexcept;

        GreenhouseConfig      config_;
        MoistureSensor*       moisture_;
        WateringController*   watering_;
        CharacterDisplay*     display_;
        RecordSink*           records_;
        GreenhouseSnapshot    snapshot_;
        GreenhouseSnapshot    decided_;
        StableRecord          pendingRecord_;
        TimePoint             sampleEpoch_;
        TimePoint             displayEpoch_;
        TimePoint             recordEpoch_;
        TimePoint             decisionAt_;
        bool                  initialized_;
        bool                  schedulesAnchored_;
        bool                  decisionReady_;
        bool                  actuationApplied_;
        bool                  pendingRecordReady_;
        bool                  outputFaultLatched_;
    };
} // namespace adk
