#include "cue_audit.h"

namespace adk {

    namespace {

        bool appendCharacter (char* output, uint8_t capacity, uint8_t& length,
                              char value) noexcept
        {
            if (length >= capacity)
            {
                return false;
            }

            output[length++] = value;
            return true;
        }

        bool appendText (char* output, uint8_t capacity, uint8_t& length,
                         const char* text) noexcept
        {
            while (*text != '\0')
            {
                if (!appendCharacter (output, capacity, length, *text++))
                {
                    return false;
                }
            }

            return true;
        }

        bool appendUnsigned (char* output, uint8_t capacity, uint8_t& length,
                             uint32_t value) noexcept
        {
            char    digits[10];
            uint8_t count = 0;

            do
            {
                digits[count++] = static_cast<char> ('0' + value % 10U);
                value /= 10U;
            }
            while (value != 0);

            while (count != 0)
            {
                if (!appendCharacter (output, capacity, length, digits[--count]))
                {
                    return false;
                }
            }

            return true;
        }

        const char* eventName (CueAuditEvent event) noexcept
        {
            switch (event)
            {
                case CueAuditEvent::Initialized: return "initialized";
                case CueAuditEvent::ReviewStarted: return "review-started";
                case CueAuditEvent::RunRequested: return "run-requested";
                case CueAuditEvent::ConfirmationRequested:
                    return "confirmation-requested";
                case CueAuditEvent::Confirmed: return "confirmed";
                case CueAuditEvent::CueShown: return "cue-shown";
                case CueAuditEvent::CueHidden: return "cue-hidden";
                case CueAuditEvent::CueSkipped: return "cue-skipped";
                case CueAuditEvent::Held: return "held";
                case CueAuditEvent::Resumed: return "resumed";
                case CueAuditEvent::Cancelled: return "cancelled";
                case CueAuditEvent::Faulted: return "faulted";
                case CueAuditEvent::Shutdown: return "shutdown";
            }

            return nullptr;
        }
    } // namespace

    CueAuditBuffer::CueAuditBuffer (CueAuditEntry* storage, uint8_t capacity) noexcept
        : storage_ (storage), capacity_ (capacity), count_ (0), nextSequence_ (0),
          initialized_ (false)
    {
    }

    CueAuditBuffer::~CueAuditBuffer () noexcept
    {
        shutdown ();
    }

    Status CueAuditBuffer::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (storage_ == nullptr || capacity_ < 3)
        {
            return StatusCode::InvalidArgument;
        }

        count_        = 0;
        nextSequence_ = 0;
        initialized_  = true;
        return StatusCode::Ok;
    }

    void CueAuditBuffer::shutdown () noexcept
    {
        count_        = 0;
        nextSequence_ = 0;
        initialized_  = false;
    }

    bool CueAuditBuffer::initialized () const noexcept
    {
        return initialized_;
    }

    uint8_t CueAuditBuffer::count () const noexcept
    {
        return count_;
    }

    Result<CueAuditEntry> CueAuditBuffer::entry (uint8_t index) const noexcept
    {
        const CueAuditEntry empty = {
            0, TimePoint (), CueAuditEvent::Faulted, 0, 0, StatusCode::NotInitialized};

        if (!initialized_)
        {
            return Result<CueAuditEntry> (StatusCode::NotInitialized, empty);
        }

        if (index >= count_)
        {
            return Result<CueAuditEntry> (StatusCode::InvalidArgument, empty);
        }

        return Result<CueAuditEntry> (StatusCode::Ok, storage_[index]);
    }

    Status CueAuditBuffer::appendOperational (TimePoint recordedAt, CueAuditEvent event,
                                              InertCueId cue, uint8_t cueIndex,
                                              Status status) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (static_cast<uint16_t> (count_) + 2U >= capacity_)
        {
            return StatusCode::CapacityExceeded;
        }

        return append (recordedAt, event, cue, cueIndex, status);
    }

    bool CueAuditBuffer::canAppendOperational (uint8_t eventCount) const noexcept
    {
        return initialized_ &&
               static_cast<uint16_t> (count_) + eventCount + 2U <= capacity_;
    }

    Status CueAuditBuffer::appendCapacityHold (TimePoint recordedAt, InertCueId cue,
                                               uint8_t cueIndex) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (static_cast<uint16_t> (count_) + 1U >= capacity_)
        {
            return StatusCode::CapacityExceeded;
        }

        return append (recordedAt, CueAuditEvent::Held, cue, cueIndex,
                       StatusCode::CapacityExceeded);
    }

    void CueAuditBuffer::appendShutdown (TimePoint recordedAt, InertCueId cue,
                                         uint8_t cueIndex) noexcept
    {
        if (initialized_ && count_ < capacity_)
        {
            append (recordedAt, CueAuditEvent::Shutdown, cue, cueIndex, StatusCode::Ok);
        }
    }

    Status CueAuditBuffer::append (TimePoint recordedAt, CueAuditEvent event,
                                   InertCueId cue, uint8_t cueIndex,
                                   Status status) noexcept
    {
        if (count_ >= capacity_)
        {
            return StatusCode::CapacityExceeded;
        }

        storage_[count_++] = {nextSequence_++, recordedAt, event, cue,
                              cueIndex,        status};
        return StatusCode::Ok;
    }

    Status CueAuditEncoder::encode (const CueAuditEntry& entry, char* output,
                                    uint8_t  outputCapacity,
                                    uint8_t& outputLength) const noexcept
    {
        const char* event = eventName (entry.event);

        if (output == nullptr || event == nullptr)
        {
            return StatusCode::InvalidArgument;
        }

        uint8_t    length = 0;
        const bool encoded =
            appendText      (output, outputCapacity, length, "adk-cue,1,") &&
            appendUnsigned  (output, outputCapacity, length, entry.sequence) &&
            appendCharacter (output, outputCapacity, length, ',') &&
            appendUnsigned  (output, outputCapacity, length,
                            entry.recordedAt.milliseconds ()) &&
            appendCharacter (output, outputCapacity, length, ',') &&
            appendText      (output, outputCapacity, length, event) &&
            appendCharacter (output, outputCapacity, length, ',') &&
            appendUnsigned  (output, outputCapacity, length, entry.cue) &&
            appendCharacter (output, outputCapacity, length, ',') &&
            appendUnsigned  (output, outputCapacity, length, entry.cueIndex) &&
            appendCharacter (output, outputCapacity, length, ',') &&
            appendText      (output, outputCapacity, length, statusName (entry.status)) &&
            appendCharacter (output, outputCapacity, length, '\n');

        if (!encoded || length > maximumLength)
        {
            return StatusCode::CapacityExceeded;
        }

        outputLength = length;
        return StatusCode::Ok;
    }
} // namespace adk
