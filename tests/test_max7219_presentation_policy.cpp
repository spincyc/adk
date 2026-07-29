#include <max7219_presentation_policy.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>

namespace adk {
    struct Max7219PresentationPolicyTestAccess
    {
        static void setLifecycleGeneration (Max7219PresentationPolicy& policy,
                                            uint32_t                   generation)
        {
            policy.lifecycleGeneration_ = generation;
        }
    };
} // namespace adk

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::Max7219PresentationConfig
    config (adk::Max7219Orientation orientation = adk::Max7219Orientation::Identity)
    {
        return {29, 7, orientation, 1};
    }

    adk::Max7219Receipt receiptFor (const adk::Max7219Command& command,
                                    adk::Status                status = adk::Status (),
                                    uint8_t                    acceptedByteCount = 2,
                                    bool     chipSelectInactive                  = true,
                                    uint32_t observedAt                          = 1)
    {
        return {command.ownerToken,
                command.lifecycleGeneration,
                command.configurationRevision,
                command.presentationGeneration,
                command.operationIndex,
                command.registerAddress,
                command.data,
                command.operation,
                acceptedByteCount,
                chipSelectInactive,
                adk::TimePoint (observedAt),
                status};
    }

    adk::Max7219Command nextCommand (adk::Max7219PresentationPolicy& policy,
                                     const adk::Max7219Receipt*      receipt = nullptr)
    {
        const auto result = policy.service (receipt);
        require                            (result.ok () && result.value ().emitted,
                 "service emits expected command");
        return result.value ();
    }

    void configurePolicy (adk::Max7219PresentationPolicy& policy,
                          uint32_t firstObservedAt = 1)
    {
        auto     command    = nextCommand (policy);
        uint32_t observedAt = firstObservedAt;
        while (true)
        {
            const auto receipt =
                receiptFor (command, adk::Status (), 2, true, observedAt++);
            require             (policy.service (&receipt).ok (), "configuration receipt succeeds");
            if (policy.snapshot ().configured)
            {
                return;
            }
            command = nextCommand (policy);
        }
    }

    void commitRows (adk::Max7219PresentationPolicy& policy, const uint8_t rows[8],
                     uint32_t source)
    {
        adk::Max7219PresentationPreview preview;
        require (policy.preview (rows, source, preview).ok (),
                 "frame preview succeeds");
        require (policy.canCommit (preview), "frame candidate can commit");
        require (policy.commit (preview).ok (), "frame commit succeeds");
    }

    bool sameFrame (const adk::Max7219Frame& left, const adk::Max7219Frame& right)
    {
        return std::memcmp (left.rows, right.rows, sizeof left.rows) == 0 &&
               left.sourceSnapshotSequence == right.sourceSnapshotSequence &&
               left.generation == right.generation;
    }

    bool sameCommand (const adk::Max7219Command& left, const adk::Max7219Command& right)
    {
        return left.ownerToken == right.ownerToken &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.configurationRevision == right.configurationRevision &&
               left.presentationGeneration == right.presentationGeneration &&
               left.operationIndex == right.operationIndex &&
               left.registerAddress == right.registerAddress &&
               left.data == right.data && left.operation == right.operation &&
               left.emitted == right.emitted;
    }

    void expectedLocation (adk::Max7219Orientation orientation, uint8_t row,
                           uint8_t column, uint8_t& expectedRow,
                           uint8_t& expectedColumn)
    {
        switch (orientation)
        {
            case adk::Max7219Orientation::Identity:
                expectedRow    = row;
                expectedColumn = column;
                return;
            case adk::Max7219Orientation::Rotate90:
                expectedRow    = column;
                expectedColumn = static_cast<uint8_t> (7U - row);
                return;
            case adk::Max7219Orientation::Rotate180:
                expectedRow    = static_cast<uint8_t> (7U - row);
                expectedColumn = static_cast<uint8_t> (7U - column);
                return;
            case adk::Max7219Orientation::Rotate270:
                expectedRow    = static_cast<uint8_t> (7U - column);
                expectedColumn = row;
                return;
        }
        require (false, "orientation is exhaustive");
    }

    void testLifecycleAndConfigurationOrder ()
    {
        static_assert (
            !std::is_copy_constructible<adk::Max7219PresentationPolicy>::value,
            "policy is not copy constructible");
        static_assert (
            !std::is_move_constructible<adk::Max7219PresentationPolicy>::value,
            "policy is not move constructible");

        adk::Max7219PresentationPolicy policy (config ());
        require                               (!policy.initialized (), "construction is inert");
        require                               (policy.service ().error () == adk::StatusCode::NotInitialized,
                 "service before initialization rejects");
        require (policy.initialize ().ok (), "initialize succeeds");
        require (policy.initialize ().ok (), "initialize is idempotent");

        const uint8_t addresses[] = {0x0c, 0x0f, 0x09, 0x0b, 0x0a, 1, 2,
                                     3,    4,    5,    6,    7,    8, 0x0c};
        const uint8_t values[]    = {0, 0, 0, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1};
        auto          command     = nextCommand (policy);
        for (uint8_t index = 0; index < sizeof addresses; ++index)
        {
            require (command.operation == adk::Max7219Operation::Configure &&
                         command.operationIndex == index &&
                         command.registerAddress == addresses[index] &&
                         command.data == values[index],
                     "configuration command has golden order and value");
            require (command.ownerToken == 29 && command.configurationRevision == 7 &&
                         command.presentationGeneration == 0,
                     "configuration command preserves correlation");

            const auto duplicateService = policy.service ();
            require                                      (duplicateService.error () == adk::StatusCode::ResourceBusy &&
                         !duplicateService.value ().emitted,
                     "missing outstanding receipt is non-mutating busy");

            const auto accepted =
                receiptFor (command, adk::Status (), 2, true, index + 1);
            const auto result = policy.service (&accepted);
            if (index + 1U < sizeof addresses)
            {
                require (result.ok () && !result.value ().emitted,
                         "accepted configuration advances without co-emission");
                command = nextCommand (policy);
            }
            else
            {
                require (result.ok () && !result.value ().emitted,
                         "final configuration acceptance emits no extra command");
            }
        }

        const auto snapshot = policy.snapshot ();
        require                               (snapshot.configured && snapshot.initialized && !snapshot.outstanding &&
                     !snapshot.physicallyIndeterminate,
                 "complete configuration becomes ready");
        policy.shutdown ();
        policy.shutdown ();
        require         (!policy.initialized (), "shutdown is idempotent");

        const adk::Max7219PresentationConfig invalid[] = {
            {0, 1, adk::Max7219Orientation::Identity, 1},
            {1, 0, adk::Max7219Orientation::Identity, 1},
            {1, 1, static_cast<adk::Max7219Orientation> (4), 1},
            {1, 1, adk::Max7219Orientation::Identity, 16}};
        for (const auto& badConfig : invalid)
        {
            adk::Max7219PresentationPolicy rejected (badConfig);
            require                                 (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid configuration remains inert");
        }
    }

    void testEveryPixelAndOrientation ()
    {
        const adk::Max7219Orientation orientations[] = {
            adk::Max7219Orientation::Identity, adk::Max7219Orientation::Rotate90,
            adk::Max7219Orientation::Rotate180, adk::Max7219Orientation::Rotate270};

        for (const auto orientation : orientations)
        {
            for (uint8_t row = 0; row < 8; ++row)
            {
                for (uint8_t column = 0; column < 8; ++column)
                {
                    adk::Max7219PresentationPolicy policy (config (orientation));
                    require                               (policy.initialize ().ok (), "pixel fixture initializes");
                    configurePolicy                       (policy);
                    uint8_t rows[8] = {};
                    rows[row]       = static_cast<uint8_t> (0x80U >> column);
                    commitRows (policy, rows,
                                static_cast<uint32_t> (row * 8U + column + 1U));

                    uint8_t expectedRow;
                    uint8_t expectedColumn;
                    expectedLocation (orientation, row, column, expectedRow,
                                      expectedColumn);
                    for (uint8_t operation = 0; operation < 8; ++operation)
                    {
                        const auto command = nextCommand (policy);
                        require                          (command.operation ==
                                         adk::Max7219Operation::SubmitRow &&
                                     command.operationIndex == operation &&
                                     command.registerAddress == operation + 1U &&
                                     command.presentationGeneration == 1,
                                 "row command preserves order and generation");
                        const uint8_t expected =
                            operation == expectedRow
                                ? static_cast<uint8_t> (0x80U >> expectedColumn)
                                : static_cast<uint8_t> (0);
                        require (command.data == expected,
                                 "basis pixel follows orientation transform");
                        const auto accepted = receiptFor (command, adk::Status (), 2,
                                                          true, operation + 20U);
                        const auto result   = policy.service (&accepted);
                        require                              (result.ok (), "row receipt advances submission");
                    }
                    const auto snapshot = policy.snapshot ();
                    require                               (snapshot.submittedFrame.generation == 1 &&
                                 snapshot.submittedFrame.sourceSnapshotSequence ==
                                     static_cast<uint32_t> (row * 8U + column + 1U) &&
                                 !snapshot.outstanding,
                             "all rows atomically advance submitted generation");
                }
            }
        }
    }

    void testPreviewOwnershipAndBusy ()
    {
        adk::Max7219PresentationPolicy first  (config ());
        adk::Max7219PresentationPolicy second (config ());
        require                               (first.initialize ().ok () && second.initialize ().ok (),
                 "preview fixtures initialize");
        const uint8_t rows[8] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
        adk::Max7219PresentationPreview beforeConfigured;
        require (first.preview (rows, 1, beforeConfigured).error () ==
                     adk::StatusCode::ResourceBusy,
                 "configuration blocks frame preview");
        configurePolicy (first);
        configurePolicy (second);

        adk::Max7219PresentationPreview candidate;
        require (first.preview (rows, 4, candidate).ok (),
                 "ready policy previews frame");
        require (!second.canCommit (candidate) && second.commit (candidate).error () ==
                                                      adk::StatusCode::InvalidArgument,
                 "foreign candidate rejects");
        require (first.commit (candidate).ok (), "candidate commits once");
        require (!first.canCommit (candidate) &&
                     first.commit (candidate).error () == adk::StatusCode::ResourceBusy,
                 "committed candidate is busy until submission completes");

        const auto                      command = nextCommand (first);
        adk::Max7219PresentationPreview busy;
        require (first.preview (rows, 5, busy).error () ==
                     adk::StatusCode::ResourceBusy,
                 "outstanding frame submission blocks replacement");
        const auto accepted = receiptFor (command, adk::Status (), 2, true, 30);
        require                          (first.service (&accepted).ok (), "outstanding row accepts");
    }

    void testReceiptCorrelationAndDuplicates ()
    {
        adk::Max7219PresentationPolicy policy (config ());
        require                               (policy.initialize ().ok (), "receipt fixture initializes");
        const auto command = nextCommand      (policy);
        const auto before  = policy.snapshot  ();

        adk::Max7219Receipt foreign = receiptFor (command);
        ++foreign.ownerToken;
        require (policy.service (&foreign).error () == adk::StatusCode::InvalidArgument,
                 "foreign receipt rejects");
        require (policy.snapshot ().outstanding == before.outstanding &&
                     policy.snapshot ().partialPrefix == before.partialPrefix,
                 "foreign receipt rejection is atomic");

        adk::Max7219Receipt changed = receiptFor (command);
        ++changed.data;
        require (policy.service (&changed).error () == adk::StatusCode::InvalidArgument,
                 "changed receipt rejects");

        auto       accepted = receiptFor     (command, adk::Status (), 2, true, 10);
        const auto next     = policy.service (&accepted);
        require                              (next.ok () && !next.value ().emitted,
                 "exact receipt advances without co-emission");
        const auto current    = nextCommand     (policy);
        const auto stateAfter = policy.snapshot ();
        const auto duplicate  = policy.service  (&accepted);
        require                                 (duplicate.ok () && !duplicate.value ().emitted,
                 "identical duplicate is idempotent");
        require (policy.snapshot ().outstanding == stateAfter.outstanding &&
                     policy.snapshot ().partialPrefix == stateAfter.partialPrefix,
                 "duplicate changes no progress");

        accepted.observedAt = adk::TimePoint (11);
        require                              (policy.service (&accepted).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed duplicate rejects");

        auto stale = receiptFor (current);
        --stale.operationIndex;
        require (policy.service (&stale).error () == adk::StatusCode::InvalidArgument,
                 "stale operation rejects");
        auto half = receiptFor (current);
        half.presentationGeneration += 0x80000000UL;
        require (policy.service (&half).error () == adk::StatusCode::InvalidArgument,
                 "exact-half-range generation rejects");
    }

    void expectFailure (uint8_t acceptedByteCount, bool chipSelectInactive,
                        adk::Status transportStatus, adk::Max7219Fault expectedFault)
    {
        adk::Max7219PresentationPolicy policy (config ());
        require                               (policy.initialize ().ok (), "failure fixture initializes");
        const auto command  = nextCommand     (policy);
        const auto failed   = receiptFor      (command, transportStatus, acceptedByteCount,
                                          chipSelectInactive, 50);
        const auto result   = policy.service  (&failed);
        const auto snapshot = policy.snapshot ();
        require                               (result.error () == snapshot.status.error (),
                 "failure reports retained primary status");
        require (snapshot.fault == expectedFault, "failure class is retained");
        require (snapshot.cleanupPending && snapshot.blankRequested &&
                     snapshot.physicallyIndeterminate,
                 "failure schedules cleanup and marks presentation unknown");
        require (snapshot.failure.operationIndex == command.operationIndex &&
                     snapshot.failure.registerAddress == command.registerAddress,
                 "failure preserves operation provenance");
        require (snapshot.failure.acceptedByteCount == acceptedByteCount,
                 "failure preserves accepted byte prefix");

        const auto cleanup = nextCommand (policy);
        require                          (cleanup.operation == adk::Max7219Operation::CleanupShutdown &&
                     cleanup.registerAddress == 0x0c && cleanup.data == 0,
                 "cleanup is one later shutdown command");
        const auto cleanupFailure =
            receiptFor (cleanup, adk::StatusCode::HardwareFailure, 1, true, 51);
        require (policy.service (&cleanupFailure).error () == snapshot.status.error (),
                 "cleanup preserves the primary reported status");
        const auto cleaned = policy.snapshot ();
        require                              (cleaned.failure.status == snapshot.failure.status &&
                     cleaned.failure.cleanupStatus.error () ==
                         adk::StatusCode::HardwareFailure &&
                     cleaned.fault == expectedFault,
                 "cleanup status cannot overwrite primary provenance");
    }

    void testFailureAndContradictionMatrix ()
    {
        expectFailure (0, true, adk::StatusCode::HardwareFailure,
                       adk::Max7219Fault::Transport);
        expectFailure (1, true, adk::StatusCode::HardwareFailure,
                       adk::Max7219Fault::Transport);
        expectFailure (0, false, adk::StatusCode::HardwareFailure,
                       adk::Max7219Fault::Transport);
        expectFailure (2, true, adk::StatusCode::HardwareFailure,
                       adk::Max7219Fault::ContradictoryReceipt);
        expectFailure (0, true, adk::Status (),
                       adk::Max7219Fault::ContradictoryReceipt);
        expectFailure (1, true, adk::Status (),
                       adk::Max7219Fault::ContradictoryReceipt);
        expectFailure (2, false, adk::Status (), adk::Max7219Fault::Transport);
    }

    void testEveryOperationFailureAndReplay ()
    {
        for (uint8_t failingOperation = 0; failingOperation < 22; ++failingOperation)
        {
            adk::Max7219PresentationPolicy policy (config ());
            require                               (policy.initialize ().ok (),
                     "operation failure fixture initializes");
            uint8_t rows[8]   = {1, 2, 4, 8, 16, 32, 64, 128};
            uint8_t operation = 0;
            auto    command   = nextCommand (policy);
            while (operation < failingOperation)
            {
                const auto accepted =
                    receiptFor (command, adk::Status (), 2, true, operation + 1U);
                const auto result = policy.service (&accepted);
                require                            (result.ok (), "prefix receipt succeeds");
                ++operation;
                if (operation == 14)
                {
                    commitRows            (policy, rows, 44);
                    command = nextCommand (policy);
                }
                else
                {
                    require (!result.value ().emitted,
                             "prefix acceptance does not co-emit");
                    command = nextCommand (policy);
                }
            }
            const auto failed =
                receiptFor (command, adk::StatusCode::HardwareFailure,
                            static_cast<uint8_t> (failingOperation & 1U), true, 100);
            require (policy.service (&failed).error () ==
                         adk::StatusCode::HardwareFailure,
                     "each configuration and row operation can fail");
            const auto snapshot = policy.snapshot ();
            require                               (snapshot.failure.operationIndex == command.operationIndex &&
                         snapshot.failure.registerAddress == command.registerAddress &&
                         snapshot.failure.acceptedByteCount ==
                             static_cast<uint8_t> (failingOperation & 1U),
                     "each failure retains operation and byte prefix");
            if (failingOperation >= 14)
            {
                require (snapshot.submittedFrame.generation == 0,
                         "partial row prefix never advances submitted frame");
                require (snapshot.partialPrefix ==
                             static_cast<uint8_t> (failingOperation - 14U),
                         "row failure retains fully accepted row prefix");
            }
        }

        adk::Max7219PresentationPolicy first  (config ());
        adk::Max7219PresentationPolicy second (config ());
        require                               (first.initialize ().ok () && second.initialize ().ok (),
                 "replay fixtures initialize");
        for (uint8_t operation = 0; operation < 14; ++operation)
        {
            const auto left  = nextCommand (first);
            const auto right = nextCommand (second);
            require                        (sameCommand (left, right),
                     "configuration commands replay fieldwise");
            const auto leftReceipt =
                receiptFor (left, adk::Status (), 2, true, operation);
            const auto rightReceipt =
                receiptFor (right, adk::Status (), 2, true, operation);
            const auto leftResult  = first.service  (&leftReceipt);
            const auto rightResult = second.service (&rightReceipt);
            require                                 (leftResult.error () == rightResult.error (),
                     "configuration outcomes replay");
        }
        uint8_t rows[8] = {0xaa, 0x55, 0x81, 0x42, 0x24, 0x18, 0xff, 0};
        commitRows (first, rows, 99);
        commitRows (second, rows, 99);
        for (uint8_t row = 0; row < 8; ++row)
        {
            const auto left  = nextCommand (first);
            const auto right = nextCommand (second);
            require                        (sameCommand (left, right), "row commands replay fieldwise");
            const auto leftReceipt =
                receiptFor (left, adk::Status (), 2, true, row + 20U);
            const auto rightReceipt =
                receiptFor (right, adk::Status (), 2, true, row + 20U);
            require (first.service (&leftReceipt).error () ==
                         second.service (&rightReceipt).error (),
                     "row outcomes replay");
        }
        require (sameFrame (first.snapshot ().submittedFrame,
                            second.snapshot ().submittedFrame),
                 "submitted snapshots replay fieldwise");
    }

    void testReceiptTimeOrdering ()
    {
        adk::Max7219PresentationPolicy wrap (config ());
        require                             (wrap.initialize ().ok (), "wrap fixture initializes");
        configurePolicy                     (wrap, 0xfffffff0UL);
        uint8_t rows[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        commitRows                 (wrap, rows, 71);
        auto command = nextCommand (wrap);
        auto receipt = receiptFor  (command, adk::Status (), 2, true,
                                   0xfffffffeUL);
        require (wrap.service (&receipt).ok (),
                 "forward receipt near wrap succeeds");
        command = nextCommand (wrap);
        receipt = receiptFor  (command, adk::Status (), 2, true,
                              0xffffffffUL);
        require (wrap.service (&receipt).ok (),
                 "next receipt before wrap succeeds");
        command = nextCommand (wrap);
        receipt = receiptFor  (command, adk::Status (), 2, true, 0);
        require               (wrap.service (&receipt).ok (),
                 "forward receipt across wrap succeeds");

        auto orderingFixture = [&rows] (
                                   uint32_t observedAt,
                                   adk::StatusCode expected,
                                   const char* message) {
            adk::Max7219PresentationPolicy policy (config ());
            require                               (policy.initialize ().ok (),
                     "ordering fixture initializes");
            configurePolicy                (policy);
            commitRows                     (policy, rows, 72);
            const auto first = nextCommand (policy);
            const auto accepted =
                receiptFor (first, adk::Status (), 2, true, 30);
            require (policy.service (&accepted).ok (),
                     "ordering fixture accepts baseline");
            const auto current = nextCommand (policy);
            const auto candidate =
                receiptFor (current, adk::Status (), 2, true, observedAt);
            const auto before = policy.snapshot ();
            require                             (policy.service (&candidate).error () == expected,
                     message);
            if (expected == adk::StatusCode::InvalidArgument)
            {
                const auto after = policy.snapshot ();
                require                            (after.outstanding == before.outstanding &&
                             after.partialPrefix == before.partialPrefix,
                         "invalid receipt time is atomic");
            }
        };

        orderingFixture (31, adk::StatusCode::Ok,
                         "strictly forward receipt succeeds");
        orderingFixture (30, adk::StatusCode::InvalidArgument,
                         "equal time on a new command rejects");
        orderingFixture (29, adk::StatusCode::InvalidArgument,
                         "regressing time on an exact command rejects");
        orderingFixture (0x8000001eUL, adk::StatusCode::InvalidArgument,
                         "exact-half-range time on an exact command rejects");

        adk::Max7219PresentationPolicy duplicate (config ());
        require                                  (duplicate.initialize ().ok (),
                 "duplicate time fixture initializes");
        const auto duplicateCommand = nextCommand (duplicate);
        const auto exact =
            receiptFor (duplicateCommand, adk::Status (), 2, true, 1);
        require (duplicate.service (&exact).ok (),
                 "duplicate fixture accepts receipt");
        const auto afterExact = duplicate.snapshot ();
        require                                    (duplicate.service (&exact).ok (),
                 "byte-identical receipt remains idempotent at equal time");
        require (duplicate.snapshot ().partialPrefix ==
                         afterExact.partialPrefix &&
                     duplicate.snapshot ().outstanding ==
                         afterExact.outstanding,
                 "idempotent duplicate changes no state");
    }

    void testResetPreservesUncertaintyAndProvenance ()
    {
        adk::Max7219PresentationPolicy abandoned (config ());
        require                                  (abandoned.initialize ().ok (),
                 "abandoned command fixture initializes");
        const auto oldCommand = nextCommand (abandoned);
        const auto oldReceipt =
            receiptFor (oldCommand, adk::Status (), 2, true, 1);
        const auto oldLifecycle = abandoned.snapshot ().lifecycleGeneration;
        abandoned.reset                              ();
        const auto reset = abandoned.snapshot        ();
        require                                      (reset.lifecycleGeneration == oldLifecycle + 1U &&
                     !reset.outstanding && !reset.configured &&
                     reset.blankRequested && reset.physicallyIndeterminate,
                 "reset abandons outstanding command into uncertain reconfiguration");
        require (reset.failure.operation == oldCommand.operation &&
                     reset.failure.operationIndex == oldCommand.operationIndex &&
                     reset.failure.registerAddress ==
                         oldCommand.registerAddress &&
                     reset.failure.acceptedByteCount == 0 &&
                     reset.failure.status.error () ==
                         adk::StatusCode::ResourceBusy &&
                     reset.failure.cleanupStatus.ok (),
                 "reset retains abandoned command provenance");
        require (abandoned.service (&oldReceipt).error () ==
                     adk::StatusCode::InvalidArgument,
                 "old lifecycle receipt rejects after reset");
        configurePolicy (abandoned, 2);
        require         (abandoned.snapshot ().configured &&
                     abandoned.snapshot ().physicallyIndeterminate,
                 "complete reconfiguration cannot launder physical uncertainty");

        adk::Max7219PresentationPolicy failed (config ());
        require                               (failed.initialize ().ok (),
                 "failed restart fixture initializes");
        const auto primaryCommand = nextCommand (failed);
        const auto primaryFailure =
            receiptFor (primaryCommand, adk::StatusCode::HardwareFailure, 1,
                        true, 10);
        require (failed.service (&primaryFailure).error () ==
                     adk::StatusCode::HardwareFailure,
                 "primary failure latches");
        const auto cleanup = nextCommand (failed);
        const auto cleanupFailure =
            receiptFor (cleanup, adk::StatusCode::HardwareFailure, 0, true,
                        11);
        require (failed.service (&cleanupFailure).error () ==
                     adk::StatusCode::HardwareFailure,
                 "cleanup failure preserves primary status");
        const auto provenance = failed.snapshot ().failure;
        failed.reset                            ();
        const auto restarted = failed.snapshot  ();
        require                                 (restarted.physicallyIndeterminate &&
                     restarted.blankRequested && !restarted.configured,
                 "fault reset starts dark uncertain reconfiguration");
        require (restarted.failure.operation == provenance.operation &&
                     restarted.failure.operationIndex ==
                         provenance.operationIndex &&
                     restarted.failure.registerAddress ==
                         provenance.registerAddress &&
                     restarted.failure.rowIndex == provenance.rowIndex &&
                     restarted.failure.acceptedByteCount ==
                         provenance.acceptedByteCount &&
                     restarted.failure.status == provenance.status &&
                     restarted.failure.cleanupStatus ==
                         provenance.cleanupStatus,
                 "fault and cleanup provenance survive reset exactly");
        configurePolicy                           (failed, 20);
        const auto reconfigured = failed.snapshot ();
        require                                   (reconfigured.configured &&
                     reconfigured.physicallyIndeterminate &&
                     reconfigured.failure.status == provenance.status &&
                     reconfigured.failure.cleanupStatus ==
                         provenance.cleanupStatus,
                 "reconfiguration preserves failure evidence and uncertainty");
    }

    void testLifecycleExhaustion ()
    {
        adk::Max7219PresentationPolicy policy                            (config ());
        adk::Max7219PresentationPolicyTestAccess::setLifecycleGeneration (policy,
                                                                          0xffffffffUL);
        require (policy.initialize ().error () == adk::StatusCode::CapacityExceeded,
                 "lifecycle wrap rejects");
        require (policy.snapshot ().fault == adk::Max7219Fault::LifecycleExhausted &&
                     !policy.initialized (),
                 "lifecycle exhaustion is terminal and inert");
        policy.reset    ();
        policy.shutdown ();
        require         (policy.initialize ().error () == adk::StatusCode::CapacityExceeded,
                 "reset and shutdown cannot hide lifecycle exhaustion");
    }
} // namespace

int main ()
{
    testLifecycleAndConfigurationOrder         ();
    testEveryPixelAndOrientation               ();
    testPreviewOwnershipAndBusy                ();
    testReceiptCorrelationAndDuplicates        ();
    testFailureAndContradictionMatrix          ();
    testEveryOperationFailureAndReplay         ();
    testReceiptTimeOrdering                    ();
    testResetPreservesUncertaintyAndProvenance ();
    testLifecycleExhaustion                    ();
    std::cout << "max7219 presentation policy tests passed\n";
    return EXIT_SUCCESS;
}
