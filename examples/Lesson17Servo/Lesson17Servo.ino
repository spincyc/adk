// Lesson 17: Servo
// A dial whose needle follows a knob: the servo glides to the knob's angle,
// from 0 to 180 degrees, and the screen shows both angles.

#include <Adk.h>

adk::Servo       needle  {44};
adk::AnalogInput knob    {A0};
adk::Lcd         lcd     {31, 32, 33, 34, 35, 36};
adk::Every       refresh {100};

constexpr char degree = char (223);    // the LCD's own degree sign

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    long angle = knob.read (0, 180);
    needle.moveTo (angle, 300);

    // The spaces at the end rub out what a longer number left.
    if (refresh.ticked ())
    {
        adk::print (lcd.at (0, 0), "Knob   ", angle, degree, "  ");
        adk::print (lcd.at (0, 1), "Needle ", needle.angle (), degree, "  ");
    }
}
