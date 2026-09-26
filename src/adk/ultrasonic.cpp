#include "ultrasonic.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr Millis PingPeriod = 60;

        // Sound takes 58 us to reach something a centimeter away and come
        // back. An echo takes 25 ms from 4.3 m, beyond which it is too faint
        // to trust.
        constexpr unsigned long UsPerCm     = 58;
        constexpr unsigned long EchoTimeout = 25000;
    }

    Ultrasonic::Ultrasonic (Pin trigger, Pin echo)
        : pinged_   ()
        , distance_ (0)
        , trigger_  (trigger)
        , echo_     (echo)
        , measured_ (false)
    {
    }

    void Ultrasonic::setup ()
    {
        if (claimOutput (trigger_))
        {
            claimInput (echo_);
        }
    }

    uint16_t Ultrasonic::distance () const
    {
        return distance_;
    }

    bool Ultrasonic::measured () const
    {
        return measured_;
    }

    bool Ultrasonic::ok () const
    {
        return distance_ != 0;
    }

    void Ultrasonic::update (Millis now)
    {
        measured_ = pinged_.elapsed (now) >= PingPeriod;

        if (measured_)
        {
            pinged_.restart (now);
            ping ();
        }
    }

    void Ultrasonic::ping ()
    {
        // A 10 us pulse on Trig sends eight 40 kHz clicks. Echo then goes
        // high until they come back, or pulseIn () gives up and returns 0.
        digitalWrite      (trigger_, LOW);
        delayMicroseconds (2);
        digitalWrite      (trigger_, HIGH);
        delayMicroseconds (10);
        digitalWrite      (trigger_, LOW);

        unsigned long echo = pulseIn (echo_, HIGH, EchoTimeout);

        if (echo > EchoTimeout)
        {
            echo = 0;
        }

        distance_ = static_cast<uint16_t> ((echo + UsPerCm / 2) / UsPerCm);
    }
}
