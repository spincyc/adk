#pragma once

#include "digital.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct PinCapability : uint8_t
    {
        DigitalInput,
        DigitalOutput,
        PwmOutput,
        AnalogInput,
        ExternalInterrupt
    };

    struct Mega2560Board
    {
        static bool   validPin (PinId pin) noexcept;
        static bool   supports (PinId pin, PinCapability capability) noexcept;
        static Status pwmTimer (PinId pin, uint8_t& timer) noexcept;
    };
}
