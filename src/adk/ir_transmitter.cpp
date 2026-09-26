#include "ir_transmitter.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr uint8_t  IrTimer = 3;
        constexpr uint32_t Clock   = 16000000;              // the Mega's crystal
        constexpr uint16_t Period  = Clock / 38000 - 1;     // 38 kHz: 420 counts
        constexpr uint16_t Duty    = Period / 3 + 1;        // lit a third of the time

        // NEC, in microseconds: a 9 ms mark and 4.5 ms space to start, then
        // every bit a 562 µs mark and a space, short for 0 and long for 1,
        // and a last mark to close the final space. A repeat is shorter.
        constexpr uint16_t StartMark   = 9000;
        constexpr uint16_t StartSpace  = 4500;
        constexpr uint16_t RepeatSpace = 2250;
        constexpr uint16_t Mark        = 562;
        constexpr uint16_t Zero        = 562;
        constexpr uint16_t One         = 1687;
    }

    IrTransmitter::IrTransmitter (Pin pin)
        : output_ (0)
        , pin_    (pin)
        , ready_  (false)
    {
    }

    void IrTransmitter::setup ()
    {
        ready_ = false;

        switch (digitalPinToTimer (pin_))
        {
            case TIMER3A: output_ = _BV (COM3A1); break;
            case TIMER3B: output_ = _BV (COM3B1); break;
            case TIMER3C: output_ = _BV (COM3C1); break;
            default:      refuse (Fault::NotInfrared, pin_); return;
        }

        if (!claimOutput (pin_) || !claimTimer (IrTimer, pin_))
        {
            return;
        }

        // Fast PWM up to ICR3, undivided: the timer counts the 38 kHz period
        // and each compare register the third of it the LED is lit. The pin
        // only follows the timer while its output bit is set.
        TCCR3A = _BV (WGM31);
        TCCR3B = _BV (WGM33) | _BV (WGM32) | _BV (CS30);
        ICR3   = Period;
        OCR3A  = Duty;
        OCR3B  = Duty;
        OCR3C  = Duty;
        ready_ = true;
    }

    void IrTransmitter::send (uint8_t command, uint16_t address)
    {
        // The address, and its inverse unless it's a 16-bit one, then the
        // command and its inverse, least significant bit first.
        uint32_t bits = address > 0xFF ? address : (address | (~address & 0xFFu) << 8);
        bits         |= static_cast<uint32_t> (command) << 16;
        bits         |= static_cast<uint32_t> (static_cast<uint8_t> (~command)) << 24;

        uint16_t widths [2 + 2 * 32 + 1];
        uint8_t  count = 0;

        widths[count++] = StartMark;
        widths[count++] = StartSpace;

        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            widths[count++] = Mark;
            widths[count++] = ((bits >> bit) & 1) ? One : Zero;
        }

        widths[count++] = Mark;
        play (widths, count);
    }

    void IrTransmitter::repeat ()
    {
        constexpr uint16_t widths [] = {StartMark, RepeatSpace, Mark};
        play (widths, 3);
    }

    void IrTransmitter::stop ()
    {
        carrier (false);
    }

    void IrTransmitter::carrier (bool on)
    {
        if (!ready_)
        {
            return;
        }

        if (on)
        {
            TCCR3A |= output_;
        }
        else
        {
            TCCR3A &= static_cast<uint8_t> (~output_);
        }
    }

    // Marks and spaces in turn, each timed from the start, so small delays
    // in one don't add up over the frame.
    void IrTransmitter::play (const uint16_t* widths, uint8_t count)
    {
        unsigned long start = micros ();
        unsigned long end   = 0;

        for (uint8_t index = 0; index < count; ++index)
        {
            carrier (index % 2 == 0);
            end += widths[index];

            while (micros () - start < end)
            {
            }
        }

        carrier (false);
    }
}
