#include "joystick.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr int8_t DeadZone = 10;
        constexpr int8_t Enter    = 50;
        constexpr int8_t Leave    = 30;

        // Each side of the center is scaled on its own, so both ends of the
        // travel reach 100 however far the center is from 512.
        int8_t percent (uint16_t reading, uint16_t center)
        {
            long offset = static_cast<long> (reading) - center;

            // Also keeps a center of 0 or 1023, with no travel beyond it on
            // one side, from dividing by zero.
            if (offset == 0)
            {
                return 0;
            }

            long span  = (offset > 0) ? 1023L - center : static_cast<long> (center);
            long value = offset * 100 / span;

            return (value > -DeadZone && value < DeadZone) ? 0 : static_cast<int8_t> (value);
        }
    }

    Joystick::Joystick (Pin x, Pin y)
        : xCenter_   (512)
        , yCenter_   (512)
        , xPin_      (x)
        , yPin_      (y)
        , x_         (0)
        , y_         (0)
        , direction_ (Center)
        , moved_     (Center)
    {
    }

    void Joystick::setup ()
    {
        if (claimAnalog (xPin_) && claimAnalog (yPin_))
        {
            xCenter_ = static_cast<uint16_t> (analogRead (xPin_));
            yCenter_ = static_cast<uint16_t> (analogRead (yPin_));
        }
    }

    void Joystick::update (Millis)
    {
        x_ = percent (static_cast<uint16_t> (analogRead (xPin_)), xCenter_);
        y_ = percent (static_cast<uint16_t> (analogRead (yPin_)), yCenter_);

        Direction next = (reach (direction_) >= Leave) ? direction_ : strongest ();

        moved_     = (next != direction_) ? next : Center;
        direction_ = next;
    }

    int8_t Joystick::x () const
    {
        return x_;
    }

    int8_t Joystick::y () const
    {
        return y_;
    }

    Joystick::Direction Joystick::direction () const
    {
        return direction_;
    }

    Joystick::Direction Joystick::moved () const
    {
        return moved_;
    }

    // How far the stick is pushed towards a direction, negative if away.
    int8_t Joystick::reach (Direction direction) const
    {
        switch (direction)
        {
            case Up:    return y_;
            case Down:  return static_cast<int8_t> (-y_);
            case Left:  return static_cast<int8_t> (-x_);
            case Right: return x_;
            default:    return 0;
        }
    }

    // A tie between the axes goes to up or down.
    Joystick::Direction Joystick::strongest () const
    {
        Direction across = (x_ > 0) ? Right : Left;
        Direction along  = (y_ > 0) ? Up : Down;
        Direction most   = (reach (across) > reach (along)) ? across : along;

        return (reach (most) > Enter) ? most : Center;
    }
}
