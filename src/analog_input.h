#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    struct AnalogInput
    {
        using Reading = uint16_t;

        static constexpr Reading maximumReading = 1023;

        AnalogInput  (ResourceRegistry& resources,
                      PinId             pin) noexcept;
        ~AnalogInput () noexcept;

        AnalogInput            (const AnalogInput&) = delete;
        AnalogInput& operator= (const AnalogInput&) = delete;
        AnalogInput            (AnalogInput&&)      = delete;
        AnalogInput& operator= (AnalogInput&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        void   update     () noexcept;

        Reading sample () const noexcept;
        Reading read   () const noexcept;

        PinId pin         () const noexcept;
        bool  initialized () const noexcept;

      private:
        ResourceRegistry* resources_;
        ResourceClaim     claim_;
        PinId             pin_;
        Reading           reading_;
        bool              initialized_;
    };
}
