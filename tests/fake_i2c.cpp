#include "fake_i2c.h"

#include <Arduino.h>

namespace fake_i2c {

    namespace {

        Chip* first = nullptr;

        // The chip taking part in the transfer under way, and whether its
        // next byte written sets its register pointer. A transfer runs from
        // its first address to its stop, across any repeated start.
        Chip* talking  = nullptr;
        bool  underWay = false;
        bool  pointing = false;

        bool hearAddress (uint8_t at, bool reading)
        {
            Chip* chip = find (at);

            if (!underWay)
            {
                underWay = true;
                talking  = nullptr;

                if (chip)
                {
                    ++chip->transfers;
                }

                if (!chip || !chip->present)
                {
                    return false;
                }

                if (chip->failures > 0)
                {
                    --chip->failures;
                    return false;
                }

                talking = chip;
            }

            pointing = !reading;
            return talking && chip == talking;
        }

        bool hearByte (uint8_t byte)
        {
            if (!talking)
            {
                return false;
            }

            if (pointing)
            {
                talking->pointer = byte;
                pointing         = false;
            }
            else
            {
                talking->registers[talking->pointer++] = byte;
            }

            return true;
        }

        uint8_t sendByte (bool)
        {
            return talking ? talking->registers[talking->pointer++] : 0xFF;
        }

        void endTransfer ()
        {
            talking  = nullptr;
            underWay = false;
        }
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
        // The first chip of a test finds the bus idle, and answers for
        // every chip there.
        if (!first)
        {
            endTransfer ();

            arduino::twi.onAddress = hearAddress;
            arduino::twi.onWrite   = hearByte;
            arduino::twi.onRead    = sendByte;
            arduino::twi.onStop    = endTransfer;
        }

        first = this;
    }

    Chip::~Chip ()
    {
        if (talking == this)
        {
            endTransfer ();
        }

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
