#pragma once

#include "servo_calibration.h"

#include <stddef.h>
#include <stdint.h>

namespace adk {

    struct ServoConfiguration
    {
        BoundedServoConfig servo;
        uint32_t           generation;
    };

    struct ServoConfigurationRecord
    {
        static const uint8_t FormatVersion = 1;
        static const size_t  Capacity      = 24;
        static const size_t  EncodedSize   = 20;

        ServoConfigurationRecord () noexcept;

        void                       clear    () noexcept;
        bool                       empty    () const noexcept;
        Status                     save     (const ServoConfiguration& config) noexcept;
        Result<ServoConfiguration> load     () const noexcept;
        Status                     import   (const uint8_t* bytes, size_t size) noexcept;
        Status                     exportTo (uint8_t* bytes, size_t capacity) const noexcept;

      private:
        uint8_t bytes_[Capacity];
        bool    empty_;
    };
}
