#include "ds18b20.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr uint8_t SkipRom        = 0xCC;
        constexpr uint8_t ConvertT       = 0x44;
        constexpr uint8_t ReadScratchpad = 0xBE;

        // A 12-bit conversion takes up to 750 ms.
        constexpr Millis ConversionTime = 750;

        // What the sensor holds from power-up until its first conversion.
        constexpr int16_t PowerOnValue = 85 * 16;

        // The Dallas CRC-8 (x^8 + x^5 + x^4 + 1), least significant bit first.
        uint8_t crc8 (const uint8_t* bytes, uint8_t length)
        {
            uint8_t crc = 0;

            for (uint8_t index = 0; index < length; ++index)
            {
                uint8_t byte = bytes[index];

                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    bool mix = (crc ^ byte) & 1;
                    crc      = static_cast<uint8_t> (crc >> 1);
                    byte     = static_cast<uint8_t> (byte >> 1);

                    if (mix)
                    {
                        crc ^= 0x8C;
                    }
                }
            }

            return crc;
        }
    }

    Ds18b20::Ds18b20 (Pin pin)
        : startedAt_  (0)
        , sixteenths_ (0)
        , pin_        (pin)
        , phase_      (Phase::Starting)
        , ok_         (false)
        , measured_   (false)
        , fresh_      (true)
    {
    }

    void Ds18b20::setup ()
    {
        claimInput (pin_, false);
    }

    float Ds18b20::celsius () const
    {
        return sixteenths_ * 0.0625f;
    }

    bool Ds18b20::ok () const
    {
        return ok_;
    }

    bool Ds18b20::measured () const
    {
        return measured_;
    }

    // The sensor converts on its own once told to. Each conversion's result
    // is collected 750 ms later, and the next conversion started at once.
    void Ds18b20::update (Millis now)
    {
        measured_ = false;

        if (phase_ != Phase::Starting && now - startedAt_ < ConversionTime)
        {
            return;
        }

        if (phase_ == Phase::Converting)
        {
            ok_       = collect ();
            measured_ = true;
        }

        startedAt_ = now;
        phase_     = command (ConvertT) ? Phase::Converting : Phase::Missing;

        // With no sensor to start one, a reading fails at once, unless one
        // has just finished.
        if (phase_ == Phase::Missing && !measured_)
        {
            ok_       = false;
            measured_ = true;
        }
    }

    // The scratchpad holds the temperature in sixteenths of a degree, least
    // significant byte first, and a CRC of its first eight bytes in the ninth.
    bool Ds18b20::collect ()
    {
        uint8_t scratchpad [9];
        uint8_t any = 0;

        if (!command (ReadScratchpad))
        {
            return false;
        }

        for (uint8_t index = 0; index < 9; ++index)
        {
            scratchpad[index] = touch (0xFF);
            any              |= scratchpad[index];
        }

        // Nine zero bytes pass the CRC too; they are a line held low.
        if (any == 0 || crc8 (scratchpad, 8) != scratchpad[8])
        {
            return false;
        }

        int16_t sixteenths = static_cast<int16_t> (scratchpad[1] << 8 | scratchpad[0]);

        // A first reading of exactly 85 degrees is most likely the power-up
        // value, left by a conversion that never ran.
        bool leftOver = fresh_ && sixteenths == PowerOnValue;
        fresh_        = false;

        if (leftOver)
        {
            return false;
        }

        sixteenths_ = sixteenths;
        return true;
    }

    // Reset the line and, if the sensor answers, give it a function command.
    // Skip ROM addresses the one sensor on the line without its ROM code.
    bool Ds18b20::command (uint8_t function)
    {
        if (!reset ())
        {
            return false;
        }

        touch (SkipRom);
        touch (function);
        return true;
    }

    // Hold the line low for 480 us. A sensor answers by pulling it low
    // itself, starting 15-60 us after the line is let go and lasting 60-240
    // us, so it is low from 60 to 75 us after release whatever the sensor's
    // timing. On the Mega the sample falls 67-71 us after release.
    bool Ds18b20::reset ()
    {
        drive ();
        delayMicroseconds (480);

        noInterrupts ();
        release ();
        delayMicroseconds (64);
        bool present = digitalRead (pin_) == LOW;
        interrupts ();

        delayMicroseconds (410);
        return present;
    }

    // One time slot per bit, least significant first. Every slot pulls the
    // line low; a 1 lets go about 6 us in, and a 0 holds on for about 65.
    // Both then sample the line, which is how a 1 slot reads a bit: a sensor
    // answering 0 holds the line low until at least 15 us in, so touch (0xFF)
    // reads a byte. The delays allow for pinMode () and digitalRead () taking
    // about 3 us each on the Mega, which puts the sample 10-15 us in.
    uint8_t Ds18b20::touch (uint8_t byte)
    {
        uint8_t seen = 0;

        for (uint8_t bit = 1; bit != 0; bit = static_cast<uint8_t> (bit << 1))
        {
            bool one = byte & bit;

            noInterrupts ();
            drive ();
            delayMicroseconds (3);

            if (one)
            {
                release ();
            }

            bool high = digitalRead (pin_) == HIGH;

            if (!one)
            {
                delayMicroseconds (55);
                release ();
            }

            interrupts ();
            delayMicroseconds (one ? 55 : 10);

            if (high)
            {
                seen |= bit;
            }
        }

        return seen;
    }

    // The line is open drain: the pin pulls it low as an output, and lets go
    // as an input for the pull-up to raise. Setting the level before the
    // direction means the pin never drives the line high.
    void Ds18b20::drive ()
    {
        digitalWrite (pin_, LOW);
        pinMode      (pin_, OUTPUT);
    }

    void Ds18b20::release ()
    {
        pinMode (pin_, INPUT);
    }
}
