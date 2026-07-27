#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "inert_channel_assessor.h"

namespace {

    static_assert (!std::is_copy_constructible<adk::InertChannelAssessor>::value,
                  "assessor owns its lifecycle");
    static_assert (!std::is_copy_assignable<adk::InertChannelAssessor>::value,
                  "assessor owns its lifecycle");
    static_assert (!std::is_move_constructible<adk::InertChannelAssessor>::value,
                  "assessor has a stable address");
    static_assert (!std::is_move_assignable<adk::InertChannelAssessor>::value,
                  "assessor has a stable address");
    static_assert (
        !std::is_copy_constructible<adk::RecordedInertObservationSource>::value,
        "source owns its lifecycle");
    static_assert (
        !std::is_move_constructible<adk::RecordedInertObservationSource>::value,
        "source has a stable address");

    adk::InertChannelObservation observation (adk::InertChannelId   channel,
                                              adk::InertObservation primary,
                                              adk::InertObservation redundant,
                                              uint32_t              observedAt) noexcept
    {
        return {channel, primary, redundant, adk::TimePoint (observedAt)};
    }

    adk::InertChannelState stateOf (const adk::InertChannelAssessor& assessor,
                                    adk::InertChannelId channel, uint32_t now)
    {
        const adk::Result<adk::InertChannelAssessment> result =
            assessor.assessment (channel, adk::TimePoint (now));

        assert (result.ok ());

        return result.value ().state;
    }

    adk::RecordedInertObservationSet recordedSet (
        uint32_t              dueAt,
        adk::InertObservation value) noexcept
    {
        const adk::TimePoint time (dueAt);

        return {
            time,
            {{0, value, value, time},
             {1, value, value, time},
             {2, value, value, time},
             {3, value, value, time},
             {4, value, value, time},
             {5, value, value, time},
             {6, value, value, time},
             {7, value, value, time}}};
    }

    void testRecordedSourceConfigurationAndLifecycle ()
    {
        adk::RecordedInertObservationSource nullSource (nullptr, 1);

        assert (nullSource.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        const adk::RecordedInertObservationSet one =
            recordedSet (0, adk::InertObservation::Open);
        adk::RecordedInertObservationSource empty (&one, 0);

        assert (empty.initialize ().error () == adk::StatusCode::InvalidArgument);

        const adk::RecordedInertObservationSet maximum[32] = {
            recordedSet (0, adk::InertObservation::Open),
            recordedSet (1, adk::InertObservation::Closed),
            recordedSet (2, adk::InertObservation::Closed),
            recordedSet (3, adk::InertObservation::Closed),
            recordedSet (4, adk::InertObservation::Closed),
            recordedSet (5, adk::InertObservation::Closed),
            recordedSet (6, adk::InertObservation::Closed),
            recordedSet (7, adk::InertObservation::Closed),
            recordedSet (8, adk::InertObservation::Closed),
            recordedSet (9, adk::InertObservation::Closed),
            recordedSet (10, adk::InertObservation::Closed),
            recordedSet (11, adk::InertObservation::Closed),
            recordedSet (12, adk::InertObservation::Closed),
            recordedSet (13, adk::InertObservation::Closed),
            recordedSet (14, adk::InertObservation::Closed),
            recordedSet (15, adk::InertObservation::Closed),
            recordedSet (16, adk::InertObservation::Closed),
            recordedSet (17, adk::InertObservation::Closed),
            recordedSet (18, adk::InertObservation::Closed),
            recordedSet (19, adk::InertObservation::Closed),
            recordedSet (20, adk::InertObservation::Closed),
            recordedSet (21, adk::InertObservation::Closed),
            recordedSet (22, adk::InertObservation::Closed),
            recordedSet (23, adk::InertObservation::Closed),
            recordedSet (24, adk::InertObservation::Closed),
            recordedSet (25, adk::InertObservation::Closed),
            recordedSet (26, adk::InertObservation::Closed),
            recordedSet (27, adk::InertObservation::Closed),
            recordedSet (28, adk::InertObservation::Closed),
            recordedSet (29, adk::InertObservation::Closed),
            recordedSet (30, adk::InertObservation::Closed),
            recordedSet (31, adk::InertObservation::Closed)};

        adk::RecordedInertObservationSource accepted  (maximum, 32);
        adk::RecordedInertObservationSource excessive (maximum, 33);

        assert (accepted.initialize ().ok ());
        assert (accepted.initialize ().ok ());
        assert (accepted.initialized ());
        assert (excessive.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        accepted.shutdown ();
        accepted.shutdown ();

        assert (!accepted.initialized ());
        assert (accepted.update (adk::TimePoint (0)).error () ==
                adk::StatusCode::NotInitialized);
        assert (!accepted.snapshot ().available);
    }

    void testRecordedSourceRejectsIncompleteAndUnorderedSets ()
    {
        adk::RecordedInertObservationSet duplicate =
            recordedSet (1, adk::InertObservation::Open);
        duplicate.observations[7].channel = 6;
        adk::RecordedInertObservationSource incomplete (&duplicate, 1);

        assert (incomplete.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::RecordedInertObservationSet duplicateTimes[] = {
            recordedSet (5, adk::InertObservation::Open),
            recordedSet (5, adk::InertObservation::Closed)};
        adk::RecordedInertObservationSource duplicateTime (duplicateTimes, 2);

        assert (duplicateTime.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::RecordedInertObservationSet reversed[] = {
            recordedSet (6, adk::InertObservation::Open),
            recordedSet (5, adk::InertObservation::Closed)};
        adk::RecordedInertObservationSource reverseSource (reversed, 2);

        assert (reverseSource.initialize ().error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testRecordedSourceRejectsMalformedObservations ()
    {
        adk::RecordedInertObservationSet invalidValue =
            recordedSet (10, adk::InertObservation::Open);
        invalidValue.observations[3].primary =
            static_cast<adk::InertObservation> (255);
        adk::RecordedInertObservationSource invalidValueSource (&invalidValue, 1);

        assert (invalidValueSource.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::RecordedInertObservationSet future =
            recordedSet (10, adk::InertObservation::Open);
        future.observations[3].observedAt = adk::TimePoint (11);
        adk::RecordedInertObservationSource futureSource   (&future, 1);

        assert (futureSource.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::RecordedInertObservationSet stale =
            recordedSet (110, adk::InertObservation::Open);
        stale.observations[3].observedAt = adk::TimePoint (10);
        adk::RecordedInertObservationSource staleSource   (&stale, 1);

        assert (staleSource.initialize ().ok ());
    }

    void testRecordedSourceSelectionAndExhaustion ()
    {
        const adk::RecordedInertObservationSet sets[] = {
            recordedSet (10, adk::InertObservation::Open),
            recordedSet (20, adk::InertObservation::Closed),
            recordedSet (30, adk::InertObservation::ShortSimulated)};
        adk::RecordedInertObservationSource source (sets, 3);

        assert (source.initialize ().ok ());
        assert (source.update (adk::TimePoint (9)).ok ());
        assert (!source.snapshot ().available);

        assert (source.update (adk::TimePoint (10)).ok ());

        const adk::RecordedInertObservationSnapshot first = source.snapshot ();

        assert (first.available);
        assert (first.observationCount == 8);
        assert (first.dueAt == adk::TimePoint (10));
        assert (first.observations == sets[0].observations);

        assert (source.update (adk::TimePoint (10)).ok ());
        assert (source.snapshot ().observations == first.observations);
        assert (source.update (adk::TimePoint (35)).ok ());
        assert (source.snapshot ().observations == sets[2].observations);
        assert (source.update (adk::TimePoint (1000)).ok ());
        assert (source.snapshot ().observations == sets[2].observations);
    }

    void testRecordedSourceRolloverAndReplay ()
    {
        const adk::RecordedInertObservationSet sets[] = {
            recordedSet (0xfffffff0u, adk::InertObservation::Open),
            recordedSet (0x00000010u, adk::InertObservation::Closed)};

        const adk::InertChannelObservation* replay[2] = {};

        for (uint8_t pass = 0; pass < 2; ++pass)
        {
            adk::RecordedInertObservationSource source (sets, 2);

            assert (source.initialize ().ok ());
            assert (source.update (adk::TimePoint (0xfffffff0u)).ok ());

            replay[pass] = source.snapshot ().observations;

            assert (source.update (adk::TimePoint (0x00000010u)).ok ());
            assert (source.snapshot ().observations == sets[1].observations);
        }

        assert (replay[0] == replay[1]);
    }

    void testLifecycleAndConfiguration ()
    {
        adk::InertChannelAssessor zero (adk::Duration (0));

        assert (zero.initialize ().error () == adk::StatusCode::InvalidArgument);

        adk::InertChannelAssessor ambiguous (adk::Duration (0x80000000u));

        assert (ambiguous.initialize ().error () == adk::StatusCode::InvalidArgument);

        adk::InertChannelAssessor assessor (adk::Duration (100));

        const adk::InertChannelObservation sample = observation (
            0, adk::InertObservation::Open, adk::InertObservation::Open, 0);

        assert (!assessor.initialized ());
        assert (assessor.update (adk::TimePoint (0), &sample, 1).error () ==
                adk::StatusCode::NotInitialized);
        assert (assessor.assessment (0, adk::TimePoint (0)).error () ==
                adk::StatusCode::NotInitialized);
        assert (assessor.initialize ().ok ());
        assert (assessor.initialize ().ok ());
        assert (assessor.initialized ());

        assessor.shutdown ();
        assessor.shutdown ();

        assert (!assessor.initialized ());
    }

    void testEveryObservationPair ()
    {
        constexpr adk::InertObservation values[] = {
            adk::InertObservation::Open, adk::InertObservation::Closed,
            adk::InertObservation::ShortSimulated, adk::InertObservation::Unavailable};

        adk::InertChannelAssessor assessor (adk::Duration (100));

        assert (assessor.initialize ().ok ());

        for (uint8_t primary = 0; primary < 4; ++primary)
        {
            for (uint8_t redundant = 0; redundant < 4; ++redundant)
            {
                const adk::InertChannelObservation sample =
                    observation (0, values[primary], values[redundant], 10);
                assert (assessor.update (adk::TimePoint (10), &sample, 1).ok ());

                adk::InertChannelState expected = adk::InertChannelState::Contradictory;

                if (primary == 3 || redundant == 3)
                {
                    expected = adk::InertChannelState::Unavailable;
                }
                else if (primary == redundant)
                {
                    expected = static_cast<adk::InertChannelState> (primary);
                }

                assert (stateOf (assessor, 0, 10) == expected);
            }
        }
    }

    void testCapacityMissingChannelsAndBoundaries ()
    {
        adk::InertChannelAssessor assessor (adk::Duration (100));

        assert (assessor.initialize ().ok ());
        assert (stateOf (assessor, 7, 0) == adk::InertChannelState::Unavailable);
        assert (assessor.assessment (8, adk::TimePoint (0)).error () ==
                adk::StatusCode::InvalidArgument);

        adk::InertChannelObservation samples[8];

        for (uint8_t channel = 0; channel < 8; ++channel)
        {
            samples[channel] = observation (channel, adk::InertObservation::Closed,
                                            adk::InertObservation::Closed, 50);
        }

        assert (assessor.update (adk::TimePoint (50), samples, 8).ok ());
        assert (assessor.update (adk::TimePoint (50), nullptr, 0).ok ());
        assert (assessor.update (adk::TimePoint (50), samples, 9).error () ==
                adk::StatusCode::InvalidArgument);
        assert (assessor.update (adk::TimePoint (50), nullptr, 1).error () ==
                adk::StatusCode::InvalidArgument);

        for (uint8_t channel = 0; channel < 8; ++channel)
        {
            assert (stateOf (assessor, channel, 50) == adk::InertChannelState::Closed);
        }
    }

    void testTransactionalRejection ()
    {
        adk::InertChannelAssessor assessor (adk::Duration (100));

        assert (assessor.initialize ().ok ());

        const adk::InertChannelObservation initial = observation (
            0, adk::InertObservation::Closed, adk::InertObservation::Closed, 10);
        assert (assessor.update (adk::TimePoint (10), &initial, 1).ok ());

        adk::InertChannelObservation duplicate[] = {
            observation (0, adk::InertObservation::Open, adk::InertObservation::Open,
                         20),
            observation (0, adk::InertObservation::Open, adk::InertObservation::Open,
                         20)};
        assert (assessor.update (adk::TimePoint (20), duplicate, 2).error () ==
                adk::StatusCode::InvalidArgument);
        assert (stateOf (assessor, 0, 20) == adk::InertChannelState::Closed);

        adk::InertChannelObservation invalid[] = {
            observation (0, adk::InertObservation::Open, adk::InertObservation::Open,
                         20),
            observation (8, adk::InertObservation::Open, adk::InertObservation::Open,
                         20)};
        assert (assessor.update (adk::TimePoint (20), invalid, 2).error () ==
                adk::StatusCode::InvalidArgument);
        assert (stateOf (assessor, 0, 20) == adk::InertChannelState::Closed);

        invalid[1] = observation (1, static_cast<adk::InertObservation> (255),
                                  adk::InertObservation::Open, 20);
        assert (assessor.update (adk::TimePoint (20), invalid, 2).error () ==
                adk::StatusCode::InvalidArgument);
        assert (stateOf (assessor, 0, 20) == adk::InertChannelState::Closed);
    }

    void testStalenessFutureAndRollover ()
    {
        adk::InertChannelAssessor assessor (adk::Duration (100));

        assert (assessor.initialize ().ok ());

        adk::InertChannelObservation sample = observation (
            0, adk::InertObservation::Open, adk::InertObservation::Open, 0xfffffff0u);
        assert (assessor.update (adk::TimePoint (0xfffffff0u), &sample, 1).ok ());
        assert (stateOf (assessor, 0, 0x00000054u) == adk::InertChannelState::Open);
        assert (stateOf (assessor, 0, 0x00000055u) == adk::InertChannelState::Stale);

        sample = observation (0, adk::InertObservation::Closed,
                              adk::InertObservation::Closed, 101);
        assert (assessor.update (adk::TimePoint (100), &sample, 1).error () ==
                adk::StatusCode::InvalidArgument);
        assert (stateOf (assessor, 0, 0x00000055u) == adk::InertChannelState::Stale);
        assert (assessor.assessment (0, adk::TimePoint (0xffffffefu)).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testRepeatedUpdateRecoveryAndShutdownClearing ()
    {
        adk::InertChannelAssessor assessor (adk::Duration (10));

        assert (assessor.initialize ().ok ());

        adk::InertChannelObservation sample = observation (
            3, adk::InertObservation::Open, adk::InertObservation::Open, 5);
        assert (assessor.update (adk::TimePoint (5), &sample, 1).ok ());
        assert (assessor.update (adk::TimePoint (5), &sample, 1).ok ());
        assert (stateOf (assessor, 3, 16) == adk::InertChannelState::Stale);

        sample = observation (3, adk::InertObservation::ShortSimulated,
                              adk::InertObservation::ShortSimulated, 16);
        assert (assessor.update (adk::TimePoint (16), &sample, 1).ok ());
        assert (stateOf (assessor, 3, 16) == adk::InertChannelState::ShortSimulated);

        assessor.shutdown ();

        assert (assessor.initialize ().ok ());
        assert (stateOf (assessor, 3, 16) == adk::InertChannelState::Unavailable);
    }

    void testDeterministicReplay ()
    {
        adk::InertChannelAssessment first[3];
        adk::InertChannelAssessment second[3];

        for (uint8_t pass = 0; pass < 2; ++pass)
        {
            adk::InertChannelAssessor assessor (adk::Duration (20));

            assert (assessor.initialize ().ok ());

            adk::InertChannelObservation samples[] = {
                observation (1, adk::InertObservation::Closed,
                             adk::InertObservation::Closed, 7),
                observation (2, adk::InertObservation::Open,
                             adk::InertObservation::ShortSimulated, 7)};
            assert (assessor.update (adk::TimePoint (7), samples, 2).ok ());

            adk::InertChannelAssessment* output = pass == 0 ? first : second;
            output[0] = assessor.assessment (1, adk::TimePoint (7)).value ();
            output[1] = assessor.assessment (2, adk::TimePoint (7)).value ();
            output[2] = assessor.assessment (1, adk::TimePoint (28)).value ();
        }

        for (uint8_t index = 0; index < 3; ++index)
        {
            assert (first[index].channel == second[index].channel);
            assert (first[index].state == second[index].state);
            assert (first[index].observedAt == second[index].observedAt);
        }
    }
} // namespace

int main ()
{
    testRecordedSourceConfigurationAndLifecycle         ();
    testRecordedSourceRejectsIncompleteAndUnorderedSets ();
    testRecordedSourceRejectsMalformedObservations      ();
    testRecordedSourceSelectionAndExhaustion            ();
    testRecordedSourceRolloverAndReplay                 ();

    testLifecycleAndConfiguration                 ();
    testEveryObservationPair                      ();
    testCapacityMissingChannelsAndBoundaries      ();
    testTransactionalRejection                    ();
    testStalenessFutureAndRollover                ();
    testRepeatedUpdateRecoveryAndShutdownClearing ();
    testDeterministicReplay                       ();
}
