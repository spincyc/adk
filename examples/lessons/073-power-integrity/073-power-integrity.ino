// E18: Keep the Supply Steady
// USB 5 V powers the load; an isolated generator switches the transistor.

#include <Adk.h>

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();
}
