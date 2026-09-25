// Lesson 17: Servo
// A dial whose needle follows a knob: the servo turns to the knob's angle,
// from 0 to 180 degrees.

#include <Adk.h>

adk::Servo       needle {44};
adk::AnalogInput knob   {A0};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    long angle = knob.read (0, 180);
    needle.moveTo (angle, 300);
}
