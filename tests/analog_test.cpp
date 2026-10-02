#include "check.h"

#include <Arduino.h>

TEST (analogInputScalesAcrossTheMegasLongRange)
{
    adk::AnalogInput knob {A0};
    constexpr long  lowest  = -2147483647L - 1;
    constexpr long  highest = 2147483647L;

    adk::setup ();

    arduino::pin (A0).analog = 0;
    CHECK (knob.read (0, 3600000L) == 0);
    CHECK (knob.read (lowest, highest) == lowest);
    CHECK (knob.read (highest, lowest) == highest);

    arduino::pin (A0).analog = 1023;
    CHECK (knob.read (0, 3600000L) == 3600000L);
    CHECK (knob.read (3600000L, 0) == 0);
    CHECK (knob.read (lowest, highest) == highest);
    CHECK (knob.read (highest, lowest) == lowest);

    arduino::pin (A0).analog = 512;
    CHECK (knob.read (0, 3600000L) == 1801759L);
    CHECK (knob.read (3600000L, 0) == 1798241L);
    CHECK (knob.read (lowest, highest) == 2099201L);
    CHECK (knob.read (highest, lowest) == -2099202L);
    CHECK (knob.read (lowest, lowest) == lowest);
}

TEST (smootherClampsLargeShiftsToSixteen)
{
    constexpr uint8_t shifts [] = {0, 16, 17, 31, 32, 255};

    for (uint8_t shift : shifts)
    {
        adk::Smoother smooth {shift};

        CHECK (smooth.value () == 0);
        CHECK (smooth.add (65535) == 65535);
        CHECK (smooth.add (65535) == 65535);
        CHECK (smooth.add (0) == (shift == 0 ? 0 : 65534));
        CHECK (smooth.add (65535) == (shift == 0 ? 65535 : 65534));
    }
}
