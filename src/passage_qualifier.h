#pragma once

#include "magnetic_observation.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

// clang-format off
namespace adk {

    enum struct PassageBoundary : uint8_t
    {
        None,
        A,
        B
    };

    enum struct PassageDirection : uint8_t
    {
        Unknown,
        AToB,
        BToA
    };

    enum struct PassageDisposition : uint8_t
    {
        Accepted,
        TimedOut,
        DuplicateSuppressed,
        Ambiguous,
        EvidenceFault
    };

    enum struct PassagePhase : uint8_t
    {
        Idle,
        FirstBoundary,
        AwaitingSecond,
        Suppressing,
        Fault
    };

    struct PassagePositionEvidence
    {
        bool    present;
        bool    reliable;
        bool    saturated;
        int32_t onsetPosition;
        int32_t endPosition;
        int32_t delta;
    };

    struct PassageQualifierConfig
    {
        Duration boundaryDwell;
        Duration passageTimeout;
        Duration duplicateWindow;
    };

    struct PassageInput
    {
        TimePoint           observedAt;
        MagneticObservation boundaryA;
        MagneticObservation boundaryB;
        bool                hasPosition;
        int32_t             position;
        Status              positionStatus;
    };

    struct PassageRecord
    {
        uint32_t                sequence;
        PassageDirection        direction;
        PassageDisposition      disposition;
        TimePoint               onset;
        TimePoint               end;
        Duration                elapsed;
        MagneticPolarity        onsetPolarity;
        MagneticPolarity        endPolarity;
        PassagePositionEvidence position;
        uint32_t                acceptedCount;
        uint32_t                suppressedCount;
        Status                  status;
    };

    struct PassageSnapshot
    {
        PassagePhase    phase;
        PassageBoundary firstBoundary;
        Duration        elapsed;
        uint32_t        nextSequence;
        uint32_t        acceptedCount;
        uint32_t        suppressedCount;
        bool            hasRecord;
        PassageRecord   record;
        Status          status;
    };

    struct PassageQualifier
    {
        explicit PassageQualifier (PassageQualifierConfig config) noexcept;


        Status initialize () noexcept;

        void   reset () noexcept;

        void   update (const PassageInput& input) noexcept;


        PassageSnapshot snapshot () const noexcept;

        bool            initialized () const noexcept;

      private:
        void clearActivity () noexcept;

        void enterFault (const PassageInput& input, Status status) noexcept;

        void emit (PassageDisposition disposition, PassageDirection direction,
                   TimePoint end, MagneticPolarity endPolarity, Status status) noexcept;

        void updatePosition (const PassageInput& input) noexcept;

        PassageQualifierConfig config_;
        PassageSnapshot        snapshot_;
        TimePoint              lastUpdate_;
        TimePoint              activeSinceA_;
        TimePoint              activeSinceB_;
        TimePoint              onset_;
        TimePoint              suppressionSince_;
        PassageInput           lastInput_;
        int32_t                onsetPosition_;
        Status                 onsetPositionStatus_;
        MagneticPolarity       onsetPolarity_;
        bool                   onsetHasPosition_;
        bool                   hasUpdate_;
        bool                   activeA_;
        bool                   activeB_;
        bool                   qualifiedA_;
        bool                   qualifiedB_;
        bool                   retreating_;
        bool                   suppressionActivation_;
        bool                   initialized_;
    };
} // namespace adk
// clang-format on
