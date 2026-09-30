#include "check.h"

#include <Arduino.h>
#include <adk/sound_sensor.h>

#include <vector>

namespace {

    // 64 ms before millis () wraps round to zero.
    constexpr adk::Millis BeforeTheWrap = 0xFFFFFFC0;

    // Update once a millisecond over [from, to), across the wrap of millis ()
    // if it lies between them, and list when levels came.
    std::vector<adk::Millis> levelsBetween (const adk::SoundSensor& sound, adk::Millis from,
                                            adk::Millis to)
    {
        std::vector<adk::Millis> times;

        for (adk::Millis now = from; now != to; ++now)
        {
            adk::update (now);

            if (sound.measured ())
            {
                times.push_back (now);
            }
        }

        return times;
    }
}

TEST (soundSensorNeedsAnAnalogPin)
{
    adk::SoundSensor sound {22};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NotAnalog);
    CHECK (check::halted.pin == 22);
}

TEST (soundSensorPinCannotBeUsedTwice)
{
    adk::SoundSensor sound {A5};
    adk::Led         led   {A5};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == A5);
}

TEST (soundSensorMeasuresHowFarTheSignalSwings)
{
    adk::SoundSensor sound {A5};

    adk::setup ();
    CHECK (!check::halted.happened);
    CHECK (adk::isClaimed (A5));

    // Quiet: the signal sits at the middle.
    arduino::pin (A5).analog = 512;
    int levels = 0;

    for (adk::Millis now = 0; now <= 50; now += 5)
    {
        adk::update (now);
        levels += sound.measured () ? 1 : 0;
    }

    CHECK (levels == 1);
    CHECK (sound.level () == 0);

    // A clap: it swings from 300 to 700 and back.
    for (adk::Millis now = 55; now <= 100; now += 5)
    {
        arduino::pin (A5).analog = (now / 5) % 2 ? 300 : 700;
        adk::update (now);
    }

    CHECK (sound.measured ());
    CHECK (sound.level () == 400);
    adk::update (105);
    CHECK (!sound.measured ());
}

TEST (soundSensorReportsEachLevelForOneUpdate)
{
    adk::SoundSensor         sound  {A5};
    std::vector<adk::Millis> levels {1050, 1100, 1150, 1200};

    adk::setup ();
    arduino::pin (A5).analog = 512;

    // The first update starts the first window.
    CHECK (levelsBetween (sound, 1000, 1201) == levels);

    // Another update in the same millisecond is not a new level.
    adk::update (1200);
    CHECK (!sound.measured ());
}

TEST (soundSensorStartsEachWindowAfresh)
{
    adk::SoundSensor sound {A5};

    adk::setup ();
    arduino::pin (A5).analog = 512;
    adk::update (0);

    // A loud reading in the update that ends a window counts in that window.
    adk::update (25);
    arduino::pin (A5).analog = 900;
    adk::update (50);
    CHECK (sound.measured ());
    CHECK (sound.level () == 388);

    // The next window forgets it, and hears only the quiet.
    arduino::pin (A5).analog = 512;
    adk::update (75);
    CHECK (!sound.measured ());
    CHECK (sound.level () == 388);

    adk::update (100);
    CHECK (sound.measured ());
    CHECK (sound.level () == 0);
}

TEST (soundSensorMeasuresEvery50msAcrossTheWrapOfMillis)
{
    adk::SoundSensor         sound  {A5};
    std::vector<adk::Millis> levels {BeforeTheWrap + 50, BeforeTheWrap + 100, BeforeTheWrap + 150};

    adk::setup ();
    arduino::pin (A5).analog = 512;

    CHECK (levelsBetween (sound, BeforeTheWrap, BeforeTheWrap + 151) == levels);
}

TEST (soundSensorHearsAClapAcrossTheWrapOfMillis)
{
    adk::SoundSensor sound {A5};

    adk::setup ();
    arduino::pin (A5).analog = 512;
    adk::update (BeforeTheWrap);
    adk::update (BeforeTheWrap + 50);
    CHECK (sound.measured ());
    CHECK (sound.level () == 0);

    // The next window holds the wrap, and the clap either side of it.
    arduino::pin (A5).analog = 300;
    adk::update (BeforeTheWrap + 60);
    arduino::pin (A5).analog = 700;
    adk::update (BeforeTheWrap + 70);
    arduino::pin (A5).analog = 512;
    adk::update (BeforeTheWrap + 99);
    CHECK (!sound.measured ());

    adk::update (BeforeTheWrap + 100);
    CHECK (sound.measured ());
    CHECK (sound.level () == 400);
}
