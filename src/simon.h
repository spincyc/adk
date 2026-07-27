#pragma once

#include "status.h"
#include "time.h"

#include <stddef.h>
#include <stdint.h>

namespace adk {

    enum struct CueId : uint8_t
    {
        One   = 0,
        Two   = 1,
        Three = 2,
        Four  = 3
    };

    enum struct SimonPhase : uint8_t
    {
        Idle,
        PlaybackOn,
        PlaybackGap,
        AwaitPress,
        AwaitRelease,
        RoundSuccess,
        GameSuccess,
        GameFailure
    };

    enum struct SimonOutcome : uint8_t
    {
        None,
        RoundComplete,
        GameComplete,
        Mismatch,
        InvalidInput,
        Timeout,
        SourceFailure
    };

    enum struct SimonAlgorithm : uint8_t
    {
        Fixed = 0,
        XorShift32V1 = 1
    };

    struct SimonInput
    {
        SimonInput (uint8_t activeMask   = 0,
                    uint8_t pressedMask  = 0,
                    uint8_t releasedMask = 0,
                    bool    startEvent   = false) noexcept;

        uint8_t activeMask   = 0;
        uint8_t pressedMask  = 0;
        uint8_t releasedMask = 0;
        bool    startEvent   = false;
    };

    struct SimonConfig
    {
        Duration cueOnDuration   = Duration (500);
        Duration cueGapDuration  = Duration (200);
        Duration inputTimeout    = Duration (2000);
        Duration resultDuration  = Duration (700);
        uint8_t  startingLength  = 1;
        uint8_t  growthPerRound  = 1;
        uint8_t  maximumLength   = 16;
    };

    struct CueSource
    {
        virtual ~CueSource () noexcept;

        virtual Status         reset            () noexcept = 0;
        virtual Status         next             (CueId& cue) noexcept = 0;
        virtual SimonAlgorithm algorithmVersion () const noexcept = 0;
        virtual uint32_t       seed             () const noexcept = 0;
    };

    struct FixedCueSource final : CueSource
    {
        FixedCueSource (const CueId* cues, size_t count) noexcept;

        Status         reset            () noexcept override;
        Status         next             (CueId& cue) noexcept override;
        SimonAlgorithm algorithmVersion () const noexcept override;
        uint32_t       seed             () const noexcept override;

      private:
        const CueId* cues_;
        size_t       count_;
        size_t       index_;
    };

    struct XorShift32CueSource final : CueSource
    {
        explicit XorShift32CueSource (uint32_t seed) noexcept;

        Status         reset            () noexcept override;
        Status         next             (CueId& cue) noexcept override;
        SimonAlgorithm algorithmVersion () const noexcept override;
        uint32_t       seed             () const noexcept override;

      private:
        uint32_t initialSeed_;
        uint32_t state_;
    };

    struct SimonSnapshot
    {
        SimonPhase     phase;
        SimonOutcome   outcome;
        Status         status;
        CueId          displayedCue;
        CueId          expectedCue;
        CueId          observedCue;
        TimePoint      nextDeadline;
        uint8_t        ledMask;
        uint8_t        sequenceLength;
        uint8_t        playbackIndex;
        uint8_t        playerIndex;
        bool           inputAccepted;
        bool           hasDisplayedCue;
        bool           hasExpectedCue;
        bool           hasObservedCue;
        bool           hasDeadline;
    };

    struct Simon
    {
        static constexpr uint8_t cueCount         = 4;
        static constexpr uint8_t sequenceCapacity = 32;

        Simon (const SimonConfig& config, CueSource& source) noexcept;

        Status initialize () noexcept;
        Status update     (TimePoint now, const SimonInput& input) noexcept;

        SimonSnapshot snapshot () const noexcept;

        SimonAlgorithm algorithmVersion () const noexcept;
        uint32_t       seed             () const noexcept;

      private:
        bool   configValid     () const noexcept;
        bool   inputValid      (const SimonInput& input) const noexcept;
        bool   timeValid       (TimePoint now) const noexcept;
        bool   deadlineDue     (TimePoint now, Duration duration) const noexcept;
        Status growSequence    (uint8_t targetLength) noexcept;
        void   startPlayback   (TimePoint now) noexcept;
        void   fail            (SimonOutcome outcome,
                                TimePoint     now,
                                uint8_t       observedMask = 0) noexcept;
        void   enter           (SimonPhase phase, TimePoint now) noexcept;
        CueId  cueAt           (uint8_t index) const noexcept;
        uint8_t cueMask        (CueId cue) const noexcept;
        CueId  cueFromMask     (uint8_t mask) const noexcept;
        bool   oneCue          (uint8_t mask) const noexcept;

        SimonConfig  config_;
        CueSource*   source_;
        CueId        sequence_[sequenceCapacity];
        SimonPhase   phase_;
        SimonOutcome outcome_;
        Status       status_;
        TimePoint    phaseSince_;
        TimePoint    lastUpdate_;
        CueId        observedCue_;
        uint8_t      sequenceLength_;
        uint8_t      playbackIndex_;
        uint8_t      playerIndex_;
        bool         initialized_;
        bool         hasLastUpdate_;
        bool         hasObservedCue_;
    };
}
