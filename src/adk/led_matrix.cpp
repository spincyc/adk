#include "led_matrix.h"

#include "font.h"

#include <Arduino.h>
#include <string.h>

namespace adk {

    namespace {

        // MAX7219 registers. Rows 0-7 are the digit registers 0x01-0x08.
        const uint8_t DecodeMode  = 0x09;
        const uint8_t Intensity   = 0x0A;
        const uint8_t ScanLimit   = 0x0B;
        const uint8_t Shutdown    = 0x0C;
        const uint8_t DisplayTest = 0x0F;

        const uint8_t Size           = 8;
        const uint8_t FontHeight     = 7;
        const uint8_t CharacterWidth = 6;   // five font columns and a gap
        const uint8_t MaxLevel       = 15;
    }

    LedMatrix::LedMatrix (Pin data, Pin clock, Pin load)
        : text_         (nullptr)
        , step_         (0)
        , stepStart_    (0)
        , offset_       (0)
        , rows_         {}
        , dirty_        (0)
        , level_        (7)
        , data_         (data)
        , clock_        (clock)
        , load_         (load)
        , levelChanged_ (false)
        , starting_     (false)
    {
    }

    void LedMatrix::setup ()
    {
        if (!claimOutput (data_) || !claimOutput (clock_) || !claimOutput (load_, true))
        {
            return;
        }

        // The chip wakes in shutdown with random rows, and resetting the Mega
        // leaves it however the last sketch left it. Set every register and
        // send the whole picture before it starts showing anything.
        send (DisplayTest, 0x00);
        send (ScanLimit,   0x07);
        send (DecodeMode,  0x00);

        levelChanged_ = true;
        dirty_        = 0xFF;
        flush ();

        send (Shutdown, 0x01);
    }

    void LedMatrix::clear ()
    {
        text_ = nullptr;

        for (uint8_t y = 0; y < Size; ++y)
        {
            draw (y, 0);
        }
    }

    void LedMatrix::set (uint8_t x, uint8_t y, bool lit)
    {
        if (x >= Size || y >= Size)
        {
            return;
        }

        uint8_t mask = static_cast<uint8_t> (0x80 >> x);

        text_ = nullptr;
        draw (y, lit ? (rows_[y] | mask) : (rows_[y] & ~mask));
    }

    bool LedMatrix::get (uint8_t x, uint8_t y) const
    {
        return x < Size && y < Size && (rows_[y] & (0x80 >> x));
    }

    void LedMatrix::row (uint8_t y, uint8_t bits)
    {
        if (y < Size)
        {
            text_ = nullptr;
            draw (y, bits);
        }
    }

    void LedMatrix::show (const uint8_t rows [8])
    {
        text_ = nullptr;

        for (uint8_t y = 0; y < Size; ++y)
        {
            draw (y, rows[y]);
        }
    }

    void LedMatrix::brightness (uint8_t level)
    {
        level = level < MaxLevel ? level : MaxLevel;

        if (level != level_)
        {
            level_        = level;
            levelChanged_ = true;
        }
    }

    void LedMatrix::scroll (const char* text, Millis step)
    {
        if (text == text_ && step == step_)
        {
            return;
        }

        text_     = (text && *text) ? text : nullptr;
        step_     = step;
        offset_   = 0;
        starting_ = true;
    }

    bool LedMatrix::isScrolling () const
    {
        return text_ != nullptr;
    }

    void LedMatrix::update (Millis now)
    {
        if (text_ && (starting_ || now - stepStart_ >= step_))
        {
            stepStart_ = now;
            starting_  = false;
            advance ();
        }

        flush ();
    }

    void LedMatrix::stop ()
    {
        clear ();
        flush ();
    }

    void LedMatrix::draw (uint8_t y, uint8_t bits)
    {
        if (rows_[y] != bits)
        {
            rows_[y] = bits;
            dirty_   = static_cast<uint8_t> (dirty_ | (1 << y));
        }
    }

    void LedMatrix::advance ()
    {
        // Text column c is drawn at x = c + 8 - offset_, so the first step
        // brings column 0 in at x 7, and the text has gone once its last lit
        // column has passed x 0.
        uint16_t length = static_cast<uint16_t> (strlen (text_));
        uint8_t  picture [Size] = {};

        ++offset_;

        for (uint8_t x = 0; x < Size; ++x)
        {
            if (offset_ + x < Size)
            {
                continue;
            }

            uint16_t column    = static_cast<uint16_t> (offset_ + x - Size);
            uint16_t character = static_cast<uint16_t> (column / CharacterWidth);
            uint8_t  dot       = static_cast<uint8_t> (column % CharacterWidth);

            if (character >= length)
            {
                break;
            }

            uint8_t dots = fontColumn (text_[character], dot);

            for (uint8_t y = 0; y < FontHeight; ++y)
            {
                if (dots & (1 << y))
                {
                    picture[y] = static_cast<uint8_t> (picture[y] | (0x80 >> x));
                }
            }
        }

        for (uint8_t y = 0; y < Size; ++y)
        {
            draw (y, picture[y]);
        }

        if (offset_ >= length * CharacterWidth - 1 + Size)
        {
            text_ = nullptr;
        }
    }

    void LedMatrix::flush ()
    {
        if (levelChanged_)
        {
            send (Intensity, level_);
            levelChanged_ = false;
        }

        for (uint8_t y = 0; y < Size; ++y)
        {
            if (dirty_ & (1 << y))
            {
                send (static_cast<uint8_t> (y + 1), rows_[y]);
            }
        }

        dirty_ = 0;
    }

    void LedMatrix::send (uint8_t address, uint8_t value)
    {
        // The chip takes the last 16 bits clocked in as load rises.
        digitalWrite (load_, LOW);
        shiftOut     (data_, clock_, MSBFIRST, address);
        shiftOut     (data_, clock_, MSBFIRST, value);
        digitalWrite (load_, HIGH);
    }
}
