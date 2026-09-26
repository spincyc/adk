#pragma once

#include <stdint.h>

class Print;

namespace adk {

    using Pin = uint8_t;

    enum class Fault : uint8_t
    {
        None,
        NoSuchPin,
        PinInUse,
        NotPwm,
        NotAnalog,
        NotInterrupt,
        NotServo,
        TimerInUse,
        NotSerial,
        NotInfrared
    };

    // A device claims the pins and timers it uses from its setup (). Each claim
    // checks that the board can do what is asked and that nothing else already
    // uses the pin, then configures it. The first failed claim is remembered,
    // and adk::setup () halts before the sketch can run with it.
    //
    // A timer is taken over by one kind of device. Devices of one kind that
    // share a timer, such as several servos, claim it with the same non-zero
    // user number.

    bool claimOutput    (Pin pin, bool high = false);
    bool claimInput     (Pin pin, bool pullUp = false);
    bool claimPwm       (Pin pin, bool high = false);
    bool claimAnalog    (Pin pin);
    bool claimInterrupt (Pin pin, bool pullUp = false);
    bool claimShared    (Pin pin);
    bool claimTimer     (uint8_t timer, Pin pin, uint8_t user = 0);

    // Record a fault a device found itself, such as a servo on a pin its
    // timer cannot reach. Returns false, like a failed claim.
    bool refuse (Fault fault, Pin pin);

    // The hardware timer behind a pin's PWM, or 0xFF if it has none or is
    // not a pin at all.
    uint8_t timerOf (Pin pin);

    // Whether some part has claimed the pin, alone or as a shared bus pin.
    bool isClaimed (Pin pin);

    Fault fault    ();
    Pin   faultPin ();

    // Forget every claim and fault. adk::setup () starts with this.
    void releaseClaims ();

    // Describe a fault in one line, such as "adk: pin 9 is used twice".
    void explain (Print& log, Fault fault, Pin pin);

    // adk::setup () calls this when a claim failed. It makes every claimed
    // pin an input again, so nothing is left driven, and blinks the fault's
    // pin number on the built-in LED forever with blinkPin ().
    void halt (Fault fault, Pin pin);

    // Blink a pin number once on the built-in LED: a long flash for each
    // ten, a short flash for each one, then a pause. Pin 0 is ten short
    // flashes, so a halted board never looks like one that is just idle.
    void blinkPin (Pin pin);
}
