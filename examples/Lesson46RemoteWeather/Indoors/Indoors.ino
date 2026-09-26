// Lesson 46: Remote Weather, Board B, indoors
// The garden's readings arrive as whole numbers. The LCD shows them in turn
// with the time the latest report came, and the light glows cold, mild or
// hot until the news stops.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Lcd    lcd   {31, 32, 33, 34, 35, 36};
adk::Rtc    rtc;
adk::RgbLed light {5, 6, 7};
adk::Every  page  {3000};    // the next reading on the top row

constexpr char degree = char (223);    // the LCD's own degree sign

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

    if (bridge.changed ("report"))
    {
        heardAt = rtc.now ();
        light.fadeTo (comfortOf (bridge.value ("air")), 1000);
    }

    if (!bridge.isConnected ())
    {
        light.fadeTo (adk::color::off, 1000);
    }

    if (page.ticked ())
    {
        shown = (shown + 1) % 3;
        showWeather ();
    }
}

// One reading on the top row, and below it when the latest report came,
// and whether the garden is still heard.
void showWeather ()
{
    lcd.at (0, 0);

    switch (shown)
    {
        case 0:
            adk::print (lcd, "Air ", celsius ("air"), degree, "C  ",
                        bridge.value ("humid"), "%  ");
            break;
        case 1:
            adk::print (lcd, "DS ", celsius ("probe"), " NTC ",
                        celsius ("ntc"), "  ");
            break;
        default:
            adk::print (lcd, "Light ", bridge.value ("light"), "%       ");
    }

    auto at   = heardAt;
    auto news = bridge.isConnected () ? "Heard   " : "No news ";

    adk::print (lcd.at (0, 1), news, at.hour / 10, at.hour % 10, ':',
                at.minute / 10, at.minute % 10, ':',
                at.second / 10, at.second % 10);
}

// A temperature arrives in tenths of a degree: 215 is 21.5.
adk::Fixed celsius (const char* name)
{
    return adk::fixed (bridge.value (name) / 10.0, 1);
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
