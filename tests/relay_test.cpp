#include "check.h"

#include <adk/relay.h>
#include <Arduino.h>

TEST (relayStartsOffAndSwitches)
{
    adk::Relay relay {30};

    adk::setup ();
    CHECK (arduino::pin (30).mode == OUTPUT);
    CHECK (arduino::pin (30).output == LOW);
    CHECK (!relay.isOn ());

    relay.on ();
    CHECK (relay.isOn ());
    CHECK (arduino::pin (30).output == HIGH);

    relay.toggle ();
    CHECK (!relay.isOn ());
    CHECK (arduino::pin (30).output == LOW);

    relay.toggle ();
    relay.off ();
    CHECK (arduino::pin (30).output == LOW);
}

TEST (activeLowRelayIsOnWhenItsPinIsLow)
{
    adk::Relay relay {30, adk::ActiveLow};

    adk::setup ();
    CHECK (arduino::pin (30).output == HIGH);

    relay.on ();
    CHECK (arduino::pin (30).output == LOW);
    CHECK (relay.isOn ());
}

TEST (relayPinCannotBeUsedTwice)
{
    adk::Relay relay {30};
    adk::Led   led   {30};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 30);
}

TEST (relayPinMustExist)
{
    adk::Relay relay {90};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NoSuchPin);
    CHECK (check::halted.pin == 90);
}

TEST (stoppedRelayIsOff)
{
    adk::Relay high {30};
    adk::Relay low  {31, adk::ActiveLow};

    adk::setup ();
    high.on ();
    low.on ();

    adk::stop ();

    CHECK (!high.isOn ());
    CHECK (!low.isOn ());
    CHECK (arduino::pin (30).output == LOW);
    CHECK (arduino::pin (31).output == HIGH);
}
