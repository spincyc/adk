// Lesson 17: Servo
// A dial whose needle follows a knob: the servo glides to the knob's angle,
// from 0 to 180 degrees. The screen shows the knob and commanded angles.

#include <Adk.h>

adk::Servo       needle  {44};
adk::AnalogInput knob    {A0};
adk::Lcd         lcd     {31, 32, 33, 34, 35, 36};
adk::Every       refresh {100};

constexpr char degree = char (223);    // the LCD's own degree sign

long target = -1;    // the last angle requested; -1 means none yet

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (refresh.ticked ())
    {
        long angle = knob.read (0, 180);

        // Ignore one-degree noise, but always reach either end of the dial.
        if (target < 0 || angle < target - 1 || angle > target + 1 ||
            angle == 0 || angle == 180)
        {
            target = angle;
            needle.moveTo (angle, 300);
        }

        // The spaces at the end rub out what a longer number left.
        adk::print (lcd.at (0, 0), "Knob   ", angle, degree, "  ");
        adk::print (lcd.at (0, 1), "Needle ", needle.angle (), degree, "  ");
    }
}
