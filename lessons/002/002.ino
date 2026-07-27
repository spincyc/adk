
#include <adk.h>

adk::led::Mono red  (4);
adk::led::Mono blue (7);

void setup ()
{
    adk::initialize ();
}

void loop ()
{
    constexpr auto intervalMs = 100;

    red .on  ();
    blue.off ();
    delay    (intervalMs);

    red .off ();
    blue.on  ();
    delay    (intervalMs);
}
