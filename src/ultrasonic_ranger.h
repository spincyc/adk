#pragma once

#include "pulse_input.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct RangeState : uint8_t
    {
        Idle,
        AwaitingEcho,
        Measuring,
        Valid,
        Timeout,
        OutOfRange
    };

    struct RangeReading
    {
        RangeState          state;
        uint16_t            distanceMm;
        MicrosecondDuration echoDuration;
        bool                valid;
    };

    struct UltrasonicRangerConfig
    {
        MicrosecondDuration echoTimeout;
        MicrosecondDuration maximumEchoDuration;
        uint16_t            minimumDistanceMm;
        uint16_t            maximumDistanceMm;
        uint16_t            soundSpeedMicrometersPerMicrosecond;
    };

    struct UltrasonicRanger
    {
        explicit UltrasonicRanger (const UltrasonicRangerConfig& config) noexcept;

        Status initialize () noexcept;
        void   reset      () noexcept;

        Status startMeasurement (MicrosecondTimePoint now, bool echoHigh) noexcept;
        Status update           (MicrosecondTimePoint now, bool echoHigh) noexcept;

        RangeReading reading     () const noexcept;
        bool         initialized () const noexcept;

      private:
        bool     validConfig      () const noexcept;
        uint16_t distanceFromEcho (MicrosecondDuration echoDuration) const noexcept;
        void     updateReading    () noexcept;

        UltrasonicRangerConfig config_;
        PulseInput             pulse_;
        RangeReading           reading_;
        bool                   initialized_;
    };
} // namespace adk
