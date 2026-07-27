#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct ResourceKind : uint8_t
    {
        Pin,
        Timer,
        Interrupt,
        I2cBus,
        SpiBus,
        SerialPort
    };

    struct ResourceId
    {
        ResourceKind kind;
        uint8_t      index;
    };

    struct ResourceRegistry;

    struct ResourceClaim
    {
        ResourceClaim  () noexcept;
        ~ResourceClaim () noexcept;

        ResourceClaim& operator= (const ResourceClaim&) = delete;
        ResourceClaim  (const ResourceClaim&)           = delete;
        ResourceClaim& operator= (ResourceClaim&&)      = delete;
        ResourceClaim  (ResourceClaim&&)                = delete;

        bool active  () const noexcept;
        void release () noexcept;

      private:
        friend struct ResourceRegistry;

        ResourceRegistry* registry_;
        ResourceId        resource_;
    };

    struct ResourceRegistry
    {
        ResourceRegistry () noexcept;

        ResourceRegistry& operator= (const ResourceRegistry&) = delete;
        ResourceRegistry  (const ResourceRegistry&)           = delete;
        ResourceRegistry& operator= (ResourceRegistry&&)      = delete;
        ResourceRegistry  (ResourceRegistry&&)                = delete;

        Status claim   (ResourceId resource, ResourceClaim& claim) noexcept;
        bool   claimed (ResourceId resource) const noexcept;

      private:
        friend struct ResourceClaim;

        static const uint8_t StorageSize = 11;

        void release    (ResourceId resource) noexcept;
        bool storageBit (ResourceId resource, uint8_t& byte, uint8_t& mask) const noexcept;

        uint8_t storage_[StorageSize];
    };
}
