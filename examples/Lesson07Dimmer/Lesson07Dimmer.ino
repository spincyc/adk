// Lesson 07: Dimmer
// Turn the knob on A0 and the white LED on pin 3 follows it, from dark to full brightness.

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

    int reading    = knob.read ();
    int brightness = knob.read (0, 255);
    led.write (brightness);

    plot (reading, brightness);
    adk::wait (20);
}

// Two labelled numbers on one line, which the Serial Plotter draws as two
// lines.
void plot (int reading, int brightness)
{
    Serial.print ("knob:");
    Serial.print (reading);
    Serial.print (" brightness:");
    Serial.println (brightness);
}
