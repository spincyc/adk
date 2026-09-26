#include "motor.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr Millis SpinDown = 500;
    }

    Motor::Motor (Pin enable, Pin forward, Pin backward)
        : spinDown_  ()
        , speed_     (0)
        , enable_    (enable)
        , forward_   (forward)
        , backward_  (backward)
        , spin_      (0)
        , driving_   (false)
    {
    }

    void Motor::setup ()
    {
        claimPwm    (enable_);
        claimOutput (forward_);
        claimOutput (backward_);
    }

    void Motor::speed (int16_t speed)
    {
        if (speed > 255)
        {
            speed = 255;
        }
        else if (speed < -255)
        {
            speed = -255;
        }

        speed_ = speed;

        if (speed_ == 0)
        {
            coast ();
            return;
        }

        // Still turning the other way: stop driving it, and update () drives
        // the new way once the motor has had time to spin down.
        if (spin_ == (speed_ > 0 ? -1 : 1))
        {
            if (driving_)
            {
                stopDriving (false);
            }

            return;
        }

        drive ();
    }

    int16_t Motor::speed () const
    {
        return speed_;
    }

    void Motor::brake ()
    {
        speed_ = 0;
        stopDriving (true);
    }

    void Motor::coast ()
    {
        speed_ = 0;
        stopDriving (false);
    }

    void Motor::update (Millis now)
    {
        if (driving_ || spin_ == 0 || spinDown_.elapsed (now) < SpinDown)
        {
            return;
        }

        spin_ = 0;

        if (speed_ != 0)
        {
            drive ();
        }
    }

    void Motor::stop ()
    {
        coast ();
    }

    void Motor::drive ()
    {
        digitalWrite (forward_,  speed_ > 0 ? HIGH : LOW);
        digitalWrite (backward_, speed_ < 0 ? HIGH : LOW);
        analogWrite  (enable_,   speed_ > 0 ? speed_ : -speed_);

        spin_    = speed_ > 0 ? 1 : -1;
        driving_ = true;
    }

    void Motor::stopDriving (bool braking)
    {
        digitalWrite (forward_,  LOW);
        digitalWrite (backward_, LOW);
        analogWrite  (enable_,   braking ? 255 : 0);

        // The spin-down is timed from when the drive stopped, so a later
        // brake () or coast () does not restart it.
        if (driving_)
        {
            driving_ = false;
            spinDown_.restart ();
        }
    }
}
