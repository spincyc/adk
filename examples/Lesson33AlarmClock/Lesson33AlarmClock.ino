// Lesson 33: Alarm Clock
// A bedside clock that wakes you with a tune, set with a knob.

#include <Adk.h>

adk::Lcd           lcd        {31, 32, 33, 34, 35, 36};
adk::Rtc           rtc;
adk::RotaryEncoder knob       {18, 19};
adk::Button        knobButton {22};
adk::Speaker       speaker    {10};
adk::Button        snooze     {23};
adk::Every         tick       {250};

const adk::Note WakeUp [] = {
    {adk::note::c5, 150}, {adk::note::e5, 150}, {adk::note::g5, 150},
    {adk::note::c6, 300}, {adk::note::g5, 150}, {adk::note::c6, 450},
    {adk::note::rest, 700}};

const int SnoozeMinutes = 5;
const int MinutesPerDay = 24 * 60;

enum State
{
    Showing, SettingHour, SettingMinute, Ringing
};

// Times of day are counted in minutes after midnight: 7 * 60 is 07:00.
State state      = Showing;
int   alarm      = 7 * 60;
int   ringAt     = 7 * 60;        // the alarm, or the end of a snooze
int   lastMinute = -1;

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

    if (tick.ticked ())
    {
        checkTheClock ();
    }

    if (state == Ringing)
    {
        answerAlarm ();
    }
    else if (knobButton.wasPressed ())
    {
        nextSetting ();
    }
    else if (state != Showing && knob.turned () != 0)
    {
        changeAlarm (knob.turned ());
    }
}

void checkTheClock ()
{
    adk::DateTime now    = rtc.now ();
    int           minute = now.hour * 60 + now.minute;

    // Only as a new minute begins, so an alarm stopped in its own
    // minute stays stopped.
    if (state == Showing && minute == ringAt && minute != lastMinute)
    {
        state = Ringing;
    }

    lastMinute = minute;

    lcd.setCursor (0, 0);
    lcd.print ("Time    ");
    printTime (minute);
    lcd.print (':');
    printTwoDigits (now.second);
    showAlarm ();
}

// The knob's button steps on to the next state in the list: setting
// the hour, then the minutes, then back to showing the time.
void nextSetting ()
{
    state = (state == SettingMinute) ? Showing : State (state + 1);

    if (state == Showing)
    {
        ringAt = alarm;
        speaker.tone (adk::note::c6, 150);
    }

    showAlarm ();
}

void changeAlarm (int turned)
{
    int step = (state == SettingHour) ? 60 : 1;

    alarm = (alarm + turned * step + MinutesPerDay) % MinutesPerDay;
    showAlarm ();
}

void answerAlarm ()
{
    if (!speaker.isPlaying ())
    {
        speaker.play (WakeUp);
    }

    if (snooze.wasPressed () || knobButton.wasPressed ())
    {
        ringAt = snooze.wasPressed ()
               ? (lastMinute + SnoozeMinutes) % MinutesPerDay
               : alarm;
        speaker.stop ();
        state = Showing;
        showAlarm ();
    }
}

// The bottom row: the alarm or the end of a snooze, which part is being
// set, or a flashing wake-up call.
void showAlarm ()
{
    lcd.setCursor (0, 1);

    if (state == Ringing)
    {
        bool flash = millis () / 500 % 2;

        lcd.print (flash ? "    Wake up!    " : "                ");
        return;
    }

    const char* labels [] = {"Alarm   ", "Hour?   ", "Minute? "};
    bool        snoozing  = state == Showing && ringAt != alarm;

    lcd.print (snoozing ? "Snooze  " : labels[state]);
    printTime (snoozing ? ringAt : alarm);
    lcd.print ("   ");
}

void printTime (int minutes)
{
    printTwoDigits (minutes / 60);
    lcd.print (':');
    printTwoDigits (minutes % 60);
}

void printTwoDigits (int value)
{
    lcd.print (value / 10);
    lcd.print (value % 10);
}
