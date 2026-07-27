#pragma once

#include "pulse_capture.h"

namespace adk {

    void megaPulseInterrupt0 () noexcept;
    void megaPulseInterrupt1 () noexcept;
    void megaPulseInterrupt2 () noexcept;
    void megaPulseInterrupt3 () noexcept;
    void megaPulseInterrupt4 () noexcept;
    void megaPulseInterrupt5 () noexcept;

    struct MegaPulseCaptureIo : PulseCaptureIo
    {
        MegaPulseCaptureIo  () noexcept;
        ~MegaPulseCaptureIo () noexcept override;

        MegaPulseCaptureIo& operator= (const MegaPulseCaptureIo&) = delete;
        MegaPulseCaptureIo (const MegaPulseCaptureIo&)            = delete;
        MegaPulseCaptureIo& operator= (MegaPulseCaptureIo&&)      = delete;
        MegaPulseCaptureIo (MegaPulseCaptureIo&&)                 = delete;

        Status               start     (PinId pin, PulseCapture& capture) noexcept override;
        void                 stop      () noexcept override;
        bool                 inputHigh () const noexcept override;
        MicrosecondTimePoint now       () const noexcept override;
        void                 lock      () noexcept override;
        void                 unlock    () noexcept override;

      private:
        void handleInterrupt () noexcept;

        friend void megaPulseInterrupt0 () noexcept;
        friend void megaPulseInterrupt1 () noexcept;
        friend void megaPulseInterrupt2 () noexcept;
        friend void megaPulseInterrupt3 () noexcept;
        friend void megaPulseInterrupt4 () noexcept;
        friend void megaPulseInterrupt5 () noexcept;

        PulseCapture* capture_;
        PinId         pin_;
        uint8_t       interrupt_;
        uint8_t       interruptState_;
        bool          started_;
    };
} // namespace adk
