// Lesson 34: RFID
// A card held to the reader lights the RGB LED green if it's known.

#include <Adk.h>

adk::Rfid   reader {53, 45};
adk::RgbLed light  {5, 6, 7};

// Your own cards' numbers, copied from the Serial Monitor.
const uint32_t KnownCards [] = {0x12345678, 0x9ABCDEF0};

const adk::Color Waiting = {0, 0, 40};

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

    if (reader.ok ())
    {
        light.show (Waiting);
    }
    else
    {
        light.show (adk::color::yellow);
        Serial.println ("No reader: check its wires and its 3.3 V.");
    }
}

void loop ()
{
    adk::update ();

    if (reader.wasRead ())
    {
        uint32_t card = reader.uid ();

        Serial.print ("Card 0x");
        Serial.println (card, HEX);

        light.show (isKnown (card) ? adk::color::green : adk::color::red);
        light.fadeTo (Waiting, 2000);
    }
}

bool isKnown (uint32_t card)
{
    for (uint32_t known : KnownCards)
    {
        if (card == known)
        {
            return true;
        }
    }

    return false;
}
