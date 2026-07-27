#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct PiezoSounder
    {
        using Frequency = uint16_t;

        static constexpr Frequency minimumFrequencyHz = 31;
        static constexpr Frequency maximumFrequencyHz = 20000;
        static constexpr uint32_t  maximumDurationMs  = 60000;

        PiezoSounder  (ResourceRegistry& resources, PinId pin) noexcept;
        ~PiezoSounder () noexcept;

        PiezoSounder& operator= (const PiezoSounder&) = delete;
        PiezoSounder  (const PiezoSounder&)            = delete;
        PiezoSounder& operator= (PiezoSounder&&)       = delete;
        PiezoSounder  (PiezoSounder&&)                 = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status play   (Frequency frequency, Duration duration, TimePoint now) noexcept;
        void   stop   () noexcept;
        void   update (TimePoint now) noexcept;

        PinId     pin         () const noexcept;
        Frequency frequency   () const noexcept;
        bool      initialized () const noexcept;
        bool      sounding    () const noexcept;

      private:
        static constexpr uint8_t toneTimer = 2;

        ResourceRegistry* resources_;
        ResourceClaim     pinClaim_;
        ResourceClaim     timerClaim_;
        PinId             pin_;
        Frequency         frequency_;
        Duration          duration_;
        TimePoint         startedAt_;
        bool              initialized_;
        bool              sounding_;
    };
}
