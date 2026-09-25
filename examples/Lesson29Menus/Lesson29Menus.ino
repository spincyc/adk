// Lesson 29: Menus
// Turn the knob to move through a menu on the LCD. Click to change the
// setting the arrow points at, turn to adjust it, and click again to go back.

#include <Adk.h>

adk::Lcd           lcd   {31, 32, 33, 34, 35, 36};
adk::RotaryEncoder knob  {18, 19};
adk::Button        click {22};
adk::PwmOutput     lamp  {3};

const char* const items  [] = {"Level", "Mode", "Speed"};
const uint8_t     limits [] = {10, 2, 2};     // the highest setting of each item
const char* const modes  [] = {"Steady", "Blink", "Breathe"};
const char* const speeds [] = {"Slow", "Medium", "Fast"};

uint8_t settings [] = {6, 0, 1};
uint8_t item        = 0;
bool    editing     = false;

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

    lamp.write (brightness ());
}

void turnKnob (int8_t clicks)
{
    if (editing)
    {
        settings[item] = constrain (settings[item] + clicks, 0, limits[item]);
    }
    else
    {
        item = (item + clicks + 3) % 3;
    }
}

// The chosen item on the top row, the next one below it. The arrow points
// at what the knob will change: the item, or its setting.
void showMenu ()
{
    lcd.clear ();
    showItem (0, item);
    showItem (1, (item + 1) % 3);
}

void showItem (uint8_t row, uint8_t which)
{
    bool chosen = (row == 0);

    lcd.setCursor (0, row);
    lcd.print (chosen && !editing ? '>' : ' ');
    lcd.print (items[which]);
    lcd.setCursor (8, row);
    lcd.print (chosen && editing ? '>' : ' ');

    if (which == 0)
    {
        lcd.print (settings[0] * 10);
        lcd.print ('%');
    }
    else
    {
        lcd.print (which == 1 ? modes[settings[1]] : speeds[settings[2]]);
    }
}

uint8_t brightness ()
{
    const unsigned long periods [] = {2000, 1000, 500};

    unsigned long period = periods[settings[2]];
    unsigned long moment = millis () % period;
    unsigned long full   = settings[0] * 255UL / 10;

    if (settings[1] == 1)
    {
        return moment < period / 2 ? full : 0;
    }

    if (settings[1] == 2)
    {
        unsigned long rise = moment < period / 2 ? moment : period - moment;
        return full * rise / (period / 2);
    }

    return full;
}
