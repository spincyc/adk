#pragma once

#include <stdint.h>

namespace adk { namespace spi {

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
    // part may use it, though a device on the bus may share it as its chip
    // select by claiming it with claimShared ().

    // Claim pins 50-53 and start the unit. Each device on the bus calls it
    // from its setup (); the pins are shared, so later calls find them
    // already claimed. False if another part already uses one of them.
    bool begin ();

    // Send a byte and return the one that came back at the same time. The
    // chip's select pin must already be low. Takes about 2 us.
    uint8_t transfer (uint8_t byte);
}}
