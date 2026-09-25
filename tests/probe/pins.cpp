// Runs a sketch's setup () on the host and prints every pin its parts
// claimed, one per line, so tests/pins.py can hold the sketch to its
// lesson's circuit. The Makefile links this with the sketch as Arduino
// preprocessed it.

#include <Adk.h>
#include <Arduino.h>

#include <stdio.h>
#include <stdlib.h>

void setup ();

namespace {

    struct StandardError : Print
    {
        size_t write (uint8_t byte) override
        {
            return fputc (byte, stderr) == EOF ? 0 : 1;
        }
    };
}

namespace adk {

    void halt (Fault fault, Pin pin)
    {
        StandardError error;

        explain (error, fault, pin);
        exit (1);
    }
}

int main ()
{
    // A sketch may wait in setup (), for a sensor to settle or to learn the
    // room's light, so let time pass as it runs.
    arduino::setClockStep (100);
    setup ();

    for (adk::Pin pin = 0; pin < NUM_DIGITAL_PINS; ++pin)
    {
        if (adk::isClaimed (pin))
        {
            if (pin >= A0)
            {
                printf ("A%d\n", pin - A0);
            }
            else
            {
                printf ("%d\n", pin);
            }
        }
    }
}
