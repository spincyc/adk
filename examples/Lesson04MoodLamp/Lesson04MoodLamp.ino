// Lesson 04: Mood Lamp
// An RGB LED fades to a new mood color each time you press the button.

#include <Adk.h>

adk::RgbLed lamp   {5, 6, 7};
adk::Button button {22};

constexpr adk::Array moods {adk::color::orange, adk::color::blue,
                            adk::color::green,  adk::color::pink};
constexpr int        rainbow = moods.size ();   // after the colors

int     mood = 0;
uint8_t hue  = 0;

void setup ()
{
    adk::setup ();
    lamp.fadeTo (moods[mood], 2000);
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        nextMood ();
    }

    if (mood == rainbow && !lamp.isFading ())
    {
        driftAroundTheWheel ();
    }
}

void nextMood ()
{
    mood = (mood + 1) % (rainbow + 1);

    if (mood != rainbow)
    {
        lamp.fadeTo (moods[mood], 1000);
    }
}

void driftAroundTheWheel ()
{
    hue = hue + 8;
    lamp.fadeTo (adk::wheel (hue), 300);
}
