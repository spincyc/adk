#include <one_wire_transaction_policy.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>

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

    adk::OneWireRomCode rom (uint8_t first = 0x28)

    {
        return {{first, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}};
    }

    adk::OneWireTransactionConfig config (bool singleDrop = true)

    {
        return {0x12345678UL,
                11,
                7,
                17,
                singleDrop,
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

    adk::OneWireOperationRequest
    request (uint32_t sequence, adk::OneWireOperation operation,

             uint32_t               startedAt = 1000,
             adk::OneWireSupplyMode supply = adk::OneWireSupplyMode::ExternallyPowered)
    {
        return {sequence,
                operation,
                rom (),

                {rom (), 0, false},

                adk::MicrosecondTimePoint (startedAt),

                supply,
                adk::StatusCode::Ok};
    }

    adk::OneWireStepReceipt receipt (const adk::OneWireStepIntent& intent,

                                     uint32_t sequence, uint32_t observedAt,
                                     bool sampledHigh = true, bool accepted = true,
                                     adk::Status status = adk::StatusCode::Ok)
    {
        return {7,
                17,
                sequence,
                adk::MicrosecondTimePoint (observedAt),

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

    bool romEqual (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)

    {
        for (uint8_t index = 0; index < 8; ++index)

        {
            if (left.bytes[index] != right.bytes[index])

            {
                return false;
            }
        }
        return true;
    }

    bool intentEqual (const adk::OneWireStepIntent& left,

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

               left.latestAt.microseconds () == right.latestAt.microseconds () &&

               romEqual (left.addressedRom, right.addressedRom);

    }

    bool requestEqual (const adk::OneWireOperationRequest& left,

                       const adk::OneWireOperationRequest& right)
    {
        return left.requestSequence == right.requestSequence &&
               left.operation == right.operation &&
               romEqual (left.addressedRom, right.addressedRom) &&

               romEqual (left.search.rom, right.search.rom) &&

               left.search.lastDiscrepancy == right.search.lastDiscrepancy &&
               left.search.lastDevice == right.search.lastDevice &&
               left.startedAt.microseconds () == right.startedAt.microseconds () &&

               left.supplyMode == right.supplyMode && left.status == right.status;
    }

    bool snapshotEqual (const adk::OneWireTransactionSnapshot& left,

                        const adk::OneWireTransactionSnapshot& right)
    {
        if (left.operation != right.operation || left.phase != right.phase ||

            left.quality != right.quality ||
            !requestEqual (left.request, right.request) ||

            !romEqual (left.searchResult.rom, right.searchResult.rom) ||

            left.searchResult.lastDiscrepancy != right.searchResult.lastDiscrepancy ||
            left.searchResult.lastDevice != right.searchResult.lastDevice ||
            !romEqual (left.returnedRom, right.returnedRom) ||

            left.readByteCount != right.readByteCount ||
            left.acceptedSlotCount != right.acceptedSlotCount ||
            left.presenceSeen != right.presenceSeen ||
            left.releaseRequested != right.releaseRequested ||
            left.releaseConfirmed != right.releaseConfirmed ||
            left.completedAt.microseconds () != right.completedAt.microseconds () ||

            left.status != right.status ||
            left.ownerToken != right.ownerToken ||
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

    adk::OneWireTransactionSnapshot
    snapshot (const adk::OneWireTransactionPolicy& policy)

    {
        adk::OneWireTransactionSnapshot value = {
            adk::OneWireOperation::ResetPresence,
            adk::OneWirePhase::Inert,
            adk::OneWireTransactionQuality::Unqualified,
            request (0, adk::OneWireOperation::ResetPresence, 0),

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

    adk::OneWireStepIntent initializeReady (adk::OneWireTransactionPolicy& policy,

                                            uint32_t                       now = 100)
    {
        adk::OneWireStepIntent release = emptyIntent ();

        require (policy.initialize (adk::MicrosecondTimePoint (now), release).ok (),

                 "policy initialization emits release");
        require (release.lineIntent == adk::OneWireLineIntent::Release,

                 "initialization requests release");
        const adk::OneWireStepReceipt applied =
            receipt (release, 1, release.earliestAt.microseconds ());

        require (policy

                     .confirmCleanup (

                         adk::MicrosecondTimePoint (applied.observedAt.microseconds ()),

                         applied)
                     .ok (),

                 "initial cleanup confirms");
        return release;
    }

    adk::OneWireStepIntent applyAt (adk::OneWireTransactionPolicy& policy,

                                    const adk::OneWireStepIntent& intent,
                                    uint32_t& receiptSequence, bool sampledHigh = true)
    {
        adk::OneWireStepIntent next = emptyIntent ();

        const uint32_t observedAt = intent.earliestAt.microseconds ();
        const adk::OneWireStepReceipt applied =
            receipt (intent, receiptSequence++, observedAt, sampledHigh);

        require (policy.update (adk::MicrosecondTimePoint (observedAt), applied, next)
                     .ok (),

                 "correlated in-window receipt advances transaction");
        return next;
    }

    adk::OneWireStepIntent establishPresence (
        adk::OneWireTransactionPolicy& policy, adk::OneWireOperation operation,
        uint32_t& receiptSequence, uint32_t startedAt = 1000)
    {
        adk::OneWireStepIntent intent = emptyIntent ();

        require (policy.begin (adk::MicrosecondTimePoint (startedAt),

                               request (1, operation, startedAt), intent)
                     .ok (),

                 "operation begins");
        intent = applyAt (policy, intent, receiptSequence);

        intent = applyAt (policy, intent, receiptSequence);

        intent = applyAt (policy, intent, receiptSequence, false);

        intent = applyAt (policy, intent, receiptSequence, true);

        return intent;
    }

    void testLifecycleAndConfiguration ()

    {
        static_assert (

            !std::is_copy_constructible<adk::OneWireTransactionPolicy>::value,
            "one-wire policy does not copy");
        static_assert (

            !std::is_move_constructible<adk::OneWireTransactionPolicy>::value,
            "one-wire policy does not move");
        static_assert (

            std::is_trivially_destructible<adk::OneWireTransactionPolicy>::value,
            "one-wire policy destruction is inert");

        adk::OneWireTransactionPolicy policy (config ());

        require (!policy.initialized (), "construction is inert");

        adk::OneWireTransactionSnapshot inert = snapshot (policy);

        require (inert.phase == adk::OneWirePhase::Inert &&

                     inert.quality == adk::OneWireTransactionQuality::Unqualified &&
                     inert.status.error () == adk::StatusCode::NotInitialized,

                 "construction snapshot is canonical");

        adk::OneWireStepIntent intent = emptyIntent ();

        require (policy.begin (adk::MicrosecondTimePoint (100),

                               request (1, adk::OneWireOperation::ResetPresence),

                               intent)
                         .error () == adk::StatusCode::NotInitialized,

                 "begin before initialize rejects");
        initializeReady (policy);

        require (policy.initialized (), "cleanup makes policy ready");


        using Mutator           = void (*) (adk::OneWireTransactionConfig&);

        const Mutator invalid[] = {
            [] (adk::OneWireTransactionConfig& v)
            {
                v.ownerToken = 0;
            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.configurationRevision = 0;
            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.expectedReceiptSourceId = 0;
            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.expectedReceiptConfigurationRevision = 0;
            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.resetLowMinimum = adk::MicrosecondDuration (0);

            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.resetLowMaximum = adk::MicrosecondDuration (0x80000000UL);

            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.resetLowMinimum = v.resetLowMaximum;
                v.resetLowMaximum = adk::MicrosecondDuration (100);

            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.transactionDeadline = adk::MicrosecondDuration (0);

            },
            [] (adk::OneWireTransactionConfig& v)
            {
                v.maximumSlots = 0;
            }};
        for (Mutator mutate : invalid)

        {
            adk::OneWireTransactionConfig value = config ();

            mutate (value);

            adk::OneWireTransactionPolicy rejected (value);

            adk::OneWireStepIntent        release = emptyIntent ();

            require (

                rejected.initialize (adk::MicrosecondTimePoint (0), release).error () ==

                        adk::StatusCode::InvalidConfiguration &&
                    !rejected.initialized (),

                "invalid configuration remains inert");
        }
    }

    void testRequestValidationAndAtomicity ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy);

        const adk::OneWireTransactionSnapshot before = snapshot (policy);

        adk::OneWireStepIntent                intent = emptyIntent ();

        require (

            policy
                    .begin (
                        adk::MicrosecondTimePoint (200),

                        request (
                            1,
                            adk::OneWireOperation::MatchRomReadConversionStatus,
                            200),

                        intent)
                    .error () == adk::StatusCode::InvalidArgument &&
                snapshotEqual (before, snapshot (policy)),

            "conversion status rejects without a confirmed conversion");

        adk::OneWireOperationRequest parasite =
            request (1, adk::OneWireOperation::ResetPresence, 200,

                     adk::OneWireSupplyMode::ParasitePower);
        require (

            policy.begin (adk::MicrosecondTimePoint (200), parasite, intent).error () ==

                    adk::StatusCode::Unsupported &&
                snapshotEqual (before, snapshot (policy)),

            "parasite power rejects before mutation");

        adk::OneWireOperationRequest malformed =
            request (0, adk::OneWireOperation::ResetPresence, 200);

        require (policy.begin (adk::MicrosecondTimePoint (200), malformed, intent)

                             .error () == adk::StatusCode::InvalidArgument &&

                     snapshotEqual (before, snapshot (policy)),

                 "zero request sequence rejects atomically");
        malformed = request (1, static_cast<adk::OneWireOperation> (0xff), 200);

        require (policy.begin (adk::MicrosecondTimePoint (200), malformed, intent)

                             .error () == adk::StatusCode::InvalidArgument &&

                     snapshotEqual (before, snapshot (policy)),

                 "unknown operation rejects atomically");
        malformed = request (1, adk::OneWireOperation::ResetPresence, 200);
        malformed.supplyMode = static_cast<adk::OneWireSupplyMode> (0xff);

        require (policy.begin (adk::MicrosecondTimePoint (200), malformed, intent)

                             .error () == adk::StatusCode::InvalidArgument &&

                     snapshotEqual (before, snapshot (policy)),

                 "unknown supply mode rejects atomically");
        const adk::Status invalidStatuses[] = {
            static_cast<adk::StatusCode> (0xff),
            adk::StatusCode::HardwareFailure,
            adk::StatusCode::ResourceBusy};
        for (const adk::Status status : invalidStatuses)

        {
            malformed = request (1, adk::OneWireOperation::ResetPresence, 200);
            malformed.status = status;

            const adk::Status result =
                policy.begin (adk::MicrosecondTimePoint (200), malformed, intent);

            require (
                result.error () ==
                        (status.error () == static_cast<adk::StatusCode> (0xff) ?
                             adk::StatusCode::InvalidArgument :
                             status.error ()) &&
                    snapshotEqual (before, snapshot (policy)),

                "invalid or non-OK request status rejects atomically");
        }
        malformed = request (1, adk::OneWireOperation::MatchRomReadScratchpad, 200);

        for (uint8_t& value : malformed.addressedRom.bytes)

        {
            value = 0;
        }
        require (policy.begin (adk::MicrosecondTimePoint (200), malformed, intent)

                             .error () == adk::StatusCode::InvalidArgument &&

                     snapshotEqual (before, snapshot (policy)),

                 "all-zero addressed ROM rejects");

        adk::OneWireOperationRequest correlated =
            request (1, adk::OneWireOperation::MatchRomReadScratchpad, 200);
        const adk::OneWireRomCode addressedRom = correlated.addressedRom;

        require (policy.begin (adk::MicrosecondTimePoint (200), correlated, intent)
                         .ok () &&

                     romEqual (intent.addressedRom, addressedRom),

                 "request binds addressed ROM into every correlated intent");
        correlated.addressedRom.bytes[0] ^= 0xffU;
        adk::OneWireStepIntent correlatedNext = emptyIntent ();

        require (policy
                     .update (intent.earliestAt,
                              receipt (intent, 2,
                                       intent.earliestAt.microseconds ()),
                              correlatedNext)
                     .ok () &&

                     romEqual (correlatedNext.addressedRom, addressedRom),

                 "receipt has no ROM field; immutable request and intent bind ROM");

        adk::OneWireTransactionPolicy multidrop (config (false));

        initializeReady (multidrop);

        require (

            multidrop
                    .begin (adk::MicrosecondTimePoint (200),

                            request (1, adk::OneWireOperation::ReadRomSingleDrop, 200),

                            intent)
                    .error () == adk::StatusCode::Unsupported,

            "single-drop read rejects on multidrop configuration");
    }

    void testCleanupCorrelationAndShutdown ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        adk::OneWireStepIntent        release = emptyIntent ();

        require (policy.initialize (adk::MicrosecondTimePoint (100), release).ok (),

                 "cleanup fixture initializes");
        const adk::OneWireTransactionSnapshot waiting = snapshot (policy);


        adk::OneWireStepReceipt foreign =
            receipt (release, 1, release.earliestAt.microseconds ());

        ++foreign.ownerToken;
        require (policy.confirmCleanup (foreign.observedAt, foreign).error () ==

                         adk::StatusCode::InvalidArgument &&
                     snapshotEqual (waiting, snapshot (policy)),

                 "foreign cleanup receipt rejects atomically");
        adk::OneWireStepReceipt early =
            receipt (release, 1, release.earliestAt.microseconds () - 1U);

        require (policy.confirmCleanup (early.observedAt, early).error () ==

                         adk::StatusCode::InvalidArgument &&
                     snapshotEqual (waiting, snapshot (policy)),

                 "early cleanup receipt rejects atomically");
        adk::OneWireStepReceipt failed =
            receipt (release, 1, release.earliestAt.microseconds (), true, false,

                     adk::StatusCode::HardwareFailure);
        require (policy.confirmCleanup (failed.observedAt, failed).error () ==

                     adk::StatusCode::HardwareFailure,
                 "failed cleanup remains attributed");
        require (snapshot (policy).quality ==

                     adk::OneWireTransactionQuality::ReleaseUnconfirmed,
                 "failed cleanup cannot claim release");

        adk::OneWireStepIntent resetRelease = emptyIntent ();

        require (policy.reset (adk::MicrosecondTimePoint (200), resetRelease).ok (),

                 "reset emits new cleanup generation");
        const adk::OneWireStepReceipt resetApplied =
            receipt (resetRelease, 2, resetRelease.earliestAt.microseconds ());

        require (policy.confirmCleanup (resetApplied.observedAt, resetApplied).ok (),

                 "reset cleanup confirms readiness");

        adk::OneWireStepIntent shutdownRelease = emptyIntent ();

        require (

            policy.shutdown (adk::MicrosecondTimePoint (300), shutdownRelease).ok () &&

                policy.initialized (),

            "shutdown remains initialized while closing");
        const adk::OneWireStepReceipt shutdownApplied =
            receipt (shutdownRelease, 3, shutdownRelease.earliestAt.microseconds ());

        require (

            policy.confirmCleanup (shutdownApplied.observedAt, shutdownApplied).ok () &&

                !policy.initialized (),

            "shutdown becomes inert only after cleanup confirmation");
    }

    void testDeadlineAdvanceAndReplay ()

    {
        adk::OneWireTransactionPolicy left (config ());

        adk::OneWireTransactionPolicy right (config ());

        initializeReady (left);

        initializeReady (right);

        adk::OneWireStepIntent             leftIntent  = emptyIntent ();

        adk::OneWireStepIntent             rightIntent = emptyIntent ();

        const adk::OneWireOperationRequest operation =
            request (1, adk::OneWireOperation::ResetPresence, 1000);

        require (

            left.begin (adk::MicrosecondTimePoint (1000), operation, leftIntent)

                    .ok () &&

                right.begin (adk::MicrosecondTimePoint (1000), operation, rightIntent)

                    .ok () &&

                intentEqual (leftIntent, rightIntent),

            "identical begin emits field-identical intent");
        const adk::OneWireTransactionSnapshot pending        = snapshot (left);

        adk::OneWireStepIntent                beforeDeadline = leftIntent;
        require (

            left.advance (adk::MicrosecondTimePoint (20999), beforeDeadline).ok () &&

                snapshotEqual (pending, snapshot (left)),

            "advance before deadline cannot synthesize evidence");
        adk::OneWireStepIntent timedOut = emptyIntent ();

        require (left.advance (adk::MicrosecondTimePoint (21001), timedOut).error () ==

                         adk::StatusCode::Timeout &&
                     timedOut.lineIntent == adk::OneWireLineIntent::Release &&
                     snapshot (left).quality ==

                         adk::OneWireTransactionQuality::ReleaseUnconfirmed,
                 "deadline crossing emits one correlated release intent");
    }

    void testResetPresenceOutcomesAndReceiptReplay ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy);

        adk::OneWireStepIntent intent = emptyIntent ();

        require (policy.begin (adk::MicrosecondTimePoint (1000),

                               request (1, adk::OneWireOperation::ResetPresence, 1000),

                               intent)
                     .ok (),

                 "reset-presence begins");
        uint32_t receiptSequence = 2;
        adk::OneWireStepReceipt first =
            receipt (intent, receiptSequence, intent.earliestAt.microseconds ());
        adk::OneWireStepIntent next = emptyIntent ();

        require (policy.update (first.observedAt, first, next).ok (),

                 "first reset receipt advances");
        const adk::OneWireTransactionSnapshot afterFirst = snapshot (policy);

        adk::OneWireStepIntent replayIntent = emptyIntent ();

        require (policy.update (first.observedAt, first, replayIntent).ok () &&

                     snapshotEqual (afterFirst, snapshot (policy)),

                 "exact receipt replay is idempotent");
        ++first.slotIndex;

        require (policy.update (first.observedAt, first, replayIntent).error () ==

                         adk::StatusCode::InvalidArgument &&
                     snapshotEqual (afterFirst, snapshot (policy)),

                 "changed same-sequence receipt rejects atomically");

        receiptSequence = 3;
        next = applyAt (policy, next, receiptSequence);

        next = applyAt (policy, next, receiptSequence, false);

        next = applyAt (policy, next, receiptSequence, true);

        const adk::OneWireTransactionSnapshot complete = snapshot (policy);

        require (complete.phase == adk::OneWirePhase::Complete &&

                     complete.quality == adk::OneWireTransactionQuality::Complete &&
                     complete.presenceSeen && complete.acceptedSlotCount == 0,

                 "reset-presence completes only after low then released presence");

        adk::OneWireTransactionPolicy absent (config ());

        initializeReady (absent);

        receiptSequence = 2;
        require (absent.begin (adk::MicrosecondTimePoint (1000),

                               request (1, adk::OneWireOperation::ResetPresence, 1000),

                               intent)
                     .ok (),

                 "no-presence fixture begins");
        intent = applyAt (absent, intent, receiptSequence);

        intent = applyAt (absent, intent, receiptSequence);

        intent = applyAt (absent, intent, receiptSequence, true);

        require (snapshot (absent).quality ==

                     adk::OneWireTransactionQuality::ReleaseUnconfirmed,

                 "sampled-high presence window enters fail-closed cleanup");
    }

    void testReadRomBitAssemblyAndWriteOrder ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy);

        uint32_t receiptSequence = 2;
        adk::OneWireStepIntent intent =
            establishPresence (policy, adk::OneWireOperation::ReadRomSingleDrop,

                               receiptSequence);
        const uint8_t expectedRom[8] = {
            0x28, 0x91, 0x00, 0xff, 0x5a, 0xa5, 0x7e, 0x81};

        for (uint16_t slot = 0; slot < 8; ++slot)

        {
            require (intent.phase == adk::OneWirePhase::WriteSlot &&

                         intent.slotIndex == slot &&
                         intent.writeBit == (((0x33U >> slot) & 1U) != 0),

                     "Read ROM command is emitted LSB first");
            do

            {
                intent = applyAt (policy, intent, receiptSequence);

            } while (intent.phase == adk::OneWirePhase::WriteSlot &&

                     intent.slotIndex == slot);
        }
        for (uint16_t slot = 0; slot < 64; ++slot)

        {
            require (intent.phase == adk::OneWirePhase::ReadSlot &&

                         intent.slotIndex == slot,

                     "read slot preserves ordered slot identity");
            const bool high =
                ((expectedRom[slot / 8U] >> (slot % 8U)) & 1U) != 0;
            bool sampled = false;
            do

            {
                const bool value = intent.sampleRequired ? high : true;

                sampled = sampled || intent.sampleRequired;
                intent = applyAt (policy, intent, receiptSequence, value);

            } while (intent.phase == adk::OneWirePhase::ReadSlot &&

                     intent.slotIndex == slot);
            require (sampled, "each read slot requests exactly one sample");
        }
        const adk::OneWireTransactionSnapshot result = snapshot (policy);

        require (result.phase == adk::OneWirePhase::Complete &&

                     result.quality == adk::OneWireTransactionQuality::Complete &&
                     result.readByteCount == 8,

                 "Read ROM trace completes with eight bytes");
        for (uint8_t index = 0; index < 8; ++index)

        {
            require (result.returnedRom.bytes[index] == expectedRom[index],

                     "Read ROM assembles bytes LSB first without qualification");
        }
    }

    void testTimingFailuresAndProducerFaults ()

    {
        adk::OneWireTransactionPolicy latePolicy (config ());

        initializeReady (latePolicy);

        adk::OneWireStepIntent intent = emptyIntent ();

        require (latePolicy
                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             intent)
                     .ok (),

                 "timing fixture begins");
        adk::OneWireStepIntent next = emptyIntent ();
        adk::OneWireStepReceipt late =
            receipt (intent, 2, intent.latestAt.microseconds () + 1U);

        require (latePolicy.update (late.observedAt, late, next).error () ==

                         adk::StatusCode::Timeout &&
                     next.lineIntent == adk::OneWireLineIntent::Release &&
                     snapshot (latePolicy).quality ==

                         adk::OneWireTransactionQuality::ReleaseUnconfirmed,

                 "receipt immediately above timing window fails closed");

        adk::OneWireTransactionPolicy producerPolicy (config ());

        initializeReady (producerPolicy);

        require (producerPolicy
                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             intent)
                     .ok (),

                 "producer-fault fixture begins");
        adk::OneWireStepReceipt rejected =
            receipt (intent, 2, intent.earliestAt.microseconds (), true, false);

        require (producerPolicy.update (rejected.observedAt, rejected, next).error () ==

                         adk::StatusCode::HardwareFailure &&
                     snapshot (producerPolicy).quality ==

                         adk::OneWireTransactionQuality::ReleaseUnconfirmed,

                 "rejected producer receipt fails closed");
    }

    void testSearchRomStraightTraceAndNoDevice ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy);

        uint32_t receiptSequence = 2;
        adk::OneWireStepIntent intent =
            establishPresence (policy, adk::OneWireOperation::SearchRomPass,

                               receiptSequence);
        const uint8_t target[8] = {
            0xa5, 0x5a, 0x3c, 0xc3, 0x96, 0x69, 0xf0, 0x0f};

        for (uint16_t slot = 0; slot < 200; ++slot)

        {
            require (intent.slotIndex == slot,

                     "Search ROM keeps one monotonic physical-slot index");
            bool sampledHigh = true;
            if (slot < 8)

            {
                require (intent.phase == adk::OneWirePhase::WriteSlot &&

                             intent.writeBit == (((0xf0U >> slot) & 1U) != 0),

                         "Search ROM emits the F0 command LSB first");
            }
            else

            {
                const uint16_t searchSlot = static_cast<uint16_t> (slot - 8U);

                const uint8_t bit = static_cast<uint8_t> (searchSlot / 3U);

                const uint8_t offset = static_cast<uint8_t> (searchSlot % 3U);

                const bool targetBit =
                    ((target[bit / 8U] >> (bit % 8U)) & 1U) != 0;
                if (offset < 2)

                {
                    require (intent.phase == adk::OneWirePhase::ReadSlot,

                             "Search ROM reads identity and complement slots");
                    sampledHigh = offset == 0 ? targetBit : !targetBit;
                }
                else

                {
                    require (intent.phase == adk::OneWirePhase::WriteSlot &&

                                 intent.writeBit == targetBit,

                             "Search ROM writes the selected straight branch");
                }
            }
            do

            {
                const bool value =
                    intent.sampleRequired ? sampledHigh : true;

                intent = applyAt (policy, intent, receiptSequence, value);

            } while (intent.phase != adk::OneWirePhase::Complete &&

                     intent.slotIndex == slot);
        }
        const adk::OneWireTransactionSnapshot result = snapshot (policy);

        require (result.phase == adk::OneWirePhase::Complete &&

                     result.quality == adk::OneWireTransactionQuality::Complete &&
                     result.acceptedSlotCount == 200 &&
                     result.searchResult.lastDiscrepancy == 0 &&
                     result.searchResult.lastDevice,

                 "straight Search ROM completes exactly 200 physical slots");
        for (uint8_t index = 0; index < 8; ++index)

        {
            require (result.searchResult.rom.bytes[index] == target[index],

                     "straight Search ROM preserves discovered identity");
        }

        adk::OneWireTransactionPolicy noDevice (config ());

        initializeReady (noDevice);

        receiptSequence = 2;
        intent = establishPresence (noDevice, adk::OneWireOperation::SearchRomPass,

                                    receiptSequence);
        for (uint16_t slot = 0; slot < 10; ++slot)

        {
            const bool high = slot >= 8;

            do

            {
                intent = applyAt (noDevice, intent, receiptSequence,

                                  intent.sampleRequired ? high : true);

            } while (snapshot (noDevice).quality ==

                         adk::OneWireTransactionQuality::Pending &&
                     intent.slotIndex == slot);
        }
        require (snapshot (noDevice).quality ==

                     adk::OneWireTransactionQuality::ReleaseUnconfirmed,

                 "11 search pair fails closed without a branch slot");
    }

    void testAddressedStreamsAndConversionContinuation ()

    {
        const adk::OneWireOperation operations[] = {
            adk::OneWireOperation::MatchRomReadPowerSupply,
            adk::OneWireOperation::MatchRomStartConversion,
            adk::OneWireOperation::MatchRomStartConversion,
            adk::OneWireOperation::MatchRomReadScratchpad};
        const uint8_t commands[] = {0xb4, 0x44, 0x44, 0xbe};
        const uint16_t readCounts[] = {1, 0, 0, 72};
        for (uint8_t operationIndex = 0; operationIndex < 4; ++operationIndex)

        {
            adk::OneWireTransactionPolicy policy (config ());

            initializeReady (policy);

            uint32_t receiptSequence = 2;
            adk::OneWireStepIntent intent =
                establishPresence (policy, operations[operationIndex],

                                   receiptSequence);
            for (uint16_t slot = 0; slot < 80; ++slot)

            {
                uint8_t expectedByte = 0x55;

                uint8_t expectedBit = static_cast<uint8_t> (slot);

                if (slot >= 8 && slot < 72)

                {
                    expectedByte = rom ().bytes[(slot - 8U) / 8U];

                    expectedBit = static_cast<uint8_t> ((slot - 8U) % 8U);

                }
                else if (slot >= 72)

                {
                    expectedByte = commands[operationIndex];

                    expectedBit = static_cast<uint8_t> (slot - 72U);

                }
                require (intent.writeBit ==

                             (((expectedByte >> expectedBit) & 1U) != 0),

                         "addressed operation emits exact LSB-first stream");
                do

                {
                    intent = applyAt (policy, intent, receiptSequence);

                } while (intent.phase == adk::OneWirePhase::WriteSlot &&

                         intent.slotIndex == slot);
            }
            for (uint16_t slot = 0; slot < readCounts[operationIndex]; ++slot)

            {
                do

                {
                    intent = applyAt (policy, intent, receiptSequence,

                                      intent.sampleRequired ?
                                          ((slot & 1U) != 0) :
                                          true);

                } while (intent.phase == adk::OneWirePhase::ReadSlot &&

                         intent.slotIndex == slot);
            }
            require (snapshot (policy).quality ==

                         adk::OneWireTransactionQuality::Complete,

                     "addressed typed operation completes exact stream");

            if (operations[operationIndex] ==

                adk::OneWireOperation::MatchRomStartConversion)

            {
                const adk::OneWireStepReceipt completionRelease =
                    receipt (intent, receiptSequence++,

                             intent.earliestAt.microseconds ());

                require (

                    policy
                        .confirmCleanup (completionRelease.observedAt,

                                         completionRelease)
                        .ok (),

                    "conversion completion release confirms before continuation");

                adk::OneWireOperationRequest continuation =
                    request (2,

                             adk::OneWireOperation::MatchRomReadConversionStatus,

                             intent.earliestAt.microseconds ());

                adk::OneWireOperationRequest wrongRom = continuation;

                ++wrongRom.addressedRom.bytes[0];

                require (

                    policy.begin (wrongRom.startedAt, wrongRom, intent).error () ==

                            adk::StatusCode::InvalidArgument,

                    "conversion status rejects a different addressed ROM");

                require (policy.begin (continuation.startedAt, continuation, intent)
                             .ok () &&

                             intent.phase == adk::OneWirePhase::ReadSlot &&
                             intent.slotIndex == 0,

                         "conversion status continues directly with one read slot");
                do

                {
                    intent = applyAt (policy, intent, receiptSequence,

                                      operationIndex == 2);

                } while (snapshot (policy).quality ==

                         adk::OneWireTransactionQuality::Pending);
                const adk::OneWireTransactionSnapshot conversion = snapshot (policy);

                require (conversion.readByteCount == 1 &&

                             conversion.readBytes[0] ==
                                 (operationIndex == 2 ? 1 : 0) &&
                             conversion.acceptedSlotCount == 1,

                         "conversion status reports either copied bit without retry");

                const adk::OneWireStepReceipt statusRelease =
                    receipt (intent, receiptSequence++,

                             intent.earliestAt.microseconds ());

                require (

                    policy
                        .confirmCleanup (statusRelease.observedAt, statusRelease)

                        .ok (),

                    "conversion-status terminal release confirms");

                adk::OneWireOperationRequest repeated = request (

                    3, adk::OneWireOperation::MatchRomReadConversionStatus,

                    statusRelease.observedAt.microseconds ());

                require (

                    policy.begin (repeated.startedAt, repeated, intent).error () ==

                        adk::StatusCode::InvalidArgument,

                    "second conversion-status request requires a new conversion");
            }
        }
    }

    adk::OneWireStepIntent completeOperation (
        adk::OneWireTransactionPolicy& policy, adk::OneWireStepIntent intent,
        uint32_t& receiptSequence, bool conversionHigh)
    {
        for (uint16_t step = 0;
             snapshot (policy).quality ==
                 adk::OneWireTransactionQuality::Pending;
             ++step)

        {
            require (step < 1400, "operation completes within bounded substeps");

            bool sampledHigh = true;
            if (intent.phase == adk::OneWirePhase::PresenceWindow)

            {
                sampledHigh = snapshot (policy).presenceSeen;
            }
            else if (intent.operation == adk::OneWireOperation::SearchRomPass &&

                     intent.phase == adk::OneWirePhase::ReadSlot &&
                     intent.sampleRequired)

            {
                sampledHigh = ((intent.slotIndex - 8U) % 3U) == 1U;
            }
            else if (intent.operation ==
                         adk::OneWireOperation::MatchRomReadConversionStatus &&

                     intent.sampleRequired)

            {
                sampledHigh = conversionHigh;
            }
            intent = applyAt (policy, intent, receiptSequence, sampledHigh);
        }
        return intent;
    }

    void testEveryOperationPublishesConfirmedEvidence ()

    {
        struct Case
        {
            adk::OneWireOperation operation;
            bool                  conversionHigh;
        };
        const Case cases[] = {
            {adk::OneWireOperation::ResetPresence, false},
            {adk::OneWireOperation::SearchRomPass, false},
            {adk::OneWireOperation::ReadRomSingleDrop, false},
            {adk::OneWireOperation::MatchRomReadPowerSupply, false},
            {adk::OneWireOperation::MatchRomStartConversion, false},
            {adk::OneWireOperation::MatchRomReadConversionStatus, false},
            {adk::OneWireOperation::MatchRomReadConversionStatus, true},
            {adk::OneWireOperation::MatchRomReadScratchpad, false}};
        for (const Case& value : cases)

        {
            adk::OneWireTransactionPolicy policy (config ());

            initializeReady (policy);

            uint32_t               receiptSequence = 2;
            adk::OneWireStepIntent intent          = emptyIntent ();
            uint32_t               startedAt       = 1000;
            if (value.operation ==
                adk::OneWireOperation::MatchRomReadConversionStatus)

            {
                const adk::OneWireOperationRequest precursor =
                    request (1,
                             adk::OneWireOperation::MatchRomStartConversion,
                             startedAt);

                require (
                    policy
                        .begin (adk::MicrosecondTimePoint (startedAt), precursor,
                                intent)
                        .ok (),

                    "conversion-status fixture starts conversion");
                intent = completeOperation (policy, intent, receiptSequence, false);

                require (intent.lineIntent == adk::OneWireLineIntent::Release,

                         "conversion precursor ends with release intent");
                const adk::OneWireStepReceipt precursorRelease =
                    receipt (intent, receiptSequence++,
                             intent.earliestAt.microseconds ());

                require (policy
                             .confirmCleanup (precursorRelease.observedAt,
                                              precursorRelease)
                             .ok (),

                         "terminal conversion cleanup confirms before continuation");
                startedAt = precursorRelease.observedAt.microseconds ();
            }

            require (
                policy
                    .begin (adk::MicrosecondTimePoint (startedAt),
                            request (
                                value.operation ==
                                        adk::OneWireOperation::
                                            MatchRomReadConversionStatus ?
                                    2 :
                                    1,
                                value.operation, startedAt),
                            intent)
                    .ok (),

                "operation table row begins");
            intent =
                completeOperation (policy, intent, receiptSequence,
                                   value.conversionHigh);

            require (intent.lineIntent == adk::OneWireLineIntent::Release,

                     "every successful operation ends with release intent");
            adk::OneWireTransactionSnapshot evidence = snapshot (policy);

            const adk::Status pendingEvidence =
                policy.completedEvidence (evidence);
            require (pendingEvidence.error () == adk::StatusCode::ResourceBusy,

                     "evidence remains unavailable before terminal release");
            const adk::OneWireStepReceipt terminalRelease =
                receipt (intent, receiptSequence++,
                         intent.earliestAt.microseconds ());

            require (
                policy
                        .confirmCleanup (terminalRelease.observedAt,
                                         terminalRelease)
                        .ok () &&

                    policy.completedEvidence (evidence).ok () &&
                    evidence.operation == value.operation &&
                    evidence.phase == adk::OneWirePhase::Complete &&
                    evidence.quality ==
                        adk::OneWireTransactionQuality::Complete &&
                    evidence.releaseRequested && evidence.releaseConfirmed &&
                    (value.operation !=
                         adk::OneWireOperation::MatchRomReadConversionStatus ||
                     (evidence.readByteCount == 1 &&
                      evidence.readBytes[0] ==
                          (value.conversionHigh ? 1U : 0U))),

                "confirmed release publishes complete operation evidence");
        }
    }
} // namespace

int main ()

{
    testLifecycleAndConfiguration ();

    testRequestValidationAndAtomicity ();

    testCleanupCorrelationAndShutdown ();

    testDeadlineAdvanceAndReplay ();

    testResetPresenceOutcomesAndReceiptReplay ();

    testReadRomBitAssemblyAndWriteOrder ();

    testTimingFailuresAndProducerFaults ();

    testSearchRomStraightTraceAndNoDevice ();

    testAddressedStreamsAndConversionContinuation ();

    testEveryOperationPublishesConfirmedEvidence ();

}
// clang-format on
