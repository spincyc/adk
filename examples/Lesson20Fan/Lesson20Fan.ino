// Lesson 20: Fan
// A knob sets the fan's speed and a button reverses it, through an L293D.

#include <Adk.h>

adk::Motor       fan     {4, 8, 9};
adk::AnalogInput knob    {A0};
adk::Button      reverse {22};

int direction = 1;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (reverse.wasPressed ())
    {
        direction = -direction;
    }

    fan.speed (direction * knobSpeed ());
}

// Off for the first tenth of the knob's turn, then from 100 up to full speed
// at 255: below about 100 the motor only hums.
int knobSpeed ()
{
    long turn = knob.read (0, 100);

    if (turn < 10)
    {
        return 0;
    }

    return map (turn, 10, 100, 100, 255);
}
