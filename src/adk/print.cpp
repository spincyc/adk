#include "print.h"

namespace adk {

    void Fixed::printTo (Print& out) const
    {
        out.print (value, decimals);
    }

    void Hex::printTo (Print& out) const
    {
        // Eight digits from the highest down, leaving out the zeros in front
        // of the first that matters, or of the digits asked for.
        for (int8_t digit = 7; digit >= 0; --digit)
        {
            uint32_t from   = value >> (4 * digit);
            uint8_t  nibble = static_cast<uint8_t> (from & 0x0F);

            if (from != 0 || digit < digits || digit == 0)
            {
                out.print (static_cast<char> (nibble < 10 ? '0' + nibble : 'A' + nibble - 10));
            }
        }
    }
}
