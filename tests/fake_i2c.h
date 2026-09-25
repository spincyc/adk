#pragma once

// A host stand-in for the I2C bus. src/adk/i2c.cpp is compiled only for the
// AVR; here adk::i2c talks to simulated chips instead. Declaring a Chip
// connects it at its address until it goes out of scope, so every test
// starts with an empty bus.

#include <stdint.h>

namespace fake_i2c {

    // A chip with 256 registers. Like the DS1307 and the MPU-6050, a write
    // sets its register pointer with the first byte, and every byte read or
    // written after that moves the pointer on by one.
    struct Chip
    {
        explicit Chip (uint8_t at);
        ~Chip ();

        Chip            (const Chip&) = delete;
        Chip& operator= (const Chip&) = delete;

        // Store a big-endian 16-bit reading at a register and the next one.
        void put (uint8_t reg, int16_t value);

        uint8_t  address;
        bool     present;           // false: it stops acknowledging, as if unplugged
        uint8_t  failures;          // how many of the next transfers to it fail
        uint8_t  pointer;
        uint16_t transfers;         // every transfer addressed to it, failed or not
        uint8_t  registers [256];

      private:
        friend Chip* find (uint8_t address);

        Chip* next_;
    };

    // The chip at an address, or nullptr if the bus has none.
    Chip* find (uint8_t address);
}
