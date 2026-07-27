#pragma once

#include "digital_output.h"

namespace adk {

    struct MonoLed
    {
        MonoLed (ResourceRegistry& resources,
                 PinId            pin,
                 bool             activeHigh = true) noexcept;
        ~MonoLed () noexcept;

        MonoLed            (const MonoLed&) = delete;
        MonoLed& operator= (const MonoLed&) = delete;
        MonoLed            (MonoLed&&)      = delete;
        MonoLed& operator= (MonoLed&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status set (bool active) noexcept;
        Status on  () noexcept;
        Status off () noexcept;

        bool active      () const noexcept;
        bool activeHigh  () const noexcept;
        bool initialized () const noexcept;
        PinId pin        () const noexcept;

      private:
        Level physicalLevel (bool active) const noexcept;

        DigitalOutput output_;
        bool          activeHigh_;
        bool          active_;
    };
}
