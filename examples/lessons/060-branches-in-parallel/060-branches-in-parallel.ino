// E05: Branches in Parallel
// USB 5 V feeds the two LED branches; no I/O pins are used.

#include <Adk.h>

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();
}
