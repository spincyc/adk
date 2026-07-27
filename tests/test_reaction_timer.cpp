#include <reaction_timer.h>

#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireStatus (
        adk::Status actual,
        adk::Status expected,
        const char* message)
    {
        require (actual == expected, message);
    }

    adk::ButtonObservation released ()
    {
        return {false, false, false};
    }

    adk::ButtonObservation held ()
    {
        return {true, false, false};
    }

    adk::ButtonObservation pressed ()
    {
        return {true, true, false};
    }

    adk::ButtonObservation releaseEvent ()
    {
        return {false, false, true};
    }

    adk::ReactionTimerConfig shortConfig ()
    {
        adk::ReactionTimerConfig config;

        config.readyDuration   = adk::Duration (10);
        config.waitDuration    = adk::Duration (20);
        config.responseTimeout = adk::Duration (30);
        config.resultDuration  = adk::Duration (40);
        return config;
    }

    void requireState (
        const adk::ReactionTimer& timer,
        adk::ReactionState        state,
        const char*               message)
    {
        require (timer.snapshot ().state == state, message);
    }

    bool sameSnapshot (
        const adk::ReactionTimerSnapshot& left,
        const adk::ReactionTimerSnapshot& right)
    {
        return left.state               == right.state &&
               left.outcome             == right.outcome &&
               left.ledPattern          == right.ledPattern &&
               left.status              == right.status &&
               left.cueTime             == right.cueTime &&
               left.reactionTime        == right.reactionTime &&
               left.cueVisible          == right.cueVisible &&
               left.ledOn               == right.ledOn &&
               left.hasCueTime          == right.hasCueTime &&
               left.hasReactionTime     == right.hasReactionTime;
    }

    void advanceToWait (adk::ReactionTimer& timer, uint32_t start)
    {
        requireStatus (
            timer.update (adk::TimePoint (start), pressed ()),
            adk::Status::Ok,
            "start trial");
        requireState (
            timer,
            adk::ReactionState::AwaitRelease,
            "await release");

        requireStatus (
            timer.update (adk::TimePoint (start + 1u), releaseEvent ()),
            adk::Status::Ok,
            "release trial");
        requireState (timer, adk::ReactionState::Ready, "ready state");

        requireStatus (
            timer.update (adk::TimePoint (start + 11u), released ()),
            adk::Status::Ok,
            "finish ready");
        requireState (timer, adk::ReactionState::Wait, "wait state");
    }

    void advanceToCue (adk::ReactionTimer& timer, uint32_t start)
    {
        advanceToWait (timer, start);

        requireStatus (
            timer.update (adk::TimePoint (start + 31u), released ()),
            adk::Status::Ok,
            "finish wait");
        requireState (timer, adk::ReactionState::Cue, "cue state");
    }

    void testInitialization ()
    {
        const adk::ReactionTimerConfig config = shortConfig     ();
        adk::ReactionTimer             timer                    (config);

        requireStatus (
            timer.update (adk::TimePoint (0), released ()),
            adk::Status::NotInitialized,
            "update before initialize");
        require (
            timer.snapshot ().status == adk::Status::NotInitialized,
            "initial snapshot status");

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "initialize");
        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "reinitialize");

        const adk::ReactionTimerSnapshot snapshot = timer.snapshot ();

        require (snapshot.state == adk::ReactionState::Idle, "initial idle");
        require (
            snapshot.outcome == adk::ReactionOutcome::None,
            "initial outcome");
        require (
            snapshot.ledPattern == adk::ReactionLedPattern::Off,
            "initial LED pattern");
        require (!snapshot.cueVisible, "initial cue hidden");
        require (!snapshot.ledOn, "initial LED off");
        require (!snapshot.hasCueTime, "initial cue time absent");
        require (!snapshot.hasReactionTime, "initial reaction absent");
    }

    void testInvalidConfiguration ()
    {
        const uint32_t invalidLarge =
            std::numeric_limits<uint32_t>::max ();

        for (uint8_t field = 0; field < 4; ++field)
        {
            adk::ReactionTimerConfig config = shortConfig ();

            switch (field)
            {
            case 0:
                config.readyDuration = adk::Duration (0);
                break;
            case 1:
                config.waitDuration = adk::Duration (0);
                break;
            case 2:
                config.responseTimeout = adk::Duration (0);
                break;
            case 3:
                config.resultDuration = adk::Duration (0);
                break;
            }

            adk::ReactionTimer timer (config);

            requireStatus (
                timer.initialize (),
                adk::Status::InvalidArgument,
                "zero duration rejected");
        }

        for (uint8_t field = 0; field < 4; ++field)
        {
            adk::ReactionTimerConfig config = shortConfig ();

            switch (field)
            {
            case 0:
                config.readyDuration = adk::Duration (invalidLarge);
                break;
            case 1:
                config.waitDuration = adk::Duration (invalidLarge);
                break;
            case 2:
                config.responseTimeout = adk::Duration (invalidLarge);
                break;
            case 3:
                config.resultDuration = adk::Duration (invalidLarge);
                break;
            }

            adk::ReactionTimer timer (config);

            requireStatus (
                timer.initialize (),
                adk::Status::InvalidArgument,
                "ambiguous duration rejected");
        }
    }

    void testReleaseGateAndBoundaries ()
    {
        adk::ReactionTimer timer (shortConfig ());

        requireStatus (timer.initialize (), adk::Status::Ok, "gate initialize");
        requireStatus (
            timer.update (adk::TimePoint (100), pressed ()),
            adk::Status::Ok,
            "gate press");
        requireStatus (
            timer.update (adk::TimePoint (101), held ()),
            adk::Status::Ok,
            "held start");
        requireState (
            timer,
            adk::ReactionState::AwaitRelease,
            "held start cannot advance");

        requireStatus (
            timer.update (adk::TimePoint (102), released ()),
            adk::Status::Ok,
            "observed release");
        requireState (timer, adk::ReactionState::Ready, "release opens gate");

        requireStatus (
            timer.update (adk::TimePoint (111), released ()),
            adk::Status::Ok,
            "before ready deadline");
        requireState (
            timer,
            adk::ReactionState::Ready,
            "ready before boundary");

        requireStatus (
            timer.update (adk::TimePoint (112), released ()),
            adk::Status::Ok,
            "at ready deadline");
        requireState (
            timer,
            adk::ReactionState::Wait,
            "ready boundary inclusive");

        requireStatus (
            timer.update (adk::TimePoint (131), released ()),
            adk::Status::Ok,
            "before wait deadline");
        requireState (
            timer,
            adk::ReactionState::Wait,
            "wait before boundary");

        requireStatus (
            timer.update (adk::TimePoint (132), pressed ()),
            adk::Status::Ok,
            "press at wait deadline");

        const adk::ReactionTimerSnapshot failure = timer.snapshot ();

        require (
            failure.state == adk::ReactionState::Failure,
            "deadline press fails");
        require (
            failure.outcome == adk::ReactionOutcome::PrematurePress,
            "input precedes wait deadline");
        require (!failure.hasCueTime, "false start has no cue");
        require (!failure.hasReactionTime, "false start has no reaction");
    }

    void testSuccessfulReaction ()
    {
        adk::ReactionTimer timer (shortConfig ());

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "success initialize");
        advanceToCue (timer, 100);

        const adk::ReactionTimerSnapshot cue = timer.snapshot ();

        require (cue.cueVisible, "cue visible");
        require (cue.ledOn, "cue LED on");
        require (cue.hasCueTime, "cue time present");
        require (cue.cueTime == adk::TimePoint (131), "cue time exact");
        require (!cue.hasReactionTime, "reaction initially absent");

        requireStatus (
            timer.update (adk::TimePoint (142), pressed ()),
            adk::Status::Ok,
            "successful press");

        const adk::ReactionTimerSnapshot success = timer.snapshot ();

        require (
            success.state == adk::ReactionState::Success,
            "success state");
        require (
            success.outcome == adk::ReactionOutcome::Success,
            "success outcome");
        require (success.hasReactionTime, "reaction present");
        require (
            success.reactionTime == adk::Duration (11),
            "reaction measured from cue");
        require (!success.cueVisible, "cue visibility ends");
        require (
            success.ledPattern == adk::ReactionLedPattern::SuccessPulse,
            "success pattern");

        requireStatus (
            timer.update (adk::TimePoint (181), held ()),
            adk::Status::Ok,
            "before result deadline");
        requireState (
            timer,
            adk::ReactionState::Success,
            "result persists before boundary");
        requireStatus (
            timer.update (adk::TimePoint (182), held ()),
            adk::Status::Ok,
            "at result deadline");
        requireState (
            timer,
            adk::ReactionState::Idle,
            "result boundary returns idle");
        requireStatus (
            timer.update (adk::TimePoint (183), held ()),
            adk::Status::Ok,
            "held after result");
        requireState (
            timer,
            adk::ReactionState::Idle,
            "held level cannot restart");
    }

    void testTimeoutPrecedence ()
    {
        adk::ReactionTimer timer (shortConfig ());

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "timeout initialize");
        advanceToCue (timer, 0);

        requireStatus (
            timer.update (adk::TimePoint (60), pressed ()),
            adk::Status::Ok,
            "press before timeout");
        require (
            timer.snapshot ().outcome == adk::ReactionOutcome::Success,
            "press one tick before timeout succeeds");

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "timeout reset");
        advanceToCue (timer, 100);

        requireStatus (
            timer.update (adk::TimePoint (161), pressed ()),
            adk::Status::Ok,
            "press at timeout");

        const adk::ReactionTimerSnapshot timeout = timer.snapshot ();

        require (
            timeout.state == adk::ReactionState::Failure,
            "timeout failure state");
        require (
            timeout.outcome == adk::ReactionOutcome::Timeout,
            "timeout precedes press");
        require (!timeout.hasReactionTime, "timeout has no reaction");
    }

    void testOneTransitionPerUpdate ()
    {
        adk::ReactionTimer timer (shortConfig ());

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "step initialize");
        requireStatus (
            timer.update (adk::TimePoint (10), pressed ()),
            adk::Status::Ok,
            "step start");
        requireStatus (
            timer.update (adk::TimePoint (1000), releaseEvent ()),
            adk::Status::Ok,
            "large release step");
        requireState (
            timer,
            adk::ReactionState::Ready,
            "large step advances once");
        requireStatus (
            timer.update (adk::TimePoint (1000), released ()),
            adk::Status::Ok,
            "same time ready update");
        requireState (
            timer,
            adk::ReactionState::Ready,
            "new state owns new deadline");

        requireStatus (
            timer.update (adk::TimePoint (1010), released ()),
            adk::Status::Ok,
            "ready deadline");
        requireState (
            timer,
            adk::ReactionState::Wait,
            "one ready transition");
    }

    void testClockValidation ()
    {
        adk::ReactionTimer timer (shortConfig ());

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "clock initialize");
        requireStatus (
            timer.update (adk::TimePoint (100), pressed ()),
            adk::Status::Ok,
            "clock baseline");

        const adk::ReactionTimerSnapshot before = timer.snapshot ();

        requireStatus (
            timer.update (adk::TimePoint (99), releaseEvent ()),
            adk::Status::InvalidArgument,
            "backward time rejected");

        const adk::ReactionTimerSnapshot rejected = timer.snapshot ();

        require (rejected.state == before.state, "rejection preserves state");
        require (
            rejected.status == adk::Status::InvalidArgument,
            "rejection visible");

        requireStatus (
            timer.update (adk::TimePoint (101), held ()),
            adk::Status::Ok,
            "valid time recovers");
    }

    void testClockWrap ()
    {
        const uint32_t start =
            std::numeric_limits<uint32_t>::max () - 15u;
        adk::ReactionTimer timer (shortConfig ());

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "wrap initialize");
        requireStatus (
            timer.update (adk::TimePoint (start), pressed ()),
            adk::Status::Ok,
            "wrap start");
        requireStatus (
            timer.update (adk::TimePoint (start + 1u), releaseEvent ()),
            adk::Status::Ok,
            "wrap release");
        requireStatus (
            timer.update (adk::TimePoint (start + 11u), released ()),
            adk::Status::Ok,
            "wrap ready");
        requireState  (timer, adk::ReactionState::Wait, "wait before wrap");
        requireStatus (
            timer.update (adk::TimePoint (start + 31u), released ()),
            adk::Status::Ok,
            "wait across wrap");
        requireState  (timer, adk::ReactionState::Cue, "cue across wrap");
        requireStatus (
            timer.update (adk::TimePoint (start + 38u), pressed ()),
            adk::Status::Ok,
            "reaction across wrap");
        require (
            timer.snapshot ().reactionTime == adk::Duration (7),
            "wrap reaction duration");
    }

    void testLedPatterns ()
    {
        adk::ReactionTimerConfig config = shortConfig ();

        config.readyDuration  = adk::Duration (1000);
        config.resultDuration = adk::Duration (1000);

        adk::ReactionTimer timer (config);

        requireStatus (
            timer.initialize (),
            adk::Status::Ok,
            "LED initialize");
        requireStatus (
            timer.update (adk::TimePoint (0), pressed ()),
            adk::Status::Ok,
            "LED start");
        requireStatus (
            timer.update (adk::TimePoint (1), releaseEvent ()),
            adk::Status::Ok,
            "LED ready");
        require       (timer.snapshot ().ledOn, "ready pulse starts on");
        requireStatus (
            timer.update (adk::TimePoint (251), released ()),
            adk::Status::Ok,
            "ready pulse off boundary");
        require       (!timer.snapshot ().ledOn, "ready pulse turns off");
        requireStatus (
            timer.update (adk::TimePoint (501), released ()),
            adk::Status::Ok,
            "ready pulse repeats");
        require (timer.snapshot ().ledOn, "ready pulse repeats on");
    }

    void testReplay ()
    {
        struct Row
        {
            uint32_t               time;
            adk::ButtonObservation button;
        };

        const Row rows[] = {
            {10, pressed      ()},
            {11, held         ()},
            {12, releaseEvent ()},
            {21, released     ()},
            {22, released     ()},
            {41, released     ()},
            {42, released     ()},
            {49, released     ()},
            {50, pressed      ()},
            {90, released     ()}
        };

        adk::ReactionTimer first  (shortConfig ());
        adk::ReactionTimer second (shortConfig ());

        requireStatus (
            first.initialize (),
            adk::Status::Ok,
            "first replay initialize");
        requireStatus (
            second.initialize (),
            adk::Status::Ok,
            "second replay initialize");

        for (const Row& row : rows)
        {
            requireStatus (
                first.update (adk::TimePoint (row.time), row.button),
                adk::Status::Ok,
                "first replay row");
            requireStatus (
                second.update (adk::TimePoint (row.time), row.button),
                adk::Status::Ok,
                "second replay row");
            require (
                sameSnapshot (first.snapshot (), second.snapshot ()),
                "replay snapshots identical");
        }
    }
}

int main ()
{
    testInitialization            ();
    testInvalidConfiguration      ();
    testReleaseGateAndBoundaries  ();
    testSuccessfulReaction        ();
    testTimeoutPrecedence         ();
    testOneTransitionPerUpdate    ();
    testClockValidation           ();
    testClockWrap                 ();
    testLedPatterns               ();
    testReplay                    ();

    std::cout << "All ADK reaction timer tests passed.\n";
}
