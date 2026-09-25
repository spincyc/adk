#pragma once

#include <stdint.h>

class Print;

namespace adk {

    using Pin = uint8_t;

    enum struct Fault : uint8_t
    {
        None,
        NoSuchPin,
        PinInUse,
        NotPwm,
        NotAnalog,
        NotInterrupt,
        TimerInUse
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

    Fault fault    ();
    Pin   faultPin ();

    // Forget every claim and fault. adk::setup () starts with this.
    void releaseClaims ();

    // Describe a fault in one line, such as "adk: pin 9 is used twice".
    void explain (Print& log, Fault fault, Pin pin);

    // adk::setup () calls this when a claim failed. It releases every claimed
    // pin and blinks the fault's pin number on the built-in LED forever: long
    // flashes for tens, short flashes for ones.
    void halt (Fault fault, Pin pin);
}
