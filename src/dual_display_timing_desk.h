#pragma once

#include "max7219_presentation_policy.h"
#include "multiplexed_digit_policy.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {
    // clang-format off

    enum struct TimingDeskStopwatchState : uint8_t
    {
        Stopped,
        Running,
        Paused,
        Faulted
    };

    enum struct TimingDeskQualification : uint8_t
    {
        Configuring,
        SelfTest,
        Ready,
        Requalify,
        Fault
    };

    enum struct TimingDeskPresentationDisposition : uint8_t
    {
        Configuring,
        SelfTest,
        Pending,
        InSync,
        ResourceBusy,
        Disagreement,
        Fault
    };

    enum struct TimingDeskFaultOwner : uint8_t
    {
        None,
        Control,
        DigitDisplay,
        MatrixDisplay,
        BothDisplays,
        Coordinator
    };

    struct TimingDeskControlIdentity
    {
        uint16_t sourceId;
        uint16_t configurationRevision;
        uint32_t sessionEpoch;
    };

    struct TimingDeskControlEvidence
    {
        TimingDeskControlIdentity source;
        uint32_t                  sequence;
        TimePoint                 observedAt;
        Status                    status;
        bool                      pressed;
        bool                      pressEvent;
    };

    struct DigitFrameReceipt
    {
        uint32_t                 ownerToken;
        uint32_t                 lifecycleGeneration;
        uint16_t                 configurationRevision;
        uint32_t                 requestedGeneration;
        uint32_t                 acceptedGeneration;
        MultiplexedDigitFrame    reportedFrame;
        uint32_t                 reportedDigest;
        TimePoint                observedAt;
        Status                   status;
        bool                     blankRequestAccepted;
    };

    struct MatrixFrameReceipt
    {
        uint32_t          ownerToken;
        uint32_t          lifecycleGeneration;
        uint16_t          configurationRevision;
        uint32_t          requestedGeneration;
        uint32_t          acceptedGeneration;
        Max7219Frame      reportedFrame;
        uint32_t          reportedDigest;
        TimePoint         observedAt;
        Status            status;
        bool              blankRequestAccepted;
    };

    struct DualDisplayTimingDeskConfig
    {
        DualDisplayTimingDeskConfig (
            uint32_t ownerToken, uint16_t configurationRevision,
            const MultiplexedDigitConfig& digitConfig,
            const Max7219PresentationConfig& matrixConfig,
            const TimingDeskControlIdentity& startPauseSource,
            const TimingDeskControlIdentity& lapSource,
            const TimingDeskControlIdentity& resetSource,
            Duration presentationGrace = Duration  (100),
            Duration selfTestTimeout    = Duration (100),
            Duration controlFreshness   = Duration (100)) noexcept;

        uint32_t                       ownerToken;
        uint16_t                       configurationRevision;
        MultiplexedDigitConfig         digitConfig;
        Max7219PresentationConfig      matrixConfig;
        TimingDeskControlIdentity      startPauseSource;
        TimingDeskControlIdentity      lapSource;
        TimingDeskControlIdentity      resetSource;
        Duration                       presentationGrace;
        Duration                       selfTestTimeout;
        Duration                       controlFreshness;
    };

    struct DualDisplayEnvelope
    {
        TimePoint                  now;
        TimingDeskControlEvidence  startPause;
        TimingDeskControlEvidence  lap;
        TimingDeskControlEvidence  reset;
        const DigitFrameReceipt*   digitReceipt;
        const MatrixFrameReceipt*  matrixReceipt;
        const Max7219Receipt*      transportReceipt;
    };

    struct DualDisplayTimingDeskResult
    {
        Status                             controlStatus;
        TimingDeskPresentationDisposition presentationDisposition;
        MultiplexedDigitTransaction        digitTransaction;
        Max7219Command                     matrixCommand;
        uint32_t                           presentationGeneration;
        uint32_t                           digitDigest;
        uint32_t                           matrixDigest;
        bool                               digitTransactionPresent;
        bool                               matrixCommandPresent;
        bool                               digitBlankRequested;
        bool                               matrixBlankRequested;
    };

    struct DualDisplayTimingDeskSnapshot
    {
        MultiplexedDigitFrame               digitFrame;
        Max7219Frame                        matrixFrame;
        Duration                            elapsed;
        Duration                            lapElapsed;
        TimePoint                           presentationPublishedAt;
        TimePoint                           selfTestStageStartedAt;
        uint32_t                            lifecycleGeneration;
        uint32_t                            snapshotSequence;
        uint32_t                            presentationGeneration;
        uint32_t                            digitDigest;
        uint32_t                            matrixDigest;
        uint8_t                             selfTestStage;
        TimingDeskStopwatchState            stopwatchState;
        TimingDeskQualification             qualification;
        TimingDeskPresentationDisposition  presentationDisposition;
        TimingDeskFaultOwner                faultOwner;
        Status                              status;
        bool                                lapVisible;
        bool                                digitAccepted;
        bool                                matrixAccepted;
        bool                                initialized;
    };

    struct DualDisplayTimingDeskTestAccess;

    // Pure coordinator. It owns no button, endpoint, bus, pin, timer, or clock.
    struct DualDisplayTimingDesk
    {
        explicit DualDisplayTimingDesk (
            const DualDisplayTimingDeskConfig& config) noexcept;

        DualDisplayTimingDesk (const DualDisplayTimingDesk&) = delete;
        DualDisplayTimingDesk& operator= (const DualDisplayTimingDesk&) = delete;
        DualDisplayTimingDesk (DualDisplayTimingDesk&&) = delete;
        DualDisplayTimingDesk& operator= (DualDisplayTimingDesk&&) = delete;

        Status initialize (TimePoint now) noexcept;
        void   reset      (TimePoint now) noexcept;
        void   shutdown   () noexcept;

        Result<DualDisplayTimingDeskResult> update (
            const DualDisplayEnvelope& envelope) noexcept;

        bool                          initialized () const noexcept;
        DualDisplayTimingDeskSnapshot snapshot    () const noexcept;

      private:
        Status publishPresentation    (TimePoint now) noexcept;
        void   enterPresentationFault (TimingDeskFaultOwner owner,
                                       Status status) noexcept;

        DualDisplayTimingDeskConfig      config_;
        MultiplexedDigitPolicy           digitPolicy_;
        Max7219PresentationPolicy        matrixPolicy_;
        MultiplexedDigitPreview          digitPreview_;
        Max7219PresentationPreview       matrixPreview_;
        MultiplexedDigitSnapshot         digitSnapshot_;
        Max7219PresentationSnapshot      matrixSnapshot_;
        Result<MultiplexedDigitTransaction> digitService_;
        Result<Max7219Command>            matrixService_;
        MultiplexedDigitFrame            expectedDigitFrame_;
        Max7219Frame                     expectedMatrixFrame_;
        uint8_t                          logicalRows_[8];
        TimePoint                        lastUpdateAt_;
        TimePoint                        runStartedAt_;
        TimePoint                        lapUntil_;
        TimePoint                        presentationPublishedAt_;
        TimePoint                        selfTestStageStartedAt_;
        Duration                         materializedElapsed_;
        Duration                         lapElapsed_;
        uint32_t                         lifecycleGeneration_;
        uint32_t                         snapshotSequence_;
        uint32_t                         presentationGeneration_;
        uint32_t                         digitDigest_;
        uint32_t                         matrixDigest_;
        uint32_t                         lastControlSequences_[3];
        uint8_t                          selfTestStage_;
        TimingDeskStopwatchState         stopwatchState_;
        TimingDeskQualification          qualification_;
        TimingDeskPresentationDisposition presentationDisposition_;
        TimingDeskFaultOwner             faultOwner_;
        Status                           status_;
        bool                             haveLastUpdate_;
        bool                             lapVisible_;
        bool                             digitAccepted_;
        bool                             matrixAccepted_;
        bool                             initialized_;

        friend struct DualDisplayTimingDeskTestAccess;
    };
    // clang-format on
} // namespace adk
