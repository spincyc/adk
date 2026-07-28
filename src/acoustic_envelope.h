#pragma once

#include "digital.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct AcousticPhase : uint8_t
    {
        Calibrating,
        Quiet,
        EventOpen,
        Refractory,
        Fault
    };

    enum struct AcousticQuality : uint8_t
    {
        Unqualified,
        ValidQuiet,
        ValidEvent,
        ClippedLow,
        ClippedHigh,
        ThresholdDisagreement,
        SourceFault,
        TimingFault
    };

    struct AcousticEnvelopeConfig
    {
        AcousticEnvelopeConfig (bool hasThreshold, Level thresholdActiveLevel,
                                uint16_t railMargin, uint16_t attackAboveBaseline,
                                uint16_t releaseAboveBaseline, uint8_t baselineShift,
                                Duration calibration, Duration eventWindow,
                                Duration quietToClose, Duration refractory) noexcept;

        bool     hasThreshold;
        Level    thresholdActiveLevel;
        uint16_t railMargin;
        uint16_t attackAboveBaseline;
        uint16_t releaseAboveBaseline;
        uint8_t  baselineShift;
        Duration calibration;
        Duration eventWindow;
        Duration quietToClose;
        Duration refractory;
    };

    struct AcousticObservation
    {
        TimePoint       observedAt;
        uint16_t        raw;
        uint16_t        baseline;
        uint16_t        amplitude;
        uint16_t        peakAmplitude;
        uint16_t        relativeIntensity;
        bool            rawThresholdActive;
        bool            eventStarted;
        bool            eventCompleted;
        TimePoint       eventStartedAt;
        Duration        eventDuration;
        AcousticPhase   phase;
        AcousticQuality quality;
        Status          status;
    };

    struct AcousticSample
    {
        TimePoint observedAt;
        uint16_t  raw;
        bool      hasThreshold;
        Level     thresholdLevel;
        Status    analogStatus;
        Status    thresholdStatus;
    };

    struct AcousticEnvelope
    {
        explicit AcousticEnvelope (const AcousticEnvelopeConfig& config) noexcept;

        AcousticEnvelope (const AcousticEnvelope&)            = delete;
        AcousticEnvelope& operator= (const AcousticEnvelope&) = delete;
        AcousticEnvelope (AcousticEnvelope&&)                 = delete;
        AcousticEnvelope& operator= (AcousticEnvelope&&)      = delete;

        Status              initialize () noexcept;

        void                reset () noexcept;

        Status              update (const AcousticSample& sample) noexcept;

        bool                initialized () const noexcept;

        AcousticObservation snapshot () const noexcept;

      private:
        bool     validConfig () const noexcept;

        bool     sameSample (const AcousticSample& sample) const noexcept;

        uint16_t amplitudeFrom (uint16_t raw) const noexcept;

        bool     validHeadroom () const noexcept;

        void     clearCurrentEvent () noexcept;

        void     rememberSample (const AcousticSample& sample) noexcept;

        Status   enterFault (const AcousticSample& sample, AcousticQuality quality,
                             Status status, bool preserveEvidence) noexcept;
        Status   checkThreshold (const AcousticSample& sample,
                                 uint16_t              amplitude) noexcept;
        void     updateBaseline (uint16_t raw) noexcept;

        void     openEvent (const AcousticSample& sample, uint16_t amplitude) noexcept;

        void     completeEvent (const AcousticSample& sample) noexcept;

        uint16_t completedIntensity () const noexcept;

        AcousticEnvelopeConfig config_;
        AcousticObservation    snapshot_;
        AcousticSample         lastSample_;
        TimePoint              calibrationStartedAt_;
        TimePoint              eventStartedAt_;
        TimePoint              quietStartedAt_;
        TimePoint              refractoryStartedAt_;
        TimePoint              disagreementStartedAt_;
        TimePoint              completedStartedAt_;
        Duration               completedDuration_;
        uint16_t               completedPeak_;
        uint16_t               completedIntensity_;
        bool                   initialized_;
        bool                   hasSample_;
        bool                   calibrationStarted_;
        bool                   quietCandidate_;
        bool                   disagreementCandidate_;
        bool                   hasCompleted_;
    };
} // namespace adk
