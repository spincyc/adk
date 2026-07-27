#include "usb_action_adapter.h"

#include <cstdlib>
#include <iostream>

namespace {

    using adk::usbmesh::ActionId;
    using adk::usbmesh::ActionKind;
    using adk::usbmesh::ActionStore;
    using adk::usbmesh::AdapterPhase;
    using adk::usbmesh::AdapterSnapshot;
    using adk::usbmesh::AdapterStatus;
    using adk::usbmesh::DestinationId;
    using adk::usbmesh::DestinationSlotId;
    using adk::usbmesh::DeviceId;
    using adk::usbmesh::LoadStatus;
    using adk::usbmesh::RouteAction;
    using adk::usbmesh::RouteFence;
    using adk::usbmesh::RouteIdentity;
    using adk::usbmesh::SlotSnapshot;
    using adk::usbmesh::SourceId;
    using adk::usbmesh::SourceSlotId;
    using adk::usbmesh::UsbActionAdapter;
    using adk::usbmesh::UsbActionBridge;

    struct FakeStore final : ActionStore
    {
        LoadStatus load (AdapterSnapshot& value) noexcept override
        {
            ++loadCount;
            if (loadFails)
            {
                return LoadStatus::Failure;
            }
            if (!hasSaved)
            {
                return LoadStatus::Empty;
            }
            value = saved;
            return LoadStatus::Ok;
        }

        bool save (const AdapterSnapshot& value) noexcept override
        {
            ++saveCount;
            if (failSaveCount != 0)
            {
                --failSaveCount;
                return false;
            }
            saved    = value;
            hasSaved = true;
            return true;
        }

        AdapterSnapshot saved{};
        unsigned int    loadCount     = 0;
        unsigned int    saveCount     = 0;
        unsigned int    failSaveCount = 0;
        bool            hasSaved      = false;
        bool            loadFails     = false;
    };

    struct FakeBridge final : UsbActionBridge
    {
        bool detach (const RouteIdentity& route, RouteFence) noexcept override
        {
            ++detachCount;
            if (!detachSucceeds)
            {
                return false;
            }
            attachedState[index (route)] = false;
            return true;
        }

        bool detached (const RouteIdentity& route, RouteFence) noexcept override
        {
            ++detachCheckCount;
            return detachVerified && !attachedState[index (route)];
        }

        bool attach (const RouteIdentity& route, RouteFence) noexcept override
        {
            ++attachCount;
            if (!attachSucceeds)
            {
                return false;
            }
            attachedState[index (route)] = true;
            return true;
        }

        bool attached (const RouteIdentity& route, RouteFence) noexcept override
        {
            ++attachCheckCount;
            return attachVerified && attachedState[index (route)];
        }

        static size_t index (const RouteIdentity& route) noexcept
        {
            return static_cast<size_t> (route.destinationSlot.value % 32);
        }

        bool         attachedState[32]{};
        unsigned int detachCount      = 0;
        unsigned int detachCheckCount = 0;
        unsigned int attachCount      = 0;
        unsigned int attachCheckCount = 0;
        bool         detachSucceeds   = true;
        bool         detachVerified   = true;
        bool         attachSucceeds   = true;
        bool         attachVerified   = true;
    };

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    void requireStatus (AdapterStatus actual, AdapterStatus expected,
                        const char* message)
    {
        require (actual == expected, message);
    }

    RouteAction action (uint64_t id, ActionKind kind, uint64_t epoch,
                        uint64_t destinationSlot = 5, uint64_t device = 1,
                        uint64_t source = 2, uint64_t sourceSlot = 3,
                        uint64_t destination = 4, uint64_t term = 1)
    {
        return RouteAction{ActionId{id}, kind,
                           RouteIdentity{DeviceId{device}, SourceId{source},
                                         SourceSlotId{sourceSlot},
                                         DestinationId{destination},
                                         DestinationSlotId{destinationSlot}},
                           RouteFence{term, epoch}};
    }

    const SlotSnapshot& slot (const UsbActionAdapter& adapter,
                              uint64_t                destinationSlot = 5)
    {
        const SlotSnapshot* found =
            adapter.snapshot (DestinationId{4}, DestinationSlotId{destinationSlot});

        require (found != nullptr, "destination slot exists");
        return *found;
    }

    void initialize (UsbActionAdapter& adapter)
    {
        requireStatus (adapter.initialize (), AdapterStatus::Ok, "initialize adapter");
    }

    void establishDetached (UsbActionAdapter& adapter, uint64_t id, uint64_t epoch,
                            uint64_t destinationSlot = 5)
    {
        const RouteAction detach =
            action (id, ActionKind::Detach, epoch, destinationSlot);

        requireStatus (adapter.plan (detach), AdapterStatus::Ok, "plan detach");

        requireStatus (adapter.apply (detach.id), AdapterStatus::Ok, "apply detach");
    }

    void establishActive (UsbActionAdapter& adapter, uint64_t detachId,
                          uint64_t attachId, uint64_t epoch,
                          uint64_t destinationSlot = 5)
    {
        establishDetached (adapter, detachId, epoch, destinationSlot);
        const RouteAction attach =
            action (attachId, ActionKind::Attach, epoch, destinationSlot);

        requireStatus (adapter.plan (attach), AdapterStatus::Ok, "plan attach");

        requireStatus (adapter.apply (attach.id), AdapterStatus::Ok, "apply attach");
    }

    void testInitializationAndDurablePlanning ()
    {
        FakeStore        store;
        FakeBridge       bridge;
        UsbActionAdapter adapter (store, bridge);


        requireStatus (adapter.plan (action (1, ActionKind::Detach, 1)),
                       AdapterStatus::NotInitialized, "planning requires durable load");

        initialize (adapter);

        store.failSaveCount = 1;

        requireStatus (adapter.plan (action (1, ActionKind::Detach, 1)),
                       AdapterStatus::PersistenceFailure, "failed plan save closes");

        require (bridge.detachCount == 0, "save precedes bridge operation");

        require (adapter.snapshot ().actionHighWater == 0,
                 "failed save does not consume action id");
    }

    void testIndependentDestinationSlotsInterleave ()
    {
        FakeStore        store;
        FakeBridge       bridge;
        UsbActionAdapter adapter (store, bridge);

        initialize (adapter);

        const RouteAction detachFive = action (1, ActionKind::Detach, 1, 5);
        const RouteAction detachSix  = action (2, ActionKind::Detach, 1, 6);

        requireStatus (adapter.plan (detachFive), AdapterStatus::Ok, "plan slot five");

        requireStatus (adapter.plan (detachSix), AdapterStatus::Ok, "plan slot six");

        requireStatus (adapter.apply (detachSix.id), AdapterStatus::Ok,
                       "apply slot six");

        requireStatus (adapter.apply (detachFive.id), AdapterStatus::Ok,
                       "apply slot five");

        const RouteAction attachFive = action (3, ActionKind::Attach, 1, 5);
        const RouteAction attachSix  = action (4, ActionKind::Attach, 1, 6);

        requireStatus (adapter.plan (attachFive), AdapterStatus::Ok,
                       "attach slot five");

        requireStatus (adapter.plan (attachSix), AdapterStatus::Ok, "attach slot six");

        requireStatus (adapter.apply (attachFive.id), AdapterStatus::Ok,
                       "activate five");

        requireStatus (adapter.apply (attachSix.id), AdapterStatus::Ok, "activate six");

        require (slot (adapter, 5).phase == AdapterPhase::Active,
                 "slot five independently active");

        require (slot (adapter, 6).phase == AdapterPhase::Active,
                 "slot six independently active");
    }

    void testCrossSlotAndCrossRouteDetachRejected ()
    {
        FakeStore        store;
        FakeBridge       bridge;
        UsbActionAdapter adapter (store, bridge);

        initialize (adapter);

        establishActive (adapter, 1, 2, 1);


        requireStatus (adapter.plan (action (3, ActionKind::Detach, 2, 5, 99)),
                       AdapterStatus::ActionConflict,
                       "different device cannot detach occupied slot");

        requireStatus (adapter.plan (action (4, ActionKind::Detach, 2, 6, 1)),
                       AdapterStatus::Ok, "another slot has independent detach");

        require (slot (adapter).phase == AdapterPhase::Active,
                 "other slot plan does not disturb active route");
    }

    void testRestartRecoversPlannedDetach ()
    {
        FakeStore  store;
        FakeBridge bridge;
        {
            UsbActionAdapter adapter (store, bridge);

            initialize (adapter);

            const RouteAction detach = action (1, ActionKind::Detach, 1);

            requireStatus (adapter.plan (detach), AdapterStatus::Ok,
                           "persist detach plan");
        }

        UsbActionAdapter restarted (store, bridge);

        initialize (restarted);

        require (slot (restarted).phase == AdapterPhase::Detached,
                 "restart completes planned detach");

        require (!slot (restarted).hasAction, "restart completes action record");

        require (bridge.detachCount == 1, "restart executes safe detach");
    }

    void testRestartRecoversUncertainAttachByDetaching ()
    {
        FakeStore  store;
        FakeBridge bridge;
        {
            UsbActionAdapter adapter (store, bridge);

            initialize (adapter);

            establishDetached (adapter, 1, 1);

            const RouteAction attach = action (2, ActionKind::Attach, 1);

            requireStatus (adapter.plan (attach), AdapterStatus::Ok,
                           "persist uncertain attach");
            bridge.attachedState[5] = true;
        }

        UsbActionAdapter restarted (store, bridge);

        initialize (restarted);

        require (slot (restarted).phase == AdapterPhase::PlannedAttach,
                 "uncertain attach remains planned");

        require (slot (restarted).hasAction, "uncertain attach remains retryable");

        require (!bridge.attachedState[5], "recovery removes uncertain attachment");

        requireStatus (restarted.apply (ActionId{2}), AdapterStatus::Ok,
                       "controller may retry recovered attach");
    }

    void testRestartValidatesActiveRoute ()
    {
        FakeStore  store;
        FakeBridge bridge;
        {
            UsbActionAdapter adapter (store, bridge);

            initialize (adapter);

            establishActive (adapter, 1, 2, 1);
        }

        UsbActionAdapter healthy (store, bridge);

        initialize (healthy);

        require (slot (healthy).phase == AdapterPhase::Active,
                 "observed active route survives restart");

        bridge.attachedState[5] = false;
        UsbActionAdapter missing (store, bridge);

        requireStatus (missing.initialize (), AdapterStatus::RecoveryFailure,
                       "missing active route enters durable fault");

        require (slot (missing).phase == AdapterPhase::Fault,
                 "missing route fault is in memory");

        require (store.saved.slots[0].phase == AdapterPhase::Fault,
                 "missing route fault is durable");
    }

    void testAttachCommitRollbackFailureIsDurable ()
    {
        FakeStore        store;
        FakeBridge       bridge;
        UsbActionAdapter adapter (store, bridge);

        initialize (adapter);

        establishDetached (adapter, 1, 1);

        const RouteAction attach = action (2, ActionKind::Attach, 1);

        requireStatus (adapter.plan (attach), AdapterStatus::Ok, "plan attach");
        store.failSaveCount   = 1;
        bridge.detachSucceeds = false;

        requireStatus (adapter.apply (attach.id), AdapterStatus::RollbackFailure,
                       "failed rollback latches fault");

        require (slot (adapter).phase == AdapterPhase::Fault,
                 "rollback fault is in memory");

        require (store.saved.slots[0].phase == AdapterPhase::Fault,
                 "rollback fault is persisted");

        requireStatus (adapter.plan (action (3, ActionKind::Detach, 2)),
                       AdapterStatus::WrongPhase,
                       "live fault blocks new planning");

        UsbActionAdapter restarted (store, bridge);

        requireStatus (restarted.initialize (), AdapterStatus::RecoveryFailure,
                       "durable fault blocks restart");

        requireStatus (restarted.plan (action (3, ActionKind::Detach, 2)),
                       AdapterStatus::NotInitialized,
                       "failed recovery blocks every new plan");
    }

    void testAttachVerificationRollbackFailureIsDurable ()
    {
        FakeStore        store;
        FakeBridge       bridge;
        UsbActionAdapter adapter (store, bridge);

        initialize (adapter);

        establishDetached (adapter, 1, 1);

        const RouteAction attach = action (2, ActionKind::Attach, 1);

        requireStatus (adapter.plan (attach), AdapterStatus::Ok, "plan attach");
        bridge.attachVerified = false;
        bridge.detachSucceeds = false;

        requireStatus (adapter.apply (attach.id), AdapterStatus::RollbackFailure,
                       "verification rollback failure reported");

        require (store.saved.slots[0].phase == AdapterPhase::Fault,
                 "verification rollback fault persisted");
    }

    void testActionsRemainImmutableAcrossRestart ()
    {
        FakeStore  store;
        FakeBridge bridge;
        {
            UsbActionAdapter adapter (store, bridge);

            initialize (adapter);

            establishDetached (adapter, 10, 1);
        }

        UsbActionAdapter restarted (store, bridge);

        initialize (restarted);

        requireStatus (restarted.apply (ActionId{10}), AdapterStatus::Ok,
                       "completed action replay is idempotent");

        requireStatus (restarted.plan (action (10, ActionKind::Detach, 2)),
                       AdapterStatus::ActionConflict, "completed id cannot be reused");

        requireStatus (restarted.plan (action (9, ActionKind::Detach, 2)),
                       AdapterStatus::ActionConflict, "older id cannot be reused");
    }

    void testMalformedAndUnavailableStoresFailClosed ()
    {
        FakeStore  store;
        FakeBridge bridge;
        store.loadFails = true;
        UsbActionAdapter unavailable (store, bridge);

        requireStatus (unavailable.initialize (), AdapterStatus::PersistenceFailure,
                       "unavailable store blocks initialization");

        store.loadFails                = false;
        store.hasSaved                 = true;
        store.saved.slots[0].occupied  = true;
        store.saved.slots[0].hasAction = true;
        store.saved.slots[0].phase     = AdapterPhase::Active;
        UsbActionAdapter malformed (store, bridge);

        requireStatus (malformed.initialize (), AdapterStatus::RecoveryFailure,
                       "malformed durable state rejected");

        require (bridge.attachCount == 0 && bridge.detachCount == 0,
                 "malformed state has no bridge effect");
    }
} // namespace

int main ()
{
    testInitializationAndDurablePlanning           ();
    testIndependentDestinationSlotsInterleave      ();
    testCrossSlotAndCrossRouteDetachRejected       ();
    testRestartRecoversPlannedDetach               ();
    testRestartRecoversUncertainAttachByDetaching  ();
    testRestartValidatesActiveRoute                ();
    testAttachCommitRollbackFailureIsDurable       ();
    testAttachVerificationRollbackFailureIsDurable ();
    testActionsRemainImmutableAcrossRestart        ();
    testMalformedAndUnavailableStoresFailClosed    ();

    std::cout << "USB mesh action adapter tests passed.\n";
    return 0;
}
