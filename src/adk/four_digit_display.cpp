#include "four_digit_display.h"

#include "segments.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr uint8_t Digits = 4;

        // Each digit stays lit this long, so the display refreshes 125 times
        // a second: well above what the eye sees as flicker.
        constexpr Millis DigitTime = 2;

        uint8_t digitGlyph (unsigned value)
        {
            return segments (static_cast<char> ('0' + value));
        }
    }

    FourDigitDisplay::FourDigitDisplay (Pin data, Pin clock, Pin latch,
                                        Pin digit1, Pin digit2, Pin digit3, Pin digit4)
        : switchedAt_ (0)
        , segments_   {data, clock, latch}
        , digits_     {digit1, digit2, digit3, digit4}
        , glyphs_     {0, 0, 0, 0}
        , current_    (Digits - 1)
        , starting_   (true)
    {
    }

    void FourDigitDisplay::setup ()
    {
        bool claimed = segments_.claim ();

        // A digit pin held high keeps its digit dark.
        for (Pin digit : digits_)
        {
            claimed = claimed && claimOutput (digit, true);
        }

        if (claimed)
        {
            segments_.write (0);
        }
    }

    void FourDigitDisplay::showNumber (long long number, uint8_t decimals)
    {
        if (number < -999 || number > 9999 || decimals >= Digits)
        {
            dashes ();
            return;
        }

        bool     negative  = number < 0;
        unsigned magnitude = static_cast<unsigned> (negative ? -number : number);
        uint8_t  position  = Digits;

        clear ();

        // Every decimal shows, and one digit before the dot, even if zero.
        do
        {
            glyphs_[--position] = digitGlyph (magnitude % 10);
            magnitude /= 10;
        }
        while (magnitude != 0 || Digits - position <= decimals);

        if (decimals > 0)
        {
            glyphs_[Digits - 1 - decimals] |= segment::dot;
        }

        if (negative && position == 0)
        {
            dashes ();
        }
        else if (negative)
        {
            glyphs_[--position] = segments ('-');
        }
    }

    void FourDigitDisplay::show (const char* text)
    {
        uint8_t position = 0;

        clear ();

        for (; text && *text; ++text)
        {
            bool dot = *text == '.';

            if (dot && position > 0 && !(glyphs_[position - 1] & segment::dot))
            {
                glyphs_[position - 1] |= segment::dot;
            }
            else if (position < Digits)
            {
                glyphs_[position++] = dot ? segment::dot : segments (*text);
            }
            else
            {
                return;
            }
        }
    }

    void FourDigitDisplay::showTime (uint8_t minutes, uint8_t seconds)
    {
        if (minutes > 99 || seconds > 59)
        {
            dashes ();
            return;
        }

        glyphs_[0] = digitGlyph (minutes / 10u);
        glyphs_[1] = digitGlyph (minutes % 10u) | segment::dot;
        glyphs_[2] = digitGlyph (seconds / 10u);
        glyphs_[3] = digitGlyph (seconds % 10u);
    }

    void FourDigitDisplay::clear ()
    {
        for (uint8_t& glyph : glyphs_)
        {
            glyph = 0;
        }
    }

    void FourDigitDisplay::update (Millis now)
    {
        if (!starting_ && now - switchedAt_ < DigitTime)
        {
            return;
        }

        starting_   = false;
        switchedAt_ = now;

        // Darken the lit digit before the segment lines change, or the next
        // digit's pattern would flash on it.
        digitalWrite    (digits_[current_], HIGH);
        current_ = static_cast<uint8_t> ((current_ + 1) % Digits);
        segments_.write (glyphs_[current_]);
        digitalWrite    (digits_[current_], LOW);
    }

    void FourDigitDisplay::stop ()
    {
        clear ();

        for (Pin digit : digits_)
        {
            digitalWrite (digit, HIGH);
        }

        segments_.write (0);
    }

    void FourDigitDisplay::dashes ()
    {
        for (uint8_t& glyph : glyphs_)
        {
            glyph = segments ('-');
        }
    }
}
