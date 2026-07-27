#include "usb_action_adapter.h"

namespace adk::usbmesh {

    namespace {

        constexpr size_t NoSlot = AdapterSlotCapacity;

        RouteAction emptyAction () noexcept
        {
            return RouteAction{ActionId{0}, ActionKind::Detach,
                               RouteIdentity{DeviceId{0}, SourceId{0}, SourceSlotId{0},
                                             DestinationId{0}, DestinationSlotId{0}},
                               RouteFence{0, 0}};
        }

        SlotSnapshot emptySlot () noexcept
        {
            return SlotSnapshot{emptyAction (), ActionId{0}, AdapterPhase::Detached,
                                false,          false,       false};
        }

        AdapterSnapshot emptySnapshot () noexcept
        {
            AdapterSnapshot value{};
            for (size_t index = 0; index < AdapterSlotCapacity; ++index)
            {
                value.slots[index] = emptySlot ();
            }
            value.actionHighWater = 0;
            return value;
        }
    } // namespace

    UsbActionAdapter::UsbActionAdapter (ActionStore&     store,
                                        UsbActionBridge& bridge) noexcept
        : store_ (store), bridge_ (bridge), snapshot_ (emptySnapshot ()),
          initialized_ (false)
    {
    }

    AdapterStatus UsbActionAdapter::initialize () noexcept
    {
        if (initialized_)
        {
            return AdapterStatus::Ok;
        }

        AdapterSnapshot  loaded = emptySnapshot ();

        const LoadStatus status = store_.load (loaded);
        if (status == LoadStatus::Failure)
        {
            return AdapterStatus::PersistenceFailure;
        }

        if (status == LoadStatus::Ok && !valid (loaded))
        {
            return AdapterStatus::RecoveryFailure;
        }

        snapshot_    = loaded;
        initialized_ = true;
        const AdapterStatus recovery = recover ();
        if (recovery != AdapterStatus::Ok)
        {
            initialized_ = false;
        }
        return recovery;
    }

    AdapterStatus UsbActionAdapter::plan (const RouteAction& action) noexcept
    {
        if (!initialized_)
        {
            return AdapterStatus::NotInitialized;
        }

        if (!valid (action))
        {
            return AdapterStatus::InvalidAction;
        }

        const size_t actionIndex = findAction (action.id);
        if (actionIndex != NoSlot)
        {
            const SlotSnapshot& existing = snapshot_.slots[actionIndex];
            if (existing.hasAction)
            {
                return same (existing.action, action) ? AdapterStatus::Ok
                                                      : AdapterStatus::ActionConflict;
            }
            return AdapterStatus::ActionConflict;
        }

        if (action.id.value <= snapshot_.actionHighWater)
        {
            return AdapterStatus::ActionConflict;
        }

        size_t index =
            findSlot (action.route.destination, action.route.destinationSlot);
        if (index == NoSlot)
        {
            if (action.kind != ActionKind::Detach)
            {
                return AdapterStatus::StaleFence;
            }
            index = freeSlot ();
            if (index == NoSlot)
            {
                return AdapterStatus::CapacityExhausted;
            }
        }

        const SlotSnapshot& current = snapshot_.slots[index];
        if (current.phase == AdapterPhase::Fault || current.hasAction)
        {
            return AdapterStatus::WrongPhase;
        }

        if (action.kind == ActionKind::Detach &&
            current.phase == AdapterPhase::Active &&
            !sameRoute (current.action.route, action.route))
        {
            return AdapterStatus::ActionConflict;
        }

        if (!fencePermits (current, action))
        {
            return AdapterStatus::StaleFence;
        }

        AdapterSnapshot candidate = snapshot_;
        SlotSnapshot&   slot      = candidate.slots[index];
        slot.occupied             = true;
        slot.action               = action;
        slot.hasAction            = true;
        slot.phase = action.kind == ActionKind::Detach ? AdapterPhase::PlannedDetach
                                                       : AdapterPhase::PlannedAttach;
        candidate.actionHighWater = action.id.value;

        return persist (candidate) ? AdapterStatus::Ok
                                   : AdapterStatus::PersistenceFailure;
    }

    AdapterStatus UsbActionAdapter::apply (ActionId action) noexcept
    {
        if (!initialized_)
        {
            return AdapterStatus::NotInitialized;
        }

        const size_t index = findAction (action);
        if (index == NoSlot)
        {
            return AdapterStatus::InvalidAction;
        }

        const SlotSnapshot& slot = snapshot_.slots[index];
        if (!slot.hasAction)
        {
            return slot.hasCompletedAction && slot.completedAction.value == action.value
                       ? AdapterStatus::Ok
                       : AdapterStatus::InvalidAction;
        }

        return slot.action.kind == ActionKind::Detach ? applyDetach (index)
                                                      : applyAttach (index);
    }

    const AdapterSnapshot& UsbActionAdapter::snapshot () const noexcept
    {
        return snapshot_;
    }

    const SlotSnapshot*
    UsbActionAdapter::snapshot (DestinationId     destination,
                                DestinationSlotId destinationSlot) const noexcept
    {
        const size_t index = findSlot (destination, destinationSlot);
        return index == NoSlot ? nullptr : &snapshot_.slots[index];
    }

    AdapterStatus UsbActionAdapter::recover () noexcept
    {
        for (size_t index = 0; index < AdapterSlotCapacity; ++index)
        {
            const AdapterStatus status = recoverSlot (index);
            if (status != AdapterStatus::Ok)
            {
                return status;
            }
        }
        return AdapterStatus::Ok;
    }

    AdapterStatus UsbActionAdapter::recoverSlot (size_t index) noexcept
    {
        const SlotSnapshot& slot = snapshot_.slots[index];
        if (!slot.occupied || slot.phase == AdapterPhase::Detached)
        {
            return AdapterStatus::Ok;
        }

        if (slot.phase == AdapterPhase::Fault)
        {
            return AdapterStatus::RecoveryFailure;
        }

        if (slot.phase == AdapterPhase::Active)
        {
            if (bridge_.attached (slot.action.route, slot.action.fence))
            {
                return AdapterStatus::Ok;
            }
            enterFault (index);
            return AdapterStatus::RecoveryFailure;
        }

        if (!bridge_.detach (slot.action.route, slot.action.fence) ||
            !bridge_.detached (slot.action.route, slot.action.fence))
        {
            return enterFault (index);
        }

        if (slot.phase == AdapterPhase::PlannedAttach)
        {
            return AdapterStatus::Ok;
        }

        AdapterSnapshot candidate = snapshot_;
        SlotSnapshot&   recovered = candidate.slots[index];
        recovered.phase           = AdapterPhase::Detached;
        complete (recovered);

        return persist (candidate) ? AdapterStatus::Ok
                                   : AdapterStatus::PersistenceFailure;
    }

    AdapterStatus UsbActionAdapter::applyDetach (size_t index) noexcept
    {
        const RouteAction action = snapshot_.slots[index].action;
        if (!bridge_.detach (action.route, action.fence))
        {
            return AdapterStatus::OperationFailure;
        }

        if (!bridge_.detached (action.route, action.fence))
        {
            return AdapterStatus::VerificationFailure;
        }

        AdapterSnapshot candidate = snapshot_;
        SlotSnapshot&   slot      = candidate.slots[index];
        slot.phase                = AdapterPhase::Detached;
        complete (slot);

        return persist (candidate) ? AdapterStatus::Ok
                                   : AdapterStatus::PersistenceFailure;
    }

    AdapterStatus UsbActionAdapter::applyAttach (size_t index) noexcept
    {
        const RouteAction action = snapshot_.slots[index].action;
        if (!bridge_.attach (action.route, action.fence))
        {
            return AdapterStatus::OperationFailure;
        }

        if (!bridge_.attached (action.route, action.fence))
        {
            if (!bridge_.detach (action.route, action.fence) ||
                !bridge_.detached (action.route, action.fence))
            {
                return enterFault (index);
            }
            return AdapterStatus::VerificationFailure;
        }

        AdapterSnapshot candidate = snapshot_;
        SlotSnapshot&   slot      = candidate.slots[index];
        slot.phase                = AdapterPhase::Active;
        complete (slot);

        if (persist (candidate))
        {
            return AdapterStatus::Ok;
        }

        if (!bridge_.detach (action.route, action.fence) ||
            !bridge_.detached (action.route, action.fence))
        {
            return enterFault (index);
        }

        return AdapterStatus::PersistenceFailure;
    }

    AdapterStatus UsbActionAdapter::enterFault (size_t index) noexcept
    {
        AdapterSnapshot candidate    = snapshot_;
        candidate.slots[index].phase = AdapterPhase::Fault;
        snapshot_.slots[index].phase = AdapterPhase::Fault;
        if (store_.save (candidate))
        {
            snapshot_ = candidate;
        }
        return AdapterStatus::RollbackFailure;
    }

    bool UsbActionAdapter::valid (const RouteAction& action) const noexcept
    {
        const RouteIdentity& route = action.route;
        return action.id.value != 0 && route.device.value != 0 &&
               route.source.value != 0 && route.sourceSlot.value != 0 &&
               route.destination.value != 0 && route.destinationSlot.value != 0 &&
               action.fence.term != 0 && action.fence.epoch != 0;
    }

    bool UsbActionAdapter::valid (const AdapterSnapshot& snapshot) const noexcept
    {
        uint64_t greatestAction = 0;
        for (size_t index = 0; index < AdapterSlotCapacity; ++index)
        {
            const SlotSnapshot& slot = snapshot.slots[index];
            if (!slot.occupied)
            {
                if (slot.hasAction || slot.hasCompletedAction)
                {
                    return false;
                }
                continue;
            }

            if (!valid (slot.action) ||
                (slot.hasAction && slot.phase != AdapterPhase::PlannedDetach &&
                 slot.phase != AdapterPhase::PlannedAttach) ||
                (!slot.hasAction && (slot.phase == AdapterPhase::PlannedDetach ||
                                     slot.phase == AdapterPhase::PlannedAttach)))
            {
                return false;
            }

            if (slot.hasAction)
            {
                greatestAction = slot.action.id.value > greatestAction
                                     ? slot.action.id.value
                                     : greatestAction;
            }
            if (slot.hasCompletedAction)
            {
                greatestAction = slot.completedAction.value > greatestAction
                                     ? slot.completedAction.value
                                     : greatestAction;
            }

            for (size_t other = index + 1; other < AdapterSlotCapacity; ++other)
            {
                const SlotSnapshot& right = snapshot.slots[other];
                if (right.occupied &&
                    right.action.route.destination.value ==
                        slot.action.route.destination.value &&
                    right.action.route.destinationSlot.value ==
                        slot.action.route.destinationSlot.value)
                {
                    return false;
                }
                if (right.occupied &&
                    ((slot.hasAction && right.hasAction &&
                      slot.action.id.value == right.action.id.value) ||
                     (slot.hasAction && right.hasCompletedAction &&
                      slot.action.id.value == right.completedAction.value) ||
                     (slot.hasCompletedAction && right.hasAction &&
                      slot.completedAction.value == right.action.id.value) ||
                     (slot.hasCompletedAction && right.hasCompletedAction &&
                      slot.completedAction.value == right.completedAction.value)))
                {
                    return false;
                }
            }
        }
        return snapshot.actionHighWater >= greatestAction;
    }

    bool UsbActionAdapter::same (const RouteAction& left,
                                 const RouteAction& right) const noexcept
    {
        return left.id.value == right.id.value && left.kind == right.kind &&
               sameRoute (left.route, right.route) &&
               left.fence.term == right.fence.term &&
               left.fence.epoch == right.fence.epoch;
    }

    bool UsbActionAdapter::sameRoute (const RouteIdentity& left,
                                      const RouteIdentity& right) const noexcept
    {
        return left.device.value == right.device.value &&
               left.source.value == right.source.value &&
               left.sourceSlot.value == right.sourceSlot.value &&
               left.destination.value == right.destination.value &&
               left.destinationSlot.value == right.destinationSlot.value;
    }

    bool UsbActionAdapter::fencePermits (const SlotSnapshot& slot,
                                         const RouteAction&  action) const noexcept
    {
        if (!slot.occupied || !slot.hasCompletedAction)
        {
            return action.kind == ActionKind::Detach;
        }

        const RouteFence accepted = slot.action.fence;
        if (action.kind == ActionKind::Attach && slot.phase == AdapterPhase::Detached)
        {
            return action.fence.term == accepted.term &&
                   action.fence.epoch == accepted.epoch;
        }

        return action.kind == ActionKind::Detach &&
               (action.fence.term > accepted.term ||
                (action.fence.term == accepted.term &&
                 action.fence.epoch > accepted.epoch));
    }

    bool UsbActionAdapter::persist (const AdapterSnapshot& candidate) noexcept
    {
        if (!store_.save (candidate))
        {
            return false;
        }

        snapshot_ = candidate;
        return true;
    }

    void UsbActionAdapter::complete (SlotSnapshot& candidate) const noexcept
    {
        candidate.completedAction    = candidate.action.id;
        candidate.hasCompletedAction = true;
        candidate.hasAction          = false;
    }

    size_t UsbActionAdapter::findAction (ActionId action) const noexcept
    {
        for (size_t index = 0; index < AdapterSlotCapacity; ++index)
        {
            const SlotSnapshot& slot = snapshot_.slots[index];
            if (slot.occupied &&
                ((slot.hasAction && slot.action.id.value == action.value) ||
                 (slot.hasCompletedAction &&
                  slot.completedAction.value == action.value)))
            {
                return index;
            }
        }
        return NoSlot;
    }

    size_t UsbActionAdapter::findSlot (DestinationId     destination,
                                       DestinationSlotId destinationSlot) const noexcept
    {
        for (size_t index = 0; index < AdapterSlotCapacity; ++index)
        {
            const SlotSnapshot& slot = snapshot_.slots[index];
            if (slot.occupied &&
                slot.action.route.destination.value == destination.value &&
                slot.action.route.destinationSlot.value == destinationSlot.value)
            {
                return index;
            }
        }
        return NoSlot;
    }

    size_t UsbActionAdapter::freeSlot () const noexcept
    {
        for (size_t index = 0; index < AdapterSlotCapacity; ++index)
        {
            if (!snapshot_.slots[index].occupied)
            {
                return index;
            }
        }
        return NoSlot;
    }
} // namespace adk::usbmesh
