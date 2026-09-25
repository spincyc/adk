#ifdef __AVR__

#include "spi.h"

#include "board.h"

#include <Arduino.h>

namespace adk::spi {

    bool begin ()
    {
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
        pinMode (MISO, INPUT);
        return true;
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

#endif
