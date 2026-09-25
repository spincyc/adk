#pragma once

#include "object.h"

namespace adk {

    // A 74HC595 shift register: eight outputs, Q0-Q7, from three Mega pins.
    // Wire it with the notch up, pin 1 at the top left:
    //
    //   74HC595                 Wire to
    //   DS    (14)              the data pin
    //   SH_CP (11)              the clock pin
    //   ST_CP (12)              the latch pin
    //   MR    (10)              5 V; reset is active low
    //   OE    (13)              GND; output enable is active low
    //   VCC   (16), GND (8)     5 V and GND, with a 100 nF capacitor across them
    //   Q0 (15), Q1-Q7 (1-7)    the outputs
    //
    // Until setup () shifts in zeros, the outputs hold whatever the chip
    // powered up with.
    struct ShiftRegister : Object
    {
        ShiftRegister (Pin data, Pin clock, Pin latch);

        // Set all eight outputs: bit 7 drives Q7, bit 0 drives Q0.
        void    write (uint8_t bits);
        uint8_t bits  () const;

      protected:
        void setup () override;
        void stop  () override;

      private:
        Pin     data_;
        Pin     clock_;
        Pin     latch_;
        uint8_t bits_;
    };

    // Shift eight bits into a 74HC595, bit 7 first so that it lands on Q7,
    // then raise the latch to copy all eight to the outputs at once.
    void shiftByte (Pin data, Pin clock, Pin latch, uint8_t bits);
}
