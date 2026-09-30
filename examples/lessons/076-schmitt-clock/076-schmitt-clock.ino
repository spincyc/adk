// Lesson 76: Make a Clock Tick
// USB powers the chip; its feedback makes the LED blink without a signal pin.

#include <Adk.h>

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();
}
