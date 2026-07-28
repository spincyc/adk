#pragma once

#include "orientation_presentation.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct BalanceInstrumentMode : uint8_t
    {
        AwaitingFrame,
        Live,
        Frozen,
        Recovering,
        Fault
    };

    enum struct SensitivityEvent : uint8_t
    {
        None,
        Increase,
        Decrease,
        Contradictory
    };

    struct BalanceJoystickObservation
    {
        int16_t          xPermille;
        int16_t          yPermille;
        SensitivityEvent event;
        TimePoint        observedAt;
        uint32_t         sequence;
        Status           status;
    };

    struct BalanceButtonObservation
    {
        bool      pressed;
        bool      pressEvent;
        bool      releaseEvent;
        TimePoint observedAt;
        uint32_t  sequence;
        Status    status;
    };

    struct BalanceInstrumentConfig
    {
        uint16_t            minimumSensitivityPermille;
        uint16_t            maximumSensitivityPermille;
        uint16_t            sensitivityStepPermille;
        Duration            inertialMaximumAge;
        uint16_t            inertialFreshnessContractRevision;
        Duration            maximumInputSkew;
        Duration            diagnosticPhase;
        BalancePresentation awaitingFramePresentation;
        BalancePresentation recoveringPresentation;
        BalancePresentation faultPresentation;
        BalancePresentation shutdownPresentation;
    };

    struct BalanceInstrumentInput
    {
        InertialObservation        inertial;
        BalanceJoystickObservation joystick;
        BalanceButtonObservation   freezeButton;
        TimePoint                  frameAt;
        uint32_t                   frameSequence;
    };

    struct CompactInertialEvidence
    {
        InertialSource        source;
        TimePoint             observedAt;
        uint32_t              sequence;
        InertialSampleQuality quality;
        Duration              maximumAge;
        uint16_t              freshnessContractRevision;
        InertialSaturation    saturation;
        bool                  acceptedDataReady;
        bool                  latestDataReady;
        Status                status;
    };

    struct BalanceMeasurementEvidence
    {
        CompactInertialEvidence provenance;
        OrientationEstimate     estimate;
        bool                    available;
    };

    struct BalanceFrameStorage
    {
        BalanceInstrumentInput previous;
        bool                   available;
    };

    struct BalanceInstrumentOutput
    {
        BalanceInstrumentMode      mode;
        BalanceMeasurementEvidence liveEvidence;
        BalanceMeasurementEvidence frozenEvidence;
        BalancePresentation        presentation;
        uint16_t                   sensitivityPermille;
        uint32_t                   acceptedFrameSequence;
        TimePoint                  acceptedFrameAt;
        Status                     inertialStatus;
        Status                     joystickStatus;
        Status                     buttonStatus;
        Status                     status;
    };

    struct BalanceInstrument
    {
        BalanceInstrument (const BalanceInstrumentConfig&   config,
                           const OrientationConfig&         orientationConfig,
                           const BalancePresentationConfig& presentationConfig,
                           BalanceFrameStorage&             replayStorage) noexcept;

        BalanceInstrument (const BalanceInstrument&)            = delete;
        BalanceInstrument& operator= (const BalanceInstrument&) = delete;
        BalanceInstrument (BalanceInstrument&&)                 = delete;
        BalanceInstrument& operator= (BalanceInstrument&&)      = delete;

        Status                  initialize       () noexcept;
        void                    shutdown         () noexcept;
        Status                  acknowledgeFault () noexcept;
        Status                  update           (const BalanceInstrumentInput& input) noexcept;
        BalanceInstrumentOutput snapshot         () const noexcept;
        bool                    initialized      () const noexcept;

      private:
        BalanceInstrumentConfig   config_;
        OrientationPolicy         orientation_;
        BalancePresentationPolicy presentation_;
        Status                    policyConfigurationStatus_;
        BalanceFrameStorage*      replayStorage_;
        BalanceInstrumentOutput   output_;
        bool                      initialized_;
        bool                      hasEpoch_;
        bool                      latestFrameHealthy_;
        bool                      recoveryNeedsForwardFrame_;
        TimePoint                 diagnosticEpoch_;
    };
} // namespace adk
