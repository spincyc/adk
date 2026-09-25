#pragma once

#include <stdint.h>

namespace adk {

    // A 5x7 font for the printable ASCII characters, ' ' to '~', kept in flash
    // (475 bytes, no RAM). A character is five columns, left to right. Each
    // column is one byte: bit 0 is the top row, bit 6 the bottom, and bit 7 is
    // always clear.

    // One column of a character, 0-4. Columns from 5 on are blank, so text
    // laid out six columns to a character has a blank column between
    // characters. A character outside ' ' to '~' is drawn as '?'.
    uint8_t fontColumn (char character, uint8_t column);
}
