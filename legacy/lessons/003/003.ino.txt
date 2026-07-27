
#include <adk.h>

adk::led::Rgb led (6, 5, 3);

const auto red    = adk::color::red    ();
const auto blue   = adk::color::blue   ();
const auto green  = adk::color::green  ();
const auto orange = adk::color::orange ();

void setup ()
{
    adk::initialize ();
}

void loop ()
{
    constexpr auto intervalMs = 250;

    led.on (red);
    delay  (intervalMs);

    led.on (green);
    delay  (intervalMs);

    led.on (blue);
    delay  (intervalMs);

    led.on (orange);
    delay  (intervalMs);
}
