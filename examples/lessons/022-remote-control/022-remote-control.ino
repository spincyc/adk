// Lesson 22: Remote Control
// The remote switches an RGB lamp on and off, picks its color and dims it.

#include <Adk.h>

adk::IrReceiver receiver {2};
adk::RgbLed     lamp     {5, 6, 7};

// A number button on the remote, and the color it chooses.
struct Choice
{
    uint8_t    button;
    adk::Color color;
};

constexpr adk::Array choices
{
    Choice {adk::remote::digit1, adk::color::red},
    Choice {adk::remote::digit2, adk::color::green},
    Choice {adk::remote::digit3, adk::color::blue},
    Choice {adk::remote::digit4, adk::color::white}
};

adk::Color color      = adk::color::white;
int        brightness = 8;          // in eighths
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
        uint8_t button = receiver.command ();

        if (!receiver.isRepeat ())
        {
            adk::println (Serial, "Button code 0x", adk::hex (button, 2));
        }

        obey (button);

        auto dimmed = adk::blend (adk::color::off, color, brightness, 8);
        lamp.fadeTo (lit ? dimmed : adk::color::off, 200);
    }
}

// Holding power must not make the lamp flicker; holding volume keeps going.
void obey (uint8_t button)
{
    if (button == adk::remote::power && !receiver.isRepeat ())
    {
        lit = !lit;
    }
    else if (button == adk::remote::volumeUp && brightness < 8)
    {
        ++brightness;
    }
    else if (button == adk::remote::volumeDown && brightness > 1)
    {
        --brightness;
    }

    for (auto choice : choices)
    {
        if (button == choice.button)
        {
            color = choice.color;
            lit   = true;
        }
    }
}
