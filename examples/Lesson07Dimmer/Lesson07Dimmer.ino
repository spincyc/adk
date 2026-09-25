// Lesson 07: Dimmer
// Turn the knob on A0 and the white LED on pin 3 follows it, from dark to
// full brightness.

#include <Adk.h>

adk::AnalogInput knob {A0};
adk::PwmOutput   led  {3};

// Which way the knob turns the LED up depends on which outer leg has 5 V.
// Make this true to turn it round without moving a wire.
constexpr bool reversed = false;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    int reading    = knob.read ();
    int brightness = reversed ? knob.read (255, 0) : knob.read (0, 255);
    led.write (brightness);

    adk::println (Serial, "knob:", reading, " brightness:", brightness);
    adk::wait (20);
}
