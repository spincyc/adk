// Lesson 05: Melody Maker
// Four keys play C, D, E and G on the buzzer, after a little tune.

#include <Adk.h>

// Each key is a button and the pitch it plays.
struct Key
{
    adk::Button button;
    uint16_t    pitch;
};

adk::Array   keys    {Key {22, adk::note::c4}, Key {23, adk::note::d4},
                      Key {24, adk::note::e4}, Key {25, adk::note::g4}};
adk::Speaker speaker {10};

constexpr adk::Note tune [] = {
    {adk::note::e4, 400}, {adk::note::d4, 400},     // Ma- ry
    {adk::note::c4, 400}, {adk::note::d4, 400},     // had a
    {adk::note::e4, 400}, {adk::note::e4, 400},     // lit- tle
    {adk::note::e4, 800},                           // lamb,
    {adk::note::d4, 400}, {adk::note::d4, 400},     // lit- tle
    {adk::note::d4, 800},                           // lamb,
    {adk::note::e4, 400}, {adk::note::g4, 400},     // lit- tle
    {adk::note::g4, 800},                           // lamb.
};

uint16_t sounding = adk::note::rest;    // the pitch a key is playing

void setup ()
{
    adk::setup ();
    speaker.play (tune);
}

void loop ()
{
    adk::update ();

    for (auto& key : keys)
    {
        if (key.button.wasPressed ())
        {
            speaker.tone (key.pitch);
            sounding = key.pitch;
        }
        else if (key.button.wasReleased () && key.pitch == sounding)
        {
            speaker.stop ();
            sounding = adk::note::rest;
        }
    }
}
