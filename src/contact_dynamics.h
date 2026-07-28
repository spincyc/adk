#pragma once

#include "digital.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

// clang-format off
namespace adk {
    enum struct ContactQuality : uint8_t
    {
        Unqualified,
        Valid,
        StuckActive,
        SourceFault,
        TimingFault
    };

    enum struct ContactDisposition : uint8_t
    {
        None,
        Accepted,
        SuppressedDuringRefractory
    };

    struct ContactDynamicsConfig
    {
        ContactDynamicsConfig (Level activeLevel, Duration qualify, Duration release,
                               Duration refractory, Duration stuckActive) noexcept;

        Level    activeLevel;
        Duration qualify;
        Duration release;
        Duration refractory;
        Duration stuckActive;
    };

    struct ContactObservation
    {
        TimePoint          observedAt;
        Level              rawLevel;
        bool               rawActive;
        bool               qualifiedActive;
        bool               attackEvent;
        bool               releaseEvent;
        Duration           qualifiedPulseWidth;
        Duration           refractoryRemaining;
        uint32_t           acceptedCount;
        uint32_t           suppressedCount;
        ContactDisposition disposition;
        ContactQuality     quality;
        Status             status;
    };

    struct ContactSample
    {
        TimePoint observedAt;
        Level     rawLevel;
        Status    status;
    };

    // Pure copied-sample policy; owns no GPIO and therefore has no shutdown.
    struct ContactDynamics
    {
        explicit ContactDynamics (const ContactDynamicsConfig& config) noexcept;

        ContactDynamics (const ContactDynamics&)            = delete;
        ContactDynamics& operator= (const ContactDynamics&) = delete;
        ContactDynamics (ContactDynamics&&)                 = delete;
        ContactDynamics& operator= (ContactDynamics&&)      = delete;

        Status             initialize  () noexcept;
        void               reset       () noexcept;
        Status             update      (const ContactSample& sample) noexcept;
        bool               initialized () const noexcept;
        ContactObservation snapshot    () const noexcept;

      private:
        void publishTimingFault (const ContactSample& sample) noexcept;
        void updateRefractory   (TimePoint now) noexcept;

        ContactDynamicsConfig config_;
        ContactObservation    observation_;
        ContactSample         lastSample_;
        Status                sourceFaultStatus_;
        TimePoint             candidateSince_;
        TimePoint             acceptedAt_;
        TimePoint             releaseSince_;
        TimePoint             lastUpdate_;
        bool                  initialized_;
        bool                  hasLastSample_;
        bool                  candidateActive_;
        bool                  releaseCandidate_;
        bool                  acceptedPulseOpen_;
        bool                  stuckActive_;
        bool                  sourceFaulted_;
    };
} // namespace adk
// clang-format on
