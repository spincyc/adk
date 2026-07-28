#pragma once

#include "passage_ledger.h"
#include "rtc.h"
#include "seven_segment_display.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    struct PassageCountDisplay
    {
        virtual ~PassageCountDisplay () noexcept;

        virtual Status initialize () noexcept = 0;
        virtual void   shutdown   () noexcept = 0;
        virtual Status present    (uint32_t count) noexcept = 0;
    };

    struct SevenSegmentPassageCountDisplay final : PassageCountDisplay
    {
        explicit SevenSegmentPassageCountDisplay (
            SevenSegmentDisplay& display) noexcept;

        Status initialize () noexcept override;
        void   shutdown   () noexcept override;
        Status present    (uint32_t count) noexcept override;

      private:
        SevenSegmentDisplay& display_;
    };

    struct LoggerConfig
    {
        PassageLabel initialLabel;
    };

    struct LoggerSnapshot
    {
        PassageLabel                     selectedLabel;
        uint32_t                         committedCount;
        uint32_t                         committedSequence;
        uint32_t                         displayedCount;
        bool                             displayValid;
        bool                             pending;
        bool                             overrun;
        PassageRecord                    pendingInput;
        PassageRecord                    overrunInput;
        bool                             hasFrozenEntry;
        LoggedPassage                    frozenEntry;
        bool                             acceptedPulse;
        bool                             committedPulse;
        bool                             persistentFault;
        PassageLedgerRecoveryDisposition recovery;
        Status                           presentationStatus;
        Status                           status;
    };

    struct MagneticPassageLogger
    {
        MagneticPassageLogger (LoggerConfig         config,
                               Rtc&                 rtc,
                               PassageLedger&       ledger,
                               PassageCountDisplay& display) noexcept;
        ~MagneticPassageLogger () noexcept;

        MagneticPassageLogger (const MagneticPassageLogger&) = delete;
        MagneticPassageLogger& operator= (const MagneticPassageLogger&) = delete;
        MagneticPassageLogger (MagneticPassageLogger&&) = delete;
        MagneticPassageLogger& operator= (MagneticPassageLogger&&) = delete;

        Status initialize () noexcept;
        Status update     (const PassageRecord& input) noexcept;
        Status cycleLabel () noexcept;
        void   shutdown   () noexcept;

        LoggerSnapshot snapshot    () const noexcept;
        bool           initialized () const noexcept;

      private:
        void   clearTransient () noexcept;

        Status present        () noexcept;

        bool   matchesCommitted (const PassageRecord& input) const noexcept;

        LoggerConfig         config_;
        Rtc&                 rtc_;
        PassageLedger&       ledger_;
        PassageCountDisplay& display_;
        LoggerSnapshot       snapshot_;
        PassageCheckpoint    checkpoint_;
        LoggedPassage        committedEntry_;
        PassageLabel         pendingLabel_;
        bool                 hasCommittedEntry_;
        bool                 initialized_;
    };
} // namespace adk
