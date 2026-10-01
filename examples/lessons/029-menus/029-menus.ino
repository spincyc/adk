// Lesson 29: Menus
// Turn to move the arrow, click to change what it points at, click to go back.

#include <Adk.h>

adk::Lcd           lcd   {31, 32, 33, 34, 35, 36};
adk::RotaryEncoder knob  {18, 19};
adk::Button        click {22};
adk::PwmOutput     lamp  {3};

constexpr adk::Array levels {"0%", "10%", "20%", "30%", "40%", "50%",
                             "60%", "70%", "80%", "90%", "100%"};
constexpr adk::Array modes  {"Steady", "Off"};

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
    Item {"Mode",  modes,  0}
};

Item& level = menu[0];
Item& mode  = menu[1];

int  current = 0;        // the item the arrow points at
bool editing = false;    // turning changes its setting, not the item

void setup ()
{
    adk::setup ();
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

    lamp.write (mode.setting == 0 ? level.setting * 255 / 10 : 0);
}

// Editing moves the setting, browsing moves the arrow; neither wraps round.
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
        int last = menu.size () - 1;

        current = constrain (current + clicks, 0, last);
    }
}

// The arrow goes in front of what the knob will change: a name or a setting.
void showMenu ()
{
    lcd.clear ();

    for (int row = 0; row < 2; ++row)
    {
        auto& item = menu[row];

        lcd.at (1, row).print (item.name);
        lcd.at (9, row).print (item.choices[item.setting]);
    }

    lcd.at (editing ? 8 : 0, current).print ('>');
}
