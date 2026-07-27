#include "route_profile.h"

namespace adk::research {

    namespace {

        AppliedProfileEvidence unavailableEvidence (const ProfilePolicy& policy,
                                                    SelectionStatus status) noexcept
        {
            const std::uint16_t requested =
                (policy.kind == ProfilePolicyKind::OrderedFallback) &&
                        (policy.orderedProfileCount > 0)
                    ? policy.orderedProfileIds[0]
                    : policy.pinnedProfileId;

            return AppliedProfileEvidence {
                status,
                requested,
                0,
                0,
                false
            };
        }

        AppliedProfileEvidence appliedEvidence (const ProfilePolicy& policy,
                                                const RouteProfile& profile,
                                                bool fallback) noexcept
        {
            const std::uint16_t requested =
                (policy.kind == ProfilePolicyKind::OrderedFallback) &&
                        (policy.orderedProfileCount > 0)
                    ? policy.orderedProfileIds[0]
                    : policy.pinnedProfileId;

            return AppliedProfileEvidence {
                SelectionStatus::Applied,
                requested,
                profile.id,
                profile.qualityRank,
                fallback
            };
        }

    }

    bool ProfileCatalog::add (const RouteProfile& profile) noexcept
    {
        if ((profile.id == 0) || (count >= ProfileCapacity) ||
            (find (profile.id) != nullptr))
        {
            return false;
        }

        profiles[count] = profile;
        ++count;
        return true;
    }

    const RouteProfile* ProfileCatalog::find (std::uint16_t profileId) const noexcept
    {
        for (std::size_t index = 0; index < count; ++index)
        {
            if (profiles[index].id == profileId)
            {
                return &profiles[index];
            }
        }

        return nullptr;
    }

    bool profileFits (const RouteProfile& profile, const ProfileBounds& bounds,
                      const PathObservation& path) noexcept
    {
        return path.available && !path.realFault &&
               (profile.bandwidthKbps <= path.capacityKbps) &&
               (path.latencyUs <= profile.maximumLatencyUs) &&
               (path.jitterUs <= profile.maximumJitterUs) &&
               (profile.bandwidthKbps <= bounds.maximumBandwidthKbps) &&
               (path.latencyUs <= bounds.maximumLatencyUs) &&
               (path.jitterUs <= bounds.maximumJitterUs) &&
               (profile.qualityRank >= bounds.minimumQualityRank);
    }

    AppliedProfileEvidence RouteProfileSelector::select (
        const ProfileCatalog& catalog, const ProfilePolicy& policy,
        const PathObservation& path) const noexcept
    {
        if (policy.kind == ProfilePolicyKind::Pinned)
        {
            const RouteProfile* profile = catalog.find (policy.pinnedProfileId);

            if ((profile == nullptr) || (profile->payload != policy.payload) ||
                !profileFits (*profile, policy.bounds, path))
            {
                return unavailableEvidence (
                    policy, SelectionStatus::PinnedProfileUnavailable);
            }

            return appliedEvidence (policy, *profile, false);
        }

        if (policy.kind == ProfilePolicyKind::OrderedFallback)
        {
            if (policy.orderedProfileCount > OrderedProfileCapacity)
            {
                return unavailableEvidence (policy, SelectionStatus::InvalidPolicy);
            }

            for (std::size_t index = 0; index < policy.orderedProfileCount; ++index)
            {
                const RouteProfile* profile =
                    catalog.find (policy.orderedProfileIds[index]);

                if ((profile != nullptr) && (profile->payload == policy.payload) &&
                    profileFits (*profile, policy.bounds, path))
                {
                    return appliedEvidence (policy, *profile, index != 0);
                }
            }

            return unavailableEvidence (policy, SelectionStatus::NoMatchingProfile);
        }

        if (policy.kind == ProfilePolicyKind::BestWithinBounds)
        {
            const RouteProfile* best = nullptr;

            for (std::size_t index = 0; index < catalog.count; ++index)
            {
                const RouteProfile& candidate = catalog.profiles[index];

                if ((candidate.payload == policy.payload) &&
                    profileFits (candidate, policy.bounds, path) &&
                    ((best == nullptr) ||
                     (candidate.qualityRank > best->qualityRank) ||
                     ((candidate.qualityRank == best->qualityRank) &&
                      (candidate.id < best->id))))
                {
                    best = &candidate;
                }
            }

            if (best != nullptr)
            {
                return appliedEvidence (policy, *best, false);
            }

            return unavailableEvidence (policy, SelectionStatus::NoMatchingProfile);
        }

        return unavailableEvidence (policy, SelectionStatus::InvalidPolicy);
    }

    RecoveryTracker::RecoveryTracker () noexcept
        : waiting_        (false),
          healthySinceMs_ (0)
    {
    }

    void RecoveryTracker::reset () noexcept
    {
        waiting_        = false;
        healthySinceMs_ = 0;
    }

    bool RecoveryTracker::update (std::uint32_t nowMs, bool contractHealthy,
                                  std::uint32_t stableIntervalMs) noexcept
    {
        if (!contractHealthy)
        {
            reset ();
            return false;
        }

        if (!waiting_)
        {
            waiting_        = true;
            healthySinceMs_ = nowMs;
        }

        return static_cast<std::uint32_t> (nowMs - healthySinceMs_) >=
               stableIntervalMs;
    }

    FailureDecision decideFailure (PayloadKind payload,
                                   const ProductionFailurePolicy& policy,
                                   bool activeContractLost) noexcept
    {
        const FailureAction action = activeContractLost
                                         ? policy.contractLoss
                                         : policy.admissionFailure;

        return FailureDecision {
            action,
            (payload == PayloadKind::Hdmi) &&
                (action == FailureAction::BlankAndMute),
            activeContractLost &&
                ((action == FailureAction::DisconnectAndPowerOff) ||
                 (action == FailureAction::StayDisconnected)),
            (payload == PayloadKind::Usb) &&
                (action == FailureAction::DisconnectAndPowerOff)
        };
    }

    InjectedObservation injectObservation (const FaultScenario& scenario,
                                           std::uint32_t elapsedMs,
                                           const PathObservation& actual) noexcept
    {
        InjectedObservation result {actual, false, FaultKind::None};

        if (!scenario.labModeEnabled || (elapsedMs >= scenario.durationMs) ||
            (scenario.eventCount > FaultEventCapacity))
        {
            return result;
        }

        for (std::size_t index = 0; index < scenario.eventCount; ++index)
        {
            const FaultEvent& event = scenario.events[index];

            if (event.offsetMs > elapsedMs)
            {
                continue;
            }

            result.testActive = true;
            result.activeFault = event.kind;

            switch (event.kind)
            {
                case FaultKind::PathUnavailable:
                    result.path.available = false;
                    break;
                case FaultKind::CapacityKbps:
                    result.path.capacityKbps = event.value;
                    break;
                case FaultKind::LatencyUs:
                    result.path.latencyUs = event.value;
                    break;
                case FaultKind::JitterUs:
                    result.path.jitterUs = event.value;
                    break;
                case FaultKind::None:
                    break;
            }
        }

        if (actual.realFault)
        {
            result.path = actual;
        }

        return result;
    }

}
