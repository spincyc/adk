// E17: Feed Back the Output
// A0 reads the knob; the op-amp follows it without a Mega output pin.

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
    adk::wait (100);
}
