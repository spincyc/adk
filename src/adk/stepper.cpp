#include "stepper.h"

#include <Arduino.h>

namespace adk {

    namespace {

        // Half-stepping: one coil, then it and the next together, then the
        // next alone, so the rotor moves half a step at a time. Each entry is
        // IN1-IN4 as four bits, IN1 first: 0x8 is 1000, 0xC is 1100.
        const uint8_t Phases [8] PROGMEM = {0x8, 0xC, 0x4, 0x6, 0x2, 0x3, 0x1, 0x9};

        // Each millisecond earns the motor its speed in thousandths of a step.
        const uint16_t StepCost = 1000;
        const uint16_t MaxSpeed = 1000;
    }

    const uint16_t Stepper::StepsPerRevolution;

    Stepper::Stepper (Pin in1, Pin in2, Pin in3, Pin in4)
        : position_  (0)
        , target_    (0)
        , then_      (0)
        , credit_    (StepCost)
        , speed_     (500)
        , pins_      {in1, in2, in3, in4}
        , holding_   (false)
        , energized_ (false)
        , starting_  (false)
    {
    }

    void Stepper::setup ()
    {
        energized_ = false;

        for (Pin pin : pins_)
        {
            claimOutput (pin);
        }
    }

    void Stepper::step (long steps)
    {
        moveTo (target_ + steps);
    }

    void Stepper::moveTo (long position)
    {
        if (!isMoving ())
        {
            starting_ = true;
        }

        target_ = position;
    }

    long Stepper::position () const
    {
        return position_;
    }

    bool Stepper::isMoving () const
    {
        return position_ != target_;
    }

    void Stepper::speed (uint16_t stepsPerSecond)
    {
        if (stepsPerSecond < 1)
        {
            stepsPerSecond = 1;
        }
        else if (stepsPerSecond > MaxSpeed)
        {
            stepsPerSecond = MaxSpeed;
        }

        speed_ = stepsPerSecond;
    }

    void Stepper::hold (bool on)
    {
        holding_ = on;

        if (isMoving ())
        {
            return;
        }

        if (on)
        {
            energize ();
        }
        else
        {
            release ();
        }
    }

    void Stepper::update (Millis now)
    {
        // Time arrives in whole milliseconds, so steps land on them: at speeds
        // that do not divide 1000 the gaps alternate, 3 and 4 ms at 300 steps
        // a second, and the average stays exact. After a step less than one
        // step of credit is left, so no two steps share a millisecond.
        //
        // A motor at rest, or just setting off, holds one step at most: a move
        // starts at once, and time spent waiting never turns into a burst.
        uint16_t most = (starting_ || !isMoving ()) ? StepCost : 2 * StepCost - 1;
        Millis   gap  = now - then_;
        uint32_t sum  = credit_ + (gap < 1000 ? gap : 1000) * speed_;

        credit_   = static_cast<uint16_t> (sum < most ? sum : most);
        then_     = now;
        starting_ = false;

        if (!isMoving ())
        {
            // A whole step in hand again means the last step has had its full
            // time, so the coils can be let go.
            if (credit_ == StepCost && energized_ && !holding_)
            {
                release ();
            }

            return;
        }

        if (credit_ < StepCost)
        {
            return;
        }

        credit_   -= StepCost;
        position_ += target_ > position_ ? 1 : -1;
        energize ();
    }

    void Stepper::stop ()
    {
        target_ = position_;
        release ();
    }

    void Stepper::energize ()
    {
        uint8_t coils = pgm_read_byte (&Phases[static_cast<uint8_t> (position_) & 7]);

        for (uint8_t coil = 0; coil < 4; ++coil)
        {
            digitalWrite (pins_[coil], (coils & (0x8 >> coil)) ? HIGH : LOW);
        }

        energized_ = true;
    }

    void Stepper::release ()
    {
        for (Pin pin : pins_)
        {
            digitalWrite (pin, LOW);
        }

        energized_ = false;
    }
}
