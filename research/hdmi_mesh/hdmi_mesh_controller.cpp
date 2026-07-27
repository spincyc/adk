#include "hdmi_mesh_controller.h"

#include <limits>

namespace adk::hdmimesh {

    namespace {

        bool validNode (NodeId node) noexcept
        {
            return node.value > 0 && node.value <= HdmiMeshController::maximumNodes;
        }

        bool validSource (SourceId source) noexcept
        {
            return source.value > 0 &&
                   source.value <= HdmiMeshController::maximumSources;
        }

        bool validSink (SinkId sink) noexcept
        {
            return sink.value > 0 && sink.value <= HdmiMeshController::maximumSinks;
        }

        bool validEndpoint (EndpointIdentity endpoint) noexcept
        {
            return validNode (endpoint.node) && endpoint.incarnation > 0 &&
                   endpoint.revision > 0;
        }
    } // namespace

    HdmiMeshController::HdmiMeshController (const MeshConfig& config) noexcept
        : config_ (config), receivers_{}, transmitters_{}, sources_{}, sinks_{},
          routes_{}
    {
    }

    MeshStatus HdmiMeshController::enrollReceiver (EndpointIdentity receiver) noexcept
    {
        if (!validEndpoint (receiver))
        {
            return MeshStatus::InvalidIdentity;
        }

        NodeRecord& record = receivers_[receiver.node.value - 1];

        if (record.enrolled && receiver.incarnation < record.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (record.enrolled && receiver.incarnation == record.identity.incarnation &&
            receiver.revision < record.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        record.identity = receiver;
        record.enrolled = true;
        return MeshStatus::Ok;
    }

    MeshStatus
    HdmiMeshController::enrollTransmitter (EndpointIdentity transmitter) noexcept
    {
        if (!validEndpoint (transmitter))
        {
            return MeshStatus::InvalidIdentity;
        }

        NodeRecord& record = transmitters_[transmitter.node.value - 1];

        if (record.enrolled && transmitter.incarnation < record.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (record.enrolled && transmitter.incarnation == record.identity.incarnation &&
            transmitter.revision < record.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        record.identity = transmitter;
        record.enrolled = true;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::observeSource (SourceIdentity source) noexcept
    {
        if (!validSource (source.source) || !validEndpoint (source.endpoint))
        {
            return MeshStatus::InvalidIdentity;
        }

        const NodeRecord& receiver = receivers_[source.endpoint.node.value - 1];

        if (!receiver.enrolled)
        {
            return MeshStatus::NotEnrolled;
        }

        if (source.endpoint.incarnation != receiver.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (source.endpoint.revision != receiver.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        SourceRecord& record = sources_[source.source.value - 1];

        if (record.observed &&
            record.identity.endpoint.node.value != source.endpoint.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (record.observed &&
            !sameEndpoint (record.identity.endpoint, source.endpoint))
        {
            for (const RouteRecord& route : routes_)
            {
                if (route.used &&
                    route.route.source.source.value == source.source.value)
                {
                    return MeshStatus::WrongState;
                }
            }
        }

        record.identity = source;
        record.observed = true;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::observeSink (SinkIdentity sink) noexcept
    {
        if (!validSink (sink.sink) || !validEndpoint (sink.endpoint))
        {
            return MeshStatus::InvalidIdentity;
        }

        const NodeRecord& transmitter = transmitters_[sink.endpoint.node.value - 1];

        if (!transmitter.enrolled)
        {
            return MeshStatus::NotEnrolled;
        }

        if (sink.endpoint.incarnation != transmitter.identity.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (sink.endpoint.revision != transmitter.identity.revision)
        {
            return MeshStatus::StaleRevision;
        }

        SinkRecord& record = sinks_[sink.sink.value - 1];

        if (record.observed &&
            record.identity.endpoint.node.value != sink.endpoint.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (record.observed &&
            !sameEndpoint  (record.identity.endpoint, sink.endpoint) &&
            sinkReserved   (sink.sink))
        {
            return MeshStatus::WrongState;
        }

        record.identity = sink;
        record.observed = true;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::requestRoute (uint64_t tick, SourceIdentity source,
                                                 SinkIdentity sink) noexcept
    {
        MeshStatus status = validateSource (source);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        status = validateSink (sink);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route != nullptr)
        {
            if (route->route.state == RouteState::Fault)
            {
                return MeshStatus::WrongState;
            }

            return MeshStatus::Ok;
        }

        if (sinkReserved (sink.sink))
        {
            return MeshStatus::SinkBusy;
        }

        route = freeRoute ();

        if (route == nullptr)
        {
            return MeshStatus::CapacityExceeded;
        }

        SourceRecord& sourceRecord = sources_[source.source.value - 1];
        status                     = advanceEpoch (sourceRecord);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        stampRoutes (source.source, sourceRecord.epoch);

        route->route.source   = source;
        route->route.sink     = sink;
        route->route.epoch    = sourceRecord.epoch;
        route->route.deadline = tick + config_.edidTimeoutTicks;
        route->route.state    = RouteState::ReadingEdid;
        route->route.assigned = true;
        route->used           = true;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::requestBlank (uint64_t tick, SourceIdentity source,
                                                 SinkIdentity sink) noexcept
    {
        MeshStatus status = validateSource (source);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        status = validateSink (sink);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::Ok;
        }

        if (route->route.state == RouteState::Blanking)
        {
            return MeshStatus::Ok;
        }

        SourceRecord& sourceRecord = sources_[source.source.value - 1];
        status                     = advanceEpoch (sourceRecord);

        if (status != MeshStatus::Ok)
        {
            return status;
        }

        stampRoutes (source.source, sourceRecord.epoch);

        route->route.epoch    = sourceRecord.epoch;
        route->route.deadline = tick + config_.blankTimeoutTicks;
        route->route.state    = RouteState::Blanking;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::confirmEdid (uint64_t tick, SourceIdentity source,
                                                SinkIdentity sink,
                                                uint64_t     epoch) noexcept
    {
        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::WrongState;
        }

        if (epoch != route->route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        if (route->route.state != RouteState::ReadingEdid)
        {
            return MeshStatus::WrongState;
        }

        route->route.state    = RouteState::AssertingHpd;
        route->route.deadline = tick + config_.hpdTimeoutTicks;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::confirmHpd (uint64_t tick, SourceIdentity source,
                                               SinkIdentity sink,
                                               uint64_t     epoch) noexcept
    {
        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::WrongState;
        }

        if (epoch != route->route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        if (route->route.state != RouteState::AssertingHpd)
        {
            return MeshStatus::WrongState;
        }

        route->route.state    = RouteState::Training;
        route->route.deadline = tick + config_.trainingTimeoutTicks;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::confirmTrained (SourceIdentity source,
                                                   SinkIdentity   sink,
                                                   uint64_t       epoch) noexcept
    {
        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::WrongState;
        }

        if (epoch != route->route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        if (route->route.state != RouteState::Training)
        {
            return MeshStatus::WrongState;
        }

        route->route.state    = RouteState::Active;
        route->route.deadline = 0;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::confirmBlanked (SourceIdentity source,
                                                   SinkIdentity   sink,
                                                   uint64_t       epoch) noexcept
    {
        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::WrongState;
        }

        if (epoch != route->route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        if (route->route.state != RouteState::Blanking)
        {
            return MeshStatus::WrongState;
        }

        *route = RouteRecord{};
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::reportFault (SourceIdentity source,
                                                SinkIdentity   sink,
                                                uint64_t       epoch) noexcept
    {
        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::WrongState;
        }

        if (epoch != route->route.epoch)
        {
            return MeshStatus::StaleEpoch;
        }

        route->route.state    = RouteState::Fault;
        route->route.deadline = 0;
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::clearFault (SourceIdentity source,
                                               SinkIdentity   sink) noexcept
    {
        RouteRecord* route = findRoute (source.source, sink.sink);

        if (route == nullptr)
        {
            return MeshStatus::Ok;
        }

        if (route->route.state != RouteState::Fault)
        {
            return MeshStatus::WrongState;
        }

        *route = RouteRecord{};
        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::update (uint64_t tick) noexcept
    {
        for (RouteRecord& route : routes_)
        {
            if (route.used && route.route.deadline != 0 &&
                expired (tick, route.route.deadline))
            {
                route.route.state    = RouteState::Fault;
                route.route.deadline = 0;
            }
        }

        return MeshStatus::Ok;
    }

    RouteSnapshot HdmiMeshController::route (SourceId source,
                                             SinkId   sink) const noexcept
    {
        const RouteRecord* record = findRoute (source, sink);

        if (record == nullptr)
        {
            return RouteSnapshot{};
        }

        return record->route;
    }

    uint64_t HdmiMeshController::sourceEpoch (SourceId source) const noexcept
    {
        if (!validSource (source))
        {
            return 0;
        }

        return sources_[source.value - 1].epoch;
    }

    MeshStatus HdmiMeshController::validateSource (SourceIdentity source) const noexcept
    {
        if (!validSource (source.source) || !validEndpoint (source.endpoint))
        {
            return MeshStatus::InvalidIdentity;
        }

        const SourceRecord& record = sources_[source.source.value - 1];

        if (!record.observed)
        {
            return MeshStatus::NotEnrolled;
        }

        if (source.endpoint.node.value != record.identity.endpoint.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (source.endpoint.incarnation != record.identity.endpoint.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (source.endpoint.revision != record.identity.endpoint.revision)
        {
            return MeshStatus::StaleRevision;
        }

        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::validateSink (SinkIdentity sink) const noexcept
    {
        if (!validSink (sink.sink) || !validEndpoint (sink.endpoint))
        {
            return MeshStatus::InvalidIdentity;
        }

        const SinkRecord& record = sinks_[sink.sink.value - 1];

        if (!record.observed)
        {
            return MeshStatus::NotEnrolled;
        }

        if (sink.endpoint.node.value != record.identity.endpoint.node.value)
        {
            return MeshStatus::InvalidIdentity;
        }

        if (sink.endpoint.incarnation != record.identity.endpoint.incarnation)
        {
            return MeshStatus::StaleIncarnation;
        }

        if (sink.endpoint.revision != record.identity.endpoint.revision)
        {
            return MeshStatus::StaleRevision;
        }

        return MeshStatus::Ok;
    }

    MeshStatus HdmiMeshController::advanceEpoch (SourceRecord& source) noexcept
    {
        if (source.epoch == std::numeric_limits<uint64_t>::max ())
        {
            return MeshStatus::EpochExhausted;
        }

        ++source.epoch;
        return MeshStatus::Ok;
    }

    void HdmiMeshController::stampRoutes (SourceId source, uint64_t epoch) noexcept
    {
        for (RouteRecord& route : routes_)
        {
            if (route.used && route.route.source.source.value == source.value)
            {
                route.route.epoch = epoch;
            }
        }
    }

    HdmiMeshController::RouteRecord*
    HdmiMeshController::findRoute (SourceId source, SinkId sink) noexcept
    {
        for (RouteRecord& route : routes_)
        {
            if (route.used && route.route.source.source.value == source.value &&
                route.route.sink.sink.value == sink.value)
            {
                return &route;
            }
        }

        return nullptr;
    }

    const HdmiMeshController::RouteRecord*
    HdmiMeshController::findRoute (SourceId source, SinkId sink) const noexcept
    {
        for (const RouteRecord& route : routes_)
        {
            if (route.used && route.route.source.source.value == source.value &&
                route.route.sink.sink.value == sink.value)
            {
                return &route;
            }
        }

        return nullptr;
    }

    HdmiMeshController::RouteRecord* HdmiMeshController::freeRoute () noexcept
    {
        for (RouteRecord& route : routes_)
        {
            if (!route.used)
            {
                return &route;
            }
        }

        return nullptr;
    }

    bool HdmiMeshController::sinkReserved (SinkId sink) const noexcept
    {
        for (const RouteRecord& route : routes_)
        {
            if (route.used && route.route.sink.sink.value == sink.value)
            {
                return true;
            }
        }

        return false;
    }

    bool HdmiMeshController::sameEndpoint (EndpointIdentity left,
                                           EndpointIdentity right) const noexcept
    {
        return left.node.value == right.node.value &&
               left.incarnation == right.incarnation && left.revision == right.revision;
    }

    bool HdmiMeshController::expired (uint64_t tick, uint64_t deadline) const noexcept
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
            case MeshStatus::SinkBusy: return "sink-busy";
            case MeshStatus::CapacityExceeded: return "capacity-exceeded";
            case MeshStatus::EpochExhausted: return "epoch-exhausted";
        }

        return "unknown";
    }

    const char* routeStateName (RouteState state) noexcept
    {
        switch (state)
        {
            case RouteState::Unassigned: return "unassigned";
            case RouteState::ReadingEdid: return "reading-edid";
            case RouteState::AssertingHpd: return "asserting-hpd";
            case RouteState::Training: return "training";
            case RouteState::Active: return "active";
            case RouteState::Blanking: return "blanking";
            case RouteState::Fault: return "fault";
        }

        return "unknown";
    }
} // namespace adk::hdmimesh
