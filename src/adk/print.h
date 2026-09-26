#pragma once

#include <Arduino.h>

namespace adk {

    // What fixed () makes, for print () to print.
    struct Fixed
    {
        double  value;
        uint8_t decimals;

        void printTo (Print& out) const;
    };

    // What hex () makes, for print () to print.
    struct Hex
    {
        uint32_t value;
        uint8_t  digits;

        void printTo (Print& out) const;
    };

    // A number printed with a set count of decimals, rounded:
    // adk::print (lcd, adk::fixed (21.46, 1)) shows 21.5.
    constexpr Fixed fixed (double value, uint8_t decimals)
    {
        return {value, decimals};
    }

    // A whole number in hexadecimal, with zeros in front to make at least
    // a set count of digits, up to eight: adk::print (Serial, "0x",
    // adk::hex (12, 2)) shows 0x0C. A bigger number prints in full.
    constexpr Hex hex (uint32_t value, uint8_t digits = 1)
    {
        return {value, digits};
    }

    // Print several things in a row, to Serial or anything else that prints,
    // such as an Lcd. Each prints as Serial.print () would print it alone,
    // except what fixed () and hex () make.
    void print (Print& out, const auto&... parts)
    {
        // Anything with printTo (), such as a Fixed, prints itself.
        auto printPart = [&out] (const auto& part)
        {
            if constexpr (requires { part.printTo (out); })
            {
                part.printTo (out);
            }
            else
            {
                out.print (part);
            }
        };

        (printPart (parts), ...);
    }

    // The same, then end the line.
    void println (Print& out, const auto&... parts)
    {
        print (out, parts...);
        out.println ();
    }

    // Text made by printing into it, for a part that shows a whole string
    // at once, such as a matrix scrolling a score:
    //
    //     adk::Text<16> message;
    //     adk::print (message, "SCORE ", score);
    //     matrix.scroll (message.c_str ());
    //
    // It holds up to Capacity characters; what doesn't fit is left off.
    template <size_t Capacity>
    struct Text : Print
    {
        // Add one character, as everything printed into it does.
        size_t write (uint8_t character) override
        {
            if (size_ == Capacity)
            {
                return 0;
            }

            text_[size_++] = static_cast<char> (character);
            text_[size_]   = '\0';
            return 1;
        }

        using Print::write;

        // The text so far, for a part that shows a string.
        const char* c_str () const { return text_; }
        size_t      size  () const { return size_; }

        // Whether it holds exactly this text.
        bool operator== (const char* other) const
        {
            return strcmp (text_, other) == 0;
        }

        // Empty it, to print something new.
        void clear ()
        {
            size_    = 0;
            text_[0] = '\0';
        }

      private:
        char   text_ [Capacity + 1] {};
        size_t size_ = 0;
    };
}
