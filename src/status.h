#pragma once

#include <stdint.h>

namespace adk {

    enum struct StatusCode : uint8_t
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

    struct Status
    {
        constexpr Status () noexcept
            : code_ (StatusCode::Ok)
        {
        }

        constexpr Status (StatusCode code) noexcept
            : code_ (code)
        {
        }

        constexpr bool ok () const noexcept
        {
            return code_ == StatusCode::Ok;
        }

        constexpr StatusCode error () const noexcept
        {
            return code_;
        }

        constexpr bool transient () const noexcept
        {
            return code_ == StatusCode::ResourceBusy ||
                   code_ == StatusCode::HardwareFailure;
        }

      private:
        StatusCode code_;
    };

    constexpr bool operator== (Status left, Status right) noexcept
    {
        return left.error () == right.error ();
    }

    constexpr bool operator!= (Status left, Status right) noexcept
    {
        return !(left == right);
    }

    const char* statusName (Status status) noexcept;

    template<typename Value>
    struct Result
    {
        Result (Status status, const Value& value) noexcept
            : status_ (status)
            , value_  (value)
        {
        }

        bool ok () const noexcept
        {
            return status_.ok ();
        }

        Status status () const noexcept
        {
            return status_;
        }

        StatusCode error () const noexcept
        {
            return status_.error ();
        }

        bool transient () const noexcept
        {
            return status_.transient ();
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
