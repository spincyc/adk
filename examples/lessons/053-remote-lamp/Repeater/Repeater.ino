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

    // A new count is a new press to send on; 0 means none yet.
    if (bridge.changed ("presses") && bridge.value ("presses") > 0)
    {
        irLed.send (bridge.value ("button"), bridge.value ("address"));
    }

    bridge.share ("relay", relay.isOn ());
    connected.set (bridge.isConnected ());
}
