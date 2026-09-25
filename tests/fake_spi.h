#pragma once

// A host stand-in for the SPI unit. adk::spi::begin () claims and configures
// the bus pins as the real one does, and adk::spi::transfer () hands each
// byte to the chip whose select pin is a low output, as the wires would.

#include <adk/board.h>
#include <adk/spi.h>

namespace fake {

    // A chip on the bus, played by a test. Chips live for one test.
    struct SpiChip
    {
        explicit SpiChip (adk::Pin select);
        virtual ~SpiChip ();

        SpiChip            (const SpiChip&) = delete;
        SpiChip& operator= (const SpiChip&) = delete;

        // A byte arrived on MOSI; the result goes back on MISO.
        virtual uint8_t exchange (uint8_t byte) = 0;

        // The select pin went high, ending the transaction.
        virtual void deselected ();

        adk::Pin select_;
        SpiChip* next_;
    };

    // What the bus saw since the first chip of the test was made.
    struct SpiLog
    {
        int begins;
        int bytes;
        int strays;    // sent with no chip selected
        int clashes;   // sent with two chips selected at once
    };

    extern SpiLog spiLog;
}
