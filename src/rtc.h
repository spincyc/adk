#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct ClockState : uint8_t
    {
        Valid,
        NotSet,
        OscillatorStopped,
        TransportFault
    };

    struct ClockReading
    {
        uint32_t   unixSeconds;
        ClockState state;
    };

    struct Rtc
    {
        virtual ~Rtc () noexcept;

        virtual Status               initialize () noexcept = 0;
        virtual void                 shutdown   () noexcept   = 0;
        virtual Result<ClockReading> read       () noexcept       = 0;
    };
} // namespace adk
