#pragma once

#include "digital_output.h"

#include <stdint.h>

namespace adk {

    struct ShiftRegisterPins
    {
        PinId data;
        PinId clock;
        PinId latch;
    };

    struct ShiftRegisterOutput
    {
        ShiftRegisterOutput (ResourceRegistry&        resources,
                             const ShiftRegisterPins& pins,
                             uint8_t                  inactiveValue = 0) noexcept;
        ~ShiftRegisterOutput () noexcept;

        ShiftRegisterOutput& operator= (const ShiftRegisterOutput&) = delete;
        ShiftRegisterOutput  (const ShiftRegisterOutput&)           = delete;
        ShiftRegisterOutput& operator= (ShiftRegisterOutput&&)      = delete;
        ShiftRegisterOutput  (ShiftRegisterOutput&&)                = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status show  (uint8_t value) noexcept;
        Status clear () noexcept;

        uint8_t                  value         () const noexcept;
        uint8_t                  inactiveValue () const noexcept;
        const ShiftRegisterPins& pins          () const noexcept;
        bool                     initialized   () const noexcept;

      private:
        ShiftRegisterPins pins_;
        DigitalOutput     data_;
        DigitalOutput     clock_;
        DigitalOutput     latch_;
        uint8_t           value_;
        uint8_t           inactiveValue_;
    };
}
