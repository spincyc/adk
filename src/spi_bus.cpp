#include "spi_bus.h"

namespace adk {

    namespace {
        bool validTimeout (Duration timeout) noexcept
        {
            return timeout.milliseconds () != 0
                && timeout.milliseconds () <= 0x7fffffffUL;
        }
    }

    SpiDriver::~SpiDriver () noexcept = default;

    SpiBus::SpiBus (ResourceRegistry& resources,
                    SpiDriver&        driver) noexcept
        : resources_   (&resources)
        , driver_      (&driver)
        , claim_       ()
        , initialized_ (false)
    {
    }

    SpiBus::~SpiBus () noexcept
    {
        shutdown ();
    }

    Status SpiBus::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        Status status =
            resources_->claim ({ResourceKind::SpiBus, 0}, claim_);

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

    void SpiBus::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        driver_->disable ();
        claim_.release   ();
        initialized_ = false;
    }

    bool SpiBus::initialized () const noexcept
    {
        return initialized_;
    }

    Status SpiBus::configureDevice (PinId chipSelect) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        return driver_->configureChipSelect (chipSelect);
    }

    void SpiBus::releaseDevice (PinId chipSelect) noexcept
    {
        driver_->releaseChipSelect (chipSelect);
    }

    Status SpiBus::transfer (
        PinId              chipSelect,
        const SpiSettings& settings,
        const uint8_t*     writeData,
        uint8_t*           readData,
        uint16_t           size,
        Duration           timeout) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (size == 0
            || size > maximumTransferSize
            || (writeData == nullptr && readData == nullptr)
            || !validTimeout (timeout))
        {
            return StatusCode::InvalidArgument;
        }

        return driver_->transfer (
            settings, chipSelect, writeData, readData, size, timeout);
    }

    SpiDevice::SpiDevice (SpiBus&            bus,
                          PinId             chipSelect,
                          const SpiSettings& settings) noexcept
        : bus_             (&bus)
        , chipSelectClaim_ ()
        , chipSelect_      (chipSelect)
        , settings_        (settings)
        , initialized_     (false)
    {
    }

    SpiDevice::~SpiDevice () noexcept
    {
        shutdown ();
    }

    Status SpiDevice::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (settings_.clockHz == 0 || settings_.mode > 3)
        {
            return StatusCode::InvalidArgument;
        }

        Status status = bus_->resources_->claim (
            {ResourceKind::Pin, chipSelect_}, chipSelectClaim_);

        if (!status.ok ())
        {
            if (status.error () == StatusCode::Unsupported)
            {
                return StatusCode::InvalidPin;
            }

            return status;
        }

        status = bus_->configureDevice (chipSelect_);

        if (!status.ok ())
        {
            bus_->releaseDevice      (chipSelect_);
            chipSelectClaim_.release ();
            return status;
        }

        initialized_ = true;
        return StatusCode::Ok;
    }

    void SpiDevice::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        bus_->releaseDevice      (chipSelect_);
        chipSelectClaim_.release ();
        initialized_ = false;
    }

    Status SpiDevice::transfer (
        const uint8_t* writeData,
        uint8_t*       readData,
        uint16_t       size,
        Duration       timeout) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        return bus_->transfer (
            chipSelect_, settings_, writeData, readData, size, timeout);
    }

    PinId SpiDevice::chipSelect () const noexcept
    {
        return chipSelect_;
    }

    SpiSettings SpiDevice::settings () const noexcept
    {
        return settings_;
    }

    bool SpiDevice::initialized () const noexcept
    {
        return initialized_;
    }
}
