
#include <led.h>

adk::led::mono red(4);
adk::led::mono blu(7);

void setup()
{
    adk::setup();
}

void loop()
{
    auto sleep = 100;

    red.on();
    blu.off();
    delay(sleep);

    red.off();
    blu.on();
    delay(sleep);
}

