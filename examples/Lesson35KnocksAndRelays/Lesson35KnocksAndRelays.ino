// Lesson 35: Knocks and Relays
// The secret knock makes a relay switch a separate lamp on or off.

#include <Adk.h>

// No debouncing: a knock is over in a few milliseconds.
adk::Switch tap        {A12, adk::ActiveLow, 0};
adk::Led    knockLight {LED_BUILTIN};
adk::Relay  relay      {11};

// Each gap between knocks is S, short, or L, long. The secret "SLS" is
// knock-knock, pause, knock-knock.
constexpr char        secret [] = "SLS";
constexpr adk::Millis longGap   = 400;     // a gap this long or more is L
constexpr adk::Millis rattle    = 80;      // how long the spring rattles
constexpr adk::Millis finished  = 1500;    // the quiet that ends a rhythm

adk::Vector<char, 16> rhythm;        // a letter for each gap heard so far
adk::Stopwatch        sinceKnock;    // the time since the last knock
adk::Timer            quiet;         // runs out once the knocking stops

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    sinceKnock.start ();
}

void loop ()
{
    adk::update ();

    if (tap.activated () && sinceKnock.elapsed () > rattle)
    {
        hearKnock ();
    }

    knockLight.set (sinceKnock.elapsed () < 100);

    if (quiet.expired ())
    {
        judgeRhythm ();
    }
}

// Each knock after the first adds a letter for the gap before it.
void hearKnock ()
{
    if (quiet.isRunning ())
    {
        rhythm.push_back (sinceKnock.elapsed () < longGap ? 'S' : 'L');
    }

    sinceKnock.restart ();
    quiet.start (finished);
}

void judgeRhythm ()
{
    Serial.print ("Heard ");

    for (char gap : rhythm)
    {
        Serial.print (gap);
    }

    Serial.println ();

    if (isSecret ())
    {
        relay.toggle ();
        adk::wait (500);    // so the relay's click isn't heard as a knock
    }
    else
    {
        knockLight.blink (100);
        adk::wait (1000);
        knockLight.off ();
    }

    rhythm.clear ();
}

// Right if the rhythm has as many letters as the secret, all the same.
bool isSecret ()
{
    bool same = rhythm.size () == strlen (secret);

    for (size_t i = 0; same && i < rhythm.size (); ++i)
    {
        same = rhythm[i] == secret[i];
    }

    return same;
}
