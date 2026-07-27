#include <assert.h>
#include <stdint.h>

#include "observation_tracker.h"

namespace {

    adk::TelemetrySample sample (uint16_t sequence, uint32_t observedMilliseconds = 0)
    {
        return {7,
                sequence,
                observedMilliseconds,
                adk::TelemetryKind::Temperature,
                adk::SampleQuality::Valid,
                215,
                -1};
    }

    adk::ObservationTracker tracker ()
    {
        return adk::ObservationTracker (7, {adk::Duration (10), adk::Duration (20)});
    }

    void testLifecycleAndConfiguration ()
    {
        adk::ObservationTracker invalid (7, {adk::Duration (20), adk::Duration (20)});
        assert                          (invalid.initialize ().error () == adk::StatusCode::InvalidArgument);
        assert                          (!invalid.initialized ());

        adk::ObservationTracker maximum (
            7, {adk::Duration (0x7ffffffe), adk::Duration (0x7fffffff)});
        assert (maximum.initialize ().ok ());

        adk::ObservationTracker agingTooLarge (
            7, {adk::Duration (0x80000000), adk::Duration (0x80000001)});
        assert (agingTooLarge.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::ObservationTracker staleTooLarge (
            7, {adk::Duration (1), adk::Duration (0x80000000)});
        assert (staleTooLarge.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::ObservationTracker value = tracker ();
        assert                                  (value.accept (sample (1), adk::TimePoint ()).error () ==
                adk::StatusCode::NotInitialized);
        assert (value.update (adk::TimePoint ()).error () ==
                adk::StatusCode::NotInitialized);
        assert         (value.initialize ().ok ());
        assert         (value.initialize ().ok ());
        assert         (value.initialized ());
        assert         (value.state ().status.ok ());
        value.shutdown ();
        value.shutdown ();
        assert         (!value.initialized ());
        assert         (value.state ().status.error () == adk::StatusCode::NotInitialized);
        assert         (value.initialize ().ok ());
    }

    void testSequenceClassificationAndReplacement ()
    {
        adk::ObservationTracker value = tracker ();
        assert                                  (value.initialize ().ok ());

        assert (value.accept (sample (65535), adk::TimePoint (100)).ok ());
        assert (value.state ().sequenceState == adk::SequenceState::First);

        assert (value.accept (sample (0), adk::TimePoint (101)).ok ());
        assert (value.state ().sequenceState == adk::SequenceState::InOrder);
        assert (value.state ().sample.sequence == 0);

        assert (value.accept (sample (0, 999), adk::TimePoint (500)).ok ());
        assert (value.state ().sequenceState == adk::SequenceState::Duplicate);
        assert (value.state ().sample.observedMilliseconds == 0);

        assert (value.accept (sample (3), adk::TimePoint (102)).ok ());
        assert (value.state ().sequenceState == adk::SequenceState::Gap);
        assert (value.state ().sample.sequence == 3);

        assert (value.accept (sample (2), adk::TimePoint (600)).ok ());
        assert (value.state ().sequenceState == adk::SequenceState::Reordered);
        assert (value.state ().sample.sequence == 3);

        assert (value.accept (sample (0x8003), adk::TimePoint (700)).ok ());
        assert (value.state ().sequenceState == adk::SequenceState::Reordered);
        assert (value.state ().sample.sequence == 3);
    }

    void testFreshnessBoundariesUseReceiptTimeOnly ()
    {
        adk::ObservationTracker value = tracker ();
        assert                                  (value.initialize ().ok ());
        assert                                  (value.accept (sample (1, UINT32_MAX), adk::TimePoint (100)).ok ());

        assert (value.update (adk::TimePoint (109)).ok ());
        assert (value.state ().freshness == adk::Freshness::Fresh);
        assert (value.update (adk::TimePoint (110)).ok ());
        assert (value.state ().freshness == adk::Freshness::Aging);
        assert (value.update (adk::TimePoint (119)).ok ());
        assert (value.state ().freshness == adk::Freshness::Aging);
        assert (value.update (adk::TimePoint (120)).ok ());
        assert (value.state ().freshness == adk::Freshness::Stale);

        adk::TelemetrySample reversed = sample (2, 0);
        assert                                 (value.accept (reversed, adk::TimePoint (200)).ok ());
        assert                                 (value.state ().freshness == adk::Freshness::Fresh);
        assert                                 (value.update (adk::TimePoint (210)).ok ());
        assert                                 (value.state ().freshness == adk::Freshness::Aging);
    }

    void testHalfRangeAmbiguityIsReordered ()
    {
        adk::ObservationTracker value = tracker ();
        assert                                  (value.initialize ().ok ());
        assert                                  (value.accept (sample (3), adk::TimePoint ()).ok ());
        assert                                  (value.accept (sample (0x8003), adk::TimePoint (1)).ok ());
        assert                                  (value.state ().sequenceState == adk::SequenceState::Reordered);
        assert                                  (value.state ().sample.sequence == 3);
    }

    void testLocalClockWrap ()
    {
        adk::ObservationTracker value = tracker ();
        assert                                  (value.initialize ().ok ());
        assert                                  (value.accept (sample (1), adk::TimePoint (UINT32_MAX - 4)).ok ());
        assert                                  (value.update (adk::TimePoint (4)).ok ());
        assert                                  (value.state ().age == adk::Duration (9));
        assert                                  (value.state ().freshness == adk::Freshness::Fresh);
        assert                                  (value.update (adk::TimePoint (5)).ok ());
        assert                                  (value.state ().age == adk::Duration (10));
        assert                                  (value.state ().freshness == adk::Freshness::Aging);
    }

    void testQualityAndInvalidInput ()
    {
        const adk::SampleQuality qualities[] = {
            adk::SampleQuality::Valid, adk::SampleQuality::SensorFault,
            adk::SampleQuality::OutOfRange, adk::SampleQuality::StaleAtSource};

        for (adk::SampleQuality quality : qualities)
        {
            adk::ObservationTracker value = tracker ();
            adk::TelemetrySample    input = sample  (1);
            input.quality                 = quality;
            assert (value.initialize ().ok ());
            assert (value.accept (input, adk::TimePoint (5)).ok ());
            assert (value.state ().sample.quality == quality);
            assert (value.state ().freshness == adk::Freshness::Fresh);
        }

        adk::ObservationTracker value = tracker ();
        assert                                  (value.initialize ().ok ());
        adk::TelemetrySample mismatch = sample  (1);
        mismatch.sourceId             = 8;
        assert (value.accept (mismatch, adk::TimePoint ()).error () ==
                adk::StatusCode::InvalidArgument);

        adk::TelemetrySample invalid = sample (1);
        invalid.quality              = static_cast<adk::SampleQuality> (255);
        assert (value.accept (invalid, adk::TimePoint ()).error () ==
                adk::StatusCode::InvalidArgument);

        invalid      = sample (1);
        invalid.kind = static_cast<adk::TelemetryKind> (255);
        assert (value.accept (invalid, adk::TimePoint ()).error () ==
                adk::StatusCode::InvalidArgument);
        assert (value.state ().sequenceState == adk::SequenceState::First);
    }

    void testRestartAndDeterministicReplay ()
    {
        adk::ObservationTracker first  = tracker ();
        adk::ObservationTracker second = tracker ();
        assert                                   (first.initialize ().ok ());
        assert                                   (second.initialize ().ok ());

        const uint16_t sequences[] = {10, 11, 11, 14, 13};
        for (uint8_t index = 0; index < 5; ++index)
        {
            const adk::TelemetrySample input = sample (sequences[index], index);
            assert                                    (first.accept (input, adk::TimePoint (index * 5)).ok ());
            assert                                    (second.accept (input, adk::TimePoint (index * 5)).ok ());
            assert                                    (first.update (adk::TimePoint (index * 5 + 2)).ok ());
            assert                                    (second.update (adk::TimePoint (index * 5 + 2)).ok ());

            const adk::ObservationState left  = first.state  ();
            const adk::ObservationState right = second.state ();
            assert                                           (left.sample.sequence == right.sample.sequence);
            assert                                           (left.sequenceState == right.sequenceState);
            assert                                           (left.freshness == right.freshness);
            assert                                           (left.age == right.age);
        }

        first.shutdown ();
        assert         (first.initialize ().ok ());
        assert         (first.accept (sample (99), adk::TimePoint ()).ok ());
        assert         (first.state ().sequenceState == adk::SequenceState::First);
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration             ();
    testSequenceClassificationAndReplacement  ();
    testFreshnessBoundariesUseReceiptTimeOnly ();
    testHalfRangeAmbiguityIsReordered         ();
    testLocalClockWrap                        ();
    testQualityAndInvalidInput                ();
    testRestartAndDeterministicReplay         ();
}
