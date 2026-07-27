#pragma once

#include <stdint.h>

namespace adk {

    struct ByteSpan
    {
        const uint8_t* data;
        uint16_t       size;
    };

    struct MutableByteSpan
    {
        uint8_t* data;
        uint16_t capacity;
    };

    struct TextSpan
    {
        const char* data;
        uint16_t    size;
    };

    struct MutableTextSpan
    {
        char*    data;
        uint16_t capacity;
    };
} // namespace adk
