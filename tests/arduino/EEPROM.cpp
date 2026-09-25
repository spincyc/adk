#include "EEPROM.h"

#include <string.h>

EEPROMClass EEPROM;

EEPROMClass::EEPROMClass ()
{
    memset (bytes, 0xFF, sizeof bytes);
}

uint8_t EEPROMClass::read (int address) const
{
    return bytes[address];
}

void EEPROMClass::write (int address, uint8_t value)
{
    bytes[address] = value;
}

void EEPROMClass::update (int address, uint8_t value)
{
    bytes[address] = value;
}
