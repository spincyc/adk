#include "mesh_controller.h"

#include <limits>
#include <sstream>
#include <string>

namespace adk::usbmesh {

    namespace {

        constexpr uint32_t persistenceVersion = 1;

        bool validNode (NodeId node) noexcept
        {
            return node.value > 0 && node.value <= MeshController::maximumNodes;
        }

        bool validDevice (DeviceId device) noexcept
        {
            return device.value > 0 && device.value <= MeshController::maximumDevices;
        }

        bool validSlot (SlotId slot) noexcept
        {
            return slot.value > 0 && slot.value <= MeshController::maximumSlots;
        }

        bool validEndpoint (EndpointIdentity endpoint) noexcept
        {
            return validNode (endpoint.node) && endpoint.incarnation > 0 &&
                   endpoint.revision > 0;
        }
    } // namespace

    MeshController::MeshController (const MeshConfig& config) noexcept
        : config_       (config),
          sources_      {},
          destinations_ {},
          devices_      {},
          slots_        {},
          configValid_  (config.detachTimeoutTicks > 0 &&
                         config.attachTimeoutTicks > 0 &&
                         config.detachTimeoutTicks <=
                             static_cast<uint64_t> (
                                 std::numeric_limits<int64_t>::max ()) &&
                         config.attachTimeoutTicks <=
                             static_cast<uint64_t> (
                                 std::numeric_limits<int64_t>::max ()))
    {
        for (std::size_t index = 0; index < maximumDevices; ++index)
        {
            devices_[index].route.state = RouteState::Unassigned;
        }
    }

    MeshStatus MeshController::enrollSource (EndpointIdentity source) noexcept
    {
        MeshStatus status = validateConfig ();

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        if (!validEndpoint (source))
        {
            return MeshStatus::InvalidIdentity;
        }

        NodeRecord& record = sources_[source.node.value - 1];

        if (record.enrolled && source.incarnation < record.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (record.enrolled && source.incarnation == record.identity.incarnation &&
            source.revision < record.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        if (record.enrolled && !sameEndpoint (record.identity, source))
        {
            status = fenceSourceRoutes (source);

            if (status != MeshStatus::Ok)
            {
                return status;
            }
        }

        record.identity = source;
        record.enrolled = true;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::enrollDestination (
        EndpointIdentity destination) noexcept
    {
        MeshStatus status = validateConfig ();

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        if (!validEndpoint (destination))
        {
            return MeshStatus::InvalidIdentity;
        }

        NodeRecord& record = destinations_[destination.node.value - 1];

        if (record.enrolled && destination.incarnation < record.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (record.enrolled &&
            destination.incarnation == record.identity.incarnation &&
            destination.revision < record.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        if (record.enrolled && !sameEndpoint (record.identity, destination))
        {
            status = fenceDestinationRoutes (destination);

            if (status != MeshStatus::Ok)
            {
                return status;
            }
        }

        record.identity = destination;
        record.enrolled = true;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::observeDevice (DeviceIdentity device) noexcept
    {
        if (!validDevice (device.device) || !validEndpoint (device.source))
        {
            return MeshStatus::InvalidIdentity;
        }

        const NodeRecord& source = sources_[device.source.node.value - 1];

        if (!source.enrolled)
        {
            return MeshStatus::NotEnrolled;
        }

        if (device.source.incarnation != source.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (device.source.revision != source.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        DeviceRecord& record = devices_[device.device.value - 1];

        if (record.observed &&
            record.identity.source.node.value != device.source.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (record.observed && record.route.state != RouteState::Unassigned &&
            record.route.state != RouteState::Fault &&
            !sameEndpoint (record.identity.source, device.source))
        {
            return MeshStatus::WrongState;
        }

        record.identity           = device;
        record.route.device       = device;
        record.observed           = true;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::observeSlot (SlotIdentity slot) noexcept
    {
        if (!validSlot (slot.slot) || !validEndpoint (slot.destination))
        {
            return MeshStatus::InvalidIdentity;
        }

        const NodeRecord& destination =
            destinations_[slot.destination.node.value - 1];

        if (!destination.enrolled)
        {
            return MeshStatus::NotEnrolled;
        }

        if (slot.destination.incarnation != destination.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (slot.destination.revision != destination.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        SlotRecord& record = slots_[slot.slot.value - 1];

        if (record.observed &&
            record.identity.destination.node.value != slot.destination.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (record.observed && !sameSlot (record.identity, slot) &&
            slotInUse (record.identity, DeviceId {0}))
        {
            return MeshStatus::WrongState;
        }

        record.identity = slot;
        record.observed = true;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::requestRoute (uint64_t tick,
                                             DeviceIdentity device,
                                             SlotIdentity slot) noexcept
    {
        MeshStatus status = validateConfig ();

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        status = validateDevice (device);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        status = validateSlot (slot);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        DeviceRecord& record = devices_[device.device.value - 1];
        RouteSnapshot& route = record.route;

        if (route.state == RouteState::Active && route.hasActiveSlot &&
            sameSlot (route.activeSlot, slot))
        {
            return MeshStatus::Ok;
        }

        if (route.state != RouteState::Unassigned &&
            route.state != RouteState::Active)
        {
            return MeshStatus::WrongState;
        }

        if (slotInUse (slot, device.device))
        {
            return MeshStatus::SlotBusy;
        }

        status = advanceEpoch (route);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        route.pendingSlot    = slot;
        route.hasPendingSlot = true;

        if (route.state == RouteState::Active)
        {
            route.state    = RouteState::Detaching;
            route.deadline = tick + config_.detachTimeoutTicks;
            return MeshStatus::Ok;
        }

        return startAttach (tick, record, slot);
    }

    MeshStatus MeshController::requestRelease (uint64_t tick,
                                               DeviceIdentity device) noexcept
    {
        MeshStatus status = validateDevice (device);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        RouteSnapshot& route = devices_[device.device.value - 1].route;

        if (route.state == RouteState::Unassigned)
        {
            return MeshStatus::Ok;
        }

        if (route.state != RouteState::Active)
        {
            return MeshStatus::WrongState;
        }

        status = advanceEpoch (route);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        route.state          = RouteState::Detaching;
        route.deadline       = tick + config_.detachTimeoutTicks;
        route.hasPendingSlot = false;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::confirmDetached (uint64_t tick,
                                                DeviceIdentity device,
                                                uint64_t epoch) noexcept
    {
        MeshStatus status = validateDevice (device);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        DeviceRecord& record = devices_[device.device.value - 1];
        RouteSnapshot& route = record.route;

        if (epoch != route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        if (route.state != RouteState::Detaching)
        {
            return MeshStatus::WrongState;
        }

        route.hasActiveSlot = false;

        if (!route.hasPendingSlot)
        {
            route.state    = RouteState::Unassigned;
            route.deadline = 0;
            return MeshStatus::Ok;
        }

        return startAttach (tick, record, route.pendingSlot);
    }

    MeshStatus MeshController::confirmAttached (uint64_t,
                                                DeviceIdentity device,
                                                SlotIdentity slot,
                                                uint64_t epoch) noexcept
    {
        MeshStatus status = validateDevice (device);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        status = validateSlot (slot);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        RouteSnapshot& route = devices_[device.device.value - 1].route;

        if (epoch != route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        if (route.state != RouteState::Attaching || !route.hasPendingSlot ||
            !sameSlot (route.pendingSlot, slot))
        {
            return MeshStatus::WrongState;
        }

        route.activeSlot     = slot;
        route.hasActiveSlot  = true;
        route.hasPendingSlot = false;
        route.state          = RouteState::Active;
        route.deadline       = 0;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::reportFault (uint64_t,
                                            DeviceIdentity device,
                                            uint64_t epoch) noexcept
    {
        MeshStatus status = validateDevice (device);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        RouteSnapshot& route = devices_[device.device.value - 1].route;

        if (epoch != route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        route.state          = RouteState::Fault;
        route.hasActiveSlot  = false;
        route.hasPendingSlot = false;
        route.deadline       = 0;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::clearFault (DeviceIdentity device) noexcept
    {
        MeshStatus status = validateDevice (device);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        RouteSnapshot& route = devices_[device.device.value - 1].route;

        if (route.state != RouteState::Fault)
        {
            return MeshStatus::WrongState;
        }

        status = advanceEpoch (route);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        route.state    = RouteState::Unassigned;
        route.deadline = 0;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::update (uint64_t tick) noexcept
    {
        for (DeviceRecord& record : devices_)
        {
            RouteSnapshot& route = record.route;

            if ((route.state == RouteState::Attaching ||
                 route.state == RouteState::Detaching) &&
                expired (tick, route.deadline))
            {
                faultRoute (route);
            }
        }

        return MeshStatus::Ok;
    }

    RouteSnapshot MeshController::route (DeviceId device) const noexcept
    {
        if (!validDevice (device))
        {
            RouteSnapshot invalid {};
            invalid.state = RouteState::Fault;
            return invalid;
        }

        return devices_[device.value - 1].route;
    }

    MeshStatus MeshController::save (std::ostream& output) const noexcept
    {
        output << "ADK_USB_MESH " << persistenceVersion << '\n';

        for (const DeviceRecord& record : devices_)
        {
            output << record.route.epoch << '\n';
        }

        return output.good () ? MeshStatus::Ok : MeshStatus::PersistenceError;
    }

    MeshStatus MeshController::load (std::istream& input) noexcept
    {
        std::string header;
        uint32_t    version = 0;

        if (!(input >> header >> version) || header != "ADK_USB_MESH" ||
            version != persistenceVersion)
        {
            return MeshStatus::PersistenceError;
        }

        std::array<uint64_t, maximumDevices> epochs {};

        for (uint64_t& epoch : epochs)
        {
            if (!(input >> epoch) || epoch == std::numeric_limits<uint64_t>::max ())
            {
                return MeshStatus::PersistenceError;
            }
        }

        std::string trailing;

        if (input >> trailing)
        {
            return MeshStatus::PersistenceError;
        }

        for (std::size_t index = 0; index < maximumDevices; ++index)
        {
            devices_[index].route.epoch          = epochs[index] + 1;
            devices_[index].route.state          = RouteState::Fault;
            devices_[index].route.hasActiveSlot  = false;
            devices_[index].route.hasPendingSlot = false;
            devices_[index].route.deadline       = 0;
        }

        return MeshStatus::Ok;
    }

    MeshStatus MeshController::validateDevice (DeviceIdentity device) const noexcept
    {
        if (!validDevice (device.device) || !validEndpoint (device.source))
        {
            return MeshStatus::InvalidIdentity;
        }

        const DeviceRecord& record = devices_[device.device.value - 1];

        if (!record.observed)
        {
            return MeshStatus::NotEnrolled;
        }

        if (device.source.node.value != record.identity.source.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (device.source.incarnation != record.identity.source.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (device.source.revision != record.identity.source.revision)
        {
            return MeshStatus::StaleRevision;
        }

        return MeshStatus::Ok;
    }

    MeshStatus MeshController::validateConfig () const noexcept
    {
        return configValid_ ? MeshStatus::Ok : MeshStatus::InvalidConfiguration;
    }

    MeshStatus MeshController::validateSlot (SlotIdentity slot) const noexcept
    {
        if (!validSlot (slot.slot) || !validEndpoint (slot.destination))
        {
            return MeshStatus::InvalidIdentity;
        }

        const SlotRecord& record = slots_[slot.slot.value - 1];

        if (!record.observed)
        {
            return MeshStatus::NotEnrolled;
        }

        if (slot.destination.node.value !=
            record.identity.destination.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (slot.destination.incarnation != record.identity.destination.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (slot.destination.revision != record.identity.destination.revision)
        {
            return MeshStatus::StaleRevision;
        }

        return MeshStatus::Ok;
    }

    MeshStatus MeshController::advanceEpoch (RouteSnapshot& route) const noexcept
    {
        if (route.epoch == std::numeric_limits<uint64_t>::max ())
        {
            return MeshStatus::EpochExhausted;
        }

        ++route.epoch;
        return MeshStatus::Ok;
    }

    MeshStatus MeshController::fenceSourceRoutes (
        EndpointIdentity source) noexcept
    {
        for (const DeviceRecord& record : devices_)
        {
            if (routeUsesSource (record.route, source.node) &&
                record.route.epoch == std::numeric_limits<uint64_t>::max ())
            {
                return MeshStatus::EpochExhausted;
            }
        }

        for (DeviceRecord& record : devices_)
        {
            if (routeUsesSource (record.route, source.node))
            {
                ++record.route.epoch;
                faultRoute (record.route);
            }

            if (record.observed &&
                record.identity.source.node.value == source.node.value)
            {
                record.observed = false;
            }
        }

        return MeshStatus::Ok;
    }

    MeshStatus MeshController::fenceDestinationRoutes (
        EndpointIdentity destination) noexcept
    {
        for (const DeviceRecord& record : devices_)
        {
            if (routeUsesDestination (record.route, destination.node) &&
                record.route.epoch == std::numeric_limits<uint64_t>::max ())
            {
                return MeshStatus::EpochExhausted;
            }
        }

        for (DeviceRecord& record : devices_)
        {
            if (routeUsesDestination (record.route, destination.node))
            {
                ++record.route.epoch;
                faultRoute (record.route);
            }
        }

        for (SlotRecord& record : slots_)
        {
            if (record.observed &&
                record.identity.destination.node.value == destination.node.value)
            {
                record.observed = false;
            }
        }

        return MeshStatus::Ok;
    }

    void MeshController::faultRoute (RouteSnapshot& route) noexcept
    {
        route.state          = RouteState::Fault;
        route.hasActiveSlot  = false;
        route.hasPendingSlot = false;
        route.deadline       = 0;
    }

    bool MeshController::routeUsesSource (const RouteSnapshot& route,
                                          NodeId source) const noexcept
    {
        return route.state != RouteState::Unassigned &&
               route.device.source.node.value == source.value;
    }

    bool MeshController::routeUsesDestination (
        const RouteSnapshot& route, NodeId destination) const noexcept
    {
        return (route.hasActiveSlot &&
                route.activeSlot.destination.node.value == destination.value) ||
               (route.hasPendingSlot &&
                route.pendingSlot.destination.node.value == destination.value);
    }

    MeshStatus MeshController::startAttach (uint64_t tick,
                                            DeviceRecord& device,
                                            SlotIdentity slot) noexcept
    {
        device.route.pendingSlot    = slot;
        device.route.hasPendingSlot = true;
        device.route.state          = RouteState::Attaching;
        device.route.deadline       = tick + config_.attachTimeoutTicks;
        return MeshStatus::Ok;
    }

    bool MeshController::slotInUse (SlotIdentity slot, DeviceId except) const noexcept
    {
        for (const DeviceRecord& record : devices_)
        {
            if (record.identity.device.value == except.value)
            {
                continue;
            }

            if (record.route.hasActiveSlot &&
                sameSlot (record.route.activeSlot, slot))
            {
                return true;
            }

            if (record.route.hasPendingSlot &&
                sameSlot (record.route.pendingSlot, slot))
            {
                return true;
            }
        }

        return false;
    }

    bool MeshController::sameEndpoint (EndpointIdentity left,
                                       EndpointIdentity right) const noexcept
    {
        return left.node.value == right.node.value &&
               left.incarnation == right.incarnation &&
               left.revision == right.revision;
    }

    bool MeshController::sameSlot (SlotIdentity left,
                                   SlotIdentity right) const noexcept
    {
        return left.slot.value == right.slot.value &&
               sameEndpoint (left.destination, right.destination);
    }

    bool MeshController::expired (uint64_t tick, uint64_t deadline) const noexcept
    {
        return static_cast<int64_t> (tick - deadline) >= 0;
    }

    const char* meshStatusName (MeshStatus status) noexcept
    {
        switch (status)
        {
            case MeshStatus::Ok: return "ok";
            case MeshStatus::InvalidIdentity: return "invalid-identity";
            case MeshStatus::NotEnrolled: return "not-enrolled";
            case MeshStatus::StaleIncarnation: return "stale-incarnation";
            case MeshStatus::StaleRevision: return "stale-revision";
            case MeshStatus::StaleEpoch: return "stale-epoch";
            case MeshStatus::WrongState: return "wrong-state";
            case MeshStatus::SlotBusy: return "slot-busy";
            case MeshStatus::CapacityExceeded: return "capacity-exceeded";
            case MeshStatus::InvalidConfiguration: return "invalid-configuration";
            case MeshStatus::EpochExhausted: return "epoch-exhausted";
            case MeshStatus::PersistenceError: return "persistence-error";
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
} // namespace adk::usbmesh
