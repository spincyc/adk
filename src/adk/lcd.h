#pragma once

#include "object.h"

#include <Arduino.h>

namespace adk {

    // A 16x2 character LCD with an HD44780 controller, such as the LCD1602,
    // written over four data wires. Wiring, by the module's pin labels:
    //
    //   VSS -> GND                 VDD -> 5 V
    //   V0  -> the wiper of a 10 kohm potentiometer between 5 V and GND
    //   RS  -> rs                  RW  -> GND (the display is only written)
    //   E   -> enable              D0-D3 unconnected
    //   D4  -> d4 ... D7 -> d7
    //   A   -> 5 V through 220 ohm (backlight), K -> GND
    //
    // The potentiometer sets the contrast: turn it until the text is sharp. A
    // top row of solid blocks means the display has power but setup () has not
    // reached it; blank with the backlight on usually means the contrast.
    //
    // An Lcd is a Print, so lcd.print (temperature, 1) works. '\n' moves to
    // the start of the other row and '\r' is ignored, so println () does the
    // same. Text past column 15 goes on into the row's hidden columns, as the
    // controller does, rather than wrapping.
    //
    // setup () takes about 60 ms, each character about 0.1 ms, and clear ()
    // and home () 2 ms. adk::stop () leaves the text showing: the display
    // drives nothing, and the last message often says why the sketch stopped.
    struct Lcd : Object, Print
    {
        Lcd (Pin rs, Pin enable, Pin d4, Pin d5, Pin d6, Pin d7);

        void clear     ();
        void home      ();
        void setCursor (uint8_t column, uint8_t row);

        // Draw character 0-7 from eight rows of five dots, bit 4 the leftmost
        // dot. Show it with write (slot); slot 0 needs write (uint8_t (0)),
        // because a bare 0 could also be a null text pointer.
        void createChar (uint8_t slot, const uint8_t rows [8]);

        size_t write (uint8_t character) override;
        using Print::write;

      protected:
        void setup () override;

      private:
        void moveTo (uint8_t address);
        void send   (uint8_t value, uint8_t rs);
        void pulse  (uint8_t nibble);

        Pin     rs_;
        Pin     enable_;
        Pin     data_ [4];
        uint8_t address_;
    };
}
