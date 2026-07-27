#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <type_traits>

#include "inert_cue_scheduler.h"

namespace {

    static_assert (!std::is_copy_constructible<adk::InertCueScheduler>::value,
                   "scheduler owns its lifecycle");
    static_assert (!std::is_move_constructible<adk::InertCueScheduler>::value,
                   "scheduler has a stable address");

    adk::CueOperatorInput input (bool review = false, bool run = false,
                                 bool confirm = false, bool skip = false,
                                 bool cancel = false) noexcept
    {
        return {review, run, confirm, skip, cancel};
    }

    adk::InertCueSchedulerConfig config (uint32_t firstOffset = 0) noexcept
    {
        adk::InertCueSchedulerConfig result;

        result.plan.cues[0] = {3, adk::Duration (firstOffset), adk::Duration (10)};

        result.plan.cues[1]       = {7, adk::Duration (firstOffset + 20U),
                                     adk::Duration (10)};
        result.plan.count         = 2;
        result.confirmationWindow = adk::Duration (5);
        return result;
    }

    adk::CueEvidenceGate
    gate (adk::CueEvidenceDisposition disposition = adk::CueEvidenceDisposition::Permit,
          adk::Status                 status      = adk::StatusCode::Ok) noexcept
    {
        return {disposition, status};
    }

    void beginRun (adk::InertCueScheduler& scheduler, uint32_t start) noexcept
    {
        assert (scheduler.update (adk::TimePoint (start), input (true)).ok ());

        assert (
            scheduler.update (adk::TimePoint (start + 1U), input (true, true)).ok ());
    }

    void testLifecycleAndCopiedConfiguration ()
    {
        adk::CueAuditEntry  storage[20];
        adk::CueAuditBuffer audit (storage, 20);

        adk::InertCueSchedulerConfig source = config ();

        adk::InertCueScheduler scheduler (source, audit);

        source.plan.cues[0].id = 31;
        assert (scheduler.initialize ().ok ());

        assert (scheduler.cueCount () == 2);
        assert (scheduler.cue (0).ok ());
        assert (scheduler.cue (0).value ().id == 3);
        assert (scheduler.cue (2).error () == adk::StatusCode::InvalidArgument);

        assert (scheduler.initialize ().ok ());

        assert (audit.initialized ());

        assert (audit.count () == 1);

        beginRun (scheduler, 10);

        assert (scheduler.snapshot ().cue == 3);

        scheduler.shutdown ();

        scheduler.shutdown ();

        assert (!scheduler.initialized ());

        assert (audit.count () > 1);

        assert (storage[audit.count () - 1].event == adk::CueAuditEvent::Shutdown);

        assert (scheduler.initialize ().error () == adk::StatusCode::ResourceBusy);

        audit.shutdown ();

        assert (scheduler.initialize ().ok ());

        assert (storage[0].sequence == 0);

        scheduler.shutdown ();

        adk::CueAuditEntry  occupiedStorage[8];
        adk::CueAuditBuffer occupied (occupiedStorage, 8);

        assert (occupied.initialize ().ok ());

        adk::InertCueScheduler rejected (config (), occupied);

        assert (rejected.initialize ().error () == adk::StatusCode::ResourceBusy);

        assert (occupied.count () == 0);
    }

    void testDestructorShutdown ()
    {
        adk::CueAuditEntry  storage[8];
        adk::CueAuditBuffer audit (storage, 8);

        {
            adk::InertCueScheduler scheduler (config (10), audit);

            assert (scheduler.initialize ().ok ());

            assert (scheduler.update (adk::TimePoint (7), input (true)).ok ());
        }

        assert (audit.count () == 3);

        assert (storage[2].event == adk::CueAuditEvent::Shutdown);

        assert (storage[2].recordedAt == adk::TimePoint (7));

        assert (!storage[2].hasCue);
    }

    void testPlanValidation ()
    {
        for (uint8_t variant = 0; variant < 9; ++variant)
        {
            adk::CueAuditEntry  storage[12];
            adk::CueAuditBuffer audit (storage, 12);

            adk::InertCueSchedulerConfig invalid = config (1);

            switch (variant)
            {
                case 0: invalid.plan.count = 0; break;
                case 1: invalid.plan.count = 33; break;
                case 2: invalid.plan.cues[0].visibleFor = adk::Duration (0); break;
                case 3: invalid.plan.cues[0].id = 32; break;
                case 4: invalid.plan.cues[1].id = invalid.plan.cues[0].id; break;
                case 5: invalid.plan.cues[1].offset = adk::Duration (1); break;

                case 6: invalid.plan.cues[1].offset = adk::Duration (10); break;
                case 7:
                    invalid.plan.cues[1].offset = adk::Duration (0x7fffffffu);

                    invalid.plan.cues[1].visibleFor = adk::Duration (1);
                    break;
                case 8: invalid.confirmationWindow = adk::Duration (0x80000000u); break;
            }

            adk::InertCueScheduler scheduler (invalid, audit);

            assert (scheduler.initialize ().error () ==
                    adk::StatusCode::InvalidConfiguration);
            assert (!audit.initialized ());
        }

        adk::CueAuditEntry  storage[12];
        adk::CueAuditBuffer audit (storage, 12);

        adk::InertCueSchedulerConfig zeroWindow = config ();

        zeroWindow.confirmationWindow = adk::Duration ();

        adk::InertCueScheduler scheduler (zeroWindow, audit);

        assert (scheduler.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
    }

    void testLogicalWindowAndVisibility ()
    {
        adk::CueAuditEntry  storage[24];
        adk::CueAuditBuffer audit (storage, 24);

        adk::InertCueScheduler scheduler (config (), audit);

        assert (scheduler.initialize ().ok ());

        beginRun (scheduler, 100);

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Confirmation);

        assert (
            scheduler.update (adk::TimePoint (106), input (true, false, true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);

        assert (scheduler.update (adk::TimePoint (115), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);

        assert (scheduler.update (adk::TimePoint (116), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Waiting);

        assert (scheduler.snapshot ().cue == 7);

        assert (scheduler.update (adk::TimePoint (121), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Confirmation);

        assert (
            scheduler.update (adk::TimePoint (126), input (true, false, true)).ok ());
        assert (scheduler.update (adk::TimePoint (135), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);

        assert (scheduler.update (adk::TimePoint (136), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Complete);

        assert (!scheduler.snapshot ().hasCue);

        assert (storage[audit.count () - 1].event == adk::CueAuditEvent::Completed);
    }

    void testDelayedCoalescingAndTimeoutStatus ()
    {
        adk::InertCueSchedulerConfig delayed;

        delayed.confirmationWindow = adk::Duration (5);
        delayed.plan.count         = 3;
        delayed.plan.cues[0]       = {1, adk::Duration (0), adk::Duration (2)};

        delayed.plan.cues[1] = {2, adk::Duration (10), adk::Duration (2)};

        delayed.plan.cues[2] = {3, adk::Duration (20), adk::Duration (2)};

        adk::CueAuditEntry  storage[20];
        adk::CueAuditBuffer audit (storage, 20);

        adk::InertCueScheduler scheduler (delayed, audit);

        assert (scheduler.initialize ().ok ());

        beginRun (scheduler, 0);

        assert (scheduler.update (adk::TimePoint (30), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Complete);

        uint8_t timeoutSkips = 0;

        for (uint8_t index = 0; index < audit.count (); ++index)
        {
            if (storage[index].event == adk::CueAuditEvent::CueSkipped)
            {
                ++timeoutSkips;
                assert (storage[index].status.error () == adk::StatusCode::Timeout);
            }
        }

        assert (timeoutSkips == 3);
    }

    void testPriorityHoldResumeAndSameTimestamp ()
    {
        adk::CueAuditEntry  storage[24];
        adk::CueAuditBuffer audit (storage, 24);

        adk::InertCueScheduler scheduler (config (), audit);

        assert (scheduler.initialize ().ok ());

        beginRun (scheduler, 0);

        assert (scheduler.update (adk::TimePoint (2), input ()).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Held);

        assert (!scheduler.snapshot ().hasCue);

        assert (scheduler.update (adk::TimePoint (3), input (true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Held);

        assert (scheduler.update (adk::TimePoint (4), input (true, true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Confirmation);

        assert (scheduler.update (adk::TimePoint (5), input (true, false, true)).ok ());

        const uint8_t count = audit.count ();

        assert (scheduler.update (adk::TimePoint (5), input (true, false, true)).ok ());

        assert (audit.count () == count);

        assert (scheduler.update (adk::TimePoint (5), input (true)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Fault);

        scheduler.shutdown ();

        audit.shutdown ();

        adk::InertCueScheduler chord (config (), audit);

        assert (chord.initialize ().ok ());

        assert (chord.update (adk::TimePoint (1), input (true)).ok ());

        assert (chord.update (adk::TimePoint (2), input (false, true, true)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (chord.snapshot ().phase == adk::CueSchedulerPhase::Fault);

        chord.shutdown ();

        audit.shutdown ();

        adk::InertCueScheduler cancelled (config (), audit);

        assert (cancelled.initialize ().ok ());

        beginRun (cancelled, 100);

        assert (cancelled
                    .update (adk::TimePoint (102), input (true, true, true, true, true))
                    .    ok ());
        assert (cancelled.snapshot ().phase == adk::CueSchedulerPhase::Cancelled);
    }

    void testEvidenceGateAndCancelIdentity ()
    {
        adk::CueAuditEntry  storage[24];
        adk::CueAuditBuffer audit (storage, 24);

        adk::InertCueScheduler scheduler (config (), audit);

        assert (scheduler.initialize ().ok ());
        assert (scheduler.update (adk::TimePoint (1), input (true)).ok ());

        assert (scheduler
                    .update (adk::TimePoint (2), input (true, true),
                       gate (adk::CueEvidenceDisposition::Hold))
                    .ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Held);
        assert (!scheduler.snapshot ().hasCue);

        const uint8_t heldCount = audit.count ();

        assert (scheduler
                    .update (adk::TimePoint (2), input (true, true),
                       gate (adk::CueEvidenceDisposition::Hold))
                    .ok ());
        assert (audit.count () == heldCount);

        assert (scheduler
                    .update (adk::TimePoint (2), input (true, true),
                       gate (adk::CueEvidenceDisposition::Permit))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Fault);

        scheduler.shutdown ();
        audit.    shutdown ();

        adk::InertCueScheduler faulted (config (), audit);

        assert (faulted.initialize ().ok ());
        assert (faulted
                    .update (adk::TimePoint (1), input (true),
                       gate (adk::CueEvidenceDisposition::Fault,
                                   adk::StatusCode::InvalidConfiguration))
                    .error () == adk::StatusCode::InvalidConfiguration);
        assert (faulted.snapshot ().phase == adk::CueSchedulerPhase::Fault);

        faulted.shutdown ();
        audit.  shutdown ();

        adk::InertCueScheduler cancelled (config (), audit);

        assert (cancelled.initialize ().ok ());
        assert (cancelled.update (adk::TimePoint (1), input (true)).ok ());
        assert (
            cancelled
                .update (adk::TimePoint (2), input (true, false, false, false, true))
                .    ok ());
        assert (cancelled.update (adk::TimePoint (2), input (true)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (cancelled.snapshot ().phase == adk::CueSchedulerPhase::Cancelled);
    }

    void testActiveHoldRecordsHiddenBeforeHeld ()
    {
        adk::CueAuditEntry     storage[24];
        adk::CueAuditBuffer    audit     (storage, 24);
        adk::InertCueScheduler scheduler (config (), audit);

        assert   (scheduler.initialize ().ok ());
        beginRun (scheduler, 0);
        assert   (scheduler.update (adk::TimePoint (2), input (true, false, true)).ok ());
        assert   (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);

        const uint8_t before = audit.count ();

        assert (scheduler
                    .update (adk::TimePoint (3), input (true),
                             gate (adk::CueEvidenceDisposition::Hold))
                    .ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Held);
        assert (!scheduler.snapshot ().hasCue);
        assert (audit.count () == static_cast<uint8_t> (before + 2U));
        assert (storage[before].event == adk::CueAuditEvent::CueHidden);
        assert (storage[before].hasCue);
        assert (storage[before + 1U].event == adk::CueAuditEvent::Held);
        assert (!storage[before + 1U].hasCue);
    }

    void testRolloverAndCapacityHold ()
    {
        adk::CueAuditEntry  storage[16];
        adk::CueAuditBuffer audit (storage, 16);

        adk::InertCueScheduler scheduler (config (), audit);

        assert (scheduler.initialize ().ok ());

        beginRun (scheduler, 0xfffffffbu);

        assert (scheduler.update (adk::TimePoint (1), input (true, false, true)).ok ());

        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);

        scheduler.shutdown ();

        assert (storage[audit.count () - 1].event == adk::CueAuditEvent::Shutdown);

        adk::CueAuditEntry  smallStorage[6];
        adk::CueAuditBuffer smallAudit (smallStorage, 6);

        adk::InertCueScheduler constrained (config (), smallAudit);

        assert (constrained.initialize ().ok ());

        beginRun (constrained, 0);

        assert (constrained.update (adk::TimePoint (1), input (true, false, true))
                    .error () == adk::StatusCode::CapacityExceeded);
        assert (constrained.snapshot ().phase == adk::CueSchedulerPhase::Held);

        assert (!constrained.snapshot ().hasCue);

        constrained.shutdown ();

        assert (smallStorage[smallAudit.count () - 1].event ==
                adk::CueAuditEvent::Shutdown);
    }

    void testDeterministicReplay ()
    {
        adk::CueAuditEntry replay[2][20];
        uint8_t            counts[2] = {};

        for (uint8_t pass = 0; pass < 2; ++pass)
        {
            adk::CueAuditBuffer audit (replay[pass], 20);

            adk::InertCueScheduler scheduler (config (), audit);

            assert (scheduler.initialize ().ok ());

            beginRun (scheduler, 10);

            assert (scheduler.update (adk::TimePoint (12), input (true, false, true))
                        .ok ());
            assert (scheduler.update (adk::TimePoint (22), input (true)).ok ());

            scheduler.shutdown ();

            counts[pass] = audit.count ();
        }

        assert (counts[0] == counts[1]);

        for (uint8_t index = 0; index < counts[0]; ++index)
        {
            assert (replay[0][index].sequence == replay[1][index].sequence);

            assert (replay[0][index].recordedAt == replay[1][index].recordedAt);

            assert (replay[0][index].event == replay[1][index].event);

            assert (replay[0][index].cue == replay[1][index].cue);

            assert (replay[0][index].cueIndex == replay[1][index].cueIndex);

            assert (replay[0][index].status == replay[1][index].status);

            assert (replay[0][index].hasCue == replay[1][index].hasCue);
        }
    }
} // namespace

int main ()
{
    testLifecycleAndCopiedConfiguration ();

    testDestructorShutdown ();

    testPlanValidation ();

    testLogicalWindowAndVisibility ();

    testDelayedCoalescingAndTimeoutStatus ();

    testPriorityHoldResumeAndSameTimestamp ();

    testEvidenceGateAndCancelIdentity ();

    testActiveHoldRecordsHiddenBeforeHeld ();

    testRolloverAndCapacityHold ();

    testDeterministicReplay ();
}
