// Lesson 11: Four Digits
// A four-digit display behind a 74HC595 on pins 37, 38 and 39, its digits on pins 40 to 43,
// says HI, then counts up ten times a second.

#include <Adk.h>

adk::FourDigitDisplay display {37, 38, 39, 40, 41, 42, 43};
adk::Every            tick    {100};

int count = 0;

void setup ()
{
    adk::setup ();

    display.show ("HI");
    adk::wait (1500);
}

void loop ()
{
    adk::update ();

    if (tick.ticked ())
    {
        count = (count + 1) % 10000;
        display.show (count);
    }
}
