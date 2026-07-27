#include "route_profile.h"

#include <cstdlib>
#include <iostream>

namespace {

    using namespace adk::research;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    RouteProfile profile (std::uint16_t id, PayloadKind payload,
                          std::uint32_t bandwidth, std::uint16_t quality,
                          std::uint32_t reconnect)
    {
        return RouteProfile {id, payload, bandwidth, 10000, 2000, quality,
                             reconnect};
    }

    ProfileBounds broadBounds ()
    {
        return ProfileBounds {100000000, 10000, 2000, 0};
    }

    PathObservation healthyPath ()
    {
        return PathObservation {true, 20000000, 1000, 100, false};
    }

    void testPinnedProfileNeverFallsBack ()
    {
        ProfileCatalog catalog {};

        require (catalog.add (profile (1, PayloadKind::Hdmi, 30000000, 100, 0)),
                 "add pinned profile");
        require (catalog.add (profile (2, PayloadKind::Hdmi, 10000000, 50, 0)),
                 "add lower profile");

        ProfilePolicy policy {};

        policy.kind            = ProfilePolicyKind::Pinned;
        policy.payload         = PayloadKind::Hdmi;
        policy.pinnedProfileId = 1;
        policy.bounds          = broadBounds ();

        const AppliedProfileEvidence evidence =
            RouteProfileSelector ().select (catalog, policy, healthyPath ());

        require (evidence.status == SelectionStatus::PinnedProfileUnavailable,
                 "unavailable pin fails");
        require (evidence.appliedProfileId == 0, "pin does not fall back");
    }

    void testOrderedFallbackUsesDeclaredOrder ()
    {
        ProfileCatalog catalog {};

        require (catalog.add (profile (1, PayloadKind::Hdmi, 30000000, 100, 0)),
                 "add first profile");
        require (catalog.add (profile (2, PayloadKind::Hdmi, 10000000, 50, 0)),
                 "add fallback profile");

        ProfilePolicy policy {};

        policy.kind                 = ProfilePolicyKind::OrderedFallback;
        policy.payload              = PayloadKind::Hdmi;
        policy.orderedProfileIds[0] = 1;
        policy.orderedProfileIds[1] = 2;
        policy.orderedProfileCount  = 2;
        policy.bounds               = broadBounds ();

        const AppliedProfileEvidence evidence =
            RouteProfileSelector ().select (catalog, policy, healthyPath ());

        require (evidence.status == SelectionStatus::Applied, "fallback applies");
        require (evidence.appliedProfileId == 2, "declared fallback selected");
        require (evidence.fallbackApplied, "fallback evidence is visible");
    }

    void testBestProfileIsDeterministic ()
    {
        ProfileCatalog catalog {};

        require (catalog.add (profile (7, PayloadKind::Usb, 1000, 40, 1000)),
                 "add equal profile");
        require (catalog.add (profile (3, PayloadKind::Usb, 1000, 40, 1000)),
                 "add lower id equal profile");
        require (catalog.add (profile (9, PayloadKind::Usb, 500, 20, 1000)),
                 "add lower quality profile");

        ProfilePolicy policy {};

        policy.kind    = ProfilePolicyKind::BestWithinBounds;
        policy.payload = PayloadKind::Usb;
        policy.bounds  = broadBounds ();

        const AppliedProfileEvidence evidence =
            RouteProfileSelector ().select (catalog, policy, healthyPath ());

        require (evidence.appliedProfileId == 3,
                 "equal quality resolves by stable id");
    }

    void testFailurePoliciesRemainPayloadSpecific ()
    {
        const ProductionFailurePolicy hdmi {
            FailureAction::BlankAndMute,
            FailureAction::PreserveExisting,
            true
        };
        const ProductionFailurePolicy usb {
            FailureAction::DisconnectAndPowerOff,
            FailureAction::PreserveExisting,
            true
        };

        const FailureDecision hdmiDecision =
            decideFailure (PayloadKind::Hdmi, hdmi, true);
        const FailureDecision usbDecision =
            decideFailure (PayloadKind::Usb, usb, true);
        const FailureDecision planFailure =
            decideFailure (PayloadKind::Usb, usb, false);

        require (hdmiDecision.retainPinnedProfile, "HDMI pin retained");
        require (!hdmiDecision.removePeripheralPower, "HDMI does not remove VBUS");
        require (usbDecision.advanceRouteEpoch, "USB failure advances epoch");
        require (usbDecision.removePeripheralPower, "USB failure removes VBUS");
        require (planFailure.action == FailureAction::PreserveExisting,
                 "failed plan preserves working route");
    }

    void testRecoveryRequiresContinuousStableIntervalAndWraps ()
    {
        RecoveryTracker recovery;

        require (!recovery.update (100, true, 5), "stability begins");
        require (!recovery.update (104, true, 5), "one tick early");
        require (recovery.update (105, true, 5), "exact stability boundary");
        require (!recovery.update (106, false, 5), "regression resets interval");
        require (!recovery.update (0xfffffffeU, true, 4), "wrap interval begins");
        require (!recovery.update (1, true, 4), "wrap one tick early");
        require (recovery.update (2, true, 4), "wrap exact boundary");
    }

    void testFaultInjectionIsBoundedAndRealFaultDominates ()
    {
        FaultScenario scenario {};

        scenario.labModeEnabled = true;
        scenario.durationMs     = 100;
        scenario.events[0]      = FaultEvent {10, FaultKind::CapacityKbps, 50};
        scenario.events[1]      = FaultEvent {20, FaultKind::LatencyUs, 9000};
        scenario.eventCount     = 2;

        const PathObservation actual = healthyPath ();
        const InjectedObservation before =
            injectObservation (scenario, 9, actual);
        const InjectedObservation during =
            injectObservation (scenario, 25, actual);
        const InjectedObservation after =
            injectObservation (scenario, 100, actual);

        require (!before.testActive, "test inactive before event");
        require (during.testActive, "test indication visible");
        require (during.path.capacityKbps == 50, "capacity injected");
        require (during.path.latencyUs == 9000, "latency injected");
        require (!after.testActive, "test stops at duration");

        PathObservation realFault = actual;

        realFault.available = false;
        realFault.realFault = true;

        const InjectedObservation dominated =
            injectObservation (scenario, 25, realFault);

        require (!dominated.path.available, "real fault remains authoritative");
        require (dominated.path.realFault, "real fault evidence retained");
    }

    void testCapacityIsFixed ()
    {
        ProfileCatalog catalog {};

        for (std::size_t index = 0; index < ProfileCapacity; ++index)
        {
            require (catalog.add (profile (static_cast<std::uint16_t> (index + 1),
                                          PayloadKind::Usb, 1, 1, 1)),
                     "catalog entry fits");
        }

        require (!catalog.add (profile (99, PayloadKind::Usb, 1, 1, 1)),
                 "catalog rejects exhaustion");
        require (!catalog.add (catalog.profiles[0]), "catalog rejects duplicate");
    }

}

int main ()
{
    testPinnedProfileNeverFallsBack                      ();
    testOrderedFallbackUsesDeclaredOrder                 ();
    testBestProfileIsDeterministic                       ();
    testFailurePoliciesRemainPayloadSpecific             ();
    testRecoveryRequiresContinuousStableIntervalAndWraps ();
    testFaultInjectionIsBoundedAndRealFaultDominates     ();
    testCapacityIsFixed                                  ();
    return 0;
}
