// Lesson 24: Room Alarm
// Arm it from the remote; anyone who moves has ten seconds to key in the code.

#include <Adk.h>

adk::Lcd        screen   {31, 32, 33, 34, 35, 36};
adk::Switch     motion   {A12, adk::ActiveHigh};
adk::IrReceiver receiver {2};
adk::Buzzer     siren    {12};
adk::RgbLed     status   {5, 6, 7};
adk::Every      second   {1000};
adk::Every      wail     {300};

const uint16_t Code  = 1234;       // your secret: any four digits
const uint8_t  Delay = 10;         // seconds to leave, or to key in the code

const uint8_t digitButtons [] {adk::remote::digit0, adk::remote::digit1,
                               adk::remote::digit2, adk::remote::digit3,
                               adk::remote::digit4, adk::remote::digit5,
                               adk::remote::digit6, adk::remote::digit7,
                               adk::remote::digit8, adk::remote::digit9};

enum State { Disarmed, Leaving, Armed, Entering, Sounding };

State    state     = Disarmed;
uint8_t  countdown = 0;
uint16_t entered   = 0;
uint8_t  keys      = 0;
bool     flash     = false;

void setup ()
{
    adk::setup ();
    enter (Disarmed, adk::color::green, "Disarmed");
}

void loop ()
{
    adk::update ();

    if (receiver.wasReceived () && !receiver.isRepeat ())
    {
        pressed (receiver.command ());
    }

    if (state == Leaving && countedDown ())
    {
        enter (Armed, {40, 0, 0}, "ARMED");
    }
    else if (state == Armed && motion.isActive ())
    {
        startCountdown (Entering, adk::color::orange, "Code, quick!");
    }
    else if (state == Entering && countedDown ())
    {
        enter (Sounding, adk::color::red, "ALARM!");
    }
    else if (state == Sounding && wail.ticked ())
    {
        flash = !flash;
        siren.beep (150);
        status.show (flash ? adk::color::red : adk::color::blue);
    }
}

// Power arms a disarmed alarm; once it is armed in any way, the number
// buttons key in the code.
void pressed (uint8_t button)
{
    if (state == Disarmed && button == adk::remote::power)
    {
        startCountdown (Leaving, adk::color::yellow, "Leave now...");
    }

    for (uint8_t digit = 0; digit < 10; digit++)
    {
        if (state != Disarmed && button == digitButtons[digit])
        {
            keyIn (digit);
        }
    }
}

// A star for each digit; after the fourth, the right code disarms and a wrong
// one earns a long beep and a fresh start.
void keyIn (uint8_t digit)
{
    entered = entered * 10 + digit;
    keys++;
    screen.setCursor (5 + keys, 1);
    screen.print ('*');

    if (keys == 4 && entered == Code)
    {
        siren.off ();
        enter (Disarmed, adk::color::green, "Disarmed");
    }
    else if (keys == 4)
    {
        siren.beep (400);
        entered = 0;
        keys    = 0;
        screen.setCursor (6, 1);
        screen.print ("    ");
    }
}

// Once a second: a short beep and the new count. True when it reaches zero.
bool countedDown ()
{
    if (!second.ticked ())
    {
        return false;
    }

    countdown--;
    siren.beep (30);
    screen.setCursor (14, 0);
    screen.print (countdown < 10 ? " " : "");
    screen.print (countdown);
    return countdown == 0;
}

void startCountdown (State next, adk::Color color, const char* message)
{
    enter (next, color, message);
    countdown = Delay;
    second.restart ();
    screen.setCursor (14, 0);
    screen.print (countdown);
}

// A new state: its color, its message on top, and the code line below.
void enter (State next, adk::Color color, const char* message)
{
    state   = next;
    entered = 0;
    keys    = 0;
    status.show (color);
    screen.clear ();
    screen.print (message);
    screen.setCursor (0, 1);
    screen.print (next == Disarmed ? "Power arms it" : "Code:");
}
