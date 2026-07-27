#include "i2c_bus.h"

namespace adk {

    namespace {
        const uint8_t unusedAddress = 0xff;

        bool validTimeout (Duration timeout) noexcept
        {
            return timeout.milliseconds () != 0
                && timeout.milliseconds () <= 0x7fffffffUL;
        }
    }

    I2cDriver::~I2cDriver () noexcept = default;

    I2cBus::I2cBus (ResourceRegistry& resources,
                    I2cDriver&        driver) noexcept
        : resources_   (&resources)
        , driver_      (&driver)
        , claim_       ()
        , addresses_   {}
        , deviceCount_ (0)
        , initialized_ (false)
    {
        for (uint8_t& address : addresses_)
        {
            address = unusedAddress;
        }
    }

    I2cBus::~I2cBus () noexcept
    {
        shutdown ();
    }

    Status I2cBus::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        Status status =
            resources_->claim ({ResourceKind::I2cBus, 0}, claim_);

        if (!status.ok ())
        {
            return status;
        }

        status = driver_->configure ();

        if (!status.ok ())
        {
            driver_->disable ();
            claim_.release   ();
            return status;
        }

        initialized_ = true;
        return StatusCode::Ok;
    }

    void I2cBus::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        driver_->disable ();
        claim_.release   ();
        initialized_ = false;
    }

    bool I2cBus::initialized () const noexcept
    {
        return initialized_;
    }

    Status I2cBus::attach (uint8_t address) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (address < 0x08 || address > 0x77)
        {
            return StatusCode::InvalidArgument;
        }

        for (uint8_t existing : addresses_)
        {
            if (existing == address)
            {
                return StatusCode::ResourceBusy;
            }
        }

        if (deviceCount_ == maximumDeviceCount)
        {
            return StatusCode::CapacityExceeded;
        }

        for (uint8_t& existing : addresses_)
        {
            if (existing == unusedAddress)
            {
                existing = address;
                ++deviceCount_;
                return StatusCode::Ok;
            }
        }

        return StatusCode::CapacityExceeded;
    }

    void I2cBus::detach (uint8_t address) noexcept
    {
        for (uint8_t& existing : addresses_)
        {
            if (existing == address)
            {
                existing = unusedAddress;
                --deviceCount_;
                return;
            }
        }
    }

    Status I2cBus::transfer (
        uint8_t        address,
        const uint8_t* writeData,
        uint8_t        writeSize,
        uint8_t*       readData,
        uint8_t        readSize,
        Duration       timeout) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (writeSize > maximumTransferSize
            || readSize > maximumTransferSize
            || (writeSize == 0 && readSize == 0)
            || (writeSize != 0 && writeData == nullptr)
            || (readSize != 0 && readData == nullptr)
            || !validTimeout (timeout))
        {
            return StatusCode::InvalidArgument;
        }

        return driver_->transfer (
            address, writeData, writeSize, readData, readSize, timeout);
    }

    I2cDevice::I2cDevice (I2cBus& bus,
                          uint8_t address) noexcept
        : bus_         (&bus)
        , address_     (address)
        , initialized_ (false)
    {
    }

    I2cDevice::~I2cDevice () noexcept
    {
        shutdown ();
    }

    Status I2cDevice::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        Status status = bus_->attach (address_);

        if (status.ok ())
        {
            initialized_ = true;
        }

        return status;
    }

    void I2cDevice::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        bus_->detach  (address_);
        initialized_ = false;
    }

    Status I2cDevice::transfer (
        const uint8_t* writeData,
        uint8_t        writeSize,
        uint8_t*       readData,
        uint8_t        readSize,
        Duration       timeout) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        return bus_->transfer (
            address_, writeData, writeSize, readData, readSize, timeout);
    }

    uint8_t I2cDevice::address () const noexcept
    {
        return address_;
    }

    bool I2cDevice::initialized () const noexcept
    {
        return initialized_;
    }
}
