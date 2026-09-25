#include "seven_segment.h"

#include "segments.h"
#include "shift_register.h"

namespace adk {

    SevenSegment::SevenSegment (Pin data, Pin clock, Pin latch, Polarity polarity)
        : data_     (data)
        , clock_    (clock)
        , latch_    (latch)
        , polarity_ (polarity)
        , glyph_    (0)
        , dot_      (false)
    {
    }

    void SevenSegment::setup ()
    {
        if (claimOutput (data_) && claimOutput (clock_) && claimOutput (latch_))
        {
            write ();
        }
    }

    void SevenSegment::show (int digit)
    {
        if (digit < 0 || digit > 15)
        {
            show ('-');
            return;
        }

        show (static_cast<char> (digit < 10 ? '0' + digit : 'A' + digit - 10));
    }

    void SevenSegment::show (char character)
    {
        glyph_ = segments (character);
        write ();
    }

    void SevenSegment::dot (bool lit)
    {
        dot_ = lit;
        write ();
    }

    void SevenSegment::clear ()
    {
        glyph_ = 0;
        dot_   = false;
        write ();
    }

    void SevenSegment::stop ()
    {
        clear ();
    }

    void SevenSegment::write ()
    {
        uint8_t lit = dot_ ? glyph_ | segment::dot : glyph_;

        // A common-anode digit lights a segment by pulling its line low.
        if (polarity_ == ActiveLow)
        {
            lit = static_cast<uint8_t> (~lit);
        }

        shiftByte (data_, clock_, latch_, lit);
    }
}
