#include "check.h"

#include <Arduino.h>

namespace {

    // millis () wraps round to zero after 2^32 ms, about 49.7 days. Times
    // are only ever subtracted, so everything carries straight on across.
    constexpr adk::Millis BeforeTheWrap = 0xFFFFFF00;
}

TEST (timerExpiresOnceAfterItsDuration)
{
    adk::Timer fuse;

    fuse.start (300);
    CHECK (fuse.isRunning ());
    CHECK (fuse.remaining () == 300);

    adk::update (1000);
    adk::update (1299);
    CHECK (!fuse.expired ());
    CHECK (fuse.remaining () == 1);

    adk::update (1300);
    CHECK (fuse.expired ());
    CHECK (!fuse.isRunning ());

    adk::update (1301);
    CHECK (!fuse.expired ());
}

TEST (stoppedTimerNeverExpires)
{
    adk::Timer fuse;

    fuse.start (100);
    adk::update (0);
    fuse.stop ();
    adk::update (500);

    CHECK (!fuse.expired ());
    CHECK (fuse.remaining () == 0);
}

TEST (stopwatchCountsWhileRunningAndCarriesOn)
{
    adk::Stopwatch watch;

    watch.start ();
    adk::update (1000);
    adk::update (1250);
    CHECK (watch.elapsed () == 250);

    watch.stop ();
    adk::update (2000);
    CHECK (watch.elapsed () == 250);

    watch.start ();
    adk::update (3000);
    adk::update (3100);
    CHECK (watch.elapsed () == 350);

    watch.reset ();
    CHECK (watch.elapsed () == 0);
    adk::update (3150);
    CHECK (watch.elapsed () == 50);
}

TEST (stopwatchRestartsFromZero)
{
    adk::Stopwatch watch;

    watch.start ();
    adk::update (100);
    adk::update (400);

    watch.restart ();
    CHECK (watch.elapsed () == 0);
    adk::update (500);
    adk::update (731);
    CHECK (watch.isRunning () && watch.elapsed () == 231);
}

TEST (anExpiredTimerStartedAgainStillReportsItsExpiry)
{
    adk::Timer fuse;

    fuse.start (100);
    adk::update (0);
    adk::update (100);
    CHECK (fuse.expired ());

    fuse.start (100);
    CHECK (fuse.expired ());
    CHECK (fuse.isRunning ());

    adk::update (101);
    CHECK (!fuse.expired ());

    adk::update (201);
    CHECK (fuse.expired ());
}

TEST (anExpiredTimerStoppedStillReportsItsExpiry)
{
    adk::Timer fuse;

    fuse.start (100);
    adk::update (0);
    adk::update (100);
    fuse.stop ();

    CHECK (fuse.expired ());

    adk::update (101);
    CHECK (!fuse.expired ());
}

TEST (adkStopCancelsEveryTimerAndHoldsEveryStopwatch)
{
    adk::Timer     fuse;
    adk::Stopwatch watch;

    fuse.start (100);
    watch.start ();
    adk::update (0);
    adk::update (60);

    adk::stop ();
    CHECK (!fuse.isRunning ());
    CHECK (!watch.isRunning ());
    CHECK (watch.elapsed () == 60);

    adk::update (500);
    CHECK (!fuse.expired ());
    CHECK (watch.elapsed () == 60);
}

TEST (aTimerRunsAcrossTheWrapOfMillis)
{
    adk::Timer fuse;

    fuse.start (0x200);
    adk::update (BeforeTheWrap);
    adk::update (0xFF);
    CHECK (!fuse.expired ());
    CHECK (fuse.remaining () == 1);

    adk::update (0x100);
    CHECK (fuse.expired ());
}

TEST (aStopwatchCountsAcrossTheWrapOfMillis)
{
    adk::Stopwatch watch;

    watch.start ();
    adk::update (BeforeTheWrap);
    adk::update (0x40);

    CHECK (watch.elapsed () == 0x140);
}

TEST (everyKeepsItsBeatAcrossTheWrapOfMillis)
{
    adk::Every tick {0x80};

    adk::update (BeforeTheWrap);
    adk::update (0xFFFFFF80);
    CHECK (tick.ticked ());

    adk::update (0xFFFFFFFF);
    CHECK (!tick.ticked ());

    adk::update (0);
    CHECK (tick.ticked ());

    adk::update (0x80);
    CHECK (tick.ticked ());
}

TEST (aRestartedEveryStillReportsItsTick)
{
    adk::Every tick {100};

    adk::update (0);
    adk::update (100);
    CHECK (tick.ticked ());

    tick.restart ();
    CHECK (tick.ticked ());

    adk::update (150);
    CHECK (!tick.ticked ());

    adk::update (249);
    CHECK (!tick.ticked ());

    adk::update (250);
    CHECK (tick.ticked ());
}
