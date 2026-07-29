#pragma once

#include <stdint.h>

namespace adk {

    enum struct SevenSegmentPolarity : uint8_t
    {
        CommonCathode,
        CommonAnode
    };

    enum struct SevenSegmentGlyph : uint8_t
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        A,
        B,
        C,
        D,
        E,
        F,
        Dash,
        Blank
    };

    inline bool validSevenSegmentGlyph (
        SevenSegmentGlyph glyph) noexcept
    {
        return static_cast<uint8_t> (glyph) <=
               static_cast<uint8_t> (SevenSegmentGlyph::Blank);
    }

    inline bool validSevenSegmentPolarity (
        SevenSegmentPolarity polarity) noexcept
    {
        return polarity == SevenSegmentPolarity::CommonCathode ||
               polarity == SevenSegmentPolarity::CommonAnode;
    }

    // Invalid glyphs encode as an inactive byte; callers validate first.
    inline uint8_t encodeSevenSegmentGlyph (
        SevenSegmentGlyph    glyph,
        SevenSegmentPolarity polarity,
        bool                 decimalPoint = false) noexcept
    {
        if (!validSevenSegmentGlyph (glyph))
        {
            return polarity == SevenSegmentPolarity::CommonCathode ? 0x00U
                                                                   : 0xffU;
        }

        uint8_t value = 0x00U;

        switch (glyph)
        {
        case SevenSegmentGlyph::Zero:  value = 0x3fU; break;
        case SevenSegmentGlyph::One:   value = 0x06U; break;
        case SevenSegmentGlyph::Two:   value = 0x5bU; break;
        case SevenSegmentGlyph::Three: value = 0x4fU; break;
        case SevenSegmentGlyph::Four:  value = 0x66U; break;
        case SevenSegmentGlyph::Five:  value = 0x6dU; break;
        case SevenSegmentGlyph::Six:   value = 0x7dU; break;
        case SevenSegmentGlyph::Seven: value = 0x07U; break;
        case SevenSegmentGlyph::Eight: value = 0x7fU; break;
        case SevenSegmentGlyph::Nine:  value = 0x6fU; break;
        case SevenSegmentGlyph::A:     value = 0x77U; break;
        case SevenSegmentGlyph::B:     value = 0x7cU; break;
        case SevenSegmentGlyph::C:     value = 0x39U; break;
        case SevenSegmentGlyph::D:     value = 0x5eU; break;
        case SevenSegmentGlyph::E:     value = 0x79U; break;
        case SevenSegmentGlyph::F:     value = 0x71U; break;
        case SevenSegmentGlyph::Dash:  value = 0x40U; break;
        case SevenSegmentGlyph::Blank: value = 0x00U; break;
        }

        if (decimalPoint)
        {
            value |= 0x80U;
        }

        return polarity == SevenSegmentPolarity::CommonAnode
                   ? static_cast<uint8_t> (~value)
                   : value;
    }
}
