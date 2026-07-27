#pragma once

#include <stdint.h>

namespace adk {

    using PinId = uint8_t;

    enum struct Level : uint8_t
    {
        Low,
        High
    };

    enum struct Pull : uint8_t
    {
        None,
        Up
    };
}
