// Lesson 19: Parking Sensor
// Beeps faster and lights green, yellow, then red as something comes closer.

#include <Adk.h>

adk::Ultrasonic sensor {14, 15};
adk::Led        green  {28};
adk::Led        yellow {27};
adk::Led        red    {26};
adk::Buzzer     buzzer {12};
adk::Every      beeps  {1000};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    // No echo means nothing within about 4 meters.
    uint16_t distance = sensor.hasEcho () ? sensor.distance () : 400;

    showGauge (distance);
    soundWarning (distance);
}

// Green while there is plenty of room, yellow to slow down, red to stop.
void showGauge (uint16_t cm)
{
    green.set  (cm >= 50);
    yellow.set (cm >= 20 && cm < 50);
    red.set    (cm < 20);
}

// A short beep every cm × 10 ms: once a second at a meter, faster and faster
// as it closes in, and one steady tone closer than 10 cm.
void soundWarning (uint16_t cm)
{
    beeps.period (cm * 10UL);

    if (cm < 10)
    {
        buzzer.on ();
    }
    else if (cm >= 100)
    {
        buzzer.off ();
    }
    else if (beeps.ticked ())
    {
        buzzer.beep (50);
    }
}
