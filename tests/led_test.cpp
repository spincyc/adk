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
