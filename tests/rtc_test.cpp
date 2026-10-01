#include "check.h"
#include "fake_i2c.h"

#include <adk/rtc.h>

#include <Arduino.h>

namespace {

    bool same (adk::DateTime left, adk::DateTime right)
    {
        return left.year == right.year && left.month == right.month && left.day == right.day
            && left.hour == right.hour && left.minute == right.minute
            && left.second == right.second;
    }

    // A running clock at 8:30:45 pm on Thursday 24 September 2026.
    void setClock (fake_i2c::Chip& chip)
    {
        const uint8_t time [] = {0x45, 0x30, 0x20, 0x04, 0x24, 0x09, 0x26};

        memcpy (chip.registers, time, sizeof time);
    }

    // Updates a tenth of a second apart, when the Rtc reads the chip again.
    // The first update only starts the count.
    void readAgain (adk::Millis& now)
    {
        now += 100;
        adk::update (now);
    }
}

TEST (rtcLooksForItsChipAtSetup)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    adk::setup ();

    CHECK (rtc.ok ());
    CHECK (chip.transfers == 1);
}

TEST (aMissingRtcIsNotOkButDoesNotHalt)
{
    adk::Rtc rtc;

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (!rtc.ok ());
    CHECK (!rtc.isRunning ());
    CHECK (same (rtc.now (), {0, 0, 0, 0, 0, 0}));
}

TEST (rtcReadsTheTimeInOneTransfer)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    setClock (chip);
    adk::setup ();

    CHECK (same (rtc.now (), {2026, 9, 24, 20, 30, 45}));
    CHECK (chip.transfers == 1);
    CHECK (chip.pointer == 7);
    CHECK (rtc.isRunning ());
}

// Asking the time is free, so a sketch may ask on every pass of loop ():
// the bus is only used ten times a second, by update ().
TEST (rtcReadsTheChipTenTimesASecondWhateverTheSketchAsks)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    setClock (chip);
    adk::setup ();
    CHECK (chip.transfers == 1);

    for (adk::Millis now = 0; now < 1000; ++now)
    {
        adk::update (now);
        rtc.now ();
        rtc.isRunning ();
        rtc.ok ();
    }

    CHECK (chip.transfers == 1 + 9);

    // A new second shows from the next reading.
    chip.registers[0] = 0x46;
    adk::update (1000);
    CHECK (rtc.now ().second == 46);
}

// As in the clock lessons: an Every of 200 ms, declared after the Rtc,
// shows the time. Each of its ticks finds a reading made in that update,
// however late the updates come.
TEST (anEveryAfterTheRtcTicksWithAFreshReading)
{
    fake_i2c::Chip chip  {0x68};
    adk::Rtc       rtc;
    adk::Every     every {200};

    setClock (chip);
    adk::setup ();

    adk::Millis now    = 0;
    int         ticks  = 0;
    bool        fresh  = true;

    for (uint8_t pass = 0; pass < 200; ++pass)
    {
        uint8_t second    = pass % 60;
        chip.registers[0] = static_cast<uint8_t> ((second / 10) << 4 | second % 10);
        now              += static_cast<adk::Millis> (random (1, 40));
        adk::update (now);

        if (every.ticked ())
        {
            fresh = fresh && rtc.now ().second == second;
            ++ticks;
        }
    }

    CHECK (fresh);
    CHECK (ticks > 10);
}

TEST (rtcReadsTheChipTenTimesASecondAcrossTheWrapOfMillis)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    setClock (chip);
    adk::setup ();

    // A second of updates, from 256 ms before the wrap to 744 ms after.
    for (adk::Millis now = 0xFFFFFF00; now != 744; now += 10)
    {
        adk::update (now);
    }

    CHECK (chip.transfers == 1 + 9);
}

TEST (rtcReadsTheTwelveHourClock)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;
    adk::Millis    now = 0;

    setClock (chip);
    adk::setup ();
    adk::update (now);

    const uint8_t pm = 0x60;
    const uint8_t am = 0x40;

    chip.registers[2] = pm | 0x08;
    readAgain (now);
    CHECK (rtc.now ().hour == 20);

    chip.registers[2] = am | 0x12;
    readAgain (now);
    CHECK (rtc.now ().hour == 0);

    chip.registers[2] = pm | 0x12;
    readAgain (now);
    CHECK (rtc.now ().hour == 12);

    chip.registers[2] = am | 0x11;
    readAgain (now);
    CHECK (rtc.now ().hour == 11);
}

TEST (rtcIgnoresTheDs3231CenturyBit)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    setClock (chip);
    chip.registers[5] = 0x80 | 0x12;
    adk::setup ();

    CHECK (rtc.now ().month == 12);
}

TEST (aHaltedClockStartsWhenSet)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    chip.registers[0] = 0x80;
    adk::setup ();
    CHECK (!rtc.isRunning ());
    CHECK (rtc.ok ());

    rtc.set ({2026, 9, 24, 20, 30, 0});

    CHECK (rtc.isRunning ());
    CHECK (same (rtc.now (), {2026, 9, 24, 20, 30, 0}));
    CHECK (chip.registers[0] == 0x00);
    CHECK (chip.registers[1] == 0x30);
    CHECK (chip.registers[2] == 0x20);
    CHECK (chip.registers[4] == 0x24);
    CHECK (chip.registers[5] == 0x09);
    CHECK (chip.registers[6] == 0x26);
}

TEST (rtcWritesTheHoursForTheTwentyFourHourClock)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    chip.registers[2] = 0x40 | 0x20 | 0x11;
    adk::setup ();
    rtc.set ({2026, 1, 1, 23, 0, 0});

    CHECK (chip.registers[2] == 0x23);
}

TEST (aClockThatHaltsIsNotRunningFromTheNextReading)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;
    adk::Millis    now = 0;

    setClock (chip);
    adk::setup ();
    adk::update (now);
    CHECK (rtc.isRunning ());

    chip.registers[0] |= 0x80;
    readAgain (now);
    CHECK (!rtc.isRunning ());
    CHECK (rtc.ok ());
}

TEST (rtcTimesSurviveTheRoundTrip)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;
    adk::Millis    now = 0;

    adk::setup ();
    adk::update (now);

    const adk::DateTime times [] = {
        {2000, 1, 1, 0, 0, 0}, {2099, 12, 31, 23, 59, 59}, {2024, 2, 29, 12, 34, 56},
        {2031, 10, 9, 7, 8, 19}};

    // Read back from the chip, not just remembered.
    for (const adk::DateTime& time : times)
    {
        rtc.set (time);
        chip.registers[0] |= 0x80;
        readAgain (now);
        chip.registers[0] &= 0x7F;
        CHECK (!rtc.isRunning ());
        CHECK (same (rtc.now (), time));
    }
}

TEST (rtcWritesTheDayOfTheWeekMondayFirst)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;

    adk::setup ();

    struct Day
    {
        adk::DateTime date;
        uint8_t       weekday;
    };

    const Day days [] = {
        {{2000, 1, 1, 0, 0, 0}, 6},    {{2024, 2, 29, 0, 0, 0}, 4}, {{2026, 1, 1, 0, 0, 0}, 4},
        {{2026, 9, 24, 0, 0, 0}, 4},   {{2026, 9, 27, 0, 0, 0}, 7}, {{2026, 9, 28, 0, 0, 0}, 1},
        {{2099, 12, 31, 0, 0, 0}, 4}};

    for (const Day& day : days)
    {
        rtc.set (day.date);
        CHECK (chip.registers[3] == day.weekday);
    }
}

TEST (rtcThatStopsAnsweringIsNotOkUntilItAnswers)
{
    fake_i2c::Chip chip {0x68};
    adk::Rtc       rtc;
    adk::Millis    now = 0;

    setClock (chip);
    adk::setup ();
    adk::update (now);

    chip.failures = 1;
    readAgain (now);
    CHECK (same (rtc.now (), {0, 0, 0, 0, 0, 0}));
    CHECK (!rtc.ok ());
    CHECK (!rtc.isRunning ());

    readAgain (now);
    CHECK (rtc.now ().second == 45);
    CHECK (rtc.ok ());

    chip.present = false;
    rtc.set ({2026, 9, 24, 20, 30, 0});
    CHECK (!rtc.ok ());
    CHECK (chip.registers[0] == 0x45);
}

TEST (compiledAtReadsTheCompilersDateAndTime)
{
    CHECK (same (adk::compiledAt (F ("Sep 24 2026"), F ("20:30:00")), {2026, 9, 24, 20, 30, 0}));
    CHECK (same (adk::compiledAt (F ("Jan  4 2027"), F ("07:05:09")), {2027, 1, 4, 7, 5, 9}));
    CHECK (same (adk::compiledAt (F ("Dec 31 2099"), F ("23:59:59")), {2099, 12, 31, 23, 59, 59}));

    const char* const months [] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    for (uint8_t month = 0; month < 12; ++month)
    {
        std::string date = std::string (months[month]) + " 15 2030";

        CHECK (adk::compiledAt (F (date.c_str ()), F ("12:00:00")).month == month + 1);
    }
}

TEST (compiledAtIsWhenThisFileWasCompiled)
{
    adk::DateTime built = adk::compiledAt ();

    CHECK (same (built, adk::compiledAt (F (__DATE__), F (__TIME__))));
    CHECK (built.year >= 2026 && built.month >= 1 && built.month <= 12);
    CHECK (built.day >= 1 && built.day <= 31 && built.hour <= 23 && built.second <= 59);
}
