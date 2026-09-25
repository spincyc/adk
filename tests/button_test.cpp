#include "check.h"

#include <Arduino.h>

TEST (buttonUsesThePullUpAndStartsReleased)
{
    adk::Button button {22};

    adk::setup ();

    CHECK (arduino::pin (22).mode == INPUT_PULLUP);
    CHECK (!button.isPressed ());
}

TEST (buttonPressIsDebouncedAndReportedOnce)
{
    adk::Button button {22};

    adk::setup ();
    adk::update (0);

    arduino::pin (22).input = LOW;
    adk::update (100);
    CHECK (!button.wasPressed ());

    arduino::pin (22).input = HIGH;
    adk::update (105);
    arduino::pin (22).input = LOW;
    adk::update (110);
    adk::update (129);
    CHECK (!button.isPressed ());

    adk::update (130);
    CHECK (button.wasPressed ());
    CHECK (button.isPressed ());

    adk::update (131);
    CHECK (!button.wasPressed ());
    CHECK (button.isPressed ());
}

TEST (buttonReportsItsRelease)
{
    adk::Button button {22};

    adk::setup ();
    arduino::pin (22).input = LOW;
    adk::update (0);
    adk::update (20);
    CHECK (button.wasPressed ());

    arduino::pin (22).input = HIGH;
    adk::update (100);
    adk::update (120);
    CHECK (button.wasReleased ());
    CHECK (!button.isPressed ());
}

TEST (aButtonHeldAtStartupIsAlreadyPressed)
{
    adk::Button button {22};

    arduino::pin (22).input = LOW;
    adk::setup ();
    adk::update (0);

    CHECK (button.isPressed ());
    CHECK (!button.wasPressed ());
}

TEST (activeHighSwitchHasNoPullUp)
{
    adk::Switch motion {7, adk::ActiveHigh, 0};

    arduino::pin (7).input = LOW;
    adk::setup ();
    CHECK (arduino::pin (7).mode == INPUT);

    arduino::pin (7).input = HIGH;
    adk::update (10);
    adk::update (10);
    CHECK (motion.activated ());
    CHECK (motion.isActive ());
}
