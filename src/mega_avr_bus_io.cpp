#include "mega_avr_bus_io.h"

#if defined(__AVR_ATmega2560__)
#include <Arduino.h>
#include <avr/io.h>
#endif

namespace adk {

    MegaAvrBusRegisters::~MegaAvrBusRegisters () noexcept = default;

    namespace {

        constexpr uint8_t twint = 1U << 7;
        constexpr uint8_t twea  = 1U << 6;
        constexpr uint8_t twsta = 1U << 5;
        constexpr uint8_t twsto = 1U << 4;
        constexpr uint8_t twen  = 1U << 2;
        constexpr uint8_t spif  = 1U << 7;
        constexpr uint8_t spe   = 1U << 6;
        constexpr uint8_t dord  = 1U << 5;
        constexpr uint8_t mstr  = 1U << 4;
        constexpr uint8_t cpol  = 1U << 3;
        constexpr uint8_t cpha  = 1U << 2;
        constexpr uint8_t spi2x = 1U << 0;

        struct DirectMegaAvrBusRegisters final : MegaAvrBusRegisters
        {
            uint8_t read (MegaAvrRegister reg) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                switch (reg)
                {
                    case MegaAvrRegister::Twbr: return TWBR;
                    case MegaAvrRegister::Twsr: return TWSR;
                    case MegaAvrRegister::Twar: return TWAR;
                    case MegaAvrRegister::Twcr: return TWCR;
                    case MegaAvrRegister::Twdr: return TWDR;
                    case MegaAvrRegister::Spcr: return SPCR;
                    case MegaAvrRegister::Spsr: return SPSR;
                    case MegaAvrRegister::Spdr: return SPDR;
                    case MegaAvrRegister::Ddrb: return DDRB;
                    case MegaAvrRegister::Portb: return PORTB;
                    case MegaAvrRegister::Ddrd: return DDRD;
                    case MegaAvrRegister::Portd: return PORTD;
                    case MegaAvrRegister::Ddrl: return DDRL;
                    case MegaAvrRegister::Portl: return PORTL;
                }
#else
                (void)reg;
#endif
                return 0;
            }

            void write (MegaAvrRegister reg, uint8_t value) noexcept override
            {
#if defined(__AVR_ATmega2560__)
                switch (reg)
                {
                    case MegaAvrRegister::Twbr: TWBR = value; break;
                    case MegaAvrRegister::Twsr: TWSR = value; break;
                    case MegaAvrRegister::Twar: TWAR = value; break;
                    case MegaAvrRegister::Twcr: TWCR = value; break;
                    case MegaAvrRegister::Twdr: TWDR = value; break;
                    case MegaAvrRegister::Spcr: SPCR = value; break;
                    case MegaAvrRegister::Spsr: SPSR = value; break;
                    case MegaAvrRegister::Spdr: SPDR = value; break;
                    case MegaAvrRegister::Ddrb: DDRB = value; break;
                    case MegaAvrRegister::Portb: PORTB = value; break;
                    case MegaAvrRegister::Ddrd: DDRD = value; break;
                    case MegaAvrRegister::Portd: PORTD = value; break;
                    case MegaAvrRegister::Ddrl: DDRL = value; break;
                    case MegaAvrRegister::Portl: PORTL = value; break;
                }
#else
                (void)reg;
                (void)value;
#endif
            }

            uint32_t microsecondsNow () noexcept override
            {
#if defined(__AVR_ATmega2560__)
                return micros ();
#else
                return 0;
#endif
            }
        };

        DirectMegaAvrBusRegisters directRegisters;

        uint32_t timeoutMicroseconds (Duration timeout) noexcept
        {
            const uint32_t milliseconds = timeout.milliseconds ();

            return milliseconds * 1000U;
        }

        bool expired (MegaAvrBusRegisters& registers, uint32_t startedAt,
                      uint32_t timeoutUs) noexcept
        {
            return static_cast<uint32_t> (registers.microsecondsNow () - startedAt) >=
                   timeoutUs;
        }
    } // namespace

    MegaAvrI2cIo::MegaAvrI2cIo () noexcept : MegaAvrI2cIo (directRegisters)
    {
    }

    MegaAvrI2cIo::MegaAvrI2cIo (MegaAvrBusRegisters& registers) noexcept
        : registers_ (&registers), snapshot_{}, configured_ (false)
    {
    }

    MegaAvrI2cIo::~MegaAvrI2cIo () noexcept
    {
        disable ();
    }

    Status MegaAvrI2cIo::configure () noexcept
    {
        if (configured_)
        {
            return StatusCode::Ok;
        }

#if !defined(__AVR_ATmega2560__)
        if (registers_ == &directRegisters)
        {
            return StatusCode::Unsupported;
        }
#endif

        snapshot_[0] = registers_->read (MegaAvrRegister::Twbr);
        snapshot_[1] = registers_->read (MegaAvrRegister::Twsr);
        snapshot_[2] = registers_->read (MegaAvrRegister::Twar);
        snapshot_[3] = registers_->read (MegaAvrRegister::Ddrd);
        snapshot_[4] = registers_->read (MegaAvrRegister::Portd);
        snapshot_[5] = registers_->read (MegaAvrRegister::Twcr);

        registers_->write (MegaAvrRegister::Twcr, 0);
        registers_->write (MegaAvrRegister::Ddrd,
                           static_cast<uint8_t> (snapshot_[3] & ~0x03U));
        registers_->write (MegaAvrRegister::Portd,
                           static_cast<uint8_t> (snapshot_[4] & ~0x03U));
        registers_->write (MegaAvrRegister::Twsr,
                           static_cast<uint8_t> (snapshot_[1] & ~0x03U));
        registers_->write (MegaAvrRegister::Twbr, 72);
        registers_->write (MegaAvrRegister::Twar, 0);
        registers_->write (MegaAvrRegister::Twcr, twen);

        configured_ = true;
        return StatusCode::Ok;
    }

    void MegaAvrI2cIo::disable () noexcept
    {
        if (!configured_)
        {
            return;
        }

        registers_->write (MegaAvrRegister::Twbr, snapshot_[0]);
        registers_->write (MegaAvrRegister::Twsr, snapshot_[1]);
        registers_->write (MegaAvrRegister::Twar, snapshot_[2]);
        registers_->write (MegaAvrRegister::Ddrd, snapshot_[3]);
        registers_->write (MegaAvrRegister::Portd, snapshot_[4]);
        registers_->write (MegaAvrRegister::Twcr, snapshot_[5]);
        configured_ = false;
    }

    Status MegaAvrI2cIo::transfer (uint8_t address, const uint8_t* writeData,
                                   uint8_t writeSize, uint8_t* readData,
                                   uint8_t readSize, Duration timeout) noexcept
    {
        if (!configured_)
        {
            return StatusCode::NotInitialized;
        }

        if (timeout.milliseconds () > UINT32_MAX / 1000U)
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t timeoutUs = timeoutMicroseconds         (timeout);
        const uint32_t startedAt = registers_->microsecondsNow ();
        Status         status    = start                       (startedAt, timeoutUs);

        if (status.ok () && writeSize != 0)
        {
            status = send (static_cast<uint8_t> (address << 1U), 0x18U, startedAt,
                           timeoutUs);
        }

        for (uint8_t index = 0; status.ok () && index < writeSize; ++index)
        {
            status = send (writeData[index], 0x28U, startedAt, timeoutUs);
        }

        if (status.ok () && readSize != 0)
        {
            if (writeSize != 0)
            {
                status = start (startedAt, timeoutUs);
            }

            if (status.ok ())
            {
                status = send (static_cast<uint8_t> ((address << 1U) | 1U), 0x40U,
                               startedAt, timeoutUs);
            }
        }

        for (uint8_t index = 0; status.ok () && index < readSize; ++index)
        {
            status =
                receive (readData[index], index + 1U < readSize, startedAt, timeoutUs);
        }

        const Status stopStatus = stop (startedAt, timeoutUs);

        if (status.ok () && !stopStatus.ok ())
        {
            return stopStatus;
        }

        return status;
    }

    Status MegaAvrI2cIo::start (uint32_t startedAt, uint32_t timeoutUs) noexcept
    {
        registers_->write     (MegaAvrRegister::Twcr, twint | twsta | twen);

        const Status status = waitForTwint (startedAt, timeoutUs);

        if (!status.ok ())
        {
            return status;
        }

        const uint8_t state =
            static_cast<uint8_t> (registers_->read (MegaAvrRegister::Twsr) & 0xf8U);
        return state == 0x08U || state == 0x10U ? Status (StatusCode::Ok)
                                                : Status (StatusCode::HardwareFailure);
    }

    Status MegaAvrI2cIo::send (uint8_t value, uint8_t expectedStatus,
                               uint32_t startedAt, uint32_t timeoutUs) noexcept
    {
        registers_->write     (MegaAvrRegister::Twdr, value);
        registers_->write     (MegaAvrRegister::Twcr, twint | twen);

        const Status status = waitForTwint (startedAt, timeoutUs);

        if (!status.ok ())
        {
            return status;
        }

        return (registers_->read (MegaAvrRegister::Twsr) & 0xf8U) == expectedStatus
                   ? Status (StatusCode::Ok)
                   : Status (StatusCode::HardwareFailure);
    }

    Status MegaAvrI2cIo::receive (uint8_t& value, bool acknowledge, uint32_t startedAt,
                                  uint32_t timeoutUs) noexcept
    {
        registers_->write (
            MegaAvrRegister::Twcr,
            static_cast<uint8_t> (twint | twen | (acknowledge ? twea : 0U)));
        const Status status = waitForTwint (startedAt, timeoutUs);

        if (!status.ok ())
        {
            return status;
        }

        const uint8_t expected = acknowledge ? 0x50U : 0x58U;

        if ((registers_->read (MegaAvrRegister::Twsr) & 0xf8U) != expected)
        {
            return StatusCode::HardwareFailure;
        }

        value = registers_->read (MegaAvrRegister::Twdr);
        return StatusCode::Ok;
    }

    Status MegaAvrI2cIo::waitForTwint (uint32_t startedAt, uint32_t timeoutUs) noexcept
    {
        while ((registers_->read (MegaAvrRegister::Twcr) & twint) == 0)
        {
            if (expired (*registers_, startedAt, timeoutUs))
            {
                return StatusCode::HardwareFailure;
            }
        }

        return StatusCode::Ok;
    }

    Status MegaAvrI2cIo::stop (uint32_t startedAt, uint32_t timeoutUs) noexcept
    {
        registers_->write (MegaAvrRegister::Twcr, twint | twsto | twen);

        while ((registers_->read (MegaAvrRegister::Twcr) & twsto) != 0)
        {
            if (expired (*registers_, startedAt, timeoutUs))
            {
                registers_->write (MegaAvrRegister::Twcr, 0);
                return StatusCode::HardwareFailure;
            }
        }

        return StatusCode::Ok;
    }

    MegaAvrSpiIo::MegaAvrSpiIo () noexcept : MegaAvrSpiIo (directRegisters)
    {
    }

    MegaAvrSpiIo::MegaAvrSpiIo (MegaAvrBusRegisters& registers) noexcept
        : registers_            (&registers)
        , busSnapshot_          {}
        , chipSelectSnapshot_   {}
        , transactionSnapshot_  {}
        , configured_           (false)
        , chipSelectConfigured_ (false)
        , transactionActive_    (false)
    {
    }

    MegaAvrSpiIo::~MegaAvrSpiIo () noexcept
    {
        disable ();
    }

    Status MegaAvrSpiIo::configureMaster () noexcept
    {
        if (configured_)
        {
            return StatusCode::Ok;
        }

#if !defined(__AVR_ATmega2560__)
        if (registers_ == &directRegisters)
        {
            return StatusCode::Unsupported;
        }
#endif

        busSnapshot_[0] = registers_->read (MegaAvrRegister::Spcr);
        busSnapshot_[1] = registers_->read (MegaAvrRegister::Spsr);
        busSnapshot_[2] = registers_->read (MegaAvrRegister::Ddrb);
        busSnapshot_[3] = registers_->read (MegaAvrRegister::Portb);

        registers_->write (MegaAvrRegister::Portb,
                           static_cast<uint8_t> (busSnapshot_[3] | 0x01U));
        registers_->write (MegaAvrRegister::Ddrb,
                           static_cast<uint8_t> ((busSnapshot_[2] | 0x07U) & ~0x08U));
        registers_->write (MegaAvrRegister::Spcr, spe | mstr);
        registers_->write (MegaAvrRegister::Spsr, 0);

        configured_ = true;
        return StatusCode::Ok;
    }

    void MegaAvrSpiIo::disable () noexcept
    {
        if (!configured_)
        {
            return;
        }

        if (chipSelectConfigured_)
        {
            setChipSelect (true);
        }

        registers_->write (MegaAvrRegister::Spcr, 0);
        registers_->write (MegaAvrRegister::Spsr, busSnapshot_[1]);
        registers_->write (MegaAvrRegister::Ddrb, busSnapshot_[2]);
        registers_->write (MegaAvrRegister::Portb, busSnapshot_[3]);
        registers_->write (MegaAvrRegister::Spcr, busSnapshot_[0]);
        transactionActive_ = false;
        configured_        = false;
    }

    Status MegaAvrSpiIo::configureChipSelect (PinId chipSelect) noexcept
    {
        if (!configured_)
        {
            return StatusCode::NotInitialized;
        }

        if (chipSelect != chipSelectPin || chipSelectConfigured_)
        {
            return chipSelectConfigured_ ? StatusCode::ResourceBusy
                                         : StatusCode::Unsupported;
        }

        chipSelectSnapshot_[0] = registers_->read (MegaAvrRegister::Ddrl);
        chipSelectSnapshot_[1] = registers_->read (MegaAvrRegister::Portl);

        setChipSelect     (true);
        registers_->write (
            MegaAvrRegister::Ddrl,
            static_cast<uint8_t> (registers_->read (MegaAvrRegister::Ddrl) | 0x01U));
        chipSelectConfigured_ = true;
        return StatusCode::Ok;
    }

    void MegaAvrSpiIo::releaseChipSelect (PinId chipSelect) noexcept
    {
        if (chipSelect != chipSelectPin || !chipSelectConfigured_)
        {
            return;
        }

        setChipSelect     (true);
        registers_->write (MegaAvrRegister::Ddrl, chipSelectSnapshot_[0]);
        registers_->write (MegaAvrRegister::Portl, chipSelectSnapshot_[1]);
        chipSelectConfigured_ = false;
    }

    Status MegaAvrSpiIo::beginTransaction (const SpiSettings& settings,
                                           PinId              chipSelect) noexcept
    {
        if (!configured_ || !chipSelectConfigured_ || chipSelect != chipSelectPin ||
            transactionActive_)
        {
            return StatusCode::NotInitialized;
        }

        transactionSnapshot_[0] = registers_->read (MegaAvrRegister::Spcr);
        transactionSnapshot_[1] = registers_->read (MegaAvrRegister::Spsr);

        const Status status = applySettings (settings);

        if (!status.ok ())
        {
            return status;
        }

        setChipSelect (false);
        transactionActive_ = true;
        return StatusCode::Ok;
    }

    Status MegaAvrSpiIo::transfer (const uint8_t* writeData, uint8_t* readData,
                                   uint16_t size, Duration timeout) noexcept
    {
        if (!transactionActive_)
        {
            return StatusCode::NotInitialized;
        }

        if (timeout.milliseconds () > UINT32_MAX / 1000U)
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t timeoutUs = timeoutMicroseconds         (timeout);
        const uint32_t startedAt = registers_->microsecondsNow ();

        for (uint16_t index = 0; index < size; ++index)
        {
            registers_->write (MegaAvrRegister::Spdr,
                               writeData == nullptr ? 0xffU : writeData[index]);
            const Status status = waitForSpif (startedAt, timeoutUs);

            if (!status.ok ())
            {
                return status;
            }

            const uint8_t received = registers_->read (MegaAvrRegister::Spdr);

            if (readData != nullptr)
            {
                readData[index] = received;
            }
        }

        return StatusCode::Ok;
    }

    Status MegaAvrSpiIo::endTransaction (PinId chipSelect) noexcept
    {
        if (chipSelect != chipSelectPin || !chipSelectConfigured_)
        {
            return StatusCode::InvalidArgument;
        }

        setChipSelect     (true);
        registers_->write (MegaAvrRegister::Spcr, transactionSnapshot_[0]);
        registers_->write (MegaAvrRegister::Spsr, transactionSnapshot_[1]);
        transactionActive_ = false;
        return StatusCode::Ok;
    }

    Status MegaAvrSpiIo::applySettings (const SpiSettings& settings) noexcept
    {
        uint8_t dividerBits = 0;
        uint8_t doubleSpeed = 0;

        switch (settings.clockHz)
        {
            case 8000000UL: doubleSpeed = spi2x; break;
            case 4000000UL: break;
            case 2000000UL:
                dividerBits = 1U;
                doubleSpeed = spi2x;
                break;
            case 1000000UL: dividerBits = 1U; break;
            case 500000UL:
                dividerBits = 2U;
                doubleSpeed = spi2x;
                break;
            case 250000UL: dividerBits = 2U; break;
            case 125000UL: dividerBits = 3U; break;
            default: return StatusCode::Unsupported;
        }

        uint8_t control = static_cast<uint8_t> (spe | mstr | dividerBits);

        if (!settings.mostSignificantBitFirst)
        {
            control |= dord;
        }

        if ((settings.mode & 0x02U) != 0)
        {
            control |= cpol;
        }

        if ((settings.mode & 0x01U) != 0)
        {
            control |= cpha;
        }

        registers_->write (MegaAvrRegister::Spcr, control);
        registers_->write (MegaAvrRegister::Spsr, doubleSpeed);
        return StatusCode::Ok;
    }

    Status MegaAvrSpiIo::waitForSpif (uint32_t startedAt, uint32_t timeoutUs) noexcept
    {
        while ((registers_->read (MegaAvrRegister::Spsr) & spif) == 0)
        {
            if (expired (*registers_, startedAt, timeoutUs))
            {
                return StatusCode::HardwareFailure;
            }
        }

        return StatusCode::Ok;
    }

    void MegaAvrSpiIo::setChipSelect (bool inactive) noexcept
    {
        uint8_t port = registers_->read (MegaAvrRegister::Portl);

        if (inactive)
        {
            port |= 0x01U;
        }
        else
        {
            port &= static_cast<uint8_t> (~0x01U);
        }

        registers_->write (MegaAvrRegister::Portl, port);
    }
} // namespace adk
