// Lesson 47: Remote Alarm, Board A, by the door
// Five tripwires each count how many times they have gone off, and the
// bridge carries the counts to the den. The red LED shows whether the den
// has armed the alarm.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Switch motion   {A12, adk::ActiveHigh};    // PIR: high on movement
adk::Switch upright  {A14};                     // tilt switch: closed upright
adk::Switch beam     {A15, adk::ActiveHigh};    // high when the beam is broken
adk::Switch obstacle {16};                      // low when something is near
adk::Switch tap      {17, adk::ActiveLow, 0};   // low for a moment on a knock
adk::Led    armed    {26};
adk::Led    online   {28};    // lit while the den is heard

long moves  = 0;    // how many times each tripwire has gone off
long tilts  = 0;
long breaks = 0;
long nears  = 0;
long knocks = 0;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    count (motion.activated (),    moves,  "motion");
    count (upright.deactivated (), tilts,  "tilt");
    count (beam.activated (),      breaks, "beam");
    count (obstacle.activated (),  nears,  "near");
    count (tap.activated (),       knocks, "knock");

    armed.set (bridge.value ("armed") == 1);
    online.set (bridge.isConnected ());
}

// A tripwire going off is over in an instant, and a message can be lost,
// so what crosses is how many times it has happened: the den sees the
// number change, and one lost message loses nothing.
void count (bool tripped, long& times, const char* name)
{
    if (tripped)
    {
        ++times;
    }

    bridge.share (name, times);
}
