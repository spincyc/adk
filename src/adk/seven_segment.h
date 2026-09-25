#pragma once

#include "digital.h"

namespace adk {

    // One seven-segment digit (5161AS, common cathode) behind a 74HC595, so it
    // needs three Mega pins instead of eight. Wire the 74HC595 as for a
    // ShiftRegister, then each output through its own 1 kohm resistor. (With
    // 220 ohm, an "8." would draw about 90 mA through the 74HC595, beyond
    // the 70 mA its supply pins are rated for.)
    //
    //   74HC595      Q0  Q1  Q2  Q3  Q4  Q5  Q6  Q7
    //   Segment      a   b   c   d   e   f   g   dp
    //   5161AS pin   7   6   4   2   1   9   10  5
    //
    // Both common pins, 3 and 8, go to GND. A common-anode digit (5161BS)
    // has its common pins at 5 V instead, and is ActiveLow.
    struct SevenSegment : Object
    {
        SevenSegment (Pin data, Pin clock, Pin latch, Polarity polarity = ActiveHigh);

        // 0-9, and 10-15 as A-F. Anything else shows a dash.
        void show (int digit);

        // A character as segments () draws it; one it cannot draw is blank.
        void show (char character);

        // The decimal point, which show () leaves as it is.
        void dot   (bool lit);
        void clear ();

      protected:
        void setup () override;
        void stop  () override;

      private:
        void write ();

        Pin      data_;
        Pin      clock_;
        Pin      latch_;
        Polarity polarity_;
        uint8_t  glyph_;
        bool     dot_;
    };
}
