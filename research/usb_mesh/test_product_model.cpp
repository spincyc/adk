#include "product_model.h"

#include <cstdlib>
#include <iostream>

namespace {

    using namespace adk::usbmesh::product;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    void requireStatus (ProductStatus actual, ProductStatus expected,
                        const char* message)
    {
        require (actual == expected, message);
    }

    ColdMovePlan makePlan (FailurePolicy policy =
                               FailurePolicy::AutomaticRecovery)
    {
        ColdMovePlan plan {
            RouteId {7},
            CauId {1},
            CauId {2},
            TopologyIdentity {
                PauId {3}, PeripheralPortId {4}, TopologyId {5}, 6, 0x1234},
            RouteProfile {
                ProfileId {2},
                ProfileId {2},
                ProfileSelection::Pinned,
                policy,
                10},
            8,
            9,
            10,
            0,
            true};
        plan.digest = coldMoveDigest (plan);
        return plan;
    }

    ProductEvidence makeEvidence (const ColdMoveState& state,
                                  EvidenceKind kind)
    {
        return ProductEvidence {
            kind,
            state.plan.route,
            state.plan.newCau,
            state.plan.topology,
            PowerObservation {PowerState::Unknown, 0, 0, true, false},
            state.controllerTerm,
            state.plan.topologyEpoch,
            state.plan.digest,
            0};
    }

    void observe (ColdMoveState& state, FakeJournal& journal,
                  ProductEvidence evidence, ColdMovePhase expected,
                  const char* message)
    {
        requireStatus (
            observeColdMove (evidence, state, journal), ProductStatus::Ok,
            message);
        require (state.phase == expected, message);
    }

    struct Fixture
    {
        ColdMovePlan  plan;
        ColdMoveState state;
        FakeJournal   journal;

        Fixture ()
            : plan    (makePlan ()),
              state   {},
              journal ()
        {
            requireStatus (
                beginColdMove (plan, state, journal), ProductStatus::Ok,
                "begin valid plan");
        }
    };

    void boundedIdentitiesAndDigestAreValidated ()
    {
        ColdMovePlan valid = makePlan ();

        requireStatus (
            validateColdMove (valid), ProductStatus::Ok, "valid plan");

        ColdMovePlan invalidPort = valid;
        invalidPort.topology.port.value = 5;
        invalidPort.digest              = coldMoveDigest (invalidPort);

        requireStatus (
            validateColdMove (invalidPort), ProductStatus::InvalidIdentity,
            "port bound");

        ColdMovePlan invalidPinned = valid;
        invalidPinned.profile.active.value = 3;
        invalidPinned.digest = coldMoveDigest (invalidPinned);

        requireStatus (
            validateColdMove (invalidPinned),
            ProductStatus::InvalidConfiguration,
            "pinned profile is exact");

        ColdMovePlan changed = valid;
        ++changed.generation;
        requireStatus (
            validateColdMove (changed), ProductStatus::InvalidDigest,
            "digest fences changed plan");
    }

    void coldMoveRequiresCompleteOrderedEvidence ()
    {
        Fixture fixture;

        observe (
            fixture.state, fixture.journal,
            makeEvidence (fixture.state, EvidenceKind::Begin),
            ColdMovePhase::AwaitingDisconnect, "begin waits for disconnect");

        ProductEvidence disconnected =
            makeEvidence (fixture.state, EvidenceKind::OldCauDisconnected);
        disconnected.cau = fixture.plan.oldCau;
        observe (
            fixture.state, fixture.journal, disconnected,
            ColdMovePhase::AwaitingDischarge, "old Cau disconnected");

        ProductEvidence discharging =
            makeEvidence (fixture.state, EvidenceKind::PauDischarging);
        discharging.power =
            PowerObservation {PowerState::Discharging, 1700, 8, true, false};
        observe (
            fixture.state, fixture.journal, discharging,
            ColdMovePhase::AwaitingDischarge, "discharge is observable");

        ProductEvidence discharged =
            makeEvidence (fixture.state, EvidenceKind::PauDischarged);
        discharged.power =
            PowerObservation {PowerState::Discharged, 0, 0, true, false};
        observe (
            fixture.state, fixture.journal, discharged,
            ColdMovePhase::AwaitingEpoch, "discharge proven");

        observe (
            fixture.state, fixture.journal,
            makeEvidence (fixture.state, EvidenceKind::EpochPersisted),
            ColdMovePhase::AwaitingPower, "epoch persisted");

        ProductEvidence admitted =
            makeEvidence (fixture.state, EvidenceKind::PauPowerAdmitted);
        admitted.power =
            PowerObservation {PowerState::Admitted, 0, 0, true, false};
        observe (
            fixture.state, fixture.journal, admitted,
            ColdMovePhase::AwaitingPower, "power admission observed");

        ProductEvidence powered =
            makeEvidence (fixture.state, EvidenceKind::PauPowered);
        powered.power =
            PowerObservation {PowerState::On, 5000, 400, true, false};
        observe (
            fixture.state, fixture.journal, powered,
            ColdMovePhase::AwaitingTopology, "protected power observed");

        observe (
            fixture.state, fixture.journal,
            makeEvidence (fixture.state, EvidenceKind::TopologyObserved),
            ColdMovePhase::AwaitingPresentation, "topology exact");
        observe (
            fixture.state, fixture.journal,
            makeEvidence (fixture.state, EvidenceKind::NewCauPresented),
            ColdMovePhase::Active, "new Cau presented");

        require (fixture.state.visualMode == VisualMode::Normal,
                 "active visual mode");
        require (fixture.state.topologyMatches, "topology match retained");
    }

    void staleAndOutOfOrderEvidenceCannotAdvance ()
    {
        Fixture fixture;
        ProductEvidence early =
            makeEvidence (fixture.state, EvidenceKind::PauPowered);
        const std::size_t before = fixture.journal.size ();

        requireStatus (
            observeColdMove (early, fixture.state, fixture.journal),
            ProductStatus::WrongPhase,
            "power before disconnect rejected");
        require (fixture.state.phase == ColdMovePhase::Planned,
                 "rejected evidence preserves phase");
        require (fixture.journal.size () == before,
                 "rejected evidence is not journaled as accepted");

        ProductEvidence stale =
            makeEvidence (fixture.state, EvidenceKind::Begin);
        --stale.topologyEpoch;
        requireStatus (
            observeColdMove (stale, fixture.state, fixture.journal),
            ProductStatus::StaleEvidence, "stale epoch rejected");
    }

    void unsafePowerFailsClosed ()
    {
        Fixture fixture;
        ProductEvidence unsafe =
            makeEvidence (fixture.state, EvidenceKind::RealFault);
        unsafe.power =
            PowerObservation {PowerState::Fault, 5100, 900, true, true};

        requireStatus (
            observeColdMove (unsafe, fixture.state, fixture.journal),
            ProductStatus::UnsafePower, "backfeed fault reported");
        require (fixture.state.phase == ColdMovePhase::Fault,
                 "unsafe power faults route");
        require (fixture.state.power.state == PowerState::Off,
                 "unsafe power model fails off");
        require (fixture.state.visualMode == VisualMode::Fault,
                 "fault is locally visible");
    }

    void controllerLossRequiresNewerAuthority ()
    {
        Fixture fixture;
        ProductEvidence lost =
            makeEvidence (fixture.state, EvidenceKind::ControllerLost);

        requireStatus (
            observeColdMove (lost, fixture.state, fixture.journal),
            ProductStatus::Ok, "controller loss accepted");
        require (!fixture.state.controllerAvailable, "authority unavailable");
        require (fixture.state.visualMode == VisualMode::ControllerLost,
                 "controller loss visible");

        ProductEvidence begin =
            makeEvidence (fixture.state, EvidenceKind::Begin);
        requireStatus (
            observeColdMove (begin, fixture.state, fixture.journal),
            ProductStatus::ControllerUnavailable,
            "no mutation without controller");

        ProductEvidence staleRecovery =
            makeEvidence (fixture.state, EvidenceKind::ControllerRecovered);
        requireStatus (
            observeColdMove (
                staleRecovery, fixture.state, fixture.journal),
            ProductStatus::StaleEvidence, "same term cannot recover");

        ProductEvidence recovered = staleRecovery;
        ++recovered.controllerTerm;
        requireStatus (
            observeColdMove (recovered, fixture.state, fixture.journal),
            ProductStatus::Ok, "new term recovers authority");
        require (fixture.state.phase == ColdMovePhase::RecoveryWait,
                 "recovery does not restore active route");
    }

    void automaticAndManualRecoveryRemainDistinct ()
    {
        Fixture automatic;
        automatic.state.phase      = ColdMovePhase::Active;
        automatic.state.visualMode = VisualMode::Normal;

        observe (
            automatic.state, automatic.journal,
            makeEvidence (automatic.state, EvidenceKind::ContractLost),
            ColdMovePhase::RecoveryWait, "automatic route waits");

        ProductEvidence partial =
            makeEvidence (automatic.state, EvidenceKind::StabilityObserved);
        partial.stableTicks = 9;
        observe (
            automatic.state, automatic.journal, partial,
            ColdMovePhase::RecoveryWait, "partial stability retained");

        ProductEvidence complete = partial;
        complete.stableTicks      = 10;
        observe (
            automatic.state, automatic.journal, complete,
            ColdMovePhase::AwaitingDischarge, "full stability restarts cold");

        ColdMovePlan  manualPlan = makePlan (FailurePolicy::ManualRecovery);
        ColdMoveState manualState {};
        FakeJournal   manualJournal;
        requireStatus (
            beginColdMove (manualPlan, manualState, manualJournal),
            ProductStatus::Ok, "manual plan");
        manualState.phase      = ColdMovePhase::Active;
        manualState.visualMode = VisualMode::Normal;

        observe (
            manualState, manualJournal,
            makeEvidence (manualState, EvidenceKind::ContractLost),
            ColdMovePhase::Fault, "manual route faults");
        observe (
            manualState, manualJournal,
            makeEvidence (
                manualState, EvidenceKind::ManualRecoveryAuthorized),
            ColdMovePhase::AwaitingDischarge,
            "manual authorization restarts cold");
    }

    void journalIsBoundedAppendOnlyAndDeterministic ()
    {
        FakeJournal first;
        FakeJournal second;

        for (std::size_t index = 0; index < FakeJournal::maximumRecords;
             ++index)
        {
            requireStatus (
                first.append (
                    JournalEvent::EvidenceAccepted, RouteId {1},
                    ColdMovePhase::Planned, 2),
                ProductStatus::Ok, "fill first journal");
            requireStatus (
                second.append (
                    JournalEvent::EvidenceAccepted, RouteId {1},
                    ColdMovePhase::Planned, 2),
                ProductStatus::Ok, "fill second journal");
        }

        require (first.digest () == second.digest (),
                 "identical journals have identical digest");
        const uint64_t fullDigest = first.digest ();

        requireStatus (
            first.append (
                JournalEvent::RouteFaulted, RouteId {1},
                ColdMovePhase::Fault, 2),
            ProductStatus::JournalFull, "journal capacity is explicit");
        require (first.size () == FakeJournal::maximumRecords,
                 "full journal is unchanged");
        require (first.digest () == fullDigest,
                 "rejected append preserves digest");
        require (first.record (0).sequence == 1,
                 "journal sequence begins at one");
        require (first.record (FakeJournal::maximumRecords - 1).sequence ==
                     FakeJournal::maximumRecords,
                 "journal order is stable");
    }
} // namespace

int main ()
{
    boundedIdentitiesAndDigestAreValidated           ();
    coldMoveRequiresCompleteOrderedEvidence          ();
    staleAndOutOfOrderEvidenceCannotAdvance          ();
    unsafePowerFailsClosed                           ();
    controllerLossRequiresNewerAuthority             ();
    automaticAndManualRecoveryRemainDistinct         ();
    journalIsBoundedAppendOnlyAndDeterministic       ();
    return 0;
}
