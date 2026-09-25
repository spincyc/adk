#include "check.h"

#include <Arduino.h>
#include <adk/joystick.h>

namespace {

    const int Middle = 512;

    // The reading that puts an axis centered on 512 at a percentage of its
    // travel: 511 steps above the center, 512 below.
    int reading (int percent)
    {
        int span = (percent >= 0) ? 1023 - Middle : Middle;
        return Middle + (percent * span + ((percent >= 0) ? 99 : -99)) / 100;
    }

    // Push the stick and update.
    void push (int x, int y)
    {
        arduino::pin (A0).analog = reading (x);
        arduino::pin (A1).analog = reading (y);
        adk::update (0);
    }

    struct Stick
    {
        Stick ()
        {
            arduino::pin (A0).analog = Middle;
            arduino::pin (A1).analog = Middle;
            adk::setup ();
        }

        adk::Joystick joystick {A0, A1};
    };
}

TEST (joystickNeedsTwoAnalogPins)
{
    adk::Joystick joystick {A0, 22};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NotAnalog);
    CHECK (check::halted.pin == 22);
}

TEST (joystickAxesCannotShareAPin)
{
    adk::Joystick joystick {A0, 0};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == A0);
}

TEST (joystickRestsAtZeroWhereverItsCentreIs)
{
    adk::Joystick joystick {A0, A1};

    arduino::pin (A0).analog = 470;
    arduino::pin (A1).analog = 560;
    adk::setup ();
    adk::update (0);

    CHECK (!check::halted.happened);
    CHECK (joystick.x () == 0);
    CHECK (joystick.y () == 0);
    CHECK (joystick.direction () == adk::Joystick::Center);
    CHECK (joystick.moved () == adk::Joystick::Center);
}

TEST (joystickScalesEachSideToItsFullTravel)
{
    adk::Joystick joystick {A0, A1};

    arduino::pin (A0).analog = 400;
    arduino::pin (A1).analog = 400;
    adk::setup ();

    arduino::pin (A0).analog = 1023;
    arduino::pin (A1).analog = 0;
    adk::update (0);
    CHECK (joystick.x () == 100);
    CHECK (joystick.y () == -100);

    arduino::pin (A0).analog = 712;
    arduino::pin (A1).analog = 200;
    adk::update (1);
    CHECK (joystick.x () == 50);
    CHECK (joystick.y () == -50);
}

TEST (joystickIgnoresSmallMovesNearTheCentre)
{
    Stick stick;

    push (9, -9);
    CHECK (stick.joystick.x () == 0);
    CHECK (stick.joystick.y () == 0);

    push (10, -10);
    CHECK (stick.joystick.x () == 10);
    CHECK (stick.joystick.y () == -10);
}

TEST (aDirectionNeedsMoreThanHalfTravel)
{
    Stick stick;

    push (50, 0);
    CHECK (stick.joystick.direction () == adk::Joystick::Center);
    CHECK (stick.joystick.moved () == adk::Joystick::Center);

    push (51, 0);
    CHECK (stick.joystick.direction () == adk::Joystick::Right);
    CHECK (stick.joystick.moved () == adk::Joystick::Right);

    push (100, 0);
    CHECK (stick.joystick.direction () == adk::Joystick::Right);
    CHECK (stick.joystick.moved () == adk::Joystick::Center);
}

TEST (aDirectionHoldsUntilTheStickFallsBelow30)
{
    Stick stick;

    push (0, -80);
    CHECK (stick.joystick.moved () == adk::Joystick::Down);

    push (0, -30);
    CHECK (stick.joystick.direction () == adk::Joystick::Down);

    push (0, -29);
    CHECK (stick.joystick.direction () == adk::Joystick::Center);
    CHECK (stick.joystick.moved () == adk::Joystick::Center);

    push (0, -40);
    CHECK (stick.joystick.direction () == adk::Joystick::Center);

    push (0, -60);
    CHECK (stick.joystick.moved () == adk::Joystick::Down);
}

TEST (theAxisPushedFurthestWins)
{
    Stick stick;

    push (60, 80);
    CHECK (stick.joystick.direction () == adk::Joystick::Up);

    push (0, 0);
    push (-90, -60);
    CHECK (stick.joystick.direction () == adk::Joystick::Left);

    push (0, 0);
    push (70, 70);
    CHECK (stick.joystick.direction () == adk::Joystick::Up);
}

TEST (rollingRoundTheRimMovesToTheNextDirection)
{
    Stick stick;

    push (100, 0);
    CHECK (stick.joystick.moved () == adk::Joystick::Right);

    push (40, 90);
    CHECK (stick.joystick.direction () == adk::Joystick::Right);

    push (20, 95);
    CHECK (stick.joystick.direction () == adk::Joystick::Up);
    CHECK (stick.joystick.moved () == adk::Joystick::Up);
}
