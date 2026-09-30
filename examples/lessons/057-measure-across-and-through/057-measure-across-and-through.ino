// Lesson 57: Measure across and through
// Hold each LED state for three seconds so a meter can settle.

#include <Adk.h>

adk::Led led {26};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    led.on ();
    adk::wait (3000);

    led.off ();
    adk::wait (3000);
}
