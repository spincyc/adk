
#include <adk.h>

adk::led::Mono led (LED_BUILTIN);

void setup ()
{
    adk::initialize ();
}

void loop ()
{
    constexpr auto intervalMs = 100;

    led.on  ();
    delay   (intervalMs);

    led.off ();
    delay   (intervalMs);
}
