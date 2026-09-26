#pragma once

// Chips on the SPI bus, for host tests. src/adk/spi.cpp drives the fake
// core's model of the SPI unit, and each byte it sends goes to the chip
// whose select pin is a low output, as the wires would take it.

#include <adk/board.h>

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
        int begins;    // the unit set up as master, once by each device
        int bytes;
        int strays;    // sent with no chip selected
        int clashes;   // sent with two chips selected at once
    };

    extern SpiLog spiLog;
}
