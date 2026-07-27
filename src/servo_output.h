#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    struct PowerDomain
    {
        virtual ~PowerDomain () noexcept;

        virtual bool commandAdmitted () const noexcept = 0;
    };

    struct ExternalPowerDomainGate final : PowerDomain
    {
        ExternalPowerDomainGate () noexcept;

        void admit  () noexcept;
        void revoke () noexcept;

        bool commandAdmitted () const noexcept override;

      private:
        bool admitted_;
    };

    struct ServoPulseIo
    {
        virtual ~ServoPulseIo () noexcept;

        virtual Status attach     (PinId pin, uint8_t timer) noexcept       = 0;
        virtual Status writePulse (PinId pin, uint16_t pulseUs) noexcept   = 0;
        virtual void   detach     (PinId pin) noexcept                     = 0;
    };

    struct MegaTimer5Registers
    {
        virtual ~MegaTimer5Registers () noexcept;

        virtual Status writeControlA      (uint8_t value) noexcept = 0;
        virtual Status writeControlB      (uint8_t value) noexcept = 0;
        virtual Status writeControlC      (uint8_t value) noexcept = 0;
        virtual Status writeInterruptMask (uint8_t value) noexcept = 0;
        virtual Status writeCounter       (uint16_t value) noexcept = 0;
        virtual Status writeTop           (uint16_t value) noexcept = 0;
        virtual Status writeCompareC      (uint16_t value) noexcept = 0;
        virtual Status writeOutputLow     () noexcept               = 0;
        virtual Status setOutputEnabled   (bool enabled) noexcept   = 0;
    };

    struct MegaTimer5ServoPulseIo final : ServoPulseIo
    {
        static constexpr PinId   signalPin = 44;
        static constexpr uint8_t timer     = 5;

        MegaTimer5ServoPulseIo          () noexcept;
        explicit MegaTimer5ServoPulseIo (MegaTimer5Registers& registers) noexcept;
        ~MegaTimer5ServoPulseIo         () noexcept override;

        Status attach     (PinId pin, uint8_t timerIndex) noexcept override;
        Status writePulse (PinId pin, uint16_t pulseUs) noexcept override;
        void   detach     (PinId pin) noexcept override;

      private:
        Status configure () noexcept;
        void   disable   () noexcept;

        MegaTimer5Registers* registers_;
        bool                 attached_;
    };

    struct ServoOutput
    {
        static constexpr uint16_t absoluteMinimumPulseUs = 544;
        static constexpr uint16_t absoluteMaximumPulseUs = 2400;
        static constexpr uint8_t  defaultTimer           = 5;

        ServoOutput  (ResourceRegistry& resources,
                      ServoPulseIo&     io,
                      const PowerDomain& power,
                      PinId             pin,
                      uint16_t          minimumPulseUs,
                      uint16_t          maximumPulseUs,
                      uint8_t           timer = defaultTimer) noexcept;
        ~ServoOutput () noexcept;

        ServoOutput            (const ServoOutput&) = delete;
        ServoOutput& operator= (const ServoOutput&) = delete;
        ServoOutput            (ServoOutput&&)      = delete;
        ServoOutput& operator= (ServoOutput&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status writePulse (uint16_t pulseUs) noexcept;

        PinId    pin            () const noexcept;
        uint8_t  timer          () const noexcept;
        uint16_t minimumPulseUs () const noexcept;
        uint16_t maximumPulseUs () const noexcept;
        uint16_t pulseUs        () const noexcept;
        bool     initialized    () const noexcept;

      private:
        ResourceRegistry* resources_;
        ServoPulseIo*     io_;
        const PowerDomain* power_;
        ResourceClaim     pinClaim_;
        ResourceClaim     timerClaim_;
        PinId             pin_;
        uint8_t           timer_;
        uint16_t          minimumPulseUs_;
        uint16_t          maximumPulseUs_;
        uint16_t          pulseUs_;
        bool              initialized_;
    };
}
