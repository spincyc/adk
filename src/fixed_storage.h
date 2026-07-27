#pragma once

#include "storage.h"

#include <stdint.h>

namespace adk {

    // Medium lifetime represents persistence across storage instances.
    struct FixedStorageMedium
    {
        static const uint16_t maximumCapacity = 512;

        explicit FixedStorageMedium (uint16_t capacity) noexcept;

        const uint8_t* data        () const noexcept;
        uint16_t       capacity    () const noexcept;
        uint16_t       durableSize () const noexcept;

      private:
        friend struct FixedStorage;

        uint8_t  data_[maximumCapacity];
        uint16_t capacity_;
        uint16_t durableSize_;
    };

    // Failed appends retain their staged prefix. Shutdown discards every
    // unsynced byte by restoring the medium's durable prefix.
    struct FixedStorage final : Storage
    {
        static const uint16_t noAppendFailure = 0xffffu;

        explicit FixedStorage (FixedStorageMedium& medium) noexcept;
        ~FixedStorage         () noexcept override;

        FixedStorage (const FixedStorage&)            = delete;
        FixedStorage& operator= (const FixedStorage&) = delete;
        FixedStorage (FixedStorage&&)                 = delete;
        FixedStorage& operator= (FixedStorage&&)      = delete;

        Status initialize () noexcept override;
        void   shutdown   () noexcept override;
        Status append     (const uint8_t* data, uint16_t size) noexcept override;
        Status sync       () noexcept override;

        void injectAppendFailureAt (uint16_t byteOffset) noexcept;
        void injectSyncFailure     (bool fail) noexcept;

        bool     initialized () const noexcept;
        uint16_t stagedSize  () const noexcept;

      private:
        FixedStorageMedium* medium_;
        uint16_t            stagedSize_;
        uint16_t            appendFailureOffset_;
        bool                syncFailure_;
        bool                initialized_;
    };
} // namespace adk
