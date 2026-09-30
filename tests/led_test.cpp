#include "check.h"

#include <Arduino.h>

TEST (ledStartsOffAndSwitches)
{
    adk::Led led {8};

    adk::setup ();
    CHECK (arduino::pin (8).mode == OUTPUT);
    CHECK (arduino::pin (8).output == LOW);

    led.on ();
    CHECK (led.isOn ());
    CHECK (arduino::pin (8).output == HIGH);

    led.toggle ();
    CHECK (!led.isOn ());
    CHECK (arduino::pin (8).output == LOW);
}

TEST (activeLowLedIsLitByALowPin)
{
    adk::Led led {8, adk::ActiveLow};

    adk::setup ();
    CHECK (arduino::pin (8).output == HIGH);

    led.on ();
    CHECK (arduino::pin (8).output == LOW);
}

TEST (blinkFlashesOnceAPeriodUntilTold)
{
    adk::Led led {8};

    adk::setup ();
    led.blink (1000);
    CHECK (led.isOn ());

    adk::update (0);
    adk::update (499);
    CHECK (led.isOn ());

    adk::update (500);
    CHECK (!led.isOn ());

    adk::update (1000);
    CHECK (led.isOn ());

    led.off ();
    adk::update (1500);
    CHECK (!led.isOn ());
}

TEST (blinkingAgainAtTheSamePeriodKeepsTheRhythm)
{
    adk::Led led {8};

    adk::setup ();
    led.blink (1000);
    adk::update (0);

    adk::update (400);
    led.blink (1000);
    adk::update (500);

    CHECK (!led.isOn ());
}

TEST (blinkingAtANewPeriodStartsAfresh)
{
    adk::Led led {8};

    adk::setup ();
    led.blink (1000);
    adk::update (0);
    adk::update (500);
    CHECK (!led.isOn ());

    led.blink (200);
    CHECK (led.isOn ());

    adk::update (600);
    adk::update (699);
    CHECK (led.isOn ());

    adk::update (700);
    CHECK (!led.isOn ());
}

// Updates come a little late, by a different amount each time, as they do
// from a busy loop (). Each flash still starts on the beat, as an Every
// ticks, not from the update that noticed the last one ended, so a
// blinking LED lights in exactly the updates an Every of its period ticks.
TEST (blinkKeepsInStepWithAnEveryOfTheSamePeriod)
{
    adk::Led   led  {8};
    adk::Every beat {300};

    adk::setup ();
    led.blink (300);

    adk::Millis start   = 0xFFFF0000;   // it crosses the wrap of millis () too
    adk::Millis now     = start;
    bool        wasOn   = true;
    bool        inStep  = true;
    unsigned    flashes = 0;

    adk::update (start);

    for (int pass = 0; pass < 5000; ++pass)
    {
        now += static_cast<adk::Millis> (random (1, 40));
        adk::update (now);

        bool lit = led.isOn () && !wasOn;
        inStep   = inStep && lit == beat.ticked ();
        flashes += lit ? 1 : 0;
        wasOn    = led.isOn ();
    }

    CHECK (inStep);
    CHECK (flashes == (now - start) / 300);
}

TEST (blinkStartsItsBeatAgainAfterALongGap)
{
    adk::Led   led  {8};
    adk::Every beat {1000};

    adk::setup ();
    led.blink (1000);
    adk::update (0);
    adk::update (500);
    CHECK (!led.isOn ());

    // Two periods and more with no update: no catching up, a new beat.
    adk::update (3700);
    CHECK (led.isOn ());
    CHECK (beat.ticked ());

    adk::update (4199);
    CHECK (led.isOn ());

    adk::update (4200);
    CHECK (!led.isOn ());

    adk::update (4700);
    CHECK (led.isOn ());
    CHECK (beat.ticked ());
}

TEST (stoppedLedIsOff)
{
    adk::Led led {8};

    adk::setup ();
    led.blink (100);
    adk::stop ();
    adk::update (1000);

    CHECK (!led.isOn ());
    CHECK (arduino::pin (8).output == LOW);
}
