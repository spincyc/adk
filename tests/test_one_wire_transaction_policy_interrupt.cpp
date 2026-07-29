#include <one_wire_transaction_policy.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::OneWireRomCode rom ()
    {
        return {{0x28, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}};
    }

    adk::OneWireTransactionConfig config ()
    {
        return {0x12345678UL,
                11,
                7,
                17,
                true,
                adk::MicrosecondDuration (480),
                adk::MicrosecondDuration (960),
                adk::MicrosecondDuration (15),
                adk::MicrosecondDuration (60),
                adk::MicrosecondDuration (15),
                adk::MicrosecondDuration (60),
                adk::MicrosecondDuration (60),
                adk::MicrosecondDuration (240),
                adk::MicrosecondDuration (60),
                adk::MicrosecondDuration (120),
                adk::MicrosecondDuration (1),
                adk::MicrosecondDuration (15),
                adk::MicrosecondDuration (1),
                adk::MicrosecondDuration (15),
                adk::MicrosecondDuration (15),
                adk::MicrosecondDuration (45),
                adk::MicrosecondDuration (60),
                adk::MicrosecondDuration (120),
                adk::MicrosecondDuration (1),
                adk::MicrosecondDuration (20),
                adk::MicrosecondDuration (20000),
                256};
    }

    adk::OneWireOperationRequest request (uint32_t sequence, uint32_t now)
    {
        return {sequence,
                adk::OneWireOperation::ReadRomSingleDrop,
                rom (),

                {rom (), 0, false},

                adk::MicrosecondTimePoint (now),
                adk::OneWireSupplyMode::ExternallyPowered,
                adk::StatusCode::Ok};
    }

    adk::OneWireStepIntent emptyIntent ()
    {
        return {0,
                0,
                0,
                0,
                0,
                adk::OneWireOperation::ResetPresence,
                adk::OneWirePhase::Inert,
                0,
                0,
                false,
                adk::OneWireLineIntent::Release,
                false,
                adk::MicrosecondTimePoint (),
                adk::MicrosecondTimePoint (),
                {{0, 0, 0, 0, 0, 0, 0, 0}}};
    }

    adk::OneWireStepReceipt receipt (const adk::OneWireStepIntent& intent,
                                     uint32_t sequence, bool sampledHigh = true,
                                     bool        accepted = true,
                                     adk::Status status   = adk::StatusCode::Ok)
    {
        return {7,
                17,
                sequence,
                intent.earliestAt,
                intent.ownerToken,
                intent.lifecycleGeneration,
                intent.requestSequence,
                intent.transactionGeneration,
                intent.operation,
                intent.phase,
                intent.phaseSequence,
                intent.slotIndex,
                intent.lineIntent,
                sampledHigh,
                accepted,
                status};
    }

    adk::OneWireTransactionSnapshot
    snapshot (const adk::OneWireTransactionPolicy& policy)
    {
        adk::OneWireTransactionSnapshot value = {
            adk::OneWireOperation::ResetPresence,
            adk::OneWirePhase::Inert,
            adk::OneWireTransactionQuality::Unqualified,
            request (1, 0),

            {rom (), 0, false},

            rom (),
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            0,
            0,
            false,
            false,
            false,
            adk::MicrosecondTimePoint (),
            adk::StatusCode::NotInitialized,
            0,
            0,
            0,
            0};
        require (policy.snapshot (value).ok (), "snapshot succeeds");
        return value;
    }

    bool sameSnapshot (const adk::OneWireTransactionSnapshot& left,
                       const adk::OneWireTransactionSnapshot& right)
    {
        if (left.operation != right.operation || left.phase != right.phase ||
            left.quality != right.quality ||
            left.request.requestSequence != right.request.requestSequence ||
            left.request.operation != right.request.operation ||
            left.readByteCount != right.readByteCount ||
            left.acceptedSlotCount != right.acceptedSlotCount ||
            left.presenceSeen != right.presenceSeen ||
            left.releaseRequested != right.releaseRequested ||
            left.releaseConfirmed != right.releaseConfirmed ||
            left.completedAt.microseconds () != right.completedAt.microseconds () ||
            left.status != right.status || left.ownerToken != right.ownerToken ||
            left.lifecycleGeneration != right.lifecycleGeneration ||
            left.configurationRevision != right.configurationRevision ||
            left.transactionGeneration != right.transactionGeneration)
        {
            return false;
        }
        for (uint8_t index = 0; index < 9; ++index)
        {
            if (left.readBytes[index] != right.readBytes[index])
            {
                return false;
            }
        }
        return true;
    }

    bool sameIntent (const adk::OneWireStepIntent& left,
                     const adk::OneWireStepIntent& right)
    {
        return left.ownerToken == right.ownerToken &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.configurationRevision == right.configurationRevision &&
               left.requestSequence == right.requestSequence &&
               left.transactionGeneration == right.transactionGeneration &&
               left.operation == right.operation && left.phase == right.phase &&
               left.phaseSequence == right.phaseSequence &&
               left.slotIndex == right.slotIndex && left.writeBit == right.writeBit &&
               left.lineIntent == right.lineIntent &&
               left.sampleRequired == right.sampleRequired &&
               left.earliestAt.microseconds () == right.earliestAt.microseconds () &&

               left.latestAt.microseconds () == right.latestAt.microseconds ();
    }

    struct Fixture
    {
        adk::OneWireTransactionPolicy policy;
        adk::OneWireStepIntent        intent;
        uint32_t                      sequence;

        Fixture () : policy (config ()), intent (emptyIntent ()), sequence (1)
        {
            require (policy.initialize (adk::MicrosecondTimePoint (100), intent).ok (),
                     "fixture initialization emits cleanup");
            const adk::OneWireStepReceipt cleanup = receipt (intent, sequence++);

            require (policy.confirmCleanup (cleanup.observedAt, cleanup).ok (),
                     "fixture initialization confirms cleanup");
            require (
                policy
                    .begin (adk::MicrosecondTimePoint (1000), request (1, 1000), intent)

                    .ok (),
                "fixture begins Read ROM");
        }

        void apply (bool sampledHigh = true)
        {
            const adk::OneWireStepReceipt applied =
                receipt (intent, sequence++, sampledHigh);
            adk::OneWireStepIntent next = emptyIntent ();

            require (policy.update (applied.observedAt, applied, next).ok (),
                     "fixture receipt advances");
            intent = next;
        }
    };

    enum struct Substep : uint8_t
    {
        ResetDrive,
        ResetRelease,
        PresenceSample,
        PresenceRelease,
        SlotDrive,
        SlotRelease,
        SlotSample,
        SlotComplete,
        SlotRecovery
    };

    bool atSubstep (const adk::OneWireStepIntent& intent, Substep substep)
    {
        switch (substep)
        {
            case Substep::ResetDrive:
                return intent.phase == adk::OneWirePhase::ResetLow &&
                       intent.lineIntent == adk::OneWireLineIntent::DriveLow;
            case Substep::ResetRelease:
                return intent.phase == adk::OneWirePhase::ResetLow &&
                       intent.lineIntent == adk::OneWireLineIntent::Release;
            case Substep::PresenceSample:
                return intent.phase == adk::OneWirePhase::PresenceWindow &&
                       intent.sampleRequired;
            case Substep::PresenceRelease:
                return intent.phase == adk::OneWirePhase::PresenceWindow &&
                       intent.sampleRequired;
            case Substep::SlotDrive:
                return intent.phase == adk::OneWirePhase::ReadSlot &&
                       intent.lineIntent == adk::OneWireLineIntent::DriveLow;
            case Substep::SlotRelease:
                return intent.phase == adk::OneWirePhase::ReadSlot &&
                       intent.lineIntent == adk::OneWireLineIntent::Release &&
                       !intent.sampleRequired &&
                       intent.earliestAt.microseconds () !=
                           intent.latestAt.microseconds ();
            case Substep::SlotSample:
                return intent.phase == adk::OneWirePhase::ReadSlot &&
                       intent.sampleRequired;
            case Substep::SlotComplete:
                return intent.phase == adk::OneWirePhase::ReadSlot &&
                       intent.lineIntent == adk::OneWireLineIntent::Release &&
                       !intent.sampleRequired &&
                       intent.earliestAt.microseconds () !=
                           intent.latestAt.microseconds ();
            case Substep::SlotRecovery:
                return intent.phase == adk::OneWirePhase::ReadSlot &&
                       intent.lineIntent == adk::OneWireLineIntent::Release &&
                       !intent.sampleRequired &&
                       intent.earliestAt.microseconds () !=
                           intent.latestAt.microseconds ();
        }
        return false;
    }

    uint8_t occurrenceFor (Substep substep)
    {
        if (substep == Substep::PresenceRelease)
        {
            return 2;
        }
        if (substep == Substep::SlotRelease)
        {
            return 1;
        }
        if (substep == Substep::SlotComplete)
        {
            return 2;
        }
        if (substep == Substep::SlotRecovery)
        {
            return 3;
        }
        return 1;
    }

    void seek (Fixture& fixture, Substep target)
    {
        uint8_t occurrences     = 0;
        uint8_t presenceSamples = 0;
        for (uint16_t steps = 0; steps < 80; ++steps)
        {
            if (atSubstep (fixture.intent, target))
            {
                ++occurrences;
                if (occurrences == occurrenceFor (target))
                {
                    return;
                }
            }
            bool presenceLow = false;
            if (fixture.intent.phase == adk::OneWirePhase::PresenceWindow &&
                fixture.intent.sampleRequired)
            {
                ++presenceSamples;
                presenceLow = presenceSamples == 1;
            }
            fixture.apply (!presenceLow);
        }
        require (false, "target substep is reachable");
    }

    void confirmRelease (Fixture& fixture, bool remainsInitialized)
    {
        const adk::OneWireStepReceipt cleanup =
            receipt (fixture.intent, fixture.sequence++);
        require (fixture.policy.confirmCleanup (cleanup.observedAt, cleanup).ok (),
                 "interruption cleanup confirms");
        require (fixture.policy.initialized () == remainsInitialized,
                 "interruption has expected lifecycle state");
    }

    void testInterruptionsAtEverySubstep ()
    {
        const Substep substeps[] = {
            Substep::ResetDrive,      Substep::ResetRelease, Substep::PresenceSample,
            Substep::PresenceRelease, Substep::SlotDrive,    Substep::SlotRelease,
            Substep::SlotSample,      Substep::SlotComplete, Substep::SlotRecovery};
        for (const Substep substep : substeps)
        {
            Fixture cancelled;
            seek (cancelled, substep);
            const uint32_t cancelledGeneration = cancelled.intent.lifecycleGeneration;
            require (
                cancelled.policy.cancel (cancelled.intent.earliestAt, cancelled.intent)
                        .error () == adk::StatusCode::InvalidArgument,
                "cancel reports its attributed cause");
            require (cancelled.intent.lineIntent == adk::OneWireLineIntent::Release &&
                         cancelled.intent.lifecycleGeneration > cancelledGeneration,
                     "cancel emits a new correlated release");
            confirmRelease (cancelled, true);
            const adk::OneWireTransactionSnapshot cancelledState =
                snapshot (cancelled.policy);
            require (cancelledState.phase == adk::OneWirePhase::Fault &&
                         cancelledState.quality ==
                             adk::OneWireTransactionQuality::ProducerFault &&
                         cancelledState.status.error () ==
                             adk::StatusCode::InvalidArgument,
                     "cancel preserves failure attribution after release");

            Fixture reset;
            seek (reset, substep);

            require (reset.policy.reset (reset.intent.earliestAt, reset.intent).ok () &&
                         reset.intent.lineIntent == adk::OneWireLineIntent::Release,
                     "reset emits release from every substep");
            confirmRelease (reset, true);

            require (snapshot (reset.policy).phase == adk::OneWirePhase::Inert,
                     "reset returns policy to ready inert state");

            Fixture shutdown;
            seek (shutdown, substep);

            require (
                shutdown.policy.shutdown (shutdown.intent.earliestAt, shutdown.intent)
                        .ok () &&
                    shutdown.intent.lineIntent == adk::OneWireLineIntent::Release,
                "shutdown emits release from every substep");
            confirmRelease (shutdown, false);
        }
    }

    std::vector<adk::OneWireStepIntent> completeTrace ()
    {
        Fixture                             fixture;
        std::vector<adk::OneWireStepIntent> trace;
        while (snapshot (fixture.policy).quality ==
               adk::OneWireTransactionQuality::Pending)
        {
            trace.push_back (fixture.intent);
            const bool lowPresence =
                fixture.intent.phase == adk::OneWirePhase::PresenceWindow &&
                trace.size () == 3;
            fixture.apply (!lowPresence);
        }
        return trace;
    }

    void testFirstMiddleAndFinalProducerFailures ()
    {
        const std::vector<adk::OneWireStepIntent> trace = completeTrace ();

        const size_t positions[] = {0, trace.size () / 2, trace.size () - 1};
        for (const size_t position : positions)
        {
            Fixture fixture;
            for (size_t index = 0; index < position; ++index)
            {
                const bool lowPresence =
                    fixture.intent.phase == adk::OneWirePhase::PresenceWindow &&
                    index == 2;
                fixture.apply (!lowPresence);
            }
            const adk::OneWireStepReceipt failed =
                receipt (fixture.intent, fixture.sequence++, true, false,
                         adk::StatusCode::HardwareFailure);
            adk::OneWireStepIntent release = emptyIntent ();

            require (
                fixture.policy.update (failed.observedAt, failed, release).error () ==
                        adk::StatusCode::HardwareFailure &&
                    release.lineIntent == adk::OneWireLineIntent::Release,
                "producer failure fails closed");
            fixture.intent = release;
            confirmRelease (fixture, true);

            const adk::OneWireTransactionSnapshot state = snapshot (fixture.policy);

            require (state.phase == adk::OneWirePhase::Fault &&
                         state.quality ==
                             adk::OneWireTransactionQuality::ProducerFault &&
                         state.status.error () == adk::StatusCode::HardwareFailure,
                     "producer failure remains attributed after cleanup");
        }
    }

    void finish (Fixture& fixture)
    {
        size_t step = 0;
        while (snapshot (fixture.policy).quality ==
               adk::OneWireTransactionQuality::Pending)
        {
            const bool lowPresence =
                fixture.intent.phase == adk::OneWirePhase::PresenceWindow && step == 2;
            fixture.apply (!lowPresence);
            ++step;
        }
    }

    void testCleanupAndCompleteInterruptions ()
    {
        adk::OneWireStepIntent cleanup = emptyIntent ();

        adk::OneWireTransactionPolicy cancelling (config ());

        require (cancelling.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                 "cleanup-state fixture initializes");
        require (cancelling
                         .cancel (adk::MicrosecondTimePoint (100), cleanup)

                         .error () == adk::StatusCode::ResourceBusy,
                 "cancel cannot overlap cleanup");

        adk::OneWireTransactionPolicy resetting (config ());

        require (resetting.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                 "reset cleanup fixture initializes");
        const uint32_t resetGeneration = cleanup.lifecycleGeneration;
        require (resetting.reset (adk::MicrosecondTimePoint (101), cleanup).ok () &&
                     cleanup.lifecycleGeneration > resetGeneration,
                 "reset supersedes an outstanding cleanup");

        adk::OneWireTransactionPolicy shuttingDown (config ());

        require (
            shuttingDown.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
            "shutdown cleanup fixture initializes");
        const uint32_t shutdownGeneration = cleanup.lifecycleGeneration;
        require (
            shuttingDown.shutdown (adk::MicrosecondTimePoint (101), cleanup).ok () &&
                cleanup.lifecycleGeneration > shutdownGeneration,
            "shutdown supersedes an outstanding cleanup");

        Fixture cancelled;
        finish (cancelled);

        const adk::MicrosecondTimePoint cancelledAt (
            cancelled.intent.earliestAt.microseconds () + 1U);

        require (cancelled.policy.cancel (cancelledAt, cancelled.intent).error () ==
                     adk::StatusCode::ResourceBusy,
                 "cancel cannot supersede completion cleanup");
        confirmRelease (cancelled, true);

        Fixture reset;
        finish (reset);

        const adk::MicrosecondTimePoint resetAt (
            reset.intent.earliestAt.microseconds () + 1U);

        require (reset.policy.reset (resetAt, reset.intent).ok (),
                 "reset releases from complete");
        confirmRelease (reset, true);

        Fixture shutdown;
        finish (shutdown);

        const adk::MicrosecondTimePoint shutdownAt (
            shutdown.intent.earliestAt.microseconds () + 1U);

        require (shutdown.policy.shutdown (shutdownAt, shutdown.intent).ok (),
                 "shutdown releases from complete");
        confirmRelease (shutdown, false);
    }

    void testInvalidEnumsAndCorrelationMutations ()
    {
        typedef void (*Mutation) (adk::OneWireStepReceipt&);
        const Mutation mutations[] = {
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.sourceId;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.configurationRevision;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.ownerToken;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.lifecycleGeneration;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.requestSequence;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.transactionGeneration;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.phaseSequence;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.slotIndex;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.operation = static_cast<adk::OneWireOperation> (0xff);
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.phase = static_cast<adk::OneWirePhase> (0xff);
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.appliedIntent = static_cast<adk::OneWireLineIntent> (0xff);
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.status = static_cast<adk::StatusCode> (0xff);
            }};
        for (const Mutation mutate : mutations)
        {
            Fixture                               fixture;
            const adk::OneWireTransactionSnapshot before = snapshot (fixture.policy);
            adk::OneWireStepReceipt               changed =
                receipt (fixture.intent, fixture.sequence);
            mutate (changed);

            adk::OneWireStepIntent ignored = emptyIntent ();

            require (
                fixture.policy.update (changed.observedAt, changed, ignored).error () ==
                        adk::StatusCode::InvalidArgument &&
                    sameSnapshot (before, snapshot (fixture.policy)),
                "invalid enum or correlation mutation rejects atomically");
        }
    }

    void testExactAndTwinReplay ()
    {
        Fixture                       fixture;
        const adk::OneWireStepReceipt first =
            receipt (fixture.intent, fixture.sequence++);
        adk::OneWireStepIntent next = emptyIntent ();

        require (fixture.policy.update (first.observedAt, first, next).ok (),
                 "first receipt advances");
        const adk::OneWireTransactionSnapshot after = snapshot (fixture.policy);

        adk::OneWireStepIntent replay = emptyIntent ();

        require (fixture.policy.update (first.observedAt, first, replay).ok () &&
                     sameSnapshot (after, snapshot (fixture.policy)),
                 "exact receipt replay is idempotent");

        adk::OneWireStepReceipt twin = first;
        twin.sampledHigh             = !twin.sampledHigh;
        require (fixture.policy.update (twin.observedAt, twin, replay).error () ==
                         adk::StatusCode::InvalidArgument &&
                     sameSnapshot (after, snapshot (fixture.policy)),
                 "same-sequence twin replay rejects atomically");

        require (fixture.policy.reset (adk::MicrosecondTimePoint (2000), fixture.intent)
                     .ok (),
                 "cleanup replay fixture resets");
        const adk::OneWireStepReceipt cleanup =
            receipt (fixture.intent, fixture.sequence++);
        require (fixture.policy.confirmCleanup (cleanup.observedAt, cleanup).ok (),
                 "cleanup receipt confirms");
        const adk::OneWireTransactionSnapshot cleaned = snapshot (fixture.policy);

        require (fixture.policy.confirmCleanup (cleanup.observedAt, cleanup).ok () &&
                     sameSnapshot (cleaned, snapshot (fixture.policy)),
                 "exact cleanup replay is idempotent");
        adk::OneWireStepReceipt cleanupTwin = cleanup;
        ++cleanupTwin.ownerToken;
        require (fixture.policy.confirmCleanup (cleanupTwin.observedAt, cleanupTwin)
                             .error () == adk::StatusCode::InvalidArgument &&
                     sameSnapshot (cleaned, snapshot (fixture.policy)),
                 "cleanup twin replay rejects atomically");
    }

    void testEveryChangedDuplicateFieldRejects ()
    {
        typedef void (*Mutation) (adk::OneWireStepReceipt&);
        const Mutation mutations[] = {
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.sourceId;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.configurationRevision;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.observedAt =
                    adk::MicrosecondTimePoint (value.observedAt.microseconds () + 1U);
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.ownerToken;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.lifecycleGeneration;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.requestSequence;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.transactionGeneration;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.operation = adk::OneWireOperation::SearchRomPass;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.phase = adk::OneWirePhase::Inert;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.phaseSequence;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.slotIndex;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.appliedIntent =
                    value.appliedIntent == adk::OneWireLineIntent::Release
                        ? adk::OneWireLineIntent::DriveLow
                        : adk::OneWireLineIntent::Release;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.sampledHigh = !value.sampledHigh;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.accepted = !value.accepted;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.status = adk::StatusCode::HardwareFailure;
            }};

        for (const Mutation mutate : mutations)
        {
            Fixture                       fixture;
            const adk::OneWireStepReceipt applied =
                receipt (fixture.intent, fixture.sequence++);
            adk::OneWireStepIntent next = emptyIntent ();

            require (fixture.policy.update (applied.observedAt, applied, next).ok (),
                     "changed-duplicate fixture advances");
            const adk::OneWireTransactionSnapshot after   = snapshot (fixture.policy);
            adk::OneWireStepReceipt               changed = applied;

            mutate (changed);

            adk::OneWireStepIntent sentinel        = emptyIntent ();
            sentinel.ownerToken                    = 99;
            sentinel.phaseSequence                 = 77;
            const adk::OneWireStepIntent unchanged = sentinel;

            const adk::Status changedStatus =
                fixture.policy.update (changed.observedAt, changed, sentinel);

            const bool snapshotPreserved =
                sameSnapshot (after, snapshot (fixture.policy));

            const bool intentPreserved = sameIntent (sentinel, unchanged);

            require (changedStatus.error () == adk::StatusCode::InvalidArgument &&
                         snapshotPreserved && intentPreserved,
                     "every changed duplicate field rejects atomically");

            adk::OneWireTransactionPolicy cleanupPolicy (config ());

            adk::OneWireStepIntent cleanup = emptyIntent ();

            require (cleanupPolicy.initialize (adk::MicrosecondTimePoint (100), cleanup)
                         .ok (),
                     "changed cleanup duplicate fixture initializes");
            const adk::OneWireStepReceipt cleanupReceipt = receipt (cleanup, 1);

            require (
                cleanupPolicy.confirmCleanup (cleanupReceipt.observedAt, cleanupReceipt)
                    .ok (),
                "changed cleanup duplicate fixture confirms");
            const adk::OneWireTransactionSnapshot cleaned = snapshot (cleanupPolicy);
            adk::OneWireStepReceipt               changedCleanup = cleanupReceipt;

            mutate (changedCleanup);

            const adk::Status cleanupStatus = cleanupPolicy.confirmCleanup (
                changedCleanup.observedAt, changedCleanup);

            require (cleanupStatus.error () == adk::StatusCode::InvalidArgument &&
                         sameSnapshot (cleaned, snapshot (cleanupPolicy)),
                     "every changed cleanup duplicate field rejects atomically");
        }
    }

    void testStaleReplayAfterLaterCommand ()
    {
        Fixture                       fixture;
        const adk::OneWireStepReceipt first =
            receipt (fixture.intent, fixture.sequence++);
        adk::OneWireStepIntent next = emptyIntent ();

        require (fixture.policy.update (first.observedAt, first, next).ok (),
                 "stale update fixture accepts first command");
        const adk::OneWireStepReceipt second = receipt (next, fixture.sequence++);

        adk::OneWireStepIntent later = emptyIntent ();

        require (fixture.policy.update (second.observedAt, second, later).ok (),
                 "stale update fixture accepts later command");
        const adk::OneWireTransactionSnapshot after = snapshot (fixture.policy);

        adk::OneWireStepIntent sentinel        = emptyIntent ();
        sentinel.ownerToken                    = 99;
        const adk::OneWireStepIntent unchanged = sentinel;

        const adk::Status staleStatus =
            fixture.policy.update (first.observedAt, first, sentinel);

        const bool snapshotPreserved = sameSnapshot (after, snapshot (fixture.policy));

        const bool intentPreserved = sameIntent (sentinel, unchanged);

        require (staleStatus.error () == adk::StatusCode::InvalidArgument &&
                     snapshotPreserved && intentPreserved,
                 "stale update replay after later command rejects atomically");

        require (fixture.policy.reset (adk::MicrosecondTimePoint (2000), next).ok (),
                 "stale cleanup fixture emits first cleanup");
        const adk::OneWireStepReceipt cleanup = receipt (next, fixture.sequence++);

        require (fixture.policy.confirmCleanup (cleanup.observedAt, cleanup).ok (),
                 "stale cleanup fixture confirms first cleanup");
        require (fixture.policy.reset (adk::MicrosecondTimePoint (2100), later).ok (),
                 "stale cleanup fixture emits later cleanup");
        const adk::OneWireTransactionSnapshot cleaning = snapshot (fixture.policy);

        require (fixture.policy.confirmCleanup (cleanup.observedAt, cleanup).error () ==
                         adk::StatusCode::InvalidArgument &&
                     sameSnapshot (cleaning, snapshot (fixture.policy)),
                 "stale cleanup replay after later command rejects atomically");
    }

    void testCleanupBackwardAndHalfRangeTimeRejects ()
    {
        const uint32_t invalidTimes[] = {99U, 100U + 0x80000000UL};
        for (const uint32_t invalidTime : invalidTimes)
        {
            adk::OneWireTransactionPolicy policy (config ());

            adk::OneWireStepIntent cleanup = emptyIntent ();

            require (policy.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                     "cleanup-time fixture initializes");
            const adk::OneWireTransactionSnapshot before = snapshot (policy);

            adk::OneWireStepReceipt changed = receipt (cleanup, 1);

            changed.observedAt = adk::MicrosecondTimePoint (invalidTime);

            require (policy.confirmCleanup (changed.observedAt, changed).error () ==
                             adk::StatusCode::InvalidArgument &&
                         sameSnapshot (before, snapshot (policy)),
                     "cleanup backward or half-range time rejects atomically");
        }
    }

    void testTerminalTriggerReplayReemitsCleanup ()
    {
        Fixture noPresence;
        noPresence.apply ();
        noPresence.apply ();
        const adk::OneWireStepReceipt absent =
            receipt (noPresence.intent, noPresence.sequence++, true);
        adk::OneWireStepIntent absentCleanup = emptyIntent ();

        require (
            noPresence.policy.update (absent.observedAt, absent, absentCleanup).ok (),
            "no-presence trigger emits cleanup");
        const adk::OneWireTransactionSnapshot absentState =
            snapshot (noPresence.policy);
        adk::OneWireStepIntent absentReplay = emptyIntent ();

        const adk::Status absentReplayStatus =
            noPresence.policy.update (absent.observedAt, absent, absentReplay);

        const bool absentIntentSame = sameIntent (absentCleanup, absentReplay);

        const bool absentSnapshotSame =
            sameSnapshot (absentState, snapshot (noPresence.policy));

        require (absentReplayStatus.ok () && absentIntentSame && absentSnapshotSame &&
                     absentReplay.ownerToken == absentState.ownerToken &&
                     absentReplay.lifecycleGeneration ==
                         absentState.lifecycleGeneration &&
                     absentReplay.transactionGeneration ==
                         absentState.transactionGeneration &&
                     absentReplay.lineIntent == adk::OneWireLineIntent::Release,
                 "no-presence trigger replay re-emits identical correlated cleanup");

        Fixture                       producerFault;
        const adk::OneWireStepReceipt failed =
            receipt (producerFault.intent, producerFault.sequence++, true, false,
                     adk::StatusCode::HardwareFailure);
        adk::OneWireStepIntent faultCleanup = emptyIntent ();

        require (producerFault.policy.update (failed.observedAt, failed, faultCleanup)
                         .error () == adk::StatusCode::HardwareFailure,
                 "producer-fault trigger emits cleanup");
        const adk::OneWireTransactionSnapshot faultState =
            snapshot (producerFault.policy);
        adk::OneWireStepIntent faultReplay = emptyIntent ();

        const adk::Status faultReplayStatus =
            producerFault.policy.update (failed.observedAt, failed, faultReplay);

        const bool faultIntentSame = sameIntent (faultCleanup, faultReplay);

        const bool faultSnapshotSame =
            sameSnapshot (faultState, snapshot (producerFault.policy));

        require (
            faultReplayStatus.ok () && faultIntentSame && faultSnapshotSame &&
                faultReplay.ownerToken == faultState.ownerToken &&
                faultReplay.lifecycleGeneration == faultState.lifecycleGeneration &&
                faultReplay.transactionGeneration == faultState.transactionGeneration &&
                faultReplay.lineIntent == adk::OneWireLineIntent::Release,
            "producer-fault trigger replay re-emits identical correlated cleanup");
    }

    void testLateCleanupAndStructuralPrecedence ()
    {
        adk::OneWireTransactionPolicy policy (config ());

        adk::OneWireStepIntent cleanup = emptyIntent ();

        require (policy.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                 "late cleanup fixture initializes");
        const adk::OneWireTransactionSnapshot before = snapshot (policy);

        adk::OneWireStepReceipt late = receipt (cleanup, 1);

        late.observedAt = adk::MicrosecondTimePoint (101);

        require (policy.confirmCleanup (late.observedAt, late).error () ==
                         adk::StatusCode::InvalidArgument &&
                     sameSnapshot (before, snapshot (policy)),
                 "late cleanup receipt rejects atomically");

        Fixture fixture;

        const adk::OneWireTransactionSnapshot pending = snapshot (fixture.policy);

        adk::OneWireStepReceipt foreign = receipt (fixture.intent, fixture.sequence);

        foreign.observedAt = adk::MicrosecondTimePoint (22000);
        ++foreign.ownerToken;
        adk::OneWireStepIntent sentinel        = emptyIntent ();
        sentinel.ownerToken                    = 99;
        sentinel.phaseSequence                 = 77;
        const adk::OneWireStepIntent unchanged = sentinel;

        require (
            fixture.policy.update (foreign.observedAt, foreign, sentinel).error () ==
                    adk::StatusCode::InvalidArgument &&
                sameSnapshot (pending, snapshot (fixture.policy)) &&

                sameIntent (sentinel, unchanged),
            "foreign receipt rejects structurally before deadline handling");
    }

    void testStuckLowPresenceRelease ()
    {
        Fixture fixture;
        seek (fixture, Substep::PresenceRelease);
        const adk::OneWireStepReceipt stuckLow =
            receipt (fixture.intent, fixture.sequence++, false);
        adk::OneWireStepIntent cleanup = emptyIntent ();

        require (fixture.policy.update (stuckLow.observedAt, stuckLow, cleanup).ok () &&
                     cleanup.lineIntent == adk::OneWireLineIntent::Release,
                 "stuck-low presence release enters correlated cleanup");
        fixture.intent = cleanup;
        confirmRelease (fixture, true);

        const adk::OneWireTransactionSnapshot state = snapshot (fixture.policy);

        require (state.phase == adk::OneWirePhase::Fault &&
                     state.quality == adk::OneWireTransactionQuality::Collision &&
                     state.status.ok () && state.presenceSeen &&
                     state.releaseRequested && state.releaseConfirmed,
                 "stuck-low presence remains collision-attributed after release");
    }

    void testCleanupSequenceMonotonicity ()
    {
        Fixture fixture;
        fixture.apply ();

        adk::OneWireStepIntent cleanup = emptyIntent ();

        require (fixture.policy.reset (adk::MicrosecondTimePoint (2000), cleanup).ok (),
                 "cleanup sequence fixture resets");
        const adk::OneWireTransactionSnapshot before = snapshot (fixture.policy);

        adk::OneWireStepReceipt regressed = receipt (cleanup, fixture.sequence - 2U);

        require (
            fixture.policy.confirmCleanup (regressed.observedAt, regressed).error () ==
                    adk::StatusCode::InvalidArgument &&
                sameSnapshot (before, snapshot (fixture.policy)),
            "cleanup receipt sequence regression rejects atomically");

        adk::OneWireStepReceipt ambiguous =
            receipt (cleanup, fixture.sequence - 1U + 0x80000000UL);
        require (
            fixture.policy.confirmCleanup (ambiguous.observedAt, ambiguous).error () ==
                    adk::StatusCode::InvalidArgument &&
                sameSnapshot (before, snapshot (fixture.policy)),
            "cleanup receipt half-range ambiguity rejects atomically");

        const adk::OneWireStepReceipt applied = receipt (cleanup, fixture.sequence++);

        require (fixture.policy.confirmCleanup (applied.observedAt, applied).ok (),
                 "monotonic cleanup receipt confirms");
    }

    void testCleanupProvenanceAndReleaseSemantics ()
    {
        typedef void (*Mutation) (adk::OneWireStepReceipt&);
        const Mutation provenanceMutations[] = {
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.sourceId;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.configurationRevision;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.requestSequence;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.transactionGeneration;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                ++value.phaseSequence;
            },
            [] (adk::OneWireStepReceipt& value)
            {
                value.appliedIntent = adk::OneWireLineIntent::DriveLow;
            }};

        for (const Mutation mutate : provenanceMutations)
        {
            adk::OneWireTransactionPolicy policy (config ());

            adk::OneWireStepIntent cleanup = emptyIntent ();

            require (policy.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                     "cleanup provenance fixture initializes");
            const adk::OneWireTransactionSnapshot before = snapshot (policy);

            adk::OneWireStepReceipt changed = receipt (cleanup, 1);

            mutate (changed);

            require (policy.confirmCleanup (changed.observedAt, changed).error () ==
                             adk::StatusCode::InvalidArgument &&
                         sameSnapshot (before, snapshot (policy)),
                     "changed cleanup provenance rejects atomically");
        }

        adk::OneWireTransactionPolicy policy (config ());

        adk::OneWireStepIntent cleanup = emptyIntent ();

        require (policy.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                 "release semantics fixture initializes");
        const adk::OneWireStepReceipt appliedRelease = receipt (cleanup, 1, false);

        require (
            policy.confirmCleanup (appliedRelease.observedAt, appliedRelease).ok (),
            "copied applied-release receipt confirms without voltage evidence");
        const adk::OneWireTransactionSnapshot released = snapshot (policy);

        require (released.releaseRequested && released.releaseConfirmed &&
                     released.phase == adk::OneWirePhase::Inert,
                 "release confirmation records producer application semantics");
    }

#if defined(ADK_TESTING)
    void testPhaseExhaustionRejectsCleanupAtomically ()
    {
        adk::OneWireTransactionPolicy policy (config ());

        adk::OneWireStepIntent cleanup = emptyIntent ();

        require (policy.initialize (adk::MicrosecondTimePoint (100), cleanup).ok (),
                 "phase exhaustion fixture initializes");
        const adk::OneWireStepReceipt initialized = receipt (cleanup, 1);

        require (policy.confirmCleanup (initialized.observedAt, initialized).ok (),
                 "phase exhaustion fixture confirms initialization");

        policy.seedSequencesForTest (1, 1, UINT32_MAX);

        const adk::OneWireTransactionSnapshot before = snapshot (policy);

        adk::OneWireStepIntent sentinel        = emptyIntent ();
        sentinel.ownerToken                    = 99;
        sentinel.phaseSequence                 = 77;
        const adk::OneWireStepIntent unchanged = sentinel;

        require (policy.reset (adk::MicrosecondTimePoint (200), sentinel).error () ==
                         adk::StatusCode::CapacityExceeded &&
                     sameSnapshot (before, snapshot (policy)) &&

                     sameIntent (sentinel, unchanged),
                 "phase exhaustion rejects cleanup atomically");
    }
#endif
} // namespace

int main ()
{
    testInterruptionsAtEverySubstep ();

    testFirstMiddleAndFinalProducerFailures ();

    testCleanupAndCompleteInterruptions ();

    testInvalidEnumsAndCorrelationMutations ();

    testExactAndTwinReplay ();

    testEveryChangedDuplicateFieldRejects ();

    testStaleReplayAfterLaterCommand ();

    testCleanupBackwardAndHalfRangeTimeRejects ();

    testTerminalTriggerReplayReemitsCleanup ();

    testLateCleanupAndStructuralPrecedence ();

    testStuckLowPresenceRelease ();

    testCleanupSequenceMonotonicity ();

    testCleanupProvenanceAndReleaseSemantics ();

#if defined(ADK_TESTING)
    testPhaseExhaustionRejectsCleanupAtomically ();
#endif
}
