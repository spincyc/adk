#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    struct Storage
    {
        virtual ~Storage () noexcept;

        virtual Status initialize () noexcept = 0;
        virtual void   shutdown   () noexcept   = 0;
        // append                 () stages bytes. Only successful sync() makes them durable.
        virtual Status append     (const uint8_t* data, uint16_t size) noexcept = 0;
        virtual Status sync       () noexcept                                     = 0;
    };
} // namespace adk
