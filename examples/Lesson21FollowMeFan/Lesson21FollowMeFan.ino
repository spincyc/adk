// Lesson 21: Follow-Me Fan
// A servo sweeps a sensor, turns to the nearest thing, and a fan blows at it.

#include <Adk.h>

adk::Servo      turret {44};
adk::Ultrasonic sensor {14, 15};
adk::Motor      fan    {4, 8, 9};

const uint8_t       Leftmost  = 30;      // the sweep, in degrees
const uint8_t       Rightmost = 150;
const uint8_t       Step      = 10;
const uint16_t      TooClose  = 15;      // cm: nearer than this, the fan stops
const uint16_t      InReach   = 80;      // cm: further counts as nobody
const unsigned long BlowTime  = 4000;    // ms of blowing between sweeps

uint8_t  targetAngle    = 90;
uint16_t targetDistance = 0;

void setup ()
{
    adk::setup ();
    turret.write (Leftmost);
    adk::wait (500);
}

void loop ()
{
    adk::update ();

    fan.speed (0);
    sweep ();

    if (targetDistance == 0)
    {
        adk::wait (1000);
        return;
    }

    turnTo (targetAngle);
    blowAtTarget ();
}

// One pass from side to side with the fan resting, a reading every Step
// degrees, remembering the nearest thing in reach.
void sweep ()
{
    targetDistance = 0;

    for (uint8_t angle = Leftmost; angle <= Rightmost; angle += Step)
    {
        turnTo (angle);
        uint16_t cm = measure ();

        if (cm != 0 && (targetDistance == 0 || cm < targetDistance))
        {
            targetDistance = cm;
            targetAngle    = angle;
        }
    }
}

// Harder the closer they are, for BlowTime, stopping early if they walk out of
// reach or come too close.
void blowAtTarget ()
{
    unsigned long start = millis ();

    while (millis () - start < BlowTime)
    {
        uint16_t cm = measure ();

        if (cm == 0 || cm < TooClose)
        {
            return;
        }

        fan.speed (map (cm, TooClose, InReach, 255, 110));
    }
}

// A gentle turn, 8 ms for every degree, so the sensor and fan on the horn
// don't swing about.
void turnTo (uint8_t angle)
{
    uint8_t from = turret.angle ();
    uint8_t turn = (from > angle) ? from - angle : angle - from;

    turret.moveTo (angle, turn * 8UL);
}

// A fresh reading once the turret has stopped: the distance in cm, or 0 when
// nothing is in reach.
uint16_t measure ()
{
    while (turret.isMoving ())
    {
        adk::update ();
    }

    adk::wait (50);

    do
    {
        adk::update ();
    }
    while (!sensor.measured ());

    uint16_t cm = sensor.distance ();
    return (cm <= InReach) ? cm : 0;
}
