#pragma once

#include "digital.h"
#include "resource.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct SpiSettings
    {
        uint32_t clockHz;
        uint8_t  mode;
        bool     mostSignificantBitFirst;
    };

    struct SpiDriver
    {
        virtual ~SpiDriver () noexcept;

        virtual Status configure           () noexcept = 0;
        virtual void   disable             () noexcept = 0;
        virtual Status configureChipSelect (PinId chipSelect) noexcept = 0;
        virtual void   releaseChipSelect   (PinId chipSelect) noexcept = 0;
        virtual Status transfer            (const SpiSettings& settings,
                                            PinId             chipSelect,
                                            const uint8_t*    writeData,
                                            uint8_t*          readData,
                                            uint16_t          size,
                                            Duration          timeout) noexcept = 0;
    };

    struct SpiDevice;

    struct SpiBus
    {
        static const uint16_t maximumTransferSize = 256;

        SpiBus  (ResourceRegistry& resources,
                 SpiDriver&        driver) noexcept;
        ~SpiBus () noexcept;

        SpiBus& operator= (const SpiBus&) = delete;
        SpiBus  (const SpiBus&)           = delete;
        SpiBus& operator= (SpiBus&&)      = delete;
        SpiBus  (SpiBus&&)                = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

      private:
        friend struct SpiDevice;

        Status configureDevice (PinId chipSelect) noexcept;
        void   releaseDevice   (PinId chipSelect) noexcept;
        Status transfer        (PinId              chipSelect,
                                const SpiSettings& settings,
                                const uint8_t*     writeData,
                                uint8_t*           readData,
                                uint16_t           size,
                                Duration           timeout) noexcept;

        ResourceRegistry* resources_;
        SpiDriver*        driver_;
        ResourceClaim     claim_;
        bool              initialized_;
    };

    struct SpiDevice
    {
        SpiDevice  (SpiBus&            bus,
                    PinId             chipSelect,
                    const SpiSettings& settings) noexcept;
        ~SpiDevice () noexcept;

        SpiDevice& operator= (const SpiDevice&) = delete;
        SpiDevice  (const SpiDevice&)           = delete;
        SpiDevice& operator= (SpiDevice&&)      = delete;
        SpiDevice  (SpiDevice&&)                = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status transfer   (const uint8_t* writeData,
                           uint8_t*       readData,
                           uint16_t       size,
                           Duration       timeout) noexcept;

        PinId       chipSelect  () const noexcept;
        SpiSettings settings    () const noexcept;
        bool        initialized () const noexcept;

      private:
        SpiBus*            bus_;
        ResourceClaim     chipSelectClaim_;
        PinId             chipSelect_;
        SpiSettings       settings_;
        bool              initialized_;
    };
}
