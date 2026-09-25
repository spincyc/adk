#pragma once

// A host stand-in for the AVR core's EEPROM library: the Mega's 4 KB, erased
// to 0xFF, in plain memory.

#include <stdint.h>

struct EEPROMClass
{
    EEPROMClass ();

    uint8_t read   (int address) const;
    void    write  (int address, uint8_t value);
    void    update (int address, uint8_t value);

    uint8_t bytes [4096];
};

extern EEPROMClass EEPROM;
