// Lesson 05: Melody Maker
// Four buttons play C, D, E and G on the buzzer, after a little tune.

#include <Adk.h>

adk::Button  keys [] {{22}, {23}, {24}, {25}};
adk::Speaker speaker {10};

const uint16_t pitches [] = {adk::note::c4, adk::note::d4, adk::note::e4,
                             adk::note::g4};

const adk::Note tune [] = {
    {adk::note::e4, 400}, {adk::note::d4, 400},     // Ma- ry
    {adk::note::c4, 400}, {adk::note::d4, 400},     // had a
    {adk::note::e4, 400}, {adk::note::e4, 400},     // lit- tle
    {adk::note::e4, 800},                           // lamb,
    {adk::note::d4, 400}, {adk::note::d4, 400},     // lit- tle
    {adk::note::d4, 800},                           // lamb,
    {adk::note::e4, 400}, {adk::note::g4, 400},     // lit- tle
    {adk::note::g4, 800},                           // lamb.
};

int sounding = -1;          // the key whose note is playing, or -1

void setup ()
{
    adk::setup ();
    speaker.play (tune);
}

void loop ()
{
    adk::update ();

    for (int key = 0; key < 4; key++)
    {
        if (keys[key].wasPressed ())
        {
            speaker.tone (pitches[key]);
            sounding = key;
        }
        else if (keys[key].wasReleased () && key == sounding)
        {
            speaker.stop ();
            sounding = -1;
        }
    }
}
