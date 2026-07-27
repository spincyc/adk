#include "mesh_controller.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>

namespace {

    using adk::usbmesh::DeviceId;
    using adk::usbmesh::DeviceIdentity;
    using adk::usbmesh::EndpointIdentity;
    using adk::usbmesh::MeshController;
    using adk::usbmesh::MeshConfig;
    using adk::usbmesh::MeshStatus;
    using adk::usbmesh::NodeId;
    using adk::usbmesh::RouteState;
    using adk::usbmesh::SlotId;
    using adk::usbmesh::SlotIdentity;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    void requireStatus (MeshStatus actual, MeshStatus expected, const char* message)
    {
        require (actual == expected, message);
    }

    struct Fixture
    {
        MeshController     controller;
        EndpointIdentity  source      {NodeId {1}, 10, 20};
        EndpointIdentity  destination {NodeId {2}, 30, 40};
        DeviceIdentity    camera      {DeviceId {1}, source};
        DeviceIdentity    keyboard    {DeviceId {2}, source};
        SlotIdentity      first       {SlotId {1}, destination};
        SlotIdentity      second      {SlotId {2}, destination};

        Fixture ()
        {
            requireStatus (controller.enrollSource (source), MeshStatus::Ok,
                           "enroll source");
            requireStatus (controller.enrollDestination (destination), MeshStatus::Ok,
                           "enroll destination");
            requireStatus (controller.observeDevice (camera), MeshStatus::Ok,
                           "observe camera");
            requireStatus (controller.observeDevice (keyboard), MeshStatus::Ok,
                           "observe keyboard");
            requireStatus (controller.observeSlot (first), MeshStatus::Ok,
                           "observe first slot");
            requireStatus (controller.observeSlot (second), MeshStatus::Ok,
                           "observe second slot");
        }
    };

    void attach (Fixture& fixture, DeviceIdentity device, SlotIdentity slot,
                 uint64_t tick)
    {
        requireStatus (
            fixture.controller.requestRoute (tick, device, slot),
            MeshStatus::Ok,
            "request route");

        const auto pending = fixture.controller.route (device.device);

        require (pending.state == RouteState::Attaching, "route attaching");

        requireStatus (
            fixture.controller.confirmAttached (
                tick + 1, device, slot, pending.epoch),
            MeshStatus::Ok,
            "confirm attach");
    }

    void multipleSlotsAllowMultipleDevices ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);
        attach (fixture, fixture.keyboard, fixture.second, 20);

        require (fixture.controller.route (fixture.camera.device).state ==
                     RouteState::Active,
                 "camera active");
        require (fixture.controller.route (fixture.keyboard.device).state ==
                     RouteState::Active,
                 "keyboard active");
    }

    void oneSlotHasOneExclusiveLease ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);

        requireStatus (
            fixture.controller.requestRoute (
                20, fixture.keyboard, fixture.first),
            MeshStatus::SlotBusy,
            "occupied slot rejected");
    }

    void moveIsBreakBeforeMake ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);
        const uint64_t firstEpoch =
            fixture.controller.route (fixture.camera.device).epoch;

        requireStatus (
            fixture.controller.requestRoute (
                20, fixture.camera, fixture.second),
            MeshStatus::Ok,
            "request move");

        auto moving = fixture.controller.route (fixture.camera.device);

        require (moving.state == RouteState::Detaching, "old route detaching");
        require (moving.epoch > firstEpoch, "move advances fence");

        requireStatus (
            fixture.controller.confirmAttached (
                21, fixture.camera, fixture.second, moving.epoch),
            MeshStatus::WrongState,
            "cannot attach before detach");
        requireStatus (
            fixture.controller.confirmDetached (
                22, fixture.camera, moving.epoch),
            MeshStatus::Ok,
            "old route detached");

        moving = fixture.controller.route (fixture.camera.device);

        require (moving.state == RouteState::Attaching, "new route attaching");
        require (!moving.hasActiveSlot, "no active slot between routes");

        requireStatus (
            fixture.controller.confirmAttached (
                23, fixture.camera, fixture.second, moving.epoch),
            MeshStatus::Ok,
            "new route attached");
    }

    void staleIdentityAndEpochAreRejected ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);

        const auto active = fixture.controller.route (fixture.camera.device);

        DeviceIdentity stale = fixture.camera;
        --stale.source.revision;

        requireStatus (
            fixture.controller.requestRelease (20, stale),
            MeshStatus::StaleRevision,
            "stale source revision rejected");

        stale.source.node = NodeId {3};

        requireStatus (
            fixture.controller.requestRelease (20, stale),
            MeshStatus::InvalidIdentity,
            "wrong source node rejected");

        requireStatus (
            fixture.controller.reportFault (
                21, fixture.camera, active.epoch - 1),
            MeshStatus::StaleEpoch,
            "stale epoch rejected");
        require (fixture.controller.route (fixture.camera.device).state ==
                     RouteState::Active,
                 "stale reports preserve route");
    }

    void releaseAndTimeoutFailClosed ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);

        requireStatus (
            fixture.controller.requestRelease (20, fixture.camera),
            MeshStatus::Ok,
            "request release");
        requireStatus (fixture.controller.update (5020), MeshStatus::Ok,
                       "advance timeout");

        const auto fault = fixture.controller.route (fixture.camera.device);

        require (fault.state == RouteState::Fault, "timeout enters fault");
        require (!fault.hasActiveSlot, "timeout clears active authority");
    }

    void persistenceRestoresOnlyFences ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);
        const uint64_t oldEpoch =
            fixture.controller.route (fixture.camera.device).epoch;
        std::stringstream saved;

        requireStatus (fixture.controller.save (saved), MeshStatus::Ok,
                       "save durable fences");

        MeshController restored;

        requireStatus (restored.load (saved), MeshStatus::Ok,
                       "restore durable fences");
        require (restored.route (fixture.camera.device).state == RouteState::Fault,
                 "restart never restores attachment");
        require (restored.route (fixture.camera.device).epoch > oldEpoch,
                 "restart advances durable fence");
    }

    void malformedPersistenceIsTransactional ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);

        const auto before = fixture.controller.route (fixture.camera.device);

        std::istringstream malformed ("ADK_USB_MESH 1\n0\n");

        requireStatus (fixture.controller.load (malformed),
                       MeshStatus::PersistenceError,
                       "short persistence rejected");

        const auto after = fixture.controller.route (fixture.camera.device);

        require (after.state == before.state, "failed load preserves state");
        require (after.epoch == before.epoch, "failed load preserves fence");
    }

    void sourceReenrollmentFencesOnlyAffectedRoutes ()
    {
        Fixture fixture;
        EndpointIdentity otherSource {NodeId {3}, 50, 60};
        DeviceIdentity   dial        {DeviceId {3}, otherSource};

        requireStatus (fixture.controller.enrollSource (otherSource),
                       MeshStatus::Ok, "enroll second source");
        requireStatus (fixture.controller.observeDevice (dial), MeshStatus::Ok,
                       "observe second source device");
        attach (fixture, fixture.camera, fixture.first, 10);
        attach (fixture, dial, fixture.second, 20);

        const auto cameraBefore = fixture.controller.route (fixture.camera.device);
        const auto dialBefore   = fixture.controller.route (dial.device);

        ++fixture.source.incarnation;
        fixture.source.revision = 1;

        requireStatus (fixture.controller.enrollSource (fixture.source),
                       MeshStatus::Ok, "reenroll source");

        const auto cameraAfter = fixture.controller.route (fixture.camera.device);
        const auto dialAfter   = fixture.controller.route (dial.device);

        require (cameraAfter.state == RouteState::Fault,
                 "source reenrollment faults affected route");
        require (cameraAfter.epoch == cameraBefore.epoch + 1,
                 "source reenrollment advances affected fence");
        require (dialAfter.state == RouteState::Active,
                 "source reenrollment preserves unrelated route");
        require (dialAfter.epoch == dialBefore.epoch,
                 "source reenrollment preserves unrelated fence");

        fixture.camera.source = fixture.source;
        requireStatus (fixture.controller.observeDevice (fixture.camera),
                       MeshStatus::Ok, "reobserve device after source restart");
        requireStatus (fixture.controller.clearFault (fixture.camera),
                       MeshStatus::Ok, "clear fenced source route");
    }

    void destinationReenrollmentFencesEveryAffectedSlot ()
    {
        Fixture fixture;

        attach (fixture, fixture.camera, fixture.first, 10);
        attach (fixture, fixture.keyboard, fixture.second, 20);

        const auto cameraBefore =
            fixture.controller.route (fixture.camera.device);
        const auto keyboardBefore =
            fixture.controller.route (fixture.keyboard.device);

        ++fixture.destination.revision;

        requireStatus (
            fixture.controller.enrollDestination (fixture.destination),
            MeshStatus::Ok,
            "reenroll destination");

        const auto cameraAfter =
            fixture.controller.route (fixture.camera.device);
        const auto keyboardAfter =
            fixture.controller.route (fixture.keyboard.device);

        require (cameraAfter.state == RouteState::Fault,
                 "destination reenrollment faults first slot");
        require (keyboardAfter.state == RouteState::Fault,
                 "destination reenrollment faults second slot");
        require (cameraAfter.epoch == cameraBefore.epoch + 1,
                 "destination reenrollment fences first route");
        require (keyboardAfter.epoch == keyboardBefore.epoch + 1,
                 "destination reenrollment fences second route");

        fixture.first.destination  = fixture.destination;
        fixture.second.destination = fixture.destination;
        requireStatus (fixture.controller.observeSlot (fixture.first),
                       MeshStatus::Ok, "reobserve first destination slot");
        requireStatus (fixture.controller.observeSlot (fixture.second),
                       MeshStatus::Ok, "reobserve second destination slot");
    }

    void timeoutConfigurationUsesUnambiguousHalfRange ()
    {
        MeshConfig invalidZero {0, 1};
        MeshController zeroController (invalidZero);
        EndpointIdentity source {NodeId {1}, 1, 1};

        requireStatus (zeroController.enrollSource (source),
                       MeshStatus::InvalidConfiguration,
                       "zero timeout rejected");

        MeshConfig invalidHalf {
            static_cast<uint64_t> (std::numeric_limits<int64_t>::max ()) + 1,
            1};
        MeshController halfController (invalidHalf);

        requireStatus (halfController.enrollSource (source),
                       MeshStatus::InvalidConfiguration,
                       "ambiguous half-range timeout rejected");
    }

    void deadlinesRemainCorrectAcrossTickWrap ()
    {
        MeshConfig    config {5, 5};
        MeshController controller (config);
        EndpointIdentity source      {NodeId {1}, 1, 1};
        EndpointIdentity destination {NodeId {2}, 1, 1};
        DeviceIdentity   device      {DeviceId {1}, source};
        SlotIdentity     slot        {SlotId {1}, destination};

        requireStatus (controller.enrollSource (source), MeshStatus::Ok,
                       "wrap enroll source");
        requireStatus (controller.enrollDestination (destination),
                       MeshStatus::Ok, "wrap enroll destination");
        requireStatus (controller.observeDevice (device), MeshStatus::Ok,
                       "wrap observe device");
        requireStatus (controller.observeSlot (slot), MeshStatus::Ok,
                       "wrap observe slot");

        const uint64_t start = std::numeric_limits<uint64_t>::max () - 2;

        requireStatus (controller.requestRoute (start, device, slot),
                       MeshStatus::Ok, "wrap request route");
        requireStatus (controller.update (1), MeshStatus::Ok,
                       "one tick before wrapped deadline");
        require (controller.route (device.device).state == RouteState::Attaching,
                 "wrapped deadline not early");
        requireStatus (controller.update (2), MeshStatus::Ok,
                       "at wrapped deadline");
        require (controller.route (device.device).state == RouteState::Fault,
                 "wrapped deadline expires exactly");
    }

    void restartPreservesEpochExhaustion ()
    {
        std::ostringstream persisted;

        persisted << "ADK_USB_MESH 1\n"
                  << std::numeric_limits<uint64_t>::max () - 1 << '\n';

        for (std::size_t index = 1; index < MeshController::maximumDevices;
             ++index)
        {
            persisted << "0\n";
        }

        std::istringstream input (persisted.str ());
        MeshController restored;

        requireStatus (restored.load (input), MeshStatus::Ok,
                       "load maximum durable fence");

        EndpointIdentity source {NodeId {1}, 1, 1};
        DeviceIdentity   device {DeviceId {1}, source};

        requireStatus (restored.enrollSource (source), MeshStatus::Ok,
                       "enroll after maximum fence restore");
        requireStatus (restored.observeDevice (device), MeshStatus::Ok,
                       "observe after maximum fence restore");
        requireStatus (restored.clearFault (device), MeshStatus::EpochExhausted,
                       "maximum restored fence cannot wrap");
    }
} // namespace

int main ()
{
    multipleSlotsAllowMultipleDevices              ();
    oneSlotHasOneExclusiveLease                    ();
    moveIsBreakBeforeMake                          ();
    staleIdentityAndEpochAreRejected               ();
    releaseAndTimeoutFailClosed                    ();
    persistenceRestoresOnlyFences                  ();
    malformedPersistenceIsTransactional            ();
    sourceReenrollmentFencesOnlyAffectedRoutes     ();
    destinationReenrollmentFencesEveryAffectedSlot ();
    timeoutConfigurationUsesUnambiguousHalfRange   ();
    deadlinesRemainCorrectAcrossTickWrap           ();
    restartPreservesEpochExhaustion                ();

    std::cout << "USB mesh controller tests passed.\n";
    return 0;
}
