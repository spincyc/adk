// Lesson 53: Remote Lamp, Board B
// The relay switches the lamp as Board A's power button says, and every
// button Board A passes on goes out again from the infrared LED, to a TV
// or another board in this room. The L LED lights while the bridge hears
// Board A.

#include <Adk.h>

adk::Relay         relay     {11};
adk::IrTransmitter irLed     {3};
adk::Led           connected {LED_BUILTIN};

// The bridge to Board A, over the LoRa modem on Serial3.
adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

long presses = -1;    // Board A's count as last heard, -1 before any

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (bridge.value ("lamp"))
    {
        relay.on ();
    }
    else
    {
        relay.off ();
    }

    // A count that went up is a new press to send on. The first count
    // heard only says where Board A has got to, as in Lesson 51.
    if (bridge.changed ("presses"))
    {
        long count = bridge.value ("presses");

        if (presses >= 0 && count > presses)
        {
            irLed.send (bridge.value ("button"), bridge.value ("address"));
        }

        presses = count;
    }

    bridge.share ("relay", relay.isOn ());
    connected.set (bridge.isConnected ());
}
