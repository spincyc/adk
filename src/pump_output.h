#pragma once

#include "digital_output.h"

#include <stdint.h>

namespace adk {

    enum struct PumpState : uint8_t
    {
        Off,
        On
    };

    struct PumpOutput
    {
        virtual ~PumpOutput () noexcept;

        virtual Status    initialize  () noexcept              = 0;
        virtual void      shutdown    () noexcept                = 0;
        virtual Status    setState    (PumpState state) noexcept = 0;
        virtual PumpState state       () const noexcept             = 0;
        virtual bool      initialized () const noexcept       = 0;
    };

    struct IndicatorPump final : PumpOutput
    {
        IndicatorPump  (ResourceRegistry& resources, PinId indicatorPin) noexcept;
        ~IndicatorPump () noexcept override;

        IndicatorPump (const IndicatorPump&)            = delete;
        IndicatorPump& operator= (const IndicatorPump&) = delete;
        IndicatorPump (IndicatorPump&&)                 = delete;
        IndicatorPump& operator= (IndicatorPump&&)      = delete;

        Status    initialize  () noexcept override;
        void      shutdown    () noexcept override;
        Status    setState    (PumpState state) noexcept override;
        PumpState state       () const noexcept override;
        bool      initialized () const noexcept override;

      private:
        DigitalOutput output_;
        PumpState     state_;
    };
} // namespace adk
