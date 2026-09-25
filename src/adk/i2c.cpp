#ifdef __AVR__

#include "i2c.h"

#include "board.h"

#include <Arduino.h>

namespace adk { namespace i2c {

    namespace {

        // TWSR status codes, with the prescaler bits masked off.
        const uint8_t Started      = 0x08;
        const uint8_t Restarted    = 0x10;
        const uint8_t WriteAcked   = 0x18;
        const uint8_t ByteAcked    = 0x28;
        const uint8_t ReadAcked    = 0x40;
        const uint8_t ByteReceived = 0x50;
        const uint8_t LastReceived = 0x58;

        // How many times a step polls for the TWI to finish: about 1 ms at
        // 16 MHz, ten byte times at 100 kHz. A bus with no pull-ups or a
        // line held low never finishes, and the transfer fails instead.
        const uint16_t Patience = 2000;

        // Start one step on the bus (a start, a byte, or a stop) and wait for
        // it. The status is 0 if the TWI never finished.
        uint8_t step (uint8_t control)
        {
            TWCR = static_cast<uint8_t> (control | _BV (TWINT) | _BV (TWEN));

            for (uint16_t polls = 0; polls < Patience; ++polls)
            {
                if (TWCR & _BV (TWINT))
                {
                    return TWSR & 0xF8;
                }
            }

            return 0;
        }

        // Send a start, or a repeated start, and the chip's address with the
        // direction bit. True when the chip acknowledges.
        bool start (uint8_t address, bool reading)
        {
            uint8_t status = step (_BV (TWSTA));

            if (status != Started && status != Restarted)
            {
                return false;
            }

            TWDR = static_cast<uint8_t> ((address << 1) | (reading ? 1 : 0));
            return step (0) == (reading ? ReadAcked : WriteAcked);
        }

        bool send (uint8_t byte)
        {
            TWDR = byte;
            return step (0) == ByteAcked;
        }

        // Send a stop and pass on whether the transfer worked. A stop that
        // never completes means the TWI is stuck, so it is reset for the next
        // transfer.
        bool finish (bool worked)
        {
            TWCR = _BV (TWINT) | _BV (TWSTO) | _BV (TWEN);

            for (uint16_t polls = 0; polls < Patience; ++polls)
            {
                if (!(TWCR & _BV (TWSTO)))
                {
                    return worked;
                }
            }

            TWCR = 0;
            TWCR = _BV (TWEN);
            return false;
        }
    }

    bool begin ()
    {
        if (!claimShared (SDA) || !claimShared (SCL))
        {
            return false;
        }

        // Another device on the bus has already started it.
        if (TWCR & _BV (TWEN))
        {
            return true;
        }

        // Like the Wire library, add the weak internal pull-ups; the
        // modules' own resistors do most of the work.
        digitalWrite (SDA, HIGH);
        digitalWrite (SCL, HIGH);

        // SCL = F_CPU / (16 + 2 * TWBR) with the prescaler at 1: 72 at 16 MHz.
        TWSR = 0;
        TWBR = static_cast<uint8_t> ((F_CPU / 100000UL - 16) / 2);
        TWCR = _BV (TWEN);
        return true;
    }

    bool present (uint8_t address)
    {
        return finish (start (address, false));
    }

    bool write (uint8_t address, const uint8_t* data, uint8_t length)
    {
        bool sent = start (address, false);

        for (uint8_t index = 0; sent && index < length; ++index)
        {
            sent = send (data[index]);
        }

        return finish (sent);
    }

    bool read (uint8_t address, uint8_t reg, uint8_t* data, uint8_t length)
    {
        // Reading nothing only sets the chip's register pointer.
        bool received = start (address, false) && send (reg)
                     && (length == 0 || start (address, true));

        // Acknowledge every byte but the last, which tells the chip to stop.
        for (uint8_t index = 0; received && index < length; ++index)
        {
            bool last = index + 1 == length;

            received    = step (last ? 0 : _BV (TWEA)) == (last ? LastReceived : ByteReceived);
            data[index] = TWDR;
        }

        return finish (received);
    }

    bool writeRegister (uint8_t address, uint8_t reg, uint8_t value)
    {
        const uint8_t bytes [] = {reg, value};

        return write (address, bytes, sizeof bytes);
    }
}}

#endif
