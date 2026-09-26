// Lesson 48: Baby Monitor, Board B, with the parent
// The matrix graphs the nursery's sound, a column every half second. The
// screen says whether the nursery is quiet, lit and dry, and when the baby
// last cried. A cry brings a soft chime; water on the floor, an alarm that
// POWER on the remote hushes; and a silent nursery, a beep, because no
// news isn't good news.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Lcd        lcd      {31, 32, 33, 34, 35, 36};
adk::Rtc        rtc;
adk::IrReceiver receiver {2};
adk::Speaker    speaker  {10};
adk::LedMatrix  matrix   {47, 48, 49};
adk::Every      step     {500};     // the graph moves on a column
adk::Every      beat     {5000};    // a beep while the nursery is silent

constexpr long perDot   = 25;     // how much more sound lights one more dot
constexpr long cryLevel = 150;    // a sound this loud is a cry
constexpr long wetAbove = 100;    // the water sensor reads less when dry
constexpr long litAbove = 30;     // the light, in percent

constexpr adk::Note chime [] = {{adk::note::e5, 150}, {adk::note::c5, 300}};
constexpr adk::Note alarm [] = {{adk::note::a5, 200}, {adk::note::e5, 200}};

adk::Array<long, 8> bars    {};    // dots in each column, oldest first
adk::DateTime       criedAt {};    // when the latest cry began
bool                crying = false;
bool                hushed = false;

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

    bool wet = bridge.value ("water") > wetAbove;

    if (receiver.wasReceived () && receiver.command () == adk::remote::power)
    {
        hushed = true;
    }

    // Hushed until the floor is dry again, so the next leak rings anew.
    hushed = hushed && wet;

    if (wet && !hushed)
    {
        speaker.play (alarm);
    }

    if (step.ticked ())
    {
        listen ();
        showNursery (wet);
    }

    if (beat.ticked () && !bridge.isConnected ())
    {
        speaker.tone (adk::note::c4, 300);
    }
}

// Each half second: move the graph on a column, add the loudest sound the
// nursery heard, and chime when a cry begins. With no news, the nursery
// counts as silent rather than as whatever it last said.
void listen ()
{
    long level = bridge.isConnected () ? bridge.value ("sound") : 0;

    for (int x = 0; x < 7; ++x)
    {
        bars[x] = bars[x + 1];
    }

    bars[7] = min (level / perDot, 8L);

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            matrix.set (x, 7 - y, y < bars[x]);
        }
    }

    bool loud = level >= cryLevel;

    if (loud && !crying)
    {
        criedAt = rtc.now ();
        speaker.play (chime);
    }

    crying = loud;
}

// The top row: quiet or crying, lit or dark, dry or wet, or no news. The
// bottom row: when the latest cry began, 00:00:00 until the first.
void showNursery (bool wet)
{
    lcd.at (0, 0);

    if (bridge.isConnected ())
    {
        adk::print (lcd, crying ? "Crying " : "Quiet  ",
                    bridge.value ("light") > litAbove ? "Lit  " : "Dark ",
                    wet ? "WET!" : "Dry ");
    }
    else
    {
        lcd.print ("No news!        ");
    }

    auto at = criedAt;

    adk::print (lcd.at (0, 1), "Cried   ", at.hour / 10, at.hour % 10, ':',
                at.minute / 10, at.minute % 10, ':',
                at.second / 10, at.second % 10);
}
