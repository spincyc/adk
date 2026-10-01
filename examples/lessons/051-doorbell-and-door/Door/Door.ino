// Lesson 51: Doorbell and Door, Board B (the door)
// The doorbell, a knock or a card goes by radio to Board A, inside, which
// decides who comes in. When A opens the door, this board's buzzer buzzes,
// as a block of flats' front door does. The L LED shows A can be heard.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};
adk::Rfid      reader {53, 45};
adk::Switch    tap    {A12, adk::ActiveLow, 0};    // as in Lesson 35
adk::Button    bell   {22};
adk::Buzzer    buzzer {12};
adk::Led       light  {LED_BUILTIN};    // the L LED: A can be heard

constexpr adk::Millis rattle = 80;    // how long the tap sensor rattles
adk::Stopwatch        sinceKnock;     // the time since the last knock

// Each of these is an event, so each crosses as a count.
long rings  = 0;
long knocks = 0;
long cards  = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    sinceKnock.start ();

    if (!reader.ok ())
    {
        Serial.println ("No card reader: check its wires and its 3.3 V.");
    }
}

void loop ()
{
    adk::update ();

    if (bell.wasPressed ())
    {
        ++rings;
    }

    // The buzzer shakes the board, so a knock while it sounds is its own.
    if (tap.activated () && sinceKnock.elapsed () > rattle && !buzzer.isOn ())
    {
        ++knocks;
        sinceKnock.restart ();
    }

    if (reader.wasRead ())
    {
        ++cards;
    }

    // A ring or a knock carries nothing more, so its payload is 0. A card
    // carries its number, all 32 bits of it in a long, as The idea
    // explains.
    bridge.shareEvent ("rings", rings, 0);
    bridge.shareEvent ("knocks", knocks, 0);
    bridge.shareEvent ("cards", cards, int32_t (reader.uid ()));

    // Board A has opened the door.
    if (bridge.changed ("door") && bridge.value ("door") == 1)
    {
        buzzer.beep (1000);
    }

    light.set (bridge.isConnected ());
}
