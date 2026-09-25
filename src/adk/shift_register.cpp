#include "shift_register.h"

#include <Arduino.h>

namespace adk {

    void shiftByte (Pin data, Pin clock, Pin latch, uint8_t bits)
    {
        digitalWrite (latch, LOW);
        shiftOut     (data, clock, MSBFIRST, bits);
        digitalWrite (latch, HIGH);
    }

    ShiftRegister::ShiftRegister (Pin data, Pin clock, Pin latch)
        : data_  (data)
        , clock_ (clock)
        , latch_ (latch)
        , bits_  (0)
    {
    }

    void ShiftRegister::setup ()
    {
        if (claimOutput (data_) && claimOutput (clock_) && claimOutput (latch_))
        {
            write (bits_);
        }
    }

    void ShiftRegister::write (uint8_t bits)
    {
        shiftByte (data_, clock_, latch_, bits);
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
