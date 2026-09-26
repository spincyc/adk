#pragma once

#include "object.h"
#include "shift_register.h"

namespace adk {

    // A four-digit seven-segment display (5461AS, common cathode). The digits
    // share their segment lines, so it lights one digit at a time and moves
    // on every 2 ms, fast enough that all four look steady. That needs
    // adk::update () at least every 2 ms (adk::wait () updates about every
    // millisecond); code that blocks for longer leaves one digit lit, and the
    // display flickers.
    //
    // The segment lines come from a 74HC595 wired as for a ShiftRegister,
    // each output through a 1k ohm resistor. The digit pins go straight to
    // the Mega, which lights a digit by pulling its pin low. That pin sinks
    // the current of every lit segment; the resistors hold it to about 25 mA
    // with all eight lit, under the 40 mA limit of a Mega pin.
    //
    //   74HC595      Q0  Q1  Q2  Q3  Q4  Q5  Q6  Q7
    //   Segment      a   b   c   d   e   f   g   dp
    //   5461AS pin   11  7   4   2   1   10  5   3
    //
    //   Digit        D1  D2  D3  D4
    //   5461AS pin   12  9   8   6     each to its own Mega pin
    struct FourDigitDisplay : Object
    {
        FourDigitDisplay (Pin data, Pin clock, Pin latch,
                          Pin digit1, Pin digit2, Pin digit3, Pin digit4);

        // A whole number of any type, right-aligned without leading zeros,
        // from -999 to 9999. Anything else shows ----. With decimals, the
        // last digits go after the dot, as Serial.print () writes decimals:
        // show (123, 1) shows 12.3, and show (5, 1) shows 0.5.
        void show (auto number, uint8_t decimals = 0)
        {
            // A fraction would be dropped without a word, so refuse one.
            static_assert (decltype (number) (1) / 2 == 0,
                           "show () takes a whole number: for 21.5, show (215, 1)");

            // Checked at full width, so 70000 shows ---- rather than the
            // 4464 left of it in the Mega's 16-bit int.
            showNumber (static_cast<long long> (number), decimals);
        }

        // The first four characters, left-aligned. A '.' lights the dot of
        // the character before it instead of taking a digit of its own.
        void show (const char* text);

        // Minutes and seconds as 05.09, the dot after the second digit
        // standing in for a colon. Past 99 minutes or 59 seconds shows ----.
        void showTime (uint8_t minutes, uint8_t seconds);

        void clear ();

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void showNumber (long long number, uint8_t decimals);
        void dashes     ();

        Millis    switchedAt_;
        ShiftPins segments_;
        Pin       digits_ [4];
        uint8_t   glyphs_ [4];
        uint8_t   current_;
        bool      starting_;
    };
}
