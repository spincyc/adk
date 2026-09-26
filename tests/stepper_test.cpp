#include "check.h"

#include <adk/stepper.h>
#include <Arduino.h>

namespace {

    // IN1-IN4 on pins 22-25, as the driver board's LEDs would show them.
    std::string coils ()
    {
        std::string lit;

        for (uint8_t pin = 22; pin <= 25; ++pin)
        {
            lit += arduino::pin (pin).output == HIGH ? '1' : '0';
        }

        return lit;
    }
}

TEST (stepperClaimsFourOutputsAndStartsReleased)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (22).mode == OUTPUT);
    CHECK (arduino::pin (25).mode == OUTPUT);
    CHECK (coils () == "0000");
    CHECK (stepper.position () == 0);
    CHECK (!stepper.isMoving ());
}

TEST (stepperPinsCannotBeUsedTwice)
{
    adk::Stepper       stepper {22, 23, 24, 25};
    adk::DigitalOutput output  {24};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 24);
}

TEST (stepperPinsMustExist)
{
    adk::Stepper stepper {22, 23, 24, 90};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NoSuchPin);
    CHECK (check::halted.pin == 90);
}

TEST (forwardStepsWalkTheHalfStepPhases)
{
    const char* phases [] = {"1100", "0100", "0110", "0010", "0011", "0001", "1001", "1000"};

    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (8);
    CHECK (stepper.isMoving ());

    for (int step = 0; step < 8; ++step)
    {
        adk::update (static_cast<adk::Millis> (step * 2));
        CHECK (coils () == phases[step]);
        CHECK (stepper.position () == step + 1);
    }

    CHECK (!stepper.isMoving ());
}

TEST (backwardStepsWalkThePhasesTheOtherWay)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (-3);

    adk::update (0);
    CHECK (coils () == "1001");

    adk::update (2);
    CHECK (coils () == "0001");

    adk::update (4);
    CHECK (coils () == "0011");
    CHECK (stepper.position () == -3);
}

TEST (stepsComeOncePerIntervalAndNoSooner)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (10);

    adk::update (1000);
    CHECK (stepper.position () == 1);

    adk::update (1001);
    CHECK (stepper.position () == 1);

    adk::update (1002);
    CHECK (stepper.position () == 2);

    adk::update (1002);
    CHECK (stepper.position () == 2);

    adk::update (1003);
    adk::update (1004);
    CHECK (stepper.position () == 3);
}

TEST (unevenIntervalsAverageOutExactly)
{
    adk::Stepper stepper {22, 23, 24, 25};
    std::string  steps;

    adk::setup ();
    stepper.speed (300);
    stepper.step (100);

    for (adk::Millis now = 0; now <= 10; ++now)
    {
        long before = stepper.position ();
        adk::update (now);
        steps += stepper.position () != before ? 'S' : '.';
    }

    CHECK (steps == "S...S..S..S");
}

TEST (aLateUpdateTakesOneStepNotABurst)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.speed (1000);
    stepper.step (100);

    adk::update (0);
    adk::update (50);
    CHECK (stepper.position () == 2);

    adk::update (50);
    CHECK (stepper.position () == 2);

    adk::update (51);
    CHECK (stepper.position () == 3);
}

TEST (speedIsKeptBetweenOneAndFiveHundred)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.speed (0);
    stepper.step (10);

    adk::update (0);
    adk::update (999);
    CHECK (stepper.position () == 1);

    adk::update (1000);
    CHECK (stepper.position () == 2);

    stepper.speed (1000);
    adk::update (1001);
    CHECK (stepper.position () == 2);

    adk::update (1002);
    CHECK (stepper.position () == 3);
}

TEST (coilsAreReleasedOneIntervalAfterTheLastStep)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (2);

    adk::update (0);
    adk::update (2);
    CHECK (coils () == "0100");

    adk::update (3);
    CHECK (coils () == "0100");

    adk::update (4);
    CHECK (coils () == "0000");
    CHECK (stepper.position () == 2);
}

TEST (holdingKeepsTheCoilsPowered)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.hold (true);
    CHECK (coils () == "1000");

    stepper.step (1);
    adk::update (0);
    adk::update (100);
    CHECK (coils () == "1100");

    stepper.hold (false);
    CHECK (coils () == "0000");
}

TEST (aRestedMotorStartsAtOnceButNeverFasterThanItsSpeed)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (1);
    adk::update (0);
    CHECK (stepper.position () == 1);

    stepper.step (1);
    adk::update (1);
    CHECK (stepper.position () == 1);

    adk::update (2);
    CHECK (stepper.position () == 2);

    adk::update (500);
    stepper.step (1);
    adk::update (500);
    CHECK (stepper.position () == 3);
}

TEST (moveToGoesToAPositionEitherWay)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.moveTo (3);

    for (adk::Millis now = 0; now <= 20; now += 2)
    {
        adk::update (now);
    }

    CHECK (stepper.position () == 3);
    CHECK (!stepper.isMoving ());

    stepper.moveTo (-2);

    for (adk::Millis now = 22; now <= 40; now += 2)
    {
        adk::update (now);
    }

    CHECK (stepper.position () == -2);
    CHECK (coils () == "0000");
}

TEST (askingAgainForTheSamePositionKeepsThePace)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();

    for (adk::Millis now = 0; now <= 10; ++now)
    {
        stepper.moveTo (100);
        adk::update (now);
    }

    CHECK (stepper.position () == 6);
    CHECK (stepper.isMoving ());
}

TEST (aMoveAskedForJustAfterTheLastStepStillStartsWithOneStep)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (1);
    adk::update (0);
    CHECK (stepper.position () == 1);
    CHECK (!stepper.isMoving ());

    // Before any update has seen the motor at rest.
    stepper.step (3);
    adk::update (1000);
    CHECK (stepper.position () == 2);

    adk::update (1001);
    CHECK (stepper.position () == 2);

    adk::update (1002);
    CHECK (stepper.position () == 3);
}

TEST (stepCountsOnFromTheEndOfTheCurrentMove)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.step (3);
    adk::update (0);
    stepper.step (2);

    for (adk::Millis now = 2; now <= 20; now += 2)
    {
        adk::update (now);
    }

    CHECK (stepper.position () == 5);
}

TEST (stoppedStepperEndsItsMoveAndReleases)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.hold (true);
    stepper.step (100);
    adk::update (0);
    adk::update (2);

    adk::stop ();
    CHECK (coils () == "0000");
    CHECK (!stepper.isMoving ());
    CHECK (stepper.position () == 2);

    adk::update (100);
    CHECK (coils () == "0000");
    CHECK (stepper.position () == 2);
}

TEST (aRevolutionEndsOnThePhaseItStartedFrom)
{
    adk::Stepper stepper {22, 23, 24, 25};

    adk::setup ();
    stepper.hold (true);
    stepper.step (adk::Stepper::StepsPerRevolution);

    for (adk::Millis now = 0; now < 8192; ++now)
    {
        adk::update (now);
    }

    CHECK (stepper.position () == 4096);
    CHECK (!stepper.isMoving ());
    CHECK (coils () == "1000");
}
