#include "../Lesson039AdapterPolicy.h"

#include <assert.h>

int main ()
{
    assert (lesson039NormalizeAdc (0) == 0);
    assert (lesson039NormalizeAdc (512) == 500);
    assert (lesson039NormalizeAdc (1023) == 1000);
    assert (lesson039NormalizeAdc (1200) == 1000);

    Lesson039SampleCadence cadence (2);

    assert (cadence.due (0));
    assert (!cadence.due (0));
    assert (!cadence.due (1));
    assert (cadence.due (2));
    assert (cadence.due (10));
    assert (!cadence.due (11));

    Lesson039SampleCadence rolloverCadence (4);

    assert (rolloverCadence.due (0xfffffffeUL));
    assert (!rolloverCadence.due (0));
    assert (rolloverCadence.due (2));

    Lesson039EvidenceLatch evidence (180);

    assert (!evidence.active (0));

    evidence.trigger (0xfffffff0UL);

    assert (evidence.active (100));
    assert (!evidence.active (164));

    Lesson039ReplaySchedule replay;
    auto                    event = replay.advance (0);

    assert (event.clear);
    assert (!event.play && event.attackMask == 0 && !event.completion);

    event = replay.advance (599);

    assert (event.attackMask == 0);

    event = replay.advance (601);

    assert (event.attackMask == 1);

    event = replay.advance (601);

    assert (event.attackMask == 0);

    event = replay.advance (661);

    assert (event.completion);
    assert (event.completionStartedAtMs == 560);
    assert (event.completionIntensity == 186);

    event = replay.advance (700);

    assert (!event.completion);

    event = replay.advance (4001);

    assert (event.play);

    event = replay.advance (4001);

    assert (!event.play);

    event = replay.advance (8001);

    assert (event.clear);

    Lesson039ReplaySchedule coalesced;
    coalesced.advance (100);

    event = coalesced.advance (2000);

    assert (event.attackMask == 7);
    assert (event.completion);
    assert (event.completionStartedAtMs == 1760);
    assert (event.completionIntensity == 306);

    Lesson039ReplaySchedule rollover;
    rollover.advance (0xfffffff0UL);

    event = rollover.advance (704);

    assert (event.clear);

    const auto duplicate = rollover.advance (704);

    assert (duplicate.attackMask == 0);
    assert (!duplicate.completion && !duplicate.play && !duplicate.clear);
}
