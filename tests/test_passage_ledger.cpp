#include <passage_ledger.h>

#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
    struct MemoryStorage final : adk::PassageLedgerStorage
    {
        struct Operation
        {
            uint8_t  kind;
            uint16_t address;
            uint8_t  value;
        };

        MemoryStorage () noexcept
            : bytes (), failRead (-1), failWrite (-1), failSync (-1), reads (0),
              writes (0), syncs (0), trace (), traceCount (0)
        {
            erase ();
        }

        uint16_t capacity () const noexcept override
        {
            return sizeof (bytes);
        }

        adk::Result<uint8_t> read (uint16_t address) noexcept override
        {
            trace[traceCount++] = {0, address, bytes[address]};

            if (reads++ == failRead)
            {
                return {adk::StatusCode::HardwareFailure, 0};
            }

            return {adk::StatusCode::Ok, bytes[address]};
        }

        adk::Status write (uint16_t address, uint8_t value) noexcept override
        {
            trace[traceCount++] = {1, address, value};

            if (writes++ == failWrite)
            {
                return adk::StatusCode::HardwareFailure;
            }

            bytes[address] = value;
            return adk::StatusCode::Ok;
        }

        adk::Status synchronize () noexcept override
        {
            trace[traceCount++] = {2, 0, 0};

            if (syncs++ == failSync)
            {
                return adk::StatusCode::HardwareFailure;
            }

            return adk::StatusCode::Ok;
        }

        void erase () noexcept
        {
            for (uint16_t index = 0; index < sizeof (bytes); ++index)
            {
                bytes[index] = 0xff;
            }
        }

        uint8_t bytes[adk::TwoSlotPassageLedger::requiredCapacity];
        int32_t failRead;
        int32_t failWrite;
        int32_t failSync;
        int32_t reads;
        int32_t writes;
        int32_t syncs;
        Operation trace[1024];
        uint16_t  traceCount;
    };

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::LoggedPassage entry (uint32_t sequence, uint32_t count)
    {
        return {sequence,
                count,
                adk::TimePoint (40 + sequence),
                adk::PassageDirection::AToB,
                adk::PassageLabel::B,
                {true, true, false, 2, 8, 6},
                {1000 + sequence, adk::ClockState::Valid},
                sequence > 1};
    }

    uint32_t fixtureCrc (const uint8_t* bytes, uint16_t count)
    {
        uint32_t crc = UINT32_MAX;

        for (uint16_t index = 0; index < count; ++index)
        {
            crc ^= bytes[index];

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const uint32_t mask =
                    static_cast<uint32_t> (0U - (crc & 1U));
                crc = (crc >> 1U) ^ (0xedb88320UL & mask);
            }
        }

        return ~crc;
    }

    void put32 (uint8_t* bytes, uint16_t offset, uint32_t value)
    {
        bytes[offset]     = static_cast<uint8_t> (value);
        bytes[offset + 1] = static_cast<uint8_t> (value >> 8U);
        bytes[offset + 2] = static_cast<uint8_t> (value >> 16U);
        bytes[offset + 3] = static_cast<uint8_t> (value >> 24U);
    }

    void refreshCrc (uint8_t* slot)
    {
        put32 (slot, 52, fixtureCrc (slot, 52));
    }

    void copySlot (uint8_t* destination, const uint8_t* source)
    {
        for (uint16_t index = 0; index < adk::TwoSlotPassageLedger::slotSize;
             ++index)
        {
            destination[index] = source[index];
        }
    }

    void testEmptyCommitAndRecovery ()
    {
        MemoryStorage            storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.initialize ().ok (), "ledger initializes");

        const auto empty = ledger.recover ();

        require (empty.disposition ==
                     adk::PassageLedgerRecoveryDisposition::Empty &&
                     empty.status.ok (),
                 "erased slots recover as empty");

        const adk::PassageCheckpoint initial = {0, 0, 0};
        const auto committed = ledger.commit (entry (1, 1), initial);

        require (committed.disposition ==
                     adk::LedgerCommitDisposition::Committed &&
                     committed.checkpoint.generation == 1,
                 "first commit uses first generation");

        const uint8_t golden[] = {
            0x4d, 0x50, 0x47, 0x31, 0x01, 0x39, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x29,
            0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x02, 0x00, 0x00, 0x00,
            0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xe9, 0x03,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
            0x00, 0x00, 0xcd, 0xeb, 0x65, 0x8f, 0xa5};

        for (uint16_t index = 0; index < sizeof (golden); ++index)
        {
            require (storage.bytes[index] == golden[index],
                     "first slot matches frozen byte format");
        }

        const auto recovered = ledger.recover ();

        require (recovered.hasEntry && recovered.entry.sequence == 1 &&
                     recovered.entry.position.delta == 6 &&
                     recovered.entry.clock.unixSeconds == 1001,
                 "full entry round trips");

        const auto second =
            ledger.commit (entry (3, 2), committed.checkpoint);

        require (second.status.ok () && second.checkpoint.generation == 2,
                 "second commit alternates generation");

        require (ledger.recover ().entry.sequence == 3,
                 "newest generation wins");
    }

    void testValidationTornAndCorruptFallback ()
    {
        MemoryStorage            storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.initialize ().ok (), "validation ledger initializes");

        const adk::PassageCheckpoint initial = {0, 0, 0};

        require (ledger.commit (entry (1, 1), initial).status.ok (),
                 "baseline commit succeeds");

        auto invalid = entry (2, 3);

        require (ledger.commit (invalid, {1, 1, 1}).status.error () ==
                     adk::StatusCode::InvalidArgument,
                 "count mismatch writes nothing");

        storage.failWrite = storage.writes + 3;

        const auto torn = ledger.commit (entry (2, 2), {1, 1, 1});

        require (torn.disposition == adk::LedgerCommitDisposition::NotCommitted,
                 "interrupted body is not committed");

        const auto fallback = ledger.recover ();

        require (fallback.disposition ==
                     adk::PassageLedgerRecoveryDisposition::
                         RecoveredWithTornPeer &&
                     fallback.entry.sequence == 1,
                 "valid peer survives torn slot");

        storage.failWrite = -1;
        storage.writes    = 0;

        require (ledger.commit (entry (2, 2), {1, 1, 1}).status.ok (),
                 "retry commits deterministic entry");

        storage.bytes[adk::TwoSlotPassageLedger::slotSize + 12] ^= 0x40;

        const auto corrupt = ledger.recover ();

        require (corrupt.disposition ==
                     adk::PassageLedgerRecoveryDisposition::
                         RecoveredWithCorruptPeer &&
                     corrupt.entry.sequence == 1,
                 "checksum corruption falls back");
    }

    void testLifecycleAndCapacity ()
    {
        struct SmallStorage final : adk::PassageLedgerStorage
        {
            uint16_t capacity () const noexcept override
            {
                return 1;
            }

            adk::Result<uint8_t> read (uint16_t) noexcept override
            {
                return {adk::StatusCode::Ok, 0xff};
            }

            adk::Status write (uint16_t, uint8_t) noexcept override
            {
                return adk::StatusCode::Ok;
            }

            adk::Status synchronize () noexcept override
            {
                return adk::StatusCode::Ok;
            }
        } small;

        adk::TwoSlotPassageLedger undersized (small);

        require (undersized.initialize ().error () ==
                     adk::StatusCode::CapacityExceeded,
                 "undersized storage rejected");

        MemoryStorage             storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.recover ().status.error () ==
                     adk::StatusCode::NotInitialized,
                 "recover before initialize rejected");

        require (ledger.initialize ().ok (), "lifecycle initialize");

        ledger.shutdown ();

        require (!ledger.initialized (), "shutdown is observable");
    }

    void testNoncanonicalWinningSlotIsPreserved ()
    {
        MemoryStorage             storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.initialize ().ok (), "mirrored ledger initializes");

        require (ledger.commit (entry (1, 1), {0, 0, 0}).status.ok (),
                 "canonical slot is committed");

        for (uint16_t index = 0;
             index < adk::TwoSlotPassageLedger::slotSize; ++index)
        {
            storage.bytes[adk::TwoSlotPassageLedger::slotSize + index] =
                storage.bytes[index];
            storage.bytes[index] = 0xff;
        }

        const auto mirrored = ledger.recover ();

        require (mirrored.hasEntry && mirrored.entry.sequence == 1,
                 "noncanonical winning slot recovers");

        storage.failWrite = storage.writes + 3;

        const auto interrupted =
            ledger.commit (entry (2, 2), mirrored.checkpoint);

        require (interrupted.disposition ==
                     adk::LedgerCommitDisposition::NotCommitted,
                 "torn replacement is not committed");

        const auto fallback = ledger.recover ();

        require (fallback.hasEntry && fallback.entry.sequence == 1,
                 "torn commit preserves noncanonical winning peer");
    }

    void testEveryWriteAndSynchronizationFailure ()
    {
        for (int32_t offset = 0;
             offset <= adk::TwoSlotPassageLedger::slotSize; ++offset)
        {
            MemoryStorage             storage;
            adk::TwoSlotPassageLedger ledger (storage);

            require (ledger.initialize ().ok (), "fault ledger initializes");

            const auto first = ledger.commit (entry (1, 1), {0, 0, 0});

            require (first.status.ok (), "fault baseline commits");

            storage.failWrite = storage.writes + offset;

            const auto attempted =
                ledger.commit (entry (2, 2), first.checkpoint);
            const auto recovered = ledger.recover ();

            require (attempted.disposition ==
                         adk::LedgerCommitDisposition::NotCommitted,
                     "each interrupted write remains uncommitted");
            require (recovered.hasEntry && recovered.entry.sequence == 1,
                     "each interrupted write preserves prior entry");
        }

        MemoryStorage             firstSyncStorage;
        adk::TwoSlotPassageLedger firstSyncLedger (firstSyncStorage);

        require (firstSyncLedger.initialize ().ok (),
                 "first-sync ledger initializes");

        const auto baseline =
            firstSyncLedger.commit (entry (1, 1), {0, 0, 0});

        firstSyncStorage.failSync = firstSyncStorage.syncs;

        const auto firstSync =
            firstSyncLedger.commit (entry (2, 2), baseline.checkpoint);

        require (firstSync.disposition ==
                     adk::LedgerCommitDisposition::NotCommitted &&
                     firstSyncLedger.recover ().entry.sequence == 1,
                 "first synchronization failure preserves prior entry");

        MemoryStorage             secondSyncStorage;
        adk::TwoSlotPassageLedger secondSyncLedger (secondSyncStorage);

        require (secondSyncLedger.initialize ().ok (),
                 "second-sync ledger initializes");

        const auto secondBaseline =
            secondSyncLedger.commit (entry (1, 1), {0, 0, 0});

        secondSyncStorage.failSync = secondSyncStorage.syncs + 1;

        const auto secondSync =
            secondSyncLedger.commit (entry (2, 2), secondBaseline.checkpoint);

        require (secondSync.disposition ==
                     adk::LedgerCommitDisposition::
                         CommittedAfterReconciliation &&
                     secondSyncLedger.recover ().entry.sequence == 2,
                 "marker-visible synchronization failure reconciles commit");
    }

    void testSemanticPositionValidation ()
    {
        MemoryStorage             storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.initialize ().ok (), "position ledger initializes");

        auto invalid = entry (1, 1);
        invalid.position.delta = 7;

        require (ledger.commit (invalid, {0, 0, 0}).status.error () ==
                     adk::StatusCode::InvalidArgument,
                 "inconsistent position delta is rejected");

        invalid = entry (1, 1);
        invalid.position.present       = false;
        invalid.position.reliable      = false;
        invalid.position.onsetPosition = 0;
        invalid.position.endPosition   = 0;
        invalid.position.delta         = 0;

        require (ledger.commit (invalid, {0, 0, 0}).status.ok (),
                 "canonical missing position is accepted");
    }

    void testReadFailureAndReinitialize ()
    {
        MemoryStorage             storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.initialize ().ok (), "read-failure ledger initializes");

        storage.failRead = 0;

        require (ledger.recover ().status.error () ==
                     adk::StatusCode::HardwareFailure,
                 "first read failure is returned");

        ledger.shutdown ();
        storage.failRead = -1;
        storage.reads    = 0;

        require (ledger.initialize ().ok () && ledger.recover ().status.ok (),
                 "shutdown and reinitialize restore usable empty ledger");
    }

    void testFrozenRecoveryDispositionMatrix ()
    {
        MemoryStorage             storage;
        adk::TwoSlotPassageLedger ledger (storage);

        require (ledger.initialize ().ok (), "matrix ledger initializes");

        require (ledger.commit (entry (1, 1), {0, 0, 0}).status.ok (),
                 "matrix source slot commits");

        uint8_t canonical[adk::TwoSlotPassageLedger::slotSize];
        copySlot (canonical, storage.bytes);

        copySlot (&storage.bytes[adk::TwoSlotPassageLedger::slotSize],
                  canonical);

        require (ledger.recover ().disposition ==
                     adk::PassageLedgerRecoveryDisposition::DuplicateIdentical,
                 "identical generations are distinguished");

        storage.bytes[adk::TwoSlotPassageLedger::slotSize + 24] =
            static_cast<uint8_t> (adk::PassageLabel::C);
        refreshCrc (&storage.bytes[adk::TwoSlotPassageLedger::slotSize]);

        require (ledger.recover ().disposition ==
                     adk::PassageLedgerRecoveryDisposition::DuplicateGeneration,
                 "unequal duplicate generations fail");

        copySlot (&storage.bytes[adk::TwoSlotPassageLedger::slotSize],
                  canonical);
        storage.bytes[adk::TwoSlotPassageLedger::slotSize + 4] = 2;
        refreshCrc (&storage.bytes[adk::TwoSlotPassageLedger::slotSize]);

        require (ledger.recover ().disposition ==
                     adk::PassageLedgerRecoveryDisposition::
                         RecoveredWithUnsupportedPeer,
                 "supported slot wins over unsupported peer");

        for (uint16_t index = 0; index < adk::TwoSlotPassageLedger::slotSize;
             ++index)
        {
            storage.bytes[index] = 0xff;
        }

        require (ledger.recover ().disposition ==
                     adk::PassageLedgerRecoveryDisposition::UnsupportedVersion,
                 "unsupported slot alone is explicit");

        copySlot (storage.bytes, canonical);
        copySlot (&storage.bytes[adk::TwoSlotPassageLedger::slotSize],
                  canonical);
        put32 (storage.bytes, 7, 1);

        refreshCrc (storage.bytes);

        put32 (&storage.bytes[adk::TwoSlotPassageLedger::slotSize], 7,
               0x80000001UL);

        refreshCrc (&storage.bytes[adk::TwoSlotPassageLedger::slotSize]);

        require (ledger.recover ().disposition ==
                     adk::PassageLedgerRecoveryDisposition::AmbiguousGeneration,
                 "exact half-range generation is ambiguous");

        put32 (storage.bytes, 7, UINT32_MAX);

        refreshCrc (storage.bytes);

        put32 (&storage.bytes[adk::TwoSlotPassageLedger::slotSize], 7, 0);

        refreshCrc (&storage.bytes[adk::TwoSlotPassageLedger::slotSize]);

        const auto wrapped = ledger.recover ();

        require (wrapped.status.ok () && wrapped.checkpoint.generation == 0,
                 "generation zero is newer across natural wrap");

        storage.bytes[0] = 0;
        storage.bytes[adk::TwoSlotPassageLedger::slotSize] = 0;

        require (ledger.recover ().disposition ==
                     adk::PassageLedgerRecoveryDisposition::BothInvalid,
                 "two corrupt nonerased slots are both invalid");
    }

    void testEveryReadFailureAndUnusableReconciliation ()
    {
        for (int32_t offset = 0;
             offset < adk::TwoSlotPassageLedger::requiredCapacity; ++offset)
        {
            MemoryStorage             storage;
            adk::TwoSlotPassageLedger ledger (storage);

            require (ledger.initialize ().ok (), "read matrix initializes");

            storage.failRead = offset;

            require (ledger.recover ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                     "every slot read failure is returned");
        }

        for (int32_t offset = 0;
             offset < adk::TwoSlotPassageLedger::requiredCapacity; ++offset)
        {
            MemoryStorage             storage;
            adk::TwoSlotPassageLedger ledger (storage);

            require (ledger.initialize ().ok (),
                     "reconciliation matrix initializes");

            const auto baseline =
                ledger.commit (entry (1, 1), {0, 0, 0});

            require (baseline.status.ok (),
                     "reconciliation baseline commits");

            storage.failWrite = storage.writes;
            storage.failRead =
                storage.reads +
                adk::TwoSlotPassageLedger::requiredCapacity + offset;

            const auto failed =
                ledger.commit (entry (2, 2), baseline.checkpoint);

            require (failed.status.error () ==
                         adk::StatusCode::HardwareFailure,
                     "reconciliation read failure is hardware failure");

            storage.failRead  = -1;
            storage.failWrite = -1;

            require (ledger.recover ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                     "ambiguous reconciliation latches ledger unusable");
            require (ledger.commit (entry (2, 2), baseline.checkpoint)
                         .status.error () == adk::StatusCode::InvalidArgument,
                     "unusable ledger rejects commit");

            ledger.shutdown ();

            require (ledger.initialize ().ok () && ledger.recover ().status.ok (),
                     "reinitialize clears unusable latch");
        }
    }

    void testIndependentSerializedReplay ()
    {
        MemoryStorage firstStorage;
        MemoryStorage secondStorage;
        adk::TwoSlotPassageLedger first (firstStorage);

        adk::TwoSlotPassageLedger second (secondStorage);

        require (first.initialize ().ok () && second.initialize ().ok (),
                 "replay ledgers initialize");

        const auto firstOne = first.commit (entry (1, 1), {0, 0, 0});

        const auto secondOne = second.commit (entry (1, 1), {0, 0, 0});

        require (firstOne.status.ok () && secondOne.status.ok (),
                 "replay first commits");

        require (first.commit (entry (3, 2), firstOne.checkpoint).status.ok () &&
                     second.commit (entry (3, 2), secondOne.checkpoint)
                         .status.ok (),
                 "replay second commits");

        for (uint16_t address = 0;
             address < adk::TwoSlotPassageLedger::requiredCapacity; ++address)
        {
            require (firstStorage.bytes[address] ==
                         secondStorage.bytes[address],
                     "all 114 serialized bytes replay identically");
        }

        require (firstStorage.traceCount == secondStorage.traceCount,
                 "storage trace lengths replay identically");

        for (uint16_t index = 0; index < firstStorage.traceCount; ++index)
        {
            require (firstStorage.trace[index].kind ==
                         secondStorage.trace[index].kind &&
                         firstStorage.trace[index].address ==
                             secondStorage.trace[index].address &&
                         firstStorage.trace[index].value ==
                             secondStorage.trace[index].value,
                     "every storage read write and sync replays identically");
        }

        const auto firstRecovery  = first.recover ();

        const auto secondRecovery = second.recover ();

        require (firstRecovery.disposition == secondRecovery.disposition &&
                     firstRecovery.checkpoint.generation ==
                         secondRecovery.checkpoint.generation &&
                     firstRecovery.entry.sequence ==
                         secondRecovery.entry.sequence &&
                     firstRecovery.entry.clock.unixSeconds ==
                         secondRecovery.entry.clock.unixSeconds,
                 "recovered public fields replay identically");
    }
} // namespace

int main ()
{
    testEmptyCommitAndRecovery ();

    testValidationTornAndCorruptFallback ();

    testLifecycleAndCapacity ();

    testNoncanonicalWinningSlotIsPreserved ();

    testEveryWriteAndSynchronizationFailure ();

    testSemanticPositionValidation ();

    testReadFailureAndReinitialize ();

    testFrozenRecoveryDispositionMatrix ();

    testEveryReadFailureAndUnusableReconciliation ();

    testIndependentSerializedReplay ();
    std::cout << "passage ledger tests passed\n";
    return EXIT_SUCCESS;
}
