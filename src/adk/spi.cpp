#include "spi.h"

#include <Arduino.h>

namespace adk::spi {

    namespace {

        // Whether a chip already selects with SS. Claims start afresh with
        // every adk::setup (), so this starts afresh with the first device
        // to find the bus pins unclaimed.
        bool ssSelects = false;

        // A chip's select pin starts high, leaving the chip deselected. SS
        // is already a high output, but two chips selected with it would
        // both answer, so only one may.
        bool claimSelect (Pin select)
        {
            if (select != SS)
            {
                return claimOutput (select, true);
            }

            if (ssSelects)
            {
                return refuse (Fault::PinInUse, select);
            }

            ssSelects = true;
            return true;
        }
    }

    bool begin (Pin select)
    {
        if (!isClaimed (SS))
        {
            ssSelects = false;
        }

        if (!claimShared (MISO) || !claimShared (MOSI) || !claimShared (SCK) || !claimShared (SS))
        {
            return false;
        }

        digitalWrite (SS, HIGH);
        pinMode      (SS, OUTPUT);

        // Mode 0, most significant bit first, 16 MHz / 4. SCK and MOSI become
        // outputs only once the unit drives them, so no chip sees a stray
        // clock edge; the unit makes MISO an input itself.
        SPCR = _BV (SPE) | _BV (MSTR);
        SPSR = 0;

        pinMode (SCK,  OUTPUT);
        pinMode (MOSI, OUTPUT);
        return claimSelect (select);
    }

    uint8_t transfer (uint8_t byte)
    {
        SPDR = byte;

        while (!(SPSR & _BV (SPIF)))
        {
        }

        return SPDR;
    }
}
