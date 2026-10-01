// Lesson 48: Room Monitor, Board A, the listener
// The matrix graphs the room's sound, a column every half second. The
// screen says whether the room is quiet, lit and dry, and when it was last
// loud. A loud sound brings a chime; water on the sensor, an alarm that
// POWER on the remote hushes; and a silent room board, a beep, because no
// news isn't good news.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Lcd        lcd      {31, 32, 33, 34, 35, 36};
adk::Rtc        rtc;
adk::IrReceiver receiver {2};
adk::Speaker    speaker  {10};
adk::LedMatrix  matrix   {47, 48, 49};
adk::Every      step     {500};     // the graph moves on a column
adk::Every      beat     {5000};    // a beep while the room is silent

constexpr long perDot    = 25;     // how much more sound lights one more dot
constexpr long loudLevel = 150;    // the threshold for a loud sound
constexpr long wetAbove  = 100;    // the water sensor reads less when dry
constexpr long litAbove  = 30;     // the light, in percent

constexpr adk::Note chime [] = {{adk::note::e5, 150}, {adk::note::c5, 300}};
constexpr adk::Note alarm [] = {{adk::note::a5, 200}, {adk::note::e5, 200}};

adk::Array<long, 8> bars    {};    // dots in each column, oldest first
adk::DateTime       loudAt  {};    // when the latest loud sound began
bool                loudNow = false;
bool                hushed  = false;

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

    // A dry reading ends the hush, so the next wet one rings again.
    hushed = hushed && wet;

    if (wet && !hushed)
    {
        speaker.play (alarm);
    }

    if (step.ticked ())
    {
        listen ();
        showRoom (wet);
    }

    if (beat.ticked () && !bridge.isConnected ())
    {
        speaker.tone (adk::note::c4, 300);
    }
}

// Each half second: move the graph on a column, add the loudest sound the
// room heard, and chime when a loud sound begins. With no news, the room
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

    bool loud = level >= loudLevel;

    if (loud && !loudNow)
    {
        loudAt = rtc.now ();
        speaker.play (chime);
    }

    loudNow = loud;
}

// The top row: quiet or loud, lit or dark, dry or wet, or no news. The
// bottom row: when the latest loud sound began, 00:00:00 until the first.
void showRoom (bool wet)
{
    lcd.at (0, 0);

    if (bridge.isConnected ())
    {
        adk::print (lcd, loudNow ? "Loud   " : "Quiet  ",
                    bridge.value ("light") > litAbove ? "Lit  " : "Dark ",
                    wet ? "WET!" : "Dry ");
    }
    else
    {
        lcd.print ("No news!        ");
    }

    auto at = loudAt;

    adk::print (lcd.at (0, 1), "Loud at ", at.hour / 10, at.hour % 10, ':',
                at.minute / 10, at.minute % 10, ':',
                at.second / 10, at.second % 10);
}
