
#include <led.h>

adk::led::rgb led(6,5,3);

void setup()
{
    adk::setup();
}

auto red    = adk::color::red();
auto blue   = adk::color::blue();
auto green  = adk::color::green();
auto orange = adk::color::orange();


void loop()
{
    auto sleep = 250;

    led.on(red);
    delay(sleep);
    
    led.on(green);
    delay(sleep);

    led.on(blue);
    delay(sleep);
  
    led.on(orange);
    delay(sleep);
}

