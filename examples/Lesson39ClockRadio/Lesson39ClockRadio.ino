// Lesson 39: Clock Radio
// A clock that wakes you with your station, fading in gently. Turn the
// rotary knob to tune, press it to set the alarm; the button switches the
// radio on and off, and the volume knob sets how loud it plays.

#include <Adk.h>

// adk::FmBand::Americas in North and South America, World elsewhere.
constexpr adk::FmBand band = adk::FmBand::World;

adk::Lcd           lcd         {31, 32, 33, 34, 35, 36};
adk::Rtc           rtc;
adk::FmRadio       radio       {40, 41, 42, band};
adk::RotaryEncoder dial        {18, 19};
adk::Button        dialButton  {22};
adk::Button        radioButton {23};
adk::AnalogInput   volumeKnob  {A0};
adk::Timer         fade;
adk::Every         tick        {200};

constexpr uint8_t bell [8] {0b00100, 0b01110, 0b01110, 0b01110,
                            0b11111, 0b00000, 0b00100, 0b00000};

constexpr int         minutesPerDay = 24 * 60;
constexpr adk::Millis fadeLength    = 30000;    // from silent to full

enum class State { Showing, SettingHour, SettingMinute };

// Times of day are in minutes after midnight, as in Lesson 33.
State state     = State::Showing;
int   alarm     = 7 * 60;
int   clockTime = -1;       // the clock's time when it was last read
bool  playing   = false;    // the radio is on

void setup ()
{
    adk::setup ();
    lcd.createChar (1, bell);

    if (!rtc.isRunning ())
    {
        rtc.set (adk::compiledAt ());
    }
}

void loop ()
{
    adk::update ();

    if (dialButton.wasPressed ())
    {
        dialPressed ();
    }

    if (dial.turned () != 0)
    {
        dialTurned (dial.turned ());
    }

    if (radioButton.wasPressed ())
    {
        playing = !playing;
        fade.stop ();
    }

    if (tick.ticked ())
    {
        readTheClock ();
        setVolume ();
    }
}

// Each press of the rotary knob moves on: the alarm's hour, its minute,
// and back to showing the time.
void dialPressed ()
{
    switch (state)
    {
        case State::Showing:       state = State::SettingHour;   break;
        case State::SettingHour:   state = State::SettingMinute; break;
        case State::SettingMinute: state = State::Showing;       break;
    }
}

// Showing the time, the rotary knob tunes the radio. Setting the alarm,
// each click moves it an hour or a minute, round and round the day.
void dialTurned (int clicks)
{
    switch (state)
    {
        case State::Showing:       radio.step (clicks);  return;
        case State::SettingHour:   alarm += clicks * 60; break;
        case State::SettingMinute: alarm += clicks;      break;
    }

    alarm = (alarm + minutesPerDay) % minutesPerDay;
}

// Five times a second: switch the radio on if it's time to wake, and show
// the time and the alarm on the top row.
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

    // Only as the alarm's minute begins, so a radio switched off stays off,
    // and only if it isn't playing already.
    if (state == State::Showing && !playing && time == alarm
        && time != clockTime)
    {
        playing = true;
        fade.start (fadeLength);
    }

    clockTime = time;
    lcd.at (0, 0);
    printTime (clockTime);
    adk::print (lcd, ':', now.second / 10, now.second % 10, "  ");
    lcd.write (1);
    printTime (alarm);
    showBottomRow ();
}

// Which part of the alarm the rotary knob sets, or the station's name and
// frequency.
void showBottomRow ()
{
    int frequency = radio.frequency ();

    adk::print (lcd.at (0, 1), "                ");
    lcd.at (0, 1);

    switch (state)
    {
        case State::SettingHour:   lcd.print ("Alarm hour?");   return;
        case State::SettingMinute: lcd.print ("Alarm minute?"); return;
        case State::Showing:       lcd.print (radio.stationName ()); break;
    }

    adk::print (lcd.at (11, 1), frequency < 1000 ? " " : "",
                adk::fixed (frequency / 10.0, 1));
}

// The volume knob sets how loud. Waking, the radio starts silent and
// climbs to the knob's volume as the half minute of the fade goes by.
void setVolume ()
{
    int         full   = playing ? volumeKnob.read (0, 15) : 0;
    adk::Millis faded  = fadeLength - fade.remaining ();
    int         volume = full * faded / fadeLength;

    if (volume != radio.volume ())
    {
        radio.setVolume (volume);
    }
}

// A time of day, in minutes after midnight, as hours and minutes: 07:05.
void printTime (int time)
{
    int hours   = time / 60;
    int minutes = time % 60;

    adk::print (lcd, hours / 10, hours % 10, ':', minutes / 10, minutes % 10);
}
