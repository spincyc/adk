#pragma once

#include <stdint.h>

namespace adk { namespace i2c {

    // The Mega's I2C bus, driven by the ATmega2560's TWI at 100 kHz. Every
    // I2C module shares the same two wires:
    //
    //   SDA -> pin 20, SCL -> pin 21, GND -> GND
    //
    // Both lines need pull-up resistors; most modules carry their own.
    //
    // A device calls begin () from its setup () and talks to its chip with
    // the rest. A transfer is false when no chip acknowledges, or when a line
    // is stuck: every wait on the bus gives up after about a millisecond, so
    // a loose wire cannot hang the sketch. It cannot share the bus with the
    // Wire library, which drives the same hardware from an interrupt.

    // Claim SDA and SCL as shared pins and start the bus. Any number of
    // devices may call it. False if either pin is already used for something
    // else.
    bool begin ();

    // A chip acknowledges its 7-bit address, such as 0x68.
    bool present (uint8_t address);

    // Send bytes to a chip. For most chips the first is a register number.
    bool write (uint8_t address, const uint8_t* data, uint8_t length);

    // Read length bytes from a chip, starting at a register.
    bool read (uint8_t address, uint8_t reg, uint8_t* data, uint8_t length);

    // Set one register.
    bool writeRegister (uint8_t address, uint8_t reg, uint8_t value);
}}
