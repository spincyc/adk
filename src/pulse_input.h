#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    struct MicrosecondDuration
    {
        using Raw = uint32_t;

        explicit MicrosecondDuration (Raw microseconds = 0) noexcept;

        Raw microseconds () const noexcept;

      private:
        Raw microseconds_;
    };

    struct MicrosecondTimePoint
    {
        using Raw = uint32_t;

        explicit MicrosecondTimePoint (Raw microseconds = 0) noexcept;

        Raw                 microseconds () const noexcept;
        MicrosecondDuration elapsedSince (MicrosecondTimePoint earlier) const noexcept;

      private:
        Raw microseconds_;
    };

    enum struct PulseInputState : uint8_t
    {
        Idle,
        AwaitingLow,
        AwaitingRise,
        MeasuringHigh,
        Complete,
        Timeout
    };

    struct PulseInputConfig
    {
        MicrosecondDuration edgeTimeout;
        MicrosecondDuration maximumPulse;
    };

    struct PulseInputSnapshot
    {
        PulseInputState     state;
        MicrosecondDuration highDuration;
        bool                inputHigh;
        bool                complete;
        bool                timedOut;
    };

    struct PulseInput
    {
        explicit PulseInput (const PulseInputConfig& config) noexcept;

        Status initialize () noexcept;
        void   reset      () noexcept;

        Status arm    (MicrosecondTimePoint now, bool inputHigh) noexcept;
        Status update (MicrosecondTimePoint now, bool inputHigh) noexcept;

        PulseInputSnapshot snapshot    () const noexcept;
        bool               initialized () const noexcept;

      private:
        bool validConfig () const noexcept;
        bool exceeded    (MicrosecondTimePoint now, MicrosecondTimePoint earlier,
                       MicrosecondDuration limit) const noexcept;

        PulseInputConfig     config_;
        PulseInputState      state_;
        MicrosecondTimePoint phaseStarted_;
        MicrosecondTimePoint pulseStarted_;
        MicrosecondDuration  highDuration_;
        bool                 inputHigh_;
        bool                 initialized_;
    };
} // namespace adk
