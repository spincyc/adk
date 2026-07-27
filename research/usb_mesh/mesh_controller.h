#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

namespace adk::usbmesh {

    struct NodeId
    {
        uint16_t value;
    };

    struct DeviceId
    {
        uint16_t value;
    };

    struct SlotId
    {
        uint16_t value;
    };

    struct EndpointIdentity
    {
        NodeId   node;
        uint64_t incarnation;
        uint64_t revision;
    };

    struct DeviceIdentity
    {
        DeviceId         device;
        EndpointIdentity source;
    };

    struct SlotIdentity
    {
        SlotId           slot;
        EndpointIdentity destination;
    };

    enum struct MeshStatus : uint8_t
    {
        Ok,
        InvalidIdentity,
        NotEnrolled,
        StaleIncarnation,
        StaleRevision,
        StaleEpoch,
        WrongState,
        SlotBusy,
        CapacityExceeded,
        InvalidConfiguration,
        EpochExhausted,
        PersistenceError
    };

    enum struct RouteState : uint8_t
    {
        Unassigned,
        Detaching,
        Attaching,
        Active,
        Fault
    };

    struct MeshConfig
    {
        uint64_t detachTimeoutTicks = 5000;
        uint64_t attachTimeoutTicks = 10000;
    };

    struct RouteSnapshot
    {
        DeviceIdentity device;
        SlotIdentity   activeSlot;
        SlotIdentity   pendingSlot;
        uint64_t       epoch;
        uint64_t       deadline;
        RouteState     state;
        bool           hasActiveSlot;
        bool           hasPendingSlot;
    };

    struct MeshController
    {
        static constexpr std::size_t maximumNodes   = 16;
        static constexpr std::size_t maximumDevices = 64;
        static constexpr std::size_t maximumSlots   = 64;

        explicit MeshController (
            const MeshConfig& config = MeshConfig{}) noexcept;

        MeshStatus enrollSource      (EndpointIdentity source) noexcept;
        MeshStatus enrollDestination (EndpointIdentity destination) noexcept;
        MeshStatus observeDevice     (DeviceIdentity device) noexcept;
        MeshStatus observeSlot       (SlotIdentity slot) noexcept;

        MeshStatus requestRoute    (uint64_t tick, DeviceIdentity device,
                                    SlotIdentity slot) noexcept;
        MeshStatus requestRelease  (uint64_t tick, DeviceIdentity device) noexcept;
        MeshStatus confirmDetached (uint64_t tick, DeviceIdentity device,
                                    uint64_t epoch) noexcept;
        MeshStatus confirmAttached (uint64_t tick, DeviceIdentity device,
                                    SlotIdentity slot, uint64_t epoch) noexcept;
        MeshStatus reportFault     (uint64_t tick, DeviceIdentity device,
                                    uint64_t epoch) noexcept;
        MeshStatus clearFault      (DeviceIdentity device) noexcept;
        MeshStatus update          (uint64_t tick) noexcept;

        RouteSnapshot route (DeviceId device) const noexcept;

        MeshStatus save (std::ostream& output) const noexcept;
        MeshStatus load (std::istream& input) noexcept;

      private:
        struct NodeRecord
        {
            EndpointIdentity identity;
            bool             enrolled;
        };

        struct DeviceRecord
        {
            DeviceIdentity identity;
            RouteSnapshot  route;
            bool           observed;
        };

        struct SlotRecord
        {
            SlotIdentity identity;
            bool         observed;
        };

        MeshStatus validateDevice         (DeviceIdentity device) const noexcept;
        MeshStatus validateSlot           (SlotIdentity slot) const noexcept;
        MeshStatus validateConfig         () const noexcept;
        MeshStatus advanceEpoch           (RouteSnapshot& route) const noexcept;
        MeshStatus fenceSourceRoutes      (EndpointIdentity source) noexcept;
        MeshStatus fenceDestinationRoutes (EndpointIdentity destination) noexcept;
        void       faultRoute             (RouteSnapshot& route) noexcept;
        bool       routeUsesSource        (const RouteSnapshot& route,
                                           NodeId source) const noexcept;
        bool       routeUsesDestination   (const RouteSnapshot& route,
                                           NodeId destination) const noexcept;
        MeshStatus startAttach            (uint64_t tick, DeviceRecord& device,
                                           SlotIdentity slot) noexcept;
        bool       slotInUse              (SlotIdentity slot,
                                           DeviceId except) const noexcept;
        bool       sameEndpoint           (EndpointIdentity left,
                                           EndpointIdentity right) const noexcept;
        bool       sameSlot               (SlotIdentity left,
                                           SlotIdentity right) const noexcept;
        bool       expired                (uint64_t tick,
                                           uint64_t deadline) const noexcept;

        MeshConfig                              config_;
        std::array<NodeRecord, maximumNodes>    sources_;
        std::array<NodeRecord, maximumNodes>    destinations_;
        std::array<DeviceRecord, maximumDevices> devices_;
        std::array<SlotRecord, maximumSlots>     slots_;
        bool                                     configValid_;
    };

    const char* meshStatusName (MeshStatus status) noexcept;
    const char* routeStateName (RouteState state) noexcept;
} // namespace adk::usbmesh
