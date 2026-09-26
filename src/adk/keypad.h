#pragma once

#include "debouncer.h"
#include "object.h"

namespace adk {

    // The 4x4 membrane keypad. Its eight-wire ribbon, left to right with the
    // keys facing you, carries rows 1-4 then columns 1-4. Plug it into eight
    // Mega pins in a row and list them in the same order:
    //
    //     adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    //
    // The keys read "123A", "456B", "789C" and "*0#D", top row first. The
    // columns use the internal pull-ups; the rows float until the scan pulls
    // one at a time low, so two keys held together can never join two driven
    // pins. One key counts at a time: a key pressed while another is held is
    // reported when the first is let go.
    struct Keypad : Object
    {
        Keypad (const Pin (&rows) [4], const Pin (&columns) [4]);

        // The key pressed in this update, or '\0': an event, once per press.
        char key () const;

        // The key held down now, or '\0'.
        char heldKey   () const;
        bool isPressed (char key) const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        uint8_t read () const;

        Debouncer<uint8_t> held_;
        Pin                rows_    [4];
        Pin                columns_ [4];
        bool               pressed_;
    };
}
