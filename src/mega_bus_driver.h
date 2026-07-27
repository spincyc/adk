#pragma once

#include "i2c_bus.h"
#include "resource.h"
#include "spi_bus.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct MegaI2cIo
    {
        virtual ~MegaI2cIo () noexcept;

        // Configuration snapshots D20/D21; disable restores that snapshot.
        virtual Status configure () noexcept = 0;
        virtual void   disable   () noexcept = 0;
        virtual Status transfer  (uint8_t        address,
                                  const uint8_t* writeData,
                                  uint8_t        writeSize,
                                  uint8_t*       readData,
                                  uint8_t        readSize,
                                  Duration       timeout) noexcept = 0;
    };

    struct MegaI2cDriver final : I2cDriver
    {
        static constexpr PinId dataPin  = 20;
        static constexpr PinId clockPin = 21;

        MegaI2cDriver  (ResourceRegistry& resources, MegaI2cIo& io) noexcept;
        ~MegaI2cDriver () noexcept override;

        MegaI2cDriver            (const MegaI2cDriver&) = delete;
        MegaI2cDriver& operator= (const MegaI2cDriver&) = delete;
        MegaI2cDriver            (MegaI2cDriver&&)      = delete;
        MegaI2cDriver& operator= (MegaI2cDriver&&)      = delete;

        Status configure () noexcept override;
        void   disable   () noexcept override;
        Status transfer  (uint8_t        address,
                          const uint8_t* writeData,
                          uint8_t        writeSize,
                          uint8_t*       readData,
                          uint8_t        readSize,
                          Duration       timeout) noexcept override;

        bool configured () const noexcept;

      private:
        ResourceRegistry* resources_;
        MegaI2cIo*        io_;
        ResourceClaim     dataClaim_;
        ResourceClaim     clockClaim_;
        bool              configured_;
    };

    struct MegaSpiIo
    {
        virtual ~MegaSpiIo () noexcept;

        // Setup snapshots D50-D53 and drives D53 output-high for master mode.
        // Disable restores D50-D53 while registered device CS stays inactive.
        virtual Status configureMaster     () noexcept = 0;
        virtual void   disable             () noexcept = 0;
        virtual Status configureChipSelect (PinId chipSelect) noexcept = 0;
        virtual void   releaseChipSelect   (PinId chipSelect) noexcept = 0;
        virtual Status beginTransaction    (const SpiSettings& settings,
                                            PinId              chipSelect) noexcept = 0;
        virtual Status transfer            (const uint8_t* writeData,
                                            uint8_t*       readData,
                                            uint16_t       size,
                                            Duration       timeout) noexcept = 0;
        virtual Status endTransaction      (PinId chipSelect) noexcept = 0;
    };

    struct MegaSpiDriver final : SpiDriver
    {
        static constexpr PinId inputPin    = 50;
        static constexpr PinId outputPin   = 51;
        static constexpr PinId clockPin    = 52;
        static constexpr PinId masterSsPin = 53;

        MegaSpiDriver  (ResourceRegistry& resources, MegaSpiIo& io) noexcept;
        ~MegaSpiDriver () noexcept override;

        MegaSpiDriver            (const MegaSpiDriver&) = delete;
        MegaSpiDriver& operator= (const MegaSpiDriver&) = delete;
        MegaSpiDriver            (MegaSpiDriver&&)      = delete;
        MegaSpiDriver& operator= (MegaSpiDriver&&)      = delete;

        Status configure           () noexcept override;
        void   disable             () noexcept override;
        Status configureChipSelect (PinId chipSelect) noexcept override;
        void   releaseChipSelect   (PinId chipSelect) noexcept override;
        Status transfer            (const SpiSettings& settings,
                                    PinId              chipSelect,
                                    const uint8_t*     writeData,
                                    uint8_t*           readData,
                                    uint16_t           size,
                                    Duration           timeout) noexcept override;

        bool configured () const noexcept;

      private:
        // Mega uses exact 16 MHz divisors; requests are never rounded.
        bool settingsSupported (const SpiSettings& settings) const noexcept;
        void releaseClaims     () noexcept;

        ResourceRegistry* resources_;
        MegaSpiIo*        io_;
        ResourceClaim     inputClaim_;
        ResourceClaim     outputClaim_;
        ResourceClaim     clockClaim_;
        ResourceClaim     masterSsClaim_;
        bool              configured_;
    };
}
