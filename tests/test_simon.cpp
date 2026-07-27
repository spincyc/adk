#include <simon.h>

#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

    struct CountingCueSource final : adk::CueSource
    {
        adk::Status reset () noexcept override
        {
            reads = 0;
            return adk::StatusCode::Ok;
        }

        adk::Status next (adk::CueId& cue) noexcept override
        {
            cue = static_cast<adk::CueId> (reads % adk::Simon::cueCount);
            ++reads;
            return adk::StatusCode::Ok;
        }

        adk::SimonAlgorithm algorithmVersion () const noexcept override
        {
            return adk::SimonAlgorithm::Fixed;
        }

        uint32_t seed () const noexcept override
        {
            return 0;
        }

        uint8_t reads = 0;
    };

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireStatus (adk::Status     actual,
                        adk::StatusCode expected,
                        const char*     message)
    {
        require (expected == adk::StatusCode::Ok ? actual.ok ()
                                                 : actual.error () == expected,
                 message);
    }

    adk::SimonConfig config (uint8_t maximum = 4)
    {
        adk::SimonConfig result;

        result.cueOnDuration  = adk::Duration (10);
        result.cueGapDuration = adk::Duration (5);
        result.inputTimeout   = adk::Duration (20);
        result.resultDuration = adk::Duration (7);
        result.startingLength = 1;
        result.growthPerRound = 1;
        result.maximumLength  = maximum;
        return result;
    }

    adk::SimonInput none ()
    {
        return {};
    }

    adk::SimonInput start ()
    {
        adk::SimonInput input;

        input.startEvent = true;
        return input;
    }

    adk::SimonInput press (uint8_t mask)
    {
        adk::SimonInput input;

        input.activeMask  = mask;
        input.pressedMask = mask;
        return input;
    }

    adk::SimonInput held (uint8_t mask)
    {
        adk::SimonInput input;

        input.activeMask = mask;
        return input;
    }

    adk::SimonInput release (uint8_t mask)
    {
        adk::SimonInput input;

        input.releasedMask = mask;
        return input;
    }

    void requirePhase (
        const adk::Simon& simon,
        adk::SimonPhase   phase,
        const char*       message)
    {
        require (simon.snapshot ().phase == phase, message);
    }

    uint32_t playOneCue (adk::Simon& simon, uint32_t now)
    {
        requirePhase (simon, adk::SimonPhase::PlaybackOn, "playback cue on");

        requireStatus (
            simon.update (adk::TimePoint (now + 9u), none ()),
            adk::StatusCode::Ok,
            "cue before boundary");
        requirePhase (simon, adk::SimonPhase::PlaybackOn, "cue remains before boundary");

        requireStatus (
            simon.update (adk::TimePoint (now + 10u), none ()),
            adk::StatusCode::Ok,
            "cue boundary");
        requirePhase (simon, adk::SimonPhase::PlaybackGap, "cue enters gap");

        requireStatus (
            simon.update (adk::TimePoint (now + 15u), none ()),
            adk::StatusCode::Ok,
            "gap boundary");
        return now + 15u;
    }

    uint32_t playSequence (adk::Simon& simon, uint32_t now)
    {
        const uint8_t length = simon.snapshot ().sequenceLength;

        for (uint8_t index = 0; index < length; ++index)
        {
            now = playOneCue (simon, now);
        }

        requirePhase (simon, adk::SimonPhase::AwaitPress, "playback accepts input");
        return now;
    }

    void startGame (adk::Simon& simon, uint32_t now)
    {
        requireStatus (simon.update (adk::TimePoint (now), start ()),
                       adk::StatusCode::Ok,
                       "start game");
        requirePhase (simon, adk::SimonPhase::PlaybackOn, "start enters playback");
    }

    void testSources ()
    {
        static const uint8_t expectedOne[] = {1, 1, 1, 3, 1, 0, 2, 2, 1, 0, 3, 3};
        static const uint8_t expectedNamed[] = {1, 3, 0, 0, 0, 1, 1, 1, 3, 1, 0, 0};
        static const uint8_t expectedZero[] = {3, 1, 0, 1, 0, 3, 0, 3, 2, 2, 3, 2};
        const uint32_t seeds[] = {1u, 0x12345678u, 0u};
        const uint8_t* vectors[] = {expectedOne, expectedNamed, expectedZero};

        for (uint8_t vector = 0; vector < 3; ++vector)
        {
            adk::XorShift32CueSource source (seeds[vector]);

            requireStatus (source.reset (), adk::StatusCode::Ok, "generator resets");

            require (
                source.algorithmVersion () == adk::SimonAlgorithm::XorShift32V1,
                "generator version stable");
            require (
                source.seed () == (seeds[vector] == 0u ? 0x6d2b79f5u : seeds[vector]),
                "generator seed metadata");

            for (uint8_t index = 0; index < 12; ++index)
            {
                adk::CueId cue = adk::CueId::One;

                requireStatus (source.next (cue), adk::StatusCode::Ok, "generator produces cue");

                require (
                    static_cast<uint8_t> (cue) == vectors[vector][index],
                    "generator golden vector");
            }

            requireStatus (source.reset (), adk::StatusCode::Ok, "generator replay reset");

            adk::CueId first = adk::CueId::One;
            requireStatus (source.next (first), adk::StatusCode::Ok, "generator replay");

            require (
                static_cast<uint8_t> (first) == vectors[vector][0],
                "generator replay deterministic");
        }

        const adk::CueId cues[] = {adk::CueId::Four, adk::CueId::Two};
        adk::FixedCueSource fixed (cues, 2);
        adk::CueId cue = adk::CueId::One;

        requireStatus (fixed.reset (), adk::StatusCode::Ok, "fixed source resets");
        requireStatus (fixed.next (cue), adk::StatusCode::Ok, "fixed first");

        require (cue == adk::CueId::Four, "fixed order first");

        requireStatus (fixed.next (cue), adk::StatusCode::Ok, "fixed second");

        require (cue == adk::CueId::Two, "fixed order second");

        requireStatus (
            fixed.next (cue), adk::StatusCode::CapacityExceeded, "fixed exhaustion");
        require (
            fixed.algorithmVersion () == adk::SimonAlgorithm::Fixed,
            "fixed version metadata");
        require (fixed.seed () == 0u, "fixed seed metadata");

        adk::FixedCueSource empty (nullptr, 0);

        requireStatus (
            empty.reset (), adk::StatusCode::InvalidArgument, "empty source rejected");
    }

    void testConfigurationAndInitialization ()
    {
        const adk::CueId cues[] = {adk::CueId::One};
        adk::FixedCueSource source (cues, 1);

        adk::Simon uninitialized (config (), source);

        requireStatus (
            uninitialized.update (adk::TimePoint (0), none ()),
            adk::StatusCode::NotInitialized,
            "update before initialize");

        for (uint8_t field = 0; field < 7; ++field)
        {
            adk::SimonConfig invalid = config ();

            switch (field)
            {
                case 0: invalid.cueOnDuration = adk::Duration (0); break;

                case 1: invalid.cueGapDuration = adk::Duration (0); break;

                case 2: invalid.inputTimeout = adk::Duration (0); break;

                case 3: invalid.resultDuration = adk::Duration (0); break;
                case 4: invalid.startingLength = 0; break;
                case 5: invalid.growthPerRound = 0; break;
                case 6: invalid.maximumLength = adk::Simon::sequenceCapacity + 1u; break;
            }

            adk::FixedCueSource invalidSource (cues, 1);

            adk::Simon simon (invalid, invalidSource);

            requireStatus (
                simon.initialize (), adk::StatusCode::InvalidArgument, "invalid config rejected");
        }

        adk::SimonConfig inverted = config ();
        inverted.startingLength = 2;
        inverted.maximumLength  = 1;
        adk::FixedCueSource invertedSource (cues, 1);

        adk::Simon invertedSimon (inverted, invertedSource);

        requireStatus (
            invertedSimon.initialize (),
            adk::StatusCode::InvalidArgument,
            "inverted lengths rejected");

        adk::SimonConfig excessive = config ();
        excessive.inputTimeout =
            adk::Duration (std::numeric_limits<uint32_t>::max ());
        adk::FixedCueSource excessiveSource (cues, 1);

        adk::Simon excessiveSimon (excessive, excessiveSource);

        requireStatus (
            excessiveSimon.initialize (),
            adk::StatusCode::InvalidArgument,
            "ambiguous duration rejected");

        adk::FixedCueSource shortSource (cues, 1);

        adk::SimonConfig needsTwo = config ();
        needsTwo.startingLength = 2;
        adk::Simon shortSimon (needsTwo, shortSource);

        requireStatus (
            shortSimon.initialize (),
            adk::StatusCode::CapacityExceeded,
            "source exhaustion reported");
        require (
            shortSimon.snapshot ().outcome == adk::SimonOutcome::SourceFailure,
            "source exhaustion outcome");

        const adk::CueId invalidCue[] = {static_cast<adk::CueId> (4)};
        adk::FixedCueSource invalidCueSource (invalidCue, 1);
        adk::Simon          invalidCueSimon  (config (1), invalidCueSource);

        requireStatus (
            invalidCueSimon.initialize (),
            adk::StatusCode::InvalidArgument,
            "out-of-range source cue rejected");
        require (
            invalidCueSimon.snapshot ().outcome == adk::SimonOutcome::SourceFailure,
            "invalid source cue records source failure");
    }

    void testPlaybackAndReleaseGate ()
    {
        const adk::CueId cues[] = {adk::CueId::Two, adk::CueId::Four};
        adk::FixedCueSource source (cues, 2);

        adk::Simon simon (config (2), source);

        requireStatus (simon.initialize (), adk::StatusCode::Ok, "play initialize");

        requirePhase (simon, adk::SimonPhase::Idle, "initialized idle");

        startGame (simon, 100);

        adk::SimonSnapshot snapshot = simon.snapshot ();

        require (snapshot.hasDisplayedCue, "playback exposes cue");
        require (snapshot.displayedCue == adk::CueId::Two, "playback cue identity");
        require (snapshot.ledMask == 0x02u, "playback LED mask");
        require (snapshot.hasDeadline, "playback exposes deadline");
        require (
            snapshot.nextDeadline == adk::TimePoint (110), "playback deadline");

        requireStatus (
            simon.update (adk::TimePoint (101), press (0x0fu)),
            adk::StatusCode::Ok,
            "playback ignores chord");
        requirePhase (simon, adk::SimonPhase::PlaybackOn, "input ignored during playback");

        uint32_t now = playSequence (simon, 100);

        snapshot = simon.snapshot ();

        require (snapshot.inputAccepted, "await press accepts input");
        require (snapshot.expectedCue == adk::CueId::Two, "expected cue exposed");

        requireStatus (
            simon.update (adk::TimePoint (++now), press (0x02u)),
            adk::StatusCode::Ok,
            "correct press");
        requirePhase (simon, adk::SimonPhase::AwaitRelease, "correct press gates release");

        requireStatus (
            simon.update (adk::TimePoint (++now), held (0x02u)),
            adk::StatusCode::Ok,
            "held input");
        requirePhase (simon, adk::SimonPhase::AwaitRelease, "hold cannot repeat");

        requireStatus (
            simon.update (adk::TimePoint (++now), release (0x02u)),
            adk::StatusCode::Ok,
            "release input");
        requirePhase (simon, adk::SimonPhase::RoundSuccess, "round succeeds after release");

        require (
            simon.snapshot ().outcome == adk::SimonOutcome::RoundComplete,
            "round outcome");

        requireStatus (
            simon.update (adk::TimePoint (now + 6u), none ()),
            adk::StatusCode::Ok,
            "result before boundary");
        requirePhase (simon, adk::SimonPhase::RoundSuccess, "result remains");

        requireStatus (
            simon.update (adk::TimePoint (now + 7u), none ()),
            adk::StatusCode::Ok,
            "result boundary");
        requirePhase (simon, adk::SimonPhase::PlaybackOn, "next round playback");

        require (simon.snapshot ().sequenceLength == 2u, "round grows sequence");
    }

    void testMismatchChordAndTimeout ()
    {
        const adk::CueId cues[] = {adk::CueId::One};

        {
            adk::FixedCueSource source (cues, 1);

            adk::Simon simon (config (1), source);

            requireStatus (simon.initialize (), adk::StatusCode::Ok, "mismatch initialize");

            startGame (simon, 0);

            uint32_t now = playSequence (simon, 0);

            requireStatus (
                simon.update (adk::TimePoint (++now), press (0x02u)),
                adk::StatusCode::Ok,
                "wrong cue");
            requirePhase (simon, adk::SimonPhase::GameFailure, "mismatch fails");

            require (
                simon.snapshot ().outcome == adk::SimonOutcome::Mismatch,
                "mismatch outcome");
            require (simon.snapshot ().hasObservedCue, "mismatch records observation");
        }

        for (uint8_t left = 0; left < 4; ++left)
        {
            for (uint8_t right = left + 1u; right < 4; ++right)
            {
                adk::FixedCueSource source (cues, 1);

                adk::Simon simon (config (1), source);

                requireStatus (simon.initialize (), adk::StatusCode::Ok, "chord initialize");

                startGame (simon, 0);

                const uint32_t now = playSequence (simon, 0);
                const uint8_t mask =
                    static_cast<uint8_t> ((1u << left) | (1u << right));
                requireStatus (
                    simon.update (adk::TimePoint (now + 1u), press (mask)),
                    adk::StatusCode::Ok,
                    "pair chord");
                require (
                    simon.snapshot ().outcome == adk::SimonOutcome::InvalidInput,
                    "every pair chord invalid");
            }
        }

        for (uint8_t mask : {uint8_t (0x07), uint8_t (0x0b), uint8_t (0x0d),
                             uint8_t (0x0e), uint8_t (0x0f)})
        {
            adk::FixedCueSource source (cues, 1);

            adk::Simon simon (config (1), source);

            requireStatus (simon.initialize (), adk::StatusCode::Ok, "multi initialize");

            startGame (simon, 0);

            const uint32_t now = playSequence (simon, 0);

            requireStatus (
                simon.update (adk::TimePoint (now + 1u), press (mask)),
                adk::StatusCode::Ok,
                "multi chord");
            require (
                simon.snapshot ().outcome == adk::SimonOutcome::InvalidInput,
                "triple/four chord invalid");
        }

        {
            adk::FixedCueSource source (cues, 1);

            adk::Simon simon (config (1), source);

            requireStatus (simon.initialize (), adk::StatusCode::Ok, "timeout initialize");

            startGame (simon, 50);

            const uint32_t now = playSequence (simon, 50);

            requireStatus (
                simon.update (adk::TimePoint (now + 19u), none ()),
                adk::StatusCode::Ok,
                "timeout before boundary");
            requirePhase (simon, adk::SimonPhase::AwaitPress, "before timeout accepted");

            requireStatus (
                simon.update (adk::TimePoint (now + 20u), press (0x01u)),
                adk::StatusCode::Ok,
                "press at timeout");
            require (
                simon.snapshot ().outcome == adk::SimonOutcome::Timeout,
                "timeout wins exact boundary");
        }
    }

    void testMaximumRestartReplayAndWrap ()
    {
        const adk::CueId cues[] = {adk::CueId::Three};
        adk::FixedCueSource source (cues, 1);

        adk::Simon simon (config (1), source);

        requireStatus (simon.initialize (), adk::StatusCode::Ok, "maximum initialize");

        startGame (simon, 0);

        uint32_t now = playSequence (simon, 0);

        requireStatus (
            simon.update (adk::TimePoint (++now), press (0x04u)),
            adk::StatusCode::Ok,
            "maximum correct press");
        requireStatus (
            simon.update (adk::TimePoint (++now), release (0x04u)),
            adk::StatusCode::Ok,
            "maximum release");
        requirePhase (simon, adk::SimonPhase::GameSuccess, "maximum enters success");

        require (
            simon.snapshot ().outcome == adk::SimonOutcome::GameComplete,
            "maximum outcome");

        requireStatus (
            simon.update (adk::TimePoint (++now), start ()),
            adk::StatusCode::Ok,
            "terminal restart");
        requirePhase (simon, adk::SimonPhase::PlaybackOn, "restart playback");

        require (
            simon.snapshot ().displayedCue == adk::CueId::Three,
            "restart resets source");

        const uint32_t nearWrap = std::numeric_limits<uint32_t>::max () - 5u;

        adk::FixedCueSource wrapSource (cues, 1);

        adk::Simon wrapSimon (config (1), wrapSource);

        requireStatus (wrapSimon.initialize (), adk::StatusCode::Ok, "wrap initialize");

        startGame (wrapSimon, nearWrap);

        requireStatus (
            wrapSimon.update (adk::TimePoint (3u), none ()),
            adk::StatusCode::Ok,
            "wrap before boundary");
        requirePhase (wrapSimon, adk::SimonPhase::PlaybackOn, "wrap before cue deadline");

        requireStatus (
            wrapSimon.update (adk::TimePoint (4u), none ()),
            adk::StatusCode::Ok,
            "wrap at boundary");
        requirePhase (wrapSimon, adk::SimonPhase::PlaybackGap, "wrap cue boundary");

        requireStatus (
            wrapSimon.update (adk::TimePoint (3u), none ()),
            adk::StatusCode::InvalidArgument,
            "backward time rejected");
    }

    void testInputValidationAndCapacity ()
    {
        const adk::CueId cues[] = {adk::CueId::One};
        const adk::SimonInput invalid[] = {
            {0x10u, 0, 0, false},
            {0, 0x10u, 0, false},
            {0, 0, 0x10u, false},
            {0, 0x01u, 0, false},
            {0x01u, 0x01u, 0x01u, false},
            {0x01u, 0, 0x01u, false}
        };

        for (const adk::SimonInput& input : invalid)
        {
            adk::FixedCueSource source (cues, 1);

            adk::Simon simon (config (1), source);

            requireStatus (simon.initialize (), adk::StatusCode::Ok, "input initialize");
            requireStatus (
                simon.update (adk::TimePoint (0), input),
                adk::StatusCode::InvalidArgument,
                "inconsistent input rejected");
        }

        CountingCueSource source;
        adk::SimonConfig maximum = config (adk::Simon::sequenceCapacity);
        maximum.startingLength = adk::Simon::sequenceCapacity;
        maximum.growthPerRound = adk::Simon::sequenceCapacity;
        adk::Simon simon (maximum, source);

        requireStatus (simon.initialize (), adk::StatusCode::Ok, "capacity initialize");

        require (
            source.reads == adk::Simon::sequenceCapacity,
            "capacity consumes exact source count");
        require (
            simon.snapshot ().sequenceLength == adk::Simon::sequenceCapacity,
            "capacity stores maximum length");
        requireStatus (simon.initialize (), adk::StatusCode::Ok, "capacity deterministic reset");

        require (
            source.reads == adk::Simon::sequenceCapacity,
            "reset does not over-read source");
    }

    void testReplaySnapshots ()
    {
        const adk::CueId cues[] = {adk::CueId::One, adk::CueId::Two};
        adk::SimonSnapshot firstRun[8];
        adk::SimonInput traceInput[8] = {
            start (), none (), none (), press (0x01u), release (0x01u),

            none (), none (), none ()
        };
        const uint32_t traceTime[8] = {10, 20, 25, 26, 27, 34, 44, 49};

        for (uint8_t run = 0; run < 2; ++run)
        {
            adk::FixedCueSource source (cues, 2);

            adk::Simon simon (config (2), source);

            requireStatus (simon.initialize (), adk::StatusCode::Ok, "replay initialize");

            for (uint8_t row = 0; row < 8; ++row)
            {
                requireStatus (
                    simon.update (adk::TimePoint (traceTime[row]), traceInput[row]),
                    adk::StatusCode::Ok,
                    "replay update");
                const adk::SimonSnapshot snapshot = simon.snapshot ();

                if (run == 0)
                {
                    firstRun[row] = snapshot;
                }
                else
                {
                    require (
                        snapshot.phase == firstRun[row].phase &&
                        snapshot.outcome == firstRun[row].outcome &&
                        snapshot.status.error () == firstRun[row].status.error () &&
                        snapshot.displayedCue == firstRun[row].displayedCue &&
                        snapshot.expectedCue == firstRun[row].expectedCue &&
                        snapshot.observedCue == firstRun[row].observedCue &&
                        snapshot.nextDeadline == firstRun[row].nextDeadline &&
                        snapshot.ledMask == firstRun[row].ledMask &&
                        snapshot.sequenceLength == firstRun[row].sequenceLength &&
                        snapshot.playbackIndex == firstRun[row].playbackIndex &&
                        snapshot.playerIndex == firstRun[row].playerIndex &&
                        snapshot.inputAccepted == firstRun[row].inputAccepted &&
                        snapshot.hasDisplayedCue == firstRun[row].hasDisplayedCue &&
                        snapshot.hasExpectedCue == firstRun[row].hasExpectedCue &&
                        snapshot.hasObservedCue == firstRun[row].hasObservedCue &&
                        snapshot.hasDeadline == firstRun[row].hasDeadline,
                        "replay snapshot identical");
                }
            }
        }
    }
}

int main ()
{
    testSources                    ();

    testConfigurationAndInitialization ();

    testPlaybackAndReleaseGate     ();
    testMismatchChordAndTimeout    ();

    testMaximumRestartReplayAndWrap ();
    testInputValidationAndCapacity  ();

    testReplaySnapshots            ();

    std::cout << "All Simon tests passed.\n";
    return EXIT_SUCCESS;
}
