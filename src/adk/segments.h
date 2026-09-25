#pragma once

#include <stdint.h>

namespace adk {

    // The segments of a seven-segment digit, one bit each:
    //
    //        a
    //      -----
    //   f |     | b
    //      --g--
    //   e |     | c
    //      -----   . dp
    //        d
    namespace segment {

        inline constexpr uint8_t a   = 0x01;
        inline constexpr uint8_t b   = 0x02;
        inline constexpr uint8_t c   = 0x04;
        inline constexpr uint8_t d   = 0x08;
        inline constexpr uint8_t e   = 0x10;
        inline constexpr uint8_t f   = 0x20;
        inline constexpr uint8_t g   = 0x40;
        inline constexpr uint8_t dot = 0x80;
    }

    // The segments that draw a character: 0-9; A-F in either case, with b
    // and d always lowercase because B and D would look like 8 and 0; the
    // letters that read clearly, G H I J L N O P Q R S T U Y, where h, o and
    // u have lowercase shapes of their own; and - _ and space. Anything else
    // is blank. The decimal point is never set.
    uint8_t segments (char character);
}
