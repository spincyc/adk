#include "matrix_controller.h"

#include <cassert>
#include <sstream>

namespace {

    using adk::usbmatrix::AuditLog;
    using adk::usbmatrix::DeviceId;
    using adk::usbmatrix::HostId;
    using adk::usbmatrix::MatrixConfig;
    using adk::usbmatrix::MatrixController;
    using adk::usbmatrix::MatrixStatus;
    using adk::usbmatrix::RouteSnapshot;
    using adk::usbmatrix::RouteState;

    RouteSnapshot attach (MatrixController& controller, uint64_t tick, HostId host,
                          DeviceId device)
    {
        assert (controller.requestRoute (tick, host, device) == MatrixStatus::Ok);

        RouteSnapshot pending = controller.snapshot (device);

        assert (pending.state == RouteState::Attaching);

        assert (controller.confirmAttached (tick + 1, host, device, pending.epoch) ==
                MatrixStatus::Ok);

        return controller.snapshot (device);
    }

    void staleConfirmationsCannotChangeANewerLease ()
    {
        AuditLog         audit;
        MatrixController controller (audit);
        DeviceId         device{4};
        HostId           firstHost{11};
        HostId           secondHost{12};

        RouteSnapshot first = attach (controller, 10, firstHost, device);

        assert (controller.requestRoute (20, secondHost, device) == MatrixStatus::Ok);

        RouteSnapshot detaching = controller.snapshot (device);

        assert (detaching.epoch > first.epoch);

        assert (controller.confirmAttached (21, firstHost, device, first.epoch) ==
                MatrixStatus::StaleEpoch);

        assert (controller.reportEndpointLost (21, device, first.epoch) ==
                MatrixStatus::StaleEpoch);

        assert (controller.confirmDetached (22, device, first.epoch) ==
                MatrixStatus::StaleEpoch);

        RouteSnapshot unchanged = controller.snapshot (device);

        assert (unchanged.epoch == detaching.epoch);

        assert (unchanged.state == RouteState::Detaching);

        assert (unchanged.activeHost.value == firstHost.value);

        assert (unchanged.pendingHost.value == secondHost.value);
    }

    void duplicateRequestsAreIdempotent ()
    {
        AuditLog         audit;
        MatrixController controller (audit);
        DeviceId         device{5};
        HostId           host{20};

        RouteSnapshot active    = attach     (controller, 30, host, device);
        std::size_t   auditSize = audit.size ();

        assert (controller.requestRoute (40, host, device) == MatrixStatus::Ok);

        RouteSnapshot unchanged = controller.snapshot (device);

        assert (unchanged.state == RouteState::Active);

        assert (unchanged.epoch == active.epoch);

        assert (audit.size () == auditSize);
    }

    void failedAndRepeatedDetachRemainFailClosed ()
    {
        AuditLog         audit;
        MatrixController controller (audit);
        DeviceId         device{6};
        HostId           host{21};

        RouteSnapshot active = attach (controller, 50, host, device);

        assert (controller.detachRoute (60, device) == MatrixStatus::Ok);

        RouteSnapshot detaching = controller.snapshot (device);

        assert (detaching.epoch > active.epoch);

        assert (controller.detachRoute (61, device) == MatrixStatus::WrongState);

        assert (controller.confirmDetached (62, device, active.epoch) ==
                MatrixStatus::StaleEpoch);

        assert (controller.confirmDetached (63, device, detaching.epoch) ==
                MatrixStatus::Ok);

        assert (controller.detachRoute (64, device) == MatrixStatus::Ok);

        RouteSnapshot released = controller.snapshot (device);

        assert (released.state == RouteState::Unassigned);

        assert (!released.hasActiveHost);

        assert (!released.hasPendingHost);
    }

    void endpointLossRequiresTheCurrentEpoch ()
    {
        AuditLog         audit;
        MatrixController controller (audit);
        DeviceId         device{7};
        HostId           host{22};

        RouteSnapshot active = attach (controller, 70, host, device);

        assert (controller.reportEndpointLost (80, device, active.epoch - 1) ==
                MatrixStatus::StaleEpoch);

        assert (controller.snapshot (device).state == RouteState::Active);

        assert (controller.reportEndpointLost (81, device, active.epoch) ==
                MatrixStatus::Ok);

        RouteSnapshot fault = controller.snapshot (device);

        assert (fault.state == RouteState::Fault);

        assert (!fault.hasActiveHost);

        assert (!fault.hasPendingHost);
    }

    void malformedPersistenceIsRejected ()
    {
        AuditLog           audit;
        MatrixController   controller (audit);
        std::istringstream malformed  ("ADK_USB_MATRIX 1\n999 1 0 0 0 0 0\n");

        assert (controller.load (malformed, 90) == MatrixStatus::PersistenceError);

        assert (controller.snapshot (DeviceId{1}).state == RouteState::Unassigned);

        assert (audit.verify ());
    }

    void oneHostCannotOwnTwoDevices ()
    {
        AuditLog         audit;
        MatrixController controller (audit);
        HostId           host{30};

        attach (controller, 100, host, DeviceId{8});

        assert (controller.requestRoute (110, host, DeviceId{9}) ==
                MatrixStatus::WrongState);

        assert (controller.snapshot (DeviceId{9}).state == RouteState::Unassigned);
    }

    void auditCapacityFailureDoesNotStartATransition ()
    {
        AuditLog audit;

        for (std::size_t index = 0; index < AuditLog::capacity - 1; ++index)
        {
            assert (audit.append (index, 1, HostId{1}, DeviceId{1},
                                  adk::usbmatrix::AuditAction::FaultCleared,
                                  RouteState::Unassigned) == MatrixStatus::Ok);
        }

        MatrixController controller (audit);

        assert (controller.requestRoute (200, HostId{31}, DeviceId{10}) ==
                MatrixStatus::CapacityExceeded);

        RouteSnapshot unchanged = controller.snapshot (DeviceId{10});

        assert (unchanged.state == RouteState::Unassigned);

        assert (unchanged.epoch == 0);

        assert (!unchanged.hasActiveHost);

        assert (!unchanged.hasPendingHost);
    }

    void transitionTimeoutFailsClosed ()
    {
        AuditLog     audit;
        MatrixConfig config;

        config.attachTimeoutTicks = 5;
        config.detachTimeoutTicks = 5;

        MatrixController controller (audit, config);
        DeviceId         device{11};

        assert (controller.requestRoute (300, HostId{32}, device) == MatrixStatus::Ok);

        assert (controller.update (304) == MatrixStatus::Ok);

        assert (controller.snapshot (device).state == RouteState::Attaching);

        assert (controller.update (305) == MatrixStatus::Ok);

        RouteSnapshot fault = controller.snapshot (device);

        assert (fault.state == RouteState::Fault);

        assert (!fault.hasActiveHost);

        assert (!fault.hasPendingHost);
    }

    void malformedLoadPreservesLiveState ()
    {
        AuditLog         audit;
        MatrixController controller (audit);
        DeviceId         device{12};
        HostId           host{33};

        RouteSnapshot      before = attach (controller, 400, host, device);
        std::istringstream malformed       ("ADK_USB_MATRIX 1\n13 2 3 1 33 0 0 13\n"
                                      "13 3 3 1 34 0 0 14\n");

        assert (controller.load (malformed, 410) == MatrixStatus::PersistenceError);

        RouteSnapshot after = controller.snapshot (device);

        assert (after.state == before.state);

        assert (after.epoch == before.epoch);

        assert (after.activeHost.value == before.activeHost.value);
    }

} // namespace

int main ()
{
    staleConfirmationsCannotChangeANewerLease   ();
    duplicateRequestsAreIdempotent              ();
    failedAndRepeatedDetachRemainFailClosed     ();
    endpointLossRequiresTheCurrentEpoch         ();
    malformedPersistenceIsRejected              ();
    oneHostCannotOwnTwoDevices                  ();
    auditCapacityFailureDoesNotStartATransition ();
    transitionTimeoutFailsClosed                ();
    malformedLoadPreservesLiveState             ();
}
