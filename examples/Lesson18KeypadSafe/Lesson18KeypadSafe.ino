// Lesson 18: Keypad Safe
// A code lock: the right code and # open the servo latch; three wrong codes lock it out.

#include <Adk.h>
#include <EEPROM.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
adk::Servo  latch  {44};
adk::Buzzer buzzer {12};

enum State
{
    Locked,
    Open,
    Choosing
};

const uint8_t closedAngle = 0;
const uint8_t openAngle   = 90;
const uint8_t maxTries    = 3;
const int     lockoutTime = 30;     // seconds
const uint8_t savedMark   = 42;     // in EEPROM address 0 once a code is saved

State   state  = Locked;
int     code   = 1234;              // until you choose your own
int     typed  = 0;
uint8_t digits = 0;
uint8_t wrong  = 0;

void setup ()
{
    adk::setup ();

    code = loadCode ();
    enter (Locked, "Locked. Code?");
}

void loop ()
{
    adk::update ();

    char key = keypad.key ();

    if (key >= '0' && key <= '9')
    {
        typeDigit (key - '0');
    }
    else if (key == '#')
    {
        pressEnter ();
    }
    else if (key == '*' && state == Locked)
    {
        enter (Locked, "Locked. Code?");
    }
    else if (key == 'A' && state == Open)
    {
        enter (Choosing, "New code, then #");
    }
}

void typeDigit (int digit)
{
    if (state == Open || digits == 4)
    {
        return;
    }

    buzzer.beep (20);
    typed = typed * 10 + digit;
    ++digits;
    lcd.print (state == Choosing ? char ('0' + digit) : '*');
}

void pressEnter ()
{
    if (state == Open)
    {
        enter (Locked, "Locked. Code?");
    }
    else if (state == Choosing && digits == 4)
    {
        code = typed;
        saveCode (code);
        enter (Open, "New code saved");
    }
    else if (state == Locked && digits == 4 && typed == code)
    {
        wrong = 0;
        enter (Open, "Open. # locks");
    }
    else if (state == Locked)
    {
        refuse ();
    }
}

// A wrong code: try again, or after too many, wait. Keys pressed while
// adk::wait () counts down are never read.
void refuse ()
{
    ++wrong;
    buzzer.beep (600);

    if (wrong < maxTries)
    {
        enter (Locked, "Wrong! Try again");
        return;
    }

    enter (Locked, "Too many tries");
    lcd.print ("Wait");

    for (int left = lockoutTime; left > 0; --left)
    {
        lcd.setCursor (5, 1);
        lcd.print (left);
        lcd.print (" s ");
        adk::wait (1000);
    }

    wrong = 0;
    enter (Locked, "Locked. Code?");
}

// Every change of state comes here: the latch moves to match, the top row
// says what's happening, and the bottom row is cleared for typing.
void enter (State next, const char* message)
{
    state  = next;
    typed  = 0;
    digits = 0;
    latch.moveTo (state == Locked ? closedAngle : openAngle, 500);

    lcd.clear ();
    lcd.print (message);
    lcd.setCursor (0, 1);

    if (state == Open)
    {
        lcd.print ("A: new code");
    }
}

// EEPROM keeps its bytes with the power off. A byte holds 0 to 255, so the
// code's first two digits go in address 1 and its last two in address 2.
int loadCode ()
{
    bool saved = EEPROM.read (0) == savedMark;
    return saved ? EEPROM.read (1) * 100 + EEPROM.read (2) : 1234;
}

void saveCode (int number)
{
    EEPROM.update (1, number / 100);
    EEPROM.update (2, number % 100);
    EEPROM.update (0, savedMark);
}
