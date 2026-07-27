#include "hdmi_mesh_controller.h"

#include <cstdlib>
#include <iostream>

namespace {

    using adk::hdmimesh::EndpointIdentity;
    using adk::hdmimesh::HdmiMeshController;
    using adk::hdmimesh::MeshStatus;
    using adk::hdmimesh::NodeId;
    using adk::hdmimesh::RouteState;
    using adk::hdmimesh::SinkId;
    using adk::hdmimesh::SinkIdentity;
    using adk::hdmimesh::SourceId;
    using adk::hdmimesh::SourceIdentity;

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
        HdmiMeshController controller;
        EndpointIdentity   receiver{NodeId{1}, 10, 20};
        EndpointIdentity   transmitter{NodeId{2}, 30, 40};
        SourceIdentity     source{SourceId{1}, receiver};
        SourceIdentity     otherSource{SourceId{2}, receiver};
        SinkIdentity       first{SinkId{1}, transmitter};
        SinkIdentity       second{SinkId{2}, transmitter};

        Fixture ()
        {
            requireStatus (controller.enrollReceiver (receiver), MeshStatus::Ok,
                           "enroll receiver");
            requireStatus (controller.enrollTransmitter (transmitter), MeshStatus::Ok,
                           "enroll transmitter");
            requireStatus (controller.observeSource (source), MeshStatus::Ok,
                           "observe source");
            requireStatus (controller.observeSource (otherSource), MeshStatus::Ok,
                           "observe second source");
            requireStatus (controller.observeSink (first), MeshStatus::Ok,
                           "observe first sink");
            requireStatus (controller.observeSink (second), MeshStatus::Ok,
                           "observe second sink");
        }
    };

    uint64_t activate (Fixture& fixture, SourceIdentity source, SinkIdentity sink,
                       uint64_t tick)
    {
        requireStatus (fixture.controller.requestRoute (tick, source, sink),
                       MeshStatus::Ok, "request route");
        auto route = fixture.controller.route (source.source, sink.sink);

        require       (route.state == RouteState::ReadingEdid, "read EDID first");
        requireStatus (
            fixture.controller.confirmEdid (tick + 1, source, sink, route.epoch),
            MeshStatus::Ok, "confirm EDID");
        requireStatus (
            fixture.controller.confirmHpd (tick + 2, source, sink, route.epoch),
            MeshStatus::Ok, "confirm HPD");
        requireStatus (fixture.controller.confirmTrained (source, sink, route.epoch),
                       MeshStatus::Ok, "confirm training");
        return route.epoch;
    }

    void routeRequiresDeterministicHandshake ()
    {
        Fixture        fixture;
        const uint64_t epoch = activate (fixture, fixture.source, fixture.first, 10);
        const auto     active =
            fixture.controller.route (fixture.source.source, fixture.first.sink);

        require (active.state == RouteState::Active, "route active");
        require (active.epoch == epoch, "handshake retains fence");
    }

    void sourceFansOutAndSinkStaysExclusive ()
    {
        Fixture fixture;

        activate (fixture, fixture.source, fixture.first, 10);
        activate (fixture, fixture.source, fixture.second, 20);

        require (fixture.controller.route (fixture.source.source, fixture.first.sink)
                         .state == RouteState::Active,
                 "first fanout active");
        require (fixture.controller.route (fixture.source.source, fixture.second.sink)
                         .state == RouteState::Active,
                 "second fanout active");
        requireStatus (
            fixture.controller.requestRoute (30, fixture.otherSource, fixture.first),
            MeshStatus::SinkBusy, "sink has one reservation");
    }

    void eachTopologyChangeAdvancesSourceFence ()
    {
        Fixture        fixture;
        const uint64_t firstEpoch =
            activate (fixture, fixture.source, fixture.first, 10);

        requireStatus (
            fixture.controller.requestRoute (20, fixture.source, fixture.second),
            MeshStatus::Ok, "add fanout");

        const auto second =
            fixture.controller.route (fixture.source.source, fixture.second.sink);
        const auto first =
            fixture.controller.route (fixture.source.source, fixture.first.sink);

        require       (second.epoch > firstEpoch, "add advances source fence");
        require       (first.epoch == second.epoch, "fanout shares source fence");
        requireStatus (
            fixture.controller.reportFault (fixture.source, fixture.first, firstEpoch),
            MeshStatus::StaleEpoch, "old report rejected");
    }

    void blankIsExplicitBeforeRelease ()
    {
        Fixture fixture;
        activate (fixture, fixture.source, fixture.first, 10);

        requireStatus (
            fixture.controller.requestBlank (20, fixture.source, fixture.first),
            MeshStatus::Ok, "request blank");

        const auto blanking =
            fixture.controller.route (fixture.source.source, fixture.first.sink);

        require       (blanking.state == RouteState::Blanking, "route blanking");
        require       (blanking.assigned, "sink remains reserved while blanking");
        requireStatus (fixture.controller.confirmBlanked (fixture.source, fixture.first,
                                                          blanking.epoch),
                       MeshStatus::Ok, "confirm blank");
        require (!fixture.controller.route (fixture.source.source, fixture.first.sink)
                      .assigned,
                 "blank confirmation releases route");
    }

    void staleIdentityAndWrongOrderFailClosed ()
    {
        Fixture fixture;

        requireStatus (
            fixture.controller.requestRoute (10, fixture.source, fixture.first),
            MeshStatus::Ok, "request route");

        const auto pending =
            fixture.controller.route (fixture.source.source, fixture.first.sink);

        requireStatus (fixture.controller.confirmHpd (11, fixture.source, fixture.first,
                                                      pending.epoch),
                       MeshStatus::WrongState, "cannot skip EDID");

        SourceIdentity stale = fixture.source;
        --stale.endpoint.revision;

        requireStatus (fixture.controller.requestBlank (12, stale, fixture.first),
                       MeshStatus::StaleRevision, "stale source rejected");
    }

    void timeoutAndFaultAreObservable ()
    {
        Fixture fixture;

        requireStatus (
            fixture.controller.requestRoute (10, fixture.source, fixture.first),
            MeshStatus::Ok, "request route");
        requireStatus (fixture.controller.update (1010), MeshStatus::Ok,
                       "advance to timeout");

        const auto fault =
            fixture.controller.route (fixture.source.source, fixture.first.sink);

        require       (fault.state == RouteState::Fault, "timeout faults route");
        require       (fault.assigned, "fault holds sink for diagnosis");
        requireStatus (fixture.controller.clearFault (fixture.source, fixture.first),
                       MeshStatus::Ok, "clear fault");
        require (!fixture.controller.route (fixture.source.source, fixture.first.sink)
                      .assigned,
                 "clear releases reservation");
    }

    void replayIsDeterministic ()
    {
        Fixture first;
        Fixture second;

        activate (first, first.source, first.first, 10);
        activate (second, second.source, second.first, 10);

        const auto left =
            first.controller.route (first.source.source, first.first.sink);
        const auto right =
            second.controller.route (second.source.source, second.first.sink);

        require (left.state == right.state, "replay state");
        require (left.epoch == right.epoch, "replay epoch");
        require (left.deadline == right.deadline, "replay deadline");
    }
} // namespace

int main ()
{
    routeRequiresDeterministicHandshake   ();
    sourceFansOutAndSinkStaysExclusive    ();
    eachTopologyChangeAdvancesSourceFence ();
    blankIsExplicitBeforeRelease          ();
    staleIdentityAndWrongOrderFailClosed  ();
    timeoutAndFaultAreObservable          ();
    replayIsDeterministic                 ();

    std::cout << "HDMI mesh controller tests passed.\n";
    return 0;
}
