#pragma once

#include "resource.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct I2cDriver
    {
        virtual ~I2cDriver () noexcept;

        virtual Status configure () noexcept = 0;
        virtual void   disable   () noexcept = 0;
        virtual Status transfer  (uint8_t        address,
                                  const uint8_t* writeData,
                                  uint8_t        writeSize,
                                  uint8_t*       readData,
                                  uint8_t        readSize,
                                  Duration       timeout) noexcept = 0;
    };

    struct I2cDevice;

    struct I2cBus
    {
        static const uint8_t maximumDeviceCount  = 8;
        static const uint8_t maximumTransferSize = 32;

        I2cBus  (ResourceRegistry& resources,
                 I2cDriver&        driver) noexcept;
        ~I2cBus () noexcept;

        I2cBus& operator= (const I2cBus&) = delete;
        I2cBus  (const I2cBus&)           = delete;
        I2cBus& operator= (I2cBus&&)      = delete;
        I2cBus  (I2cBus&&)                = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

      private:
        friend struct I2cDevice;

        Status attach   (uint8_t address) noexcept;
        void   detach   (uint8_t address) noexcept;
        Status transfer (uint8_t        address,
                         const uint8_t* writeData,
                         uint8_t        writeSize,
                         uint8_t*       readData,
                         uint8_t        readSize,
                         Duration       timeout) noexcept;

        ResourceRegistry* resources_;
        I2cDriver*        driver_;
        ResourceClaim     claim_;
        uint8_t           addresses_[maximumDeviceCount];
        uint8_t           deviceCount_;
        bool              initialized_;
    };

    struct I2cDevice
    {
        I2cDevice  (I2cBus& bus,
                    uint8_t address) noexcept;
        ~I2cDevice () noexcept;

        I2cDevice& operator= (const I2cDevice&) = delete;
        I2cDevice  (const I2cDevice&)           = delete;
        I2cDevice& operator= (I2cDevice&&)      = delete;
        I2cDevice  (I2cDevice&&)                = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status transfer   (const uint8_t* writeData,
                           uint8_t        writeSize,
                           uint8_t*       readData,
                           uint8_t        readSize,
                           Duration       timeout) noexcept;

        uint8_t address     () const noexcept;
        bool    initialized () const noexcept;

      private:
        I2cBus* bus_;
        uint8_t address_;
        bool    initialized_;
    };
}
