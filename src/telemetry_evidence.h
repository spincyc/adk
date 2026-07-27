#pragma once

#include <stdint.h>

#include "observation_tracker.h"
#include "status.h"
#include "telemetry_packet.h"
#include "time.h"

namespace adk {

    enum struct TelemetryEvidenceSignal : uint8_t
    {
        Fresh,
        GapOrAging,
        Corrupt,
        Stale,
        Fault
    };

    struct TelemetryEvidenceModel
    {
        TelemetryEvidenceSignal decide (PacketValidity          validity,
                                        const ObservationState& observation,
                                        Status                  status) const noexcept;
    };

    enum struct TelemetryFixtureAction : uint8_t
    {
        None,
        Accept,
        Corrupt,
        Silence
    };

    struct TelemetryFixtureDecision
    {
        TelemetryFixtureAction action;
        uint16_t               sequence;
    };

    struct TelemetryFixtureSchedule
    {
        TelemetryFixtureSchedule  () noexcept;
        ~TelemetryFixtureSchedule () noexcept;

        TelemetryFixtureSchedule (const TelemetryFixtureSchedule&)            = delete;
        TelemetryFixtureSchedule& operator= (const TelemetryFixtureSchedule&) = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

        Result<TelemetryFixtureDecision> update (TimePoint now) noexcept;

      private:
        TimePoint startedAt_;
        uint32_t  cycle_;
        uint16_t  nextSequence_;
        uint8_t   phase_;
        bool      initialized_;
        bool      started_;
    };
} // namespace adk
