// Lesson 46: Remote Weather, Board B, indoors
// Each reading carries its own report number. Missing or old readings
// show as dashes; only fresh, valid air data drives the comfort light.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Lcd    lcd   {31, 32, 33, 34, 35, 36};
adk::Rtc    rtc;
adk::RgbLed light {5, 6, 7};
adk::Every  page  {3000};    // the next reading on the top row

constexpr adk::Array names {"air", "humid", "probe", "ntc", "light"};
adk::Array<adk::Stopwatch, 5> age;    // each reading's new report number
constexpr char degree = char (223);
constexpr long noReading = -10000;
constexpr adk::Millis staleAfter = 10000;

adk::DateTime heardAt {};    // when any new sensor record last arrived
bool          heardAny = false;
int           shown = 0;

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

    for (uint8_t sensor = 0; sensor < names.size (); ++sensor)
    {
        if (bridge.changed (names[sensor]))
        {
            age[sensor].restart ();
            heardAt = rtc.now ();
            heardAny = true;
        }
    }

    long air = bridge.payload ("air");
    light.fadeTo (fresh (0) && air != noReading
                   ? comfortOf (air) : adk::color::off, 1000);

    if (page.ticked ())
    {
        shown = (shown + 1) % 3;
        showWeather ();
    }
}

// Every displayed value has its own freshness and validity check.
void showWeather ()
{
    lcd.at (0, 0).print ("                ");
    lcd.at (0, 0);

    if (!heardAny)
    {
        lcd.print ("Waiting for data");
        lcd.at (0, 1).print ("No report yet   ");
        return;
    }

    switch (shown)
    {
        case 0:
            lcd.print ("Air ");
            printReading (0, true);
            adk::print (lcd, degree, "C ");
            printReading (1, false);
            lcd.print ('%');
            break;
        case 1:
            lcd.print ("DS ");
            printReading (2, true);
            lcd.print (" NTC ");
            printReading (3, true);
            break;
        default:
            lcd.print ("Light ");
            printReading (4, false);
            lcd.print ('%');
    }

    bool allFresh = true;
    for (uint8_t sensor = 0; sensor < names.size (); ++sensor)
    {
        allFresh = allFresh && fresh (sensor);
    }
    auto at = heardAt;
    auto news = !bridge.isConnected () ? "No news "
                  : allFresh ? "Heard   " : "Stale   ";
    adk::print (lcd.at (0, 1), news, at.hour / 10, at.hour % 10, ':',
                at.minute / 10, at.minute % 10, ':',
                at.second / 10, at.second % 10);
}

// Another sensor's record or a heartbeat cannot freshen this reading.
bool fresh (uint8_t sensor)
{
    return bridge.isConnected () && age[sensor].isRunning ()
        && age[sensor].elapsed () < staleAfter;
}

void printReading (uint8_t sensor, bool temperature)
{
    long value = bridge.payload (names[sensor]);
    if (!fresh (sensor) || value == noReading)
    {
        lcd.print ("----");
    }
    else if (temperature)
    {
        adk::print (lcd, adk::fixed (value / 10.0, 1));
    }
    else
    {
        adk::print (lcd, value);
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
