#include "check.h"
#include "fake_i2c.h"

#include <adk/mpu6050.h>

#include <Arduino.h>

#include <math.h>

namespace {

    bool near (float value, float expected)
    {
        return fabsf (value - expected) < 0.01f;
    }

    bool same (adk::Axes axes, int16_t x, int16_t y, int16_t z)
    {
        return axes.x == x && axes.y == y && axes.z == z;
    }

    // Raw counts as the chip stores them, big-endian from 0x3B.
    void tilt (fake_i2c::Chip& chip, int16_t x, int16_t y, int16_t z)
    {
        chip.put (0x3B, x);
        chip.put (0x3D, y);
        chip.put (0x3F, z);
    }

    // Set up, then run the updates that bring the first reading.
    void measureOnce ()
    {
        adk::setup ();
        adk::update (0);
        adk::update (20);
    }

    // A QMI8658, the chip on the board some kits carry in the GY-521's
    // place. Its WHO_AM_I register says what it is, and it stores readings
    // little-endian: temperature from 0x33, then acceleration and rotation.
    struct Qmi8658 : fake_i2c::Chip
    {
        explicit Qmi8658 (uint8_t at = 0x6B)
            : Chip (at)
        {
            registers[0x00] = 0x05;
        }

        void put (uint8_t reg, int16_t value)
        {
            uint16_t bits = static_cast<uint16_t> (value);

            registers[reg]     = static_cast<uint8_t> (bits);
            registers[reg + 1] = static_cast<uint8_t> (bits >> 8);
        }

        void tilt (int16_t x, int16_t y, int16_t z)
        {
            put (0x35, x);
            put (0x37, y);
            put (0x39, z);
        }
    };
}

TEST (mpuWakesAndConfiguresItsChipAtSetup)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    memset (chip.registers, 0xFF, sizeof chip.registers);
    chip.registers[0x6B] = 0x40;

    adk::setup ();

    CHECK (mpu.ok ());
    CHECK (chip.registers[0x6B] == 0x00);
    CHECK (chip.registers[0x1C] == 0x00);
    CHECK (chip.registers[0x1B] == 0x00);
    CHECK (chip.registers[0x1A] == 0x03);
}

TEST (mpuCanShareTheBusWithAClockAtTheOtherAddress)
{
    fake_i2c::Chip clock {0x68};
    fake_i2c::Chip motion {0x69};
    adk::Mpu6050   mpu {0x69};

    clock.registers[0x6B] = 0x40;
    adk::setup ();

    CHECK (mpu.ok ());
    CHECK (motion.transfers == 4);
    CHECK (clock.transfers == 0);
    CHECK (clock.registers[0x6B] == 0x40);
}

TEST (mpuReadsEveryTwentyMilliseconds)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    adk::setup ();
    uint16_t configured = chip.transfers;

    adk::update (1000);
    adk::update (1019);
    CHECK (!mpu.measured ());
    CHECK (chip.transfers == configured);

    adk::update (1020);
    CHECK (mpu.measured ());
    CHECK (chip.transfers == configured + 1);
    CHECK (chip.pointer == 0x3B + 14);

    adk::update (1021);
    CHECK (!mpu.measured ());

    adk::update (1039);
    CHECK (!mpu.measured ());

    adk::update (1040);
    CHECK (mpu.measured ());
    CHECK (chip.transfers == configured + 2);
}

TEST (mpuScalesAccelerationToMilliG)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    tilt (chip, 16384, -8192, 4096);
    measureOnce ();
    CHECK (same (mpu.acceleration (), 1000, -500, 250));

    tilt (chip, 32767, -32768, 0);
    adk::update (40);
    CHECK (same (mpu.acceleration (), 1999, -2000, 0));
}

TEST (mpuScalesRotationToDegreesPerSecond)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    chip.put (0x43, 131);
    chip.put (0x45, -13100);
    chip.put (0x47, 32767);
    measureOnce ();

    CHECK (same (mpu.rotation (), 1, -100, 250));
}

TEST (mpuFindsPitchAndRollFromGravity)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    tilt (chip, 0, 0, 16384);
    measureOnce ();
    CHECK (near (mpu.pitch (), 0));
    CHECK (near (mpu.roll (), 0));

    tilt (chip, 16384, 0, 0);
    adk::update (40);
    CHECK (near (mpu.pitch (), 90));

    tilt (chip, -16384, 0, 0);
    adk::update (60);
    CHECK (near (mpu.pitch (), -90));

    tilt (chip, 0, 16384, 0);
    adk::update (80);
    CHECK (near (mpu.pitch (), 0));
    CHECK (near (mpu.roll (), 90));

    tilt (chip, 0, -16384, 0);
    adk::update (100);
    CHECK (near (mpu.roll (), -90));

    tilt (chip, 11585, 0, 11585);
    adk::update (120);
    CHECK (near (mpu.pitch (), 45));
    CHECK (near (mpu.roll (), 0));
}

TEST (mpuReadsItsTemperature)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    chip.put (0x41, 0);
    measureOnce ();
    CHECK (near (mpu.temperature (), 36.53f));

    chip.put (0x41, -3920);
    adk::update (40);
    CHECK (near (mpu.temperature (), 25));
}

TEST (mpuKeepsItsReadingBetweenMeasurements)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    tilt (chip, 0, 0, 16384);
    measureOnce ();

    tilt (chip, 16384, 0, 0);
    adk::update (39);
    CHECK (same (mpu.acceleration (), 0, 0, 1000));
}

TEST (aMissingMpuIsNotOkButDoesNotHalt)
{
    adk::Mpu6050 mpu;

    measureOnce ();

    CHECK (!check::halted.happened);
    CHECK (mpu.measured ());
    CHECK (!mpu.ok ());
    CHECK (same (mpu.acceleration (), 0, 0, 0));
}

TEST (anMpuThatStopsAnsweringIsSetUpAgain)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    tilt (chip, 0, 0, 16384);
    measureOnce ();
    CHECK (mpu.measured ());

    chip.failures = 1;
    tilt (chip, 16384, 0, 0);
    adk::update (40);
    CHECK (mpu.measured ());
    CHECK (!mpu.ok ());
    CHECK (same (mpu.acceleration (), 0, 0, 1000));

    // It came back from a power glitch, asleep again.
    chip.registers[0x6B] = 0x40;
    adk::update (60);
    CHECK (mpu.ok ());
    CHECK (mpu.measured ());
    CHECK (chip.registers[0x6B] == 0x00);
    CHECK (same (mpu.acceleration (), 1000, 0, 0));
}

TEST (anMpuMissingAtSetupIsSetUpWhenItAnswers)
{
    fake_i2c::Chip chip {0x68};
    adk::Mpu6050   mpu;

    chip.present = false;
    adk::setup ();
    CHECK (!mpu.ok ());

    chip.present = true;
    chip.registers[0x6B] = 0x40;
    adk::update (0);
    adk::update (20);

    CHECK (mpu.ok ());
    CHECK (mpu.measured ());
    CHECK (chip.registers[0x6B] == 0x00);
}

TEST (mpuFindsAQmi8658BoardAndConfiguresIt)
{
    Qmi8658      chip;
    adk::Mpu6050 mpu;

    adk::setup ();

    CHECK (mpu.ok ());
    CHECK (chip.registers[0x02] == 0x40);
    CHECK (chip.registers[0x03] == 0x05);
    CHECK (chip.registers[0x04] == 0x45);
    CHECK (chip.registers[0x06] == 0x77);
    CHECK (chip.registers[0x08] == 0x03);
}

TEST (aQmi8658ReadsInTheSameUnitsAndAxes)
{
    Qmi8658      chip;
    adk::Mpu6050 mpu;

    chip.tilt (16384, -8192, 4096);
    chip.put (0x3B, 128);
    chip.put (0x3D, -12800);
    chip.put (0x3F, 32767);
    chip.put (0x33, 25 * 256 + 128);
    measureOnce ();

    CHECK (mpu.measured ());
    CHECK (chip.pointer == 0x33 + 14);
    CHECK (same (mpu.acceleration (), 1000, -500, 250));
    CHECK (same (mpu.rotation (), 1, -100, 255));
    CHECK (near (mpu.temperature (), 25.5f));

    chip.tilt (11585, 0, 11585);
    adk::update (40);
    CHECK (near (mpu.pitch (), 45));
    CHECK (near (mpu.roll (), 0));

    chip.tilt (0, 16384, 0);
    adk::update (60);
    CHECK (near (mpu.roll (), 90));
}

TEST (aQmi8658AnswersBesideAClockAtTheMpuAddress)
{
    fake_i2c::Chip clock  {0x68};
    Qmi8658        motion {0x6B};
    adk::Mpu6050   mpu    {0x68};

    clock.registers[0x6B] = 0x40;
    motion.tilt (0, 0, 16384);
    measureOnce ();

    CHECK (mpu.ok ());
    CHECK (clock.transfers == 0);
    CHECK (clock.registers[0x6B] == 0x40);
    CHECK (same (mpu.acceleration (), 0, 0, 1000));
}

TEST (aQmi8658WithItsAddressPinHighGoesWithAddress0x69)
{
    Qmi8658      low  {0x6B};
    Qmi8658      high {0x6A};
    adk::Mpu6050 mpu  {0x69};

    low.tilt (16384, 0, 0);
    high.tilt (0, 0, 16384);
    measureOnce ();

    CHECK (same (mpu.acceleration (), 0, 0, 1000));
    CHECK (low.transfers == 0);
}

TEST (aChipAt0x6BThatIsNotAQmi8658IsLeftAlone)
{
    fake_i2c::Chip other {0x6B};
    fake_i2c::Chip chip  {0x68};
    adk::Mpu6050   mpu;

    other.registers[0x00] = 0x42;
    tilt (chip, 0, 0, 16384);
    measureOnce ();

    CHECK (mpu.ok ());
    CHECK (other.transfers == 1);
    CHECK (other.registers[0x02] == 0x00);
    CHECK (same (mpu.acceleration (), 0, 0, 1000));
}

TEST (aQmi8658ThatStopsAnsweringIsSetUpAgain)
{
    Qmi8658      chip;
    adk::Mpu6050 mpu;

    chip.tilt (0, 0, 16384);
    measureOnce ();
    CHECK (mpu.ok ());

    chip.failures = 1;
    adk::update (40);
    CHECK (mpu.measured ());
    CHECK (!mpu.ok ());

    // It came back from a power glitch with its settings lost.
    chip.registers[0x08] = 0x00;
    adk::update (60);
    CHECK (mpu.ok ());
    CHECK (chip.registers[0x08] == 0x03);
}
