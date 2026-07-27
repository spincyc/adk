#pragma once

#include <stdint.h>

#include "status.h"
#include "telemetry_packet.h"
#include "time.h"

namespace adk {

    enum struct SequenceState : uint8_t
    {
        First,
        InOrder,
        Duplicate,
        Gap,
        Reordered
    };

    enum struct Freshness : uint8_t
    {
        Fresh,
        Aging,
        Stale
    };

    struct ObservationState
    {
        TelemetrySample sample;
        SequenceState   sequenceState;
        Freshness       freshness;
        Duration        age;
        Status          status;
    };

    struct ObservationTrackerConfig
    {
        Duration agingAfter;
        Duration staleAfter;
    };

    struct ObservationTracker
    {
        ObservationTracker (uint16_t                        sourceId,
                            const ObservationTrackerConfig& config) noexcept;
        ~ObservationTracker () noexcept;

        ObservationTracker (const ObservationTracker&)            = delete;
        ObservationTracker& operator= (const ObservationTracker&) = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;
        Status accept      (const TelemetrySample& sample, TimePoint receivedAt) noexcept;
        Status update      (TimePoint now) noexcept;

        ObservationState state () const noexcept;

      private:
        uint16_t                 sourceId_;
        ObservationTrackerConfig config_;
        ObservationState         state_;
        TimePoint                receivedAt_;
        bool                     initialized_;
        bool                     hasSample_;
    };
} // namespace adk
