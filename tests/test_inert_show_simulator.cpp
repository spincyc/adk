#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "inert_show_simulator.h"

namespace {

    static_assert (!std::is_copy_constructible<adk::InertShowSimulator>::value,
                   "simulator coordinates stable dependency references");
    static_assert (!std::is_move_constructible<adk::InertShowSimulator>::value,
                   "simulator has a stable address");

    adk::InertCueSchedulerConfig schedulerConfig () noexcept
    {
        adk::InertCueSchedulerConfig result;

        result.plan.cues[0]       = {3, adk::Duration (0), adk::Duration (10)};
        result.plan.cues[1]       = {7, adk::Duration (20), adk::Duration (10)};
        result.plan.count         = 2;
        result.confirmationWindow = adk::Duration (5);
        return result;
    }

    adk::InertCueChannelMap channelMap () noexcept
    {
        adk::InertCueChannelMap result = {};

        result.channels[0] = 5;
        result.channels[1] = 2;
        result.count       = 2;
        return result;
    }

    adk::CueOperatorInput operatorInput (bool review = false, bool run = false,
                                         bool confirm = false, bool skip = false,
                                         bool cancel = false) noexcept
    {
        return {review, run, confirm, skip, cancel};
    }

    void fillObservations (
        adk::InertChannelObservation (
            &observations)[adk::InertChannelAssessor::capacity],
        adk::TimePoint        now,
        adk::InertObservation state = adk::InertObservation::Closed) noexcept
    {
        for (uint8_t channel = 0; channel < adk::InertChannelAssessor::capacity;
             ++channel)
        {
            observations[channel] = {channel, state, state, now};
        }
    }

    adk::InertShowInput
    showInput (adk::InertChannelObservation (
                   &observations)[adk::InertChannelAssessor::capacity],
               const adk::CueOperatorInput& input) noexcept
    {
        return {observations, adk::InertChannelAssessor::capacity, input};
    }

    struct Fixture
    {
        adk::InertChannelAssessor assessor;
        adk::CueAuditEntry        storage[64];
        adk::CueAuditBuffer       audit;
        adk::InertCueScheduler    scheduler;
        adk::InertShowSimulator   simulator;

        Fixture () noexcept
            : assessor (adk::Duration (50)), audit (storage, 64),
              scheduler (schedulerConfig (), audit),
              simulator (channelMap (), assessor, scheduler, audit)
        {
        }
    };

    void beginRun (Fixture& fixture,
                   adk::InertChannelObservation (
                       &observations)[adk::InertChannelAssessor::capacity]) noexcept
    {
        fillObservations (observations, adk::TimePoint (0));

        assert (fixture.simulator
                    .update (adk::TimePoint (0),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Review);

        fillObservations (observations, adk::TimePoint (1));

        assert (fixture.simulator
                    .update (adk::TimePoint (1),
                             showInput (observations, operatorInput (true, true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Ready);
        assert (fixture.simulator.snapshot ().selectedChannel.channel == 5);
    }

    void testLifecycleAndMapValidation ()
    {
        Fixture fixture;

        assert (!fixture.assessor.initialized ());
        assert (!fixture.scheduler.initialized ());
        assert (!fixture.audit.initialized ());
        assert (fixture.simulator.initialize ().ok ());
        assert (fixture.simulator.initialize ().ok ());
        assert (fixture.assessor.initialized ());
        assert (fixture.scheduler.initialized ());
        assert (fixture.audit.count () == 1);

        fixture.simulator.shutdown ();
        fixture.simulator.shutdown ();

        assert (!fixture.assessor.initialized ());
        assert (!fixture.scheduler.initialized ());
        assert (fixture.audit.initialized ());
        assert (fixture.storage[fixture.audit.count () - 1].event ==
                adk::CueAuditEvent::Shutdown);

        assert (fixture.simulator.initialize ().ok ());
        assert (fixture.audit.count () == 1);

        fixture.simulator.shutdown ();

        adk::InertChannelAssessor invalidAssessor (adk::Duration (10));
        adk::CueAuditEntry        invalidStorage[8];
        adk::CueAuditBuffer       invalidAudit (invalidStorage, 8);

        adk::InertCueScheduler invalidScheduler (schedulerConfig (), invalidAudit);

        adk::InertCueChannelMap invalidMap = channelMap ();

        invalidMap.channels[0] = adk::InertChannelAssessor::capacity;

        adk::InertShowSimulator invalid (invalidMap, invalidAssessor, invalidScheduler,
                                         invalidAudit);

        assert (invalid.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        assert (!invalidAssessor.initialized ());
        assert (!invalidScheduler.initialized ());
    }

    void testClosedHoldRecoveryAndContradiction ()
    {
        Fixture fixture;

        assert (fixture.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        beginRun (fixture, observations);

        fillObservations (observations, adk::TimePoint (2));

        assert (
            fixture.simulator
                .update (adk::TimePoint (2),
                         showInput (observations, operatorInput (true, false, true)))
                .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Running);
        assert (fixture.simulator.snapshot ().schedule.hasCue);

        fillObservations (observations, adk::TimePoint (3));
        observations[5].primary   = adk::InertObservation::Open;
        observations[5].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (3),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Held);
        assert (!fixture.simulator.snapshot ().schedule.hasCue);

        fillObservations (observations, adk::TimePoint (4));
        observations[2].primary   = adk::InertObservation::Open;
        observations[2].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (4),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Held);

        fillObservations (observations, adk::TimePoint (5));
        observations[2].primary   = adk::InertObservation::Open;
        observations[2].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (5),
                             showInput (observations, operatorInput (true, true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Held);

        fillObservations (observations, adk::TimePoint (6));

        assert (fixture.simulator
                    .update (adk::TimePoint (6),
                             showInput (observations, operatorInput (true, true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Ready);
        assert (fixture.simulator.snapshot ().selectedChannel.channel == 2);

        fillObservations (observations, adk::TimePoint (7));
        observations[2].primary   = adk::InertObservation::Closed;
        observations[2].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (7),
                             showInput (observations, operatorInput (true)))
                    .error () == adk::StatusCode::InvalidConfiguration);
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Fault);
        assert (fixture.simulator.snapshot ().fault ==
                adk::InertShowFault::ObservationContradictory);
        assert (!fixture.simulator.snapshot ().schedule.hasCue);
    }

    void testFrameIdentityCancelAndReplayDigest ()
    {
        uint32_t digests[2] = {};

        for (uint8_t pass = 0; pass < 2; ++pass)
        {
            Fixture fixture;

            assert (fixture.simulator.initialize ().ok ());

            adk::InertChannelObservation
                observations[adk::InertChannelAssessor::capacity];

            beginRun (fixture, observations);

            const adk::InertShowSnapshot before = fixture.simulator.snapshot ();

            assert (fixture.simulator
                        .update (adk::TimePoint (1),
                                 showInput (observations, operatorInput (true, true)))
                        .ok ());
            assert (fixture.simulator.snapshot ().traceDigest == before.traceDigest);

            fillObservations (observations, adk::TimePoint (2));

            assert (
                fixture.simulator
                    .update (adk::TimePoint (2),
                             showInput (observations, operatorInput (true, false, false,
                                                                     false, true)))
                    .ok ());
            assert (fixture.simulator.snapshot ().state ==
                    adk::InertShowState::Cancelled);
            assert (!fixture.simulator.snapshot ().schedule.hasCue);

            digests[pass] = fixture.simulator.snapshot ().traceDigest;
        }

        assert (digests[0] == digests[1]);

        Fixture changed;

        assert (changed.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        fillObservations (observations, adk::TimePoint (1));

        assert (changed.simulator
                    .update (adk::TimePoint (1),
                             showInput (observations, operatorInput (true)))
                    .ok ());

        observations[0].primary = adk::InertObservation::Open;

        assert (changed.simulator
                    .update (adk::TimePoint (1),
                             showInput (observations, operatorInput (true)))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (changed.simulator.snapshot ().state == adk::InertShowState::Fault);
        assert (!changed.simulator.snapshot ().schedule.hasCue);
    }

    void testCompleteFrameValidation ()
    {
        Fixture fixture;

        assert (fixture.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        fillObservations (observations, adk::TimePoint (1));

        adk::InertShowInput incomplete = {observations,
                                          adk::InertChannelAssessor::capacity - 1,
                                          operatorInput (true)};

        assert (fixture.simulator.update (adk::TimePoint (1), incomplete).error () ==
                adk::StatusCode::InvalidArgument);
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Startup);
        assert (!fixture.simulator.snapshot ().schedule.hasCue);

        const adk::InertShowInput cancelWithoutFrame = {
            nullptr, 0, operatorInput (false, false, false, false, true)};

        const adk::Status cancelStatus =
            fixture.simulator.update (adk::TimePoint (2), cancelWithoutFrame);

        assert (cancelStatus.ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Cancelled);
    }

    void testDestinationGateAndReplayedFrameRejection ()
    {
        Fixture fixture;

        assert (fixture.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        beginRun (fixture, observations);

        fillObservations (observations, adk::TimePoint (2));

        assert (
            fixture.simulator
                .update (adk::TimePoint (2),
                         showInput (observations, operatorInput (true, false, true)))
                .ok ());

        fillObservations (observations, adk::TimePoint (12));
        observations[2].primary   = adk::InertObservation::Open;
        observations[2].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (12),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().schedule.phase ==
                adk::CueSchedulerPhase::Waiting);
        assert (fixture.simulator.snapshot ().schedule.cueIndex == 1);

        fillObservations (observations, adk::TimePoint (13));
        observations[2].primary   = adk::InertObservation::Open;
        observations[2].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (13),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Held);
        assert (!fixture.simulator.snapshot ().schedule.hasCue);

        const adk::InertShowSnapshot before = fixture.simulator.snapshot ();

        const uint8_t auditCount = fixture.audit.count ();

        fillObservations (observations, adk::TimePoint (11));

        assert (fixture.simulator
                    .update (adk::TimePoint (11),
                             showInput (observations, operatorInput (true)))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (fixture.simulator.snapshot ().state == before.state);
        assert (fixture.simulator.snapshot ().traceDigest == before.traceDigest);
        assert (fixture.audit.count () == auditCount);
    }

    void testCancelBeatsContradictoryEvidence ()
    {
        Fixture fixture;

        assert (fixture.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        beginRun (fixture, observations);

        fillObservations (observations, adk::TimePoint (2));
        observations[5].redundant = adk::InertObservation::Open;

        assert (fixture.simulator
                    .update (adk::TimePoint (2),
                             showInput (observations, operatorInput (true, false, false,
                                                                     false, true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Cancelled);
        assert (fixture.simulator.snapshot ().fault == adk::InertShowFault::None);
        assert (!fixture.simulator.snapshot ().schedule.hasCue);
    }

    void testNonterminalEvidenceHoldClasses ()
    {
        const adk::InertObservation holdStates[] = {
            adk::InertObservation::Open, adk::InertObservation::ShortSimulated,
            adk::InertObservation::Unavailable};

        for (const adk::InertObservation state : holdStates)
        {
            Fixture fixture;

            assert (fixture.simulator.initialize ().ok ());

            adk::InertChannelObservation
                observations[adk::InertChannelAssessor::capacity];

            beginRun (fixture, observations);

            fillObservations (observations, adk::TimePoint (2));
            observations[5].primary   = state;
            observations[5].redundant = state;

            assert (fixture.simulator
                        .update (adk::TimePoint (2),
                                 showInput (observations, operatorInput (true)))
                        .ok ());
            assert (fixture.simulator.snapshot ().state == adk::InertShowState::Held);
            assert (fixture.simulator.snapshot ().fault == adk::InertShowFault::None);
            assert (!fixture.simulator.snapshot ().schedule.hasCue);
        }

        Fixture stale;

        assert (stale.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        beginRun (stale, observations);

        fillObservations (observations, adk::TimePoint (60));

        observations[5].observedAt = adk::TimePoint (0);

        assert (stale.simulator
                    .update (adk::TimePoint (60),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (stale.simulator.snapshot ().state == adk::InertShowState::Held);
        assert (stale.simulator.snapshot ().fault == adk::InertShowFault::None);
        assert (!stale.simulator.snapshot ().schedule.hasCue);
    }

    void testAuditCapacityFaultRetainsShutdown ()
    {
        adk::InertChannelAssessor assessor (adk::Duration (50));
        adk::CueAuditEntry        storage[6];
        adk::CueAuditBuffer       audit     (storage, 6);
        adk::InertCueScheduler    scheduler (schedulerConfig (), audit);
        adk::InertShowSimulator   simulator (channelMap (), assessor, scheduler, audit);

        assert (simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        fillObservations (observations, adk::TimePoint (0));

        assert (simulator
                    .update (adk::TimePoint (0),
                             showInput (observations, operatorInput (true)))
                    .ok ());

        fillObservations (observations, adk::TimePoint (1));

        assert (simulator
                    .update (adk::TimePoint (1),
                             showInput (observations, operatorInput (true, true)))
                    .ok ());

        fillObservations (observations, adk::TimePoint (2));

        assert (
            simulator
                .update (adk::TimePoint (2),
                         showInput (observations, operatorInput (true, false, true)))
                .error () == adk::StatusCode::CapacityExceeded);
        assert (simulator.snapshot ().state == adk::InertShowState::Held);
        assert (simulator.snapshot ().fault == adk::InertShowFault::AuditFull);
        assert (!simulator.snapshot ().schedule.hasCue);

        simulator.shutdown ();

        assert (audit.count () == 6);
        assert (storage[audit.count () - 1].event == adk::CueAuditEvent::Shutdown);
    }

    void testFinalActiveHoldResumesComplete ()
    {
        Fixture fixture;

        assert (fixture.simulator.initialize ().ok ());

        adk::InertChannelObservation observations[adk::InertChannelAssessor::capacity];

        beginRun (fixture, observations);

        fillObservations (observations, adk::TimePoint (2));

        assert (
            fixture.simulator
                .update (adk::TimePoint (2),
                         showInput (observations, operatorInput (true, false, true)))
                .ok ());

        fillObservations (observations, adk::TimePoint (12));

        assert (fixture.simulator
                    .update (adk::TimePoint (12),
                             showInput (observations, operatorInput (true)))
                    .ok ());

        fillObservations (observations, adk::TimePoint (21));

        assert (fixture.simulator
                    .update (adk::TimePoint (21),
                             showInput (observations, operatorInput (true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().schedule.phase ==
                adk::CueSchedulerPhase::Confirmation);

        fillObservations (observations, adk::TimePoint (22));

        assert (
            fixture.simulator
                .update (adk::TimePoint (22),
                         showInput (observations, operatorInput (true, false, true)))
                .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Running);

        fillObservations (observations, adk::TimePoint (23));

        assert (fixture.simulator
                    .update (adk::TimePoint (23),
                             showInput (observations, operatorInput (false)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Held);
        assert (fixture.simulator.snapshot ().schedule.cueIndex ==
                fixture.scheduler.cueCount ());

        fillObservations (observations, adk::TimePoint (24));

        assert (fixture.simulator
                    .update (adk::TimePoint (24),
                             showInput (observations, operatorInput (true, true)))
                    .ok ());
        assert (fixture.simulator.snapshot ().state == adk::InertShowState::Complete);
        assert (fixture.simulator.snapshot ().fault == adk::InertShowFault::None);
        assert (!fixture.simulator.snapshot ().schedule.hasCue);
    }
} // namespace

int main ()
{
    testLifecycleAndMapValidation ();

    testClosedHoldRecoveryAndContradiction ();

    testFrameIdentityCancelAndReplayDigest ();

    testCompleteFrameValidation ();

    testDestinationGateAndReplayedFrameRejection ();

    testCancelBeatsContradictoryEvidence ();

    testNonterminalEvidenceHoldClasses ();

    testAuditCapacityFaultRetainsShutdown ();

    testFinalActiveHoldResumesComplete ();
}
