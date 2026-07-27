#pragma once

#include "status.h"
#include "time.h"

namespace adk {

    struct Button;

    enum struct ReactionState : uint8_t
    {
        Idle,
        AwaitRelease,
        Ready,
        Wait,
        Cue,
        Success,
        Failure
    };

    enum struct ReactionOutcome : uint8_t
    {
        None,
        Success,
        PrematurePress,
        Timeout
    };

    enum struct ReactionLedPattern : uint8_t
    {
        Off,
        ReadyPulse,
        Cue,
        SuccessPulse,
        FailurePulse
    };

    struct ReactionTimerConfig
    {
        Duration readyDuration   = Duration (1000);
        Duration waitDuration    = Duration (2000);
        Duration responseTimeout = Duration (1500);
        Duration resultDuration  = Duration (1000);
    };

    struct ButtonObservation
    {
        bool pressed      = false;
        bool pressEvent   = false;
        bool releaseEvent = false;
    };

    struct ReactionTimerSnapshot
    {
        ReactionState      state;
        ReactionOutcome    outcome;
        ReactionLedPattern ledPattern;
        Status             status;
        TimePoint          cueTime;
        Duration           reactionTime;
        bool               cueVisible;
        bool               ledOn;
        bool               hasCueTime;
        bool               hasReactionTime;
    };

    struct ReactionTimer
    {
        explicit ReactionTimer (const ReactionTimerConfig& config) noexcept;

        Status initialize () noexcept;
        Status update     (TimePoint now, const ButtonObservation& button) noexcept;
        Status update     (TimePoint now, const Button& button) noexcept;

        ReactionTimerSnapshot snapshot () const noexcept;

      private:
        bool configValid  () const noexcept;
        bool deadlineDue  (TimePoint now, Duration duration) const noexcept;
        bool timeValid    (TimePoint now) const noexcept;
        bool outputActive (TimePoint now) const noexcept;
        void enter        (ReactionState state, TimePoint now) noexcept;

        ReactionTimerConfig config_;
        ReactionState       state_;
        ReactionOutcome     outcome_;
        Status              status_;
        TimePoint           stateSince_;
        TimePoint           cueTime_;
        TimePoint           lastUpdate_;
        Duration            reactionTime_;
        bool                initialized_;
        bool                hasLastUpdate_;
        bool                hasCueTime_;
        bool                hasReactionTime_;
    };
}
