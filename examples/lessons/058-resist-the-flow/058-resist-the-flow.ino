// Lesson 58: Resist the Flow
// Give the LED a long on-time so there is time to read the meter.

#include <Adk.h>

adk::Led led {26};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    led.on ();
    adk::wait (10000);

    led.off ();
    adk::wait (2000);
}
