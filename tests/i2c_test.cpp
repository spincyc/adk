#include "check.h"
#include "fake_i2c.h"

#include <adk/i2c.h>
#include <adk/mpu6050.h>
#include <adk/rtc.h>

#include <Arduino.h>

TEST (busDevicesShareSdaAndScl)
{
    fake_i2c::Chip clock  {0x68};
    fake_i2c::Chip motion {0x69};
    adk::Rtc       rtc;
    adk::Mpu6050   mpu {0x69};

    CHECK (adk::start ());
    CHECK (rtc.ok ());
    CHECK (mpu.ok ());
}

TEST (theBusPinsCannotAlsoBeUsedForSomethingElse)
{
    adk::Rtc rtc;

    CHECK (adk::start ());
    CHECK (!adk::claimOutput (SDA));
    CHECK (!adk::claimInput (SCL));
    CHECK (adk::fault () == adk::Fault::PinInUse);
}

TEST (anLedOnTheBusHaltsSetupAtItsPin)
{
    adk::Led     led {SCL};
    adk::Mpu6050 mpu;

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == SCL);
}

// The rest drive src/adk/i2c.cpp against the fake core's model of the TWI,
// and read what it did on the bus from arduino::twi.log.

TEST (i2cBeginsAt100kHzWithThePullUpsOn)
{
    CHECK (adk::i2c::begin ());

    // SCL = 16 MHz / (16 + 2 * TWBR * prescaler), the prescaler 1.
    CHECK (TWBR == 72);
    CHECK ((TWSR & (_BV (TWPS1) | _BV (TWPS0))) == 0);
    CHECK (TWCR & _BV (TWEN));
    CHECK (arduino::pin (SDA).output == HIGH);
    CHECK (arduino::pin (SCL).output == HIGH);
    CHECK (arduino::twi.log.empty ());
}

TEST (i2cBeginLeavesARunningBusAlone)
{
    CHECK (adk::i2c::begin ());
    TWBR = 12;

    CHECK (adk::i2c::begin ());
    CHECK (TWBR == 12);
}

TEST (i2cPresentAddressesAChipAndStops)
{
    fake_i2c::Chip clock {0x68};

    adk::i2c::begin ();

    CHECK (adk::i2c::present (0x68));
    CHECK (!adk::i2c::present (0x50));
    CHECK (arduino::twi.log == "S 68w+ P S 50w- P");
}

TEST (i2cWriteSendsEachByteAfterTheAddress)
{
    fake_i2c::Chip    clock   {0x68};
    constexpr uint8_t time [] = {0x00, 0x30, 0x59};

    adk::i2c::begin ();

    CHECK (adk::i2c::write (0x68, time, sizeof time));
    CHECK (arduino::twi.log == "S 68w+ 00+ 30+ 59+ P");
    CHECK (clock.registers[0x01] == 0x59);
}

TEST (i2cWriteRegisterSendsTheRegisterThenItsValue)
{
    fake_i2c::Chip motion {0x69};

    adk::i2c::begin ();

    CHECK (adk::i2c::writeRegister (0x69, 0x6B, 0x00));
    CHECK (arduino::twi.log == "S 69w+ 6B+ 00+ P");
}

TEST (i2cReadTurnsTheBusRoundWithARepeatedStart)
{
    fake_i2c::Chip motion {0x69};
    uint8_t        bytes [3] = {};

    motion.registers[0x3B] = 0xAA;
    motion.registers[0x3C] = 0xBB;
    motion.registers[0x3D] = 0xCC;
    adk::i2c::begin ();

    // Every byte but the last is acknowledged, which asks for another.
    CHECK (adk::i2c::read (0x69, 0x3B, bytes, sizeof bytes));
    CHECK (arduino::twi.log == "S 69w+ 3B+ Sr 69r+ AA+ BB+ CC- P");
    CHECK (bytes[0] == 0xAA && bytes[1] == 0xBB && bytes[2] == 0xCC);
}

TEST (i2cReadingNothingOnlySetsTheRegister)
{
    fake_i2c::Chip clock {0x68};

    adk::i2c::begin ();

    CHECK (adk::i2c::read (0x68, 0x07, nullptr, 0));
    CHECK (arduino::twi.log == "S 68w+ 07+ P");
    CHECK (clock.pointer == 0x07);
}

TEST (i2cGivesUpAtTheFirstByteRefused)
{
    constexpr uint8_t bytes [] = {0x01, 0x02, 0x03};
    int               written  = 0;

    arduino::twi.onAddress = [] (uint8_t, bool) { return true; };
    arduino::twi.onWrite   = [&] (uint8_t) { return ++written < 2; };

    adk::i2c::begin ();

    CHECK (!adk::i2c::write (0x40, bytes, sizeof bytes));
    CHECK (arduino::twi.log == "S 40w+ 01+ 02- P");
}

TEST (i2cReadFromAnAbsentChipSendsNoRegister)
{
    uint8_t byte = 0;

    adk::i2c::begin ();

    CHECK (!adk::i2c::read (0x68, 0x00, &byte, 1));
    CHECK (arduino::twi.log == "S 68w- P");
}

TEST (i2cWaitsForASlowStep)
{
    fake_i2c::Chip clock {0x68};

    arduino::twi.polls = 1500;
    adk::i2c::begin ();

    CHECK (adk::i2c::present (0x68));
    CHECK (arduino::twi.log == "S 68w+ P");
}

TEST (i2cGivesUpOnAStuckBusAndResetsTheTwi)
{
    fake_i2c::Chip clock {0x68};

    arduino::twi.stuck = true;
    adk::i2c::begin ();

    // Neither the start nor the stop ever finishes, so after about a
    // millisecond each the TWI is switched off and on again.
    CHECK (!adk::i2c::present (0x68));
    CHECK (arduino::twi.log == "off");
    CHECK (TWCR & _BV (TWEN));

    arduino::twi.stuck = false;

    CHECK (adk::i2c::present (0x68));
    CHECK (arduino::twi.log == "off S 68w+ P");
}

TEST (i2cGivesUpOnAChipThatHoldsTheClockMidTransfer)
{
    fake_i2c::Chip clock {0x68};
    uint8_t        time [7] = {};

    arduino::twi.onRead = [] (bool)
    {
        arduino::twi.stuck = true;
        return uint8_t {0x12};
    };

    adk::i2c::begin ();

    CHECK (!adk::i2c::read (0x68, 0x00, time, sizeof time));
    CHECK (arduino::twi.log == "S 68w+ 00+ Sr 68r+ 12+ off");
}
