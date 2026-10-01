// E06: Tap a Divider
// Plot the voltage picked up by the knob's wiper on A0.

#include <Adk.h>

adk::AnalogInput knob {A0};

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();
    adk::println (Serial, "A0:", knob.read ());
    adk::wait (50);
}
