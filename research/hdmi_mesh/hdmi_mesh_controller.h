#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace adk::hdmimesh {

    struct NodeId
    {
        uint16_t value;
    };

    struct SourceId
    {
        uint16_t value;
    };

    struct SinkId
    {
        uint16_t value;
    };

    struct EndpointIdentity
    {
        NodeId   node;
        uint64_t incarnation;
        uint64_t revision;
    };

    struct SourceIdentity
    {
        SourceId         source;
        EndpointIdentity endpoint;
    };

    struct SinkIdentity
    {
        SinkId           sink;
        EndpointIdentity endpoint;
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
        SinkBusy,
        CapacityExceeded,
        EpochExhausted
    };

    enum struct RouteState : uint8_t
    {
        Unassigned,
        ReadingEdid,
        AssertingHpd,
        Training,
        Active,
        Blanking,
        Fault
    };

    struct MeshConfig
    {
        uint64_t edidTimeoutTicks     = 1000;
        uint64_t hpdTimeoutTicks      = 1000;
        uint64_t trainingTimeoutTicks = 5000;
        uint64_t blankTimeoutTicks    = 1000;
    };

    struct RouteSnapshot
    {
        SourceIdentity source;
        SinkIdentity   sink;
        uint64_t       epoch;
        uint64_t       deadline;
        RouteState     state;
        bool           assigned;
    };

    struct HdmiMeshController
    {
        static constexpr std::size_t maximumNodes   = 16;
        static constexpr std::size_t maximumSources = 32;
        static constexpr std::size_t maximumSinks   = 64;
        static constexpr std::size_t maximumRoutes  = 64;

        explicit HdmiMeshController (const MeshConfig& config = MeshConfig{}) noexcept;

        MeshStatus enrollReceiver    (EndpointIdentity receiver) noexcept;
        MeshStatus enrollTransmitter (EndpointIdentity transmitter) noexcept;
        MeshStatus observeSource     (SourceIdentity source) noexcept;
        MeshStatus observeSink       (SinkIdentity sink) noexcept;

        MeshStatus requestRoute (uint64_t tick, SourceIdentity source,
                                 SinkIdentity sink) noexcept;
        MeshStatus requestBlank (uint64_t tick, SourceIdentity source,
                                 SinkIdentity sink) noexcept;
        MeshStatus confirmEdid (uint64_t tick, SourceIdentity source, SinkIdentity sink,
                                uint64_t epoch) noexcept;
        MeshStatus confirmHpd (uint64_t tick, SourceIdentity source, SinkIdentity sink,
                               uint64_t epoch) noexcept;
        MeshStatus confirmTrained (SourceIdentity source, SinkIdentity sink,
                                   uint64_t epoch) noexcept;
        MeshStatus confirmBlanked (SourceIdentity source, SinkIdentity sink,
                                   uint64_t epoch) noexcept;
        MeshStatus reportFault (SourceIdentity source, SinkIdentity sink,
                                uint64_t epoch) noexcept;
        MeshStatus clearFault (SourceIdentity source, SinkIdentity sink) noexcept;
        MeshStatus update     (uint64_t tick) noexcept;

        RouteSnapshot route       (SourceId source, SinkId sink) const noexcept;
        uint64_t      sourceEpoch (SourceId source) const noexcept;

      private:
        struct NodeRecord
        {
            EndpointIdentity identity;
            bool             enrolled;
        };

        struct SourceRecord
        {
            SourceIdentity identity;
            uint64_t       epoch;
            bool           observed;
        };

        struct SinkRecord
        {
            SinkIdentity identity;
            bool         observed;
        };

        struct RouteRecord
        {
            RouteSnapshot route;
            bool          used;
        };

        MeshStatus         validateSource (SourceIdentity source) const noexcept;
        MeshStatus         validateSink   (SinkIdentity sink) const noexcept;
        MeshStatus         advanceEpoch   (SourceRecord& source) noexcept;
        void               stampRoutes    (SourceId source, uint64_t epoch) noexcept;
        RouteRecord*       findRoute      (SourceId source, SinkId sink) noexcept;
        const RouteRecord* findRoute      (SourceId source,
                                           SinkId sink) const noexcept;
        RouteRecord*       freeRoute      () noexcept;
        bool               sinkReserved   (SinkId sink) const noexcept;
        bool               sameEndpoint   (EndpointIdentity left,
                                           EndpointIdentity right) const noexcept;
        bool               expired        (uint64_t tick,
                                           uint64_t deadline) const noexcept;

        MeshConfig                               config_;
        std::array<NodeRecord, maximumNodes>     receivers_;
        std::array<NodeRecord, maximumNodes>     transmitters_;
        std::array<SourceRecord, maximumSources> sources_;
        std::array<SinkRecord, maximumSinks>     sinks_;
        std::array<RouteRecord, maximumRoutes>   routes_;
    };

    const char* meshStatusName (MeshStatus status) noexcept;
    const char* routeStateName (RouteState state) noexcept;
} // namespace adk::hdmimesh
