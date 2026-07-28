#pragma once

#include "digital_input.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct Rotation : int8_t
    {
        CounterClockwise = -1,
        None             =  0,
        Clockwise        =  1
    };

    struct QuadratureEncoderConfig
    {
        QuadratureEncoderConfig (PinId phaseA,
                                 PinId phaseB,
                                 Pull  pull     = Pull::Up,
                                 bool  reversed = false) noexcept;

        PinId phaseA;
        PinId phaseB;
        Pull  pull;
        bool  reversed;
    };

    struct QuadratureEncoderSnapshot
    {
        int32_t  position;
        int8_t   delta;
        Rotation rotation;
        uint8_t  phaseMask;
        uint16_t invalidTransitions;
        bool     positionSaturated;
        Status   status;
    };

    struct QuadratureEncoder
    {
        QuadratureEncoder (ResourceRegistry&              resources,
                           const QuadratureEncoderConfig& config) noexcept;
        ~QuadratureEncoder () noexcept;

        QuadratureEncoder            (const QuadratureEncoder&) = delete;
        QuadratureEncoder& operator= (const QuadratureEncoder&) = delete;
        QuadratureEncoder            (QuadratureEncoder&&)      = delete;
        QuadratureEncoder& operator= (QuadratureEncoder&&)      = delete;

        Status initialize    () noexcept;
        void   shutdown      () noexcept;
        Status update        () noexcept;
        void   resetPosition (int32_t position = 0) noexcept;

        bool                      initialized () const noexcept;
        QuadratureEncoderSnapshot snapshot    () const noexcept;

        const DigitalInput& phaseAInput () const noexcept;
        const DigitalInput& phaseBInput () const noexcept;

      private:
        bool   validPull     () const noexcept;
        uint8_t currentPhase () const noexcept;
        int8_t transition    (uint8_t previous, uint8_t current) const noexcept;
        void   applyDelta    (int8_t delta) noexcept;

        QuadratureEncoderConfig   config_;
        DigitalInput              phaseA_;
        DigitalInput              phaseB_;
        QuadratureEncoderSnapshot snapshot_;
    };
}
