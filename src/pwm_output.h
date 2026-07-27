#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    // Default-frequency PWM shares the board timer configuration.
    // Frequency-changing endpoints must claim their timer.
    struct PwmOutput
    {
        using Duty = uint8_t;

        PwmOutput  (ResourceRegistry& resources,
                    PinId            pin,
                    Duty             initialDuty = 0) noexcept;
        ~PwmOutput () noexcept;

        PwmOutput            (const PwmOutput&) = delete;
        PwmOutput& operator= (const PwmOutput&) = delete;
        PwmOutput            (PwmOutput&&)      = delete;
        PwmOutput& operator= (PwmOutput&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status write (Duty duty) noexcept;

        PinId pin         () const noexcept;
        Duty  duty        () const noexcept;
        bool  initialized () const noexcept;

      private:
        ResourceRegistry*   resources_;
        ResourceClaim       pinClaim_;
        SharedResourceClaim timerClaim_;
        PinId               pin_;
        Duty                initialDuty_;
        Duty                duty_;
        bool                initialized_;
    };
}
