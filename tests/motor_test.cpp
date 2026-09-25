#include "check.h"

#include <adk/motor.h>
#include <Arduino.h>

namespace {

    // The bridge as the motor sees it: "F" or "B" and the enable duty while
    // driven, "brake" with the enable high and both inputs low, and "coast"
    // with the enable low.
    std::string bridge ()
    {
        bool forward  = arduino::pin (30).output == HIGH;
        bool backward = arduino::pin (31).output == HIGH;
        int  enable   = arduino::pin (5).pwm;

        if (forward == backward)
        {
            return enable == 255 && !forward ? "brake" : enable == 0 ? "coast" : "fault";
        }

        return (forward ? "F" : "B") + std::to_string (enable);
    }
}

TEST (motorClaimsItsPinsAndStartsCoasting)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (5).mode == OUTPUT);
    CHECK (arduino::pin (5).output == LOW);
    CHECK (arduino::pin (30).mode == OUTPUT);
    CHECK (arduino::pin (31).mode == OUTPUT);
    CHECK (bridge () == "coast");
    CHECK (motor.speed () == 0);
}

TEST (motorEnableNeedsAPwmPin)
{
    adk::Motor motor {22, 30, 31};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NotPwm);
    CHECK (check::halted.pin == 22);
}

TEST (motorPinsCannotBeUsedTwice)
{
    adk::Motor motor {5, 30, 30};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 30);
}

TEST (motorRunsEitherWayAtItsSpeed)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();

    motor.speed (200);
    CHECK (bridge () == "F200");
    CHECK (motor.speed () == 200);

    motor.speed (80);
    CHECK (bridge () == "F80");

    motor.speed (400);
    CHECK (bridge () == "F255");
    CHECK (motor.speed () == 255);
}

TEST (motorStartsBackwardsAtOnceFromRest)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (-1000);

    CHECK (bridge () == "B255");
    CHECK (motor.speed () == -255);
}

TEST (reversingCoastsForATenthOfASecondFirst)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (200);
    adk::update (1000);

    motor.speed (-150);
    CHECK (bridge () == "coast");
    CHECK (motor.speed () == -150);

    adk::update (1000);
    adk::update (1099);
    CHECK (bridge () == "coast");

    motor.speed (-150);
    adk::update (1099);
    CHECK (bridge () == "coast");

    adk::update (1100);
    CHECK (bridge () == "B150");

    motor.speed (-60);
    CHECK (bridge () == "B60");
}

TEST (coastingBeforeReversingCountsTowardsTheWait)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (200);
    adk::update (0);

    motor.speed (0);
    CHECK (bridge () == "coast");
    adk::update (10);

    adk::update (60);
    motor.speed (-100);
    adk::update (109);
    CHECK (bridge () == "coast");

    adk::update (110);
    CHECK (bridge () == "B100");
}

TEST (aMotorThatHasSpunDownReversesAtOnce)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (200);
    motor.coast ();
    adk::update (0);
    adk::update (100);

    motor.speed (-100);
    CHECK (bridge () == "B100");
}

TEST (goingBackToTheOldDirectionCancelsAReversal)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (200);
    motor.speed (-200);
    adk::update (0);
    adk::update (50);

    motor.speed (150);
    CHECK (bridge () == "F150");

    adk::update (100);
    CHECK (bridge () == "F150");
}

TEST (brakeShortsTheMotorAndStillWaitsBeforeReversing)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (-200);

    motor.brake ();
    CHECK (bridge () == "brake");
    CHECK (arduino::pin (5).pwm == 255);
    CHECK (motor.speed () == 0);

    adk::update (0);
    motor.speed (100);
    adk::update (99);
    CHECK (bridge () == "brake");

    adk::update (100);
    CHECK (bridge () == "F100");

    motor.coast ();
    CHECK (bridge () == "coast");
    CHECK (motor.speed () == 0);
}

TEST (stoppedMotorCoastsAndForgetsAWaitingReversal)
{
    adk::Motor motor {5, 30, 31};

    adk::setup ();
    motor.speed (200);
    motor.speed (-200);
    adk::update (0);

    adk::stop ();
    CHECK (bridge () == "coast");
    CHECK (motor.speed () == 0);

    adk::update (1000);
    CHECK (bridge () == "coast");
}
