#pragma once

#include "greenhouse_controller.h"
#include "rgb_led.h"

namespace adk {

    struct GreenhouseHealthPattern
    {
        explicit GreenhouseHealthPattern  (RgbLed& led) noexcept;
        ~GreenhouseHealthPattern          () noexcept;

        GreenhouseHealthPattern (const GreenhouseHealthPattern&)            = delete;
        GreenhouseHealthPattern& operator= (const GreenhouseHealthPattern&) = delete;
        GreenhouseHealthPattern (GreenhouseHealthPattern&&)                 = delete;
        GreenhouseHealthPattern& operator= (GreenhouseHealthPattern&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now, GreenhouseMode mode) noexcept;

        bool initialized () const noexcept;

      private:
        Rgb chooseColor (Duration elapsed, GreenhouseMode mode) const noexcept;

        RgbLed*        led_;
        GreenhouseMode mode_;
        Rgb            color_;
        TimePoint      modeSince_;
        TimePoint      lastUpdate_;
        bool           initialized_;
        bool           hasMode_;
        bool           hasUpdate_;
    };
} // namespace adk
