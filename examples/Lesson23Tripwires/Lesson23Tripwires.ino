// Lesson 23: Tripwires
// Four on/off sensors each light their own LED the moment they are tripped.

#include <Adk.h>

adk::Switch motion   {A12, adk::ActiveHigh};   // PIR: high on movement
adk::Switch obstacle {A13};                    // low when something is near
adk::Switch upright  {A14};                    // tilt switch: closed upright
adk::Switch beam     {A15, adk::ActiveHigh};   // high when the beam is broken

adk::Led red    {26};
adk::Led yellow {27};
adk::Led green  {28};
adk::Led blue   {29};

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    red.set    (motion.isActive ());
    yellow.set (obstacle.isActive ());
    green.set  (!upright.isActive ());
    blue.set   (beam.isActive ());

    announce (motion.activated (),     "Movement!");
    announce (obstacle.activated (),   "Something in front!");
    announce (upright.deactivated (),  "Tilted!");
    announce (beam.activated (),       "Beam broken!");
}

void announce (bool tripped, const char* message)
{
    if (tripped)
    {
        Serial.println (message);
    }
}
