#include <passage_qualifier.h>

#include <cstdlib>
#include <iostream>
#include <limits>

// clang-format off
namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';

            std::exit (EXIT_FAILURE);
        }
    }

    adk::MagneticObservation observation (uint32_t milliseconds, bool active)
    {
        return {adk::MagneticSource::ContactDigital,
                static_cast<uint16_t> (active ? 1 : 0),
                active ? adk::Level::High : adk::Level::Low,
                adk::TimePoint (milliseconds),
                adk::MagneticPolarity::Unspecified,
                false,
                false,
                active,
                adk::Duration (),
                adk::MagneticQuality::Valid,
                adk::StatusCode::Ok};
    }

    adk::PassageInput input (uint32_t milliseconds, bool activeA, bool activeB,
                             bool hasPosition = false, int32_t position = 0,
                             adk::Status positionStatus = adk::StatusCode::Ok)
    {
        return {adk::TimePoint (milliseconds),
                observation (milliseconds, activeA),
                observation (milliseconds, activeB),
                hasPosition,
                position,
                positionStatus};
    }

    adk::PassageQualifier qualifier ()
    {
        return adk::PassageQualifier (
            {adk::Duration (5), adk::Duration (20), adk::Duration (10)});
    }

    void testLifecycleAndDirections ()
    {
        auto passage = qualifier ();


        passage.update (input (0, false, false));

        require (passage.snapshot ().status.error () == adk::StatusCode::NotInitialized,
                 "update before initialize rejected");

        require (passage.initialize ().ok (), "initialize succeeds");

        require (passage.initialize ().ok (), "initialize is idempotent");


        passage.update (input (0, true, false, true, 10));

        passage.update (input (5, true, false, true, 10));

        require (passage.snapshot ().phase == adk::PassagePhase::AwaitingSecond,
                 "A qualifies after exact dwell");

        passage.update (input (6, true, true, true, 12));

        passage.update (input (11, true, true, true, 20));


        const auto accepted = passage.snapshot ();

        require (accepted.hasRecord, "accepted passage emits record");

        require (accepted.record.direction == adk::PassageDirection::AToB,
                 "A to B direction");

        require (accepted.record.disposition == adk::PassageDisposition::Accepted,
                 "accepted disposition");

        require (accepted.acceptedCount == 1, "accept increments count");

        require (accepted.record.position.present &&
                     accepted.record.position.reliable &&
                     accepted.record.position.delta == 10,
                 "agreeing position copied");


        passage.update (input (11, true, true, true, 20));

        require (passage.snapshot ().hasRecord &&
                     passage.snapshot ().record.sequence == accepted.record.sequence,
                 "identical same-time terminal frame preserves record");


        passage.update (input (12, true, true));

        require (!passage.snapshot ().hasRecord, "record lasts exactly one snapshot");

        passage.update (input (13, false, false));

        passage.update (input (20, false, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Suppressing,
                 "duplicate window remains active before exact edge");

        passage.update (input (21, false, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Idle,
                 "rearms at exact expiry with both inactive");


        passage.update (input (30, false, true, true, 20));

        passage.update (input (35, false, true, true, 20));

        passage.update (input (36, true, true, true, 15));

        passage.update (input (41, true, true, true, 10));

        require (passage.snapshot ().record.direction == adk::PassageDirection::BToA,
                 "B to A direction");

        require (passage.snapshot ().record.position.reliable,
                 "negative position agrees with B to A");
    }

    void testTimeoutAmbiguityRetreatAndSuppression ()
    {
        auto passage = qualifier ();

        require (passage.initialize ().ok (), "policy initialize");


        passage.update (input (0, true, true));

        passage.update (input (5, true, true));

        require (passage.snapshot ().record.disposition ==
                     adk::PassageDisposition::Ambiguous,
                 "simultaneous dwell is ambiguous");


        passage.reset ();

        passage.update (input (0, true, false));

        passage.update (input (5, true, false));

        passage.update (input (26, true, true));

        require (passage.snapshot ().record.disposition ==
                     adk::PassageDisposition::TimedOut,
                 "late opposite boundary times out before completion");


        passage.reset ();

        passage.update (input (0, true, false));

        passage.update (input (5, true, false));

        passage.update (input (6, false, true));

        passage.update (input (11, false, true));

        require (!passage.snapshot ().hasRecord &&
                     passage.snapshot ().phase == adk::PassagePhase::AwaitingSecond,
                 "retreat cannot reverse into acceptance");

        passage.update (input (12, false, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Idle,
                 "retreat rearms only when both inactive");


        passage.update (input (20, true, false));

        passage.update (input (25, true, false));

        passage.update (input (26, true, true));

        passage.update (input (31, true, true));

        passage.update (input (32, false, false));

        passage.update (input (33, true, false));

        require (passage.snapshot ().record.disposition ==
                     adk::PassageDisposition::DuplicateSuppressed,
                 "activation in duplicate window emits suppressed record");

        require (passage.snapshot ().suppressedCount == 1,
                 "suppressed activation counted once");

        passage.update (input (34, true, false));

        require (!passage.snapshot ().hasRecord &&
                     passage.snapshot ().suppressedCount == 1,
                 "held duplicate is not recounted");

        passage.update (input (35, false, false));

        passage.update (input (41, false, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Idle,
                 "duplicate evidence does not restart suppression window");
    }

    void testFaultsTimeAndRecovery ()
    {
        auto passage = qualifier ();

        require (passage.initialize ().ok (), "fault initialize");


        passage.update (input (10, true, false));

        passage.update (input (15, true, false));


        auto mismatch                 = input (16, true, false);

        mismatch.boundaryB.observedAt = adk::TimePoint (15);

        passage.update (mismatch);

        require (passage.snapshot ().record.disposition ==
                     adk::PassageDisposition::EvidenceFault,
                 "timestamp mismatch faults active candidate");

        require (passage.snapshot ().phase == adk::PassagePhase::Fault,
                 "fault phase entered");


        passage.update (input (17, true, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Fault,
                 "active healthy frame does not recover");

        passage.update (input (18, false, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Idle &&
                     !passage.snapshot ().hasRecord,
                 "later healthy inactive frame recovers silently");


        passage.update (input (19, false, false));

        const auto stable = passage.snapshot ();

        passage.update (input (19, false, false));

        require (passage.snapshot ().phase == stable.phase &&
                     !passage.snapshot ().hasRecord,
                 "identical same-time frame is idempotent");


        passage.update (input (19, true, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Fault,
                 "changed same-time frame faults");


        passage.reset ();

        passage.update (input (0, false, false, true, 4));

        auto changedPosition = input (0, false, false, true, 5);

        passage.update (changedPosition);

        require (passage.snapshot ().phase == adk::PassagePhase::Fault,
                 "changed same-time position faults");


        passage.reset ();

        passage.update (input (0xfffffffcu, true, false));

        passage.update (input (1, true, false));

        require (passage.snapshot ().phase == adk::PassagePhase::AwaitingSecond,
                 "dwell crosses rollover");

        passage.update (input (0x80000001u, true, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Fault,
                 "exact half-range jump faults");
    }

    void testResetFaultMetadataAndHealthyRearm ()
    {
        auto passage = qualifier ();

        require (passage.initialize ().ok (), "metadata initialize");

        passage.update (input (10, true, false, true, 10));

        passage.update (input (15, true, false, true, 10));

        passage.update (input (16, true, true, true, 12));

        passage.update (input (21, true, true, true, 20));

        passage.reset ();


        passage.update (input (100, true, false, true, 30));

        auto faulty              = input (101, true, false, true, 31);
        faulty.boundaryA.quality = adk::MagneticQuality::Unqualified;

        passage.update (faulty);

        require (passage.snapshot ().hasRecord &&
                     passage.snapshot ().record.disposition ==
                         adk::PassageDisposition::EvidenceFault &&
                     passage.snapshot ().record.onset == adk::TimePoint (100) &&
                     passage.snapshot ().record.position.onsetPosition == 0,
                 "pre-dwell fault uses current canonical metadata after reset");


        passage.reset ();

        passage.update (input (0, true, false));

        passage.update (input (5, true, false));

        passage.update (input (26, true, false));

        require (passage.snapshot ().status.error () == adk::StatusCode::Timeout,
                 "timeout status accompanies terminal record");

        passage.update (input (27, false, false));

        passage.update (input (36, false, false));

        require (passage.snapshot ().phase == adk::PassagePhase::Idle &&
                     passage.snapshot ().status.ok (),
                 "healthy suppression rearm restores ok status");
    }

    void testPositionAndConfiguration ()
    {
        adk::PassageQualifier invalid (
            {adk::Duration (), adk::Duration (20), adk::Duration (10)});

        require (invalid.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "zero duration rejected");


        adk::PassageQualifier shortTimeout (
            {adk::Duration (5), adk::Duration (4), adk::Duration (10)});

        require (shortTimeout.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "timeout below dwell rejected");


        auto passage = qualifier ();

        require (passage.initialize ().ok (), "position initialize");

        passage.update (
            input (0, true, false, true, std::numeric_limits<int32_t>::min ()));

        passage.update (
            input (5, true, false, true, std::numeric_limits<int32_t>::min ()));

        passage.update (
            input (6, true, true, true, std::numeric_limits<int32_t>::max ()));

        passage.update (
            input (11, true, true, true, std::numeric_limits<int32_t>::max ()));

        require (passage.snapshot ().record.position.saturated &&
                     !passage.snapshot ().record.position.reliable,
                 "widened delta clamps and marks saturation");


        passage.reset ();

        passage.update (input (0, true, false));

        passage.update (input (5, true, false));

        passage.update (input (6, true, true));

        passage.update (input (11, true, true));

        require (!passage.snapshot ().record.position.present &&
                     passage.snapshot ().record.position.delta == 0,
                 "missing position is canonical");


        passage.reset ();

        passage.update (input (0, true, false));

        passage.update (input (5, true, false));

        passage.update (input (6, true, true, true, 4));

        passage.update (input (11, true, true, true, 10));

        require (!passage.snapshot ().record.position.present &&
                     !passage.snapshot ().record.position.reliable,
                 "missing onset position remains canonical");


        passage.reset ();

        passage.update (
            input (0, true, false, true, 4, adk::StatusCode::HardwareFailure));

        passage.update (
            input (5, true, false, true, 4, adk::StatusCode::HardwareFailure));

        passage.update (input (6, true, true, true, 6));

        passage.update (input (11, true, true, true, 10));

        require ( passage.snapshot ().record.position.present &&
                  passage.snapshot ().record.position.onsetPosition == 4 &&
                  passage.snapshot ().record.position.endPosition == 10 &&
                 !passage.snapshot ().record.position.reliable,
                 "onset position source fault retains values but is unreliable");


        passage.reset ();

        passage.update (input (0, true, false, true, 4));

        passage.update (input (5, true, false, true, 4));

        passage.update (input (6, true, true, true, 6));

        passage.update (
            input (11, true, true, true, 10, adk::StatusCode::HardwareFailure));

        require ( passage.snapshot ().record.position.present &&
                  passage.snapshot ().record.position.onsetPosition == 4 &&
                  passage.snapshot ().record.position.endPosition == 10 &&
                 !passage.snapshot ().record.position.reliable,
                 "end position source fault retains values but is unreliable");
    }
} // namespace

int main ()
{
    testLifecycleAndDirections ();

    testTimeoutAmbiguityRetreatAndSuppression ();

    testFaultsTimeAndRecovery ();

    testResetFaultMetadataAndHealthyRearm ();

    testPositionAndConfiguration ();
    std::cout << "passage qualifier tests passed\n";
    return EXIT_SUCCESS;
}
// clang-format on
