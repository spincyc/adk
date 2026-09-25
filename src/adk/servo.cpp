#include "servo.h"

#include <Arduino.h>

namespace adk {

    namespace {

        // Servos share Timer 5 with each other and with nothing else.
        const uint8_t  ServoTimer = 5;
        const uint8_t  ServoUser  = 1;
        const uint16_t Period     = 39999;

        // Pins 44, 45 and 46 are Timer 5's outputs C, B and A.
        volatile uint16_t& compare (Pin pin)
        {
            switch (pin)
            {
                case 44: return OCR5C;
                case 45: return OCR5B;
                default: return OCR5A;
            }
        }

        uint8_t output (Pin pin)
        {
            switch (pin)
            {
                case 44: return _BV (COM5C1);
                case 45: return _BV (COM5B1);
                default: return _BV (COM5A1);
            }
        }
    }

    Servo::Servo (Pin pin, uint16_t minMicros, uint16_t maxMicros)
        : moveStart_  (0)
        , moveLength_ (0)
        , minMicros_  (minMicros)
        , maxMicros_  (maxMicros)
        , micros_     (static_cast<uint16_t> ((minMicros + maxMicros) / 2))
        , from_       (0)
        , to_         (0)
        , pin_        (pin)
        , pulsing_    (false)
        , starting_   (false)
    {
    }

    void Servo::setup ()
    {
        pulsing_    = false;
        moveLength_ = 0;

        if (timerOf (pin_) != ServoTimer)
        {
            refuse (Fault::NotServo, pin_);
            return;
        }

        if (!claimOutput (pin_) || !claimTimer (ServoTimer, pin_, ServoUser))
        {
            return;
        }

        // Fast PWM up to ICR5 (mode 14) with the clock divided by 8: the timer
        // counts half microseconds and starts a pulse every 20 ms. Every
        // output stays disconnected until its servo's first write ().
        TCCR5A = _BV (WGM51);
        TCCR5B = _BV (WGM53) | _BV (WGM52) | _BV (CS51);
        ICR5   = Period;
    }

    void Servo::write (uint8_t degrees)
    {
        writeMicroseconds (pulseFor (degrees));
    }

    void Servo::writeMicroseconds (uint16_t micros)
    {
        if (micros < minMicros_)
        {
            micros = minMicros_;
        }
        else if (micros > maxMicros_)
        {
            micros = maxMicros_;
        }

        moveLength_ = 0;
        pulse (micros);
    }

    void Servo::moveTo (uint8_t degrees, Millis duration)
    {
        uint16_t target = pulseFor (degrees);

        if (isMoving () && target == to_)
        {
            return;
        }

        if (!pulsing_ || duration == 0 || target == micros_)
        {
            writeMicroseconds (target);
            return;
        }

        from_       = micros_;
        to_         = target;
        moveLength_ = duration;
        starting_   = true;
    }

    bool Servo::isMoving () const
    {
        return moveLength_ != 0;
    }

    uint8_t Servo::angle () const
    {
        uint16_t range = static_cast<uint16_t> (maxMicros_ - minMicros_);

        if (range == 0)
        {
            return 0;
        }

        uint32_t above = static_cast<uint16_t> (micros_ - minMicros_);
        return static_cast<uint8_t> ((above * 180 + range / 2) / range);
    }

    void Servo::update (Millis now)
    {
        if (moveLength_ == 0)
        {
            return;
        }

        if (starting_)
        {
            moveStart_ = now;
            starting_  = false;
        }

        Millis elapsed = now - moveStart_;

        if (elapsed >= moveLength_)
        {
            moveLength_ = 0;
            pulse (to_);
            return;
        }

        // Glides longer than about a minute are measured in coarser steps, so
        // that distance times elapsed still fits in 32 bits.
        Millis   length   = moveLength_;
        uint16_t distance = static_cast<uint16_t> (to_ > from_ ? to_ - from_ : from_ - to_);

        while (length > 0xFFFF)
        {
            length  >>= 1;
            elapsed >>= 1;
        }

        uint16_t moved = static_cast<uint16_t> (distance * elapsed / length);
        pulse (static_cast<uint16_t> (to_ > from_ ? from_ + moved : from_ - moved));
    }

    void Servo::stop ()
    {
        moveLength_ = 0;

        // Disconnected from the timer, the pin sits at the low level its claim set.
        if (pulsing_)
        {
            TCCR5A   = static_cast<uint8_t> (TCCR5A & ~output (pin_));
            pulsing_ = false;
        }
    }

    uint16_t Servo::pulseFor (uint8_t degrees) const
    {
        uint32_t range = static_cast<uint16_t> (maxMicros_ - minMicros_);

        if (degrees > 180)
        {
            degrees = 180;
        }

        return static_cast<uint16_t> (minMicros_ + (degrees * range + 90) / 180);
    }

    void Servo::pulse (uint16_t micros)
    {
        // The timer takes a new compare value at the end of a period, so a
        // pulse is never cut short, and setting it before connecting the
        // output keeps the first pulse from having an old width.
        compare (pin_) = static_cast<uint16_t> (micros * 2);

        if (!pulsing_)
        {
            TCCR5A   = static_cast<uint8_t> (TCCR5A | output (pin_));
            pulsing_ = true;
        }

        micros_ = micros;
    }
}
