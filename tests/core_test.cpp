#include "check.h"

#include <Arduino.h>

namespace {

    // The built-in LED as blinkPin () flashes a pin number: L for a long
    // flash, S for a short one, then how long the LED stayed dark after the
    // last, in milliseconds.
    std::string blinked (adk::Pin pin)
    {
        std::string   seen;
        unsigned long changed = 0;

        arduino::onDigitalWrite = [&] (uint8_t written, uint8_t level)
        {
            if (written != LED_BUILTIN)
            {
                return;
            }

            unsigned long lit = (arduino::now () - changed) / 1000;

            if (level == LOW)
            {
                seen += lit == 800 ? "L" : lit == 200 ? "S" : "?";
            }

            changed = arduino::now ();
        };

        adk::blinkPin (pin);
        arduino::onDigitalWrite = nullptr;

        return seen + " " + std::to_string ((arduino::now () - changed) / 1000);
    }
}

TEST (setupConfiguresObjectsInDeclarationOrder)
{
    adk::DigitalOutput output {8};
    adk::DigitalInput  input  {22, true};

    adk::setup ();

    CHECK (arduino::pin (8).mode == OUTPUT);
    CHECK (arduino::pin (8).output == LOW);
    CHECK (arduino::pin (22).mode == INPUT_PULLUP);
    CHECK (!check::halted.happened);
}

TEST (aPinUsedTwiceHaltsWithThatPin)
{
    adk::DigitalOutput first  {8};
    adk::DigitalOutput second {8};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 8);
}

TEST (onlyTheFirstFaultIsKept)
{
    adk::DigitalOutput missing {90};
    adk::PwmOutput     notPwm  {22};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::NoSuchPin);
    CHECK (adk::faultPin () == 90);
}

TEST (setupWithALogExplainsTheFault)
{
    adk::PwmOutput dimmer {22};
    arduino::Log   log;

    adk::setup (log);

    CHECK (log.text == "adk: pin 22 cannot do PWM; use 2-13 or 44-46\r\n");
    CHECK (check::halted.fault == adk::Fault::NotPwm);
}

TEST (setupCanRunAgainAfterAFix)
{
    {
        adk::DigitalOutput first  {8};
        adk::DigitalOutput second {8};
        CHECK (!adk::start ());
    }

    adk::DigitalOutput only {8};
    CHECK (adk::start ());
}

TEST (analogClaimsAcceptChannelNumbersAndPins)
{
    adk::AnalogInput byChannel {0};
    adk::AnalogInput byPin     {A1};
    adk::AnalogInput digital   {22};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::NotAnalog);
    CHECK (adk::faultPin () == 22);
}

TEST (aChannelAndItsPinAreTheSameAnalogInput)
{
    adk::AnalogInput byChannel {0};
    adk::AnalogInput byPin     {A0};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::PinInUse);
    CHECK (adk::faultPin () == A0);
}

TEST (pwmOnATimerTakenBySoundIsRefused)
{
    adk::Speaker   speaker {30};
    adk::PwmOutput dimmer  {9};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::TimerInUse);
    CHECK (adk::faultPin () == 9);
}

TEST (soundOnATimerUsedForPwmIsRefused)
{
    adk::PwmOutput dimmer  {10};
    adk::Speaker   speaker {30};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::TimerInUse);
    CHECK (adk::faultPin () == 30);
}

TEST (pwmPinsSharingATimerAreFine)
{
    adk::PwmOutput first  {9};
    adk::PwmOutput second {10};

    CHECK (adk::start ());
}

TEST (interruptClaimsNeedAnInterruptPin)
{
    CHECK (adk::claimInterrupt (2));
    CHECK (adk::claimInterrupt (21));
    CHECK (!adk::claimInterrupt (22));
    CHECK (adk::fault () == adk::Fault::NotInterrupt);
}

TEST (sharedPinsCanBeClaimedAgainButNotTakenExclusively)
{
    CHECK (adk::claimShared (20));
    CHECK (adk::claimShared (20));
    CHECK (!adk::claimOutput (20));
    CHECK (adk::fault () == adk::Fault::PinInUse);
}

TEST (updatePassesOneTimestampToEveryObject)
{
    adk::Every tick {100};

    adk::setup ();

    adk::update (1000);
    CHECK (!tick.ticked ());

    adk::update (1099);
    CHECK (!tick.ticked ());

    adk::update (1100);
    CHECK (tick.ticked ());

    adk::update (1101);
    CHECK (!tick.ticked ());
}

TEST (everyKeepsItsBeatButSkipsMissedBeats)
{
    adk::Every tick {100};

    adk::update (0);
    adk::update (130);
    CHECK (tick.ticked ());

    adk::update (200);
    CHECK (tick.ticked ());

    adk::update (650);
    CHECK (tick.ticked ());

    adk::update (700);
    CHECK (!tick.ticked ());

    adk::update (750);
    CHECK (tick.ticked ());
}

TEST (updateWithoutATimeUsesMillis)
{
    adk::Every tick {100};

    adk::update ();
    arduino::advance (100);
    adk::update ();

    CHECK (tick.ticked ());
}

TEST (waitKeepsObjectsUpdating)
{
    adk::Led led {13};

    adk::setup ();
    led.blink (200);

    adk::wait (100);
    CHECK (!led.isOn ());
    CHECK (arduino::now () >= 100000);

    adk::wait (100);
    CHECK (led.isOn ());
}

TEST (stopPutsEveryObjectInItsSafeState)
{
    adk::Led       led    {13};
    adk::PwmOutput dimmer {5};

    adk::setup ();
    led.on ();
    dimmer.write (200);

    adk::stop ();

    CHECK (!led.isOn ());
    CHECK (arduino::pin (13).output == LOW);
    CHECK (arduino::pin (5).pwm == 0);
}

TEST (objectsLeaveTheRegistryWhenDestroyed)
{
    {
        adk::DigitalOutput temporary {8};
    }

    adk::DigitalOutput lasting {8};
    CHECK (adk::start ());
}

TEST (digitalOutputSetsItsLevelBeforeDriving)
{
    adk::DigitalOutput output {8};
    std::string        order;

    arduino::pin (8).output = HIGH;
    arduino::onDigitalWrite = [&] (uint8_t, uint8_t)
    {
        order += arduino::pin (8).mode == OUTPUT ? "late" : "early";
    };

    adk::setup ();

    CHECK (order == "early");
    CHECK (arduino::pin (8).output == LOW);
}

TEST (digitalOutputWritesAndToggles)
{
    adk::DigitalOutput output {8};

    adk::setup ();
    output.write (true);
    CHECK (arduino::pin (8).output == HIGH);

    output.toggle ();
    CHECK (arduino::pin (8).output == LOW);
    CHECK (!output.isHigh ());
}

TEST (digitalInputReadsTheLevel)
{
    adk::DigitalInput input {22};

    adk::setup ();

    arduino::pin (22).input = LOW;
    CHECK (!input.read ());

    arduino::pin (22).input = HIGH;
    CHECK (input.read ());
}

TEST (analogInputScalesLikeMap)
{
    adk::AnalogInput knob {A0};

    adk::setup ();

    arduino::pin (A0).analog = 0;
    CHECK (knob.read (0, 255) == 0);

    arduino::pin (A0).analog = 1023;
    CHECK (knob.read (0, 255) == 255);
    CHECK (knob.read (100, -100) == -100);

    arduino::pin (A0).analog = 512;
    CHECK (knob.read () == 512);
}

TEST (smootherStartsAtTheFirstSampleThenFollowsSlowly)
{
    adk::Smoother smooth {2};

    CHECK (smooth.add (400) == 400);
    CHECK (smooth.add (800) == 500);
    CHECK (smooth.add (800) == 575);
}

TEST (debouncerWaitsForAQuietWindow)
{
    adk::Debouncer debouncer;

    CHECK (!debouncer.sample (true, 0, 20));
    CHECK (!debouncer.sample (false, 5, 20));
    CHECK (!debouncer.sample (true, 8, 20));
    CHECK (!debouncer.sample (true, 27, 20));
    CHECK (debouncer.sample (true, 28, 20));
    CHECK (debouncer.stable ());
    CHECK (!debouncer.sample (true, 29, 20));
}

TEST (debouncerSettlesAcrossTheWrapOfMillis)
{
    adk::Debouncer debouncer;

    CHECK (!debouncer.sample (true, 0xFFFFFFF0, 20));
    CHECK (!debouncer.sample (true, 0x03, 20));
    CHECK (debouncer.sample (true, 0x04, 20));
}

TEST (devicesOfOneKindCanShareATimer)
{
    CHECK (adk::claimTimer (5, 44, 7));
    CHECK (adk::claimTimer (5, 45, 7));
    CHECK (!adk::claimPwm (46));
    CHECK (!adk::claimTimer (5, 30, 8));
    CHECK (!adk::claimTimer (5, 31));
    CHECK (!adk::claimTimer (0, 32, 7));
}

TEST (devicesCanRefuseAPinThemselves)
{
    CHECK (!adk::refuse (adk::Fault::NotServo, 30));
    CHECK (adk::fault () == adk::Fault::NotServo);

    arduino::Log log;
    adk::explain (log, adk::fault (), adk::faultPin ());
    CHECK (log.text == "adk: pin 30 cannot drive a servo; use 44, 45 or 46\r\n");
}

TEST (timerOfNamesThePwmTimer)
{
    CHECK (adk::timerOf (9) == 2);
    CHECK (adk::timerOf (44) == 5);
    CHECK (adk::timerOf (13) == 0);
    CHECK (adk::timerOf (22) == 0xFF);
}

TEST (timerOfAPinPastTheLastIsNone)
{
    CHECK (adk::timerOf (NUM_DIGITAL_PINS) == 0xFF);
    CHECK (adk::timerOf (200) == 0xFF);
}

TEST (aTimerFaultNamesOnlyThePartsThatTakeThatTimer)
{
    arduino::Log speaker;
    adk::explain (speaker, adk::Fault::TimerInUse, 9);
    CHECK (speaker.text == "adk: pin 9 needs a timer that is already in use\r\n"
                           "adk: a Speaker stops PWM on pins 9 and 10\r\n");

    arduino::Log servo;
    adk::explain (servo, adk::Fault::TimerInUse, 45);
    CHECK (servo.text.find ("Servo") != std::string::npos);
    CHECK (servo.text.find ("Speaker") == std::string::npos);
    CHECK (servo.text.find ("radio") == std::string::npos);

    arduino::Log radio;
    adk::explain (radio, adk::Fault::TimerInUse, 11);
    CHECK (radio.text.find ("radio") != std::string::npos);
    CHECK (radio.text.find ("Speaker") == std::string::npos);
    CHECK (radio.text.find ("Servo") == std::string::npos);

    arduino::Log untaken;
    adk::explain (untaken, adk::Fault::TimerInUse, 3);
    CHECK (untaken.text == "adk: pin 3 needs a timer that is already in use\r\n");
}

TEST (aTimerFaultOnAPinWithoutATimerNamesEveryPartThatTakesOne)
{
    arduino::Log log;
    adk::explain (log, adk::Fault::TimerInUse, 30);

    CHECK (log.text.find ("Speaker") != std::string::npos);
    CHECK (log.text.find ("Servo") != std::string::npos);
    CHECK (log.text.find ("radio") != std::string::npos);
}

TEST (aHaltedBoardBlinksTensLongAndOnesShort)
{
    CHECK (blinked (22) == "LLSS 2300");
    CHECK (blinked (9) == "SSSSSSSSS 2300");
    CHECK (blinked (10) == "L 2300");
    CHECK (blinked (69) == "LLLLLLSSSSSSSSS 2300");
}

TEST (aHaltedBoardBlinksPinZeroAsTenShortFlashes)
{
    CHECK (blinked (0) == "SSSSSSSSSS 2300");
    CHECK (arduino::pin (LED_BUILTIN).output == LOW);
}

TEST (colorsAreEqualWhenEveryChannelIs)
{
    constexpr adk::Color warm {255, 64, 0};

    static_assert (warm == adk::color::orange);
    CHECK (warm == adk::color::orange);
    CHECK (warm != adk::color::red);
    CHECK ((adk::Color {255, 64, 1}) != warm);
}

TEST (partsCanBeDeclaredInArrays)
{
    adk::Led    lamps   [] {{26}, {27}, {28}};
    adk::Button buttons [] {{22}, {23}};

    CHECK (adk::start ());
    CHECK (arduino::pin (28).mode == OUTPUT);
    CHECK (arduino::pin (23).mode == INPUT_PULLUP);

    lamps[2].on ();
    CHECK (arduino::pin (28).output == HIGH);
    CHECK (!buttons[1].isPressed ());
}
