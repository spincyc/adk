#include "check.h"

#include <Arduino.h>

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
