
#include <led.h>

adk::led::mono led(LED_BUILTIN);

void setup()
{
    adk::setup();
}

void loop()
{
    auto sleep = 100;

    led.on();
    delay(sleep);

    led.off();
    delay(sleep);
}

