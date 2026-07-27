#include "fixed_storage.h"

namespace adk {

    FixedStorageMedium::FixedStorageMedium (uint16_t capacity) noexcept
        : data_ (), capacity_ (capacity), durableSize_ (0)
    {
    }

    const uint8_t* FixedStorageMedium::data () const noexcept
    {
        return data_;
    }

    uint16_t FixedStorageMedium::capacity () const noexcept
    {
        return capacity_;
    }

    uint16_t FixedStorageMedium::durableSize () const noexcept
    {
        return durableSize_;
    }

    FixedStorage::FixedStorage (FixedStorageMedium& medium) noexcept
        : medium_ (&medium), stagedSize_ (medium.durableSize_),
          appendFailureOffset_ (noAppendFailure), syncFailure_ (false),
          initialized_         (false)
    {
    }

    FixedStorage::~FixedStorage () noexcept
    {
        shutdown ();
    }

    Status FixedStorage::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }

        if (medium_->capacity_ == 0 ||
            medium_->capacity_ > FixedStorageMedium::maximumCapacity)
        {
            return StatusCode::InvalidArgument;
        }

        stagedSize_  = medium_->durableSize_;
        initialized_ = true;
        return Status ();
    }

    void FixedStorage::shutdown () noexcept
    {
        stagedSize_  = medium_->durableSize_;
        initialized_ = false;
    }

    Status FixedStorage::append (const uint8_t* data, uint16_t size) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (size != 0 && data == 0)
        {
            return StatusCode::InvalidArgument;
        }

        if (size > static_cast<uint16_t> (medium_->capacity_ - stagedSize_))
        {
            return StatusCode::CapacityExceeded;
        }

        for (uint16_t index = 0; index < size; ++index)
        {
            if (index == appendFailureOffset_)
            {
                return StatusCode::HardwareFailure;
            }

            medium_->data_[stagedSize_] = data[index];
            ++stagedSize_;
        }

        return Status ();
    }

    Status FixedStorage::sync () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (syncFailure_)
        {
            return StatusCode::HardwareFailure;
        }

        medium_->durableSize_ = stagedSize_;
        return Status ();
    }

    void FixedStorage::injectAppendFailureAt (uint16_t byteOffset) noexcept
    {
        appendFailureOffset_ = byteOffset;
    }

    void FixedStorage::injectSyncFailure (bool fail) noexcept
    {
        syncFailure_ = fail;
    }

    bool FixedStorage::initialized () const noexcept
    {
        return initialized_;
    }

    uint16_t FixedStorage::stagedSize () const noexcept
    {
        return stagedSize_;
    }
} // namespace adk
