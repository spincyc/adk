#pragma once

#include "object.h"

namespace adk {

    // An SG90 hobby servo. Its horn turns to an angle from 0 to 180 degrees,
    // set by the width of a pulse sent every 20 ms. Timer 5 makes the pulses in
    // hardware, so they never jitter, and it can only do that on pins 44, 45
    // and 46. Up to three servos share Timer 5, and while any servo uses it,
    // none of those pins can do PWM.
    //
    //   brown  -> GND
    //   red    -> 5 V from a separate supply, such as a breadboard power
    //             module, with its GND joined to the Mega's GND
    //   orange -> pin 44, 45 or 46
    //
    // A moving servo draws bursts of several hundred milliamps. Never power it
    // from the Mega's 5 V pin: the dip can reset the board.
    //
    // A servo gets no pulses until its first write () or moveTo (), so it does
    // not jump at power-up; until then angle () reports 90. adk::stop () ends
    // the pulses and the servo goes limp; the next write () starts them again.
    struct Servo : Object
    {
        // Most SG90s turn through about 180 degrees for pulses from 544 to
        // 2400 microseconds. If one buzzes at either end, it is pushing against
        // its stop: bring minMicros up or maxMicros down. minMicros must stay
        // below maxMicros.
        explicit Servo (Pin pin, uint16_t minMicros = 544, uint16_t maxMicros = 2400);

        // Turn at once, as fast as the servo can: to an angle from 0 to 180,
        // or to a pulse width kept between minMicros and maxMicros.
        void write             (uint8_t degrees);
        void writeMicroseconds (uint16_t micros);

        // Glide to an angle at a steady pace, arriving after duration, while
        // the sketch carries on. Asking again for the angle it is already
        // gliding to changes nothing, so moveTo () can be called from every
        // pass of loop (). A limp servo goes straight there instead, because
        // it may have been pushed anywhere.
        void moveTo (uint8_t degrees, Millis duration);

        // Whether a glide is under way, and the angle being sent now: during
        // a glide, how far it has got.
        bool    isMoving () const;
        uint8_t angle    () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        uint16_t pulseFor (uint8_t degrees) const;
        void     pulse    (uint16_t micros);

        Millis   moveStart_;
        Millis   moveLength_;
        uint16_t minMicros_;
        uint16_t maxMicros_;
        uint16_t micros_;
        uint16_t from_;
        uint16_t to_;
        Pin      pin_;
        bool     pulsing_;
        bool     starting_;
    };
}
