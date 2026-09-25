#pragma once

#include "board.h"

#include <Arduino.h>

namespace adk {

    // Claim the two pins of one of the Mega's spare serial ports, for a
    // part that talks over it: Serial1 on 18 (TX) and 19 (RX), Serial2 on
    // 16 and 17, Serial3 on 14 and 15. TX idles high. Serial itself, on 0
    // and 1, belongs to the USB cable and the Serial Monitor, and is refused.
    bool claimSerial (HardwareSerial& port);

    // Gathers the text arriving on a serial port a line at a time, without
    // waiting for it: each read () takes whatever has arrived. A line ends
    // at a newline, a carriage return is dropped, and what doesn't fit is
    // left off. A part keeps one as a member.
    template <uint8_t Capacity>
    struct LineReader
    {
        // True when a whole line has arrived, which line () then holds
        // until the next read ().
        bool read (HardwareSerial& port)
        {
            if (complete_)
            {
                size_     = 0;
                text_[0]  = '\0';
                complete_ = false;
            }

            while (port.available () > 0)
            {
                int character = port.read ();

                if (character == '\n')
                {
                    complete_ = true;
                    return true;
                }

                if (character != '\r' && size_ < Capacity)
                {
                    text_[size_++] = static_cast<char> (character);
                    text_[size_]   = '\0';
                }
            }

            return false;
        }

        const char* line () const { return text_; }
        uint8_t     size () const { return size_; }

      private:
        char    text_ [Capacity + 1] {};
        uint8_t size_     = 0;
        bool    complete_ = false;
    };
}
