#include "matrix_controller.h"

#include <istream>
#include <ostream>
#include <string>

namespace adk::usbmatrix {

    namespace {

        constexpr uint64_t fnvOffset = 14695981039346656037ull;
        constexpr uint64_t fnvPrime  = 1099511628211ull;

        uint64_t mix (uint64_t hash, uint64_t value) noexcept
        {
            for (uint8_t index = 0; index < 8; ++index)
            {
                hash ^= static_cast<uint8_t> (value & 0xffu);
                hash *= fnvPrime;
                value >>= 8u;
            }

            return hash;
        }

        uint64_t entryHash (const AuditEntry& entry) noexcept
        {
            uint64_t hash = fnvOffset;

            hash = mix (hash, entry.previousHash);
            hash = mix (hash, entry.sequence);
            hash = mix (hash, entry.tick);
            hash = mix (hash, entry.epoch);
            hash = mix (hash, entry.host.value);
            hash = mix (hash, entry.device.value);
            hash = mix (hash, static_cast<uint8_t> (entry.action));
            hash = mix (hash, static_cast<uint8_t> (entry.state));

            return hash;
        }

    } // namespace

    AuditLog::AuditLog () noexcept : entries_{}, size_ (0)
    {
    }

    MatrixStatus AuditLog::append (uint64_t tick, uint64_t epoch, HostId host,
                                   DeviceId device, AuditAction action,
                                   RouteState state) noexcept
    {
        if (size_ == capacity)
        {
            return MatrixStatus::CapacityExceeded;
        }

        AuditEntry& entry = entries_[size_];

        entry.sequence     = size_;
        entry.tick         = tick;
        entry.epoch        = epoch;
        entry.previousHash = size_ == 0 ? 0 : entries_[size_ - 1].hash;
        entry.host         = host;
        entry.device       = device;
        entry.action       = action;
        entry.state        = state;
        entry.hash         = entryHash (entry);
        ++size_;

        return MatrixStatus::Ok;
    }

    const AuditEntry& AuditLog::at (std::size_t index) const noexcept
    {
        return entries_[index];
    }

    std::size_t AuditLog::size () const noexcept
    {
        return size_;
    }

    std::size_t AuditLog::free () const noexcept
    {
        return capacity - size_;
    }

    uint64_t AuditLog::head () const noexcept
    {
        return size_ == 0 ? 0 : entries_[size_ - 1].hash;
    }

    bool AuditLog::verify () const noexcept
    {
        uint64_t previousHash = 0;

        for (std::size_t index = 0; index < size_; ++index)
        {
            const AuditEntry& entry = entries_[index];

            if (entry.sequence != index || entry.previousHash != previousHash ||
                entry.hash != entryHash (entry))
            {
                return false;
            }

            previousHash = entry.hash;
        }

        return true;
    }

    void AuditLog::clear () noexcept
    {
        size_ = 0;
    }

    MatrixController::MatrixController (AuditLog&           audit,
                                        const MatrixConfig& config) noexcept
        : audit_ (audit), config_ (config), routes_{}
    {
        for (Route& route : routes_)
        {
            route.epoch          = 0;
            route.activeHost     = HostId{0};
            route.pendingHost    = HostId{0};
            route.state          = RouteState::Unassigned;
            route.hasActiveHost  = false;
            route.hasPendingHost = false;
            route.deadline       = 0;
        }
    }

    MatrixStatus MatrixController::requestRoute (uint64_t tick, HostId host,
                                                 DeviceId device) noexcept
    {
        if (!valid (host, device))
        {
            return MatrixStatus::InvalidId;
        }

        Route& route = routes_[device.value];

        if (route.state == RouteState::Active && route.hasActiveHost &&
            route.activeHost.value == host.value)
        {
            return MatrixStatus::Ok;
        }

        if (route.state != RouteState::Unassigned && route.state != RouteState::Active)
        {
            return MatrixStatus::WrongState;
        }

        if (hostInUse (host, device))
        {
            return MatrixStatus::WrongState;
        }

        if (!auditSpace (2))
        {
            return MatrixStatus::CapacityExceeded;
        }

        if (!epochReady (route))
        {
            return MatrixStatus::EpochExhausted;
        }

        ++route.epoch;
        route.pendingHost    = host;
        route.hasPendingHost = true;

        MatrixStatus status = record (tick, host, device, AuditAction::RouteRequested,
                                      route.state, route.epoch);
        if (status != MatrixStatus::Ok)
        {
            route.hasPendingHost = false;
            --route.epoch;
            return status;
        }

        if (route.state == RouteState::Active)
        {
            route.state    = RouteState::Detaching;
            route.deadline = tick + config_.detachTimeoutTicks;
            return record (tick, route.activeHost, device, AuditAction::DetachRequested,
                           route.state, route.epoch);
        }

        return startAttach (tick, host, device, route);
    }

    MatrixStatus MatrixController::detachRoute (uint64_t tick, DeviceId device) noexcept
    {
        if (device.value >= maximumDevices)
        {
            return MatrixStatus::InvalidId;
        }

        Route& route = routes_[device.value];

        if (route.state == RouteState::Unassigned)
        {
            return MatrixStatus::Ok;
        }

        if (route.state != RouteState::Active)
        {
            return MatrixStatus::WrongState;
        }

        if (!auditSpace (1))
        {
            return MatrixStatus::CapacityExceeded;
        }

        if (!epochReady (route))
        {
            return MatrixStatus::EpochExhausted;
        }

        ++route.epoch;
        route.hasPendingHost = false;
        route.state          = RouteState::Detaching;
        route.deadline       = tick + config_.detachTimeoutTicks;

        return record (tick, route.activeHost, device, AuditAction::DetachRequested,
                       route.state, route.epoch);
    }

    MatrixStatus MatrixController::confirmDetached (uint64_t tick, DeviceId device,
                                                    uint64_t epoch) noexcept
    {
        if (device.value >= maximumDevices)
        {
            return MatrixStatus::InvalidId;
        }

        Route& route = routes_[device.value];

        if (epoch != route.epoch)
        {
            return MatrixStatus::StaleEpoch;
        }

        if (route.state != RouteState::Detaching)
        {
            return MatrixStatus::WrongState;
        }

        if (!auditSpace (route.hasPendingHost ? 2 : 1))
        {
            return MatrixStatus::CapacityExceeded;
        }

        HostId oldHost      = route.activeHost;
        route.hasActiveHost = false;

        MatrixStatus status = record (tick, oldHost, device, AuditAction::Detached,
                                      route.state, route.epoch);
        if (status != MatrixStatus::Ok)
        {
            route.state          = RouteState::Fault;
            route.hasPendingHost = false;
            return status;
        }

        if (!route.hasPendingHost)
        {
            route.state    = RouteState::Unassigned;
            route.deadline = 0;
            return MatrixStatus::Ok;
        }

        return startAttach (tick, route.pendingHost, device, route);
    }

    MatrixStatus MatrixController::confirmAttached (uint64_t tick, HostId host,
                                                    DeviceId device,
                                                    uint64_t epoch) noexcept
    {
        if (!valid (host, device))
        {
            return MatrixStatus::InvalidId;
        }

        Route& route = routes_[device.value];

        if (epoch != route.epoch)
        {
            return MatrixStatus::StaleEpoch;
        }

        if (route.state != RouteState::Attaching || !route.hasPendingHost ||
            route.pendingHost.value != host.value)
        {
            return MatrixStatus::WrongState;
        }

        if (!auditSpace (1))
        {
            return MatrixStatus::CapacityExceeded;
        }

        route.activeHost     = host;
        route.hasActiveHost  = true;
        route.hasPendingHost = false;
        route.state          = RouteState::Active;
        route.deadline       = 0;

        return record (tick, host, device, AuditAction::Attached, route.state,
                       route.epoch);
    }

    MatrixStatus MatrixController::reportEndpointLost (uint64_t tick, DeviceId device,
                                                       uint64_t epoch) noexcept
    {
        if (device.value >= maximumDevices)
        {
            return MatrixStatus::InvalidId;
        }

        Route& route = routes_[device.value];

        if (epoch != route.epoch)
        {
            return MatrixStatus::StaleEpoch;
        }

        if (route.state == RouteState::Unassigned)
        {
            return MatrixStatus::WrongState;
        }

        if (!auditSpace (1))
        {
            return MatrixStatus::CapacityExceeded;
        }

        route.hasActiveHost  = false;
        route.hasPendingHost = false;
        route.state          = RouteState::Fault;
        route.deadline       = 0;

        return record (tick, HostId{0}, device, AuditAction::EndpointLost, route.state,
                       route.epoch);
    }

    MatrixStatus MatrixController::clearFault (uint64_t tick, DeviceId device) noexcept
    {
        if (device.value >= maximumDevices)
        {
            return MatrixStatus::InvalidId;
        }

        Route& route = routes_[device.value];

        if (route.state != RouteState::Fault)
        {
            return MatrixStatus::WrongState;
        }

        if (!auditSpace (1))
        {
            return MatrixStatus::CapacityExceeded;
        }

        if (!epochReady (route))
        {
            return MatrixStatus::EpochExhausted;
        }

        ++route.epoch;
        route.state    = RouteState::Unassigned;
        route.deadline = 0;

        return record (tick, HostId{0}, device, AuditAction::FaultCleared, route.state,
                       route.epoch);
    }

    MatrixStatus MatrixController::update (uint64_t tick) noexcept
    {
        for (std::size_t index = 0; index < routes_.size (); ++index)
        {
            Route& route = routes_[index];

            if ((route.state != RouteState::Detaching &&
                 route.state != RouteState::Attaching) ||
                static_cast<int64_t> (tick - route.deadline) < 0)
            {
                continue;
            }

            if (!auditSpace (1))
            {
                return MatrixStatus::CapacityExceeded;
            }

            route.hasActiveHost  = false;
            route.hasPendingHost = false;
            route.state          = RouteState::Fault;
            route.deadline       = 0;

            MatrixStatus status =
                record (tick, HostId{0}, DeviceId{static_cast<uint16_t> (index)},
                        AuditAction::TransitionTimedOut, route.state, route.epoch);
            if (status != MatrixStatus::Ok)
            {
                return status;
            }
        }

        return MatrixStatus::Ok;
    }

    RouteSnapshot MatrixController::snapshot (DeviceId device) const noexcept
    {
        if (device.value >= maximumDevices)
        {
            return RouteSnapshot{device, HostId{0}, HostId{0}, 0, RouteState::Fault,
                                 false,  false,     0};
        }

        const Route& route = routes_[device.value];

        return RouteSnapshot{
            device,      route.activeHost,    route.pendingHost,    route.epoch,
            route.state, route.hasActiveHost, route.hasPendingHost, route.deadline};
    }

    MatrixStatus MatrixController::save (std::ostream& output) const noexcept
    {
        output << "ADK_USB_MATRIX 1\n";

        for (std::size_t index = 0; index < routes_.size (); ++index)
        {
            const Route& route = routes_[index];

            if (route.epoch == 0 && route.state == RouteState::Unassigned)
            {
                continue;
            }

            output << index << ' ' << route.epoch << ' '
                   << static_cast<unsigned> (route.state) << ' ' << route.hasActiveHost
                   << ' ' << route.activeHost.value << ' ' << route.hasPendingHost
                   << ' ' << route.pendingHost.value << '\n';
        }

        return output.good () ? MatrixStatus::Ok : MatrixStatus::PersistenceError;
    }

    MatrixStatus MatrixController::load (std::istream& input, uint64_t tick) noexcept
    {
        char     magic[15] = {};
        unsigned version   = 0;

        input >> magic >> version;
        if (!input.good () || version != 1 || std::string (magic) != "ADK_USB_MATRIX")
        {
            return MatrixStatus::PersistenceError;
        }

        std::array<Route, maximumDevices> restored{};

        for (Route& route : restored)
        {
            route.epoch          = 0;
            route.activeHost     = HostId{0};
            route.pendingHost    = HostId{0};
            route.state          = RouteState::Unassigned;
            route.hasActiveHost  = false;
            route.hasPendingHost = false;
        }

        std::size_t index = 0;
        while (input >> index)
        {
            uint64_t epoch          = 0;
            unsigned stateValue     = 0;
            bool     hasActiveHost  = false;
            unsigned activeHost     = 0;
            bool     hasPendingHost = false;
            unsigned pendingHost    = 0;

            input >> epoch >> stateValue >> hasActiveHost >> activeHost >>
                hasPendingHost >> pendingHost;

            if (!input.good () || index >= maximumDevices ||
                stateValue > static_cast<unsigned> (RouteState::Fault) ||
                activeHost > UINT16_MAX || pendingHost > UINT16_MAX ||
                epoch == UINT64_MAX)
            {
                return MatrixStatus::PersistenceError;
            }

            Route& route = restored[index];
            if (route.epoch != 0)
            {
                return MatrixStatus::PersistenceError;
            }

            route.epoch          = epoch + 1;
            route.activeHost     = HostId{static_cast<uint16_t> (activeHost)};
            route.pendingHost    = HostId{static_cast<uint16_t> (pendingHost)};
            route.hasActiveHost  = false;
            route.hasPendingHost = false;
            route.state = stateValue == static_cast<unsigned> (RouteState::Unassigned)
                              ? RouteState::Unassigned
                              : RouteState::Fault;
            route.deadline = 0;
        }

        if (!input.eof ())
        {
            return MatrixStatus::PersistenceError;
        }

        std::size_t restoredCount = 0;
        for (const Route& route : restored)
        {
            if (route.epoch != 0)
            {
                ++restoredCount;
            }
        }

        if (!auditSpace (restoredCount))
        {
            return MatrixStatus::CapacityExceeded;
        }

        routes_ = restored;
        for (std::size_t index = 0; index < routes_.size (); ++index)
        {
            const Route& route = routes_[index];
            if (route.epoch == 0)
            {
                continue;
            }

            MatrixStatus status =
                record (tick, HostId{0}, DeviceId{static_cast<uint16_t> (index)},
                        AuditAction::StateRestored, route.state, route.epoch);
            if (status != MatrixStatus::Ok)
            {
                return status;
            }
        }

        return MatrixStatus::Ok;
    }

    MatrixStatus MatrixController::startAttach (uint64_t tick, HostId host,
                                                DeviceId device, Route& route) noexcept
    {
        route.state    = RouteState::Attaching;
        route.deadline = tick + config_.attachTimeoutTicks;

        return record (tick, host, device, AuditAction::AttachRequested, route.state,
                       route.epoch);
    }

    MatrixStatus MatrixController::record (uint64_t tick, HostId host, DeviceId device,
                                           AuditAction action, RouteState state,
                                           uint64_t epoch) noexcept
    {
        return audit_.append (tick, epoch, host, device, action, state);
    }

    bool MatrixController::valid (HostId host, DeviceId device) const noexcept
    {
        return host.value != 0 && device.value < maximumDevices;
    }

    bool MatrixController::hostInUse (HostId host, DeviceId except) const noexcept
    {
        for (std::size_t index = 0; index < routes_.size (); ++index)
        {
            if (index == except.value)
            {
                continue;
            }

            const Route& route = routes_[index];
            if ((route.hasActiveHost && route.activeHost.value == host.value) ||
                (route.hasPendingHost && route.pendingHost.value == host.value))
            {
                return true;
            }
        }

        return false;
    }

    bool MatrixController::auditSpace (std::size_t entries) const noexcept
    {
        return audit_.free () >= entries;
    }

    bool MatrixController::epochReady (const Route& route) const noexcept
    {
        return route.epoch != UINT64_MAX;
    }

    const char* matrixStatusName (MatrixStatus status) noexcept
    {
        switch (status)
        {
            case MatrixStatus::Ok: return "ok";
            case MatrixStatus::InvalidId: return "invalid-id";
            case MatrixStatus::StaleEpoch: return "stale-epoch";
            case MatrixStatus::WrongState: return "wrong-state";
            case MatrixStatus::CapacityExceeded: return "capacity-exceeded";
            case MatrixStatus::EpochExhausted: return "epoch-exhausted";
            case MatrixStatus::PersistenceError: return "persistence-error";
            case MatrixStatus::AuditFailure: return "audit-failure";
        }

        return "unknown";
    }

    const char* routeStateName (RouteState state) noexcept
    {
        switch (state)
        {
            case RouteState::Unassigned: return "unassigned";
            case RouteState::Detaching: return "detaching";
            case RouteState::Attaching: return "attaching";
            case RouteState::Active: return "active";
            case RouteState::Fault: return "fault";
        }

        return "unknown";
    }
} // namespace adk::usbmatrix
