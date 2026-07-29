// clang-format off

#include <one_wire_transaction_policy.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace {
    struct SearchTrace
    {
        bool idHigh[64];
        bool complementHigh[64];
    };

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::OneWireRomCode emptyRom ()
    {
        return {{0, 0, 0, 0, 0, 0, 0, 0}};
    }

    adk::OneWireTransactionConfig config ()
    {
        return {0x12345678UL,
                11,
                7,
                17,
                false,
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
                200};
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
                emptyRom                  ()};
    }

    adk::OneWireOperationRequest request (uint32_t sequence, uint32_t startedAt,
                                          const adk::OneWireSearchState& search)
    {
        return {sequence,
                adk::OneWireOperation::SearchRomPass,
                emptyRom (),
                search,
                adk::MicrosecondTimePoint (startedAt),
                adk::OneWireSupplyMode::ExternallyPowered,
                adk::StatusCode::Ok};
    }

    adk::OneWireStepReceipt receipt (const adk::OneWireStepIntent& intent,
                                     uint32_t sequence, bool sampledHigh)
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
                true,
                adk::StatusCode::Ok};
    }

    bool sameRom (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)
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
               left.latestAt.microseconds   () == right.latestAt.microseconds () &&
               sameRom                      (left.addressedRom, right.addressedRom);
    }

    adk::OneWireTransactionSnapshot
    snapshot (const adk::OneWireTransactionPolicy& policy)
    {
        adk::OneWireTransactionSnapshot value = {
            adk::OneWireOperation::ResetPresence,
            adk::OneWirePhase::Inert,
            adk::OneWireTransactionQuality::Unqualified,
            request   (0, 0, {emptyRom (), 0, false}),
            {emptyRom (), 0, false},
            emptyRom  (),
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
            !sameRom (left.request.search.rom, right.request.search.rom) ||
            left.request.search.lastDiscrepancy !=
                right.request.search.lastDiscrepancy ||
            left.request.search.lastDevice != right.request.search.lastDevice ||
            !sameRom (left.searchResult.rom, right.searchResult.rom) ||
            left.searchResult.lastDiscrepancy != right.searchResult.lastDiscrepancy ||
            left.searchResult.lastDevice != right.searchResult.lastDevice ||
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

    void initialize (adk::OneWireTransactionPolicy& policy)
    {
        adk::OneWireStepIntent intent = emptyIntent ();
        require                                     (policy.initialize (adk::MicrosecondTimePoint (100), intent).ok (),
                 "search policy initializes");
        const adk::OneWireStepReceipt applied = receipt (intent, 1, true);
        require                                         (policy.confirmCleanup (applied.observedAt, applied).ok (),
                 "initial release confirms");
    }

    adk::OneWireStepIntent apply (adk::OneWireTransactionPolicy& policy,
                                  const adk::OneWireStepIntent&  intent,
                                  uint32_t& sequence, bool sampledHigh, bool duplicate)
    {
        const adk::OneWireStepReceipt applied =
            receipt (intent, sequence++, sampledHigh);
        adk::OneWireStepIntent next = emptyIntent ();
        require                                   (policy.update (applied.observedAt, applied, next).ok (),
                 "search receipt advances");
        if (duplicate)
        {
            const adk::OneWireTransactionSnapshot before = snapshot    (policy);
            adk::OneWireStepIntent                replay = emptyIntent ();
            require                                                    (policy.update (applied.observedAt, applied, replay).ok (),
                     "exact duplicate receipt is accepted");
            require (sameIntent (next, replay),
                     "exact duplicate reproduces the pending intent");
            require (sameSnapshot (before, snapshot (policy)),
                     "exact duplicate does not advance search state");
        }
        return next;
    }

    adk::OneWireStepIntent beginSearch (adk::OneWireTransactionPolicy& policy,
                                        const adk::OneWireSearchState& search,
                                        uint32_t& sequence, bool duplicate)
    {
        adk::OneWireStepIntent             intent    = emptyIntent ();
        const adk::OneWireOperationRequest operation = request     (1, 1000, search);
        require                                                    (policy.begin (operation.startedAt, operation, intent).ok (),
                 "Search ROM pass begins");
        intent = apply (policy, intent, sequence, true, duplicate);
        intent = apply (policy, intent, sequence, true, duplicate);
        intent = apply (policy, intent, sequence, false, duplicate);
        intent = apply (policy, intent, sequence, true, duplicate);
        return intent;
    }

    adk::OneWireTransactionSnapshot runSearch (adk::OneWireTransactionPolicy& policy,
                                               const adk::OneWireSearchState& search,
                                               const SearchTrace& trace, bool duplicate)
    {
        uint32_t               sequence = 2;
        adk::OneWireStepIntent intent =
            beginSearch (policy, search, sequence, duplicate);

        while (snapshot (policy).quality == adk::OneWireTransactionQuality::Pending)
        {
            const uint16_t slot        = intent.slotIndex;
            bool           sampledHigh = true;
            if (slot < 8)
            {
                require (intent.phase == adk::OneWirePhase::WriteSlot,
                         "search command uses write slots");
                require (intent.writeBit == (((0xf0U >> slot) & 1U) != 0),
                         "search command is F0 LSB first");
            }
            else
            {
                const uint16_t searchSlot = static_cast<uint16_t> (slot - 8U);
                const uint8_t  bit        = static_cast<uint8_t> (searchSlot / 3U);
                const uint8_t  offset     = static_cast<uint8_t> (searchSlot % 3U);
                if (offset == 0)
                {
                    require (intent.phase == adk::OneWirePhase::ReadSlot,
                             "identity bit uses a read slot");
                    sampledHigh = trace.idHigh[bit];
                }
                else if (offset == 1)
                {
                    require (intent.phase == adk::OneWirePhase::ReadSlot,
                             "complement bit uses a read slot");
                    sampledHigh = trace.complementHigh[bit];
                }
                else
                {
                    require (intent.phase == adk::OneWirePhase::WriteSlot,
                             "selected branch uses a write slot");
                }
            }
            do
            {
                intent = apply (policy, intent, sequence,
                                intent.sampleRequired ? sampledHigh : true, duplicate);
            }
            while (snapshot (policy).quality ==
                       adk::OneWireTransactionQuality::Pending &&
                   intent.slotIndex == slot);
        }
        return snapshot (policy);
    }

    SearchTrace straightTrace (const adk::OneWireRomCode& rom)
    {
        SearchTrace trace = {};
        for (uint8_t bit = 0; bit < 64; ++bit)
        {
            const bool high           = ((rom.bytes[bit / 8U] >> (bit % 8U)) & 1U) != 0;
            trace.idHigh[bit]         = high;
            trace.complementHigh[bit] = !high;
        }
        return trace;
    }

    void addCollision (SearchTrace& trace, uint8_t bitNumber)
    {
        trace.idHigh[bitNumber - 1U]         = false;
        trace.complementHigh[bitNumber - 1U] = false;
    }

    void testCollisionFreePass ()
    {
        const adk::OneWireRomCode rom = {
            {0x28, 0x91, 0x42, 0x24, 0x18, 0x81, 0x7e, 0x55}};
        const SearchTrace trace = straightTrace (rom);

        adk::OneWireTransactionPolicy policy (config ());
        initialize                           (policy);
        const adk::OneWireTransactionSnapshot result =
            runSearch (policy, {emptyRom (), 0, false}, trace, false);

        require (result.phase == adk::OneWirePhase::Complete &&
                     result.quality == adk::OneWireTransactionQuality::Complete &&
                     result.status.ok () && result.acceptedSlotCount == 200,
                 "collision-free Search completes all 200 physical slots");
        require (sameRom (result.searchResult.rom, rom) &&
                     result.searchResult.lastDiscrepancy == 0 &&
                     result.searchResult.lastDevice,
                 "collision-free Search returns the sole ROM and final marker");
        require (result.ownerToken == 0x12345678UL &&
                     result.configurationRevision == 11 &&
                     result.request.requestSequence == 1 &&
                     result.transactionGeneration != 0,
                 "collision-free Search preserves transaction provenance");
    }

    void testCancellationCleanupAndProvenance ()
    {
        adk::OneWireTransactionPolicy policy (config ());
        initialize                           (policy);

        uint32_t               sequence = 2;
        adk::OneWireStepIntent intent =
            beginSearch (policy, {emptyRom (), 0, false}, sequence, false);
        intent = apply (policy, intent, sequence, true, false);

        const adk::OneWireTransactionSnapshot active = snapshot (policy);
        const uint32_t activeLifecycleGeneration = active.lifecycleGeneration;
        require (policy.cancel (intent.earliestAt, intent).error () ==
                     adk::StatusCode::InvalidArgument,
                 "Search cancellation reports its attributed cause");
        require (intent.lineIntent == adk::OneWireLineIntent::Release &&
                     intent.phase == adk::OneWirePhase::RollingBack &&
                     intent.ownerToken == active.ownerToken &&
                     intent.configurationRevision == active.configurationRevision &&
                     intent.requestSequence == active.request.requestSequence &&
                     intent.transactionGeneration == active.transactionGeneration &&
                     intent.lifecycleGeneration > activeLifecycleGeneration,
                 "Search cancellation emits correlated release cleanup");

        const adk::OneWireTransactionSnapshot releasing = snapshot (policy);

        require (releasing.phase == adk::OneWirePhase::RollingBack &&
                     releasing.quality ==
                         adk::OneWireTransactionQuality::ReleaseUnconfirmed &&
                     releasing.releaseRequested && !releasing.releaseConfirmed &&
                     releasing.status.error () == adk::StatusCode::InvalidArgument,
                 "Search cancellation exposes release cleanup");
        require (releasing.ownerToken == active.ownerToken,
                 "Search cancellation preserves owner provenance");
        require (releasing.lifecycleGeneration == intent.lifecycleGeneration,
                 "Search cancellation publishes cleanup lifecycle provenance");
        require (releasing.configurationRevision == active.configurationRevision,
                 "Search cancellation preserves configuration provenance");
        require (releasing.transactionGeneration == active.transactionGeneration,
                 "Search cancellation preserves transaction provenance");

        const adk::OneWireStepReceipt cleanup = receipt (intent, sequence++, true);

        require (policy.confirmCleanup (cleanup.observedAt, cleanup).ok (),
                 "Search cancellation cleanup confirms");
        const adk::OneWireTransactionSnapshot cancelled = snapshot (policy);

        require (cancelled.phase == adk::OneWirePhase::Fault &&
                     cancelled.quality ==
                         adk::OneWireTransactionQuality::ProducerFault &&
                     cancelled.releaseRequested && cancelled.releaseConfirmed &&
                     cancelled.status.error () == adk::StatusCode::InvalidArgument,
                 "Search cancellation remains attributed after cleanup");
        require (cancelled.ownerToken == active.ownerToken &&
                     cancelled.lifecycleGeneration == intent.lifecycleGeneration &&
                     cancelled.configurationRevision ==
                         active.configurationRevision &&
                     cancelled.request.requestSequence == intent.requestSequence &&
                     cancelled.transactionGeneration ==
                         active.transactionGeneration,
                 "confirmed Search cancellation preserves provenance");
    }

    void testCollisionAndResumedBranch ()
    {
        const adk::OneWireRomCode base = {
            {0x5a, 0xa5, 0x3c, 0xc3, 0x96, 0x69, 0xf0, 0x0f}};
        SearchTrace trace = straightTrace (base);
        addCollision                      (trace, 3);
        addCollision                      (trace, 7);
        addCollision                      (trace, 20);

        adk::OneWireTransactionPolicy firstPolicy (config ());
        initialize                                (firstPolicy);
        const adk::OneWireTransactionSnapshot first =
            runSearch (firstPolicy, {emptyRom (), 0, false}, trace, false);

        require (first.phase == adk::OneWirePhase::Complete &&
                     first.quality == adk::OneWireTransactionQuality::Complete &&
                     first.acceptedSlotCount == 200,
                 "collision pass completes all 200 physical slots");
        require (first.searchResult.lastDiscrepancy == 20 &&
                     !first.searchResult.lastDevice,
                 "first collision pass records the final zero branch");
        require ((first.searchResult.rom.bytes[0] & 0x44U) == 0 &&
                     (first.searchResult.rom.bytes[2] & 0x08U) == 0,
                 "first pass selects zero at every collision");

        adk::OneWireTransactionPolicy resumedPolicy (config ());
        initialize                                  (resumedPolicy);
        const adk::OneWireTransactionSnapshot resumed =
            runSearch (resumedPolicy, first.searchResult, trace, false);

        require (resumed.searchResult.lastDiscrepancy == 7 &&
                     !resumed.searchResult.lastDevice,
                 "resumed pass retains the latest earlier zero branch");
        require ((resumed.searchResult.rom.bytes[2] & 0x08U) != 0,
                 "resumed pass selects one at the prior discrepancy");
        require ((resumed.searchResult.rom.bytes[0] & 0x44U) == 0,
                 "resumed pass preserves prior collision branches");
    }

    void testPriorPathSelection ()
    {
        const adk::OneWireRomCode base = {
            {0x91, 0x42, 0x24, 0x18, 0x81, 0x7e, 0x55, 0xaa}};
        SearchTrace trace = straightTrace (base);
        addCollision                      (trace, 3);
        addCollision                      (trace, 7);
        addCollision                      (trace, 20);
        addCollision                      (trace, 33);

        adk::OneWireSearchState prior = {emptyRom (), 20, false};
        prior.rom.bytes[0]            = 0x04;

        adk::OneWireTransactionPolicy policy (config ());
        initialize                           (policy);
        const adk::OneWireTransactionSnapshot result =
            runSearch (policy, prior, trace, false);

        require ((result.searchResult.rom.bytes[0] & 0x04U) != 0 &&
                     (result.searchResult.rom.bytes[0] & 0x40U) == 0,
                 "bits before discrepancy follow the prior ROM path");
        require ((result.searchResult.rom.bytes[2] & 0x08U) != 0,
                 "bit at prior discrepancy selects the resumed one branch");
        require ((result.searchResult.rom.bytes[4] & 0x01U) == 0 &&
                     result.searchResult.lastDiscrepancy == 33 &&
                     !result.searchResult.lastDevice,
                 "later collision selects zero and becomes next discrepancy");
    }

    void testNoDevicePair ()
    {
        SearchTrace trace       = straightTrace (emptyRom ());
        trace.idHigh[0]         = true;
        trace.complementHigh[0] = true;

        adk::OneWireTransactionPolicy policy (config ());
        initialize                           (policy);
        const adk::OneWireTransactionSnapshot failed =
            runSearch (policy, {emptyRom (), 0, false}, trace, false);

        require (failed.quality == adk::OneWireTransactionQuality::ReleaseUnconfirmed &&
                     failed.releaseRequested && !failed.releaseConfirmed &&
                     failed.acceptedSlotCount == 10,
                 "11 pair stops before a branch slot and requests cleanup");
    }

    void testExactDuplicatesAndTwinReplay ()
    {
        const adk::OneWireRomCode base = {
            {0x28, 0x01, 0x7e, 0x80, 0x55, 0xaa, 0x33, 0xcc}};
        SearchTrace trace = straightTrace (base);
        addCollision                      (trace, 1);
        addCollision                      (trace, 16);
        addCollision                      (trace, 64);

        adk::OneWireTransactionPolicy duplicatePolicy (config ());
        initialize                                    (duplicatePolicy);
        const adk::OneWireTransactionSnapshot duplicate =
            runSearch (duplicatePolicy, {emptyRom (), 0, false}, trace, true);

        adk::OneWireTransactionPolicy left  (config ());
        adk::OneWireTransactionPolicy right (config ());
        initialize                          (left);
        initialize                          (right);
        const adk::OneWireTransactionSnapshot leftResult =
            runSearch (left, {emptyRom (), 0, false}, trace, false);
        const adk::OneWireTransactionSnapshot rightResult =
            runSearch (right, {emptyRom (), 0, false}, trace, false);

        require (sameSnapshot (leftResult, rightResult),
                 "twin replay produces byte-identical search snapshots");
        require (sameSnapshot (duplicate, leftResult),
                 "exact receipt duplicates preserve the canonical replay");
        require (leftResult.searchResult.lastDiscrepancy == 64 &&
                     !leftResult.searchResult.lastDevice,
                 "twin collision replay preserves final discrepancy");
    }

    void testCapacityAndForeignReceiptRejection ()
    {
        adk::OneWireTransactionConfig undersized = config ();
        undersized.maximumSlots                  = 199;
        adk::OneWireTransactionPolicy capacityPolicy (undersized);
        initialize                                   (capacityPolicy);

        const adk::OneWireTransactionSnapshot capacityBefore =
            snapshot (capacityPolicy);
        adk::OneWireStepIntent capacityIntent = emptyIntent ();
        require                                             (
            capacityPolicy
                    .begin (adk::MicrosecondTimePoint (1000),
                            request (1, 1000, {emptyRom (), 0, false}),
                            capacityIntent)
                    .error () == adk::StatusCode::CapacityExceeded,
            "Search ROM rejects an exact 199-slot budget");
        require (sameSnapshot (capacityBefore, snapshot (capacityPolicy)),
                 "Search capacity rejection leaves policy state atomic");

        adk::OneWireTransactionPolicy provenancePolicy (config ());
        initialize                                     (provenancePolicy);
        adk::OneWireStepIntent current = emptyIntent   ();
        require                                        (
            provenancePolicy
                .begin (adk::MicrosecondTimePoint (1000),
                        request (1, 1000, {emptyRom (), 0, false}), current)
                .ok (),
            "Search provenance fixture begins");

        const adk::OneWireTransactionSnapshot provenanceBefore =
            snapshot (provenancePolicy);
        adk::OneWireStepReceipt foreign = receipt                   (current, 2, true);
        foreign.observedAt              = adk::MicrosecondTimePoint (21001);
        ++foreign.ownerToken;
        adk::OneWireStepIntent ignored = emptyIntent ();
        require                                      (
            provenancePolicy
                    .update (foreign.observedAt, foreign, ignored)
                    .error  () == adk::StatusCode::InvalidArgument,
            "late foreign Search receipt rejects by provenance");
        require (sameSnapshot (provenanceBefore, snapshot (provenancePolicy)),
                 "late foreign Search receipt leaves state atomic");
    }
} // namespace

int main ()
{
    testCollisionFreePass                  ();
    testCancellationCleanupAndProvenance   ();
    testCollisionAndResumedBranch          ();
    testPriorPathSelection                 ();
    testNoDevicePair                       ();
    testExactDuplicatesAndTwinReplay       ();
    testCapacityAndForeignReceiptRejection ();
}

// clang-format on
