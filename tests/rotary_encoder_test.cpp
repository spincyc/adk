#include "check.h"

#include <Arduino.h>
#include <adk/rotary_encoder.h>

namespace {

    const char* const Clockwise        = "01 00 10 11";
    const char* const CounterClockwise = "10 00 01 11";

    // Move the contacts through readings such as "01 00", CLK then DT, and
    // update after each. A detent rests at "11".
    void contacts (const std::string& readings)
    {
        for (size_t at = 0; at + 1 < readings.size (); at += 3)
        {
            arduino::pin (30).input = (readings[at] == '1') ? HIGH : LOW;
            arduino::pin (31).input = (readings[at + 1] == '1') ? HIGH : LOW;
            adk::update (0);
        }
    }
}

TEST (encoderContactsArePulledUp)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (30).mode == INPUT_PULLUP);
    CHECK (arduino::pin (31).mode == INPUT_PULLUP);
}

TEST (encoderPinUsedTwiceHalts)
{
    adk::Button        button {31};
    adk::RotaryEncoder knob   {30, 31};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 31);
}

TEST (aClockwiseDetentCountsAtItsFourthStep)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();

    contacts ("01 00 10");
    CHECK (knob.position () == 0);
    CHECK (knob.turned () == 0);

    contacts ("11");
    CHECK (knob.position () == 1);
    CHECK (knob.turned () == 1);

    adk::update (1);
    CHECK (knob.position () == 1);
    CHECK (knob.turned () == 0);
}

TEST (aCounterClockwiseDetentCountsDown)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();
    contacts (CounterClockwise);

    CHECK (knob.position () == -1);
    CHECK (knob.turned () == -1);
}

TEST (detentsAddUpBothWays)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();
    contacts (Clockwise);
    contacts (Clockwise);
    contacts (Clockwise);
    contacts (CounterClockwise);

    CHECK (knob.position () == 2);
}

TEST (aBouncingContactCancelsItselfOut)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();

    contacts ("01 11 01 11 01");
    CHECK (knob.position () == 0);

    contacts ("00 10 11");
    CHECK (knob.position () == 1);
}

TEST (aMissedStepCountsNothing)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();

    contacts ("00 11");
    CHECK (knob.position () == 0);
    CHECK (knob.turned () == 0);

    contacts (Clockwise);
    CHECK (knob.position () == 1);
}

TEST (turningBackBeforeTheDetentCountsNothing)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();
    contacts ("01 00 10 00 01 11");

    CHECK (knob.position () == 0);
}

TEST (anEncoderWithTwoStepsPerDetent)
{
    adk::RotaryEncoder knob {30, 31, 2};

    adk::setup ();

    contacts ("01 00");
    CHECK (knob.turned () == 1);

    contacts ("10 11");
    CHECK (knob.position () == 2);
}

TEST (resetCountsOnFromANewPosition)
{
    adk::RotaryEncoder knob {30, 31};

    adk::setup ();
    contacts (Clockwise);

    knob.reset (10);
    CHECK (knob.position () == 10);

    contacts (Clockwise);
    CHECK (knob.position () == 11);

    knob.reset ();
    CHECK (knob.position () == 0);
}
