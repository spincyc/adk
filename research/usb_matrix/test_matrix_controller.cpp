#include "matrix_controller.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {

    using adk::usbmatrix::DeviceId;
    using adk::usbmatrix::HostId;
    using adk::usbmatrix::MatrixStatus;
    using adk::usbmatrix::RouteState;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    void requireStatus (MatrixStatus actual, MatrixStatus expected, const char* message)
    {
        require (actual == expected, message);
    }

    void attach (adk::usbmatrix::MatrixController& controller, uint64_t tick,
                 HostId host, DeviceId device)
    {
        requireStatus (controller.requestRoute (tick, host, device), MatrixStatus::Ok,
                       "request route");

        const auto requested = controller.snapshot (device);
        require                                    (requested.state == RouteState::Attaching, "route attaching");

        requireStatus (
            controller.confirmAttached (tick + 1, host, device, requested.epoch),
            MatrixStatus::Ok, "confirm attached");
    }

    void testDirectAssignment ()
    {
        adk::usbmatrix::AuditLog         audit;
        adk::usbmatrix::MatrixController controller (audit);
        const HostId                     host{1};
        const DeviceId                   device{7};

        attach (controller, 10, host, device);

        const auto route = controller.snapshot (device);
        require                                (route.state == RouteState::Active, "route active");
        require                                (route.hasActiveHost, "active host present");
        require                                (route.activeHost.value == host.value, "active host matches");
        require                                (audit.size () == 3, "request attach and confirmation audited");
        require                                (audit.verify (), "audit chain valid");
    }

    void testReassignmentDetachesFirst ()
    {
        adk::usbmatrix::AuditLog         audit;
        adk::usbmatrix::MatrixController controller (audit);
        const HostId                     first{1};
        const HostId                     second{2};
        const DeviceId                   device{8};

        attach        (controller, 10, first, device);
        requireStatus (controller.requestRoute (20, second, device), MatrixStatus::Ok,
                       "request reassignment");

        auto route = controller.snapshot (device);
        require                          (route.state == RouteState::Detaching, "old host detaches first");
        require                          (route.activeHost.value == first.value, "old host remains recorded");
        require                          (route.pendingHost.value == second.value, "new host pending");

        requireStatus (controller.confirmAttached (21, second, device, route.epoch),
                       MatrixStatus::WrongState, "attach rejected before detach");
        requireStatus (controller.confirmDetached (22, device, route.epoch),
                       MatrixStatus::Ok, "old host detached");

        route = controller.snapshot (device);
        require                     (route.state == RouteState::Attaching, "new host now attaching");
        require                     (!route.hasActiveHost, "no host active between assignments");

        requireStatus (controller.confirmAttached (23, second, device, route.epoch),
                       MatrixStatus::Ok, "new host attached");
        require (controller.snapshot (device).activeHost.value == second.value,
                 "new host owns device");
    }

    void testStaleEpochCannotResurrectRoute ()
    {
        adk::usbmatrix::AuditLog         audit;
        adk::usbmatrix::MatrixController controller (audit);
        const HostId                     first{1};
        const HostId                     second{2};
        const DeviceId                   device{9};

        attach                                        (controller, 10, first, device);
        const uint64_t oldEpoch = controller.snapshot (device).epoch;

        requireStatus (controller.requestRoute (20, second, device), MatrixStatus::Ok,
                       "request second host");
        const uint64_t currentEpoch = controller.snapshot (device).epoch;

        require       (currentEpoch > oldEpoch, "epoch advances");
        requireStatus (controller.confirmDetached (21, device, oldEpoch),
                       MatrixStatus::StaleEpoch, "stale detach rejected");
        requireStatus (controller.confirmAttached (22, first, device, oldEpoch),
                       MatrixStatus::StaleEpoch, "stale attach rejected");
        require (controller.snapshot (device).state == RouteState::Detaching,
                 "stale reports leave state unchanged");
    }

    void testLossRequiresExplicitRecovery ()
    {
        adk::usbmatrix::AuditLog         audit;
        adk::usbmatrix::MatrixController controller (audit);
        const HostId                     host{1};
        const DeviceId                   device{10};

        attach                                     (controller, 10, host, device);
        const uint64_t epoch = controller.snapshot (device).epoch;

        requireStatus (controller.reportEndpointLost (20, device, epoch),
                       MatrixStatus::Ok, "endpoint loss");
        auto route = controller.snapshot (device);
        require                          (route.state == RouteState::Fault, "loss enters fault");
        require                          (!route.hasActiveHost, "fault has no active host");
        requireStatus                    (controller.requestRoute (21, host, device),
                       MatrixStatus::WrongState, "fault cannot silently restore route");

        requireStatus (controller.clearFault (22, device), MatrixStatus::Ok,
                       "fault explicitly cleared");
        route = controller.snapshot (device);
        require                     (route.state == RouteState::Unassigned, "clear leaves unassigned");
        require                     (route.epoch > epoch, "clear fences old reports");
    }

    void testPersistenceRestoresNoActiveLease ()
    {
        adk::usbmatrix::AuditLog         firstAudit;
        adk::usbmatrix::MatrixController first (firstAudit);
        const HostId                     host{3};
        const DeviceId                   device{11};

        attach (first, 10, host, device);

        std::stringstream persisted;
        requireStatus (first.save (persisted), MatrixStatus::Ok, "save state");

        adk::usbmatrix::AuditLog         secondAudit;
        adk::usbmatrix::MatrixController second (secondAudit);
        requireStatus                           (second.load (persisted, 100), MatrixStatus::Ok, "load state");

        const auto restored = second.snapshot (device);
        require                               (restored.state == RouteState::Fault,
                 "restart cannot assert old active route");
        require (!restored.hasActiveHost, "restart has no active host");
        require (restored.epoch > first.snapshot (device).epoch,
                 "restart advances fence");
        require (secondAudit.verify (), "restore audit chain valid");
    }

    void testMalformedPersistenceIsTransactional ()
    {
        adk::usbmatrix::AuditLog         audit;
        adk::usbmatrix::MatrixController controller (audit);
        const HostId                     host{4};
        const DeviceId                   device{12};

        attach                                         (controller, 10, host, device);
        const auto        before = controller.snapshot (device);
        std::stringstream malformed                    ("ADK_USB_MATRIX 1\n999 1 3 1 4 0 0\n");

        requireStatus (controller.load (malformed, 100), MatrixStatus::PersistenceError,
                       "malformed state rejected");

        const auto after = controller.snapshot (device);
        require                                (after.state == before.state, "failed load preserves state");
        require                                (after.epoch == before.epoch, "failed load preserves epoch");
        require                                (after.activeHost.value == before.activeHost.value,
                 "failed load preserves owner");
    }
} // namespace

int main ()
{
    testDirectAssignment                    ();
    testReassignmentDetachesFirst           ();
    testStaleEpochCannotResurrectRoute      ();
    testLossRequiresExplicitRecovery        ();
    testPersistenceRestoresNoActiveLease    ();
    testMalformedPersistenceIsTransactional ();

    std::cout << "USB matrix controller tests passed.\n";
    return 0;
}
