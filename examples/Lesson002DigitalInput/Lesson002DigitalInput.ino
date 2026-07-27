#include <Adk.h>

adk::Runtime      runtime;
adk::DigitalInput button (runtime.resources (), 22, adk::Pull::Up);
adk::MonoLed      led    (runtime.resources (), LED_BUILTIN);

void setup ()
{
    button.initialize ();
    led.initialize    ();
}

void loop ()
{
    button.update ();
    led.set       (button.read () == adk::Level::Low);
}
