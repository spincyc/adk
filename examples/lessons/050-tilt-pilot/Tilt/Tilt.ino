// Lesson 50: Tilt Pilot, Board A (the tilt)
// Tilt this breadboard, and a ball rolls the same way across Board B's
// matrix while B's servo leans to match. The L LED shows B can be heard.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};
adk::Mpu6050   tilt   {0x68};
adk::Led       light  {LED_BUILTIN};    // the L LED: B can be heard

// The tilt, smoothed: each reading moves it a quarter of the way towards
// the new one, so a shaky hand doesn't change it every time.
float pitch = 0;
float roll  = 0;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (tilt.measured () && tilt.ok ())
    {
        pitch += (tilt.pitch () - pitch) / 4;
        roll  += (tilt.roll () - roll) / 4;
    }

    // In whole degrees. The bridge sends only a change, so a board held
    // still sends nothing new, and a board on the move up to ten a second.
    bridge.share ("pitch", lround (pitch));
    bridge.share ("roll", lround (roll));
    bridge.share ("sensor", tilt.ok ());

    light.set (bridge.isConnected ());
}
