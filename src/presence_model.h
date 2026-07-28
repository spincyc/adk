#pragma once

#include "digital.h"
#include "optical_observation.h"
#include "status.h"
#include "time.h"
#include "ultrasonic_ranger.h"

#include <stdint.h>

namespace adk {

    enum struct PirPhase : uint8_t
    {
        Warming,
        ReadyClear,
        Motion,
        StuckMotion,
        Fault
    };

    enum struct PresenceQuality : uint8_t
    {
        Unqualified,
        Valid,
        Stale,
        Disagreement,
        SourceFault,
        TimingFault
    };

    struct PirSample
    {
        uint8_t   sourceId;
        TimePoint observedAt;
        Level     rawLevel;
        Status    status;
    };

    struct PirObservation
    {
        uint8_t   sourceId;
        TimePoint observedAt;
        Level     rawLevel;
        PirPhase  phase;
        bool      motionEvent;
        bool      clearEvent;
        Duration  stableFor;
        Status    status;
    };

    struct PirObservationConfig
    {
        uint8_t  sourceId;
        Level    motionLevel;
        Duration warmup;
        Duration qualifyMotion;
        Duration qualifyClear;
        Duration stuckMotion;
    };

    struct PirObservationPolicy
    {
        explicit PirObservationPolicy (const PirObservationConfig& config) noexcept;

        PirObservationPolicy (const PirObservationPolicy&)            = delete;
        PirObservationPolicy& operator= (const PirObservationPolicy&) = delete;
        PirObservationPolicy (PirObservationPolicy&&)                 = delete;
        PirObservationPolicy& operator= (PirObservationPolicy&&)      = delete;

        Status         initialize  () noexcept;
        void           reset       () noexcept;
        Status         update      (const PirSample& sample) noexcept;
        PirObservation snapshot    () const noexcept;
        bool           initialized () const noexcept;

      private:
        bool validConfig () const noexcept;

        PirObservationConfig config_;
        PirObservation       observation_;
        PirSample            lastSample_;
        TimePoint            warmupStarted_;
        TimePoint            candidateStarted_;
        TimePoint            stableStarted_;
        uint8_t              state_;
    };

    struct TimedRangeEvidence
    {
        uint8_t              sourceId;
        TimePoint            startedAt;
        TimePoint            completedAt;
        MicrosecondTimePoint measurementStartedAt;
        MicrosecondDuration  measurementLatency;
        RangeReading         reading;
        Status               status;
    };

    struct OptionalPirObservation
    {
        bool           present;
        PirObservation value;
    };

    struct OptionalBeamObservation
    {
        bool            present;
        BeamObservation value;
    };

    struct OptionalReflectiveObservation
    {
        bool                  present;
        ReflectiveObservation value;
    };

    struct OptionalTimedRangeEvidence
    {
        bool               present;
        TimedRangeEvidence value;
    };

    struct PresenceInput
    {
        TimePoint                     observedAt;
        OptionalPirObservation        pir;
        OptionalBeamObservation       beam;
        OptionalReflectiveObservation finishGuard;
        OptionalTimedRangeEvidence    range;
    };

    enum struct PresenceSourceBit : uint8_t
    {
        Pir         = 1U << 0,
        Beam        = 1U << 1,
        FinishGuard = 1U << 2,
        Range       = 1U << 3
    };

    struct PresenceModelConfig
    {
        uint8_t  requiredSources;
        uint8_t  agreementSources;
        Duration pirMaximumAge;
        Duration beamMaximumAge;
        Duration finishGuardMaximumAge;
        Duration rangeMaximumAge;
        Duration beamPassageWindow;
        Duration agreementWindow;
        uint16_t approachMinimumMm;
        uint16_t approachMaximumMm;
    };

    struct PirPresenceState
    {
        bool           available;
        PirObservation evidence;
        Duration       age;
        bool           valid;
        bool           stale;
    };

    struct OpticalPresenceState
    {
        bool              available;
        OpticalProvenance provenance;
        OpticalQuality    quality;
        Duration          age;
        bool              valid;
        bool              stale;
        bool              active;
        bool              activationEvent;
        bool              deactivationEvent;
        Status            status;
    };

    struct RangePresenceState
    {
        bool               available;
        TimedRangeEvidence evidence;
        Duration           age;
        bool               valid;
        bool               stale;
        bool               approachValid;
    };

    struct PresenceSnapshot
    {
        PirPresenceState     pir;
        OpticalPresenceState beam;
        OpticalPresenceState finishGuard;
        RangePresenceState   range;
        bool                 pirEligible;
        bool                 passageEvent;
        bool                 disagreement;
        Duration             disagreementFor;
        PresenceQuality      quality;
        Status               status;
    };

    struct PresenceModel
    {
        explicit PresenceModel (const PresenceModelConfig& config) noexcept;

        PresenceModel (const PresenceModel&)            = delete;
        PresenceModel& operator= (const PresenceModel&) = delete;
        PresenceModel (PresenceModel&&)                 = delete;
        PresenceModel& operator= (PresenceModel&&)      = delete;

        Status           initialize  () noexcept;
        void             reset       () noexcept;
        Status           update      (const PresenceInput& input) noexcept;
        PresenceSnapshot snapshot    () const noexcept;
        bool             initialized () const noexcept;

      private:
        bool             validConfig  () const noexcept;
        PresenceSnapshot makeSnapshot () const noexcept;

        PresenceModelConfig config_;
        // Absent wrappers retain prior payload; presence marks the current frame.
        PresenceInput       evidence_;
        TimePoint           disagreementStarted_;
        TimePoint           beamInterruptedAt_;
        uint8_t             state_;
    };
} // namespace adk
