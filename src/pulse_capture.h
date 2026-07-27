#pragma once

#include <stdint.h>

#include "digital.h"
#include "pulse_input.h"
#include "resource.h"
#include "status.h"

namespace adk {

    enum struct PulseLevel : uint8_t
    {
        Mark,
        Space
    };

    struct Pulse
    {
        Pulse (PulseLevel          level    = PulseLevel::Space,
               MicrosecondDuration duration = MicrosecondDuration ()) noexcept;

        PulseLevel          level;
        MicrosecondDuration duration;
    };

    enum struct CaptureState : uint8_t
    {
        Idle,
        Capturing,
        Complete,
        Overflow,
        TimingFault
    };

    struct PulseCaptureConfig
    {
        MicrosecondDuration frameGap;
        MicrosecondDuration minimumPulse;
        MicrosecondDuration maximumPulse;
    };

    struct PulseFrame
    {
        const Pulse* data;
        uint8_t      size;
        uint32_t     sequence;
        CaptureState state;
    };

    struct PulseCapture;
    struct PulseCaptureTestAccess;

    struct PulseCaptureIo
    {
        virtual ~PulseCaptureIo () noexcept = default;

        virtual Status start                (PinId pin, PulseCapture& capture) noexcept = 0;
        virtual void   stop                 () noexcept                                  = 0;
        virtual bool   inputHigh            () const noexcept                       = 0;
        virtual MicrosecondTimePoint now    () const noexcept               = 0;
        virtual void                 lock   () noexcept                    = 0;
        virtual void                 unlock () noexcept                  = 0;
    };

    struct PulseCapture
    {
        static constexpr uint8_t capacity = 100;

        PulseCapture (ResourceRegistry& resources, PulseCaptureIo& io, PinId pin,
                      const PulseCaptureConfig& config) noexcept;
        ~PulseCapture () noexcept;

        PulseCapture& operator= (const PulseCapture&) = delete;
        PulseCapture (const PulseCapture&)            = delete;
        PulseCapture& operator= (PulseCapture&&)      = delete;
        PulseCapture (PulseCapture&&)                 = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (MicrosecondTimePoint now) noexcept;

        PulseFrame frame       () const noexcept;
        Status     acknowledge (uint32_t sequence) noexcept;

        void recordActiveLowEdge (MicrosecondTimePoint now, bool inputHigh) noexcept;

        PinId pin         () const noexcept;
        bool  initialized () const noexcept;

      private:
        friend struct PulseCaptureTestAccess;

        void    processEdge  (MicrosecondTimePoint now, bool inputHigh) noexcept;
        bool    validConfig  () const noexcept;
        Status  publish      (CaptureState state) noexcept;
        void    resetCapture () noexcept;
        uint8_t interruptFor (PinId pin) const noexcept;

        ResourceRegistry*  resources_;
        PulseCaptureIo*    io_;
        ResourceClaim      pinClaim_;
        ResourceClaim      interruptClaim_;
        PulseCaptureConfig config_;
        uint32_t           captureDurations_[capacity];
        uint8_t            captureLevels_[capacity];
        Pulse              published_[capacity];
        volatile uint32_t  edgeTimes_[2][capacity + 1];
        volatile uint8_t   edgeLevels_[2][capacity + 1];
        volatile uint32_t  initializedAtUs_;
        volatile uint32_t  lastEdgeUs_;
        PulseFrame         frame_;
        CaptureState       captureState_;
        CaptureState       pendingState_;
        PinId              pin_;
        uint8_t            captureSize_;
        volatile uint8_t   edgeCounts_[2];
        volatile uint8_t   activeEdgeQueue_;
        uint32_t           nextSequence_;
        bool               inputHigh_;
        bool               haveEdge_;
        bool               idleGapSeen_;
        volatile bool      initialized_;
        bool               awaitingAcknowledgement_;
        bool               pendingPublication_;
        volatile bool      edgeQueueOverflow_;
        bool               overrun_;
    };
} // namespace adk
