// Lesson 48: Room Monitor, Board A, in the nursery
// Listens, feels for water and watches the light, and sends what it finds
// across the bridge: the loudest sound in each half second, the water
// sensor's reading every two seconds, and how bright the room is.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::SoundSensor   sound      {A5};
adk::AnalogInput   water      {A6};
adk::DigitalOutput waterPower {A7};    // on only while the water is felt
adk::AnalogInput   light      {A1};
adk::Led           online     {28};    // lit while the parent is heard
adk::Every         halfSecond {500};
adk::Every         feel       {2000};
adk::Timer         settle;             // the water sensor's power is on

uint16_t loudest = 0;    // the loudest sound so far in this half second

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    online.set (bridge.isConnected ());

    // A new level comes every 50 ms, ten in each half second, and only
    // the loudest goes. A lost radio packet can still lose this peak.
    if (sound.measured ())
    {
        loudest = max (loudest, sound.level ());
    }

    if (halfSecond.ticked ())
    {
        adk::println (Serial, "Peak ", loudest);
        bridge.share ("sound", loudest);
        bridge.share ("light", light.read (0, 100));
        loudest = 0;
    }

    // Current through wet traces slowly eats them away, so the sensor is
    // powered for just 10 ms every two seconds: long enough to read.
    if (feel.ticked ())
    {
        waterPower.write (true);
        settle.start (10);
    }

    if (settle.expired ())
    {
        int reading = water.read ();
        adk::println (Serial, "Water ", reading);
        bridge.share ("water", reading);
        waterPower.write (false);
    }
}
