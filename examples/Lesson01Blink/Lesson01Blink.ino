#include <Adk.h>

adk::Led led {LED_BUILTIN};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    led.on ();
    adk::wait (500);
    led.off ();
    adk::wait (500);
}
