// Lesson 56: Close the Loop
// A red LED on pin 26 blinks when its return path reaches GND.

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
