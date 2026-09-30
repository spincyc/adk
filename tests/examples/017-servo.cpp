#include <Arduino.h>

#include <stdio.h>

#include "../../examples/lessons/017-servo/017-servo.ino"

namespace {

    // A real knob can alternate between neighboring ADC readings. Run the
    // actual lesson, including its screen updates, against that input.
    void turnKnob (adk::Millis duration, int first, int second)
    {
        unsigned long began = millis ();
        bool          odd   = false;

        while (millis () - began < duration)
        {
            arduino::pin (A0).analog = odd ? first : second;
            odd = !odd;
            arduino::advance (1);
            loop ();
        }
    }

    int expectAngle (int low, int high, const char* action)
    {
        int angle = needle.angle ();

        if (angle >= low && angle <= high)
        {
            return 0;
        }

        printf ("After %s: expected %d to %d degrees, got %d\n",
                action, low, high, angle);
        return 1;
    }
}

int main ()
{
    arduino::reset ();
    setup ();

    turnKnob (200, 0, 0);
    int failures = expectAngle (0, 0, "starting at the low end");

    // 511 and 512 straddle the boundary between targets of 89 and 90 degrees.
    turnKnob (1000, 511, 512);
    failures += expectAngle (89, 90, "moving with one-degree input noise");

    turnKnob (600, 1018, 1018);
    failures += expectAngle (179, 179, "turning just below the high end");

    turnKnob (600, 1023, 1023);
    failures += expectAngle (180, 180, "turning fully up");

    turnKnob (1000, 1022, 1023);
    failures += expectAngle (180, 180, "holding near the high end");

    turnKnob (600, 6, 6);
    failures += expectAngle (1, 1, "turning just above the low end");

    turnKnob (600, 0, 0);
    failures += expectAngle (0, 0, "turning fully down");

    turnKnob (1000, 5, 6);
    failures += expectAngle (0, 0, "holding near the low end");

    printf ("Servo lesson: 8 checks, %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
