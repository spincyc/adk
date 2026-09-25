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
        showDateAndTime (rtc.now ());
    }
}

void showDateAndTime (adk::DateTime now)
{
    lcd.setCursor (0, 0);

    if (!rtc.ok ())
    {
        lcd.print ("No clock found! ");
        lcd.setCursor (0, 1);
        lcd.print ("Check pins 20,21");
        return;
    }

    lcd.print ("Date  ");
    lcd.print (now.year);
    lcd.print ('-');
    printTwoDigits (now.month);
    lcd.print ('-');
    printTwoDigits (now.day);

    lcd.setCursor (0, 1);
    lcd.print ("Time    ");
    printTwoDigits (now.hour);
    lcd.print (':');
    printTwoDigits (now.minute);
    lcd.print (':');
    printTwoDigits (now.second);
}

void printTwoDigits (uint8_t value)
{
    if (value < 10)
    {
        lcd.print ('0');
    }

    lcd.print (value);
}
