#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <type_traits>

#include "percussion_sequencer.h"

namespace {

    adk::PercussionSequencerConfig
    config (uint8_t steps = 16, uint16_t minimumTempo = 60, uint16_t maximumTempo = 120,
            uint32_t simultaneous = 10, uint32_t association = 100) noexcept
    {
        return {steps, minimumTempo, maximumTempo, adk::Duration (simultaneous),
                adk::Duration (association)};
    }

    adk::PercussionSequencerInput input (uint32_t now, uint16_t tempo = 0) noexcept
    {
        adk::PercussionSequencerInput result;

        result.observedAt    = adk::TimePoint (now);
        result.tempoPosition = tempo;
        return result;
    }

    void attack (adk::PercussionSequencerInput& value, uint8_t surface) noexcept
    {
        value.attackMask = static_cast<uint8_t> (
            value.attackMask | static_cast<uint8_t> (UINT8_C (1) << surface));
    }

    void quiet (adk::PercussionSequencerInput& value) noexcept
    {
        (void)value;
    }

    void completion (adk::PercussionSequencerInput& value, uint32_t startedAt,
                     uint32_t duration, uint16_t intensity) noexcept
    {
        value.acousticCompletion.present        = true;
        value.acousticCompletion.eventStartedAt = adk::TimePoint (startedAt);
        value.acousticCompletion.eventDuration  = adk::Duration  (duration);
        value.acousticCompletion.intensity      = intensity;
    }

    void recordGroup (adk::PercussionSequencer& sequencer, uint32_t at, uint8_t mask,
                      uint16_t intensity) noexcept
    {
        adk::PercussionSequencerInput group = input (at);

        group.attackMask = mask;
        assert (sequencer.update (group).ok ());

        adk::PercussionSequencerInput close = input (at + 1u);

        assert (sequencer.update (close).ok ());

        adk::PercussionSequencerInput finish = input (at + 2u);

        completion (finish, at, 1, intensity);
        assert     (sequencer.update (finish).ok ());
    }

    void testConfigurationTraitsAndLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::PercussionSequencer>::value,
                       "sequencer must not copy");
        static_assert (!std::is_move_constructible<adk::PercussionSequencer>::value,
                       "sequencer must not move");

        adk::PercussionSequencer sequencer (config ());

        assert (!sequencer.initialized ());
        assert (sequencer.update (input (0)).error () ==
                adk::StatusCode::NotInitialized);
        assert (sequencer.initialize ().ok ());
        assert (sequencer.initialize ().ok ());
        assert (sequencer.snapshot ().mode == adk::PercussionMode::Recording);

        sequencer.shutdown ();
        sequencer.shutdown ();
        assert             (!sequencer.initialized ());
        assert             (sequencer.snapshot ().status.error () ==
                adk::StatusCode::NotInitialized);
        assert (sequencer.initialize ().ok ());

        adk::PercussionSequencer tooFewSteps (config (3));
        assert                               (tooFewSteps.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer tooManySteps (config (17));
        assert                                (tooManySteps.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer slow     (config (16, 29));
        assert                            (slow.initialize ().error () == adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer fast     (config (16, 60, 241));
        assert                            (fast.initialize ().error () == adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer reversed (config (16, 121, 120));
        assert                            (reversed.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer zeroWindow (config (16, 60, 120, 0));
        assert                              (zeroWindow.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer shortAssociation (config (16, 60, 120, 10, 10));
        assert                                    (shortAssociation.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::PercussionSequencer ambiguous (
            config (16, 60, 120, 10, UINT32_C (0x80000000)));
        assert (ambiguous.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
    }

    void testGroupingAssociationAndOrdering ()
    {
        adk::PercussionSequencer sequencer (config ());

        assert (sequencer.initialize ().ok ());

        adk::PercussionSequencerInput first = input (100, 500);

        quiet  (first);
        attack (first, 3);
        attack (first, 1);
        assert (sequencer.update (first).ok ());
        assert (sequencer.snapshot ().tempoBpm == 90);
        assert (sequencer.snapshot ().hitAccepted);
        assert (sequencer.snapshot ().hitCount == 0);

        adk::PercussionSequencerInput edge = input (110, 1000);

        quiet  (edge);
        attack (edge, 0);
        assert (sequencer.update (edge).ok ());

        adk::PercussionSequencerInput completed = input (111, 1000);

        completion (completed, 90, 20, 700);
        assert     (sequencer.update (completed).ok ());

        adk::PercussionSequencerSnapshot snapshot = sequencer.snapshot ();

        assert (snapshot.hitCount == 3);
        assert (snapshot.lastHit.surface == 3);
        assert (snapshot.lastHit.intensity == 700);
        assert (snapshot.lastAssociation ==
                adk::PercussionAssociation::AcousticCompletion);
        assert (snapshot.lastHit.association ==
                adk::PercussionAssociation::AcousticCompletion);
        assert (sequencer.hit (0).value ().surface == 0);
        assert (sequencer.hit (1).value ().surface == 1);
        assert (sequencer.hit (2).value ().surface == 3);
        assert (sequencer.hit (0).value ().step == 0);

        adk::PercussionSequencerInput duplicate = input (120, 0);

        quiet  (duplicate);
        attack (duplicate, 0);
        assert (sequencer.update (duplicate).ok ());

        adk::PercussionSequencerInput close = input (131);

        quiet  (close);
        assert (sequencer.update (close).ok ());

        adk::PercussionSequencerInput duplicateCompletion = input (132);

        completion (duplicateCompletion, 119, 2, 900);
        assert     (sequencer.update (duplicateCompletion).ok ());
        assert     (sequencer.snapshot ().hitCount == 3);
        assert     (sequencer.snapshot ().hitSuppressed);

        const uint32_t nextOrdinal = sequencer.snapshot ().nextOrdinal;

        sequencer.shutdown ();
        assert             (sequencer.snapshot ().hitCount == 3);
        assert             (sequencer.snapshot ().nextOrdinal == nextOrdinal);
        assert             (sequencer.hit (2).ok ());
        assert             (!sequencer.snapshot ().hitAccepted);
        assert             (!sequencer.snapshot ().hitSuppressed);
        assert             (sequencer.snapshot ().lastAssociation ==
                adk::PercussionAssociation::None);
        assert (sequencer.initialize ().ok ());
        assert (sequencer.snapshot ().hitCount == 3);
        assert (sequencer.hit (3).error () == adk::StatusCode::InvalidArgument);
    }

    void testClosedGroupSuppressionAndTimeoutPrecedence ()
    {
        adk::PercussionSequencer sequencer (config ());

        assert                                      (sequencer.initialize ().ok ());
        adk::PercussionSequencerInput first = input (0);

        quiet  (first);
        attack (first, 2);
        assert (sequencer.update (first).ok ());

        adk::PercussionSequencerInput afterWindow = input (11);

        quiet  (afterWindow);
        attack (afterWindow, 1);
        assert (sequencer.update (afterWindow).ok ());
        assert (sequencer.snapshot ().hitSuppressed);

        adk::PercussionSequencerInput timeout = input (100);

        completion (timeout, 0, 100, 999);
        assert     (sequencer.update (timeout).ok ());
        assert     (sequencer.snapshot ().hitCount == 1);
        assert     (sequencer.hit (0).value ().intensity == 0);
        assert     (sequencer.hit (0).value ().association ==
                adk::PercussionAssociation::AssociationTimeout);
        assert (sequencer.snapshot ().lastAssociation ==
                adk::PercussionAssociation::AssociationTimeout);
    }

    void testAssociationEdgesAndInvalidCompletion ()
    {
        adk::PercussionSequencer sequencer (config ());

        assert                                      (sequencer.initialize ().ok ());
        adk::PercussionSequencerInput first = input (50);

        quiet                                       (first);
        attack                                      (first, 0);
        assert                                      (sequencer.update (first).ok ());
        adk::PercussionSequencerInput close = input (60);
        quiet                                       (close);
        assert                                      (sequencer.update (close).ok ());

        adk::PercussionSequencerInput before = input (61);

        completion (before, 51, 10, 200);
        assert     (sequencer.update (before).ok ());
        assert     (sequencer.snapshot ().hitCount == 0);

        adk::PercussionSequencerInput inclusiveStart = input (62);

        completion (inclusiveStart, 50, 1, 300);
        assert     (sequencer.update (inclusiveStart).ok ());
        assert     (sequencer.snapshot ().hitCount == 1);
        assert     (sequencer.hit (0).value ().intensity == 300);

        adk::PercussionSequencer invalidDuration (config ());

        assert                                        (invalidDuration.initialize ().ok ());
        adk::PercussionSequencerInput pending = input (0);
        quiet                                         (pending);
        attack                                        (pending, 0);
        assert                                        (invalidDuration.update (pending).ok ());
        adk::PercussionSequencerInput invalid = input (10);
        completion                                    (invalid, 0, 0, 999);
        assert                                        (invalidDuration.update (invalid).error () ==
                adk::StatusCode::InvalidArgument);
        assert (invalidDuration.snapshot ().faultSource ==
                adk::PercussionFaultSource::Acoustic);
        assert (invalidDuration.snapshot ().hitCount == 0);

        adk::PercussionSequencer futureCompletion (config ());

        assert                                       (futureCompletion.initialize ().ok ());
        adk::PercussionSequencerInput future = input (60);
        completion                                   (future, 100, 1, 999);
        assert                                       (futureCompletion.update (future).error () ==
                adk::StatusCode::InvalidArgument);
        assert (futureCompletion.snapshot ().faultSource ==
                adk::PercussionFaultSource::Acoustic);

        adk::PercussionSequencer ambiguousCompletion (config ());

        assert                                          (ambiguousCompletion.initialize ().ok ());
        adk::PercussionSequencerInput ambiguous = input (100);
        completion                                      (ambiguous, 0, UINT32_C (0x80000000), 999);
        assert                                          (ambiguousCompletion.update (ambiguous).error () ==
                adk::StatusCode::InvalidArgument);
        assert (ambiguousCompletion.snapshot ().faultSource ==
                adk::PercussionFaultSource::Acoustic);
    }

    void testQuantizationPlaybackFramesAndTempo ()
    {
        adk::PercussionSequencer sequencer (config (16, 60, 120, 1, 20));

        assert (sequencer.initialize ().ok ());

        adk::PercussionSequencerInput first = input (1000, 0);

        quiet                                             (first);
        attack                                            (first, 0);
        assert                                            (sequencer.update (first).ok ());
        adk::PercussionSequencerInput closeFirst = input  (1001);
        quiet                                             (closeFirst);
        assert                                            (sequencer.update (closeFirst).ok ());
        adk::PercussionSequencerInput finishFirst = input (1002);
        completion                                        (finishFirst, 1000, 1, 100);
        assert                                            (sequencer.update (finishFirst).ok ());

        adk::PercussionSequencerInput second = input (1125, 1000);

        quiet                                              (second);
        attack                                             (second, 1);
        assert                                             (sequencer.update (second).ok ());
        adk::PercussionSequencerInput closeSecond = input  (1126);
        quiet                                              (closeSecond);
        assert                                             (sequencer.update (closeSecond).ok ());
        adk::PercussionSequencerInput finishSecond = input (1127);
        completion                                         (finishSecond, 1125, 1, 500);
        assert                                             (sequencer.update (finishSecond).ok ());
        assert                                             (sequencer.hit (1).value ().step == 1);

        adk::PercussionSequencerInput play = input (1200, 0);

        quiet (play);
        play.playEvent = true;
        assert (sequencer.update (play).ok ());
        assert (sequencer.snapshot ().mode == adk::PercussionMode::Playing);
        assert (sequencer.snapshot ().frame.step == 0);
        assert (sequencer.snapshot ().frame.surfaceMask == 1);
        assert (sequencer.snapshot ().frame.frequencyHz == 262);
        assert (sequencer.snapshot ().frame.toneDuration == adk::Duration (60));

        adk::PercussionSequencerInput changedTempo = input (1300, 1000);

        quiet  (changedTempo);
        assert (sequencer.update (changedTempo).ok ());
        assert (sequencer.snapshot ().currentStep == 0);
        assert (sequencer.snapshot ().tempoBpm == 60);

        adk::PercussionSequencerInput boundary = input (1450, 1000);

        quiet  (boundary);
        assert (sequencer.update (boundary).ok ());
        assert (sequencer.snapshot ().currentStep == 1);
        assert (sequencer.snapshot ().frame.surfaceMask == 2);
        assert (sequencer.snapshot ().frame.frequencyHz == 330);

        adk::PercussionSequencerInput sparse = input (1825, 1000);

        quiet  (sparse);
        assert (sequencer.update (sparse).ok ());
        assert (sequencer.snapshot ().currentStep == 4);
        assert (sequencer.snapshot ().frame.surfaceMask == 0);
        assert (sequencer.snapshot ().frame.frequencyHz == 0);
        assert (sequencer.snapshot ().frame.heartbeat);
    }

    void testClearIdentityFaultAndRollover ()
    {
        adk::PercussionSequencer sequencer (config ());

        assert                                      (sequencer.initialize ().ok ());
        adk::PercussionSequencerInput first = input (UINT32_MAX - 5);

        quiet  (first);
        attack (first, 0);
        assert (sequencer.update (first).ok ());
        assert (sequencer.update (first).ok ());

        adk::PercussionSequencerInput wrapped = input (5);

        quiet  (wrapped);
        assert (sequencer.update (wrapped).ok ());

        adk::PercussionSequencerInput changed = wrapped;

        changed.playEvent = true;
        assert (sequencer.update (changed).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sequencer.snapshot ().mode == adk::PercussionMode::Fault);
        assert (!sequencer.snapshot ().frameValid);

        sequencer.shutdown ();
        assert             (sequencer.initialize ().ok ());
        sequencer.clear    ();
        assert             (sequencer.snapshot ().hitCount == 0);

        adk::PercussionSequencerInput badTempo = input (0);

        quiet (badTempo);
        badTempo.tempoPosition = 1001;
        assert (sequencer.update (badTempo).error () ==
                adk::StatusCode::InvalidArgument);

        sequencer.shutdown                            ();
        assert                                        (sequencer.initialize ().ok ());
        adk::PercussionSequencerInput invalid = input (0);

        quiet                        (invalid);
        invalid.attackMask = UINT8_C (0x10);
        invalid.clearEvent = true;
        assert (sequencer.update (invalid).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sequencer.snapshot ().faultSource == adk::PercussionFaultSource::Input);
    }

    void testCapacityFullPlaybackAndClear ()
    {
        adk::PercussionSequencer sequencer (config (16, 60, 60, 1, 20));

        assert (sequencer.initialize ().ok ());

        for (uint8_t step = 0; step < 7; ++step)
        {
            recordGroup (sequencer, static_cast<uint32_t> (step) * 250u, UINT8_C (0x0f),
                         static_cast<uint16_t> (step + 1u));
        }

        recordGroup (sequencer, 1750, UINT8_C (0x07), 8);
        assert      (sequencer.snapshot ().hitCount == 31);
        assert      (sequencer.snapshot ().nextOrdinal == 31);
        recordGroup (sequencer, 2000, UINT8_C (0x03), 9);
        assert      (sequencer.snapshot ().hitCount == 31);
        assert      (sequencer.snapshot ().nextOrdinal == 31);
        assert      (sequencer.snapshot ().hitSuppressed);
        assert      (sequencer.snapshot ().mode == adk::PercussionMode::Recording);
        recordGroup (sequencer, 2250, UINT8_C (0x01), 10);
        assert      (sequencer.snapshot ().hitCount == 32);
        assert      (sequencer.snapshot ().nextOrdinal == 32);
        assert      (sequencer.snapshot ().mode == adk::PercussionMode::Full);
        assert      (sequencer.snapshot ().patternFull);

        adk::PercussionSequencerInput rejected = input (2500);

        quiet  (rejected);
        attack (rejected, 0);
        assert (sequencer.update (rejected).ok ());
        assert (sequencer.snapshot ().hitSuppressed);
        assert (sequencer.snapshot ().hitCount == 32);

        adk::PercussionSequencerInput play = input (2501);

        quiet (play);
        play.playEvent = true;
        assert (sequencer.update (play).ok ());
        assert (sequencer.snapshot ().mode == adk::PercussionMode::Playing);

        adk::PercussionSequencerInput stop = input (2502);

        quiet (stop);
        stop.playEvent = true;
        assert (sequencer.update (stop).ok ());
        assert (sequencer.snapshot ().mode == adk::PercussionMode::Full);

        adk::PercussionSequencerInput clear = input (2503);

        quiet (clear);
        clear.playEvent  = true;
        clear.clearEvent = true;
        attack (clear, 0);
        assert (sequencer.update (clear).ok ());
        assert (sequencer.snapshot ().mode == adk::PercussionMode::Recording);
        assert (sequencer.snapshot ().hitCount == 0);
        assert (!sequencer.snapshot ().frameValid);
    }

    void testTimingAndNestedEvidenceFaults ()
    {
        adk::PercussionSequencer halfRange (config ());

        assert                                          (halfRange.initialize ().ok ());
        adk::PercussionSequencerInput first = input     (0);
        quiet                                           (first);
        assert                                          (halfRange.update (first).ok ());
        adk::PercussionSequencerInput ambiguous = input (UINT32_C (0x80000000));
        quiet                                           (ambiguous);
        assert                                          (halfRange.update (ambiguous).error () ==
                adk::StatusCode::InvalidArgument);

        adk::PercussionSequencer nestedTime (config ());

        assert                                         (nestedTime.initialize ().ok ());
        adk::PercussionSequencerInput mismatch = input (10);
        quiet                                          (mismatch);
        mismatch.surfaceStatus[2] = adk::StatusCode::HardwareFailure;
        assert (nestedTime.update (mismatch).error () ==
                adk::StatusCode::HardwareFailure);
        assert (nestedTime.snapshot ().faultSource ==
                adk::PercussionFaultSource::Surface2);

        adk::PercussionSequencer invalidTuple (config ());

        assert                                         (invalidTuple.initialize ().ok ());
        adk::PercussionSequencerInput acoustic = input (0);
        acoustic.acousticCompletion.intensity  = 1;
        assert (invalidTuple.update (acoustic).error () ==
                adk::StatusCode::InvalidArgument);

        adk::PercussionSequencer contactFault (config ());

        assert                                        (contactFault.initialize ().ok ());
        adk::PercussionSequencerInput contact = input (0);
        quiet                                         (contact);
        contact.surfaceStatus[0] = adk::StatusCode::InvalidArgument;
        assert (contactFault.update (contact).error () ==
                adk::StatusCode::InvalidArgument);
        assert (contactFault.snapshot ().faultSource ==
                adk::PercussionFaultSource::Surface0);

        adk::PercussionSequencer collision (config ());

        assert                                         (collision.initialize ().ok ());
        adk::PercussionSequencerInput faults = input   (0);
        faults.attackMask                    = UINT8_C (0x0f);
        faults.surfaceStatus[2]              = adk::StatusCode::ResourceBusy;
        faults.surfaceStatus[0]              = adk::StatusCode::HardwareFailure;
        faults.acousticStatus                = adk::StatusCode::Timeout;
        assert (collision.update (faults).error () == adk::StatusCode::HardwareFailure);
        assert (collision.snapshot ().faultSource ==
                adk::PercussionFaultSource::Surface0);
        assert (collision.snapshot ().hitCount == 0);
    }

    void appendSnapshot (char* output, size_t capacity, size_t& offset,
                         const adk::PercussionSequencer& sequencer)
    {
        const adk::PercussionSequencerSnapshot snapshot = sequencer.snapshot ();
        int                                    written  = snprintf           (
            output + offset, capacity - offset,
            "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
            static_cast<unsigned> (snapshot.mode), snapshot.tempoBpm,
            static_cast<unsigned> (snapshot.currentStep),
            static_cast<unsigned> (snapshot.hitCount), snapshot.nextOrdinal,
            snapshot.hitAccepted, snapshot.hitSuppressed, snapshot.patternFull,
            snapshot.frameValid, static_cast<unsigned> (snapshot.faultSource),
            static_cast<unsigned> (snapshot.lastAssociation),
            static_cast<unsigned> (snapshot.lastHit.surface),
            static_cast<unsigned> (snapshot.lastHit.step), snapshot.lastHit.intensity,
            snapshot.lastHit.ordinal,
            static_cast<unsigned> (snapshot.lastHit.association),
            static_cast<unsigned> (snapshot.status.error ()),
            static_cast<unsigned> (snapshot.frame.step),
            static_cast<unsigned> (snapshot.frame.surfaceMask));

        assert (written > 0);
        offset += static_cast<size_t> (written);

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            written = snprintf (output + offset, capacity - offset, " %u",
                                snapshot.frame.intensity[surface]);
            assert (written > 0);
            offset += static_cast<size_t> (written);
        }

        written = snprintf (
            output + offset, capacity - offset, " %u %u %u", snapshot.frame.frequencyHz,
            snapshot.frame.toneDuration.milliseconds (), snapshot.frame.heartbeat);
        assert (written > 0);
        offset += static_cast<size_t> (written);

        for (uint8_t index = 0; index < snapshot.hitCount; ++index)
        {
            const adk::PercussionHit hit = sequencer.hit (index).value ();

            written = snprintf (output + offset, capacity - offset, " %u %u %u %u %u",
                                static_cast<unsigned> (hit.surface),
                                static_cast<unsigned> (hit.step), hit.intensity,
                                hit.ordinal, static_cast<unsigned> (hit.association));
            assert (written > 0);
            offset += static_cast<size_t> (written);
        }

        assert (offset + 1 < capacity);
        output[offset++] = '\n';
        output[offset]   = '\0';
    }

    void testVersionedFieldwiseGoldenReplay ()
    {
        FILE* trace  = fopen ("tests/fixtures/percussion_sequencer_v1.trace", "r");
        FILE* golden = fopen ("tests/fixtures/percussion_sequencer_v1.golden", "r");

        assert (trace != nullptr);
        assert (golden != nullptr);

        char version[64];

        assert (fscanf (trace, "%63s", version) == 1);
        assert (strcmp (version, "PERCUSSION_TRACE_V1") == 0);

        adk::PercussionSequencer sequencer (config ());
        char                     actual[8192] = "PERCUSSION_GOLDEN_V1\n";
        size_t                   offset       = strlen (actual);

        assert (sequencer.initialize ().ok ());

        for (;;)
        {
            unsigned time;
            unsigned attackMask;
            unsigned surfaceStatus[4];
            unsigned acousticStatus;
            unsigned present;
            unsigned startedAt;
            unsigned duration;
            unsigned intensity;
            unsigned tempo;
            unsigned play;
            unsigned clear;

            const int fields =
                fscanf (trace, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u", &time,
                        &attackMask, &surfaceStatus[0], &surfaceStatus[1],
                        &surfaceStatus[2], &surfaceStatus[3], &acousticStatus, &present,
                        &startedAt, &duration, &intensity, &tempo, &play, &clear);

            if (fields == EOF)
            {
                break;
            }
            assert (fields == 14);

            adk::PercussionSequencerInput replay =
                input (time, static_cast<uint16_t> (tempo));

            replay.attackMask = static_cast<uint8_t> (attackMask);
            for (uint8_t surface = 0; surface < 4; ++surface)
            {
                replay.surfaceStatus[surface] =
                    static_cast<adk::StatusCode> (surfaceStatus[surface]);
            }
            replay.acousticStatus = static_cast<adk::StatusCode> (acousticStatus);
            replay.acousticCompletion.present = present != 0;
            if (replay.acousticCompletion.present)
            {
                replay.acousticCompletion.eventStartedAt = adk::TimePoint (startedAt);
                replay.acousticCompletion.eventDuration  = adk::Duration  (duration);
                replay.acousticCompletion.intensity = static_cast<uint16_t> (intensity);
            }
            replay.playEvent  = play != 0;
            replay.clearEvent = clear != 0;

            assert         (sequencer.update (replay).ok ());
            appendSnapshot (actual, sizeof (actual), offset, sequencer);
        }

        char         expected[8192];
        const size_t expectedSize = fread (expected, 1, sizeof (expected) - 1, golden);
        expected[expectedSize]    = '\0';

        assert (strcmp (actual, expected) == 0);
        assert (fclose (trace) == 0);
        assert (fclose (golden) == 0);
    }
} // namespace

int main ()
{
    testConfigurationTraitsAndLifecycle            ();
    testGroupingAssociationAndOrdering             ();
    testClosedGroupSuppressionAndTimeoutPrecedence ();
    testAssociationEdgesAndInvalidCompletion       ();
    testQuantizationPlaybackFramesAndTempo         ();
    testClearIdentityFaultAndRollover              ();
    testCapacityFullPlaybackAndClear               ();
    testTimingAndNestedEvidenceFaults              ();
    testVersionedFieldwiseGoldenReplay             ();
}
