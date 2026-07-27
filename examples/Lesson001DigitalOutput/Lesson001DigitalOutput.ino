#include <Adk.h>

adk::Runtime       runtime;
adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);

void setup ()
{
    led.initialize ();
}

void loop ()
{
    led.write (adk::Level::High);
    delay     (500);
    led.write (adk::Level::Low);
    delay     (500);
}
