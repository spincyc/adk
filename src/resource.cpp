#include "resource.h"

namespace adk {

    namespace {
        const uint8_t PinCount       = 70;
        const uint8_t TimerCount     = 6;
        const uint8_t InterruptCount = 6;
        const uint8_t I2cBusCount    = 1;
        const uint8_t SpiBusCount    = 1;
        const uint8_t SerialCount    = 4;
    }

    ResourceClaim::ResourceClaim () noexcept
        : registry_ (nullptr)
        , resource_ ({ResourceKind::Pin, 0})
    {
    }

    ResourceClaim::~ResourceClaim () noexcept
    {
        release ();
    }

    bool ResourceClaim::active () const noexcept
    {
        return registry_ != nullptr;
    }

    void ResourceClaim::release () noexcept
    {
        if (registry_)
        {
            registry_->release (resource_);
            registry_ = nullptr;
        }
    }

    ResourceRegistry::ResourceRegistry () noexcept
        : storage_ {}
    {
    }

    Status ResourceRegistry::claim (
        ResourceId     resource,
        ResourceClaim& claim) noexcept
    {
        if (claim.active ())
        {
            return Status::InvalidArgument;
        }

        uint8_t byte = 0;
        uint8_t mask = 0;
        if (!storageBit (resource, byte, mask))
        {
            return Status::Unsupported;
        }

        if (storage_[byte] & mask)
        {
            return Status::ResourceBusy;
        }

        storage_[byte] |= mask;
        claim.registry_ = this;
        claim.resource_ = resource;
        return Status::Ok;
    }

    bool ResourceRegistry::claimed (ResourceId resource) const noexcept
    {
        uint8_t byte = 0;
        uint8_t mask = 0;
        return storageBit (resource, byte, mask) && (storage_[byte] & mask);
    }

    void ResourceRegistry::release (ResourceId resource) noexcept
    {
        uint8_t byte = 0;
        uint8_t mask = 0;
        if (storageBit (resource, byte, mask))
        {
            storage_[byte] &= static_cast<uint8_t> (~mask);
        }
    }

    bool ResourceRegistry::storageBit (
        ResourceId resource,
        uint8_t&   byte,
        uint8_t&   mask) const noexcept
    {
        uint8_t offset = 0;
        uint8_t count  = 0;

        switch (resource.kind)
        {
        case ResourceKind::Pin:
            offset = 0;
            count  = PinCount;
            break;
        case ResourceKind::Timer:
            offset = 70;
            count  = TimerCount;
            break;
        case ResourceKind::Interrupt:
            offset = 76;
            count  = InterruptCount;
            break;
        case ResourceKind::I2cBus:
            offset = 82;
            count  = I2cBusCount;
            break;
        case ResourceKind::SpiBus:
            offset = 83;
            count  = SpiBusCount;
            break;
        case ResourceKind::SerialPort:
            offset = 84;
            count  = SerialCount;
            break;
        }

        if (resource.index >= count)
        {
            return false;
        }

        const uint8_t bit = offset + resource.index;
        byte              = bit / 8;
        mask              = static_cast<uint8_t> (1U << (bit % 8));
        return byte < StorageSize;
    }
}
