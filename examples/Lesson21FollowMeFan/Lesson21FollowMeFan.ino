// Lesson 21: Follow-Me Fan
// A servo sweeps a sensor, turns to the nearest thing, and a fan blows at it.

#include <Adk.h>

adk::Servo      turret {44};
adk::Ultrasonic sensor {14, 15};
adk::Motor      fan    {4, 8, 9};
adk::Timer      blowing;

constexpr int leftmost  = 30;      // the sweep, in degrees
constexpr int rightmost = 150;
constexpr int step      = 10;
constexpr int tooClose  = 15;      // cm: nearer than this, the fan stops
constexpr int reach     = 80;      // cm: further counts as nobody
constexpr int blowTime  = 4000;    // ms of blowing between sweeps

// Something the sensor saw: which way, and how far away in cm.
struct Sighting
{
    int angle;
    int distance;
};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    Sighting nearest = sweep ();

    if (nearest.distance > reach)
    {
        adk::wait (1000);    // nobody about: rest, then look again
        return;
    }

    turnTo (nearest.angle);
    blow ();
}

// One pass from side to side, with the fan resting, taking a reading at each
// step and remembering the nearest thing seen.
Sighting sweep ()
{
    Sighting nearest {90, 400};    // nothing, yet

    for (int angle = leftmost; angle <= rightmost; angle += step)
    {
        turnTo (angle);
        int cm = measure ();

        if (cm < nearest.distance)
        {
            nearest = {angle, cm};
        }
    }

    return nearest;
}

// Harder the closer they are, for blowTime, stopping early if they walk out
// of reach or come too close.
void blow ()
{
    blowing.start (blowTime);

    while (blowing.isRunning ())
    {
        int cm = measure ();

        if (cm < tooClose || cm > reach)
        {
            break;
        }

        fan.speed (map (cm, tooClose, reach, 255, 110));
    }

    fan.speed (0);
}

// A gentle turn, 8 ms for every degree, so the sensor and fan on the horn
// don't swing about, then 50 ms more to settle and let old echoes die away.
void turnTo (int angle)
{
    int glide = abs (angle - turret.angle ()) * 8;

    turret.moveTo (angle, glide);
    adk::wait (glide + 50);
}

// A brand new reading, in cm. No echo means nothing within about 4 m, which
// counts as 400, as in Lesson 19.
int measure ()
{
    do
    {
        adk::update ();
    }
    while (!sensor.measured ());

    return sensor.ok () ? sensor.distance () : 400;
}
