#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

namespace adk::usbmatrix {

    struct HostId
    {
        uint16_t value;
    };

    struct DeviceId
    {
        uint16_t value;
    };

    enum struct RouteState : uint8_t
    {
        Unassigned,
        Detaching,
        Attaching,
        Active,
        Fault
    };

    enum struct MatrixStatus : uint8_t
    {
        Ok,
        InvalidId,
        StaleEpoch,
        WrongState,
        CapacityExceeded,
        EpochExhausted,
        PersistenceError,
        AuditFailure
    };

    enum struct AuditAction : uint8_t
    {
        RouteRequested,
        DetachRequested,
        Detached,
        AttachRequested,
        Attached,
        EndpointLost,
        TransitionTimedOut,
        FaultCleared,
        StateRestored
    };

    struct RouteSnapshot
    {
        DeviceId   device;
        HostId     activeHost;
        HostId     pendingHost;
        uint64_t   epoch;
        RouteState state;
        bool       hasActiveHost;
        bool       hasPendingHost;
        uint64_t   deadline;
    };

    struct MatrixConfig
    {
        uint64_t detachTimeoutTicks = 5000;
        uint64_t attachTimeoutTicks = 10000;
    };

    struct AuditEntry
    {
        uint64_t    sequence;
        uint64_t    tick;
        uint64_t    epoch;
        uint64_t    previousHash;
        uint64_t    hash;
        HostId      host;
        DeviceId    device;
        AuditAction action;
        RouteState  state;
    };

    struct AuditLog
    {
        static constexpr std::size_t capacity = 1024;

        AuditLog () noexcept;

        MatrixStatus      append (uint64_t tick, uint64_t epoch, HostId host,
                                  DeviceId device, AuditAction action,
                                  RouteState state) noexcept;
        const AuditEntry& at     (std::size_t index) const noexcept;
        std::size_t       size   () const noexcept;
        std::size_t       free   () const noexcept;
        uint64_t          head   () const noexcept;
        bool              verify () const noexcept;
        void              clear  () noexcept;

      private:
        std::array<AuditEntry, capacity> entries_;
        std::size_t                      size_;
    };

    struct MatrixController
    {
        static constexpr std::size_t maximumDevices = 256;

        explicit MatrixController (
            AuditLog& audit, const MatrixConfig& config = MatrixConfig{}) noexcept;

        MatrixStatus requestRoute (uint64_t tick, HostId host,
                                   DeviceId device) noexcept;
        MatrixStatus detachRoute     (uint64_t tick, DeviceId device) noexcept;
        MatrixStatus confirmDetached (uint64_t tick, DeviceId device,
                                      uint64_t epoch) noexcept;
        MatrixStatus confirmAttached (uint64_t tick, HostId host, DeviceId device,
                                      uint64_t epoch) noexcept;
        MatrixStatus reportEndpointLost (uint64_t tick, DeviceId device,
                                         uint64_t epoch) noexcept;
        MatrixStatus clearFault (uint64_t tick, DeviceId device) noexcept;
        MatrixStatus update     (uint64_t tick) noexcept;

        RouteSnapshot snapshot (DeviceId device) const noexcept;
        MatrixStatus  save     (std::ostream& output) const noexcept;
        MatrixStatus  load     (std::istream& input, uint64_t tick) noexcept;

      private:
        struct Route
        {
            uint64_t   epoch;
            HostId     activeHost;
            HostId     pendingHost;
            RouteState state;
            bool       hasActiveHost;
            bool       hasPendingHost;
            uint64_t   deadline;
        };

        MatrixStatus startAttach (uint64_t tick, HostId host, DeviceId device,
                                  Route& route) noexcept;
        MatrixStatus record (uint64_t tick, HostId host, DeviceId device,
                             AuditAction action, RouteState state,
                             uint64_t epoch) noexcept;
        bool         valid      (HostId host, DeviceId device) const noexcept;
        bool         hostInUse  (HostId host, DeviceId except) const noexcept;
        bool         auditSpace (std::size_t entries) const noexcept;
        bool         epochReady (const Route& route) const noexcept;

        AuditLog&                         audit_;
        MatrixConfig                      config_;
        std::array<Route, maximumDevices> routes_;
    };

    const char* matrixStatusName (MatrixStatus status) noexcept;
    const char* routeStateName   (RouteState state) noexcept;
} // namespace adk::usbmatrix
