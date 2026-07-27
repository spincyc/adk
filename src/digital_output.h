#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"

namespace adk {

    struct DigitalOutput
    {
        DigitalOutput (ResourceRegistry& resources,
                       PinId            pin,
                       Level            initial = Level::Low) noexcept;
        ~DigitalOutput () noexcept;

        DigitalOutput            (const DigitalOutput&) = delete;
        DigitalOutput& operator= (const DigitalOutput&) = delete;
        DigitalOutput            (DigitalOutput&&)      = delete;
        DigitalOutput& operator= (DigitalOutput&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status write (Level level) noexcept;

        PinId pin         () const noexcept;
        Level level       () const noexcept;
        bool  initialized () const noexcept;

      private:
        ResourceRegistry* resources_;
        ResourceClaim     claim_;
        PinId             pin_;
        Level             initial_;
        Level             level_;
        bool              initialized_;
    };
}
