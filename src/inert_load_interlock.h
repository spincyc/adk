#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    struct PowerDomain;

    enum struct SimulatedLoad : uint8_t
    {
        None,
        Fan,
        Pump,
        Heater
    };

    struct InertLoadSnapshot
    {
        SimulatedLoad requested;
        SimulatedLoad active;
        Status        status;
    };

    struct InertLoadInterlock
    {
        explicit InertLoadInterlock (const PowerDomain& power) noexcept;
        ~InertLoadInterlock         () noexcept;

        InertLoadInterlock            (const InertLoadInterlock&)            = delete;
        InertLoadInterlock& operator= (const InertLoadInterlock&) = delete;
        InertLoadInterlock            (InertLoadInterlock&&)                 = delete;
        InertLoadInterlock& operator= (InertLoadInterlock&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status select (SimulatedLoad load) noexcept;
        Status update () noexcept;

        InertLoadSnapshot snapshot    () const noexcept;
        bool              initialized () const noexcept;

      private:
        bool validLoad (SimulatedLoad load) const noexcept;

        const PowerDomain* power_;
        SimulatedLoad      requested_;
        SimulatedLoad      active_;
        Status             status_;
        bool               initialized_;
    };
} // namespace adk
