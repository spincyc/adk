#pragma once

#include <cstddef>
#include <cstdint>

namespace adk::usbmesh {

    struct ActionId
    {
        uint64_t value;
    };

    struct DeviceId
    {
        uint64_t value;
    };

    struct SourceId
    {
        uint64_t value;
    };

    struct SourceSlotId
    {
        uint64_t value;
    };

    struct DestinationId
    {
        uint64_t value;
    };

    struct DestinationSlotId
    {
        uint64_t value;
    };

    struct RouteFence
    {
        uint64_t term;
        uint64_t epoch;
    };

    struct RouteIdentity
    {
        DeviceId          device;
        SourceId          source;
        SourceSlotId      sourceSlot;
        DestinationId     destination;
        DestinationSlotId destinationSlot;
    };

    enum struct ActionKind : uint8_t
    {
        Detach,
        Attach
    };

    enum struct AdapterPhase : uint8_t
    {
        Detached,
        PlannedDetach,
        PlannedAttach,
        Active,
        Fault
    };

    enum struct AdapterStatus : uint8_t
    {
        Ok,
        NotInitialized,
        InvalidAction,
        ActionConflict,
        CapacityExhausted,
        StaleFence,
        WrongPhase,
        PersistenceFailure,
        OperationFailure,
        VerificationFailure,
        RollbackFailure,
        RecoveryFailure
    };

    enum struct LoadStatus : uint8_t
    {
        Ok,
        Empty,
        Failure
    };

    struct RouteAction
    {
        ActionId      id;
        ActionKind    kind;
        RouteIdentity route;
        RouteFence    fence;
    };

    struct SlotSnapshot
    {
        RouteAction  action;
        ActionId     completedAction;
        AdapterPhase phase;
        bool         occupied;
        bool         hasAction;
        bool         hasCompletedAction;
    };

    constexpr size_t AdapterSlotCapacity = 16;

    struct AdapterSnapshot
    {
        SlotSnapshot slots[AdapterSlotCapacity];
        uint64_t     actionHighWater;
    };

    struct ActionStore
    {
        virtual ~ActionStore () noexcept = default;

        virtual LoadStatus load (AdapterSnapshot& snapshot) noexcept       = 0;
        virtual bool       save (const AdapterSnapshot& snapshot) noexcept = 0;
    };

    struct UsbActionBridge
    {
        virtual ~UsbActionBridge () noexcept = default;

        virtual bool detach   (const RouteIdentity& route,
                               RouteFence           fence) noexcept = 0;
        virtual bool detached (const RouteIdentity& route,
                               RouteFence           fence) noexcept = 0;
        virtual bool attach   (const RouteIdentity& route,
                               RouteFence           fence) noexcept = 0;
        virtual bool attached (const RouteIdentity& route,
                               RouteFence           fence) noexcept = 0;
    };

    struct UsbActionAdapter
    {
        UsbActionAdapter (ActionStore& store, UsbActionBridge& bridge) noexcept;

        AdapterStatus initialize () noexcept;
        AdapterStatus plan       (const RouteAction& action) noexcept;
        AdapterStatus apply      (ActionId action) noexcept;

        const AdapterSnapshot& snapshot () const noexcept;
        const SlotSnapshot*    snapshot (DestinationId     destination,
                                         DestinationSlotId destinationSlot) const noexcept;

      private:
        AdapterStatus recover     () noexcept;
        AdapterStatus recoverSlot (size_t index) noexcept;
        AdapterStatus applyDetach (size_t index) noexcept;
        AdapterStatus applyAttach (size_t index) noexcept;
        AdapterStatus enterFault  (size_t index) noexcept;

        bool   valid        (const RouteAction& action) const noexcept;
        bool   valid        (const AdapterSnapshot& snapshot) const noexcept;
        bool   same         (const RouteAction& left,
                             const RouteAction& right) const noexcept;
        bool   sameRoute    (const RouteIdentity& left,
                             const RouteIdentity& right) const noexcept;
        bool   fencePermits (const SlotSnapshot& slot,
                             const RouteAction&  action) const noexcept;
        bool   persist      (const AdapterSnapshot& candidate) noexcept;
        void   complete     (SlotSnapshot& candidate) const noexcept;
        size_t findAction   (ActionId action) const noexcept;
        size_t findSlot     (DestinationId     destination,
                             DestinationSlotId destinationSlot) const noexcept;
        size_t freeSlot     () const noexcept;

        ActionStore&     store_;
        UsbActionBridge& bridge_;
        AdapterSnapshot  snapshot_;
        bool             initialized_;
    };
} // namespace adk::usbmesh
