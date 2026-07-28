#pragma once

#include "carousel_evidence.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct HomingPhase : uint8_t
    {
        Uninitialized,
        PositionUnknown,
        SeekingHomeRelease,
        SeekingHome,
        Homed,
        Moving,
        Stopped,
        Fault
    };

    enum struct HomingFault : uint8_t
    {
        None,
        InvalidConfiguration,
        HomeStuckActive,
        HomeNotFound,
        TravelExceeded,
        EvidenceFault,
        TimingFault,
        Interrupted
    };

    struct BoundedHomingConfig
    {
        int32_t  minimumLogicalPosition;
        int32_t  maximumLogicalPosition;
        int32_t  homeLogicalPosition;
        int8_t   homeSearchDirection;
        uint16_t maximumReleaseSteps;
        uint16_t maximumSearchSteps;
        Duration maximumReleaseDuration;
        Duration maximumSearchDuration;
        Duration stepInterval;
        Duration maximumEvidenceAge;
        Duration maximumInputSkew;
    };

    struct HomingInput
    {
        TimePoint            frameAt;
        uint32_t             frameSequence;
        CopiedBinaryEvidence home;
        CopiedBinaryEvidence stop;
    };

    struct HomingCommand
    {
        bool    requestHome;
        bool    requestMove;
        int32_t targetLogicalPosition;
    };

    struct HomingSnapshot
    {
        HomingPhase phase;
        HomingFault fault;
        int32_t     logicalPosition;
        bool        positionKnown;
        int8_t      stepDirection;
        bool        stepRequested;
        int8_t      requestedStepDirection;
        bool        stopIntent;
        uint16_t    homingSteps;
        uint32_t    homeEpoch;
        uint32_t    acceptedFrameSequence;
        Status      status;
    };

    struct HomingExcursionBounds
    {
        int32_t minimum;
        int32_t maximum;
    };

    struct BoundedHomingPolicy;

    struct HomingPreview
    {
        HomingPreview () noexcept;

        HomingSnapshot snapshot;
        Status         status;

      private:
        friend struct BoundedHomingPolicy;

        const BoundedHomingPolicy* owner_;
        CopiedBinaryEvidence       lastHome_;
        CopiedBinaryEvidence       lastStop_;
        TimePoint                  lastUpdateAt_;
        TimePoint                  phaseStartedAt_;
        TimePoint                  nextStepAt_;
        int32_t                    targetLogicalPosition_;
        uint32_t                   generation_;
        uint32_t                   attemptQualificationEpoch_;
        uint32_t                   lastHomeEpoch_;
        bool                       hasLastHome_ : 1;
        bool                       hasLastStop_ : 1;
        bool                       hasLastUpdate_ : 1;
        bool                       hasNextStep_ : 1;
        bool                       homeWasActive_ : 1;
    };

    // Pure copied-evidence policy; it owns no endpoint, actuator, or clock.
    struct BoundedHomingPolicy
    {
        explicit BoundedHomingPolicy (const BoundedHomingConfig& config) noexcept;

        BoundedHomingPolicy (const BoundedHomingPolicy&)            = delete;
        BoundedHomingPolicy& operator= (const BoundedHomingPolicy&) = delete;
        BoundedHomingPolicy (BoundedHomingPolicy&&)                 = delete;
        BoundedHomingPolicy& operator= (BoundedHomingPolicy&&)      = delete;

        // clang-format off
        Status initialize () noexcept;
        void   reset      () noexcept;
        void   shutdown   () noexcept;

        Status preview   (TimePoint now, const HomingInput& input,
                          const HomingCommand& command,
                          HomingPreview& candidate) const noexcept;
        bool   canCommit (const HomingPreview& candidate) const noexcept;
        Status commit    (const HomingPreview& candidate) noexcept;

        bool                   initialized     () const noexcept;
        HomingSnapshot         snapshot        () const noexcept;
        HomingExcursionBounds  excursionBounds () const noexcept;
        // clang-format on

      private:
        static void publishFault (HomingPreview& candidate, HomingFault fault,
                                  Status status) noexcept;

        BoundedHomingConfig  config_;
        HomingSnapshot       snapshot_;
        CopiedBinaryEvidence lastHome_;
        CopiedBinaryEvidence lastStop_;
        TimePoint            lastUpdateAt_;
        TimePoint            phaseStartedAt_;
        TimePoint            nextStepAt_;
        int32_t              targetLogicalPosition_;
        uint32_t             generation_;
        uint32_t             attemptQualificationEpoch_;
        uint32_t             lastHomeEpoch_;
        bool                 hasLastHome_ : 1;
        bool                 hasLastStop_ : 1;
        bool                 hasLastUpdate_ : 1;
        bool                 hasNextStep_ : 1;
        bool                 homeWasActive_ : 1;
        bool                 initialized_ : 1;
    };
} // namespace adk
