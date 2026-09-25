#include "board.h"

#include <Arduino.h>

namespace adk {

    namespace {

        const uint8_t PinBytes   = (NUM_DIGITAL_PINS + 7) / 8;
        const uint8_t TimerCount = 6;
        const uint8_t NoTimer    = 0xFF;
        const uint8_t TakenOver  = 0x80;

        // One bit per pin, a second bit for pins a bus shares, and per timer
        // either the number of PWM pins using it or TakenOver plus the user.
        uint8_t claimed [PinBytes];
        uint8_t shared  [PinBytes];
        uint8_t timers  [TimerCount];
        Fault   firstFault = Fault::None;
        Pin     firstPin   = 0;

        bool fail (Fault fault, Pin pin)
        {
            if (firstFault == Fault::None)
            {
                firstFault = fault;
                firstPin   = pin;
            }

            return false;
        }

        bool test (const uint8_t* bits, Pin pin)
        {
            return bits[pin / 8] & (1 << (pin % 8));
        }

        void mark (uint8_t* bits, Pin pin)
        {
            bits[pin / 8] |= static_cast<uint8_t> (1 << (pin % 8));
        }

        bool claim (Pin pin)
        {
            if (pin >= NUM_DIGITAL_PINS)
            {
                return fail (Fault::NoSuchPin, pin);
            }

            if (test (claimed, pin))
            {
                return fail (Fault::PinInUse, pin);
            }

            mark (claimed, pin);
            return true;
        }

        void flash (unsigned long on)
        {
            digitalWrite (LED_BUILTIN, HIGH);
            delay        (on);
            digitalWrite (LED_BUILTIN, LOW);
            delay        (300);
        }
    }

    uint8_t timerOf (Pin pin)
    {
        switch (digitalPinToTimer (pin))
        {
            case TIMER0A: case TIMER0B:                             return 0;
            case TIMER1A: case TIMER1B: case TIMER1C:               return 1;
            case TIMER2:  case TIMER2A: case TIMER2B:               return 2;
            case TIMER3A: case TIMER3B: case TIMER3C:               return 3;
            case TIMER4A: case TIMER4B: case TIMER4C: case TIMER4D: return 4;
            case TIMER5A: case TIMER5B: case TIMER5C:               return 5;
            default:                                                return NoTimer;
        }
    }

    bool claimOutput (Pin pin, bool high)
    {
        if (!claim (pin))
        {
            return false;
        }

        // Set the level first so the pin never glitches the other way.
        digitalWrite (pin, high ? HIGH : LOW);
        pinMode      (pin, OUTPUT);
        return true;
    }

    bool claimInput (Pin pin, bool pullUp)
    {
        if (!claim (pin))
        {
            return false;
        }

        pinMode (pin, pullUp ? INPUT_PULLUP : INPUT);
        return true;
    }

    bool claimPwm (Pin pin, bool high)
    {
        if (pin >= NUM_DIGITAL_PINS)
        {
            return fail (Fault::NoSuchPin, pin);
        }

        uint8_t timer = timerOf (pin);

        if (timer == NoTimer)
        {
            return fail (Fault::NotPwm, pin);
        }

        if (timers[timer] & TakenOver)
        {
            return fail (Fault::TimerInUse, pin);
        }

        if (!claimOutput (pin, high))
        {
            return false;
        }

        ++timers[timer];
        return true;
    }

    bool claimAnalog (Pin pin)
    {
        // analogRead () accepts 0-15 as well as A0-A15; claim the real pin.
        if (pin < NUM_ANALOG_INPUTS)
        {
            pin = static_cast<Pin> (analogInputToDigitalPin (pin));
        }

        if (pin < A0 || pin >= A0 + NUM_ANALOG_INPUTS)
        {
            return fail (Fault::NotAnalog, pin);
        }

        return claimInput (pin);
    }

    bool claimInterrupt (Pin pin, bool pullUp)
    {
        if (pin < NUM_DIGITAL_PINS && digitalPinToInterrupt (pin) == NOT_AN_INTERRUPT)
        {
            return fail (Fault::NotInterrupt, pin);
        }

        return claimInput (pin, pullUp);
    }

    bool isClaimed (Pin pin)
    {
        return pin < NUM_DIGITAL_PINS && test (claimed, pin);
    }

    bool claimShared (Pin pin)
    {
        if (pin < NUM_DIGITAL_PINS && test (shared, pin))
        {
            return true;
        }

        if (!claim (pin))
        {
            return false;
        }

        mark (shared, pin);
        return true;
    }

    bool claimTimer (uint8_t timer, Pin pin, uint8_t user)
    {
        // Timer 0 keeps millis () running, so nothing may take it over.
        if (timer == 0 || timer >= TimerCount)
        {
            return fail (Fault::TimerInUse, pin);
        }

        uint8_t takenOver = static_cast<uint8_t> (TakenOver | user);

        if (timers[timer] == 0 || (user != 0 && timers[timer] == takenOver))
        {
            timers[timer] = takenOver;
            return true;
        }

        return fail (Fault::TimerInUse, pin);
    }

    bool refuse (Fault fault, Pin pin)
    {
        return fail (fault, pin);
    }

    Fault fault ()
    {
        return firstFault;
    }

    Pin faultPin ()
    {
        return firstPin;
    }

    void releaseClaims ()
    {
        memset (claimed, 0, sizeof claimed);
        memset (shared,  0, sizeof shared);
        memset (timers,  0, sizeof timers);

        firstFault = Fault::None;
        firstPin   = 0;
    }

    void explain (Print& log, Fault fault, Pin pin)
    {
        log.print (F ("adk: pin "));
        log.print (pin);

        switch (fault)
        {
            case Fault::None:
                log.println (F (" is fine"));
                break;
            case Fault::NoSuchPin:
                log.println (F (" does not exist on this board"));
                break;
            case Fault::PinInUse:
                log.println (F (" is used twice"));
                break;
            case Fault::NotPwm:
                log.println (F (" cannot do PWM; use 2-13 or 44-46"));
                break;
            case Fault::NotAnalog:
                log.println (F (" is not an analog input; use A0-A15"));
                break;
            case Fault::NotInterrupt:
                log.println (F (" cannot interrupt; use 2, 3, 18, 19, 20 or 21"));
                break;
            case Fault::NotServo:
                log.println (F (" cannot drive a servo; use 44, 45 or 46"));
                break;
            case Fault::TimerInUse:
                log.println (F (" needs a timer that is already in use"));

                if (timerOf (pin) != 5)
                {
                    log.println (F ("adk: a Speaker stops PWM on pins 9 and 10"));
                }

                if (timerOf (pin) != 2)
                {
                    log.println (F ("adk: a Servo stops PWM on pins 44, 45 and 46"));
                }

                if (timerOf (pin) != 2 && timerOf (pin) != 5)
                {
                    log.println (F ("adk: a 433 MHz radio stops PWM on pins 11 and 12"));
                }
                break;
            case Fault::NotSerial:
                log.println (F (" is not on a spare serial port; use Serial1, Serial2 or Serial3"));
                break;
        }
    }

    __attribute__ ((weak)) void halt (Fault, Pin pin)
    {
        for (Pin claimedPin = 0; claimedPin < NUM_DIGITAL_PINS; ++claimedPin)
        {
            if (test (claimed, claimedPin))
            {
                pinMode (claimedPin, INPUT);
            }
        }

        pinMode (LED_BUILTIN, OUTPUT);

        for (;;)
        {
            for (uint8_t tens = 0; tens < pin / 10; ++tens)
            {
                flash (800);
            }

            for (uint8_t ones = 0; ones < pin % 10; ++ones)
            {
                flash (200);
            }

            delay (2000);
        }
    }
}
