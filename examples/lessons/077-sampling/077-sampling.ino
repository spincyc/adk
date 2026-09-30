// Lesson 77: Sample a Voltage
// Reuse Lesson 7's dimmer and watch each A0 reading on the Serial Plotter.

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
