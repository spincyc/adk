#include "check.h"

#include <Arduino.h>
#include <adk/shift_register.h>

namespace {

    uint8_t lastShifted ()
    {
        return static_cast<uint8_t> (arduino::shifted.back ());
    }
}

TEST (shiftRegisterClaimsThreeOutputsAndClearsThem)
{
    adk::ShiftRegister outputs {30, 31, 32};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (30).mode == OUTPUT);
    CHECK (arduino::pin (31).mode == OUTPUT);
    CHECK (arduino::pin (32).mode == OUTPUT);
    CHECK (arduino::shifted == std::string (1, '\0'));
    CHECK (arduino::pin (32).output == HIGH);
}

TEST (shiftRegisterHaltsOnAPinInUse)
{
    adk::Led           led     {31};
    adk::ShiftRegister outputs {30, 31, 32};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 31);
}

TEST (shiftRegisterLatchesOnlyOnceAllEightBitsAreIn)
{
    adk::ShiftRegister outputs {30, 31, 32};
    std::string        latch;

    adk::setup ();

    arduino::onDigitalWrite = [&] (uint8_t pin, uint8_t value)
    {
        if (pin == 32)
        {
            latch += value == HIGH ? 'H' : 'L';
            latch += static_cast<char> ('0' + arduino::shifted.size ());
        }
    };

    outputs.write (0xA5);

    CHECK (latch == "L1H2");
    CHECK (lastShifted () == 0xA5);
    CHECK (outputs.bits () == 0xA5);
}

TEST (shiftByteLatchesWithoutAnObject)
{
    adk::shiftByte (40, 41, 42, 0x3C);

    CHECK (arduino::shifted.size () == 1);
    CHECK (lastShifted () == 0x3C);
    CHECK (arduino::pin (42).output == HIGH);
}

TEST (stoppedShiftRegisterTurnsEveryOutputOff)
{
    adk::ShiftRegister outputs {30, 31, 32};

    adk::setup ();
    outputs.write (0xFF);
    adk::stop ();

    CHECK (lastShifted () == 0);
    CHECK (outputs.bits () == 0);
}
