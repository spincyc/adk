#include "seven_segment_display.h"

namespace adk {

    namespace {
        uint8_t blankValue (SevenSegmentPolarity polarity) noexcept
        {
            return polarity == SevenSegmentPolarity::CommonCathode ? 0x00U
                                                                   : 0xffU;
        }
    }

    SevenSegmentDisplay::SevenSegmentDisplay (
        ResourceRegistry&        resources,
        const ShiftRegisterPins& pins,
        SevenSegmentPolarity     polarity) noexcept
        : output_       (resources, pins, blankValue (polarity))
        , polarity_     (polarity)
        , glyph_        (SevenSegmentGlyph::Blank)
        , decimalPoint_ (false)
    {
    }

    SevenSegmentDisplay::~SevenSegmentDisplay () noexcept
    {
        shutdown ();
    }

    Status SevenSegmentDisplay::initialize () noexcept
    {
        if (!validSevenSegmentPolarity (polarity_))
        {
            return StatusCode::InvalidArgument;
        }

        return output_.initialize ();
    }

    void SevenSegmentDisplay::shutdown () noexcept
    {
        output_.shutdown ();
        glyph_        = SevenSegmentGlyph::Blank;
        decimalPoint_ = false;
    }

    Status SevenSegmentDisplay::show (
        SevenSegmentGlyph glyph,
        bool              decimalPoint) noexcept
    {
        if (!validSevenSegmentGlyph (glyph))
        {
            return StatusCode::InvalidArgument;
        }

        const uint8_t value =
            encodeSevenSegmentGlyph (glyph, polarity_, decimalPoint);

        const Status status = output_.show (value);

        if (status.ok ())
        {
            glyph_        = glyph;
            decimalPoint_ = decimalPoint;
        }

        return status;
    }

    Status SevenSegmentDisplay::blank () noexcept
    {
        return show (SevenSegmentGlyph::Blank);
    }

    SevenSegmentGlyph SevenSegmentDisplay::glyph () const noexcept
    {
        return glyph_;
    }

    SevenSegmentPolarity SevenSegmentDisplay::polarity () const noexcept
    {
        return polarity_;
    }

    bool SevenSegmentDisplay::decimalPoint () const noexcept
    {
        return decimalPoint_;
    }

    uint8_t SevenSegmentDisplay::encodedValue () const noexcept
    {
        return output_.value ();
    }

    bool SevenSegmentDisplay::initialized () const noexcept
    {
        return output_.initialized ();
    }

}
