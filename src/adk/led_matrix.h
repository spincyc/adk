#pragma once

#include "object.h"

namespace adk {

    // An 8x8 LED matrix on a MAX7219 module. Wiring, from the module's input
    // end (the pins labeled DIN, not DOUT):
    //
    //   VCC -> 5 V     DIN -> data     CLK -> clock
    //   GND -> GND     CS  -> load
    //
    // The module sets the LED current with its own resistor, so the LEDs need
    // none. With every LED lit at full brightness it can draw about 330 mA,
    // most of what USB supplies; the default brightness, 7, takes about half.
    //
    // x runs 0-7 left to right and y 0-7 top to bottom. Modules are built in
    // different orientations: if set (0, 0) lights another corner, turn the
    // module until it is top left.
    //
    // Drawing changes a picture held in RAM, and the next update () sends the
    // rows that changed, about 0.2 ms each. Drawing also ends a scroll.
    struct LedMatrix : Object
    {
        LedMatrix (Pin data, Pin clock, Pin load);

        void clear ();
        void set   (uint8_t x, uint8_t y, bool lit = true);
        bool get   (uint8_t x, uint8_t y) const;

        // One row as eight bits, bit 7 at x 0 and bit 0 at x 7.
        void row (uint8_t y, uint8_t bits);

        // A whole picture, eight rows from top to bottom.
        void show (const uint8_t rows [8]);

        // 0 is dim, not off, and 15 is brightest.
        void brightness (uint8_t level);

        // Scroll text in from the right edge until it has left the left
        // edge, one column every step. The text is not copied, so it must
        // outlast the scroll: a string literal is ideal. Asking again for the
        // same text while it scrolls changes nothing, so scroll () can be
        // called from every pass of loop ().
        void scroll      (const char* text, Millis step = 80);
        bool isScrolling () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void draw    (uint8_t y, uint8_t bits);
        void advance ();
        void flush   ();
        void send    (uint8_t address, uint8_t value);

        const char* text_;
        Millis      step_;
        Millis      stepStart_;
        uint16_t    offset_;
        uint8_t     rows_ [8];
        uint8_t     dirty_;
        uint8_t     level_;
        Pin         data_;
        Pin         clock_;
        Pin         load_;
        bool        levelChanged_;
        bool        starting_;
    };
}
