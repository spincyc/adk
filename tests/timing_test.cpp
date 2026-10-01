#include "check.h"

#include <adk/timing.h>

TEST (aStartTimeCountsFromTheFirstUpdateItSees)
{
    adk::StartTime start;

    CHECK (start.elapsed (4000) == 0);  // reading it starts nothing
    start.start (5000);
    CHECK (start.elapsed (5000) == 0);
    start.start (5100);                 // already started: no change
    CHECK (start.elapsed (5250) == 250);
}

TEST (aStartTimeRestartedByACommandCountsFromTheNextUpdate)
{
    adk::StartTime start;

    start.start (0);
    start.restart ();
    CHECK (start.elapsed (650) == 0);
    start.start (700);
    CHECK (start.elapsed (700) == 0);
    CHECK (start.elapsed (900) == 200);

    start.restart (1000);
    CHECK (start.elapsed (1100) == 100);
}

TEST (aStartTimeSurvivesMillisWrappingRound)
{
    adk::StartTime start;

    start.restart (0xFFFFFF00);
    CHECK (start.elapsed (0x00000100) == 0x200);
}

TEST (aBeatKeepsTimeHoweverLateItsUpdates)
{
    adk::StartTime start;

    CHECK (!start.beat (1000, 100));    // the first update only starts it
    CHECK (!start.beat (1099, 100));
    CHECK (start.beat (1130, 100));     // 30 ms late, and the next beat is still at 1200
    CHECK (!start.beat (1199, 100));
    CHECK (start.beat (1200, 100));

    // Two periods or more with no update: it starts again from then.
    CHECK (start.beat (1450, 100));
    CHECK (!start.beat (1549, 100));
    CHECK (start.beat (1550, 100));

    start.restart ();
    CHECK (!start.beat (1551, 100));
    CHECK (start.beat (1651, 100));
}

TEST (aBeatCrossesTheWrapOfMillis)
{
    adk::StartTime start;

    start.restart (0xFFFFFFC0);
    CHECK (!start.beat (0x0000001F, 100));
    CHECK (start.beat (0x00000024, 100));
    CHECK (start.beat (0x00000088, 100));
}

TEST (dueIsTrueAtTheFirstUpdateThenEachPeriodFromTheLast)
{
    adk::StartTime start;

    CHECK (start.due (1000, 100));
    CHECK (!start.due (1099, 100));
    CHECK (start.due (1130, 100));      // 30 ms late, which puts the next back to 1230
    CHECK (!start.due (1229, 100));
    CHECK (start.due (1230, 100));

    start.restart ();
    CHECK (start.due (1231, 100));
    CHECK (!start.due (1330, 100));

    start.restart (0xFFFFFFC0);
    CHECK (!start.due (0x00000023, 100));
    CHECK (start.due (0x00000024, 100));
}

TEST (passedIsTrueAPeriodAfterTheFirstUpdateThenEachPeriodFromTheLast)
{
    adk::StartTime start;

    CHECK (!start.passed (1000, 100));  // the first update only starts it
    CHECK (!start.passed (1099, 100));
    CHECK (start.passed (1130, 100));   // 30 ms late, which puts the next back to 1230
    CHECK (!start.passed (1229, 100));
    CHECK (start.passed (1230, 100));

    start.restart ();
    CHECK (!start.passed (1231, 100));
    CHECK (start.passed (1331, 100));

    start.restart (0xFFFFFFC0);
    CHECK (!start.passed (0x00000023, 100));
    CHECK (start.passed (0x00000024, 100));
}

TEST (interpolateGoesEitherWayAndStopsAtTheEnd)
{
    CHECK (adk::interpolate (100, 300, 0, 1000) == 100);
    CHECK (adk::interpolate (100, 300, 250, 1000) == 150);
    CHECK (adk::interpolate (300, 100, 250, 1000) == 250);
    CHECK (adk::interpolate (100, 300, 1000, 1000) == 300);
    CHECK (adk::interpolate (100, 300, 5000, 1000) == 300);
    CHECK (adk::interpolate (100, 300, 0, 0) == 300);
}

TEST (interpolateStaysOnCourseOverTheLongestTimes)
{
    CHECK (adk::interpolate (0, 65535, 0x80000000, 0xFFFFFFFF) == 32768);
    CHECK (adk::interpolate (65535, 0, 0x40000000, 0xFFFFFFFF) == 49151);
    CHECK (adk::interpolate (0, 256, 18000000, 36000000) == 128);
}
