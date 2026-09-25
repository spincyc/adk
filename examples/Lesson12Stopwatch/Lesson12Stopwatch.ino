// Lesson 12: Stopwatch
// A stopwatch and kitchen timer on the four-digit display. The button on
// pin 22 starts and stops, 23 takes a lap or resets, 24 changes mode; the
// active buzzer on pin 12 clicks at every press and beeps at zero.

#include <Adk.h>

adk::FourDigitDisplay display   {37, 38, 39, 40, 41, 42, 43};
adk::Button           startStop {22};
adk::Button           lapReset  {23};
adk::Button           mode      {24};
adk::Buzzer           buzzer    {12};

// What the mode button steps through, in milliseconds: 0 is the stopwatch,
// which counts up; the others are timers that count down from 10 seconds,
// 1 minute and 3 minutes.
constexpr adk::Array<adk::Millis, 4> modes {0, 10000, 60000, 180000};

enum class State { Stopped, Running, Done };

State          state   = State::Stopped;
int            current = 0;         // which of the modes
adk::Stopwatch stopwatch;           // the time counted, in every mode
adk::Timer     alarm;               // a timer's zero
adk::Timer     lapShown;            // how long a lap stays on the display
adk::Millis    lap     = 0;         // the time the lap froze

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (alarm.expired ())
    {
        finish ();
    }

    if (startStop.wasPressed ())
    {
        buzzer.beep (20);
        startOrStop ();
    }

    if (lapReset.wasPressed ())
    {
        buzzer.beep (20);
        lapOrReset ();
    }

    if (mode.wasPressed () && state != State::Running)
    {
        buzzer.beep (20);
        current = (current + 1) % modes.size ();
        reset ();
    }

    showClock ();
}

void startOrStop ()
{
    switch (state)
    {
        case State::Stopped: start (); break;
        case State::Running: pause (); break;
        case State::Done:    reset (); break;
    }
}

void lapOrReset ()
{
    if (state == State::Running)
    {
        lap = clockTime ();
        lapShown.start (3000);
    }
    else
    {
        reset ();
    }
}

// A timer also sets its alarm for the time it has left.
void start ()
{
    stopwatch.start ();

    if (modes[current] > 0)
    {
        alarm.start (clockTime ());
    }

    state = State::Running;
}

void pause ()
{
    stopwatch.stop ();
    alarm.stop ();
    state = State::Stopped;
}

void reset ()
{
    stopwatch.reset ();
    lapShown.stop ();
    state = State::Stopped;
}

// A timer has reached zero: say so, and sound the buzzer three times.
void finish ()
{
    stopwatch.stop ();
    state = State::Done;
    display.show ("donE");

    for (int beep = 0; beep < 3; ++beep)
    {
        buzzer.beep (250);
        adk::wait (500);
    }
}

// What the clock shows: the time counted or, for a timer, the time left.
adk::Millis clockTime ()
{
    auto total = modes[current];
    auto time  = stopwatch.elapsed ();

    if (total == 0)
    {
        return time;
    }

    return time < total ? total - time : 0;
}

void showClock ()
{
    if (state != State::Done)
    {
        showTime (lapShown.isRunning () ? lap : clockTime ());
    }
}

// Seconds and tenths, such as " 12.3", with the dot lit after the seconds.
void showTime (adk::Millis ms)
{
    int  tenths = ms / 100 % 10000;
    char text [6];

    snprintf (text, sizeof text, "%3d.%d", tenths / 10, tenths % 10);
    display.show (text);
}
