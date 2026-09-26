#include "check.h"

#include <Arduino.h>

TEST (rgbLedClaimsThreePwmPinsAndStartsDark)
{
    adk::RgbLed led {5, 6, 7};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (5).pwm == 0);
    CHECK (arduino::pin (6).pwm == 0);
    CHECK (arduino::pin (7).pwm == 0);
}

TEST (rgbLedShowsAColor)
{
    adk::RgbLed led {5, 6, 7};

    adk::setup ();
    led.show (adk::color::orange);

    CHECK (arduino::pin (5).pwm == 255);
    CHECK (arduino::pin (6).pwm == 64);
    CHECK (arduino::pin (7).pwm == 0);
    CHECK (led.color () == adk::color::orange);
}

TEST (commonAnodeRgbLedInvertsEveryChannel)
{
    adk::RgbLed led {5, 6, 7, adk::ActiveLow};

    adk::setup ();
    CHECK (arduino::pin (5).pwm == 255);

    led.show (adk::color::red);
    CHECK (arduino::pin (5).pwm == 0);
    CHECK (arduino::pin (6).pwm == 255);
}

TEST (rgbLedNeedsPwmPins)
{
    adk::RgbLed led {5, 6, 22};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NotPwm);
    CHECK (check::halted.pin == 22);
}

TEST (fadeStartsOnTheNextUpdateAndEndsExactly)
{
    adk::RgbLed led {5, 6, 7};

    adk::setup ();
    led.fadeTo (adk::color::white, 1000);
    CHECK (led.isFading ());

    adk::update (5000);
    CHECK (led.color () == adk::color::off);

    adk::update (5500);
    CHECK (led.color ().red == 127);

    adk::update (6000);
    CHECK (led.color () == adk::color::white);
    CHECK (!led.isFading ());
}

TEST (showingAColorCancelsAFade)
{
    adk::RgbLed led {5, 6, 7};

    adk::setup ();
    led.fadeTo (adk::color::white, 1000);
    adk::update (0);
    led.show (adk::color::blue);
    adk::update (500);

    CHECK (led.color () == adk::color::blue);
}

TEST (fadingAgainToTheSameColorKeepsItsPace)
{
    adk::RgbLed led {5, 6, 7};

    adk::setup ();

    for (adk::Millis now = 0; now < 500; now += 10)
    {
        led.fadeTo (adk::color::white, 1000);
        adk::update (now);
    }

    led.fadeTo (adk::color::white, 1000);
    adk::update (500);
    CHECK (led.color ().red == 127);

    adk::update (1000);
    CHECK (led.color () == adk::color::white);
    CHECK (!led.isFading ());

    led.fadeTo (adk::color::white, 1000);
    CHECK (!led.isFading ());
}

TEST (fadingToAnotherColorStartsFromWhereTheFadeHasGot)
{
    adk::RgbLed led {5, 6, 7};

    adk::setup ();
    led.fadeTo (adk::color::red, 1000);
    adk::update (0);
    adk::update (500);
    CHECK (led.color ().red == 127);

    led.fadeTo (adk::color::blue, 1000);
    adk::update (500);
    adk::update (1000);
    CHECK (led.color () == (adk::Color {64, 0, 127}));

    adk::update (1500);
    CHECK (led.color () == adk::color::blue);
}

TEST (aFadeOfTenHoursIsHalfwayAfterFive)
{
    constexpr adk::Millis hour = 3600000;

    adk::RgbLed led {5, 6, 7};

    adk::setup ();
    led.fadeTo (adk::color::white, 10 * hour);
    adk::update (0);

    adk::update (5 * hour);
    CHECK (led.color ().red == 127);

    adk::update (9 * hour);
    CHECK (led.color ().red == 229);
    CHECK (led.isFading ());

    adk::update (10 * hour);
    CHECK (led.color () == adk::color::white);
}

TEST (stoppedRgbLedIsDarkAndForgetsItsFade)
{
    adk::RgbLed led {5, 6, 7};
    adk::RgbLed anode {2, 3, 4, adk::ActiveLow};

    adk::setup ();
    led.fadeTo (adk::color::white, 1000);
    anode.show (adk::color::white);
    adk::update (0);
    adk::update (500);

    adk::stop ();
    CHECK (led.color () == adk::color::off);
    CHECK (!led.isFading ());
    CHECK (arduino::pin (5).pwm == 0);
    CHECK (arduino::pin (2).pwm == 255);

    adk::update (1000);
    CHECK (led.color () == adk::color::off);
}

TEST (blendAndWheel)
{
    CHECK (adk::blend (adk::color::off, adk::color::white, 1, 2) == (adk::Color {127, 127, 127}));
    CHECK (adk::blend (adk::color::red, adk::color::blue, 2, 2) == adk::color::blue);
    CHECK (adk::wheel (0) == adk::color::red);
    CHECK (adk::wheel (85) == adk::color::green);
    CHECK (adk::wheel (170) == adk::color::blue);
}

TEST (theWheelComesRoundToRed)
{
    CHECK (adk::wheel (254).red > 245);
    CHECK (adk::wheel (254).blue < 10);
    CHECK (adk::wheel (255) == adk::color::red);
}
