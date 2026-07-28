#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct StepDirection : uint8_t
    {
        Stopped,
        Forward,
        Reverse
    };

    enum struct StepSequencePhase : uint8_t
    {
        Inactive,
        Holding,
        Moving,
        Complete,
        Cancelled,
        Fault
    };

    enum struct StepSequenceDisposition : uint8_t
    {
        None,
        Accepted,
        Replaced,
        Cancelled,
        Rejected
    };

    struct StepperSequenceConfig
    {
        StepperSequenceConfig (Duration minimumStepInterval,
                               Duration maximumStepInterval, Duration maximumCommandAge,
                               int32_t minimumLogicalPosition,
                               int32_t maximumLogicalPosition,
                               bool    holdAtRest) noexcept;

        Duration minimumStepInterval;
        Duration maximumStepInterval;
        Duration maximumCommandAge;
        int32_t  minimumLogicalPosition;
        int32_t  maximumLogicalPosition;
        bool     holdAtRest;
    };

    struct StepperCommand
    {
        uint32_t      commandId;
        TimePoint     issuedAt;
        StepDirection direction;
        uint32_t      stepCount;
        Duration      stepInterval;
        bool          cancel;
        Status        status;
    };

    struct BoundedStepperSequence;

    struct StepperSequenceSnapshot
    {
        uint32_t                commandId;
        StepSequencePhase       phase;
        StepSequenceDisposition disposition;
        StepDirection           direction;
        int32_t                 logicalPosition;
        uint32_t                requestedSteps;
        uint32_t                completedSteps;
        uint8_t                 coilIntent;
        TimePoint               phaseSince;
        TimePoint               nextStepAt;
        bool                    hasDeadline;
        Status                  status;
    };

    struct StepperSequencePreview
    {
        StepperSequencePreview () noexcept;

      private:
        friend struct BoundedStepperSequence;

        StepperSequenceSnapshot       snapshot_;
        StepperCommand                command_;
        const BoundedStepperSequence* owner_;
        TimePoint                     lastUpdateAt_;
        uint32_t                      generation_;
        uint8_t                       phaseIndex_;
        bool                          hasCommand_;
        bool                          hasIdentity_;
        bool                          hasLastUpdate_;
    };

    // Pure logical intent policy; it owns no endpoint, timer, or clock.
    struct BoundedStepperSequence
    {
        explicit BoundedStepperSequence (const StepperSequenceConfig& config) noexcept;

        BoundedStepperSequence (const BoundedStepperSequence&)            = delete;
        BoundedStepperSequence& operator= (const BoundedStepperSequence&) = delete;
        BoundedStepperSequence (BoundedStepperSequence&&)                 = delete;
        BoundedStepperSequence& operator= (BoundedStepperSequence&&)      = delete;

        // clang-format off
        Status initialize () noexcept;
        void   reset      () noexcept;
        Status preview    (TimePoint now, const StepperCommand& command,
                           StepperSequencePreview& candidate) const noexcept;
        bool   canCommit  (const StepperSequencePreview& candidate) const noexcept;
        Status commit     (const StepperSequencePreview& candidate) noexcept;
        Status stop       (TimePoint now) noexcept;

        bool                    initialized () const noexcept;
        StepperSequenceSnapshot snapshot    () const noexcept;
        // clang-format on

      private:
        StepperSequenceConfig   config_;
        StepperSequenceSnapshot snapshot_;
        StepperCommand          command_;
        TimePoint               lastUpdateAt_;
        uint32_t                generation_;
        uint8_t                 phaseIndex_;
        bool                    hasCommand_;
        bool                    hasIdentity_;
        bool                    hasLastUpdate_;
        bool                    initialized_;
    };
} // namespace adk
