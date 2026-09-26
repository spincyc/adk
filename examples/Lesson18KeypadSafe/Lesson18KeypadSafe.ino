// Lesson 18: Keypad Safe
// The right code opens the latch; three wrong codes lock the keypad out.

#include <Adk.h>
#include <EEPROM.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
adk::Servo  latch  {44};
adk::Buzzer buzzer {12};

enum class State { Locked, Open, Choosing };

constexpr int     lockedAngle = 0;
constexpr int     openAngle   = 90;
constexpr int     maxTries    = 3;
constexpr int     lockoutTime = 30;    // seconds
constexpr uint8_t savedMark   = 42;    // in EEPROM once a code is saved

State                state = State::Locked;
adk::Array           code  {'1', '2', '3', '4'};  // until you choose one
adk::Vector<char, 4> typed;                       // the keys typed so far
int                  wrong = 0;                   // wrong codes in a row

void setup ()
{
    adk::setup ();

    loadCode ();
    enter (State::Locked, "Locked. Code?");
}

void loop ()
{
    adk::update ();

    char key = keypad.key ();

    if (key >= '0' && key <= '9')
    {
        typeDigit (key);
    }
    else if (key == '#')
    {
        pressEnter ();
    }
    else if (key == '*')
    {
        enter (State::Locked, "Locked. Code?");
    }
    else if (key == 'A' && state == State::Open)
    {
        enter (State::Choosing, "New code, then #");
    }
}

// Each digit clicks, and shows as a star, or as itself while choosing.
void typeDigit (char key)
{
    if (state != State::Open && !typed.full ())
    {
        typed.push_back (key);
        buzzer.beep (20);
        lcd.print (state == State::Choosing ? key : '*');
    }
}

void pressEnter ()
{
    if (state == State::Open)
    {
        enter (State::Locked, "Locked. Code?");
    }
    else if (state == State::Choosing && typed.full ())
    {
        saveCode (typed);
        enter (State::Open, "New code saved");
    }
    else if (state == State::Locked && adk::equal (typed, code))
    {
        wrong = 0;
        enter (State::Open, "Open. # locks");
    }
    else if (state == State::Locked)
    {
        refuse ();
    }
}

// A wrong code: try again or, after too many, wait. Keys pressed while
// adk::wait () counts down are never read.
void refuse ()
{
    wrong++;
    buzzer.beep (600);

    if (wrong < maxTries)
    {
        enter (State::Locked, "Wrong! Try again");
        return;
    }

    enter (State::Locked, "Too many tries");

    for (int left = lockoutTime; left > 0; left--)
    {
        adk::print (lcd.at (0, 1), "Wait ", left, " s ");
        adk::wait (1000);
    }

    wrong = 0;
    enter (State::Locked, "Locked. Code?");
}

// Every change of state comes here: the latch moves to match, the top row
// says what's happening, and the bottom row is cleared for typing.
void enter (State next, const char* message)
{
    state = next;
    typed.clear ();
    latch.moveTo (state == State::Locked ? lockedAngle : openAngle, 500);

    lcd.clear ();
    lcd.print (message);
    lcd.at (0, 1);

    if (state == State::Open)
    {
        lcd.print ("A: new code");
    }
}

// EEPROM keeps its bytes with the power off: the mark in address 0 once a
// code is saved, and the code's four keys in addresses 1 to 4.
void loadCode ()
{
    if (EEPROM.read (0) == savedMark)
    {
        for (size_t i = 0; i < code.size (); i++)
        {
            code[i] = EEPROM.read (1 + i);
        }
    }
}

// The keys become the code, here and in EEPROM.
void saveCode (adk::Span<const char> keys)
{
    for (size_t i = 0; i < keys.size (); i++)
    {
        code[i] = keys[i];
        EEPROM.update (1 + i, keys[i]);
    }

    EEPROM.update (0, savedMark);
}
