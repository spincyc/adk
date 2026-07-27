#pragma once

#include <stdint.h>

#include "cue_audit.h"
#include "status.h"
#include "time.h"

namespace adk {

    struct InertCue
    {
        InertCueId id;
        Duration   offset;
        Duration   visibleFor;
    };

    struct InertCuePlan
    {
        static constexpr uint8_t capacity = 32;

        InertCue cues[capacity];
        uint8_t  count;
    };

    struct InertCueSchedulerConfig
    {
        InertCuePlan plan;
        Duration     confirmationWindow;
    };

    enum struct CueDecision : uint8_t
    {
        Waiting,
        ConfirmationRequired,
        Active,
        Complete,
        Skipped,
        Held,
        Cancelled
    };

    struct CueOperatorInput
    {
        bool reviewHeld;
        bool runPressed;
        bool confirmPressed;
        bool skipPressed;
        bool cancelPressed;
    };

    enum struct CueSchedulerPhase : uint8_t
    {
        Idle,
        Review,
        Waiting,
        Confirmation,
        Active,
        Held,
        Complete,
        Cancelled,
        Fault
    };

    struct CueSchedulerSnapshot
    {
        CueSchedulerPhase phase;
        CueDecision       decision;
        InertCueId        cue;
        uint8_t           cueIndex;
        Duration          planElapsed;
        Duration          cueElapsed;
        Status            status;
        bool              hasCue;
    };

    struct InertCueScheduler
    {
        InertCueScheduler (const InertCueSchedulerConfig& config,
                           CueAuditBuffer&                audit) noexcept;
        ~InertCueScheduler () noexcept;

        InertCueScheduler (const InertCueScheduler&)            = delete;
        InertCueScheduler& operator= (const InertCueScheduler&) = delete;
        InertCueScheduler (InertCueScheduler&&)                 = delete;
        InertCueScheduler& operator= (InertCueScheduler&&)      = delete;

        Status initialize () noexcept;

        void   shutdown () noexcept;

        bool   initialized () const noexcept;

        Status update (TimePoint now, const CueOperatorInput& input) noexcept;

        CueSchedulerSnapshot snapshot () const noexcept;

      private:
        bool validPlan () const noexcept;

        bool append (TimePoint now, CueAuditEvent event, Status status = StatusCode::Ok,
                     bool hasCue = false, uint8_t cueIndex = 0) noexcept;
        void enterHeld (TimePoint now, Status status) noexcept;

        Status enterFault (TimePoint now, Status status) noexcept;

        Status process (TimePoint now, const CueOperatorInput& input) noexcept;

        Status processIdle (TimePoint now, const CueOperatorInput& input) noexcept;

        Status processReview (TimePoint now, const CueOperatorInput& input) noexcept;

        Status processWaiting (TimePoint now, const CueOperatorInput& input) noexcept;

        Status processConfirmation (TimePoint               now,
                                    const CueOperatorInput& input) noexcept;
        Status processActive (TimePoint now, const CueOperatorInput& input) noexcept;

        Status finishCue (TimePoint now, CueAuditEvent firstEvent,
                          Status firstStatus) noexcept;
        void   refreshElapsed (TimePoint now) noexcept;

        bool   hasChord (const CueOperatorInput& input) const noexcept;

        bool   sameInput (const CueOperatorInput& left,
                          const CueOperatorInput& right) const noexcept;

        InertCuePlan         plan_;
        Duration             confirmationWindow_;
        CueAuditBuffer&      audit_;
        CueSchedulerSnapshot snapshot_;
        TimePoint            planStartedAt_;
        TimePoint            cueShownAt_;
        TimePoint            lastUpdatedAt_;
        CueOperatorInput     lastInput_;
        bool                 planStarted_;
        bool                 hasLastUpdate_;
        bool                 initialized_;
    };
} // namespace adk
