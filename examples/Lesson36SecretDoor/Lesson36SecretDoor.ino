// Lesson 36: Secret Door
// A latch that opens for a card it knows, or for the secret knock.

#include <Adk.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Rfid   reader {53, 45};
adk::Switch tap    {A12, adk::ActiveLow, 0};
adk::Servo  latch  {44};
adk::Buzzer buzzer {12};

struct Friend
{
    uint32_t    card;
    const char* name;
};

// Your cards' numbers, from the Serial Monitor, and their owners.
const Friend Friends [] = {
    {0x12345678, "Ada"},
    {0x9ABCDEF0, "Sam"}};

const char          Secret []   = "SLS";  // knock-knock ... knock-knock
const unsigned long LongGap     = 400;
const unsigned long Rattle      = 80;
const unsigned long Finished    = 1500;
const unsigned long OpenTime    = 5000;
const uint8_t       LockedAngle = 0;
const uint8_t       OpenAngle   = 90;

bool          isOpen      = false;
unsigned long openedAt    = 0;
char          rhythm [16] = "";
uint8_t       knocks      = 0;
unsigned long lastKnock   = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

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

    unsigned long now = millis ();

    if (isOpen)
    {
        if (now - openedAt >= OpenTime)
        {
            lock ();
        }
    }
    else if (reader.wasRead ())
    {
        checkCard (reader.uid ());
    }
    else if (tap.activated () && now - lastKnock > Rattle)
    {
        hearKnock (now);
    }
    else if (knocks > 0 && now - lastKnock > Finished)
    {
        judgeRhythm ();
    }
}

void checkCard (uint32_t card)
{
    Serial.print ("Card 0x");
    Serial.println (card, HEX);

    for (const Friend& person : Friends)
    {
        if (person.card == card)
        {
            openFor (person.name);
            return;
        }
    }

    refuse ("Unknown card");
}

// Lesson 35's rhythm letters, with a star on the screen for each knock.
void hearKnock (unsigned long now)
{
    if (knocks == 0)
    {
        show ("Listening...", "");
    }
    else if (knocks < sizeof rhythm)
    {
        rhythm[knocks - 1] = (now - lastKnock < LongGap) ? 'S' : 'L';
        rhythm[knocks]     = '\0';
    }

    lcd.setCursor (knocks % 16, 1);
    lcd.print ('*');
    knocks++;
    lastKnock = now;
}

void judgeRhythm ()
{
    if (strcmp (rhythm, Secret) == 0)
    {
        openFor ("the knocker");
    }
    else
    {
        refuse ("Wrong knock");
    }
}

void openFor (const char* name)
{
    show ("Welcome,", name);
    latch.moveTo (OpenAngle, 500);
    buzzer.beep (80);
    adk::wait (160);
    buzzer.beep (80);

    isOpen   = true;
    openedAt = millis ();
}

void refuse (const char* reason)
{
    show (reason, "Access denied");
    buzzer.beep (600);
    adk::wait (2000);
    lock ();
}

// Back to waiting, once the latch has swung shut and the table is still.
void lock ()
{
    latch.moveTo (LockedAngle, 500);
    adk::wait (700);

    isOpen    = false;
    knocks    = 0;
    rhythm[0] = '\0';
    show ("Secret Door", "Card or knock...");
}

void show (const char* top, const char* bottom)
{
    lcd.clear ();
    lcd.print (top);
    lcd.setCursor (0, 1);
    lcd.print (bottom);
}
