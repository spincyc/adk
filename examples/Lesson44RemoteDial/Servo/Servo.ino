// Lesson 44: Remote Dial, Board B
// The servo turns to the angle Board A's knob asks for, and once it gets
// there, tells Board A the angle it has reached. The yellow LED shows
// that Board A can be heard.

#include <Adk.h>

adk::LoraModem radio     {Serial3, 2, {.partner = 1,
                                       .speed   = adk::LoraSpeed::Quick,
                                       .power   = 10}};
adk::Bridge    bridge    {radio};
adk::Servo     servo     {44};
adk::Led       connected {27};    // yellow: Board A is there

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    // Only when a new angle arrives: until the first, value () says 0.
    if (bridge.changed ("angle"))
    {
        servo.moveTo (bridge.value ("angle"), 300);
    }

    // While it glides it keeps quiet, leaving the air to Board A.
    if (!servo.isMoving ())
    {
        bridge.share ("at", servo.angle ());
    }

    connected.set (bridge.isConnected ());
}
