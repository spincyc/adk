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

unsigned long run = 0;    // which test the dots belong to

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
        showHeard (radio.text ());
    }
}

// "2 17 abcd..." is test 2, message 17. Any packet can announce a new
// test: its first sixteen messages might all have been lost.
void showHeard (const char* text)
{
    char* end;
    unsigned long nextRun = strtoul (text, &end, 10);

    if (end == text || *end != ' ')
    {
        return;
    }

    long number = strtol (end + 1, &end, 10);

    if (number < 1 || number > 64 || *end != ' ')
    {
        return;
    }

    if (nextRun != run)
    {
        matrix.clear ();
        run = nextRun;
    }

    int dot = int (number) - 1;
    matrix.set (dot % 8, dot / 8);
}
