#include <magnetic_passage_logger.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>

namespace {
    namespace fake = adk::test::arduino;

    struct FakeRtc final : adk::Rtc
    {
        adk::Status initialize () noexcept override
        {
            initialized = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            initialized = false;
        }

        adk::Result<adk::ClockReading> read () noexcept override
        {
            ++reads;
            return {readStatus, reading};
        }

        adk::Status       initializeStatus = adk::StatusCode::Ok;
        adk::Status       readStatus       = adk::StatusCode::Ok;
        adk::ClockReading reading          = {1234, adk::ClockState::Valid};
        uint8_t           reads            = 0;
        bool              initialized      = false;
    };

    struct FakeLedger final : adk::PassageLedger
    {
        adk::Status initialize () noexcept override
        {
            initialized = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            initialized = false;
        }

        adk::PassageLedgerRecovery recover () noexcept override
        {
            return recovery;
        }

        adk::LedgerCommitResult commit (
            const adk::LoggedPassage& value,
            const adk::PassageCheckpoint& expected) noexcept override
        {
            ++commits;
            entry      = value;
            checkpoint = expected;

            if (!commitStatus.ok ())
            {
                return {adk::LedgerCommitDisposition::NotCommitted, expected,
                        commitStatus};
            }

            const adk::PassageCheckpoint next = {
                expected.generation + 1, value.committedCount, value.sequence};
            return {commitDisposition, next, adk::StatusCode::Ok};
        }

        adk::Status initializeStatus = adk::StatusCode::Ok;
        adk::Status commitStatus     = adk::StatusCode::Ok;
        adk::LedgerCommitDisposition commitDisposition =
            adk::LedgerCommitDisposition::Committed;
        adk::PassageLedgerRecovery recovery = {
            adk::PassageLedgerRecoveryDisposition::Empty,
            false,
            {0, 0, 0},
            false,
            {0,
             0,
             adk::TimePoint (),
             adk::PassageDirection::Unknown,
             adk::PassageLabel::None,
             {false, false, false, 0, 0, 0},
             {0, adk::ClockState::NotSet},
             false},
            adk::StatusCode::Ok};
        adk::LoggedPassage entry = recovery.entry;
        adk::PassageCheckpoint checkpoint = {0, 0, 0};
        uint8_t commits = 0;
        bool initialized = false;
    };

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::PassageRecord accepted (uint32_t sequence, int32_t delta = 4)
    {
        return {sequence,
                adk::PassageDirection::AToB,
                adk::PassageDisposition::Accepted,
                adk::TimePoint (10),
                adk::TimePoint (20 + sequence),
                adk::Duration  (10),
                adk::MagneticPolarity::Unspecified,
                adk::MagneticPolarity::Unspecified,
                {true, true, false, 2, 2 + delta, delta},
                sequence,
                0,
                adk::StatusCode::Ok};
    }

    bool samePosition (const adk::PassagePositionEvidence& left,
                       const adk::PassagePositionEvidence& right)
    {
        return left.present == right.present &&
               left.reliable == right.reliable &&
               left.saturated == right.saturated &&
               left.onsetPosition == right.onsetPosition &&
               left.endPosition == right.endPosition &&
               left.delta == right.delta;
    }

    bool sameRecord (const adk::PassageRecord& left,
                     const adk::PassageRecord& right)
    {
        return left.sequence == right.sequence &&
               left.direction == right.direction &&
               left.disposition == right.disposition &&
               left.onset == right.onset && left.end == right.end &&
               left.elapsed == right.elapsed &&
               left.onsetPolarity == right.onsetPolarity &&
               left.endPolarity == right.endPolarity &&
               samePosition (left.position, right.position) &&
               left.acceptedCount == right.acceptedCount &&
               left.suppressedCount == right.suppressedCount &&
               left.status == right.status;
    }

    bool sameEntry (const adk::LoggedPassage& left,
                    const adk::LoggedPassage& right)
    {
        return left.sequence == right.sequence &&
               left.committedCount == right.committedCount &&
               left.acceptedAt == right.acceptedAt &&
               left.direction == right.direction && left.label == right.label &&
               samePosition (left.position, right.position) &&
               left.clock.unixSeconds == right.clock.unixSeconds &&
               left.clock.state == right.clock.state &&
               left.sequenceGap == right.sequenceGap;
    }

    bool sameSnapshot (const adk::LoggerSnapshot& left,
                       const adk::LoggerSnapshot& right)
    {
        return left.selectedLabel == right.selectedLabel &&
               left.committedCount == right.committedCount &&
               left.committedSequence == right.committedSequence &&
               left.displayedCount == right.displayedCount &&
               left.displayValid == right.displayValid &&
               left.pending == right.pending && left.overrun == right.overrun &&
               sameRecord (left.pendingInput, right.pendingInput) &&
               sameRecord (left.overrunInput, right.overrunInput) &&
               left.hasFrozenEntry == right.hasFrozenEntry &&
               sameEntry (left.frozenEntry, right.frozenEntry) &&
               left.acceptedPulse == right.acceptedPulse &&
               left.committedPulse == right.committedPulse &&
               left.persistentFault == right.persistentFault &&
               left.recovery == right.recovery &&
               left.presentationStatus == right.presentationStatus &&
               left.status == right.status;
    }

    struct Fixture
    {
        Fixture ()
            : resources (),
              display (resources, {22, 23, 24},
                       adk::SevenSegmentPolarity::CommonCathode),
              countDisplay (display),
              logger       ({adk::PassageLabel::A}, rtc, ledger, countDisplay)
        {
            fake::reset ();
        }

        adk::ResourceRegistry resources;
        FakeRtc               rtc;
        FakeLedger            ledger;
        adk::SevenSegmentDisplay display;
        adk::SevenSegmentPassageCountDisplay countDisplay;
        adk::MagneticPassageLogger logger;
    };

    struct FailingInitialPresentationDisplay final
        : adk::PassageCountDisplay
    {
        adk::Status initialize () noexcept override
        {
            initialized = true;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            initialized = false;
        }

        adk::Status present (uint32_t) noexcept override
        {
            return adk::StatusCode::HardwareFailure;
        }

        bool initialized = false;
    };

    void testCommitFreezeAndPresentation ()
    {
        Fixture fixture;

        require (fixture.logger.initialize ().ok (), "logger initializes");

        const auto input = accepted (1);

        require (fixture.logger.update (input).ok (), "accepted record commits");

        const auto snapshot = fixture.logger.snapshot ();

        require (snapshot.acceptedPulse && snapshot.committedPulse,
                 "accept and commit pulses emitted");
        require (snapshot.committedCount == 1 &&
                     snapshot.committedSequence == 1,
                 "logical checkpoint advances");
        require (snapshot.displayValid && snapshot.displayedCount == 1,
                 "durable count presented");
        require (fixture.ledger.entry.label == adk::PassageLabel::A &&
                     fixture.ledger.entry.clock.unixSeconds == 1234,
                 "label and clock frozen");

        require (fixture.logger.cycleLabel ().ok (), "label cycles");
        require (!fixture.logger.snapshot ().acceptedPulse &&
                     !fixture.logger.snapshot ().committedPulse,
                 "next call clears pulses");

        require (fixture.logger.update (input).ok (),
                 "same committed sequence is idempotent");
        require (fixture.ledger.commits == 1 && fixture.rtc.reads == 1,
                 "idempotence performs no I/O");
    }

    void testRtcAndLedgerRetry ()
    {
        Fixture fixture;

        require (fixture.logger.initialize ().ok (), "retry logger initializes");

        fixture.rtc.readStatus = adk::StatusCode::HardwareFailure;
        const auto input       = accepted (1);

        require (fixture.logger.update (input).error () ==
                     adk::StatusCode::HardwareFailure,
                 "RTC failure leaves input pending");
        require (fixture.logger.snapshot ().pending &&
                     !fixture.logger.snapshot ().hasFrozenEntry,
                 "failed clock is not read");

        fixture.rtc.readStatus    = adk::StatusCode::Ok;
        fixture.ledger.commitStatus = adk::StatusCode::HardwareFailure;

        require (fixture.logger.update (input).error () ==
                     adk::StatusCode::HardwareFailure,
                 "ledger failure retains frozen entry");
        const auto frozen = fixture.logger.snapshot ().frozenEntry;

        fixture.rtc.reading.unixSeconds = 9999;
        fixture.ledger.commitStatus     = adk::StatusCode::Ok;

        require (fixture.logger.update (input).ok (), "ledger retry commits");
        require (fixture.rtc.reads == 2 &&
                     fixture.ledger.entry.clock.unixSeconds ==
                         frozen.clock.unixSeconds,
                 "ledger retry does not reread RTC");
        require (fixture.logger.snapshot ().persistentFault,
                 "transient failures leave persistent evidence");
    }

    void testOverrunRejectionAndLabelFreeze ()
    {
        Fixture fixture;

        require (fixture.logger.initialize ().ok (), "overrun initializes");

        fixture.rtc.readStatus = adk::StatusCode::HardwareFailure;

        require (!fixture.logger.update (accepted (1)).ok (),
                 "first input becomes pending");
        require (fixture.logger.cycleLabel ().ok (), "selection can change");
        require (fixture.logger.update (accepted (2)).error () ==
                     adk::StatusCode::CapacityExceeded,
                 "newer pending input becomes bounded overrun");
        require (fixture.logger.snapshot ().overrun &&
                     fixture.logger.snapshot ().overrunInput.sequence == 2,
                 "overrun evidence retained");

        fixture.rtc.readStatus = adk::StatusCode::Ok;

        require (fixture.logger.update (accepted (1)).ok (),
                 "original pending input commits");
        require (fixture.ledger.entry.label == adk::PassageLabel::A,
                 "pending label remains frozen");

        auto rejected        = accepted (3);
        rejected.disposition = adk::PassageDisposition::TimedOut;

        require (fixture.logger.update (rejected).error () ==
                     adk::StatusCode::InvalidArgument,
                 "nonaccepted disposition rejected");
    }

    void testRecoveryAndMismatch ()
    {
        Fixture fixture;
        const auto prior = accepted (7);

        fixture.ledger.recovery = {
            adk::PassageLedgerRecoveryDisposition::Recovered,
            true,
            {4, 6, 7},
            true,
            {7,
             6,
             prior.end,
             prior.direction,
             adk::PassageLabel::C,
             prior.position,
             {777, adk::ClockState::Valid},
             true},
            adk::StatusCode::Ok};

        require (fixture.logger.initialize ().ok (), "recovery initializes");
        require (fixture.logger.snapshot ().committedCount == 6,
                 "checkpoint restored");
        require (fixture.logger.update (prior).ok (),
                 "matching recovered sequence is idempotent");

        const auto mismatch = accepted (7, -2);

        require (fixture.logger.update (mismatch).error () ==
                     adk::StatusCode::InternalInvariant,
                 "same-sequence disagreement faults");
        require (fixture.logger.snapshot ().persistentFault,
                 "mismatch sets persistent fault");
    }

    void testDirectionsLabelsClocksPositionsAndGap ()
    {
        for (uint8_t label = 0;
             label <= static_cast<uint8_t> (adk::PassageLabel::C); ++label)
        {
            Fixture fixture;

            require (fixture.logger.initialize ().ok (),
                     "variant logger initializes");

            const uint8_t cycleCount =
                static_cast<uint8_t> ((label + 3U) % 4U);

            for (uint8_t step = 0; step < cycleCount; ++step)
            {
                require (fixture.logger.cycleLabel ().ok (),
                         "variant label cycles");
            }

            fixture.rtc.reading = {
                static_cast<uint32_t> (2000U + label),
                static_cast<adk::ClockState> (label)};
            auto input = accepted (3);
            input.direction =
                (label & 1U) == 0 ? adk::PassageDirection::AToB
                                  : adk::PassageDirection::BToA;
            input.position = label == 0
                                 ? adk::PassagePositionEvidence{
                                       false, false, false, 0, 0, 0}
                                 : adk::PassagePositionEvidence{
                                       true, false, false, 9, 9, 0};

            require (fixture.logger.update (input).ok (),
                     "variant input commits");
            require (fixture.ledger.entry.label ==
                         static_cast<adk::PassageLabel> (label) &&
                         fixture.ledger.entry.clock.state ==
                             static_cast<adk::ClockState> (label) &&
                         fixture.ledger.entry.direction == input.direction &&
                         fixture.ledger.entry.position.present ==
                             input.position.present &&
                         fixture.ledger.entry.sequenceGap,
                     "public variant fields freeze deterministically");
        }
    }

    void testSequenceReconciliationAndOverrunReplacement ()
    {
        Fixture fixture;

        require (fixture.logger.initialize ().ok (),
                 "sequence logger initializes");

        fixture.ledger.commitDisposition =
            adk::LedgerCommitDisposition::CommittedAfterReconciliation;

        require (fixture.logger.update (accepted (2)).ok (),
                 "reconciled commit advances state");
        require (fixture.logger.snapshot ().committedSequence == 2,
                 "reconciled sequence is durable");
        require (fixture.logger.update (accepted (1)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "older sequence is rejected");

        Fixture pending;

        require (pending.logger.initialize ().ok (),
                 "replacement logger initializes");

        pending.rtc.readStatus = adk::StatusCode::HardwareFailure;

        require (!pending.logger.update (accepted (1)).ok (),
                 "replacement source is pending");
        require (pending.logger.update (accepted (2)).error () ==
                     adk::StatusCode::CapacityExceeded,
                 "first overrun is bounded");
        require (pending.logger.update (accepted (5)).error () ==
                     adk::StatusCode::CapacityExceeded &&
                     pending.logger.snapshot ().overrunInput.sequence == 5,
                 "later overrun replaces prior evidence");
    }

    void testRollbackPersistentFaultAndReplay ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner (resources, 22);
        FakeRtc               rtc;
        FakeLedger            ledger;
        adk::SevenSegmentDisplay display (
            resources, {22, 23, 24},
            adk::SevenSegmentPolarity::CommonCathode);
        adk::SevenSegmentPassageCountDisplay countDisplay (display);

        adk::MagneticPassageLogger logger (
            {adk::PassageLabel::None}, rtc, ledger, countDisplay);

        require (owner.initialize ().ok (), "display conflict is acquired");
        require (logger.initialize ().error () ==
                     adk::StatusCode::ResourceBusy,
                 "display initialization failure is returned");
        require (!rtc.initialized && !ledger.initialized &&
                     !logger.initialized (),
                 "display failure rolls back RTC and ledger");

        owner.shutdown ();

        rtc.readStatus = adk::StatusCode::HardwareFailure;

        require (logger.initialize ().ok (), "logger reinitializes");
        require (!logger.update (accepted (1)).ok (),
                 "runtime failure establishes persistent fault");

        logger.shutdown ();

        ledger.initializeStatus = adk::StatusCode::HardwareFailure;

        require (!logger.initialize ().ok () &&
                     logger.snapshot ().persistentFault,
                 "failed reinitialize preserves persistent fault");

        ledger.initializeStatus = adk::StatusCode::Ok;
        rtc.readStatus          = adk::StatusCode::Ok;

        require (logger.initialize ().ok () &&
                     !logger.snapshot ().persistentFault,
                 "successful reinitialize clears persistent fault");

        fake::clearTrace ();

        require (logger.update (accepted (1)).ok (), "first replay commits");

        const auto firstSnapshot = logger.snapshot ();

        const auto firstTrace    = fake::trace ();

        logger.shutdown ();

        FakeRtc replayRtc;
        FakeLedger replayLedger;
        adk::SevenSegmentDisplay replayDisplay (
            resources, {22, 23, 24},
            adk::SevenSegmentPolarity::CommonCathode);
        adk::SevenSegmentPassageCountDisplay replayCountDisplay (
            replayDisplay);

        adk::MagneticPassageLogger replay (
            {adk::PassageLabel::None}, replayRtc, replayLedger,
            replayCountDisplay);

        require (replay.initialize ().ok (), "replay logger initializes");

        fake::clearTrace ();

        require (replay.update (accepted (1)).ok (), "second replay commits");

        const auto replaySnapshot = replay.snapshot ();

        const auto replayTrace    = fake::trace ();

        require (sameSnapshot (firstSnapshot, replaySnapshot) &&
                     firstTrace.size () == replayTrace.size (),
                 "public state and operation trace replay deterministically");

        for (size_t index = 0; index < firstTrace.size (); ++index)
        {
            require (firstTrace[index].kind == replayTrace[index].kind &&
                         firstTrace[index].pin == replayTrace[index].pin &&
                         firstTrace[index].value == replayTrace[index].value &&
                         firstTrace[index].timeUs == replayTrace[index].timeUs,
                     "each replay operation is identical");
        }
    }

    void testInitializationRollbackAndPresentationAfterDurability ()
    {
        {
            Fixture fixture;
            fixture.ledger.initializeStatus =
                adk::StatusCode::HardwareFailure;

            require (fixture.logger.initialize ().error () ==
                         adk::StatusCode::HardwareFailure &&
                         !fixture.rtc.initialized &&
                         !fixture.ledger.initialized,
                     "ledger initialization failure touches no dependency");
        }

        {
            Fixture fixture;
            fixture.rtc.initializeStatus = adk::StatusCode::HardwareFailure;

            require (fixture.logger.initialize ().error () ==
                         adk::StatusCode::HardwareFailure &&
                         !fixture.rtc.initialized &&
                         !fixture.ledger.initialized,
                     "RTC initialization failure rolls ledger back");
        }

        {
            fake::reset ();

            adk::ResourceRegistry resources;
            FakeRtc               rtc;
            FakeLedger            ledger;
            adk::SevenSegmentDisplay display (
                resources, {22, 23, 24},
                adk::SevenSegmentPolarity::CommonCathode);
            adk::SevenSegmentPassageCountDisplay countDisplay (display);

            adk::MagneticPassageLogger logger (
                {static_cast<adk::PassageLabel> (255)}, rtc, ledger,
                countDisplay);

            require (logger.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration &&
                         !rtc.initialized && !ledger.initialized,
                     "invalid label is rejected before dependencies");
        }

        {
            Fixture fixture;

            require (fixture.logger.initialize ().ok (),
                     "presentation fixture initializes");

            fixture.display.shutdown ();

            require (fixture.logger.update (accepted (1)).error () ==
                         adk::StatusCode::NotInitialized,
                     "presentation failure is returned after durable commit");
            const auto failedPresentation = fixture.logger.snapshot ();

            require (fixture.ledger.commits == 1 &&
                         failedPresentation.committedCount == 1 &&
                         !failedPresentation.displayValid &&
                         failedPresentation.persistentFault,
                     "presentation cannot roll durable state back or retry it");

            require (fixture.logger.update (accepted (1)).ok (),
                     "same durable input remains idempotent");

            require (fixture.ledger.commits == 1,
                     "presentation failure causes no ledger retry");
        }
    }

    void testInitializePresentationFailureRollback ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        FakeRtc               rtc;
        FakeLedger            ledger;
        FailingInitialPresentationDisplay display;

        adk::MagneticPassageLogger logger (
            {adk::PassageLabel::None}, rtc, ledger, display);

        require (logger.initialize ().error () ==
                     adk::StatusCode::HardwareFailure,
                 "initial presentation failure is returned");
        require (!logger.initialized () && !display.initialized &&
                     !rtc.initialized && !ledger.initialized,
                 "initial presentation failure reverses every dependency");
        require (logger.snapshot ().persistentFault,
                 "initial presentation failure persists");
    }
} // namespace

int main ()
{
    testCommitFreezeAndPresentation ();

    testRtcAndLedgerRetry ();

    testOverrunRejectionAndLabelFreeze ();

    testRecoveryAndMismatch ();

    testDirectionsLabelsClocksPositionsAndGap ();

    testSequenceReconciliationAndOverrunReplacement ();

    testRollbackPersistentFaultAndReplay ();

    testInitializationRollbackAndPresentationAfterDurability ();

    testInitializePresentationFailureRollback ();
    std::cout << "magnetic passage logger tests passed\n";
    return EXIT_SUCCESS;
}
