// Lesson 53: Remote Lamp, Board A
// Point a remote at this board. The kit remote's power button switches
// the lamp on Board B; every other button crosses the bridge and comes
// out of Board B's infrared LED, on the far side of the wall.

#include <Adk.h>

adk::IrReceiver receiver {2};
adk::Lcd        lcd      {31, 32, 33, 34, 35, 36};
adk::Every      tick     {100};

// The bridge to Board B, over the LoRa modem on Serial3.
adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

bool lampOn = false;        // what the power button last asked for

// The latest button to pass on, and the remote it came from. A press is
// an event, so a count goes with it: Board B sends a code each time the
// count changes, even for the same button twice.
long button  = 0;
long address = 0;
long presses = 0;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (receiver.wasReceived () && !receiver.isRepeat ())
    {
        obey (receiver.command (), receiver.address ());
    }

    bridge.share ("lamp", lampOn);
    bridge.share ("button", button);
    bridge.share ("address", address);
    bridge.share ("presses", presses);

    if (tick.ticked ())
    {
        showState ();
    }
}

// The kit remote's power button is the lamp's. Any other button, from any
// remote that speaks NEC, is passed on.
void obey (uint8_t command, uint16_t from)
{
    if (command == adk::remote::power)
    {
        lampOn = !lampOn;
    }
    else
    {
        button  = command;
        address = from;
        ++presses;
    }
}

// The last button sent and how many so far, and what Board B says its
// relay is doing.
void showState ()
{
    adk::print (lcd.at (0, 0), "Sent 0x", adk::hex (button, 2), ", #",
                presses, "   ");

    if (!bridge.isConnected ())
    {
        adk::print (lcd.at (0, 1), "No word from B  ");
    }
    else
    {
        adk::print (lcd.at (0, 1), "Lamp is ",
                    bridge.value ("relay") ? "on      " : "off     ");
    }
}
