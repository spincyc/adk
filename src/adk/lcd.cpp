#include "lcd.h"

namespace adk {

    namespace {

        // HD44780 instructions, from the datasheet's instruction table.
        const uint8_t ClearDisplay = 0x01;
        const uint8_t ReturnHome   = 0x02;
        const uint8_t EntryMode    = 0x06;   // move right after each character
        const uint8_t DisplayOn    = 0x0C;   // no cursor, no blink
        const uint8_t FunctionSet  = 0x28;   // 4-bit bus, two lines, 5x8 dots
        const uint8_t SetCgram     = 0x40;
        const uint8_t SetDdram     = 0x80;

        // Each row holds 40 characters in display RAM, 16 of them on screen.
        // Row 0 starts at address 0x00 and row 1 at 0x40.
        const uint8_t Columns   = 16;
        const uint8_t RowLength = 40;
        const uint8_t SecondRow = 0x40;
    }

    Lcd::Lcd (Pin rs, Pin enable, Pin d4, Pin d5, Pin d6, Pin d7)
        : rs_      (rs)
        , enable_  (enable)
        , data_    {d4, d5, d6, d7}
        , address_ (0)
    {
    }

    void Lcd::setup ()
    {
        if (!claimOutput (rs_) || !claimOutput (enable_))
        {
            return;
        }

        for (Pin pin : data_)
        {
            if (!claimOutput (pin))
            {
                return;
            }
        }

        // Initialising by instruction, as the datasheet's 4-bit flowchart
        // gives it. The controller may have woken in 8-bit mode or been reset
        // halfway through a byte, so three 0x3 nibbles force 8-bit mode before
        // 0x2 selects the 4-bit bus. It needs 40 ms after power-up first.
        delay (50);
        pulse (0x3);
        delay (5);
        pulse (0x3);
        delayMicroseconds (150);
        pulse (0x3);
        delayMicroseconds (150);
        pulse (0x2);
        delayMicroseconds (50);

        send  (FunctionSet, LOW);
        send  (DisplayOn,   LOW);
        clear ();
        send  (EntryMode,   LOW);
    }

    void Lcd::clear ()
    {
        send  (ClearDisplay, LOW);
        delay (2);   // clear and home take 1.52 ms
        address_ = 0;
    }

    void Lcd::home ()
    {
        send  (ReturnHome, LOW);
        delay (2);
        address_ = 0;
    }

    void Lcd::setCursor (uint8_t column, uint8_t row)
    {
        column = column < Columns ? column : Columns - 1;
        moveTo (static_cast<uint8_t> ((row == 0 ? 0 : SecondRow) + column));
    }

    void Lcd::createChar (uint8_t slot, const uint8_t rows [8])
    {
        send (static_cast<uint8_t> (SetCgram | ((slot & 0x07) << 3)), LOW);

        for (uint8_t line = 0; line < 8; ++line)
        {
            send (rows[line], HIGH);
        }

        // Characters go to the pattern memory until an address in display
        // memory is set again.
        moveTo (address_);
    }

    size_t Lcd::write (uint8_t character)
    {
        if (character == '\n')
        {
            moveTo (address_ < SecondRow ? SecondRow : 0);
        }
        else if (character != '\r')
        {
            send (character, HIGH);

            // The controller's address runs on from the end of row 0 to the
            // start of row 1, and from the end of row 1 back to row 0.
            ++address_;
            address_ = (address_ == RowLength)             ? SecondRow
                     : (address_ == SecondRow + RowLength) ? 0
                     :                                       address_;
        }

        // Print stops at the first write that returns 0, so '\r' counts too.
        return 1;
    }

    void Lcd::moveTo (uint8_t address)
    {
        send (SetDdram | address, LOW);
        address_ = address;
    }

    void Lcd::send (uint8_t value, uint8_t rs)
    {
        digitalWrite      (rs_, rs);
        pulse             (value >> 4);
        pulse             (value & 0x0F);
        delayMicroseconds (50);   // most instructions take 37 us
    }

    void Lcd::pulse (uint8_t nibble)
    {
        for (uint8_t line = 0; line < 4; ++line)
        {
            digitalWrite (data_[line], (nibble >> line) & 1 ? HIGH : LOW);
        }

        // D4-D7 are read as E falls; E must be high for at least 450 ns.
        digitalWrite      (enable_, HIGH);
        delayMicroseconds (1);
        digitalWrite      (enable_, LOW);
    }
}
