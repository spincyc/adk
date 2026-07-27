#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"

namespace adk {

    struct DigitalInput
    {
        DigitalInput  (ResourceRegistry& resources,
                       PinId             pin,
                       Pull              pull = Pull::None) noexcept;
        ~DigitalInput () noexcept;

        DigitalInput& operator= (const DigitalInput&) = delete;
        DigitalInput  (const DigitalInput&)            = delete;
        DigitalInput& operator= (DigitalInput&&)       = delete;
        DigitalInput  (DigitalInput&&)                 = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        void   update     () noexcept;

        Level sample () const noexcept;
        Level read   () const noexcept;

        PinId pin         () const noexcept;
        Pull  pull        () const noexcept;
        bool  initialized () const noexcept;

      private:
        ResourceRegistry* resources_;
        ResourceClaim     claim_;
        PinId             pin_;
        Pull              pull_;
        Level             level_;
        bool              initialized_;
    };
}
