#include "magnetic_passage_logger.h"

#include <limits.h>

namespace adk {
    namespace {
        PassagePositionEvidence emptyPosition () noexcept
        {
            return {false, false, false, 0, 0, 0};
        }

        PassageRecord emptyRecord () noexcept
        {
            return {0,
                    PassageDirection::Unknown,
                    PassageDisposition::EvidenceFault,
                    TimePoint       (),
                    TimePoint       (),
                    Duration        (),
                    MagneticPolarity::Unspecified,
                    MagneticPolarity::Unspecified,
                    emptyPosition (),
                    0,
                    0,
                    StatusCode::Ok};
        }

        LoggedPassage emptyEntry () noexcept
        {
            return {0,
                    0,
                    TimePoint (),
                    PassageDirection::Unknown,
                    PassageLabel::None,
                    emptyPosition (),
                    {0, ClockState::NotSet},
                    false};
        }

        LoggerSnapshot emptySnapshot (PassageLabel label) noexcept
        {
            return {label,
                    0,
                    0,
                    0,
                    false,
                    false,
                    false,
                    emptyRecord (),
                    emptyRecord (),
                    false,
                    emptyEntry (),
                    false,
                    false,
                    false,
                    PassageLedgerRecoveryDisposition::Empty,
                    StatusCode::NotInitialized,
                    StatusCode::NotInitialized};
        }

        bool validLabel (PassageLabel label) noexcept
        {
            return label == PassageLabel::None || label == PassageLabel::A ||
                   label == PassageLabel::B || label == PassageLabel::C;
        }

        bool samePosition (const PassagePositionEvidence& left,
                           const PassagePositionEvidence& right) noexcept
        {
            return left.present == right.present &&
                   left.reliable == right.reliable &&
                   left.saturated == right.saturated &&
                   left.onsetPosition == right.onsetPosition &&
                   left.endPosition == right.endPosition &&
                   left.delta == right.delta;
        }

        uint32_t incrementSaturating (uint32_t value) noexcept
        {
            return value == UINT32_MAX ? value : value + 1;
        }
    } // namespace

    PassageCountDisplay::~PassageCountDisplay () noexcept
    {
    }

    SevenSegmentPassageCountDisplay::SevenSegmentPassageCountDisplay (
        SevenSegmentDisplay& display) noexcept
        : display_ (display)
    {
    }

    Status SevenSegmentPassageCountDisplay::initialize () noexcept
    {
        return display_.initialize ();
    }

    void SevenSegmentPassageCountDisplay::shutdown () noexcept
    {
        display_.shutdown ();
    }

    Status SevenSegmentPassageCountDisplay::present (uint32_t count) noexcept
    {
        return display_.show (
            static_cast<SevenSegmentGlyph> (count % 10U), count > 9U);
    }

    MagneticPassageLogger::MagneticPassageLogger (
        LoggerConfig config, Rtc& rtc, PassageLedger& ledger,
        PassageCountDisplay& display) noexcept
        : config_            (config)
        , rtc_               (rtc)
        , ledger_            (ledger)
        , display_           (display)
        , snapshot_          (emptySnapshot (config.initialLabel))
        , checkpoint_        ({0, 0, 0})
        , committedEntry_    (emptyEntry ())
        , pendingLabel_      (PassageLabel::None)
        , hasCommittedEntry_ (false)
        , initialized_       (false)
    {
    }

    MagneticPassageLogger::~MagneticPassageLogger () noexcept
    {
        shutdown ();
    }

    Status MagneticPassageLogger::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const bool priorPersistentFault = snapshot_.persistentFault;
        snapshot_ = emptySnapshot (config_.initialLabel);
        snapshot_.persistentFault = priorPersistentFault;

        if (!validLabel (config_.initialLabel))
        {
            snapshot_.status = StatusCode::InvalidConfiguration;
            return snapshot_.status;
        }

        Status status = ledger_.initialize ();

        if (!status.ok ())
        {
            snapshot_.status = status;
            return status;
        }

        status = rtc_.initialize ();

        if (!status.ok ())
        {
            ledger_.shutdown ();
            snapshot_.status = status;
            return status;
        }

        status = display_.initialize ();

        if (!status.ok ())
        {
            rtc_.shutdown    ();
            ledger_.shutdown ();
            snapshot_.status = status;
            return status;
        }

        const PassageLedgerRecovery recovery = ledger_.recover ();
        snapshot_.recovery = recovery.disposition;

        if (!recovery.status.ok ())
        {
            display_.shutdown ();
            rtc_.shutdown     ();
            ledger_.shutdown  ();
            snapshot_.status = recovery.status;
            return recovery.status;
        }

        checkpoint_ = recovery.hasCheckpoint
                          ? recovery.checkpoint
                          : PassageCheckpoint{0, 0, 0};
        hasCommittedEntry_ = recovery.hasEntry;
        committedEntry_ = recovery.hasEntry ? recovery.entry : emptyEntry ();
        snapshot_.committedCount    = checkpoint_.committedCount;
        snapshot_.committedSequence = checkpoint_.committedSequence;
        initialized_                = true;
        snapshot_.status            = StatusCode::Ok;
        snapshot_.presentationStatus = present ();

        if (!snapshot_.presentationStatus.ok ())
        {
            display_.shutdown ();
            rtc_.shutdown     ();
            ledger_.shutdown  ();
            initialized_              = false;
            snapshot_.persistentFault = true;
            snapshot_.status          = snapshot_.presentationStatus;
            return snapshot_.status;
        }

        snapshot_.persistentFault = false;
        return StatusCode::Ok;
    }

    Status MagneticPassageLogger::update (const PassageRecord& input) noexcept
    {
        clearTransient ();

        if (!initialized_)
        {
            snapshot_.status = StatusCode::NotInitialized;
            return snapshot_.status;
        }

        if (input.disposition != PassageDisposition::Accepted ||
            !input.status.ok ())
        {
            snapshot_.status = StatusCode::InvalidArgument;
            return snapshot_.status;
        }

        if (snapshot_.pending && input.sequence != snapshot_.pendingInput.sequence)
        {
            if (input.sequence > snapshot_.pendingInput.sequence)
            {
                snapshot_.overrun      = true;
                snapshot_.overrunInput = input;
                snapshot_.status       = StatusCode::CapacityExceeded;
            }
            else
            {
                snapshot_.status = StatusCode::InvalidArgument;
            }

            return snapshot_.status;
        }

        if (!snapshot_.pending)
        {
            if (input.sequence == snapshot_.committedSequence &&
                snapshot_.committedSequence != 0)
            {
                snapshot_.status = matchesCommitted (input)
                                       ? Status (StatusCode::Ok)
                                       : Status (StatusCode::InternalInvariant);
                if (!snapshot_.status.ok ())
                {
                    snapshot_.persistentFault = true;
                }
                return snapshot_.status;
            }

            if (input.sequence <= snapshot_.committedSequence ||
                (snapshot_.committedSequence == UINT32_MAX &&
                 input.sequence != snapshot_.committedSequence))
            {
                snapshot_.status = StatusCode::InvalidArgument;
                return snapshot_.status;
            }

            snapshot_.pending      = true;
            snapshot_.pendingInput = input;
            pendingLabel_          = snapshot_.selectedLabel;
            snapshot_.acceptedPulse = true;
        }

        if (!snapshot_.hasFrozenEntry)
        {
            const Result<ClockReading> reading = rtc_.read ();

            if (!reading.ok ())
            {
                snapshot_.status          = reading.status ();
                snapshot_.persistentFault = true;
                return snapshot_.status;
            }

            snapshot_.frozenEntry = {
                snapshot_.pendingInput.sequence,
                incrementSaturating (snapshot_.committedCount),
                snapshot_.pendingInput.end,
                snapshot_.pendingInput.direction,
                pendingLabel_,
                snapshot_.pendingInput.position,
                reading.value (),
                snapshot_.pendingInput.sequence >
                    snapshot_.committedSequence + 1U};
            snapshot_.hasFrozenEntry = true;
        }

        const LedgerCommitResult committed =
            ledger_.commit (snapshot_.frozenEntry, checkpoint_);

        if (!committed.status.ok () ||
            committed.disposition == LedgerCommitDisposition::NotCommitted)
        {
            snapshot_.status          = committed.status;
            snapshot_.persistentFault = true;
            return snapshot_.status;
        }

        checkpoint_                  = committed.checkpoint;
        committedEntry_             = snapshot_.frozenEntry;
        hasCommittedEntry_           = true;
        snapshot_.committedCount     = checkpoint_.committedCount;
        snapshot_.committedSequence  = checkpoint_.committedSequence;
        snapshot_.pending            = false;
        snapshot_.pendingInput       = emptyRecord ();
        snapshot_.hasFrozenEntry     = false;
        snapshot_.frozenEntry        = emptyEntry ();
        snapshot_.committedPulse     = true;
        snapshot_.status             = StatusCode::Ok;
        snapshot_.presentationStatus = present ();

        if (!snapshot_.presentationStatus.ok ())
        {
            snapshot_.persistentFault = true;
            snapshot_.status          = snapshot_.presentationStatus;
        }

        return snapshot_.status;
    }

    Status MagneticPassageLogger::cycleLabel () noexcept
    {
        clearTransient ();

        if (!initialized_)
        {
            snapshot_.status = StatusCode::NotInitialized;
            return snapshot_.status;
        }

        const uint8_t next =
            static_cast<uint8_t> (snapshot_.selectedLabel) + 1U;
        snapshot_.selectedLabel =
            next > static_cast<uint8_t> (PassageLabel::C)
                ? PassageLabel::None
                : static_cast<PassageLabel> (next);
        snapshot_.status = StatusCode::Ok;
        return snapshot_.status;
    }

    void MagneticPassageLogger::shutdown () noexcept
    {
        const bool persistentFault = snapshot_.persistentFault;

        if (initialized_)
        {
            display_.shutdown ();
            rtc_.shutdown     ();
            ledger_.shutdown  ();
        }

        initialized_ = false;
        snapshot_    = emptySnapshot (config_.initialLabel);
        snapshot_.persistentFault = persistentFault;
    }

    LoggerSnapshot MagneticPassageLogger::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool MagneticPassageLogger::initialized () const noexcept
    {
        return initialized_;
    }

    void MagneticPassageLogger::clearTransient () noexcept
    {
        snapshot_.acceptedPulse  = false;
        snapshot_.committedPulse = false;

        if (!snapshot_.overrun)
        {
            snapshot_.overrunInput = emptyRecord ();
        }
    }

    Status MagneticPassageLogger::present () noexcept
    {
        const Status status = display_.present (snapshot_.committedCount);

        if (status.ok ())
        {
            snapshot_.displayedCount = snapshot_.committedCount;
            snapshot_.displayValid   = true;
        }
        else
        {
            snapshot_.displayValid = false;
        }

        return status;
    }

    bool MagneticPassageLogger::matchesCommitted (
        const PassageRecord& input) const noexcept
    {
        return hasCommittedEntry_ &&
               input.sequence == committedEntry_.sequence &&
               input.direction == committedEntry_.direction &&
               input.end == committedEntry_.acceptedAt &&
               samePosition (input.position, committedEntry_.position);
    }
} // namespace adk
