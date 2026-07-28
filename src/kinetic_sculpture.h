#pragma once

#include "bounded_stepper_sequence.h"
#include "interaction_intent_policy.h"

#include <stdint.h>

namespace adk {
    // clang-format off
    enum struct SculpturePhase : uint8_t
    {
        Inactive,
        Ready,
        Preview,
        Running,
        Complete,
        Stopped,
        Fault
    };

    enum struct AuthorizationDisposition : uint8_t
    {
        None,
        Pending,
        Accepted,
        Inhibited,
        BoundRejected
    };

    struct AuthorizationRecord
    {
        uint32_t                 originatingFrameId;
        InteractionSource        contactSource;
        InteractionSource        directionalSource;
        uint32_t                 contactSequence;
        uint32_t                 directionalSequence;
        InteractionDirection     direction;
        AuthorizationDisposition disposition;
        Status                   status;
    };

    struct SculptureInput
    {
        TimePoint           observedAt;
        uint32_t            frameId;
        InteractionSource   stopSource;
        TimePoint           stopObservedAt;
        uint32_t            stopSequence;
        bool                stopActive;
        ContactQuality      stopQuality;
        Status              stopStatus;
        InteractionSource   touchSource;
        uint32_t            touchSequence;
        ContactSample       touchSample;
        DirectionalEvidence directional;
        Status              status;
    };

    struct SculptureLightIntent
    {
        uint8_t shiftRegisterBits;
        bool    ready;
        bool    running;
        bool    stopped;
        bool    fault;
        bool    travelLimit;
    };

    struct SculptureSnapshot
    {
        uint32_t                frameId;
        SculpturePhase          phase;
        InteractionIntent       interaction;
        StepperSequenceSnapshot motion;
        SculptureLightIntent    lights;
        InteractionSource       stopSource;
        TimePoint               stopObservedAt;
        uint32_t                stopSequence;
        bool                    stopActive;
        bool                    hasStopIdentity;
        bool                    travelLimit;
        bool                    hasPendingAuthorization;
        AuthorizationRecord     pendingAuthorization;
        bool                    hasLastTerminalAuthorization;
        AuthorizationRecord     lastTerminalAuthorization;
        ContactQuality          stopQuality;
        Status                  stopStatus;
        Status                  interactionStatus;
        Status                  motionStatus;
        uint32_t                acceptedMotifCount;
        Status                  status;
    };

    struct KineticLightSculpture
    {
        KineticLightSculpture (const InteractionIntentConfig& interactionConfig,
                               const StepperSequenceConfig&   sequenceConfig,
                               Duration                       maximumFrameAge,
                               Duration maximumSourceSkew) noexcept;

        KineticLightSculpture (const KineticLightSculpture&)            = delete;
        KineticLightSculpture& operator= (const KineticLightSculpture&) = delete;
        KineticLightSculpture (KineticLightSculpture&&)                 = delete;
        KineticLightSculpture& operator= (KineticLightSculpture&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (const SculptureInput& input) noexcept;

        bool              initialized () const noexcept;
        SculptureSnapshot snapshot    () const noexcept;

      private:
        InteractionIntentPolicy interaction_;
        BoundedStepperSequence  sequence_;
        Duration                maximumFrameAge_;
        Duration                maximumSourceSkew_;
        SculptureSnapshot       snapshot_;
        StepperCommand          motionCommand_;
        TimePoint               lastProjectTime_;
        TimePoint               pendingCreatedAt_;
        TimePoint               lastStopObservedAt_;
        InteractionSource       lastStopSource_;
        int32_t                 minimumLogicalPosition_;
        int32_t                 maximumLogicalPosition_;
        uint32_t                lastFrameId_;
        uint32_t                lastStopSequence_;
        uint32_t                nextCommandId_;
        uint32_t                terminalFrameId_;
        bool                    initialized_;
        bool                    hasProjectTime_;
        bool                    hasFrameIdentity_;
        bool                    hasStopEvidence_;
        bool                    hasMotionCommand_;
        bool                    lastStopActive_;
        ContactQuality          lastStopQuality_;
        Status                  lastStopStatus_;
        bool                    stoppedLatch_;
        bool                    faultLatch_;
    };
    // clang-format on
} // namespace adk
