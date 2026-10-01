// Lesson 55: Reliability Meter, Board B
// Sends every message from Board A straight back, word for word, and
// lights a dot on the matrix for each one it hears, so you can see which
// got this far.

#include <Adk.h>

// Quick or Far: the same on both boards, or they can't hear each other.
constexpr adk::LoraSpeed speed = adk::LoraSpeed::Quick;

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = speed,
                                    .power   = 0}};    // its lowest
adk::LedMatrix matrix {47, 48, 49};

int last = 0;     // the number of the last message heard

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (radio.wasReceived ())
    {
        radio.send (radio.text ());
        showHeard (atoi (radio.text ()));
    }
}

// "17 abcd..." is message 17: its dot is the 17th, counting along each
// row of eight. A number lower than the last one heard starts a new test.
void showHeard (int number)
{
    if (number < last)
    {
        matrix.clear ();
    }

    if (number >= 1 && number <= 64)
    {
        matrix.set ((number - 1) % 8, (number - 1) / 8);
    }

    last = number;
}
