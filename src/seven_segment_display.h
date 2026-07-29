#pragma once

#include "seven_segment_glyph.h"
#include "shift_register.h"

#include <stdint.h>

namespace adk {

    // Q0..Q6 drive segments a..g; Q7 drives the decimal point.
    struct SevenSegmentDisplay
    {
        SevenSegmentDisplay (ResourceRegistry&         resources,
                             const ShiftRegisterPins&  pins,
                             SevenSegmentPolarity      polarity) noexcept;
        ~SevenSegmentDisplay () noexcept;

        SevenSegmentDisplay& operator= (const SevenSegmentDisplay&) = delete;
        SevenSegmentDisplay  (const SevenSegmentDisplay&)           = delete;
        SevenSegmentDisplay& operator= (SevenSegmentDisplay&&)      = delete;
        SevenSegmentDisplay  (SevenSegmentDisplay&&)                = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status show  (SevenSegmentGlyph glyph,
                      bool              decimalPoint = false) noexcept;
        Status blank () noexcept;

        SevenSegmentGlyph    glyph        () const noexcept;
        SevenSegmentPolarity polarity     () const noexcept;
        bool                 decimalPoint () const noexcept;
        uint8_t              encodedValue () const noexcept;
        bool                 initialized  () const noexcept;

      private:
        ShiftRegisterOutput  output_;
        SevenSegmentPolarity polarity_;
        SevenSegmentGlyph    glyph_;
        bool                 decimalPoint_;
    };
}
