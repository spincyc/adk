#include "check.h"
#include "fake_si4703.h"

#include <Arduino.h>

namespace {

    struct SetupProbe : adk::Object
    {
        explicit SetupProbe (adk::Pin pin)
            : pin_ (pin)
        {
        }

        void setup () override
        {
            ++setups;
            adk::claimOutput (pin_, true);
        }

        void stop () override
        {
            ++stops;
            digitalWrite (pin_, LOW);
        }

        adk::Pin pin_;
        int      setups = 0;
        int      stops  = 0;
    };

    constexpr adk::Pin  Sdio   = 40;
    constexpr adk::Pin  Sclk   = 41;
    constexpr adk::Pin  Reset  = 42;
    constexpr uint16_t Unmute = 1u << 14;
    constexpr uint16_t Enable = 1u << 0;
}

TEST (lifecycleStopsSetupAtTheFirstFaultAndOnlyStopsEarlierObjects)
{
    SetupProbe first  {26};
    SetupProbe failed {26};
    SetupProbe later  {27};

    CHECK (!adk::start ());
    CHECK (adk::fault () == adk::Fault::PinInUse);
    CHECK (adk::faultPin () == 26);
    CHECK (first.setups == 1);
    CHECK (first.stops == 1);
    CHECK (failed.setups == 1);
    CHECK (failed.stops == 0);
    CHECK (later.setups == 0);
    CHECK (later.stops == 0);
    CHECK (arduino::pin (26).output == LOW);
    CHECK (!adk::isClaimed (27));
}

TEST (lifecycleNeverStopsAFailedObjectWithAnInvalidPin)
{
    SetupProbe first  {26};
    SetupProbe failed {90};
    SetupProbe later  {27};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::NoSuchPin);
    CHECK (check::halted.pin == 90);
    CHECK (first.stops == 1);
    CHECK (failed.stops == 0);
    CHECK (later.setups == 0);
    CHECK (later.stops == 0);
    CHECK (arduino::pin (26).output == LOW);
}

TEST (lifecycleSuccessfulSetupKeepsItsOutputsRunning)
{
    SetupProbe first  {26};
    SetupProbe second {27};

    CHECK (adk::start ());
    CHECK (first.stops == 0);
    CHECK (second.stops == 0);
    CHECK (arduino::pin (26).output == HIGH);
    CHECK (arduino::pin (27).output == HIGH);
}

TEST (lifecycleFaultBeforeTheFmRadioLeavesItUnconfigured)
{
    fake::Si4703 chip   {Sdio, Sclk, Reset};
    adk::Led     first  {26};
    adk::Led     failed {26};
    adk::FmRadio radio  {Sdio, Sclk, Reset};

    CHECK (!adk::start ());
    CHECK (!radio.ok ());
    CHECK (!chip.twoWire);
    CHECK (!(chip.registers[0x02] & Enable));
    CHECK (!adk::isClaimed (Sdio));
    CHECK (!adk::isClaimed (Sclk));
    CHECK (!adk::isClaimed (Reset));
    CHECK (!chip.drivenHigh);
}

TEST (lifecycleFaultAfterTheFmRadioMutesItBeforeHalting)
{
    fake::Si4703 chip   {Sdio, Sclk, Reset};
    adk::FmRadio radio  {Sdio, Sclk, Reset};
    adk::Led     failed {90};
    arduino::Log log;

    adk::setup (log);

    CHECK (radio.ok ());
    CHECK (chip.twoWire);
    CHECK (chip.registers[0x02] & Enable);
    CHECK (!(chip.registers[0x02] & Unmute));
    CHECK (!chip.drivenHigh);
    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::NoSuchPin);
    CHECK (check::halted.pin == 90);
    CHECK (log.text == "adk: pin 90 does not exist on this board\r\n");
}

TEST (lifecycleInvalidFmPinsDoNotTouchUnclaimedPins)
{
    for (adk::Pin invalid : {Sdio, Sclk, Reset})
    {
        int writes = 0;
        arduino::onDigitalWrite = [&] (uint8_t, uint8_t) { ++writes; };

        adk::FmRadio radio {adk::Pin (invalid == Sdio ? 90 : Sdio),
                            adk::Pin (invalid == Sclk ? 90 : Sclk),
                            adk::Pin (invalid == Reset ? 90 : Reset)};

        CHECK (!adk::start ());
        CHECK (adk::fault () == adk::Fault::NoSuchPin);
        CHECK (adk::faultPin () == 90);
        CHECK (!radio.ok ());
        CHECK (writes == 0);
        arduino::onDigitalWrite = nullptr;
    }
}
