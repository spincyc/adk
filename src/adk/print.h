#pragma once

#include <Arduino.h>

namespace adk {

    // A number printed with a set count of decimals, rounded:
    // adk::print (lcd, adk::fixed (21.46, 1)) shows 21.5.
    struct Fixed
    {
        double  value;
        uint8_t decimals;
    };

    constexpr Fixed fixed (double value, uint8_t decimals)
    {
        return {value, decimals};
    }

    inline void printPart (Print& out, Fixed number)
    {
        out.print (number.value, number.decimals);
    }

    void printPart (Print& out, const auto& part)
    {
        out.print (part);
    }

    // Print several things in a row, to Serial or anything else that prints,
    // such as an Lcd. Each prints as Serial.print () would print it alone.
    void print (Print& out, const auto&... parts)
    {
        (printPart (out, parts), ...);
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
    // What doesn't fit is left off.
    template <size_t Capacity>
    struct Text : Print
    {
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

        const char* c_str () const { return text_; }
        size_t      size  () const { return size_; }

        bool operator== (const char* other) const
        {
            return strcmp (text_, other) == 0;
        }

        void clear ()
        {
            size_     = 0;
            text_[0]  = '\0';
        }

      private:
        char   text_ [Capacity + 1] {};
        size_t size_ = 0;
    };
}
