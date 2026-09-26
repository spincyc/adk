// The ATmega2560's TWI and SPI unit, modeled from their registers as the
// datasheet describes them, so the library's own bus code runs on the host
// just as it would on the chip. The chips at the far end of the wires are
// hooks, in arduino::twi and arduino::spi.

#include "Arduino.h"

#include <stdio.h>
#include <stdlib.h>

arduino::Register TWBR {arduino::Register::Twbr};
arduino::Register TWSR {arduino::Register::Twsr};
arduino::Register TWDR {arduino::Register::Twdr};
arduino::Register TWCR {arduino::Register::Twcr};
arduino::Register SPCR {arduino::Register::Spcr};
arduino::Register SPSR {arduino::Register::Spsr};
arduino::Register SPDR {arduino::Register::Spdr};

namespace arduino {

    Twi twi;
    Spi spi;

    namespace {

        // TWSR status codes, from the datasheet's master tables.
        constexpr uint8_t Started      = 0x08;
        constexpr uint8_t Restarted    = 0x10;
        constexpr uint8_t WriteAcked   = 0x18;
        constexpr uint8_t WriteRefused = 0x20;
        constexpr uint8_t ByteAcked    = 0x28;
        constexpr uint8_t ByteRefused  = 0x30;
        constexpr uint8_t ReadAcked    = 0x40;
        constexpr uint8_t ReadRefused  = 0x48;
        constexpr uint8_t ByteReceived = 0x50;
        constexpr uint8_t LastReceived = 0x58;
        constexpr uint8_t Idle         = 0xF8;

        // What a write of TWINT sets going, and what the next byte is.
        enum class Step : uint8_t
        {
            None,
            Start,
            Byte,
            Stop
        };

        enum class Next : uint8_t
        {
            Nothing,
            Address,
            Write,
            Read
        };

        struct TwiUnit
        {
            uint8_t  control   = 0;      // TWCR as written, less TWINT and TWWC
            uint8_t  status    = Idle;
            uint8_t  prescaler = 0;      // TWSR's two writable bits
            uint8_t  data      = 0xFF;
            uint8_t  rate      = 0;
            bool     finished  = false;  // TWINT
            bool     collided  = false;  // TWWC
            bool     owner     = false;  // holding the bus, from a start to its stop
            Step     step      = Step::None;
            Next     next      = Next::Nothing;
            unsigned polled    = 0;
        };

        struct SpiUnit
        {
            uint8_t  control  = 0;
            uint8_t  data     = 0;
            uint8_t  sending  = 0;
            bool     busy     = false;
            bool     finished = false;   // SPIF
            bool     collided = false;   // WCOL
            bool     doubled  = false;   // SPI2X
            bool     noticed  = false;   // SPIF read, so the next SPDR access clears it
            unsigned polled   = 0;
        };

        TwiUnit twiUnit;
        SpiUnit spiUnit;

        void note (const char* what)
        {
            twi.log += twi.log.empty () ? "" : " ";
            twi.log += what;
        }

        // A byte on the bus in the log: 68w+ for an address, 3B- for data.
        void noteByte (uint8_t byte, const char* direction, bool acknowledged)
        {
            char text [8];
            snprintf (text, sizeof text, "%02X%s%c", byte, direction, acknowledged ? '+' : '-');
            note (text);
        }

        void moveByte (TwiUnit& unit)
        {
            switch (unit.next)
            {
                case Next::Address:
                {
                    uint8_t address = static_cast<uint8_t> (unit.data >> 1);
                    bool    reading = unit.data & 1;
                    bool    acked   = twi.onAddress && twi.onAddress (address, reading);

                    noteByte (address, reading ? "r" : "w", acked);
                    unit.status = reading ? (acked ? ReadAcked : ReadRefused)
                                          : (acked ? WriteAcked : WriteRefused);
                    unit.next   = !acked ? Next::Nothing : reading ? Next::Read : Next::Write;
                    break;
                }
                case Next::Write:
                {
                    bool acked = twi.onWrite && twi.onWrite (unit.data);

                    noteByte (unit.data, "", acked);
                    unit.status = acked ? ByteAcked : ByteRefused;
                    unit.next   = acked ? Next::Write : Next::Nothing;
                    break;
                }
                case Next::Read:
                {
                    // TWEA says whether the master acknowledges the byte,
                    // asking for another, or ends the read with it.
                    bool more = unit.control & _BV (TWEA);

                    unit.data   = twi.onRead ? twi.onRead (more) : 0xFF;
                    noteByte (unit.data, "", more);
                    unit.status = more ? ByteReceived : LastReceived;
                    unit.next   = more ? Next::Read : Next::Nothing;
                    break;
                }
                case Next::Nothing:
                    note ("?");
                    break;
            }
        }

        void finishStep (TwiUnit& unit)
        {
            Step step = unit.step;
            unit.step = Step::None;

            switch (step)
            {
                case Step::None:
                    return;
                case Step::Start:
                    note (unit.owner ? "Sr" : "S");
                    unit.status = unit.owner ? Restarted : Started;
                    unit.owner  = true;
                    unit.next   = Next::Address;
                    break;
                case Step::Byte:
                    moveByte (unit);
                    break;
                case Step::Stop:
                    // A stop clears TWSTO when it is done, and leaves TWINT.
                    note ("P");
                    unit.status   = Idle;
                    unit.owner    = false;
                    unit.next     = Next::Nothing;
                    unit.control &= static_cast<uint8_t> (~_BV (TWSTO));

                    if (twi.onStop)
                    {
                        twi.onStop ();
                    }
                    return;
            }

            unit.finished = true;
        }

        void writeTwcr (TwiUnit& unit, uint8_t value)
        {
            // Switched off, the TWI drops whatever it was doing and lets go
            // of the bus.
            if (!(value & _BV (TWEN)))
            {
                if (unit.control & _BV (TWEN))
                {
                    note ("off");
                }

                if (unit.owner && twi.onStop)
                {
                    twi.onStop ();
                }

                unit = {.prescaler = unit.prescaler, .data = unit.data, .rate = unit.rate};
                return;
            }

            unit.control = static_cast<uint8_t> (value & ~(_BV (TWINT) | _BV (TWWC)));

            // Writing TWINT clears it, which starts the next step.
            if (value & _BV (TWINT))
            {
                bool start = value & _BV (TWSTA);
                bool stop  = value & _BV (TWSTO);

                unit.finished = false;
                unit.polled   = 0;
                unit.step     = start ? Step::Start : stop ? Step::Stop : Step::Byte;

                if (start && stop)
                {
                    note ("?");
                    unit.step = Step::None;
                }
            }
        }

        uint8_t readTwcr (TwiUnit& unit)
        {
            if (unit.step != Step::None && !twi.stuck && ++unit.polled >= twi.polls)
            {
                finishStep (unit);
            }

            return static_cast<uint8_t> (unit.control | (unit.finished ? _BV (TWINT) : 0)
                                                      | (unit.collided ? _BV (TWWC) : 0));
        }

        void writeTwdr (TwiUnit& unit, uint8_t value)
        {
            // TWDR only takes a byte between steps; a write mid-step collides.
            unit.collided = !unit.finished;

            if (unit.finished)
            {
                unit.data = value;
            }
        }

        // An SS pin that is an input held low throws the unit out of master
        // mode, which it reports by setting SPIF.
        void checkSs (SpiUnit& unit)
        {
            const PinState& ss     = pin (SS);
            bool            master = (unit.control & _BV (MSTR)) && (unit.control & _BV (SPE));

            if (master && ss.mode != OUTPUT && ss.input == LOW)
            {
                unit.control  = static_cast<uint8_t> (unit.control & ~_BV (MSTR));
                unit.busy     = false;
                unit.finished = true;
            }
        }

        // SPIF and WCOL clear once SPDR is used after SPSR showed SPIF set.
        void touchSpdr (SpiUnit& unit)
        {
            if (unit.noticed)
            {
                unit.finished = false;
                unit.collided = false;
                unit.noticed  = false;
            }
        }

        void writeSpdr (SpiUnit& unit, uint8_t value)
        {
            checkSs   (unit);
            touchSpdr (unit);

            if (unit.busy)
            {
                unit.collided = true;
                return;
            }

            // Switched off or a slave, the unit waits for a clock no master
            // will send, and spi::transfer () would wait forever.
            if (!(unit.control & _BV (SPE)) || !(unit.control & _BV (MSTR)))
            {
                fprintf (stderr, "SPDR written while the SPI unit is not a master: "
                                 "the transfer would never finish\n");
                abort ();
            }

            unit.sending = value;
            unit.busy    = true;
            unit.polled  = 0;
        }

        uint8_t readSpsr (SpiUnit& unit)
        {
            checkSs (unit);

            if (unit.busy && ++unit.polled >= spi.polls)
            {
                unit.data     = spi.onTransfer ? spi.onTransfer (unit.sending) : 0xFF;
                unit.busy     = false;
                unit.finished = true;
            }

            unit.noticed = unit.noticed || unit.finished;

            return static_cast<uint8_t> ((unit.finished ? _BV (SPIF) : 0)
                                       | (unit.collided ? _BV (WCOL) : 0)
                                       | (unit.doubled ? _BV (SPI2X) : 0));
        }
    }

    // Arduino.cpp's reset () calls this.
    void resetBuses ()
    {
        twiUnit = {};
        spiUnit = {};
        twi     = {};
        spi     = {};
    }

    Register& Register::operator= (uint8_t value)
    {
        switch (name)
        {
            case Twbr: twiUnit.rate      = value;                           break;
            case Twsr: twiUnit.prescaler = static_cast<uint8_t> (value & 3); break;
            case Twdr: writeTwdr (twiUnit, value);                          break;
            case Twcr: writeTwcr (twiUnit, value);                          break;
            case Spsr: spiUnit.doubled   = value & _BV (SPI2X);             break;
            case Spdr: writeSpdr (spiUnit, value);                          break;
            case Spcr:
                spiUnit.control = value;

                if ((value & _BV (SPE)) && (value & _BV (MSTR)) && spi.onEnable)
                {
                    spi.onEnable ();
                }
                break;
        }

        return *this;
    }

    Register::operator uint8_t () const
    {
        switch (name)
        {
            case Twbr: return twiUnit.rate;
            case Twsr: return static_cast<uint8_t> (twiUnit.status | twiUnit.prescaler);
            case Twdr: return twiUnit.data;
            case Twcr: return readTwcr (twiUnit);
            case Spsr: return readSpsr (spiUnit);
            case Spcr:
                checkSs (spiUnit);
                return spiUnit.control;
            case Spdr:
                touchSpdr (spiUnit);
                return spiUnit.data;
        }

        return 0;
    }
}
