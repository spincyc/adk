#include "servo_output.h"

#include "board.h"

#if defined(__AVR_ATmega2560__)
#include <avr/io.h>
#endif

namespace adk {

    PowerDomain::~PowerDomain () noexcept = default;

    ExternalPowerDomainGate::ExternalPowerDomainGate () noexcept
        : admitted_ (false)
    {
    }

    void ExternalPowerDomainGate::admit () noexcept
    {
        admitted_ = true;
    }

    void ExternalPowerDomainGate::revoke () noexcept
    {
        admitted_ = false;
    }

    bool ExternalPowerDomainGate::commandAdmitted () const noexcept
    {
        return admitted_;
    }

    ServoPulseIo::~ServoPulseIo () noexcept = default;

    MegaTimer5Registers::~MegaTimer5Registers () noexcept = default;

    namespace {

        struct DirectMegaTimer5Registers final : MegaTimer5Registers
        {
            Status writeControlA (uint8_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                TCCR5A = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeControlB (uint8_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                TCCR5B = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeControlC (uint8_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                TCCR5C = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeInterruptMask (uint8_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                TIMSK5 = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeCounter (uint16_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                TCNT5 = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeTop (uint16_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                ICR5 = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeCompareC (uint16_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                OCR5C = value;
                return StatusCode::Ok;
#else
                (void)value;
                return StatusCode::Unsupported;
#endif
            }

            Status writeOutputLow () noexcept override
            {
#if defined(__AVR_ATmega2560__)
                PORTL &= static_cast<uint8_t> (~_BV (PL5));
                return StatusCode::Ok;
#else
                return StatusCode::Unsupported;
#endif
            }

            Status setOutputEnabled (bool enabled) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                if (enabled)
                {
                    DDRL |= _BV (DDL5);
                }
                else
                {
                    DDRL &= static_cast<uint8_t> (~_BV (DDL5));
                }
                return StatusCode::Ok;
#else
                (void)enabled;
                return StatusCode::Unsupported;
#endif
            }
        };

        DirectMegaTimer5Registers directRegisters;

        constexpr uint8_t timerModeA       = 1U << 1;
        constexpr uint8_t timerModeB       = (1U << 4) | (1U << 3);
        constexpr uint8_t timerClockDiv8   = 1U << 1;
        constexpr uint8_t compareCEnabled  = 1U << 3;
        constexpr uint16_t timerPeriodTicks = 39999;
    }

    MegaTimer5ServoPulseIo::MegaTimer5ServoPulseIo () noexcept
        : registers_ (&directRegisters)
        , attached_  (false)
    {
    }

    MegaTimer5ServoPulseIo::MegaTimer5ServoPulseIo (
        MegaTimer5Registers& registers) noexcept
        : registers_ (&registers)
        , attached_  (false)
    {
    }

    MegaTimer5ServoPulseIo::~MegaTimer5ServoPulseIo () noexcept
    {
        detach (signalPin);
    }

    Status MegaTimer5ServoPulseIo::attach (
        PinId  pin,
        uint8_t timerIndex) noexcept
    {
        if (pin != signalPin || timerIndex != timer)
        {
            return StatusCode::Unsupported;
        }

        if (attached_)
        {
            return StatusCode::Ok;
        }

        const Status status = configure ();

        if (!status.ok ())
        {
            disable ();
            return status;
        }

        attached_ = true;
        return StatusCode::Ok;
    }

    Status MegaTimer5ServoPulseIo::writePulse (
        PinId   pin,
        uint16_t pulseUs) noexcept
    {
        if (!attached_ || pin != signalPin)
        {
            return StatusCode::NotInitialized;
        }

        const Status compareStatus =
            registers_->writeCompareC (static_cast<uint16_t> (pulseUs * 2U));

        if (!compareStatus.ok ())
        {
            return compareStatus;
        }

        const Status connectStatus =
            registers_->writeControlA (timerModeA | compareCEnabled);

        if (!connectStatus.ok ())
        {
            registers_->writeCompareC (0);
            return connectStatus;
        }

        return StatusCode::Ok;
    }

    void MegaTimer5ServoPulseIo::detach (PinId pin) noexcept
    {
        if (pin != signalPin || !attached_)
        {
            return;
        }

        disable ();
        attached_ = false;
    }

    Status MegaTimer5ServoPulseIo::configure () noexcept
    {
        disable ();

        Status status = registers_->writeOutputLow ();

        if (!status.ok ())
        {
            return status;
        }

        status = registers_->setOutputEnabled (true);

        if (!status.ok ())
        {
            return status;
        }

        status = registers_->writeTop (timerPeriodTicks);

        if (!status.ok ())
        {
            return status;
        }

        status = registers_->writeControlA (timerModeA);

        if (!status.ok ())
        {
            return status;
        }

        return registers_->writeControlB (timerModeB | timerClockDiv8);
    }

    void MegaTimer5ServoPulseIo::disable () noexcept
    {
        registers_->writeControlA      (0);
        registers_->writeControlB      (0);
        registers_->writeControlC      (0);
        registers_->writeInterruptMask (0);
        registers_->writeCompareC      (0);
        registers_->writeTop           (0);
        registers_->writeCounter       (0);
        registers_->writeOutputLow     ();
        registers_->setOutputEnabled   (false);
    }

    ServoOutput::ServoOutput (ResourceRegistry& resources,
                              ServoPulseIo&     io,
                              const PowerDomain& power,
                              PinId             pin,
                              uint16_t          minimumPulseUs,
                              uint16_t          maximumPulseUs,
                              uint8_t           timer) noexcept
        : resources_      (&resources)
        , io_             (&io)
        , power_          (&power)
        , pinClaim_       ()
        , timerClaim_     ()
        , pin_            (pin)
        , timer_          (timer)
        , minimumPulseUs_ (minimumPulseUs)
        , maximumPulseUs_ (maximumPulseUs)
        , pulseUs_        (0)
        , initialized_    (false)
    {
    }

    ServoOutput::~ServoOutput () noexcept
    {
        shutdown ();
    }

    Status ServoOutput::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (pin_))
        {
            return StatusCode::InvalidPin;
        }

        if (!Mega2560Board::supports (pin_, PinCapability::DigitalOutput) ||
            timer_ >= 6)
        {
            return StatusCode::Unsupported;
        }

        if (minimumPulseUs_ < absoluteMinimumPulseUs ||
            maximumPulseUs_ > absoluteMaximumPulseUs ||
            minimumPulseUs_ > maximumPulseUs_)
        {
            return StatusCode::InvalidArgument;
        }

        Status status =
            resources_->claim ({ResourceKind::Pin, pin_}, pinClaim_);

        if (!status.ok ())
        {
            return status;
        }

        status = resources_->claim ({ResourceKind::Timer, timer_}, timerClaim_);

        if (!status.ok ())
        {
            pinClaim_.release ();
            return status;
        }

        status = io_->attach (pin_, timer_);

        if (!status.ok ())
        {
            io_->detach         (pin_);
            timerClaim_.release ();
            pinClaim_  .release ();
            return status;
        }

        pulseUs_     = 0;
        initialized_ = true;
        return StatusCode::Ok;
    }

    void ServoOutput::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        io_->detach         (pin_);
        timerClaim_.release ();
        pinClaim_  .release ();

        pulseUs_     = 0;
        initialized_ = false;
    }

    Status ServoOutput::writePulse (uint16_t pulseUs) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (pulseUs < minimumPulseUs_ || pulseUs > maximumPulseUs_)
        {
            return StatusCode::InvalidArgument;
        }

        if (!power_->commandAdmitted ())
        {
            return StatusCode::HardwareFailure;
        }

        const Status status = io_->writePulse (pin_, pulseUs);

        if (status.ok ())
        {
            pulseUs_ = pulseUs;
        }

        return status;
    }

    PinId ServoOutput::pin () const noexcept
    {
        return pin_;
    }

    uint8_t ServoOutput::timer () const noexcept
    {
        return timer_;
    }

    uint16_t ServoOutput::minimumPulseUs () const noexcept
    {
        return minimumPulseUs_;
    }

    uint16_t ServoOutput::maximumPulseUs () const noexcept
    {
        return maximumPulseUs_;
    }

    uint16_t ServoOutput::pulseUs () const noexcept
    {
        return pulseUs_;
    }

    bool ServoOutput::initialized () const noexcept
    {
        return initialized_;
    }
}
