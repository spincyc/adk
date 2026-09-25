// Lesson 29: Menus
// Turn the knob to move through a menu on the LCD. Click to change the
// setting the arrow points at, turn to adjust it, and click again to go back.

#include <Adk.h>

adk::Lcd           lcd      {31, 32, 33, 34, 35, 36};
adk::RotaryEncoder knob     {18, 19};
adk::Button        click    {22};
adk::PwmOutput     lamp     {3};
adk::Stopwatch     lampTime;    // times the lamp's blinks and breaths

constexpr adk::Array levels {"0%", "10%", "20%", "30%", "40%", "50%",
                             "60%", "70%", "80%", "90%", "100%"};
constexpr adk::Array modes  {"Steady", "Blink", "Breathe"};
constexpr adk::Array speeds {"Slow", "Medium", "Fast"};

// A menu item: its name, the choices it offers, and which one is set.
struct Item
{
    const char*                  name;
    adk::Span<const char* const> choices;
    int                          setting;
};

adk::Array menu
{
    Item {"Level", levels, 6},
    Item {"Mode",  modes,  0},
    Item {"Speed", speeds, 1}
};

Item& level = menu[0];
Item& mode  = menu[1];
Item& speed = menu[2];

int  current = 0;        // the item on the top row
bool editing = false;    // turning changes its setting, not the item

void setup ()
{
    adk::setup ();
    lampTime.start ();
    showMenu ();
}

void loop ()
{
    adk::update ();

    if (click.wasPressed ())
    {
        editing = !editing;
        showMenu ();
    }

    if (knob.turned () != 0)
    {
        turnKnob (knob.turned ());
        showMenu ();
    }

    lamp.write (brightness ());
}

// Editing, turn the setting, stopping at either end of its choices.
// Browsing, move through the items, and round from the last to the first.
void turnKnob (int clicks)
{
    if (editing)
    {
        auto& item = menu[current];
        int   last = item.choices.size () - 1;

        item.setting = constrain (item.setting + clicks, 0, last);
    }
    else
    {
        current = wrap (current + clicks, menu.size ());
    }
}

// The current item on the top row and the next one below it, then the
// arrow, in front of what the knob will change: the item, or its setting.
void showMenu ()
{
    lcd.clear ();

    for (int row = 0; row < 2; ++row)
    {
        auto& item = menu[wrap (current + row, menu.size ())];

        lcd.at (1, row).print (item.name);
        lcd.at (9, row).print (item.choices[item.setting]);
    }

    lcd.at (editing ? 8 : 0, 0).print ('>');
}

// Blink is on for the first half of each period. Breathe rises through the
// first half and falls through the second.
int brightness ()
{
    constexpr adk::Array<long, 3> periods {2000, 1000, 500};    // by speed

    int  full   = level.setting * 255 / 10;
    long period = periods[speed.setting];
    long moment = lampTime.elapsed () % period;
    long rise   = moment < period / 2 ? moment : period - moment;

    switch (mode.setting)
    {
        case 1:  return moment < period / 2 ? full : 0;    // Blink
        case 2:  return full * rise / (period / 2);        // Breathe
        default: return full;                              // Steady
    }
}

// Count round in a circle: after the last comes the first, and before the
// first comes the last.
int wrap (int index, int count)
{
    index = index % count;
    return index < 0 ? index + count : index;
}
