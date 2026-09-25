// Lesson 32: Real-Time Clock
// A clock module keeps the time, even unplugged, and the LCD shows it.

#include <Adk.h>

adk::Lcd   lcd  {31, 32, 33, 34, 35, 36};
adk::Rtc   rtc;
adk::Every tick {200};

void setup ()
{
    adk::setup ();

    // A new clock sits stopped until it is set: start it at the time
    // this sketch was compiled. A running clock is left alone.
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
        auto now = rtc.now ();

        if (rtc.ok ())
        {
            showDateAndTime (now);
        }
        else
        {
            adk::print (lcd.at (0, 0), "No clock found! ");
            adk::print (lcd.at (0, 1), "Check pins 20,21");
        }
    }
}

// Each two-digit number is its tens, then its ones, as in Lesson 10, so
// nine minutes past eight shows as 20:09, not 20:9.
void showDateAndTime (adk::DateTime now)
{
    adk::print (lcd.at (0, 0), "Date  ", now.year, '-',
                now.month / 10, now.month % 10, '-',
                now.day / 10,   now.day % 10);

    adk::print (lcd.at (0, 1), "Time    ",
                now.hour / 10,   now.hour % 10,   ':',
                now.minute / 10, now.minute % 10, ':',
                now.second / 10, now.second % 10);
}
