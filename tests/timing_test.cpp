#include "check.h"

#include <adk/timing.h>

TEST (aStartTimeCountsFromTheFirstUpdateItSees)
{
    adk::StartTime start;

    CHECK (start.elapsed (5000) == 0);
    CHECK (start.elapsed (5250) == 250);
}

TEST (aStartTimeRestartedByACommandCountsFromTheNextUpdate)
{
    adk::StartTime start;

    start.elapsed (0);
    start.restart ();
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
