#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace adk::usbmesh::product {

    struct CauId
    {
        uint16_t value;
    };

    struct PauId
    {
        uint16_t value;
    };

    struct PeripheralPortId
    {
        uint8_t value;
    };

    struct TopologyId
    {
        uint16_t value;
    };

    struct RouteId
    {
        uint16_t value;
    };

    struct ProfileId
    {
        uint8_t value;
    };

    struct TopologyIdentity
    {
        PauId            pau;
        PeripheralPortId port;
        TopologyId       topology;
        uint64_t         generation;
        uint64_t         descriptorDigest;
    };

    enum struct ProfileSelection : uint8_t
    {
        Pinned,
        Allowed,
        BestWithinBounds
    };

    enum struct FailurePolicy : uint8_t
    {
        AutomaticRecovery,
        ManualRecovery
    };

    struct RouteProfile
    {
        ProfileId        requested;
        ProfileId        active;
        ProfileSelection selection;
        FailurePolicy    failurePolicy;
        uint32_t         stabilityTicks;
    };

    enum struct PowerState : uint8_t
    {
        Unknown,
        Off,
        Discharging,
        Discharged,
        Admitted,
        On,
        Fault
    };

    struct PowerObservation
    {
        PowerState state;
        uint16_t   millivolts;
        uint16_t   milliamps;
        bool       suppliedByPau;
        bool       backfeedDetected;
    };

    struct ColdMovePlan
    {
        RouteId         route;
        CauId           oldCau;
        CauId           newCau;
        TopologyIdentity topology;
        RouteProfile    profile;
        uint64_t        controllerTerm;
        uint64_t        topologyEpoch;
        uint64_t        generation;
        uint64_t        digest;
        bool            hasOldCau;
    };

    enum struct ColdMovePhase : uint8_t
    {
        Planned,
        AwaitingDisconnect,
        AwaitingDischarge,
        AwaitingEpoch,
        AwaitingPower,
        AwaitingTopology,
        AwaitingPresentation,
        Active,
        RecoveryWait,
        Fault
    };

    enum struct EvidenceKind : uint8_t
    {
        Begin,
        OldCauDisconnected,
        PauDischarging,
        PauDischarged,
        EpochPersisted,
        PauPowerAdmitted,
        PauPowered,
        TopologyObserved,
        NewCauPresented,
        ContractLost,
        StabilityObserved,
        ManualRecoveryAuthorized,
        ControllerLost,
        ControllerRecovered,
        RealFault
    };

    struct ProductEvidence
    {
        EvidenceKind     kind;
        RouteId          route;
        CauId            cau;
        TopologyIdentity topology;
        PowerObservation power;
        uint64_t         controllerTerm;
        uint64_t         topologyEpoch;
        uint64_t         planDigest;
        uint32_t         stableTicks;
    };

    enum struct ProductStatus : uint8_t
    {
        Ok,
        InvalidIdentity,
        InvalidConfiguration,
        InvalidDigest,
        StaleEvidence,
        WrongPhase,
        UnsafePower,
        ControllerUnavailable,
        GenerationExhausted,
        JournalFull
    };

    enum struct VisualMode : uint8_t
    {
        Startup,
        Normal,
        Night,
        Attention,
        Fault,
        Test,
        ControllerLost,
        Maintenance
    };

    enum struct JournalEvent : uint8_t
    {
        PlanAccepted,
        EvidenceAccepted,
        PhaseChanged,
        ControllerLost,
        ControllerRecovered,
        RouteActive,
        RouteFaulted
    };

    struct JournalRecord
    {
        uint32_t      sequence;
        JournalEvent event;
        RouteId      route;
        ColdMovePhase phase;
        uint64_t      planDigest;
        uint64_t      chainDigest;
    };

    struct FakeJournal
    {
        static constexpr std::size_t maximumRecords = 64;

        FakeJournal () noexcept;

        ProductStatus append (JournalEvent event, RouteId route,
                              ColdMovePhase phase,
                              uint64_t planDigest) noexcept;
        std::size_t   size   () const noexcept;
        JournalRecord record (std::size_t index) const noexcept;
        uint64_t      digest () const noexcept;

      private:
        std::array<JournalRecord, maximumRecords> records_;
        std::size_t                               size_;
        uint64_t                                  digest_;
    };

    struct ColdMoveState
    {
        ColdMovePlan plan;
        ColdMovePhase phase;
        PowerObservation power;
        VisualMode   visualMode;
        uint64_t     acceptedTopologyEpoch;
        uint64_t     controllerTerm;
        uint32_t     stableTicks;
        bool         controllerAvailable;
        bool         topologyMatches;
        bool         valid;
    };

    uint64_t      coldMoveDigest   (const ColdMovePlan& plan) noexcept;
    ProductStatus validateColdMove (const ColdMovePlan& plan) noexcept;
    ProductStatus beginColdMove    (const ColdMovePlan& plan,
                                    ColdMoveState& state,
                                    FakeJournal& journal) noexcept;
    ProductStatus observeColdMove  (const ProductEvidence& evidence,
                                    ColdMoveState& state,
                                    FakeJournal& journal) noexcept;
    const char*   coldMovePhaseName (ColdMovePhase phase) noexcept;
    const char*   visualModeName    (VisualMode mode) noexcept;
} // namespace adk::usbmesh::product
