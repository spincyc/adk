#include "mega_bus_driver.h"

namespace adk {

    MegaI2cIo::~MegaI2cIo () noexcept = default;

    MegaI2cDriver::MegaI2cDriver (ResourceRegistry& resources, MegaI2cIo& io) noexcept
        : resources_ (&resources), io_ (&io), dataClaim_ (), clockClaim_ (),
          configured_ (false)
    {
    }

    MegaI2cDriver::~MegaI2cDriver () noexcept
    {
        disable ();
    }

    Status MegaI2cDriver::configure () noexcept
    {
        if (configured_)
        {
            return StatusCode::Ok;
        }

        Status status = resources_->claim ({ResourceKind::Pin, dataPin}, dataClaim_);

        if (!status.ok ())
        {
            return status;
        }

        status = resources_->claim ({ResourceKind::Pin, clockPin}, clockClaim_);

        if (!status.ok ())
        {
            dataClaim_.release ();
            return status;
        }

        status = io_->configure ();

        if (!status.ok ())
        {
            io_->disable        ();
            clockClaim_.release ();
            dataClaim_.release  ();
            return status;
        }

        configured_ = true;
        return StatusCode::Ok;
    }

    void MegaI2cDriver::disable () noexcept
    {
        if (configured_ || dataClaim_.active () || clockClaim_.active ())
        {
            io_->disable ();
        }

        configured_ = false;
        clockClaim_.release ();
        dataClaim_.release  ();
    }

    Status MegaI2cDriver::transfer (uint8_t address, const uint8_t* writeData,
                                    uint8_t writeSize, uint8_t* readData,
                                    uint8_t readSize, Duration timeout) noexcept
    {
        if (!configured_)
        {
            return StatusCode::NotInitialized;
        }

        if (timeout.milliseconds () == 0)
        {
            return StatusCode::InvalidArgument;
        }

        return io_->transfer (address, writeData, writeSize, readData, readSize,
                              timeout);
    }

    bool MegaI2cDriver::configured () const noexcept
    {
        return configured_;
    }

    MegaSpiIo::~MegaSpiIo () noexcept = default;

    MegaSpiDriver::MegaSpiDriver (ResourceRegistry& resources, MegaSpiIo& io) noexcept
        : resources_ (&resources), io_ (&io), inputClaim_ (), outputClaim_ (),
          clockClaim_ (), masterSsClaim_ (), configured_ (false)
    {
    }

    MegaSpiDriver::~MegaSpiDriver () noexcept
    {
        disable ();
    }

    Status MegaSpiDriver::configure () noexcept
    {
        if (configured_)
        {
            return StatusCode::Ok;
        }

        Status status = resources_->claim ({ResourceKind::Pin, inputPin}, inputClaim_);

        if (!status.ok ())
        {
            return status;
        }

        status = resources_->claim ({ResourceKind::Pin, outputPin}, outputClaim_);

        if (!status.ok ())
        {
            releaseClaims ();
            return status;
        }

        status = resources_->claim ({ResourceKind::Pin, clockPin}, clockClaim_);

        if (!status.ok ())
        {
            releaseClaims ();
            return status;
        }

        status = resources_->claim ({ResourceKind::Pin, masterSsPin}, masterSsClaim_);

        if (!status.ok ())
        {
            releaseClaims ();
            return status;
        }

        status = io_->configureMaster ();

        if (!status.ok ())
        {
            io_->disable  ();
            releaseClaims ();
            return status;
        }

        configured_ = true;
        return StatusCode::Ok;
    }

    void MegaSpiDriver::disable () noexcept
    {
        if (configured_ || inputClaim_.active () || outputClaim_.active () ||
            clockClaim_.active () || masterSsClaim_.active ())
        {
            io_->disable ();
        }

        configured_ = false;
        releaseClaims ();
    }

    Status MegaSpiDriver::configureChipSelect (PinId chipSelect) noexcept
    {
        if (!configured_)
        {
            return StatusCode::NotInitialized;
        }

        if (chipSelect == inputPin || chipSelect == outputPin ||
            chipSelect == clockPin || chipSelect == masterSsPin)
        {
            return StatusCode::InvalidArgument;
        }

        return io_->configureChipSelect (chipSelect);
    }

    void MegaSpiDriver::releaseChipSelect (PinId chipSelect) noexcept
    {
        io_->releaseChipSelect (chipSelect);
    }

    Status MegaSpiDriver::transfer (const SpiSettings& settings, PinId chipSelect,
                                    const uint8_t* writeData, uint8_t* readData,
                                    uint16_t size, Duration timeout) noexcept
    {
        if (!configured_)
        {
            return StatusCode::NotInitialized;
        }

        if (timeout.milliseconds () == 0)
        {
            return StatusCode::InvalidArgument;
        }

        if (!settingsSupported (settings))
        {
            return StatusCode::Unsupported;
        }

        Status status = io_->beginTransaction (settings, chipSelect);

        if (status.ok ())
        {
            status = io_->transfer (writeData, readData, size, timeout);
        }

        const Status restoreStatus = io_->endTransaction (chipSelect);

        if (!restoreStatus.ok ())
        {
            io_->disable  ();
            configured_ = false;
            releaseClaims ();
            return restoreStatus;
        }

        return status;
    }

    bool MegaSpiDriver::configured () const noexcept
    {
        return configured_;
    }

    bool MegaSpiDriver::settingsSupported (
        const SpiSettings& settings) const noexcept
    {
        switch (settings.clockHz)
        {
        case 8000000UL:
        case 4000000UL:
        case 2000000UL:
        case 1000000UL:
        case 500000UL:
        case 250000UL:
        case 125000UL:
            return true;
        default:
            return false;
        }
    }

    void MegaSpiDriver::releaseClaims () noexcept
    {
        masterSsClaim_.release ();
        clockClaim_.release    ();
        outputClaim_.release   ();
        inputClaim_.release    ();
    }
}
