// Lesson 19: Parking Sensor
// Beeps faster and lights green, yellow, then red as something comes closer.

#include <Adk.h>

adk::Ultrasonic sensor {14, 15};
adk::Led        green  {28};
adk::Led        yellow {27};
adk::Led        red    {26};
adk::Buzzer     buzzer {12};
adk::Every      beeps  {1000};

// Where each warning starts, in cm, as something comes closer.
constexpr int ticking  = 100;   // a beep, faster and faster
constexpr int slowDown = 50;    // yellow
constexpr int danger   = 20;    // red
constexpr int touching = 10;    // one steady tone

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    // No echo means nothing within about 4 meters.
    int distance = sensor.ok () ? sensor.distance () : 400;

    showGauge (distance);
    soundWarning (distance);
}

// Green while there is plenty of room, yellow to slow down, red to stop.
void showGauge (int cm)
{
    green.set  (cm >= slowDown);
    yellow.set (cm >= danger && cm < slowDown);
    red.set    (cm < danger);
}

// A short beep every cm × 10 ms: once a second at a meter, faster and faster
// as it closes in, then one steady tone.
void soundWarning (int cm)
{
    beeps.period (cm * 10);

    if (cm < touching)
    {
        buzzer.on ();
    }
    else if (cm >= ticking)
    {
        buzzer.off ();
    }
    else if (beeps.ticked ())
    {
        buzzer.beep (50);
    }
}
