#pragma once

#include "board.h"

namespace adk::spi {

    // The Mega's SPI unit as the bus master: mode 0, most significant bit
    // first, 4 MHz. Every chip on the bus shares three pins, and each has a
    // chip-select pin of its own that its device holds high until it talks:
    //
    //   Chip   Mega
    //   MISO   50
    //   MOSI   51
    //   SCK    52
    //
    // Pin 53 (SS) belongs to the bus too. It stays a high output, because the
    // unit drops out of master mode whenever SS is an input held low. No other
    // part may use it, though one chip on the bus may have it as its select.

    // Claim pins 50-53 and a chip's select pin, held high until the chip is
    // talked to, and start the unit. Each device on the bus calls it from
    // its setup (); the bus pins are shared, so later calls find them
    // already claimed. False if another part already uses one of the pins,
    // or another chip already selects with 53.
    bool begin (Pin select);

    // Send a byte and return the one that came back at the same time. The
    // chip's select pin must already be low. Takes about 2 us.
    uint8_t transfer (uint8_t byte);
}
