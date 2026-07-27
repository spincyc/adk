#include <assert.h>
#include <stdint.h>

#include "observation_tracker.h"
#include "telemetry_evidence.h"
#include "telemetry_packet.h"

namespace {

    adk::TelemetrySample sample (uint16_t sequence, uint32_t observed)
    {
        return {26,
                sequence,
                observed,
                adk::TelemetryKind::Temperature,
                adk::SampleQuality::Valid,
                215,
                -1};
    }

    adk::TelemetryEvidenceSignal
    apply (adk::TelemetryFixtureSchedule& schedule, adk::ObservationTracker& tracker,
           adk::TelemetryPacketCodec& codec, adk::TimePoint now,
           adk::TelemetryFixtureAction expectedAction,
           adk::SequenceState expectedSequence, adk::Freshness expectedFreshness)
    {
        const adk::Result<adk::TelemetryFixtureDecision> scheduled =
            schedule.update (now);
        assert (scheduled.ok ());
        assert (scheduled.value ().action == expectedAction);

        adk::PacketValidity validity = adk::PacketValidity::Valid;

        if (scheduled.value ().action == adk::TelemetryFixtureAction::Accept ||
            scheduled.value ().action == adk::TelemetryFixtureAction::Corrupt)
        {
            const adk::TelemetrySample fixture =
                sample (scheduled.value ().sequence, now.milliseconds ());
            uint8_t packet[adk::TelemetryPacketCodec::size] = {};
            assert (codec.encode (fixture, {packet, sizeof (packet)}).ok ());

            if (scheduled.value ().action == adk::TelemetryFixtureAction::Corrupt)
            {
                packet[sizeof (packet) - 1] ^= 1;
            }

            adk::TelemetrySample decoded = {};
            validity = codec.decode ({packet, sizeof (packet)}, decoded);

            if (validity == adk::PacketValidity::Valid)
            {
                assert (tracker.accept (decoded, now).ok ());
            }
        }

        assert                             (tracker.update (now).ok ());
        assert                             (tracker.state ().sequenceState == expectedSequence);
        assert                             (tracker.state ().freshness == expectedFreshness);
        return adk::TelemetryEvidenceModel ().decide (validity, tracker.state (),
                                                      adk::StatusCode::Ok);
    }

    void testExactLessonTrace ()
    {
        adk::TelemetryFixtureSchedule schedule;
        adk::ObservationTracker tracker (26,
                                         {adk::Duration (2000), adk::Duration (5000)});
        adk::TelemetryPacketCodec codec;
        assert (schedule.initialize ().ok ());
        assert (tracker.initialize ().ok ());

        assert (apply (schedule, tracker, codec, adk::TimePoint (0),
                       adk::TelemetryFixtureAction::Accept, adk::SequenceState::First,
                       adk::Freshness::Fresh) == adk::TelemetryEvidenceSignal::Fresh);
        assert (apply (schedule, tracker, codec, adk::TimePoint (1000),
                       adk::TelemetryFixtureAction::Accept, adk::SequenceState::Gap,
                       adk::Freshness::Fresh) ==
                adk::TelemetryEvidenceSignal::GapOrAging);
        assert (apply (schedule, tracker, codec, adk::TimePoint (3000),
                       adk::TelemetryFixtureAction::Corrupt, adk::SequenceState::Gap,
                       adk::Freshness::Aging) == adk::TelemetryEvidenceSignal::Corrupt);
        assert (apply (schedule, tracker, codec, adk::TimePoint (5000),
                       adk::TelemetryFixtureAction::Silence, adk::SequenceState::Gap,
                       adk::Freshness::Aging) ==
                adk::TelemetryEvidenceSignal::GapOrAging);
        assert (apply (schedule, tracker, codec, adk::TimePoint (6000),
                       adk::TelemetryFixtureAction::None, adk::SequenceState::Gap,
                       adk::Freshness::Stale) == adk::TelemetryEvidenceSignal::Stale);
        assert (apply (schedule, tracker, codec, adk::TimePoint (12000),
                       adk::TelemetryFixtureAction::Accept, adk::SequenceState::InOrder,
                       adk::Freshness::Fresh) == adk::TelemetryEvidenceSignal::Fresh);
    }

    void testClockRollover ()
    {
        adk::TelemetryFixtureSchedule schedule;
        assert (schedule.initialize ().ok ());

        const adk::TimePoint beforeWrap (UINT32_MAX - 499);
        assert                          (schedule.update (beforeWrap).value ().action ==
                adk::TelemetryFixtureAction::Accept);
        assert (schedule.update (adk::TimePoint (500)).value ().action ==
                adk::TelemetryFixtureAction::Accept);
        assert (schedule.update (adk::TimePoint (2500)).value ().action ==
                adk::TelemetryFixtureAction::Corrupt);
        assert (schedule.update (adk::TimePoint (4500)).value ().action ==
                adk::TelemetryFixtureAction::Silence);
        assert (schedule.update (adk::TimePoint (11500)).value ().action ==
                adk::TelemetryFixtureAction::Accept);

        adk::TelemetryFixtureSchedule skippedCycle;
        assert (skippedCycle.initialize ().ok ());
        assert (skippedCycle.update (adk::TimePoint (10)).value ().action ==
                adk::TelemetryFixtureAction::Accept);
        assert (skippedCycle.update (adk::TimePoint (12010)).value ().action ==
                adk::TelemetryFixtureAction::Accept);
    }

    void testLifecycleAndReplay ()
    {
        adk::TelemetryFixtureSchedule first;
        adk::TelemetryFixtureSchedule second;
        assert (first.update (adk::TimePoint ()).error () ==
                adk::StatusCode::NotInitialized);
        assert (first.initialize ().ok ());
        assert (first.initialize ().ok ());
        assert (second.initialize ().ok ());

        const uint32_t times[] = {50, 1050, 3050, 5050, 6050, 12050};
        for (uint8_t index = 0; index < 6; ++index)
        {
            const adk::Result<adk::TelemetryFixtureDecision> left =
                first.update (adk::TimePoint (times[index]));
            const adk::Result<adk::TelemetryFixtureDecision> right =
                second.update (adk::TimePoint (times[index]));
            assert (left.ok ());
            assert (right.ok ());
            assert (left.value ().action == right.value ().action);
            assert (left.value ().sequence == right.value ().sequence);
        }

        first.shutdown ();
        first.shutdown ();
        assert         (!first.initialized ());
        assert         (first.initialize ().ok ());
        assert         (first.update (adk::TimePoint (999)).value ().sequence == 1);
    }

    void testEvidencePrecedence ()
    {
        const adk::ObservationState fresh = {sample (1, 0), adk::SequenceState::First,
                                             adk::Freshness::Fresh, adk::Duration (),
                                             adk::StatusCode::Ok};
        const adk::TelemetryEvidenceModel model;

        assert (model.decide (adk::PacketValidity::BadIntegrity, fresh,
                              adk::StatusCode::Ok) ==
                adk::TelemetryEvidenceSignal::Corrupt);
        assert (model.decide (adk::PacketValidity::BadIntegrity, fresh,
                              adk::StatusCode::HardwareFailure) ==
                adk::TelemetryEvidenceSignal::Fault);
    }
} // namespace

int main ()
{
    testExactLessonTrace   ();
    testClockRollover      ();
    testLifecycleAndReplay ();
    testEvidencePrecedence ();
}
