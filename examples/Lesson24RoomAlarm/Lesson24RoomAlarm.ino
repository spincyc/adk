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

constexpr int        secretCode   = 1234;   // any four digits
constexpr int        delaySeconds = 10;     // to leave, or to key in the code
constexpr adk::Color dimRed       {40, 0, 0};

// The remote's number buttons in order, so each one's place is its digit.
constexpr adk::Array digitButtons {adk::remote::digit0, adk::remote::digit1,
                                   adk::remote::digit2, adk::remote::digit3,
                                   adk::remote::digit4, adk::remote::digit5,
                                   adk::remote::digit6, adk::remote::digit7,
                                   adk::remote::digit8, adk::remote::digit9};

enum class State { Disarmed, Leaving, Armed, Entering, Sounding };

State state     = State::Disarmed;
int   countdown = 0;        // seconds left
int   entered   = 0;        // the code so far
int   keys      = 0;        // how many digits of it
bool  flash     = false;

void setup ()
{
    adk::setup ();
    enter (State::Disarmed, adk::color::green, "Disarmed");
}

void loop ()
{
    adk::update ();

    if (receiver.wasReceived () && !receiver.isRepeat ())
    {
        pressed (receiver.command ());
    }

    if (state == State::Leaving && countedDown ())
    {
        enter (State::Armed, dimRed, "ARMED");
    }
    else if (state == State::Armed && motion.isActive ())
    {
        enter (State::Entering, adk::color::orange, "Code, quick!");
    }
    else if (state == State::Entering && countedDown ())
    {
        enter (State::Sounding, adk::color::red, "ALARM!");
    }
    else if (state == State::Sounding && wail.ticked ())
    {
        flash = !flash;
        siren.beep (150);
        status.show (flash ? adk::color::red : adk::color::blue);
    }
}

// Power arms a disarmed alarm. Once it is armed in any way, the number
// buttons key in the code.
void pressed (uint8_t button)
{
    if (state == State::Disarmed && button == adk::remote::power)
    {
        enter (State::Leaving, adk::color::yellow, "Leave now...");
    }

    for (int digit = 0; digit < 10; ++digit)
    {
        if (state != State::Disarmed && button == digitButtons[digit])
        {
            keyIn (digit);
        }
    }
}

// A star for each digit. After the fourth, the right code disarms, and a
// wrong one earns a long beep and a fresh start.
void keyIn (int digit)
{
    entered = entered * 10 + digit;
    ++keys;
    screen.at (5 + keys, 1).print ('*');

    if (keys == 4 && entered == secretCode)
    {
        siren.off ();
        enter (State::Disarmed, adk::color::green, "Disarmed");
    }
    else if (keys == 4)
    {
        siren.beep (400);
        entered = 0;
        keys    = 0;
        screen.at (6, 1).print ("    ");
    }
}

// Once a second: a short beep and the new count. True when it reaches zero.
bool countedDown ()
{
    if (!second.ticked ())
    {
        return false;
    }

    --countdown;
    siren.beep (30);
    adk::print (screen.at (14, 0), countdown < 10 ? " " : "", countdown);
    return countdown == 0;
}

// Every change of state comes here: its light, its message on the top row
// with the countdown beside it while there is one, and a fresh code line.
void enter (State next, adk::Color light, const char* message)
{
    state     = next;
    countdown = delaySeconds;
    entered   = 0;
    keys      = 0;
    second.restart ();

    status.show (light);
    screen.clear ();
    screen.print (message);

    if (state == State::Leaving || state == State::Entering)
    {
        screen.at (14, 0).print (countdown);
    }

    screen.setCursor (0, 1);
    screen.print (state == State::Disarmed ? "Power arms it" : "Code:");
}
