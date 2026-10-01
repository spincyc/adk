// E23: Average PWM
// The knob sets pin 3's on-time; the separate resistor and capacitor
// smooth its pulses for a meter without taking away the LED's pulses.

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

    int reading = knob.read ();
    int duty = reading / 4;
    led.write (duty);

    adk::println (Serial, "knob:", reading, " duty:", duty);
    adk::wait (100);
}
