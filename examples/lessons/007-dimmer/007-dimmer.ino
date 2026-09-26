// Lesson 07: Dimmer
// Turn the knob on A0 and the white LED on pin 3 follows it, from dark to
// full brightness.

#include <Adk.h>

adk::AnalogInput knob {A0};
adk::PwmOutput   led  {3};

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    int reading    = knob.read ();      // 0 to 1023
    int brightness = reading / 4;       // 0 to 255
    led.write (brightness);

    adk::println (Serial, "knob:", reading, " brightness:", brightness);
    adk::wait (20);
}
