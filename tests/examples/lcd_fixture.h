#pragma once

#include <Arduino.h>

#include <string>

namespace lesson_test {

    // Listen after setup's special initialization nibbles. All these
    // lessons use RS 31, E 32 and D4-D7 33-36.
    inline std::string display;
    inline int highNibble = -1;

    inline void listenToLcd ()
    {
        display.clear ();
        highNibble = -1;
        arduino::onDigitalWrite = [] (uint8_t pin, uint8_t value)
        {
            if (pin != 32 || value != LOW)
            {
                return;
            }

            int nibble = 0;

            for (uint8_t bit = 0; bit < 4; ++bit)
            {
                nibble |= arduino::pin (static_cast<uint8_t> (33 + bit)).output << bit;
            }

            if (highNibble < 0)
            {
                highNibble = nibble;
            }
            else
            {
                if (arduino::pin (31).output == HIGH)
                {
                    display += char ((highNibble << 4) | nibble);
                }

                highNibble = -1;
            }
        };
    }
}
