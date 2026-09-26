#include "shift_register.h"

#include <Arduino.h>

namespace adk {

    bool ShiftPins::claim () const
    {
        return claimOutput (data) && claimOutput (clock) && claimOutput (latch);
    }

    void ShiftPins::write (uint8_t bits) const
    {
        digitalWrite (latch, LOW);
        shiftOut     (data, clock, MSBFIRST, bits);
        digitalWrite (latch, HIGH);
    }

    ShiftRegister::ShiftRegister (Pin data, Pin clock, Pin latch)
        : pins_ {data, clock, latch}
        , bits_ (0)
    {
    }

    void ShiftRegister::setup ()
    {
        if (pins_.claim ())
        {
            write (bits_);
        }
    }

    void ShiftRegister::write (uint8_t bits)
    {
        pins_.write (bits);
        bits_ = bits;
    }

    uint8_t ShiftRegister::bits () const
    {
        return bits_;
    }

    void ShiftRegister::stop ()
    {
        write (0);
    }
}
