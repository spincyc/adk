#pragma once

#include "digital.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct OpticalSourceKind : uint8_t
    {
        ReflectiveAnalog,
        InterruptedDigital
    };

    enum struct OpticalQuality : uint8_t
    {
        Unqualified,
        Valid,
        BelowQualifiedRange,
        AboveQualifiedRange,
        DegenerateCalibration,
        SourceFault,
        TimingFault
    };

    struct OpticalProvenance
    {
        uint8_t   sourceId;
        uint16_t  calibrationRevision;
        TimePoint observedAt;
    };

    struct ReflectiveSample
    {
        OpticalProvenance provenance;
        uint16_t          raw;
        Status            status;
    };

    struct BeamSample
    {
        OpticalProvenance provenance;
        Level             rawLevel;
        Status            status;
    };

    struct ReflectiveObservation
    {
        OpticalProvenance provenance;
        uint16_t          raw;
        uint16_t          darkReference;
        uint16_t          lightReference;
        uint16_t          normalizedPermille;
        bool              markerActive;
        bool              activationEvent;
        bool              deactivationEvent;
        Duration          stableFor;
        OpticalQuality    quality;
        Status            status;
    };

    struct BeamObservation
    {
        OpticalProvenance provenance;
        Level             rawLevel;
        bool              interrupted;
        bool              interruptionEvent;
        bool              restorationEvent;
        Duration          stableFor;
        OpticalQuality    quality;
        Status            status;
    };

    struct ReflectiveObservationConfig
    {
        uint8_t  sourceId;
        uint16_t calibrationRevision;
        uint16_t qualifiedMinimum;
        uint16_t qualifiedMaximum;
        uint16_t darkReference;
        uint16_t lightReference;
        uint16_t activatePermille;
        uint16_t releasePermille;
        Duration dwell;
        bool     darkerIsActive;
    };

    struct BeamObservationConfig
    {
        uint8_t  sourceId;
        uint16_t calibrationRevision;
        Level    interruptedLevel;
        Duration interruptDwell;
        Duration restoreDwell;
    };

    // Pure copied-sample policies; they own no endpoint or clock.
    struct ReflectiveObservationPolicy
    {
        explicit ReflectiveObservationPolicy (
            const ReflectiveObservationConfig& config) noexcept;

        ReflectiveObservationPolicy (const ReflectiveObservationPolicy&) = delete;
        ReflectiveObservationPolicy&
        operator= (const ReflectiveObservationPolicy&)                         = delete;
        ReflectiveObservationPolicy (ReflectiveObservationPolicy&&)            = delete;
        ReflectiveObservationPolicy& operator= (ReflectiveObservationPolicy&&) = delete;

        Status                initialize  () noexcept;
        void                  reset       () noexcept;
        Status                update      (const ReflectiveSample& sample) noexcept;
        ReflectiveObservation snapshot    () const noexcept;
        bool                  initialized () const noexcept;

      private:
        ReflectiveObservationConfig config_;
        ReflectiveObservation       observation_;
        ReflectiveSample            lastSample_;
        TimePoint                   candidateSince_;
        TimePoint                   stableSince_;
        bool                        initialized_;
        bool                        hasSample_;
        bool                        hasCandidate_;
        bool                        candidateActive_;
        bool                        faulted_;
    };

    struct BeamObservationPolicy
    {
        explicit BeamObservationPolicy (const BeamObservationConfig& config) noexcept;

        BeamObservationPolicy (const BeamObservationPolicy&)            = delete;
        BeamObservationPolicy& operator= (const BeamObservationPolicy&) = delete;
        BeamObservationPolicy (BeamObservationPolicy&&)                 = delete;
        BeamObservationPolicy& operator= (BeamObservationPolicy&&)      = delete;

        Status          initialize  () noexcept;
        void            reset       () noexcept;
        Status          update      (const BeamSample& sample) noexcept;
        BeamObservation snapshot    () const noexcept;
        bool            initialized () const noexcept;

      private:
        BeamObservationConfig config_;
        BeamObservation       observation_;
        BeamSample            lastSample_;
        TimePoint             candidateSince_;
        TimePoint             stableSince_;
        bool                  initialized_;
        bool                  hasSample_;
        bool                  hasCandidate_;
        bool                  candidateInterrupted_;
        bool                  faulted_;
    };
} // namespace adk
