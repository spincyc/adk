#pragma once

#include <stdint.h>

#include "status.h"
#include "time.h"

namespace adk {

    using InertCueId = uint8_t;

    enum struct CueAuditEvent : uint8_t
    {
        Initialized,
        ReviewStarted,
        RunRequested,
        ConfirmationRequested,
        Confirmed,
        CueShown,
        CueHidden,
        CueSkipped,
        Held,
        Resumed,
        Cancelled,
        Faulted,
        Completed,
        Shutdown
    };

    struct CueAuditEntry
    {
        uint32_t      sequence;
        TimePoint     recordedAt;
        CueAuditEvent event;
        InertCueId    cue;
        uint8_t       cueIndex;
        Status        status;
        bool          hasCue;
    };

    struct CueAuditBuffer
    {
        CueAuditBuffer (CueAuditEntry* storage, uint8_t capacity) noexcept;

        ~CueAuditBuffer () noexcept;

        CueAuditBuffer (const CueAuditBuffer&)            = delete;
        CueAuditBuffer& operator= (const CueAuditBuffer&) = delete;
        CueAuditBuffer (CueAuditBuffer&&)                 = delete;
        CueAuditBuffer& operator= (CueAuditBuffer&&)      = delete;

        Status initialize () noexcept;

        void   shutdown () noexcept;

        bool   initialized () const noexcept;

        uint8_t               count () const noexcept;

        uint8_t               capacity () const noexcept;

        Result<CueAuditEntry> entry (uint8_t index) const noexcept;

      private:
        friend struct InertCueScheduler;

        Status appendOperational (TimePoint recordedAt, CueAuditEvent event,
                                  InertCueId cue, uint8_t cueIndex, Status status,
                                  bool hasCue) noexcept;
        bool   canAppendOperational (uint8_t eventCount) const noexcept;

        Status appendCapacityHold (TimePoint recordedAt, InertCueId cue,
                                   uint8_t cueIndex) noexcept;
        void   appendShutdown (TimePoint recordedAt) noexcept;

        Status append (TimePoint recordedAt, CueAuditEvent event, InertCueId cue,
                       uint8_t cueIndex, Status status, bool hasCue) noexcept;

        CueAuditEntry* storage_;
        uint8_t        capacity_;
        uint8_t        count_;
        uint32_t       nextSequence_;
        bool           initialized_;
    };

    struct CueAuditEncoder
    {
        static constexpr uint8_t maximumLength = 96;

        Result<uint8_t> requiredSize (const CueAuditEntry& entry) const noexcept;

        Result<uint8_t> encode (const CueAuditEntry& entry, char* output,
                                uint8_t outputCapacity) const noexcept;
    };
} // namespace adk
