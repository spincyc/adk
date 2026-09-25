// Lesson 35: Knocks and Relays
// The secret knock makes a relay switch a separate lamp on or off.

#include <Adk.h>

// No debouncing: a knock is over in a few milliseconds.
adk::Switch tap        {A12, adk::ActiveLow, 0};
adk::Led    knockLight {LED_BUILTIN};
adk::Relay  relay      {11};

// Each gap between knocks is S, short, or L, long. The secret "SLS" is
// knock-knock, pause, knock-knock.
const char          Secret [] = "SLS";
const unsigned long LongGap   = 400;   // ms: a gap this long or more is L
const unsigned long Rattle    = 80;    // ms a knock shakes the spring
const unsigned long Finished  = 1500;  // ms of quiet that end the rhythm

char          rhythm [16] = "";
uint8_t       knocks      = 0;
unsigned long lastKnock   = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    unsigned long now = millis ();

    if (tap.activated () && now - lastKnock > Rattle)
    {
        hearKnock (now);
    }

    knockLight.set (now - lastKnock < 100);

    if (knocks > 0 && now - lastKnock > Finished)
    {
        judgeRhythm ();
    }
}

// Each knock after the first adds a letter for the gap before it.
void hearKnock (unsigned long now)
{
    if (knocks > 0 && knocks < sizeof rhythm)
    {
        rhythm[knocks - 1] = (now - lastKnock < LongGap) ? 'S' : 'L';
        rhythm[knocks]     = '\0';
    }

    knocks++;
    lastKnock = now;
}

void judgeRhythm ()
{
    Serial.print ("Heard ");
    Serial.println (rhythm);

    if (strcmp (rhythm, Secret) == 0)
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

    knocks    = 0;
    rhythm[0] = '\0';
}
