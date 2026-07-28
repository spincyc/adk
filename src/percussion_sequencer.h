#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct PercussionMode : uint8_t
    {
        Recording,
        Playing,
        Full,
        Fault
    };

    enum struct PercussionFaultSource : uint8_t
    {
        None,
        Surface0,
        Surface1,
        Surface2,
        Surface3,
        Acoustic,
        Timing,
        Tempo,
        Input
    };

    enum struct PercussionAssociation : uint8_t
    {
        None,
        AcousticCompletion,
        AssociationTimeout
    };

    struct PercussionHit
    {
        uint8_t               surface;
        uint8_t               step;
        uint16_t              intensity;
        uint32_t              ordinal;
        PercussionAssociation association;
    };

    struct PercussionSequencerConfig
    {
        PercussionSequencerConfig (uint8_t steps, uint16_t minimumTempoBpm,
                                   uint16_t maximumTempoBpm,
                                   Duration simultaneousWindow,
                                   Duration acousticAssociationTimeout) noexcept;

        uint8_t  steps;
        uint16_t minimumTempoBpm;
        uint16_t maximumTempoBpm;
        Duration simultaneousWindow;
        Duration acousticAssociationTimeout;
    };

    struct PercussionAcousticCompletion
    {
        PercussionAcousticCompletion () noexcept;

        bool      present;
        TimePoint eventStartedAt;
        Duration  eventDuration;
        uint16_t  intensity;
    };

    struct PercussionSequencerInput
    {
        PercussionSequencerInput () noexcept;

        TimePoint                    observedAt;
        uint8_t                      attackMask;
        Status                       surfaceStatus[4];
        Status                       acousticStatus;
        PercussionAcousticCompletion acousticCompletion;
        uint16_t                     tempoPosition;
        bool                         playEvent;
        bool                         clearEvent;
    };

    struct PercussionFrame
    {
        uint8_t  step;
        uint8_t  surfaceMask;
        uint16_t intensity[4];
        uint16_t frequencyHz;
        Duration toneDuration;
        bool     heartbeat;
    };

    struct PercussionSequencerSnapshot
    {
        PercussionMode        mode;
        uint16_t              tempoBpm;
        uint8_t               currentStep;
        uint8_t               hitCount;
        uint32_t              nextOrdinal;
        bool                  hitAccepted;
        bool                  hitSuppressed;
        bool                  patternFull;
        bool                  frameValid;
        PercussionFaultSource faultSource;
        PercussionAssociation lastAssociation;
        PercussionHit         lastHit;
        PercussionFrame       frame;
        Status                status;
    };

    struct PercussionSequencer
    {
        static constexpr uint8_t maximumHits  = 32;
        static constexpr uint8_t maximumSteps = 16;

        explicit PercussionSequencer (const PercussionSequencerConfig& config) noexcept;

        PercussionSequencer (const PercussionSequencer&)            = delete;
        PercussionSequencer& operator= (const PercussionSequencer&) = delete;
        PercussionSequencer (PercussionSequencer&&)                 = delete;
        PercussionSequencer& operator= (PercussionSequencer&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (const PercussionSequencerInput& input) noexcept;
        void   clear      () noexcept;

        bool                        initialized () const noexcept;
        PercussionSequencerSnapshot snapshot    () const noexcept;
        Result<PercussionHit>       hit         (uint8_t index) const noexcept;

      private:
        struct StoredHit
        {
            uint8_t  surface;
            uint8_t  step;
            uint16_t intensity;
            uint32_t ordinal;
        };

        bool configValid () const noexcept;
        bool inputEqual  (const PercussionSequencerInput& left,
                         const PercussionSequencerInput& right) const noexcept;
        bool inputEvidenceValid (const PercussionSequencerInput& input, Status& failure,
                                 PercussionFaultSource& source) const noexcept;
        bool acousticContainsAttack (
            const PercussionAcousticCompletion& completion) const noexcept;
        uint16_t mapTempo         (uint16_t position) const noexcept;
        uint32_t stepMilliseconds (uint16_t tempoBpm) const noexcept;
        uint8_t  quantizedStep    (TimePoint attackAt) const noexcept;
        void     clearTransient   () noexcept;
        void     clearPattern     () noexcept;
        void     beginPending     (TimePoint attackAt) noexcept;
        void     collectAttacks   (const PercussionSequencerInput& input) noexcept;
        void     finalizePending  (uint16_t              intensity,
                                  PercussionAssociation association) noexcept;
        void     togglePlayback (TimePoint now, uint16_t tempoBpm) noexcept;
        void     applyPlayback  (TimePoint now, uint16_t tempoBpm) noexcept;
        void     publishFrame   () noexcept;
        void     enterFault     (Status status, PercussionFaultSource source) noexcept;

        PercussionSequencerConfig   config_;
        PercussionSequencerSnapshot snapshot_;
        StoredHit                   hits_[maximumHits];
        uint32_t                    timeoutAssociations_;
        PercussionSequencerInput    lastInput_;
        TimePoint                   lastUpdate_;
        TimePoint                   recordingEpoch_;
        TimePoint                   pendingFirstAttack_;
        TimePoint                   playbackDeadline_;
        uint16_t                    recordingTempoBpm_;
        uint32_t                    playbackStepCount_;
        bool                        pendingSurfaces_[4];
        bool                        initialized_;
        bool                        hasLastUpdate_;
        bool                        hasRecordingEpoch_;
        bool                        pending_;
        bool                        pendingClosed_;
    };
} // namespace adk
