// Lesson 43: The Bridge, Board B
// Hold the button and Board A's red LED lights; hold Board A's and this
// one's does. The yellow LED shows that the boards can hear each other.
// Board A's sketch is the same, but for its address and its partner's.

#include <Adk.h>

adk::LoraModem radio     {Serial3, 2, {.partner = 1,
                                       .speed   = adk::LoraSpeed::Quick,
                                       .power   = 10}};
adk::Bridge    bridge    {radio};
adk::Button    button    {22};
adk::Led       light     {26};    // red: the other board's button
adk::Led       connected {27};    // yellow: the other board is there

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

    if (!radio.ok ())
    {
        adk::println (Serial, "No reply from the modem");
    }
}

void loop ()
{
    adk::update ();

    bridge.share ("button", button.isPressed ());

    light.set (bridge.value ("button"));
    connected.set (bridge.isConnected ());

    // Every message, as it arrives: the bridge's own words.
    if (radio.wasReceived ())
    {
        adk::println (Serial, "Heard: ", radio.text ());
    }
}
