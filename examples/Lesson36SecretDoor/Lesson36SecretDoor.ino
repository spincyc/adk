// Lesson 36: Secret Door
// A latch that opens for a card it knows, or for the secret knock.

#include <Adk.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Rfid   reader {53, 45};
adk::Switch tap    {A12, adk::ActiveLow, 0};
adk::Servo  latch  {44};
adk::Buzzer buzzer {12};

// A friend is a card's number, from the Serial Monitor, and its owner.
struct Friend
{
    uint32_t    card;
    const char* name;
};

constexpr adk::Array friends
{
    Friend {0x12345678, "Ada"},
    Friend {0x9ABCDEF0, "Sam"}
};

// Lesson 35's secret knock, and the latch's two angles.
constexpr char        secret []   = "SLS";
constexpr adk::Millis longGap     = 400;
constexpr adk::Millis rattle      = 80;
constexpr adk::Millis finished    = 1500;
constexpr uint8_t     lockedAngle = 0;
constexpr uint8_t     openAngle   = 90;

adk::Text<16>         rhythm;        // a letter for each gap heard so far
adk::Stopwatch        sinceKnock;    // the time since the last knock
adk::Timer            quiet;         // runs out once the knocking stops

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    sinceKnock.start ();

    if (!reader.ok ())
    {
        show ("No card reader!", "Check its wires");
        adk::wait (3000);
    }

    lock ();
}

void loop ()
{
    adk::update ();

    if (reader.wasRead ())
    {
        checkCard (reader.uid ());
    }
    else if (tap.activated () && sinceKnock.elapsed () > rattle)
    {
        hearKnock ();
    }
    else if (quiet.expired () && rhythm == secret)
    {
        openFor ("the knocker");
    }
    else if (quiet.expired ())
    {
        refuse ("Wrong knock");
    }
}

void checkCard (uint32_t card)
{
    Serial.print ("Card 0x");
    Serial.println (card, HEX);

    for (const Friend& person : friends)
    {
        if (person.card == card)
        {
            openFor (person.name);
            return;
        }
    }

    refuse ("Unknown card");
}

// Lesson 35's letters, with a star on the screen for each knock.
void hearKnock ()
{
    if (quiet.isRunning ())
    {
        rhythm.print (sinceKnock.elapsed () < longGap ? 'S' : 'L');
    }
    else
    {
        show ("Listening...", "");
    }

    adk::print (lcd.at (rhythm.size () % 16, 1), '*');
    sinceKnock.restart ();
    quiet.start (finished);
}

// Open for five seconds. Cards and knocks meanwhile go unnoticed.
void openFor (const char* name)
{
    show ("Welcome,", name);
    latch.moveTo (openAngle, 500);
    buzzer.beep (80);
    adk::wait (160);
    buzzer.beep (80);
    adk::wait (5000);
    lock ();
}

void refuse (const char* reason)
{
    show (reason, "Access denied");
    buzzer.beep (600);
    adk::wait (2000);
    lock ();
}

// Wait for the latch to swing shut and the box to stop shaking, so the
// latch can't knock on its own door, then forget any knocks so far.
void lock ()
{
    latch.moveTo (lockedAngle, 500);
    adk::wait (700);
    rhythm.clear ();
    quiet.stop ();
    show ("Secret Door", "Card or knock...");
}

void show (const char* top, const char* bottom)
{
    lcd.clear ();
    adk::print (lcd.at (0, 0), top);
    adk::print (lcd.at (0, 1), bottom);
}
