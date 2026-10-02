// Lesson 46: Remote Weather, Board A, indoors
// The garden's report arrives every five seconds. The screen shows its
// readings one after another, with the time the latest report came, and
// the light glows blue, green or red for a cold, mild or hot garden.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Lcd    lcd   {31, 32, 33, 34, 35, 36};
adk::Rtc    rtc;
adk::RgbLed light {5, 6, 7};
adk::Every  page  {3000};    // the next reading on the top row

constexpr long noReading = -10000;        // a thermometer that didn't answer
constexpr char degree    = char (223);    // the LCD's own degree sign

adk::DateTime heardAt {};    // when the latest report arrived
int           shown = 0;     // which reading the top row shows

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

    // A new report number: the garden has measured again, even if every
    // reading is the same as last time.
    if (bridge.changed ("report"))
    {
        heardAt = rtc.now ();
    }

    // The light needs a temperature: it fades out without one.
    long air   = bridge.payload ("air");
    bool known = hasReading ("air") && air != noReading;
    light.fadeTo (known ? comfortOf (air) : adk::color::off, 1000);

    if (page.ticked ())
    {
        shown = (shown + 1) % 3;
        showWeather ();
    }
}

// Readings are worth showing once a report has come, and while the
// garden can still be heard.
bool hasNews ()
{
    return bridge.isConnected () && bridge.value ("report") > 0;
}

// A reading must belong to this report, not a previous packet or startup.
bool hasReading (const char* name)
{
    return hasNews () && bridge.value (name) == bridge.value ("report");
}

// One reading at a time on the top row; below it, when the latest report
// came, or that the garden has gone quiet.
void showWeather ()
{
    lcd.clear ();

    switch (shown)
    {
        case 0:
            lcd.print ("Air ");
            showReading ("air", true);
            adk::print (lcd, degree, "C ");
            showReading ("humid", false);
            lcd.print ('%');
            break;
        case 1:
            lcd.print ("DS ");
            showReading ("probe", true);
            lcd.print (" NTC ");
            showReading ("ntc", true);
            break;
        default:
            lcd.print ("Light ");
            showReading ("light", false);
            lcd.print ('%');
    }

    lcd.at (0, 1);

    if (bridge.value ("report") == 0)
    {
        lcd.print ("No report yet");
        return;
    }

    lcd.print (bridge.isConnected () ? "Heard   " : "No news ");
    adk::print (lcd, heardAt.hour / 10, heardAt.hour % 10, ':',
                heardAt.minute / 10, heardAt.minute % 10, ':',
                heardAt.second / 10, heardAt.second % 10);
}

// A temperature in tenths shows with its fraction: 215 is 21.5.
void showReading (const char* name, bool temperature)
{
    long value = bridge.payload (name);

    if (!hasReading (name) || value == noReading)
    {
        lcd.print ("----");
    }
    else if (temperature)
    {
        adk::print (lcd, adk::fixed (value / 10.0, 1));
    }
    else
    {
        lcd.print (value);
    }
}

// Blue below 18 degrees, red above 25 and green between, as in Lesson 15.
adk::Color comfortOf (long tenths)
{
    if (tenths < 180)
    {
        return adk::color::blue;
    }

    return tenths > 250 ? adk::color::red : adk::color::green;
}
