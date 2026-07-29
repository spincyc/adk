#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <one_wire_transaction_policy.h>
namespace {
    constexpr uint32_t halfRange = 0x80000000UL;
    void               require (bool condition, const char* message)

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
    adk::OneWireTransactionConfig config (uint16_t maximumSlots = 256)

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

                adk::MicrosecondDuration (20),

                adk::MicrosecondDuration (55),

                adk::MicrosecondDuration (60),

                adk::MicrosecondDuration (240),

                adk::MicrosecondDuration (60),

                adk::MicrosecondDuration (120),

                adk::MicrosecondDuration (1),

                adk::MicrosecondDuration (15),

                adk::MicrosecondDuration (2),

                adk::MicrosecondDuration (14),

                adk::MicrosecondDuration (16),

                adk::MicrosecondDuration (45),

                adk::MicrosecondDuration (61),

                adk::MicrosecondDuration (120),

                adk::MicrosecondDuration (3),

                adk::MicrosecondDuration (20),

                adk::MicrosecondDuration (20000),

                maximumSlots};
    }
    adk::OneWireOperationRequest
    request (uint32_t sequence, adk::OneWireOperation operation, uint32_t startedAt)

    {
        return {sequence,
                operation,
                rom (),

                {rom (), 0, false},

                adk::MicrosecondTimePoint (startedAt),

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

                                     uint32_t sequence, uint32_t observedAt,
                                     bool sampledHigh = true)
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
                true,
                adk::StatusCode::Ok};
    }
    void initializeReady (adk::OneWireTransactionPolicy& policy, uint32_t now)

    {
        adk::OneWireStepIntent release = emptyIntent ();

        require (policy.initialize (adk::MicrosecondTimePoint (now), release).ok (),

                 "policy initializes");
        const adk::OneWireStepReceipt applied = receipt (release, 1, now);

        require (policy.confirmCleanup (adk::MicrosecondTimePoint (now), applied).ok (),

                 "initial release confirms");
    }
    adk::OneWireStepIntent apply (adk::OneWireTransactionPolicy& policy,

                                  const adk::OneWireStepIntent& intent,
                                  uint32_t& sequence, bool sampledHigh = true)
    {
        adk::OneWireStepIntent next = emptyIntent ();

        const uint32_t at = intent.earliestAt.microseconds ();

        require (policy

                     .update (adk::MicrosecondTimePoint (at),

                              receipt (intent, sequence++, at, sampledHigh), next)

                     .ok (),

                 "fixture advances at earliest boundary");
        return next;
    }
    adk::OneWireStepIntent intentAt (adk::OneWireTransactionPolicy& policy,

                                     uint8_t target, uint32_t startedAt,
                                     uint32_t& sequence)
    {
        adk::OneWireStepIntent intent = emptyIntent ();

        require (policy

                     .begin (adk::MicrosecondTimePoint (startedAt),

                             request (1, adk::OneWireOperation::ReadRomSingleDrop,

                                      startedAt),
                             intent)
                     .ok (),

                 "timing fixture begins");
        uint8_t seen = 0;
        while (seen < target)
        {
            const bool presenceSample =
                intent.phase == adk::OneWirePhase::PresenceWindow &&
                intent.sampleRequired && seen == 2;
            intent = apply (policy, intent, sequence, !presenceSample);

            ++seen;
        }
        return intent;
    }
    enum struct WindowBoundary : uint8_t
    {
        BelowLower,
        AtLower,
        AboveLower,
        BelowUpper,
        AtUpper,
        AboveUpper
    };
    uint32_t boundaryTime (const adk::OneWireStepIntent& intent,

                           WindowBoundary boundary)
    {
        switch (boundary)
        {
            case WindowBoundary::BelowLower:
                return intent.earliestAt.microseconds () - 1U;

            case WindowBoundary::AtLower: return intent.earliestAt.microseconds ();

            case WindowBoundary::AboveLower:
                return intent.earliestAt.microseconds () + 1U;

            case WindowBoundary::BelowUpper:
                return intent.latestAt.microseconds () - 1U;

            case WindowBoundary::AtUpper: return intent.latestAt.microseconds ();

            case WindowBoundary::AboveUpper:
                return intent.latestAt.microseconds () + 1U;
        }
        return 0;
    }
    void verifyBoundary (uint8_t target, WindowBoundary boundary, bool accepted)

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy, 100);

        uint32_t                     sequence = 2;
        const adk::OneWireStepIntent intent = intentAt (policy, target, 1000, sequence);

        const uint32_t at = boundaryTime (intent, boundary);

        adk::OneWireStepIntent next = emptyIntent ();

        const adk::Status status = policy.update (

            adk::MicrosecondTimePoint (at),

            receipt (intent, sequence, at,

                     intent.sampleRequired
                         ? intent.phase != adk::OneWirePhase::PresenceWindow
                         : true),
            next);
        require (status.ok () == accepted, "runtime timing boundary result");

        if (!accepted)
        {
            require (status.error () == adk::StatusCode::Timeout,

                     "out-of-window receipt times out");
            require (next.lineIntent == adk::OneWireLineIntent::Release,

                     "out-of-window receipt fails closed");
        }
    }
    void testEveryRuntimeWindow ()

    {
        // Reset low, reset-release/presence-start intersection, presence low,
        // write-one low, slot complete, recovery, and write-zero low.
        const uint8_t windowTargets[] = {1, 2, 3, 5, 6, 7, 13};
        for (uint8_t target : windowTargets)
        {
            verifyBoundary (target, WindowBoundary::BelowLower, false);

            verifyBoundary (target, WindowBoundary::AtLower, true);

            verifyBoundary (target, WindowBoundary::AboveLower, true);

            verifyBoundary (target, WindowBoundary::BelowUpper, true);

            verifyBoundary (target, WindowBoundary::AtUpper, true);

            verifyBoundary (target, WindowBoundary::AboveUpper, false);
        }
        // Advance through the eight command slots to the first read slot:
        // drive, read-initiate release, read sample, complete, and recovery.
        const uint8_t readTargets[] = {37, 38, 39, 40};
        for (uint8_t target : readTargets)
        {
            verifyBoundary (target, WindowBoundary::BelowLower, false);

            verifyBoundary (target, WindowBoundary::AtLower, true);

            verifyBoundary (target, WindowBoundary::AboveLower, true);

            verifyBoundary (target, WindowBoundary::BelowUpper, true);

            verifyBoundary (target, WindowBoundary::AtUpper, true);

            verifyBoundary (target, WindowBoundary::AboveUpper, false);
        }
    }
    void testRolloverBackwardAndHalfRange ()

    {
        const uint32_t start = std::numeric_limits<uint32_t>::max () - 200U;

        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy, start - 1U);

        uint32_t               sequence = 2;
        adk::OneWireStepIntent intent   = intentAt (policy, 1, start, sequence);

        require (intent.earliestAt.microseconds () < start,

                 "reset-low window wraps across zero");
        intent = apply (policy, intent, sequence);

        require (intent.phase == adk::OneWirePhase::PresenceWindow,

                 "wrapped in-window receipt advances");
        adk::OneWireTransactionPolicy backward (config ());

        initializeReady (backward, 100);

        adk::OneWireStepIntent current = emptyIntent ();

        require (backward

                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             current)
                     .ok (),

                 "backward-time fixture begins");
        adk::OneWireStepIntent next = emptyIntent ();

        require (backward.update (adk::MicrosecondTimePoint (999),

                                  receipt (current, 2, 999), next)

                         .error () == adk::StatusCode::InvalidArgument,

                 "ordinary backward receipt time rejects");
        require (backward.update (adk::MicrosecondTimePoint (1000U + halfRange),

                                  receipt (current, 2, 1000U + halfRange), next)

                         .error () == adk::StatusCode::InvalidArgument,

                 "exact half-range receipt time rejects");
    }
    void testOverallDeadline ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy, 100);

        adk::OneWireStepIntent current = emptyIntent ();

        require (policy

                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             current)
                     .ok (),

                 "deadline fixture begins");
        adk::OneWireStepIntent release = emptyIntent ();

        require (policy.advance (adk::MicrosecondTimePoint (21000), release).ok (),

                 "exact overall deadline remains pending");
        adk::OneWireTransactionSnapshot snapshot;
        require (policy.snapshot (snapshot).ok () &&

                     snapshot.quality == adk::OneWireTransactionQuality::Pending,
                 "exact overall deadline preserves pending state");
        require (policy.advance (adk::MicrosecondTimePoint (21001), release).error () ==

                         adk::StatusCode::Timeout &&
                     release.lineIntent == adk::OneWireLineIntent::Release,
                 "first tick after overall deadline emits cleanup");
        adk::OneWireStepIntent sentinel = emptyIntent ();

        sentinel.ownerToken = UINT32_MAX;
        sentinel.lineIntent = adk::OneWireLineIntent::DriveLow;
        require (policy.advance (adk::MicrosecondTimePoint (21002), sentinel).ok () &&

                     sentinel.ownerToken == UINT32_MAX &&
                     sentinel.lineIntent == adk::OneWireLineIntent::DriveLow,
                 "repeated timeout emits no second cleanup intent");
        require (policy.snapshot (snapshot).ok () &&

                     snapshot.quality ==
                         adk::OneWireTransactionQuality::ReleaseUnconfirmed &&
                     snapshot.status.error () == adk::StatusCode::Timeout,

                 "repeated timeout preserves pending cleanup and terminal status");
    }
    void testGlobalTimeOnCommands ()

    {
        const uint32_t invalidTimes[] = {99, 100U + halfRange};
        for (uint32_t invalidAt : invalidTimes)
        {
            adk::OneWireTransactionPolicy initializePolicy (config ());

            initializeReady (initializePolicy, 100);

            adk::OneWireStepIntent intent = emptyIntent ();

            require (initializePolicy

                             .initialize (adk::MicrosecondTimePoint (invalidAt), intent)

                             .error () == adk::StatusCode::InvalidArgument,

                     "initialize rejects invalid global time");

            adk::OneWireTransactionPolicy beginPolicy (config ());

            initializeReady (beginPolicy, 100);

            require (beginPolicy

                             .begin (adk::MicrosecondTimePoint (invalidAt),

                                     request (1, adk::OneWireOperation::ResetPresence,

                                              invalidAt),
                                     intent)
                             .error () == adk::StatusCode::InvalidArgument,

                     "begin rejects invalid global time");

            adk::OneWireTransactionPolicy advancePolicy (config ());

            initializeReady (advancePolicy, 100);

            require (

                advancePolicy
                    .begin (adk::MicrosecondTimePoint (1000),

                            request (1, adk::OneWireOperation::ResetPresence, 1000),

                            intent)
                    .ok (),

                "advance-time fixture begins");
            require (

                advancePolicy
                        .advance (adk::MicrosecondTimePoint (invalidAt), intent)

                        .error () == adk::StatusCode::InvalidArgument,

                "advance rejects invalid global time");

            adk::OneWireTransactionPolicy cancelPolicy (config ());

            initializeReady (cancelPolicy, 100);

            require (

                cancelPolicy
                    .begin (adk::MicrosecondTimePoint (1000),

                            request (1, adk::OneWireOperation::ResetPresence, 1000),

                            intent)
                    .ok (),

                "cancel-time fixture begins");
            require (cancelPolicy
                             .cancel (adk::MicrosecondTimePoint (invalidAt), intent)

                             .error () == adk::StatusCode::InvalidArgument,

                     "cancel rejects invalid global time");

            adk::OneWireTransactionPolicy resetPolicy (config ());

            initializeReady (resetPolicy, 100);

            require (resetPolicy
                             .reset (adk::MicrosecondTimePoint (invalidAt), intent)

                             .error () == adk::StatusCode::InvalidArgument,

                     "reset rejects invalid global time");

            adk::OneWireTransactionPolicy shutdownPolicy (config ());

            initializeReady (shutdownPolicy, 100);

            require (

                shutdownPolicy
                        .shutdown (adk::MicrosecondTimePoint (invalidAt), intent)

                        .error () == adk::StatusCode::InvalidArgument,

                "shutdown rejects invalid global time");

            adk::OneWireTransactionPolicy cleanupPolicy (config ());

            adk::OneWireStepIntent release = emptyIntent ();

            require (cleanupPolicy
                         .initialize (adk::MicrosecondTimePoint (100), release)

                         .ok (),

                     "cleanup-time fixture initializes");
            require (cleanupPolicy

                             .confirmCleanup (adk::MicrosecondTimePoint (invalidAt),

                                              receipt (release, 1, invalidAt))

                             .error () == adk::StatusCode::InvalidArgument,

                     "cleanup confirmation rejects invalid global time");
        }
    }
    void testReceiptSequenceOrdering ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy, 100);

        adk::OneWireStepIntent intent = emptyIntent ();

        require (policy

                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             intent)
                     .ok (),

                 "sequence fixture begins");
        adk::OneWireStepIntent next = emptyIntent ();

        require (policy

                     .update (adk::MicrosecondTimePoint (1000),

                              receipt (intent, 10, 1000), next)

                     .ok (),

                 "first receipt sequence is accepted");
        const uint32_t at = next.earliestAt.microseconds ();

        require (policy.update (adk::MicrosecondTimePoint (at), receipt (next, 9, at),

                                intent)
                         .error () == adk::StatusCode::InvalidArgument,

                 "receipt sequence regression rejects");
        require (policy.update (adk::MicrosecondTimePoint (at),

                                receipt (next, 10U + halfRange, at), intent)

                         .error () == adk::StatusCode::InvalidArgument,

                 "receipt sequence half-range ambiguity rejects");
        adk::OneWireTransactionPolicy exhausted (config ());

        adk::OneWireStepIntent exhaustionRelease = emptyIntent ();

        require (

            exhausted

                .initialize (adk::MicrosecondTimePoint (100), exhaustionRelease)

                .ok (),

            "receipt exhaustion fixture initializes");

        require (

            exhausted

                .confirmCleanup (

                    adk::MicrosecondTimePoint (100),

                    receipt (exhaustionRelease, UINT32_MAX - 1U, 100))

                .ok (),

            "receipt exhaustion fixture seeds adjacent maximum sequence");

        require (exhausted

                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             intent)
                     .ok (),

                 "receipt exhaustion fixture begins");
        require (exhausted.update (adk::MicrosecondTimePoint (1000),

                                   receipt (intent, UINT32_MAX, 1000), next)

                             .error () == adk::StatusCode::CapacityExceeded &&
                     next.lineIntent == adk::OneWireLineIntent::Release,
                 "maximum receipt sequence fails closed without incrementing");
        adk::OneWireTransactionSnapshot before;
        require (exhausted.snapshot (before).ok () &&
                     before.quality ==
                         adk::OneWireTransactionQuality::ReleaseUnconfirmed &&
                     before.status.error () == adk::StatusCode::CapacityExceeded &&
                     before.releaseRequested && !before.releaseConfirmed,
                 "receipt exhaustion enters correlated cleanup");

        adk::OneWireStepIntent sentinel = emptyIntent ();

        sentinel.ownerToken = UINT32_MAX;
        sentinel.lineIntent = adk::OneWireLineIntent::DriveLow;
        require (exhausted.update (adk::MicrosecondTimePoint (999),

                                   receipt (intent, 1, 999), sentinel)

                             .error () == adk::StatusCode::InvalidArgument &&
                     sentinel.ownerToken == UINT32_MAX &&
                     sentinel.lineIntent == adk::OneWireLineIntent::DriveLow,
                 "stale update rejects before pending-cleanup handling");
        adk::OneWireTransactionSnapshot after;
        require (exhausted.snapshot (after).ok () && after.phase == before.phase &&
                     after.quality == before.quality &&
                     after.acceptedSlotCount == before.acceptedSlotCount &&
                     after.releaseRequested == before.releaseRequested &&
                     after.releaseConfirmed == before.releaseConfirmed &&
                     after.completedAt.microseconds () ==
                         before.completedAt.microseconds () &&
                     after.status == before.status &&
                     after.ownerToken == before.ownerToken &&
                     after.lifecycleGeneration == before.lifecycleGeneration &&
                     after.configurationRevision == before.configurationRevision &&
                     after.transactionGeneration == before.transactionGeneration,
                 "stale cleanup update is atomic");
        require (exhausted
                         .confirmCleanup (adk::MicrosecondTimePoint (1000),

                                          receipt (next, 1, 1000))

                         .error () == adk::StatusCode::InvalidArgument,
                 "receipt sequence exhaustion rejects wrap to one");
        require (exhausted.snapshot (after).ok () && after.phase == before.phase &&
                     after.quality == before.quality &&
                     after.releaseRequested == before.releaseRequested &&
                     after.releaseConfirmed == before.releaseConfirmed &&
                     after.status == before.status,
                 "receipt wrap rejection preserves cleanup atomically");
    }
    void testSeededIdentityExhaustion ()

    {
        adk::OneWireStepIntent intent = emptyIntent ();

        adk::OneWireTransactionPolicy lifecycle (config ());

        lifecycle.seedSequencesForTest (UINT32_MAX, 0, 0);

        require (

            lifecycle.initialize (adk::MicrosecondTimePoint (100), intent).error () ==

                adk::StatusCode::CapacityExceeded,
            "lifecycle generation exhaustion rejects initialization");
        adk::OneWireTransactionPolicy transaction (config ());

        initializeReady (transaction, 100);

        transaction.seedSequencesForTest (1, UINT32_MAX, 1);

        require (

            transaction
                    .begin (adk::MicrosecondTimePoint (1000),

                            request (1, adk::OneWireOperation::ResetPresence, 1000),

                            intent)
                    .error () == adk::StatusCode::CapacityExceeded,

            "transaction generation exhaustion rejects begin");
        adk::OneWireTransactionPolicy phase (config ());

        initializeReady (phase, 100);

        adk::OneWireTransactionSnapshot before;
        require (phase.snapshot (before).ok (), "begin exhaustion snapshot copies");

        phase.seedSequencesForTest (1, 1, UINT32_MAX);

        intent.ownerToken = UINT32_MAX;
        require (phase.begin (adk::MicrosecondTimePoint (1000),

                              request (1, adk::OneWireOperation::ResetPresence, 1000),

                              intent)
                             .error () == adk::StatusCode::CapacityExceeded &&

                     intent.ownerToken == UINT32_MAX,
                 "phase sequence exhaustion rejects begin atomically");
        adk::OneWireTransactionSnapshot after;
        require (phase.snapshot (after).ok () && after.phase == before.phase &&

                     after.quality == before.quality &&
                     after.request.requestSequence == before.request.requestSequence &&
                     after.status == before.status,
                 "failed begin preserves the prior snapshot");
        adk::OneWireTransactionPolicy resetPolicy (config ());

        initializeReady (resetPolicy, 100);

        adk::OneWireStepIntent pending = emptyIntent ();

        require (

            resetPolicy
                    .begin (adk::MicrosecondTimePoint (1000),

                            request (1, adk::OneWireOperation::ResetPresence, 1000),

                            pending)
                    .ok () &&

                resetPolicy.snapshot (before).ok (),

            "reset exhaustion fixture begins");
        resetPolicy.seedSequencesForTest (UINT32_MAX, 1, 1);

        intent = emptyIntent ();

        intent.ownerToken = UINT32_MAX;
        require (

            resetPolicy.reset (adk::MicrosecondTimePoint (1001), intent).error () ==

                    adk::StatusCode::CapacityExceeded &&
                intent.ownerToken == UINT32_MAX,
            "lifecycle exhaustion rejects reset atomically");
        require (resetPolicy.snapshot (after).ok () && after.phase == before.phase &&

                     after.quality == before.quality &&
                     after.request.requestSequence == before.request.requestSequence &&
                     after.status == before.status,
                 "failed reset preserves the pending snapshot");
    }
    void testMaximumSlots ()

    {
        adk::OneWireStepIntent intent = emptyIntent ();

        adk::OneWireTransactionPolicy exact (config (72));

        initializeReady (exact, 100);

        require (

            exact
                .begin (adk::MicrosecondTimePoint (1000),

                        request (1, adk::OneWireOperation::ReadRomSingleDrop, 1000),

                        intent)
                .ok (),

            "exact maximum-slot operation is admitted");
        adk::OneWireTransactionPolicy shortByOne (config (71));

        initializeReady (shortByOne, 100);

        require (

            shortByOne
                    .begin (adk::MicrosecondTimePoint (1000),

                            request (1, adk::OneWireOperation::ReadRomSingleDrop, 1000),

                            intent)
                    .error () == adk::StatusCode::CapacityExceeded,

            "operation above maximum slots rejects");
        adk::OneWireTransactionPolicy searchExact (config (200));

        initializeReady (searchExact, 100);

        require (searchExact

                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::SearchRomPass, 1000),

                             intent)
                     .ok (),

                 "Search ROM admits its exact 200-slot budget");
    }
    void testAdversarialOrderingAndConfiguration ()

    {
        adk::OneWireTransactionPolicy policy (config ());

        initializeReady (policy, 100);

        adk::OneWireStepIntent current = emptyIntent ();

        require (policy

                     .begin (adk::MicrosecondTimePoint (1000),

                             request (1, adk::OneWireOperation::ResetPresence, 1000),

                             current)
                     .ok (),

                 "late-foreign fixture begins");
        adk::OneWireTransactionSnapshot before;
        require (policy.snapshot (before).ok (), "pending snapshot copies");

        adk::OneWireStepReceipt foreign = receipt (current, 2, 21001);

        ++foreign.ownerToken;
        adk::OneWireStepIntent sentinel = emptyIntent ();

        require (policy.update (adk::MicrosecondTimePoint (21001), foreign, sentinel)

                         .error () == adk::StatusCode::InvalidArgument,

                 "foreign receipt after overall deadline rejects structurally");
        adk::OneWireTransactionSnapshot after;
        require (policy.snapshot (after).ok () && after.phase == before.phase &&

                     after.quality == before.quality &&
                     after.acceptedSlotCount == before.acceptedSlotCount &&
                     after.status == before.status,
                 "late foreign receipt leaves transaction state atomic");
        adk::OneWireStepIntent next = emptyIntent ();

        require (policy

                     .update (adk::MicrosecondTimePoint (1000),

                              receipt (current, 2, 1000), next)

                     .ok (),

                 "distinct issued equal-time substep receipt is accepted");
        require (policy.update (adk::MicrosecondTimePoint (999), receipt (next, 3, 999),

                                sentinel)
                         .error () == adk::StatusCode::InvalidArgument,

                 "backward issued-substep receipt rejects atomically");
        require (policy.update (adk::MicrosecondTimePoint (1000U + halfRange),

                                receipt (next, 3, 1000U + halfRange), sentinel)

                         .error () == adk::StatusCode::InvalidArgument,

                 "half-range issued-substep receipt rejects atomically");
        adk::OneWireTransactionConfig emptyIntersection = config ();

        emptyIntersection.resetReleaseMaximum = adk::MicrosecondDuration (19);

        adk::OneWireTransactionPolicy invalid (emptyIntersection);

        adk::OneWireStepIntent release = emptyIntent ();

        require (invalid.initialize (adk::MicrosecondTimePoint (0), release).error () ==

                         adk::StatusCode::InvalidConfiguration &&
                     !invalid.initialized (),

                 "empty reset-release and presence-start intersection rejects config");
    }
} // namespace
int main ()

{
    testEveryRuntimeWindow ();

    testRolloverBackwardAndHalfRange ();

    testOverallDeadline ();

    testGlobalTimeOnCommands ();

    testReceiptSequenceOrdering ();

    testSeededIdentityExhaustion ();

    testMaximumSlots ();

    testAdversarialOrderingAndConfiguration ();

    return EXIT_SUCCESS;
}
