// Lesson 22: Remote Control
// The remote switches an RGB lamp on and off, picks its color and dims it.

#include <Adk.h>

adk::IrReceiver receiver {2};
adk::RgbLed     lamp     {5, 6, 7};

// Buttons 1 to 6 pick these colors.
const uint8_t colorButtons [] {adk::remote::digit1, adk::remote::digit2,
                               adk::remote::digit3, adk::remote::digit4,
                               adk::remote::digit5, adk::remote::digit6};

const adk::Color colors [] {adk::color::red,    adk::color::green,
                            adk::color::blue,   adk::color::yellow,
                            adk::color::purple, adk::color::white};

adk::Color color      = adk::color::white;
uint8_t    brightness = 8;          // in eighths
bool       lit        = false;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    if (receiver.wasReceived ())
    {
        if (!receiver.isRepeat ())
        {
            printCode (receiver.command ());
        }

        obey (receiver.command (), receiver.isRepeat ());
        lamp.fadeTo (lit ? dimmed (color) : adk::color::off, 200);
    }
}

// Power toggles, volume dims or brightens (hold it down to keep going), and
// the number buttons choose a color.
void obey (uint8_t button, bool held)
{
    if (button == adk::remote::power && !held)
    {
        lit = !lit;
    }
    else if (button == adk::remote::volumeUp && brightness < 8)
    {
        brightness++;
    }
    else if (button == adk::remote::volumeDown && brightness > 1)
    {
        brightness--;
    }

    for (uint8_t index = 0; index < 6; index++)
    {
        if (button == colorButtons[index])
        {
            color = colors[index];
            lit   = true;
        }
    }
}

// The color at the chosen brightness: some eighths of the way up from off.
adk::Color dimmed (adk::Color full)
{
    return adk::blend (adk::color::off, full, brightness, 8);
}

void printCode (uint8_t button)
{
    Serial.print (button < 0x10 ? "Button code 0x0" : "Button code 0x");
    Serial.println (button, HEX);
}
