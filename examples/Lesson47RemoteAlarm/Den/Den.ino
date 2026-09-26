// Lesson 47: Remote Alarm, Board B, in the den
// The door's tripwires arrive as counts. When one rises, the screen names
// the tripwire and the time, and if the alarm is armed the siren sounds.
// POWER on the remote arms and disarms it, and the door shows which.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Lcd        lcd      {31, 32, 33, 34, 35, 36};
adk::Rtc        rtc;
adk::IrReceiver receiver {2};
adk::Buzzer     siren    {12};
adk::Every      beat     {400};    // the siren's beeps, and the top row

// A tripwire at the door: its name on the bridge, its name on the screen,
// and the latest count heard from it, -1 until the first.
struct Tripwire
{
    const char* name;
    const char* label;
    long        count;
};

adk::Array tripwires {Tripwire {"motion", "Motion  ", -1},
                      Tripwire {"tilt",   "Tilted  ", -1},
                      Tripwire {"beam",   "Beam    ", -1},
                      Tripwire {"near",   "Near    ", -1},
                      Tripwire {"knock",  "Knock   ", -1}};

bool armed    = false;
bool sounding = false;

void setup ()
{
    adk::setup ();

    if (!rtc.isRunning ())
    {
        rtc.set (adk::compiledAt ());
    }
}

void loop ()
{
    adk::update ();

    if (receiver.wasReceived () && !receiver.isRepeat ()
        && receiver.command () == adk::remote::power)
    {
        armed    = !armed;
        sounding = false;
    }

    bridge.share ("armed", armed);

    for (auto& wire : tripwires)
    {
        if (bridge.changed (wire.name))
        {
            heard (wire);
        }
    }

    if (beat.ticked ())
    {
        showState ();

        if (sounding)
        {
            siren.beep (200);
        }
    }
}

// A count that rises means the tripwire went off. The first count heard
// only says where it starts, and one that falls means the door board has
// started again from 0.
void heard (Tripwire& wire)
{
    long count = bridge.value (wire.name);

    if (wire.count >= 0 && count > wire.count)
    {
        auto at = rtc.now ();

        adk::print (lcd.at (0, 1), wire.label, at.hour / 10, at.hour % 10,
                    ':', at.minute / 10, at.minute % 10, ':',
                    at.second / 10, at.second % 10);
        sounding = sounding || armed;
    }

    wire.count = count;
}

// The top row: the alarm's state, and whether the door is heard.
void showState ()
{
    auto state = armed ? "Armed   " : "Disarmed";
    auto door  = bridge.isConnected () ? "Door ok " : "No news ";

    adk::print (lcd.at (0, 0), sounding ? "ALARM!  " : state, door);
}
