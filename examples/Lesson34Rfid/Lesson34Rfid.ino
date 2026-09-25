// Lesson 34: RFID
// A card held to the reader lights the RGB LED green if it's known.

#include <Adk.h>

adk::Rfid   reader {53, 45};
adk::RgbLed light  {5, 6, 7};

// Your own cards' numbers, copied from the Serial Monitor.
constexpr adk::Array<uint32_t, 2> knownCards {0x12345678, 0x9ABCDEF0};

constexpr adk::Color quietBlue {0, 0, 40};

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

    if (reader.ok ())
    {
        light.show (quietBlue);
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
        auto card = reader.uid ();

        Serial.print ("Card 0x");
        Serial.println (card, HEX);

        light.show (isKnown (card) ? adk::color::green : adk::color::red);
        light.fadeTo (quietBlue, 2000);
    }
}

bool isKnown (uint32_t card)
{
    for (auto known : knownCards)
    {
        if (known == card)
        {
            return true;
        }
    }

    return false;
}
