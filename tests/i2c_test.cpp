#include "check.h"
#include "fake_i2c.h"

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
