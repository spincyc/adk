#pragma once

#include "presence_model.h"

#include <stdint.h>

namespace adk {

    struct CheckpointId
    {
        uint8_t value;
    };

    struct CheckpointBinding
    {
        CheckpointId      checkpointId;
        OpticalSourceKind sourceKind;
        uint8_t           sourceId;
        uint16_t          calibrationRevision;
    };

    enum struct CourseStartSource : uint8_t
    {
        None,
        ExplicitButtonWithPirEligibility
    };

    struct CourseStartInput
    {
        TimePoint        observedAt;
        uint8_t          buttonSourceId;
        bool             buttonPressEvent;
        PirPresenceState pir;
    };

    struct CourseStartEvent
    {
        bool              present;
        CourseStartSource source;
        uint8_t           buttonSourceId;
        TimePoint         observedAt;
        PirPresenceState  pir;
        Status            status;
    };

    struct CourseStartPolicy
    {
        explicit CourseStartPolicy (uint8_t buttonSourceId) noexcept;

        CourseStartPolicy (const CourseStartPolicy&)            = delete;
        CourseStartPolicy& operator= (const CourseStartPolicy&) = delete;
        CourseStartPolicy (CourseStartPolicy&&)                 = delete;
        CourseStartPolicy& operator= (CourseStartPolicy&&)      = delete;

        Status           initialize  () noexcept;
        void             reset       () noexcept;
        Status           update      (const CourseStartInput& input) noexcept;
        CourseStartEvent snapshot    () const noexcept;
        bool             initialized () const noexcept;

      private:
        uint8_t          buttonSourceId_;
        CourseStartEvent event_;
        CourseStartInput lastInput_;
        bool             initialized_;
        bool             hasInput_;
    };

    struct CheckpointEvent
    {
        CheckpointId      checkpointId;
        OpticalSourceKind sourceKind;
        OpticalProvenance provenance;
        OpticalQuality    quality;
        Status            status;
    };

    struct CourseMarshalConfig
    {
        CheckpointBinding orderedCheckpoints[4];
        uint8_t           checkpointCount;
        uint8_t           buttonSourceId;
        Duration          checkpointEventMaximumAge;
        Duration          checkpointSimultaneityWindow;
        Duration          finishAgreementWindow;
        Duration          maximumRunDuration;
    };

    enum struct MarshalPhase : uint8_t
    {
        Disarmed,
        Arming,
        Ready,
        Running,
        Finished,
        Rejected,
        Fault
    };

    enum struct RunDisposition : uint8_t
    {
        None,
        Accepted,
        SkippedCheckpoint,
        ReversedCheckpoint,
        DuplicateCheckpoint,
        SimultaneousCheckpoints,
        FinishTooEarly,
        TimedOut,
        EvidenceFault
    };

    enum struct RunTriggerKind : uint8_t
    {
        None,
        Checkpoint,
        FinishGuard,
        Range,
        Start,
        RunTimeout,
        PresenceFault
    };

    struct RecordedCheckpoint
    {
        TimePoint         observedAt;
        uint16_t          calibrationRevision;
        CheckpointId      checkpointId;
        OpticalSourceKind sourceKind;
        uint8_t           sourceId;
        OpticalQuality    quality;
        Status            status;
    };

    struct CourseTriggerRecord
    {
        RunTriggerKind     kind;
        TimePoint          observedAt;
        uint8_t            checkpointCount;
        RecordedCheckpoint checkpoints[4];
        CourseStartEvent   start;
        Status             status;
    };

    struct CourseRunRecord
    {
        uint32_t           sequence;
        TimePoint          startedAt;
        TimePoint          finishedAt;
        Duration           elapsed;
        uint8_t            acceptedCheckpointCount;
        RecordedCheckpoint acceptedCheckpoints[4];
        CourseStartEvent   start;
        RunDisposition     disposition;
        bool               sequenceExhausted;
        Status             status;
    };

    // Caller-owned bounded storage. A marshal retains every storage reference
    // for its complete lifetime; none may be moved or destroyed first.
    struct CourseRunStorage
    {
        CourseRunRecord record;
    };

    struct CourseTriggerStorage
    {
        CourseTriggerRecord trigger;
    };

    // The selected presence payload lives separately so both fixed storage
    // objects remain within the AVR object ceiling. It is canonical zero
    // unless trigger.kind is FinishGuard, Range, or PresenceFault.
    struct CourseTriggerPresenceStorage
    {
        PresenceSnapshot presence;
    };

    struct CourseReplayFrameStorage
    {
        TimePoint        observedAt;
        CourseStartEvent start;
        uint8_t          eventCount;
        bool             present;
    };

    struct CourseReplayPresenceStorage
    {
        PresenceSnapshot presence;
    };

    struct CourseReplayEventStorage
    {
        CheckpointEvent events[4];
    };

    struct CourseMarshalInputView
    {
        // Pointers need only remain valid for update(); the marshal copies a
        // fieldwise replay identity and never retains either pointer.
        TimePoint               observedAt;
        CourseStartEvent        start;
        const PresenceSnapshot* presence;
        const CheckpointEvent*  events;
        uint8_t                 eventCount;
    };

    struct CourseMarshalSnapshot
    {
        MarshalPhase   phase;
        uint8_t        checkpointCount;
        uint8_t        expectedSlot;
        CheckpointId   expectedCheckpointId;
        uint8_t        acceptedCheckpointCount;
        Duration       elapsed;
        bool           hasRecord;
        uint32_t       recordSequence;
        RunDisposition disposition;
        Status         status;
    };

    struct CourseMarshal
    {
        CourseMarshal (const CourseMarshalConfig& config, CourseRunStorage& runStorage,
                       CourseTriggerStorage&         triggerStorage,
                       CourseTriggerPresenceStorage& triggerPresenceStorage,
                       CourseReplayFrameStorage&     replayFrameStorage,
                       CourseReplayPresenceStorage&  replayPresenceStorage,
                       CourseReplayEventStorage&     replayEventStorage) noexcept;

        CourseMarshal (const CourseMarshal&)            = delete;
        CourseMarshal& operator= (const CourseMarshal&) = delete;
        CourseMarshal (CourseMarshal&&)                 = delete;
        CourseMarshal& operator= (CourseMarshal&&)      = delete;

        Status                 initialize          () noexcept;
        void                   reset               () noexcept;
        void                   acknowledgeRecord   () noexcept;
        Status                 update              (const CourseMarshalInputView& input) noexcept;
        CourseMarshalSnapshot  snapshot            () const noexcept;
        const CourseRunRecord& record              () const noexcept;
        const CourseTriggerRecord& trigger         () const noexcept;
        const PresenceSnapshot&    triggerPresence () const noexcept;
        bool                       initialized     () const noexcept;

      private:
        bool validConfig () const noexcept;

        CourseMarshalConfig           config_;
        CourseRunStorage*             runStorage_;
        CourseTriggerStorage*         triggerStorage_;
        CourseTriggerPresenceStorage* triggerPresenceStorage_;
        CourseReplayFrameStorage*     replayFrameStorage_;
        CourseReplayPresenceStorage*  replayPresenceStorage_;
        CourseReplayEventStorage*     replayEventStorage_;
        TimePoint                     lastFrameAt_;
        TimePoint                     startedAt_;
        uint32_t                      nextSequence_;
        uint8_t                       expectedSlot_;
        MarshalPhase                  phase_;
        Status                        status_;
        bool                          initialized_;
        bool                          hasFrame_;
        bool                          hasRecord_;
        bool                          sequenceExhausted_;
    };

    enum struct CoursePresentationPhase : uint8_t
    {
        Starting,
        Ready,
        Running,
        Finished,
        Rejected,
        Fault
    };

    struct CoursePresentationIntent
    {
        TimePoint               observedAt;
        Duration                elapsed;
        uint32_t                recordSequence;
        CoursePresentationPhase phase;
        uint8_t                 acceptedMask;
        uint8_t                 expectedSlot;
        uint8_t                 displayCell;
        uint8_t                 displayValue;
        bool                    allRed;
        bool                    heartbeat;
        bool                    hasRecord;
        Status                  status;
    };

    struct CourseMarshalPresenter
    {
        CourseMarshalPresenter (Duration displayQuantum,
                                Duration heartbeatInterval) noexcept;

        CourseMarshalPresenter (const CourseMarshalPresenter&)            = delete;
        CourseMarshalPresenter& operator= (const CourseMarshalPresenter&) = delete;
        CourseMarshalPresenter (CourseMarshalPresenter&&)                 = delete;
        CourseMarshalPresenter& operator= (CourseMarshalPresenter&&)      = delete;

        Status initialize                    () noexcept;
        void   reset                         () noexcept;
        Status update                        (TimePoint now, const CourseMarshalSnapshot& snapshot) noexcept;
        CoursePresentationIntent intent      () const noexcept;
        bool                     initialized () const noexcept;

      private:
        Duration                 displayQuantum_;
        Duration                 heartbeatInterval_;
        CoursePresentationIntent intent_;
        CourseMarshalSnapshot    lastSnapshot_;
        TimePoint                epoch_;
        TimePoint                lastUpdate_;
        bool                     initialized_;
        bool                     hasUpdate_;
    };
} // namespace adk
