#include "seven_segment_display.h"

namespace adk {

    namespace {
        uint8_t blankValue (SevenSegmentPolarity polarity) noexcept
        {
            return polarity == SevenSegmentPolarity::CommonCathode ? 0x00U
                                                                   : 0xffU;
        }

        bool validGlyph (SevenSegmentGlyph glyph) noexcept
        {
            return static_cast<uint8_t> (glyph) <=
                   static_cast<uint8_t> (SevenSegmentGlyph::Blank);
        }

        bool validPolarity (SevenSegmentPolarity polarity) noexcept
        {
            return polarity == SevenSegmentPolarity::CommonCathode ||
                   polarity == SevenSegmentPolarity::CommonAnode;
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
        if (!validPolarity (polarity_))
        {
            return Status::InvalidArgument;
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
        if (!validGlyph (glyph))
        {
            return Status::InvalidArgument;
        }

        uint8_t value = segmentPattern (glyph);

        if (decimalPoint)
        {
            value |= 0x80U;
        }

        if (polarity_ == SevenSegmentPolarity::CommonAnode)
        {
            value = static_cast<uint8_t> (~value);
        }

        const Status status = output_.show (value);

        if (status == Status::Ok)
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

    uint8_t SevenSegmentDisplay::segmentPattern (
        SevenSegmentGlyph glyph) noexcept
    {
        switch (glyph)
        {
        case SevenSegmentGlyph::Zero:  return 0x3fU;
        case SevenSegmentGlyph::One:   return 0x06U;
        case SevenSegmentGlyph::Two:   return 0x5bU;
        case SevenSegmentGlyph::Three: return 0x4fU;
        case SevenSegmentGlyph::Four:  return 0x66U;
        case SevenSegmentGlyph::Five:  return 0x6dU;
        case SevenSegmentGlyph::Six:   return 0x7dU;
        case SevenSegmentGlyph::Seven: return 0x07U;
        case SevenSegmentGlyph::Eight: return 0x7fU;
        case SevenSegmentGlyph::Nine:  return 0x6fU;
        case SevenSegmentGlyph::A:     return 0x77U;
        case SevenSegmentGlyph::B:     return 0x7cU;
        case SevenSegmentGlyph::C:     return 0x39U;
        case SevenSegmentGlyph::D:     return 0x5eU;
        case SevenSegmentGlyph::E:     return 0x79U;
        case SevenSegmentGlyph::F:     return 0x71U;
        case SevenSegmentGlyph::Dash:  return 0x40U;
        case SevenSegmentGlyph::Blank: return 0x00U;
        }

        return 0x00U;
    }
}
