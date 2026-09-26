#include "keypad.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr uint8_t Debounce = 20;

        // Key codes count along each row in turn, as the keys are printed.
        // NoKey indexes the terminating '\0'.
        constexpr char    Keys [] PROGMEM = "123A456B789C*0#D";
        constexpr uint8_t NoKey           = 16;
    }

    Keypad::Keypad (const Pin (&rows) [4], const Pin (&columns) [4])
        : held_    (NoKey)
        , rows_    {rows[0], rows[1], rows[2], rows[3]}
        , columns_ {columns[0], columns[1], columns[2], columns[3]}
        , pressed_ (false)
    {
    }

    void Keypad::setup ()
    {
        bool claimed = true;

        for (Pin row : rows_)
        {
            claimed = claimed && claimInput (row);
        }

        for (Pin column : columns_)
        {
            claimed = claimed && claimInput (column, true);
        }

        if (!claimed)
        {
            return;
        }

        // A key held while the sketch starts is held, but was never pressed.
        held_    = Debouncer<uint8_t> {read ()};
        pressed_ = false;
    }

    void Keypad::update (Millis now)
    {
        // A new key counts once it has read the same for a whole window.
        pressed_ = held_.sample (read (), now, Debounce) && held_.stable () != NoKey;
    }

    char Keypad::key () const
    {
        return pressed_ ? heldKey () : '\0';
    }

    char Keypad::heldKey () const
    {
        return static_cast<char> (pgm_read_byte (&Keys[held_.stable ()]));
    }

    bool Keypad::isPressed (char key) const
    {
        return key != '\0' && heldKey () == key;
    }

    // Each row in turn is set low and made an output, the columns it pulls
    // low are its held keys, and it goes back to floating. Setting the level
    // first means a row is never driven high, even for an instant.
    uint8_t Keypad::read () const
    {
        uint8_t first    = NoKey;
        bool    heldDown = false;

        for (uint8_t row = 0; row < 4; ++row)
        {
            digitalWrite (rows_[row], LOW);
            pinMode      (rows_[row], OUTPUT);

            for (uint8_t column = 0; column < 4; ++column)
            {
                if (digitalRead (columns_[column]) == LOW)
                {
                    uint8_t code = static_cast<uint8_t> (row * 4 + column);

                    heldDown = heldDown || code == held_.stable ();
                    first    = (first == NoKey) ? code : first;
                }
            }

            pinMode (rows_[row], INPUT);
        }

        // The key already held keeps its place while it stays down.
        return heldDown ? held_.stable () : first;
    }
}
