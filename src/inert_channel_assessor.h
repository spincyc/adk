#pragma once

#include <stdint.h>

#include "status.h"
#include "time.h"

namespace adk {

    using InertChannelId = uint8_t;

    enum struct InertObservation : uint8_t
    {
        Open,
        Closed,
        ShortSimulated,
        Unavailable
    };

    struct InertChannelObservation
    {
        InertChannelId   channel;
        InertObservation primary;
        InertObservation redundant;
        TimePoint        observedAt;
    };

    enum struct InertChannelState : uint8_t
    {
        Open,
        Closed,
        ShortSimulated,
        Stale,
        Contradictory,
        Unavailable
    };

    struct InertChannelAssessment
    {
        InertChannelId    channel;
        InertChannelState state;
        TimePoint         observedAt;
    };

    struct RecordedInertObservationSet
    {
        TimePoint                dueAt;
        InertChannelObservation observations[8];
    };

    struct RecordedInertObservationSnapshot
    {
        const InertChannelObservation* observations;
        uint8_t                        observationCount;
        TimePoint                      dueAt;
        bool                           available;
    };

    struct RecordedInertObservationSource
    {
        static constexpr uint8_t capacity = 32;

        RecordedInertObservationSource (
            const RecordedInertObservationSet* sets,
            uint8_t                            setCount) noexcept;
        ~RecordedInertObservationSource () noexcept;

        RecordedInertObservationSource (
            const RecordedInertObservationSource&) = delete;
        RecordedInertObservationSource& operator= (
            const RecordedInertObservationSource&) = delete;
        RecordedInertObservationSource (
            RecordedInertObservationSource&&) = delete;
        RecordedInertObservationSource& operator= (
            RecordedInertObservationSource&&) = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

        Status update (TimePoint now) noexcept;

        RecordedInertObservationSnapshot snapshot () const noexcept;

      private:
        bool validSets () const noexcept;

        const RecordedInertObservationSet* sets_;
        uint8_t                            setCount_;
        uint8_t                            nextSet_;
        uint8_t                            selectedSet_;
        bool                               selected_;
        bool                               initialized_;
    };

    struct InertChannelAssessor
    {
        static constexpr uint8_t capacity = 8;

        explicit InertChannelAssessor (Duration staleAfter) noexcept;
        ~InertChannelAssessor         () noexcept;

        InertChannelAssessor (const InertChannelAssessor&)            = delete;
        InertChannelAssessor& operator= (const InertChannelAssessor&) = delete;
        InertChannelAssessor (InertChannelAssessor&&)                 = delete;
        InertChannelAssessor& operator= (InertChannelAssessor&&)      = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

        Status update (TimePoint now, const InertChannelObservation* observations,
                       uint8_t observationCount) noexcept;

        Result<InertChannelAssessment> assessment (InertChannelId channel,
                                                   TimePoint      now) const noexcept;

      private:
        struct Slot
        {
            InertChannelObservation observation;
            bool                    present;
        };

        Duration staleAfter_;
        Slot     slots_[capacity];
        bool     initialized_;
    };
} // namespace adk
