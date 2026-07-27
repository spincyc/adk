#pragma once

#include "climate_sensor.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct EnvironmentalHealth : uint8_t
    {
        Starting,
        Healthy,
        SensorFault,
        Stale,
        TimingFault
    };

    struct EnvironmentalStationConfig
    {
        Duration samplePeriod = Duration (2000);
        Duration staleAfter   = Duration (5000);
    };

    struct EnvironmentalRecord
    {
        ClimateSample       sample;
        EnvironmentalHealth health;
        Status              sensorStatus;
        TimePoint           recordedAt;
        uint32_t            sequence;
    };

    struct EnvironmentalSnapshot
    {
        EnvironmentalRecord record;
        TimePoint           nextSampleAt;
        Status              status;
        bool                hasDeadline;
        bool                recordReady;
    };

    struct EnvironmentalStation
    {
        EnvironmentalStation (ClimateSensor&                    sensor,
                              const EnvironmentalStationConfig& config) noexcept;
        ~EnvironmentalStation () noexcept;

        EnvironmentalStation (const EnvironmentalStation&)            = delete;
        EnvironmentalStation& operator= (const EnvironmentalStation&) = delete;
        EnvironmentalStation (EnvironmentalStation&&)                 = delete;
        EnvironmentalStation& operator= (EnvironmentalStation&&)      = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;
        Status reset       () noexcept;
        Status update      (TimePoint now) noexcept;

        EnvironmentalSnapshot snapshot () const noexcept;

      private:
        bool                configValid  () const noexcept;
        bool                sampleDue    (TimePoint now) const noexcept;
        EnvironmentalHealth chooseHealth (const ClimateSample& sample,
                                           Status               sensorStatus) const noexcept;
        void                clearState   (Status status) noexcept;

        ClimateSensor*             sensor_;
        EnvironmentalStationConfig config_;
        EnvironmentalSnapshot      snapshot_;
        TimePoint                  lastUpdateAt_;
        bool                       initialized_;
        bool                       hasUpdated_;
    };
} // namespace adk
