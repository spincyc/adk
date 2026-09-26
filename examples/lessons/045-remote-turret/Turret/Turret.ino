// Lesson 45: Remote Turret, Board B
// The servo aims the fan and the sensor wherever Board A's joystick says,
// and the fan blows while Board A asks it to. Twice a second it tells
// Board A how far away the nearest thing is. The Mega's L LED shows that
// Board A can be heard.

#include <Adk.h>

adk::LoraModem  radio     {Serial2, 2, {.partner = 1,
                                        .speed   = adk::LoraSpeed::Quick,
                                        .power   = 10}};
adk::Bridge     bridge    {radio};
adk::Servo      turret    {44};
adk::Ultrasonic sensor    {14, 15};
adk::Motor      fan       {4, 8, 9};
adk::Led        connected {13};    // the L LED, on the Mega itself
adk::Timer      settling;          // the fan rests until the turret is still
adk::Every      report    {500};

constexpr int nothing  = 400;    // cm: no echo, nothing within 4 m
constexpr int tooClose = 15;     // cm: nearer than this, the fan stops
constexpr int fanSpeed = 200;    // out of 255

int distance = nothing;    // cm to the nearest thing the sensor sees

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (bridge.changed ("aim"))
    {
        turret.moveTo (bridge.value ("aim"), 100);
        settling.start (300);
    }

    if (sensor.measured ())
    {
        distance = sensor.ok () ? sensor.distance () : nothing;
    }

    // Twice a second is plenty, and leaves the air to Board A's stick.
    if (report.ticked ())
    {
        bridge.share ("dist", distance);
    }

    fan.speed (shouldBlow () ? fanSpeed : 0);
    connected.set (bridge.isConnected ());
}

// Only while Board A can be heard and asks for it, nobody is too close,
// and the turret is still: as in Lesson 21, the fan and the servo never
// pull hard on the power module together.
bool shouldBlow ()
{
    return bridge.isConnected () && bridge.value ("fan") == 1
        && distance >= tooClose && !settling.isRunning ();
}
