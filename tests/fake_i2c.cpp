#include "fake_i2c.h"

#include <adk/board.h>
#include <adk/i2c.h>

#include <Arduino.h>

namespace fake_i2c {

    namespace {

        Chip* first = nullptr;
    }

    Chip::Chip (uint8_t at)
        : address   (at)
        , present   (true)
        , failures  (0)
        , pointer   (0)
        , transfers (0)
        , registers ()
        , next_     (first)
    {
        first = this;
    }

    Chip::~Chip ()
    {
        for (Chip** link = &first; *link; link = &(*link)->next_)
        {
            if (*link == this)
            {
                *link = next_;
                return;
            }
        }
    }

    void Chip::put (uint8_t reg, int16_t value)
    {
        uint16_t bits = static_cast<uint16_t> (value);

        registers[reg]                            = static_cast<uint8_t> (bits >> 8);
        registers[static_cast<uint8_t> (reg + 1)] = static_cast<uint8_t> (bits);
    }

    Chip* find (uint8_t address)
    {
        for (Chip* chip = first; chip; chip = chip->next_)
        {
            if (chip->address == address)
            {
                return chip;
            }
        }

        return nullptr;
    }
}

namespace adk { namespace i2c {

    namespace {

        // The chip that acknowledges a transfer to an address, if any.
        fake_i2c::Chip* reach (uint8_t address)
        {
            fake_i2c::Chip* chip = fake_i2c::find (address);

            if (!chip)
            {
                return nullptr;
            }

            ++chip->transfers;

            if (!chip->present)
            {
                return nullptr;
            }

            if (chip->failures > 0)
            {
                --chip->failures;
                return nullptr;
            }

            return chip;
        }
    }

    bool begin ()
    {
        return claimShared (SDA) && claimShared (SCL);
    }

    bool present (uint8_t address)
    {
        return reach (address) != nullptr;
    }

    bool write (uint8_t address, const uint8_t* data, uint8_t length)
    {
        fake_i2c::Chip* chip = reach (address);

        if (!chip)
        {
            return false;
        }

        for (uint8_t index = 0; index < length; ++index)
        {
            if (index == 0)
            {
                chip->pointer = data[index];
            }
            else
            {
                chip->registers[chip->pointer++] = data[index];
            }
        }

        return true;
    }

    bool read (uint8_t address, uint8_t reg, uint8_t* data, uint8_t length)
    {
        fake_i2c::Chip* chip = reach (address);

        if (!chip)
        {
            return false;
        }

        chip->pointer = reg;

        for (uint8_t index = 0; index < length; ++index)
        {
            data[index] = chip->registers[chip->pointer++];
        }

        return true;
    }

    bool writeRegister (uint8_t address, uint8_t reg, uint8_t value)
    {
        const uint8_t bytes [] = {reg, value};

        return write (address, bytes, sizeof bytes);
    }
}}
