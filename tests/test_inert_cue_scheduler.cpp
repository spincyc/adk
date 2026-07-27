#include <assert.h>
#include <stdint.h>
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

    adk::InertCuePlan plan () noexcept
    {
        adk::InertCuePlan result;

        result.cues[0] = {3, adk::Duration (0), adk::Duration (10)};
        result.cues[1] = {7, adk::Duration (20), adk::Duration (10)};
        result.count   = 2;
        return result;
    }

    void beginFirstConfirmation (adk::InertCueScheduler& scheduler,
                                 uint32_t                start) noexcept
    {
        assert (scheduler.update (adk::TimePoint (start), input (true)).ok ());
        assert (
            scheduler.update (adk::TimePoint (start + 1), input (true, true)).ok ());
        assert (scheduler.update (adk::TimePoint (start + 2), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Confirmation);
    }

    void testConfigurationAndCopiedPlan ()
    {
        adk::CueAuditEntry  storage[16];
        adk::CueAuditBuffer audit         (storage, 16);
        adk::InertCuePlan   source = plan ();

        assert (audit.initialize ().ok ());

        adk::InertCueScheduler scheduler (source, adk::Duration (5), audit);

        source.cues[0].id = 31;
        assert                 (scheduler.initialize ().ok ());
        assert                 (scheduler.initialize ().ok ());
        beginFirstConfirmation (scheduler, 0);
        assert                 (scheduler.snapshot ().cue == 3);
        scheduler.shutdown     ();
        scheduler.shutdown     ();
        assert                 (!scheduler.initialized ());
        assert                 (scheduler.update (adk::TimePoint (), input ()).error () ==
                adk::StatusCode::NotInitialized);
        assert (storage[audit.count () - 1].event == adk::CueAuditEvent::Shutdown);

        adk::CueAuditEntry     otherStorage[16];
        adk::CueAuditBuffer    inactiveAudit (otherStorage, 16);
        adk::InertCueScheduler missingAudit  (source, adk::Duration (5), inactiveAudit);

        assert (missingAudit.initialize ().error () == adk::StatusCode::NotInitialized);
    }

    void testInvalidPlans ()
    {
        for (uint8_t variant = 0; variant < 8; ++variant)
        {
            adk::CueAuditEntry  storage[16];
            adk::CueAuditBuffer audit          (storage, 16);
            adk::InertCuePlan   invalid = plan ();

            switch (variant)
            {
                case 0: invalid.count = 0; break;
                case 1: invalid.count = 33; break;
                case 2: invalid.cues[0].offset = adk::Duration     (1); break;
                case 3: invalid.cues[0].visibleFor = adk::Duration (0); break;
                case 4: invalid.cues[0].id = 32; break;
                case 5: invalid.cues[1].offset = adk::Duration (9); break;
                case 6:
                    invalid.cues[1].offset     = adk::Duration (0x7fffffffu);
                    invalid.cues[1].visibleFor = adk::Duration (1);
                    break;
                case 7: invalid.cues[1].offset = adk::Duration (0); break;
            }

            assert (audit.initialize ().ok ());

            adk::InertCueScheduler scheduler (invalid, adk::Duration (5), audit);

            assert (scheduler.initialize ().error () ==
                    adk::StatusCode::InvalidArgument);
        }

        adk::CueAuditEntry     storage[16];
        adk::CueAuditBuffer    audit           (storage, 16);
        adk::InertCueScheduler zeroWindow      (plan (), adk::Duration (0), audit);
        adk::InertCueScheduler ambiguousWindow (plan (), adk::Duration (0x80000000u),
                                                audit);

        assert (audit.initialize ().ok ());
        assert (zeroWindow.initialize ().error () == adk::StatusCode::InvalidArgument);
        assert (ambiguousWindow.initialize ().error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testHappyPathAndExactWindow ()
    {
        adk::CueAuditEntry     storage[24];
        adk::CueAuditBuffer    audit     (storage, 24);
        adk::InertCueScheduler scheduler (plan (), adk::Duration (5), audit);

        assert                 (audit.initialize ().ok ());
        assert                 (scheduler.initialize ().ok ());
        beginFirstConfirmation (scheduler, 100);

        assert (
            scheduler.update (adk::TimePoint (107), input (true, false, true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);
        assert (scheduler.snapshot ().cueElapsed == adk::Duration (0));
        assert (scheduler.update (adk::TimePoint (116), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);
        assert (scheduler.update (adk::TimePoint (117), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Waiting);
        assert (scheduler.snapshot ().cue == 7);
        assert (scheduler.update (adk::TimePoint (126), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Waiting);
        assert (scheduler.update (adk::TimePoint (127), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Confirmation);
        assert (
            scheduler.update (adk::TimePoint (132), input (true, false, true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Active);
        assert (scheduler.update (adk::TimePoint (137), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Complete);

        bool sawShown = false;

        for (uint8_t index = 0; index < audit.count (); ++index)
        {
            sawShown = sawShown || storage[index].event == adk::CueAuditEvent::CueShown;
            assert (storage[index].sequence == index);
        }

        assert (sawShown);
    }

    void testOutsideWindowSkipAndNoCatchUp ()
    {
        adk::CueAuditEntry     storage[24];
        adk::CueAuditBuffer    audit     (storage, 24);
        adk::InertCueScheduler scheduler (plan (), adk::Duration (5), audit);

        assert                 (audit.initialize ().ok ());
        assert                 (scheduler.initialize ().ok ());
        beginFirstConfirmation (scheduler, 0);
        assert                 (scheduler.update (adk::TimePoint (8), input (true)).ok ());
        assert                 (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Complete ||
                scheduler.snapshot ().phase == adk::CueSchedulerPhase::Waiting);

        scheduler.shutdown     ();
        audit.shutdown         ();
        assert                 (audit.initialize ().ok ());
        assert                 (scheduler.initialize ().ok ());
        beginFirstConfirmation (scheduler, 100);
        assert                 (
            scheduler.update (adk::TimePoint (103), input (true, false, true)).ok ());
        assert (scheduler.update (adk::TimePoint (113), input (true)).ok ());
        assert (scheduler.update (adk::TimePoint (200), input (true)).ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Complete);
        assert (scheduler.snapshot ().decision == adk::CueDecision::Complete);
    }

    void testHoldResumeAndStopDominance ()
    {
        adk::CueAuditEntry     storage[24];
        adk::CueAuditBuffer    audit     (storage, 24);
        adk::InertCueScheduler scheduler (plan (), adk::Duration (5), audit);

        assert                 (audit.initialize ().ok ());
        assert                 (scheduler.initialize ().ok ());
        beginFirstConfirmation (scheduler, 0);
        assert                 (scheduler.update (adk::TimePoint (3), input ()).ok ());
        assert                 (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Held);
        assert                 (scheduler.update (adk::TimePoint (4), input (true)).ok ());
        assert                 (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Review);
        assert                 (
            scheduler.update (adk::TimePoint (5), input (true, true, true, true, true))
                .ok ());
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Cancelled);
        assert (storage[audit.count () - 1].event == adk::CueAuditEvent::Cancelled);
    }

    void testFaultsAndRollover ()
    {
        adk::CueAuditEntry     storage[24];
        adk::CueAuditBuffer    audit (storage, 24);
        adk::InertCueScheduler chord (plan (), adk::Duration (5), audit);

        assert (audit.initialize ().ok ());
        assert (chord.initialize ().ok ());
        assert (chord.update (adk::TimePoint (1), input (true, true, true)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (chord.snapshot ().phase == adk::CueSchedulerPhase::Fault);

        chord.shutdown ();
        audit.shutdown ();
        assert         (audit.initialize ().ok ());

        adk::InertCueScheduler rollover (plan (), adk::Duration (5), audit);

        assert                 (rollover.initialize ().ok ());
        beginFirstConfirmation (rollover, 0xfffffffbu);
        assert                 (rollover.update (adk::TimePoint (2), input (true, false, true)).ok ());
        assert                 (rollover.snapshot ().phase == adk::CueSchedulerPhase::Active);
        assert                 (rollover.update (adk::TimePoint (12), input (true)).ok ());
        assert                 (rollover.snapshot ().phase == adk::CueSchedulerPhase::Waiting);
        assert                 (rollover.update (adk::TimePoint (0xfffffff0u), input (true)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (rollover.snapshot ().phase == adk::CueSchedulerPhase::Fault);
    }

    void testAuditExhaustionPreservesShutdownSlot ()
    {
        adk::CueAuditEntry     storage[6];
        adk::CueAuditBuffer    audit     (storage, 6);
        adk::InertCueScheduler scheduler (plan (), adk::Duration (5), audit);

        assert (audit.initialize ().ok ());
        assert (scheduler.initialize ().ok ());
        assert (scheduler.update (adk::TimePoint (0), input (true)).ok ());
        assert (scheduler.update (adk::TimePoint (1), input (true, true)).ok ());
        assert (scheduler.update (adk::TimePoint (2), input (true)).ok ());
        assert (
            scheduler.update (adk::TimePoint (3), input (true, false, true)).error () ==
            adk::StatusCode::CapacityExceeded);
        assert (scheduler.snapshot ().phase == adk::CueSchedulerPhase::Held);
        assert (scheduler.snapshot ().status.error () ==
                adk::StatusCode::CapacityExceeded);
        assert             (storage[audit.count () - 1].event == adk::CueAuditEvent::Held);
        scheduler.shutdown ();
        assert             (audit.count () == 6);
        assert             (storage[5].event == adk::CueAuditEvent::Shutdown);
    }

    void testDeterministicReplay ()
    {
        adk::CueAuditEntry replay[2][24];
        uint8_t            counts[2] = {};

        for (uint8_t pass = 0; pass < 2; ++pass)
        {
            adk::CueAuditBuffer    audit     (replay[pass], 24);
            adk::InertCueScheduler scheduler (plan (), adk::Duration (5), audit);

            assert                 (audit.initialize ().ok ());
            assert                 (scheduler.initialize ().ok ());
            beginFirstConfirmation (scheduler, 10);
            assert                 (scheduler.update (adk::TimePoint (13), input (true, false, true))
                        .ok ());
            assert                     (scheduler.update (adk::TimePoint (23), input (true)).ok ());
            scheduler.shutdown         ();
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
        }
    }
} // namespace

int main ()
{
    testConfigurationAndCopiedPlan           ();
    testInvalidPlans                         ();
    testHappyPathAndExactWindow              ();
    testOutsideWindowSkipAndNoCatchUp        ();
    testHoldResumeAndStopDominance           ();
    testFaultsAndRollover                    ();
    testAuditExhaustionPreservesShutdownSlot ();
    testDeterministicReplay                  ();
}
