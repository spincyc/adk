#pragma once

#include "status.h"
#include "time.h"

#include <stddef.h>
#include <stdint.h>

namespace adk {

    enum struct ClimateSampleState : uint8_t
    {
        Unavailable,
        Valid,
        TransportTimeout,
        ChecksumFailure,
        TemperatureOutOfRange,
        HumidityOutOfRange,
        Stale,
        InvalidLimits,
        InvalidTiming
    };

    struct ClimateSample
    {
        int16_t            temperatureCentiCelsius;
        uint16_t           humidityPermille;
        TimePoint          observedAt;
        ClimateSampleState state;
    };

    struct ClimateSampleLimits
    {
        int16_t  minimumTemperatureCentiCelsius;
        int16_t  maximumTemperatureCentiCelsius;
        uint16_t maximumHumidityPermille;
    };

    // Invalid limits produce InvalidLimits. Relative humidity never exceeds
    // 1000 permille, even when a caller requests a wider range.
    ClimateSample validateClimateSample (int16_t   temperatureCentiCelsius,
                                         uint16_t  humidityPermille,
                                         TimePoint observedAt,
                                         const ClimateSampleLimits& limits) noexcept;

    struct ClimateSensor
    {
        virtual ~ClimateSensor () noexcept;

        virtual Status        initialize  () noexcept                         = 0;
        virtual void          shutdown    () noexcept                         = 0;
        virtual bool          initialized () const noexcept                   = 0;
        virtual Status        update      (TimePoint now) noexcept             = 0;
        virtual ClimateSample sample      (TimePoint now,
                                           Duration  staleAfter) const noexcept = 0;
    };

    struct RecordedClimateFrame
    {
        TimePoint     availableAt;
        ClimateSample sample;
        Status        updateStatus;
    };

    // Times advance within the unsigned half-range. Equal-time frames retain
    // trace order and the final consumed frame supplies update() status.
    // Freshness is valid through staleAfter; larger or ambiguous ages are
    // Stale or InvalidTiming respectively.
    struct RecordedClimateSensor final : ClimateSensor
    {
        RecordedClimateSensor (const RecordedClimateFrame* frames,
                               size_t                      frameCount) noexcept;
        ~RecordedClimateSensor () noexcept override;

        RecordedClimateSensor (const RecordedClimateSensor&)            = delete;
        RecordedClimateSensor& operator= (const RecordedClimateSensor&) = delete;
        RecordedClimateSensor (RecordedClimateSensor&&)                 = delete;
        RecordedClimateSensor& operator= (RecordedClimateSensor&&)      = delete;

        Status        initialize  () noexcept override;
        void          shutdown    () noexcept override;
        bool          initialized () const noexcept override;
        Status        update      (TimePoint now) noexcept override;
        ClimateSample sample      (TimePoint now,
                                   Duration  staleAfter) const noexcept override;

        size_t frameIndex () const noexcept;

      private:
        bool traceValid () const noexcept;
        bool frameDue   (TimePoint now, TimePoint availableAt) const noexcept;

        const RecordedClimateFrame* frames_;
        size_t                      frameCount_;
        size_t                      frameIndex_;
        ClimateSample               sample_;
        TimePoint                   lastUpdateAt_;
        bool                        initialized_;
        bool                        hasUpdated_;
    };
} // namespace adk
