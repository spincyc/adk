// Lesson 01: Blink
// A red LED on pin 26 flashes on and off, once a second, forever.

#include <Adk.h>

adk::Led led {26};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    led.on ();
    adk::wait (500);

    led.off ();
    adk::wait (500);
}
