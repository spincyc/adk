#pragma once

#include <stdint.h>

namespace adk {

    enum struct Status : uint8_t
    {
        Ok,
        InvalidArgument,
        InvalidPin,
        Unsupported,
        ResourceBusy,
        NotInitialized,
        CapacityExceeded,
        HardwareFailure
    };

    const char* statusName (Status status) noexcept;

    template<typename Value>
    struct Result
    {
        Result (Status status, const Value& value)
            : status_ (status)
            , value_  (value)
        {
        }

        bool ok () const noexcept
        {
            return status_ == Status::Ok;
        }

        Status status () const noexcept
        {
            return status_;
        }

        const Value& value () const noexcept
        {
            return value_;
        }

      private:
        Status status_;
        Value  value_;
    };
}
