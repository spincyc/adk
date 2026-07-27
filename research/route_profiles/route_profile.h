#ifndef ADK_RESEARCH_ROUTE_PROFILE_H
#define ADK_RESEARCH_ROUTE_PROFILE_H

#include <cstddef>
#include <cstdint>

namespace adk::research {

    constexpr std::size_t ProfileCapacity       = 16;
    constexpr std::size_t OrderedProfileCapacity = 8;
    constexpr std::size_t FaultEventCapacity     = 8;

    enum struct PayloadKind : std::uint8_t
    {
        Usb,
        Hdmi
    };

    enum struct ProfilePolicyKind : std::uint8_t
    {
        Pinned,
        OrderedFallback,
        BestWithinBounds
    };

    enum struct SelectionStatus : std::uint8_t
    {
        Applied,
        NoMatchingProfile,
        PinnedProfileUnavailable,
        InvalidPolicy
    };

    enum struct FailureAction : std::uint8_t
    {
        PreserveExisting,
        BlankAndMute,
        DisconnectAndPowerOff,
        StayDisconnected
    };

    enum struct FaultKind : std::uint8_t
    {
        None,
        PathUnavailable,
        CapacityKbps,
        LatencyUs,
        JitterUs
    };

    struct RouteProfile
    {
        std::uint16_t id;
        PayloadKind   payload;
        std::uint32_t bandwidthKbps;
        std::uint32_t maximumLatencyUs;
        std::uint32_t maximumJitterUs;
        std::uint16_t qualityRank;
        std::uint32_t stableReconnectMs;
    };

    struct PathObservation
    {
        bool          available;
        std::uint32_t capacityKbps;
        std::uint32_t latencyUs;
        std::uint32_t jitterUs;
        bool          realFault;
    };

    struct ProfileBounds
    {
        std::uint32_t maximumBandwidthKbps;
        std::uint32_t maximumLatencyUs;
        std::uint32_t maximumJitterUs;
        std::uint16_t minimumQualityRank;
    };

    struct ProfilePolicy
    {
        ProfilePolicyKind kind;
        PayloadKind       payload;
        std::uint16_t     pinnedProfileId;
        std::uint16_t     orderedProfileIds[OrderedProfileCapacity];
        std::size_t       orderedProfileCount;
        ProfileBounds     bounds;
    };

    struct AppliedProfileEvidence
    {
        SelectionStatus status;
        std::uint16_t   requestedProfileId;
        std::uint16_t   appliedProfileId;
        std::uint16_t   appliedQualityRank;
        bool            fallbackApplied;
    };

    struct ProductionFailurePolicy
    {
        FailureAction contractLoss;
        FailureAction admissionFailure;
        bool          automaticReconnect;
    };

    struct FailureDecision
    {
        FailureAction action;
        bool          retainPinnedProfile;
        bool          advanceRouteEpoch;
        bool          removePeripheralPower;
    };

    struct FaultEvent
    {
        std::uint32_t offsetMs;
        FaultKind     kind;
        std::uint32_t value;
    };

    struct FaultScenario
    {
        bool          labModeEnabled;
        std::uint32_t durationMs;
        FaultEvent    events[FaultEventCapacity];
        std::size_t   eventCount;
    };

    struct InjectedObservation
    {
        PathObservation path;
        bool            testActive;
        FaultKind       activeFault;
    };

    struct ProfileCatalog
    {
        RouteProfile profiles[ProfileCapacity];
        std::size_t  count;

        bool                add  (const RouteProfile& profile) noexcept;
        const RouteProfile* find (std::uint16_t profileId) const noexcept;
    };

    struct RouteProfileSelector
    {
        AppliedProfileEvidence select (const ProfileCatalog& catalog,
                                       const ProfilePolicy& policy,
                                       const PathObservation& path) const noexcept;
    };

    struct RecoveryTracker
    {
        RecoveryTracker () noexcept;

        void reset  () noexcept;
        bool update (std::uint32_t nowMs, bool contractHealthy,
                     std::uint32_t stableIntervalMs) noexcept;

    private:
        bool          waiting_;
        std::uint32_t healthySinceMs_;
    };

    bool profileFits (const RouteProfile& profile, const ProfileBounds& bounds,
                      const PathObservation& path) noexcept;

    FailureDecision decideFailure (PayloadKind payload,
                                   const ProductionFailurePolicy& policy,
                                   bool activeContractLost) noexcept;

    InjectedObservation injectObservation (const FaultScenario& scenario,
                                           std::uint32_t elapsedMs,
                                           const PathObservation& actual) noexcept;

}

#endif
