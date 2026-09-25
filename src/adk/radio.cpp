#include "radio.h"

#include <Arduino.h>
#include <string.h>

namespace adk {

    namespace {

        // Timer 1 counts at 2 MHz and interrupts every 125 counts: 16 kHz,
        // eight samples of each bit at 2000 bits a second.
        const uint8_t  RadioTimer    = 1;
        const uint8_t  RadioUser     = 1;
        const uint16_t Compare       = 124;
        const uint8_t  SamplesPerBit = 8;

        // The receiver keeps in step with the sender with a ramp that climbs
        // by 20 a sample and takes a bit each time it passes 160. An edge
        // before halfway means the ramp is ahead, so it climbs less that
        // sample; after halfway, behind, so more. These are RH_ASK's numbers.
        const uint8_t RampLength     = 160;
        const uint8_t RampStep       = RampLength / SamplesPerBit;
        const uint8_t RampTransition = RampLength / 2;
        const uint8_t RampAdjust     = 9;
        const uint8_t RampRetard     = RampStep - RampAdjust;
        const uint8_t RampAdvance    = RampStep + RampAdjust;

        // Four bits as six, each with three ones and three zeros, so the
        // signal never stays high or low long enough for the receiver's
        // automatic gain to drift.
        const uint8_t Symbols [16] PROGMEM = {
            0x0D, 0x0E, 0x13, 0x15, 0x16, 0x19, 0x1A, 0x1C,
            0x23, 0x25, 0x26, 0x29, 0x2A, 0x2C, 0x32, 0x34};

        // Six symbols of alternating bits to lock onto, then the two that
        // mark where the message starts. Bits go out least significant first.
        const uint8_t  Preamble [8] PROGMEM = {0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x38, 0x2C};
        const uint8_t  PreambleSymbols      = sizeof Preamble;
        const uint16_t StartSymbol          = 0xB38;

        // A message is its length (counting everything), RH_ASK's four header
        // bytes (to, from, id and flags), the text, and a checksum.
        const uint8_t  HeaderLength = 4;
        const uint8_t  Overhead     = 1 + HeaderLength + 2;
        const uint8_t  Everyone     = 0xFF;
        const uint16_t Good         = 0xF0B8;   // what a message and its checksum add up to

        // The CCITT checksum, least significant bit first, as avr-libc's
        // _crc_ccitt_update () works it out.
        uint16_t checksum (uint16_t crc, uint8_t data)
        {
            data = static_cast<uint8_t> (data ^ (crc & 0xFF));
            data = static_cast<uint8_t> (data ^ (data << 4));
            return static_cast<uint16_t> (((static_cast<uint16_t> (data) << 8) | (crc >> 8))
                                          ^ (data >> 4) ^ (static_cast<uint16_t> (data) << 3));
        }

        uint8_t nibbleOf (uint8_t symbol)
        {
            for (uint8_t nibble = 0; nibble < 16; ++nibble)
            {
                if (pgm_read_byte (&Symbols[nibble]) == symbol)
                {
                    return nibble;
                }
            }

            return 0;
        }

        // Keeps the compiler from moving reads or writes of a message
        // across the flag that hands it between the interrupt and the sketch.
        inline void barrier ()
        {
            asm volatile ("" ::: "memory");
        }
    }

    // Timer 1's interrupt, shared by the one transmitter and one receiver.
    struct RadioClock
    {
        static RadioTransmitter* transmitter;
        static RadioReceiver*    receiver;

        static bool start (Pin pin)
        {
            if (!claimTimer (RadioTimer, pin, RadioUser))
            {
                return false;
            }

            TCCR1A  = 0;
            TCCR1B  = _BV (WGM12) | _BV (CS11);
            OCR1A   = Compare;
            TIMSK1 |= _BV (OCIE1A);
            return true;
        }

        static void tick ()
        {
            if (receiver)
            {
                receiver->sample ();
            }

            if (transmitter)
            {
                transmitter->step ();
            }
        }
    };

    RadioTransmitter* RadioClock::transmitter = nullptr;
    RadioReceiver*    RadioClock::receiver    = nullptr;

    RadioTransmitter::RadioTransmitter (Pin data)
        : frame_   {}
        , length_  (0)
        , symbol_  (0)
        , bit_     (0)
        , tick_    (0)
        , pin_     (data)
        , sending_ (false)
    {
    }

    RadioTransmitter::~RadioTransmitter ()
    {
        if (RadioClock::transmitter == this)
        {
            RadioClock::transmitter = nullptr;
        }
    }

    void RadioTransmitter::setup ()
    {
        if (RadioClock::transmitter && RadioClock::transmitter != this)
        {
            refuse (Fault::TimerInUse, pin_);
            return;
        }

        if (claimOutput (pin_) && RadioClock::start (pin_))
        {
            RadioClock::transmitter = this;
        }
    }

    bool RadioTransmitter::send (const char* text)
    {
        size_t length = strlen (text);
        return length <= MaxLength
            && send (reinterpret_cast<const uint8_t*> (text), static_cast<uint8_t> (length));
    }

    bool RadioTransmitter::send (const uint8_t* bytes, uint8_t length)
    {
        if (sending_ || length > MaxLength)
        {
            return false;
        }

        uint8_t  count = static_cast<uint8_t> (length + Overhead);
        uint16_t crc   = 0xFFFF;
        uint8_t  size  = 0;

        auto add = [&] (uint8_t byte)
        {
            frame_[size++] = byte;
            crc            = checksum (crc, byte);
        };

        add (count);
        add (Everyone);
        add (Everyone);
        add (0);
        add (0);

        for (uint8_t index = 0; index < length; ++index)
        {
            add (bytes[index]);
        }

        crc            = static_cast<uint16_t> (~crc);
        frame_[size++] = static_cast<uint8_t> (crc);
        frame_[size++] = static_cast<uint8_t> (crc >> 8);

        length_ = size;
        symbol_ = 0;
        bit_    = 0;
        tick_   = 0;
        barrier ();
        sending_ = true;
        return true;
    }

    bool RadioTransmitter::isSending () const
    {
        return sending_;
    }

    // Every sample; a new bit every eighth. Each byte goes out as two
    // symbols, its high four bits first.
    void RadioTransmitter::step ()
    {
        if (!sending_)
        {
            return;
        }

        if (tick_++ != 0)
        {
            if (tick_ == SamplesPerBit)
            {
                tick_ = 0;
            }

            return;
        }

        if (symbol_ == PreambleSymbols + 2 * length_)
        {
            digitalWrite (pin_, LOW);
            sending_ = false;
            return;
        }

        uint8_t symbol;

        if (symbol_ < PreambleSymbols)
        {
            symbol = pgm_read_byte (&Preamble[symbol_]);
        }
        else
        {
            uint8_t index  = static_cast<uint8_t> (symbol_ - PreambleSymbols);
            uint8_t byte   = frame_[index / 2];
            uint8_t nibble = (index % 2 == 0) ? byte >> 4 : byte & 0x0F;
            symbol         = pgm_read_byte (&Symbols[nibble]);
        }

        digitalWrite (pin_, ((symbol >> bit_) & 1) ? HIGH : LOW);

        if (++bit_ == 6)
        {
            bit_ = 0;
            ++symbol_;
        }
    }

    void RadioTransmitter::stop ()
    {
        sending_ = false;
        digitalWrite (pin_, LOW);
    }

    RadioReceiver::RadioReceiver (Pin data)
        : frame_      {}
        , text_       {}
        , bits_       (0)
        , ramp_       (0)
        , integrator_ (0)
        , bitCount_   (0)
        , count_      (0)
        , heard_      (0)
        , length_     (0)
        , pin_        (data)
        , last_       (false)
        , active_     (false)
        , received_   (false)
        , full_       (false)
    {
    }

    RadioReceiver::~RadioReceiver ()
    {
        if (RadioClock::receiver == this)
        {
            RadioClock::receiver = nullptr;
        }
    }

    void RadioReceiver::setup ()
    {
        if (RadioClock::receiver && RadioClock::receiver != this)
        {
            refuse (Fault::TimerInUse, pin_);
            return;
        }

        if (claimInput (pin_) && RadioClock::start (pin_))
        {
            RadioClock::receiver = this;
        }
    }

    bool RadioReceiver::wasReceived () const
    {
        return received_;
    }

    const char* RadioReceiver::text () const
    {
        return text_;
    }

    uint8_t RadioReceiver::length () const
    {
        return length_;
    }

    void RadioReceiver::update (Millis)
    {
        received_ = false;

        if (!full_)
        {
            return;
        }

        barrier ();
        uint16_t crc = 0xFFFF;

        for (uint8_t index = 0; index < count_; ++index)
        {
            crc = checksum (crc, frame_[index]);
        }

        if (crc == Good)
        {
            length_ = static_cast<uint8_t> (count_ - Overhead);
            memcpy (text_, frame_ + 1 + HeaderLength, length_);
            text_[length_] = '\0';
            received_      = true;
        }

        barrier ();
        full_ = false;
    }

    // Every sample: count the high ones, keep the ramp in step, and at the
    // end of each bit shift it in. Twelve bits make a byte.
    void RadioReceiver::sample ()
    {
        bool high = digitalRead (pin_) == HIGH;

        if (high)
        {
            ++integrator_;
        }

        uint8_t climb = RampStep;

        if (high != last_)
        {
            climb = (ramp_ < RampTransition) ? RampRetard : RampAdvance;
            last_ = high;
        }

        ramp_ = static_cast<uint8_t> (ramp_ + climb);

        if (ramp_ < RampLength)
        {
            return;
        }

        ramp_ -= RampLength;
        bits_  = static_cast<uint16_t> (bits_ >> 1);

        if (integrator_ >= SamplesPerBit / 2 + 1)
        {
            bits_ |= 0x800;
        }

        integrator_ = 0;

        // The sketch hasn't taken the last message yet.
        if (full_)
        {
            return;
        }

        if (!active_)
        {
            if (bits_ == StartSymbol)
            {
                active_   = true;
                bitCount_ = 0;
                heard_    = 0;
            }

            return;
        }

        if (++bitCount_ < 12)
        {
            return;
        }

        bitCount_ = 0;

        uint8_t first  = nibbleOf (static_cast<uint8_t> (bits_ & 0x3F));
        uint8_t second = nibbleOf (static_cast<uint8_t> (bits_ >> 6));
        uint8_t byte   = static_cast<uint8_t> (first << 4 | second);

        if (heard_ == 0)
        {
            if (byte < Overhead || byte > RadioTransmitter::MaxLength + Overhead)
            {
                active_ = false;
                return;
            }

            count_ = byte;
        }

        frame_[heard_++] = byte;

        if (heard_ == count_)
        {
            active_ = false;
            barrier ();
            full_ = true;
        }
    }
}

ISR (TIMER1_COMPA_vect)
{
    adk::RadioClock::tick ();
}
