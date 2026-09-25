#include "check.h"

#include <Arduino.h>
#include <adk/ultrasonic.h>

namespace {

    // Plays the HC-SR04: every ping answers with an echo this many
    // microseconds long, and counts itself.
    unsigned long echo  = 0;
    int           pings = 0;

    void answerWithEcho (unsigned long us)
    {
        echo  = us;
        pings = 0;

        arduino::onPulseIn = [] (uint8_t, uint8_t, unsigned long)
        {
            ++pings;
            return echo;
        };
    }
}

TEST (ultrasonicClaimsATriggerOutputAndAnEchoInput)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (30).mode == OUTPUT);
    CHECK (arduino::pin (30).output == LOW);
    CHECK (arduino::pin (31).mode == INPUT);
}

TEST (ultrasonicOnOnePinHalts)
{
    adk::Ultrasonic ranger {30, 30};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 30);
}

TEST (ultrasonicEchoPinMustExist)
{
    adk::Ultrasonic ranger {30, 90};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::NoSuchPin);
    CHECK (adk::faultPin () == 90);
}

TEST (ultrasonicTriggersForTenMicrosecondsThenTimesTheEcho)
{
    adk::Ultrasonic ranger {30, 31};
    unsigned long   rose     = 0;
    unsigned long   fell     = 0;
    unsigned long   listened = 0;
    uint8_t         pin      = 0;
    uint8_t         level    = 0;
    unsigned long   timeout  = 0;

    adk::setup ();
    arduino::advanceMicros (1000);

    arduino::onDigitalWrite = [&] (uint8_t written, uint8_t value)
    {
        if (written == 30)
        {
            (value == HIGH ? rose : fell) = arduino::now ();
        }
    };

    arduino::onPulseIn = [&] (uint8_t echoPin, uint8_t state, unsigned long limit)
    {
        listened = arduino::now ();
        pin      = echoPin;
        level    = state;
        timeout  = limit;
        return 1166UL;
    };

    adk::update (0);

    CHECK (rose == 1002);
    CHECK (fell == 1012);
    CHECK (listened == fell);
    CHECK (pin == 31);
    CHECK (level == HIGH);
    CHECK (timeout == 25000);
    CHECK (arduino::pin (30).output == LOW);
}

TEST (ultrasonicMeasuresOnTheFirstUpdateForThatUpdateOnly)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();
    answerWithEcho (1166);
    CHECK (!ranger.measured ());

    adk::update (5000);
    CHECK (ranger.measured ());
    CHECK (ranger.hasEcho ());
    CHECK (ranger.distance () == 20);

    adk::update (5001);
    CHECK (!ranger.measured ());
    CHECK (ranger.distance () == 20);
    CHECK (pings == 1);
}

TEST (ultrasonicPingsNoMoreOftenThanEverySixtyMilliseconds)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();
    answerWithEcho (580);

    adk::update (0);
    adk::update (59);
    CHECK (!ranger.measured ());
    CHECK (pings == 1);

    adk::update (60);
    CHECK (ranger.measured ());
    CHECK (pings == 2);

    // A late update pings at once, and the next ping waits a full period.
    adk::update (130);
    CHECK (ranger.measured ());
    adk::update (189);
    CHECK (!ranger.measured ());
    adk::update (190);
    CHECK (ranger.measured ());
    CHECK (pings == 4);
}

TEST (ultrasonicRoundsToTheNearestCentimetre)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();
    answerWithEcho (86);
    adk::update (0);
    CHECK (ranger.distance () == 1);

    echo = 87;
    adk::update (60);
    CHECK (ranger.distance () == 2);

    echo = 25000;
    adk::update (120);
    CHECK (ranger.distance () == 431);
}

TEST (ultrasonicWithoutAnEchoReportsNoDistance)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();
    answerWithEcho (1166);
    adk::update (0);
    CHECK (ranger.hasEcho ());

    echo = 0;
    adk::update (60);
    CHECK (ranger.measured ());
    CHECK (!ranger.hasEcho ());
    CHECK (ranger.distance () == 0);
}

TEST (ultrasonicIgnoresAnEchoLongerThanItsRange)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();
    answerWithEcho (25001);
    adk::update (0);

    CHECK (ranger.measured ());
    CHECK (!ranger.hasEcho ());
    CHECK (ranger.distance () == 0);
}

TEST (ultrasonicTooCloseToMeasureHasNoEcho)
{
    adk::Ultrasonic ranger {30, 31};

    adk::setup ();
    answerWithEcho (28);
    adk::update (0);

    CHECK (!ranger.hasEcho ());
    CHECK (ranger.distance () == 0);
}
