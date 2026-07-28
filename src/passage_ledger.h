#pragma once

#include "passage_qualifier.h"
#include "rtc.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct PassageLabel : uint8_t
    {
        None,
        A,
        B,
        C
    };

    struct PassageCheckpoint
    {
        uint32_t generation;
        uint32_t committedCount;
        uint32_t committedSequence;
    };

    struct LoggedPassage
    {
        uint32_t                sequence;
        uint32_t                committedCount;
        TimePoint               acceptedAt;
        PassageDirection        direction;
        PassageLabel            label;
        PassagePositionEvidence position;
        ClockReading            clock;
        bool                    sequenceGap;
    };

    enum struct PassageLedgerRecoveryDisposition : uint8_t
    {
        Empty,
        Recovered,
        RecoveredWithErasedPeer,
        RecoveredWithTornPeer,
        RecoveredWithCorruptPeer,
        RecoveredWithUnsupportedPeer,
        BothInvalid,
        DuplicateIdentical,
        DuplicateGeneration,
        AmbiguousGeneration,
        UnsupportedVersion
    };

    struct PassageLedgerRecovery
    {
        PassageLedgerRecoveryDisposition disposition;
        bool                             hasCheckpoint;
        PassageCheckpoint                checkpoint;
        bool                             hasEntry;
        LoggedPassage                    entry;
        Status                           status;
    };

    struct PassageLedgerStorage
    {
        virtual ~PassageLedgerStorage () noexcept;

        virtual uint16_t        capacity    () const noexcept = 0;
        virtual Result<uint8_t> read        (uint16_t address) noexcept = 0;
        virtual Status          write       (uint16_t address,
                                             uint8_t  value) noexcept = 0;
        virtual Status          synchronize () noexcept = 0;
    };

    enum struct LedgerCommitDisposition : uint8_t
    {
        NotCommitted,
        Committed,
        CommittedAfterReconciliation
    };

    struct LedgerCommitResult
    {
        LedgerCommitDisposition disposition;
        PassageCheckpoint       checkpoint;
        Status                  status;
    };

    struct PassageLedger
    {
        virtual ~PassageLedger () noexcept;

        virtual Status                initialize () noexcept = 0;
        virtual void                  shutdown   () noexcept = 0;
        virtual PassageLedgerRecovery recover    () noexcept = 0;
        virtual LedgerCommitResult    commit     (const LoggedPassage& entry,
                                                   const PassageCheckpoint&
                                                       checkpoint) noexcept = 0;
    };

    struct TwoSlotPassageLedger final : PassageLedger
    {
        static constexpr uint16_t slotSize = 57;
        static constexpr uint16_t requiredCapacity = slotSize * 2;

        explicit TwoSlotPassageLedger (PassageLedgerStorage& storage) noexcept;

        ~TwoSlotPassageLedger () noexcept override;

        TwoSlotPassageLedger (const TwoSlotPassageLedger&) = delete;
        TwoSlotPassageLedger& operator= (const TwoSlotPassageLedger&) = delete;
        TwoSlotPassageLedger (TwoSlotPassageLedger&&) = delete;
        TwoSlotPassageLedger& operator= (TwoSlotPassageLedger&&) = delete;

        Status                initialize () noexcept override;
        void                  shutdown   () noexcept override;
        PassageLedgerRecovery recover    () noexcept override;
        LedgerCommitResult    commit     (const LoggedPassage& entry,
                                           const PassageCheckpoint&
                                               checkpoint) noexcept override;

        bool initialized () const noexcept;

      private:
        PassageLedgerStorage& storage_;
        bool                  initialized_;
        bool                  usable_;
    };
} // namespace adk
