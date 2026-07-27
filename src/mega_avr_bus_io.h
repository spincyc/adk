#pragma once

#include "mega_bus_driver.h"

#include <stdint.h>

namespace adk {

    enum struct MegaAvrRegister : uint8_t
    {
        Twbr,
        Twsr,
        Twar,
        Twcr,
        Twdr,
        Spcr,
        Spsr,
        Spdr,
        Ddrb,
        Portb,
        Ddrd,
        Portd,
        Ddrl,
        Portl
    };

    struct MegaAvrBusRegisters
    {
        virtual ~MegaAvrBusRegisters () noexcept;

        virtual uint8_t  read            (MegaAvrRegister reg) noexcept                 = 0;
        virtual void     write           (MegaAvrRegister reg, uint8_t value) noexcept = 0;
        virtual uint32_t microsecondsNow () noexcept                                    = 0;
    };

    struct MegaAvrI2cIo final : MegaI2cIo
    {
        MegaAvrI2cIo          () noexcept;
        explicit MegaAvrI2cIo (MegaAvrBusRegisters& registers) noexcept;
        ~MegaAvrI2cIo         () noexcept override;

        MegaAvrI2cIo& operator= (const MegaAvrI2cIo&) = delete;
        MegaAvrI2cIo (const MegaAvrI2cIo&)            = delete;
        MegaAvrI2cIo& operator= (MegaAvrI2cIo&&)      = delete;
        MegaAvrI2cIo (MegaAvrI2cIo&&)                 = delete;

        Status configure () noexcept override;
        void   disable   () noexcept override;
        Status transfer  (uint8_t address, const uint8_t* writeData, uint8_t writeSize,
                          uint8_t* readData, uint8_t readSize,
                          Duration timeout) noexcept override;

      private:
        Status start        (uint32_t startedAt, uint32_t timeoutUs) noexcept;
        Status send         (uint8_t value, uint8_t expectedStatus, uint32_t startedAt,
                             uint32_t timeoutUs) noexcept;
        Status receive      (uint8_t& value, bool acknowledge, uint32_t startedAt,
                             uint32_t timeoutUs) noexcept;
        Status waitForTwint (uint32_t startedAt, uint32_t timeoutUs) noexcept;
        Status stop         (uint32_t startedAt, uint32_t timeoutUs) noexcept;

        MegaAvrBusRegisters* registers_;
        uint8_t              snapshot_[6];
        bool                 configured_;
    };

    struct MegaAvrSpiIo final : MegaSpiIo
    {
        static constexpr PinId chipSelectPin = 49;

        MegaAvrSpiIo          () noexcept;
        explicit MegaAvrSpiIo (MegaAvrBusRegisters& registers) noexcept;
        ~MegaAvrSpiIo         () noexcept override;

        MegaAvrSpiIo& operator= (const MegaAvrSpiIo&) = delete;
        MegaAvrSpiIo (const MegaAvrSpiIo&)            = delete;
        MegaAvrSpiIo& operator= (MegaAvrSpiIo&&)      = delete;
        MegaAvrSpiIo (MegaAvrSpiIo&&)                 = delete;

        Status configureMaster     () noexcept override;
        void   disable             () noexcept override;
        Status configureChipSelect (PinId chipSelect) noexcept override;
        void   releaseChipSelect   (PinId chipSelect) noexcept override;
        Status beginTransaction    (const SpiSettings& settings,
                                    PinId              chipSelect) noexcept override;
        Status transfer            (const uint8_t* writeData, uint8_t* readData,
                                    uint16_t size, Duration timeout) noexcept override;
        Status endTransaction      (PinId chipSelect) noexcept override;

      private:
        Status applySettings (const SpiSettings& settings) noexcept;
        Status waitForSpif   (uint32_t startedAt, uint32_t timeoutUs) noexcept;
        void   setChipSelect (bool inactive) noexcept;

        MegaAvrBusRegisters* registers_;
        uint8_t              busSnapshot_[4];
        uint8_t              chipSelectSnapshot_[2];
        uint8_t              transactionSnapshot_[2];
        bool                 configured_;
        bool                 chipSelectConfigured_;
        bool                 transactionActive_;
    };
} // namespace adk
