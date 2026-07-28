#include "passage_ledger.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t magic       = 0x3147504dUL;
        constexpr uint8_t  version     = 1;
        constexpr uint8_t  validMarker = 0xa5;
        constexpr uint32_t halfRange   = 0x80000000UL;
        constexpr uint16_t checksumAt  = TwoSlotPassageLedger::slotSize - 5;
        constexpr uint16_t markerAt    = TwoSlotPassageLedger::slotSize - 1;

        enum struct SlotKind : uint8_t
        {
            Erased,
            Torn,
            Corrupt,
            Unsupported,
            Valid,
            ReadFailure
        };

        struct Slot
        {
            SlotKind          kind;
            uint8_t           bytes[TwoSlotPassageLedger::slotSize];
            PassageCheckpoint checkpoint;
            LoggedPassage     entry;
            Status            status;
        };

        LoggedPassage emptyEntry () noexcept
        {
            return {0,
                    0,
                    TimePoint (),
                    PassageDirection::Unknown,
                    PassageLabel::None,
                    {false, false, false, 0, 0, 0},
                    {0, ClockState::NotSet},
                    false};
        }

        PassageLedgerRecovery emptyRecovery (
            PassageLedgerRecoveryDisposition disposition, Status status) noexcept
        {
            return {disposition, false, {0, 0, 0}, false, emptyEntry (), status};
        }

        uint32_t read32 (const uint8_t* bytes) noexcept
        {
            return static_cast<uint32_t> (bytes[0]) |
                   (static_cast<uint32_t> (bytes[1]) << 8U) |
                   (static_cast<uint32_t> (bytes[2]) << 16U) |
                   (static_cast<uint32_t> (bytes[3]) << 24U);
        }

        uint16_t read16 (const uint8_t* bytes) noexcept
        {
            return static_cast<uint16_t> (
                static_cast<uint16_t> (bytes[0]) |
                static_cast<uint16_t> (static_cast<uint16_t> (bytes[1]) << 8U));
        }

        void write32 (uint8_t* bytes, uint32_t value) noexcept
        {
            bytes[0] = static_cast<uint8_t> (value);
            bytes[1] = static_cast<uint8_t> (value >> 8U);
            bytes[2] = static_cast<uint8_t> (value >> 16U);
            bytes[3] = static_cast<uint8_t> (value >> 24U);
        }

        void write16 (uint8_t* bytes, uint16_t value) noexcept
        {
            bytes[0] = static_cast<uint8_t> (value);
            bytes[1] = static_cast<uint8_t> (value >> 8U);
        }

        uint32_t crc32 (const uint8_t* bytes, uint16_t count) noexcept
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

        bool validDirection (PassageDirection value) noexcept
        {
            return value == PassageDirection::AToB ||
                   value == PassageDirection::BToA;
        }

        bool validLabel (PassageLabel value) noexcept
        {
            return value == PassageLabel::None || value == PassageLabel::A ||
                   value == PassageLabel::B || value == PassageLabel::C;
        }

        bool validClockState (ClockState value) noexcept
        {
            return value == ClockState::Valid || value == ClockState::NotSet ||
                   value == ClockState::OscillatorStopped ||
                   value == ClockState::TransportFault;
        }

        bool validPosition (const PassagePositionEvidence& position) noexcept
        {
            if (!position.present)
            {
                return !position.reliable && !position.saturated &&
                       position.onsetPosition == 0 && position.endPosition == 0 &&
                       position.delta == 0;
            }

            const int64_t difference =
                static_cast<int64_t> (position.endPosition) -
                static_cast<int64_t> (position.onsetPosition);
            const bool saturated =
                difference > INT32_MAX || difference < INT32_MIN;
            const int32_t expected =
                difference > INT32_MAX
                    ? INT32_MAX
                    : (difference < INT32_MIN
                           ? INT32_MIN
                           : static_cast<int32_t> (difference));

            return position.saturated == saturated &&
                   position.delta == expected &&
                   (!position.reliable ||
                    (!position.saturated && position.delta != 0));
        }

        bool allErased (const uint8_t* bytes) noexcept
        {
            for (uint16_t index = 0; index < TwoSlotPassageLedger::slotSize;
                 ++index)
            {
                if (bytes[index] != 0xff)
                {
                    return false;
                }
            }

            return true;
        }

        void encode (uint8_t* bytes, const LoggedPassage& entry,
                     const PassageCheckpoint& checkpoint) noexcept
        {
            for (uint16_t index = 0; index < TwoSlotPassageLedger::slotSize;
                 ++index)
            {
                bytes[index] = 0xff;
            }

            write32 (&bytes[0], magic);
            bytes[4] = version;
            write16 (&bytes[5], TwoSlotPassageLedger::slotSize);
            write32 (&bytes[7], checkpoint.generation);
            write32 (&bytes[11], entry.sequence);
            write32 (&bytes[15], entry.committedCount);
            write32 (&bytes[19], entry.acceptedAt.milliseconds ());
            bytes[23] = static_cast<uint8_t> (entry.direction);
            bytes[24] = static_cast<uint8_t> (entry.label);
            bytes[25] = static_cast<uint8_t> (
                (entry.position.present ? 1U : 0U) |
                (entry.position.reliable ? 2U : 0U) |
                (entry.position.saturated ? 4U : 0U));
            write32 (&bytes[26],
                     static_cast<uint32_t> (entry.position.onsetPosition));
            write32 (&bytes[30],
                     static_cast<uint32_t> (entry.position.endPosition));
            write32 (&bytes[34], static_cast<uint32_t> (entry.position.delta));
            write32 (&bytes[38], entry.clock.unixSeconds);
            bytes[42] = static_cast<uint8_t> (entry.clock.state);
            bytes[43] = static_cast<uint8_t> (entry.sequenceGap ? 1 : 0);
            write32 (&bytes[44], checkpoint.committedCount);
            write32 (&bytes[48], checkpoint.committedSequence);
            write32 (&bytes[checksumAt], crc32 (bytes, checksumAt));
            bytes[markerAt] = validMarker;
        }

        void decode (Slot& slot) noexcept
        {
            const uint8_t flags = slot.bytes[25];

            slot.checkpoint = {read32 (&slot.bytes[7]),
                               read32 (&slot.bytes[44]),
                               read32 (&slot.bytes[48])};
            slot.entry = {
                read32    (&slot.bytes[11]),
                read32    (&slot.bytes[15]),
                TimePoint (read32 (&slot.bytes[19])),
                static_cast<PassageDirection> (slot.bytes[23]),
                static_cast<PassageLabel> (slot.bytes[24]),
                {static_cast<bool> ((flags & 1U) != 0),
                 static_cast<bool> ((flags & 2U) != 0),
                 static_cast<bool> ((flags & 4U) != 0),
                 static_cast<int32_t> (read32 (&slot.bytes[26])),
                 static_cast<int32_t> (read32 (&slot.bytes[30])),
                 static_cast<int32_t> (read32 (&slot.bytes[34]))},
                {read32 (&slot.bytes[38]),
                 static_cast<ClockState> (slot.bytes[42])},
                static_cast<bool> (slot.bytes[43] != 0)};
        }

        Slot readSlot (PassageLedgerStorage& storage, uint8_t slotIndex) noexcept
        {
            Slot slot = {SlotKind::Erased, {}, {0, 0, 0}, emptyEntry (),
                         StatusCode::Ok};
            const uint16_t base =
                static_cast<uint16_t> (slotIndex * TwoSlotPassageLedger::slotSize);

            for (uint16_t index = 0; index < TwoSlotPassageLedger::slotSize;
                 ++index)
            {
                const Result<uint8_t> result =
                    storage.read (static_cast<uint16_t> (base + index));

                if (!result.ok ())
                {
                    slot.kind   = SlotKind::ReadFailure;
                    slot.status = result.status ();
                    return slot;
                }

                slot.bytes[index] = result.value ();
            }

            if (allErased (slot.bytes))
            {
                return slot;
            }

            if (slot.bytes[markerAt] != validMarker)
            {
                slot.kind = SlotKind::Torn;
                return slot;
            }

            if (read32 (&slot.bytes[0]) != magic ||
                read16 (&slot.bytes[5]) != TwoSlotPassageLedger::slotSize ||
                read32 (&slot.bytes[checksumAt]) !=
                    crc32 (slot.bytes, checksumAt))
            {
                slot.kind = SlotKind::Corrupt;
                return slot;
            }

            if (slot.bytes[4] != version)
            {
                slot.kind = SlotKind::Unsupported;
                return slot;
            }

            decode (slot);

            if (!validDirection  (slot.entry.direction) ||
                !validLabel      (slot.entry.label) ||
                !validClockState (slot.entry.clock.state) ||
                !validPosition   (slot.entry.position) ||
                slot.bytes[25] > 7 || slot.bytes[43] > 1 ||
                slot.entry.sequence != slot.checkpoint.committedSequence ||
                slot.entry.committedCount != slot.checkpoint.committedCount)
            {
                slot.kind = SlotKind::Corrupt;
                return slot;
            }

            slot.kind = SlotKind::Valid;
            return slot;
        }

        bool sameBytes (const Slot& left, const Slot& right) noexcept
        {
            for (uint16_t index = 0; index < TwoSlotPassageLedger::slotSize;
                 ++index)
            {
                if (left.bytes[index] != right.bytes[index])
                {
                    return false;
                }
            }

            return true;
        }

        PassageLedgerRecovery recovered (
            PassageLedgerRecoveryDisposition disposition,
            const Slot& slot) noexcept
        {
            return {disposition, true, slot.checkpoint, true, slot.entry,
                    StatusCode::Ok};
        }

        PassageLedgerRecovery classify (const Slot& first,
                                        const Slot& second) noexcept
        {
            if (first.kind == SlotKind::ReadFailure)
            {
                return emptyRecovery (
                    PassageLedgerRecoveryDisposition::BothInvalid, first.status);
            }

            if (second.kind == SlotKind::ReadFailure)
            {
                return emptyRecovery (
                    PassageLedgerRecoveryDisposition::BothInvalid, second.status);
            }

            if (first.kind == SlotKind::Valid && second.kind == SlotKind::Valid)
            {
                if (first.checkpoint.generation == second.checkpoint.generation)
                {
                    return sameBytes (first, second)
                               ? recovered (
                                     PassageLedgerRecoveryDisposition::
                                         DuplicateIdentical,
                                     first)
                               : emptyRecovery (
                                     PassageLedgerRecoveryDisposition::
                                         DuplicateGeneration,
                                     StatusCode::InternalInvariant);
                }

                const uint32_t difference =
                    first.checkpoint.generation - second.checkpoint.generation;

                if (difference == halfRange)
                {
                    return emptyRecovery (
                        PassageLedgerRecoveryDisposition::AmbiguousGeneration,
                        StatusCode::InternalInvariant);
                }

                return recovered (PassageLedgerRecoveryDisposition::Recovered,
                                  difference < halfRange ? first : second);
            }

            if (first.kind == SlotKind::Valid || second.kind == SlotKind::Valid)
            {
                const Slot& valid = first.kind == SlotKind::Valid ? first : second;
                const SlotKind peer =
                    first.kind == SlotKind::Valid ? second.kind : first.kind;
                PassageLedgerRecoveryDisposition disposition =
                    PassageLedgerRecoveryDisposition::RecoveredWithCorruptPeer;

                if (peer == SlotKind::Erased)
                {
                    disposition =
                        PassageLedgerRecoveryDisposition::RecoveredWithErasedPeer;
                }
                else if (peer == SlotKind::Torn)
                {
                    disposition =
                        PassageLedgerRecoveryDisposition::RecoveredWithTornPeer;
                }
                else if (peer == SlotKind::Unsupported)
                {
                    disposition = PassageLedgerRecoveryDisposition::
                        RecoveredWithUnsupportedPeer;
                }

                return recovered (disposition, valid);
            }

            if (first.kind == SlotKind::Erased && second.kind == SlotKind::Erased)
            {
                return emptyRecovery (PassageLedgerRecoveryDisposition::Empty,
                                      StatusCode::Ok);
            }

            if ((first.kind == SlotKind::Unsupported &&
                 second.kind == SlotKind::Erased) ||
                (second.kind == SlotKind::Unsupported &&
                 first.kind == SlotKind::Erased) ||
                (first.kind == SlotKind::Unsupported &&
                 second.kind == SlotKind::Unsupported))
            {
                return emptyRecovery (
                    PassageLedgerRecoveryDisposition::UnsupportedVersion,
                    StatusCode::Unsupported);
            }

            return emptyRecovery (PassageLedgerRecoveryDisposition::BothInvalid,
                                  StatusCode::HardwareFailure);
        }

        bool sameCheckpoint (const PassageCheckpoint& left,
                             const PassageCheckpoint& right) noexcept
        {
            return left.generation == right.generation &&
                   left.committedCount == right.committedCount &&
                   left.committedSequence == right.committedSequence;
        }
    } // namespace

    PassageLedgerStorage::~PassageLedgerStorage () noexcept
    {
    }

    PassageLedger::~PassageLedger () noexcept
    {
    }

    TwoSlotPassageLedger::TwoSlotPassageLedger (
        PassageLedgerStorage& storage) noexcept
        : storage_ (storage), initialized_ (false), usable_ (false)
    {
    }

    TwoSlotPassageLedger::~TwoSlotPassageLedger () noexcept
    {
        shutdown ();
    }

    Status TwoSlotPassageLedger::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (storage_.capacity () < requiredCapacity)
        {
            return StatusCode::CapacityExceeded;
        }

        initialized_ = true;
        usable_      = true;
        return StatusCode::Ok;
    }

    void TwoSlotPassageLedger::shutdown () noexcept
    {
        initialized_ = false;
        usable_      = false;
    }

    PassageLedgerRecovery TwoSlotPassageLedger::recover () noexcept
    {
        if (!initialized_)
        {
            return emptyRecovery (PassageLedgerRecoveryDisposition::BothInvalid,
                                  StatusCode::NotInitialized);
        }

        if (!usable_)
        {
            return emptyRecovery (PassageLedgerRecoveryDisposition::BothInvalid,
                                  StatusCode::HardwareFailure);
        }

        return classify (readSlot (storage_, 0), readSlot (storage_, 1));
    }

    LedgerCommitResult TwoSlotPassageLedger::commit (
        const LoggedPassage& entry,
        const PassageCheckpoint& checkpoint) noexcept
    {
        const LedgerCommitResult failure = {
            LedgerCommitDisposition::NotCommitted, checkpoint,
            initialized_ ? StatusCode::InvalidArgument
                         : StatusCode::NotInitialized};

        if (!initialized_ || !usable_ || !validDirection (entry.direction) ||
            !validLabel      (entry.label) ||
            !validClockState (entry.clock.state) ||
            !validPosition   (entry.position) ||
            entry.committedCount !=
                (checkpoint.committedCount == UINT32_MAX
                     ? UINT32_MAX
                     : checkpoint.committedCount + 1) ||
            entry.sequence <= checkpoint.committedSequence)
        {
            return failure;
        }

        const Slot first  = readSlot (storage_, 0);

        const Slot second = readSlot (storage_, 1);

        const PassageLedgerRecovery current = classify (first, second);
        const PassageCheckpoint actual =
            current.disposition == PassageLedgerRecoveryDisposition::Empty
                ? PassageCheckpoint{0, 0, 0}
                : current.checkpoint;

        if (!current.status.ok () || !sameCheckpoint (actual, checkpoint))
        {
            return failure;
        }

        const PassageCheckpoint next = {
            checkpoint.generation + 1, entry.committedCount, entry.sequence};
        uint8_t bytes[slotSize];
        encode (bytes, entry, next);

        uint8_t target = 0;

        if (current.disposition != PassageLedgerRecoveryDisposition::Empty)
        {
            const bool firstWins =
                first.kind == SlotKind::Valid &&
                sameCheckpoint (first.checkpoint, current.checkpoint);
            target = static_cast<uint8_t> (firstWins ? 1U : 0U);
        }
        const uint16_t base = static_cast<uint16_t> (target * slotSize);

        Status status =
            storage_.write (static_cast<uint16_t> (base + markerAt), 0xff);

        for (uint16_t index = 0; status.ok () && index < markerAt; ++index)
        {
            status = storage_.write (static_cast<uint16_t> (base + index),
                                     bytes[index]);
        }

        if (status.ok ())
        {
            status = storage_.synchronize ();
        }

        if (status.ok ())
        {
            status = storage_.write (
                static_cast<uint16_t> (base + markerAt), validMarker);
        }

        if (status.ok ())
        {
            status = storage_.synchronize ();
        }

        if (status.ok ())
        {
            return {LedgerCommitDisposition::Committed, next, StatusCode::Ok};
        }

        const PassageLedgerRecovery reconciled = recover ();

        if (!reconciled.status.ok ())
        {
            usable_ = false;
            return {LedgerCommitDisposition::NotCommitted, checkpoint,
                    StatusCode::HardwareFailure};
        }

        if (reconciled.hasCheckpoint &&
            sameCheckpoint (reconciled.checkpoint, next))
        {
            return {LedgerCommitDisposition::CommittedAfterReconciliation, next,
                    StatusCode::Ok};
        }

        return {LedgerCommitDisposition::NotCommitted, checkpoint, status};
    }

    bool TwoSlotPassageLedger::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
