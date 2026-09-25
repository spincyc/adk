// Lesson 33: Alarm Clock
// A bedside clock that wakes you with a tune, set with a knob.

#include <Adk.h>

adk::Lcd           lcd        {31, 32, 33, 34, 35, 36};
adk::Rtc           rtc;
adk::RotaryEncoder knob       {18, 19};
adk::Button        knobButton {22};
adk::Speaker       speaker    {10};
adk::Button        snooze     {23};
adk::Every         tick       {100};

constexpr adk::Note wakeUp [] =
{
    {adk::note::c5, 150}, {adk::note::e5, 150}, {adk::note::g5, 150},
    {adk::note::c6, 300}, {adk::note::g5, 150}, {adk::note::c6, 450},
    {adk::note::rest, 700}
};

constexpr int minutesPerDay = 24 * 60;
constexpr int snoozeMinutes = 5;

enum class State { Showing, SettingHour, SettingMinute, Ringing };

// Times of day are counted in minutes after midnight: 7 * 60 is 07:00.
State state     = State::Showing;
int   alarm     = 7 * 60;
int   ringAt    = alarm;    // the alarm, or the end of a snooze
int   clockTime = -1;       // the clock's time when it was last read

void setup ()
{
    adk::setup ();

    if (!rtc.isRunning ())
    {
        rtc.set (adk::compiledAt ());
    }
}

void loop ()
{
    adk::update ();

    if (knobButton.wasPressed ())
    {
        knobPressed ();
    }

    if (knob.turned () != 0)
    {
        knobTurned (knob.turned ());
    }

    if (snooze.wasPressed () && state == State::Ringing)
    {
        sleepUntil (clockTime + snoozeMinutes);
    }

    if (tick.ticked ())
    {
        readTheClock ();
    }
}

// What a press of the knob means depends on the state of the clock.
void knobPressed ()
{
    switch (state)
    {
        case State::Showing:       state = State::SettingHour;   break;
        case State::SettingHour:   state = State::SettingMinute; break;
        case State::Ringing:       sleepUntil (alarm);           break;
        case State::SettingMinute:
            sleepUntil (alarm);
            speaker.tone (adk::note::c6, 150);
            break;
    }
}

// While setting, each click moves the alarm an hour or a minute, round
// and round the day.
void knobTurned (int clicks)
{
    switch (state)
    {
        case State::SettingHour:   alarm += clicks * 60; break;
        case State::SettingMinute: alarm += clicks;      break;
        case State::Showing:
        case State::Ringing:       return;
    }

    alarm = (alarm + minutesPerDay) % minutesPerDay;
}

// Quiet, and back to showing the time until it's time to ring again.
void sleepUntil (int time)
{
    speaker.stop ();
    ringAt = time % minutesPerDay;
    state  = State::Showing;
}

// Ten times a second: ring if it's time, and show the time and the alarm.
void readTheClock ()
{
    auto now = rtc.now ();

    if (!rtc.ok ())
    {
        adk::print (lcd.at (0, 0), "No clock found! ");
        adk::print (lcd.at (0, 1), "Check pins 20,21");
        return;
    }

    int time = now.hour * 60 + now.minute;

    // Only as the minute begins, so an alarm stopped in its own minute
    // stays stopped.
    if (state == State::Showing && time == ringAt && time != clockTime)
    {
        state = State::Ringing;
    }

    // The tune starts again each time it ends, until someone stops it.
    if (state == State::Ringing && !speaker.isPlaying ())
    {
        speaker.play (wakeUp);
    }

    clockTime = time;
    adk::print (lcd.at (0, 0), "Time    ");
    printTime (clockTime);
    adk::print (lcd, ':', now.second / 10, now.second % 10);
    showAlarm (now.second);
}

// The bottom row: when the alarm rings next, which part of it the knob
// sets, or a wake-up call that flashes with the seconds.
void showAlarm (int second)
{
    const char* label = ringAt == alarm ? "Alarm   " : "Snooze  ";

    switch (state)
    {
        case State::Showing:                           break;
        case State::SettingHour:   label = "Hour?   "; break;
        case State::SettingMinute: label = "Minute? "; break;
        case State::Ringing:
            adk::print (lcd.at (0, 1), second % 2 == 0 ? "    Wake up!    "
                                                       : "                ");
            return;
    }

    adk::print (lcd.at (0, 1), label);
    printTime (state == State::Showing ? ringAt : alarm);
    lcd.print ("   ");
}

// A time of day, in minutes after midnight, as hours and minutes: 07:05.
void printTime (int time)
{
    int hours   = time / 60;
    int minutes = time % 60;

    adk::print (lcd, hours / 10, hours % 10, ':', minutes / 10, minutes % 10);
}
