// Lesson 20: Fan
// A knob sets the fan's speed and a button reverses it, through an L293D.

#include <Adk.h>

adk::Motor       fan     {4, 8, 9};
adk::AnalogInput knob    {A0};
adk::Button      reverse {22};

constexpr int slowest = 100;    // below this, the motor only hums

int direction = 1;              // 1 forwards, -1 backwards

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

// Off for the first tenth of the knob's turn, then from the slowest speed
// that turns the fan up to full speed, 255.
int knobSpeed ()
{
    long turn = knob.read (0, 100);    // how far round, in percent

    return turn < 10 ? 0 : map (turn, 10, 100, slowest, 255);
}
