// Lesson 12: Stopwatch
// A stopwatch and countdown timer on the four-digit display. The button on pin 22 starts and
// stops, 23 takes a lap or resets, 24 changes mode; the active buzzer on pin 12 sounds at zero.

#include <Adk.h>

adk::FourDigitDisplay display   {37, 38, 39, 40, 41, 42, 43};
adk::Button           startStop {22};
adk::Button           lapReset  {23};
adk::Button           mode      {24};
adk::Buzzer           buzzer    {12};

// The modes the mode button steps through, in milliseconds: 0 is the
// stopwatch, which counts up; the others are timers that count down.
const unsigned long modes [] {0, 10000, 60000, 180000};

enum State
{
    Stopped,
    Running,
    Done
};

State         state      = Stopped;
int           current    = 0;       // which of the modes
unsigned long startedAt  = 0;       // millis () when the clock last started
unsigned long banked     = 0;       // time counted before that start
unsigned long lap        = 0;       // the time a lap froze
unsigned long lapAt      = 0;       // millis () when it did
bool          showingLap = false;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (startStop.wasPressed ())
    {
        startOrStop ();
    }

    if (lapReset.wasPressed ())
    {
        lapOrReset ();
    }

    if (mode.wasPressed () && state != Running)
    {
        buzzer.beep (20);
        current = (current + 1) % 4;
        reset ();
    }

    if (state == Running && modes[current] > 0 && elapsed () >= modes[current])
    {
        finish ();
    }

    showClock ();
}

void startOrStop ()
{
    buzzer.beep (20);

    if (state == Running)
    {
        banked = elapsed ();
        state  = Stopped;
    }
    else if (state == Stopped)
    {
        startedAt = millis ();
        state     = Running;
    }
    else
    {
        reset ();
    }
}

void lapOrReset ()
{
    buzzer.beep (20);

    if (state == Running)
    {
        lap        = clockTime ();
        lapAt      = millis ();
        showingLap = true;
    }
    else
    {
        reset ();
    }
}

void reset ()
{
    state      = Stopped;
    banked     = 0;
    showingLap = false;
}

// A timer has reached zero: say so, and sound the buzzer three times.
void finish ()
{
    state = Done;
    display.show ("donE");

    for (int beep = 0; beep < 3; ++beep)
    {
        buzzer.beep (250);
        adk::wait (500);
    }
}

// The time counted so far, in milliseconds.
unsigned long elapsed ()
{
    return state == Running ? banked + (millis () - startedAt) : banked;
}

// What the clock shows: the time counted or, for a timer, the time left.
unsigned long clockTime ()
{
    unsigned long total = modes[current];

    if (total == 0)
    {
        return elapsed ();
    }

    return elapsed () >= total ? 0 : total - elapsed ();
}

void showClock ()
{
    if (state == Done)
    {
        return;
    }

    showingLap = showingLap && millis () - lapAt < 3000;
    showTime (showingLap ? lap : clockTime ());
}

// Seconds and tenths, such as " 12.3", with the dot lit after the seconds.
void showTime (unsigned long ms)
{
    unsigned long tenths = (ms / 100) % 10000;
    char          text [8];

    snprintf (text, sizeof text, "%3lu.%lu", tenths / 10, tenths % 10);
    display.show (text);
}
