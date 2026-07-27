#pragma once

#include "pwm_output.h"

#include <stdint.h>

namespace adk {

    struct Rgb
    {
        Rgb (uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) noexcept;

        bool operator== (const Rgb& other) const noexcept;
        bool operator!= (const Rgb& other) const noexcept;

        uint8_t red   () const noexcept;
        uint8_t green () const noexcept;
        uint8_t blue  () const noexcept;

      private:
        uint8_t red_;
        uint8_t green_;
        uint8_t blue_;
    };

    struct RgbLedChannel
    {
        PinId    pin;
        uint16_t resistorOhms;
    };

    struct RgbLed
    {
        RgbLed (ResourceRegistry&    resources,
                const RgbLedChannel& red,
                const RgbLedChannel& green,
                const RgbLedChannel& blue) noexcept;
        ~RgbLed () noexcept;

        RgbLed            (const RgbLed&) = delete;
        RgbLed& operator= (const RgbLed&) = delete;
        RgbLed            (RgbLed&&)      = delete;
        RgbLed& operator= (RgbLed&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status set (const Rgb& color) noexcept;
        Status off () noexcept;

        const Rgb& color       () const noexcept;
        bool       initialized () const noexcept;

        const RgbLedChannel& redChannel   () const noexcept;
        const RgbLedChannel& greenChannel () const noexcept;
        const RgbLedChannel& blueChannel  () const noexcept;

      private:
        RgbLedChannel redChannel_;
        RgbLedChannel greenChannel_;
        RgbLedChannel blueChannel_;
        PwmOutput     red_;
        PwmOutput     green_;
        PwmOutput     blue_;
        Rgb           color_;
    };
}
