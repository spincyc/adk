#include "product_model.h"

#include <limits>

namespace adk::usbmesh::product {

    namespace {

        constexpr uint16_t maximumCaus       = 256;
        constexpr uint16_t maximumPaus       = 256;
        constexpr uint8_t  portsPerPau       = 4;
        constexpr uint16_t maximumTopologies = 1024;
        constexpr uint16_t maximumRoutes     = 1024;
        constexpr uint8_t  maximumProfiles   = 32;
        constexpr uint64_t digestSeed        = UINT64_C (1469598103934665603);
        constexpr uint64_t digestPrime       = UINT64_C (1099511628211);

        uint64_t mix (uint64_t digest, uint64_t value) noexcept
        {
            for (uint8_t byte = 0; byte < 8; ++byte)
            {
                digest ^= value & UINT64_C (0xff);
                digest *= digestPrime;
                value >>= 8;
            }

            return digest;
        }

        bool sameTopology (TopologyIdentity left,
                           TopologyIdentity right) noexcept
        {
            return left.pau.value == right.pau.value &&
                   left.port.value == right.port.value &&
                   left.topology.value == right.topology.value &&
                   left.generation == right.generation &&
                   left.descriptorDigest == right.descriptorDigest;
        }

        bool sameRouteEvidence (const ProductEvidence& evidence,
                                const ColdMoveState& state) noexcept
        {
            return evidence.route.value == state.plan.route.value &&
                   evidence.planDigest == state.plan.digest;
        }

        bool unsafePower (PowerObservation power) noexcept
        {
            return power.backfeedDetected || power.state == PowerState::Fault;
        }

        ProductStatus appendTransition (ColdMoveState& state,
                                        FakeJournal& journal,
                                        JournalEvent event) noexcept
        {
            return journal.append (
                event, state.plan.route, state.phase, state.plan.digest);
        }

        ProductStatus changePhase (ColdMovePhase phase, ColdMoveState& state,
                                   FakeJournal& journal) noexcept
        {
            JournalEvent event = JournalEvent::PhaseChanged;
            if (phase == ColdMovePhase::Active)
            {
                event = JournalEvent::RouteActive;
            }
            else if (phase == ColdMovePhase::Fault)
            {
                event = JournalEvent::RouteFaulted;
            }

            const ProductStatus status = journal.append (
                event, state.plan.route, phase, state.plan.digest);

            if (status != ProductStatus::Ok)
            {
                return status;
            }

            state.phase = phase;
            state.visualMode =
                phase == ColdMovePhase::Active
                    ? VisualMode::Normal
                    : (phase == ColdMovePhase::Fault ? VisualMode::Fault
                                                     : VisualMode::Attention);
            return ProductStatus::Ok;
        }

        ProductStatus enterFault (ColdMoveState& state,
                                  FakeJournal& journal,
                                  ProductStatus status) noexcept
        {
            const ProductStatus journalStatus =
                changePhase (ColdMovePhase::Fault, state, journal);

            if (journalStatus != ProductStatus::Ok)
            {
                return journalStatus;
            }

            state.power.state = PowerState::Off;
            return status;
        }
    } // namespace

    FakeJournal::FakeJournal () noexcept
        : records_ {},
          size_    (0),
          digest_  (digestSeed)
    {
    }

    ProductStatus FakeJournal::append (JournalEvent event, RouteId route,
                                       ColdMovePhase phase,
                                       uint64_t planDigest) noexcept
    {
        if (size_ == maximumRecords)
        {
            return ProductStatus::JournalFull;
        }

        uint64_t nextDigest = mix (digest_, size_ + 1);
        nextDigest          = mix (nextDigest, static_cast<uint8_t> (event));
        nextDigest          = mix (nextDigest, route.value);
        nextDigest          = mix (nextDigest, static_cast<uint8_t> (phase));
        nextDigest          = mix (nextDigest, planDigest);

        records_[size_] = JournalRecord {
            static_cast<uint32_t> (size_ + 1),
            event,
            route,
            phase,
            planDigest,
            nextDigest};
        ++size_;
        digest_ = nextDigest;
        return ProductStatus::Ok;
    }

    std::size_t FakeJournal::size () const noexcept
    {
        return size_;
    }

    JournalRecord FakeJournal::record (std::size_t index) const noexcept
    {
        return index < size_ ? records_[index] : JournalRecord {};
    }

    uint64_t FakeJournal::digest () const noexcept
    {
        return digest_;
    }

    uint64_t coldMoveDigest (const ColdMovePlan& plan) noexcept
    {
        uint64_t digest = digestSeed;

        digest = mix (digest, plan.route.value);
        digest = mix (digest, plan.oldCau.value);
        digest = mix (digest, plan.newCau.value);
        digest = mix (digest, plan.topology.pau.value);
        digest = mix (digest, plan.topology.port.value);
        digest = mix (digest, plan.topology.topology.value);
        digest = mix (digest, plan.topology.generation);
        digest = mix (digest, plan.topology.descriptorDigest);
        digest = mix (digest, plan.profile.requested.value);
        digest = mix (digest, plan.profile.active.value);
        digest = mix (
            digest, static_cast<uint8_t> (plan.profile.selection));
        digest = mix (
            digest, static_cast<uint8_t> (plan.profile.failurePolicy));
        digest = mix (digest, plan.profile.stabilityTicks);
        digest = mix (digest, plan.controllerTerm);
        digest = mix (digest, plan.topologyEpoch);
        digest = mix (digest, plan.generation);
        digest = mix (digest, plan.hasOldCau ? 1 : 0);
        return digest;
    }

    ProductStatus validateColdMove (const ColdMovePlan& plan) noexcept
    {
        const bool validOldCau =
            !plan.hasOldCau ||
            (plan.oldCau.value > 0 && plan.oldCau.value <= maximumCaus);
        const bool distinctCaus =
            !plan.hasOldCau || plan.oldCau.value != plan.newCau.value;

        if (plan.route.value == 0 || plan.route.value > maximumRoutes ||
            plan.newCau.value == 0 || plan.newCau.value > maximumCaus ||
            !validOldCau || !distinctCaus ||
            plan.topology.pau.value == 0 ||
            plan.topology.pau.value > maximumPaus ||
            plan.topology.port.value == 0 ||
            plan.topology.port.value > portsPerPau ||
            plan.topology.topology.value == 0 ||
            plan.topology.topology.value > maximumTopologies ||
            plan.profile.requested.value == 0 ||
            plan.profile.requested.value > maximumProfiles ||
            plan.profile.active.value == 0 ||
            plan.profile.active.value > maximumProfiles)
        {
            return ProductStatus::InvalidIdentity;
        }

        if (plan.topology.generation == 0 ||
            plan.topology.descriptorDigest == 0 ||
            plan.controllerTerm == 0 || plan.topologyEpoch == 0 ||
            plan.generation == 0 || plan.profile.stabilityTicks == 0)
        {
            return ProductStatus::InvalidConfiguration;
        }

        if (plan.profile.selection == ProfileSelection::Pinned &&
            plan.profile.requested.value != plan.profile.active.value)
        {
            return ProductStatus::InvalidConfiguration;
        }

        if (plan.digest == 0 || plan.digest != coldMoveDigest (plan))
        {
            return ProductStatus::InvalidDigest;
        }

        return ProductStatus::Ok;
    }

    ProductStatus beginColdMove (const ColdMovePlan& plan,
                                 ColdMoveState& state,
                                 FakeJournal& journal) noexcept
    {
        const ProductStatus status = validateColdMove (plan);

        if (status != ProductStatus::Ok)
        {
            return status;
        }

        ColdMoveState next {};
        next.plan                  = plan;
        next.phase                 = ColdMovePhase::Planned;
        next.power                 = PowerObservation {
            PowerState::Unknown, 0, 0, true, false};
        next.visualMode            = VisualMode::Attention;
        next.acceptedTopologyEpoch = plan.topologyEpoch - 1;
        next.controllerTerm        = plan.controllerTerm;
        next.stableTicks           = 0;
        next.controllerAvailable   = true;
        next.topologyMatches       = false;
        next.valid                 = true;

        const ProductStatus journalStatus = journal.append (
            JournalEvent::PlanAccepted, plan.route, next.phase, plan.digest);

        if (journalStatus != ProductStatus::Ok)
        {
            return journalStatus;
        }

        state = next;
        return ProductStatus::Ok;
    }

    ProductStatus observeColdMove (const ProductEvidence& evidence,
                                   ColdMoveState& state,
                                   FakeJournal& journal) noexcept
    {
        if (!state.valid || !sameRouteEvidence (evidence, state))
        {
            return ProductStatus::StaleEvidence;
        }

        if (evidence.kind != EvidenceKind::ControllerRecovered &&
            evidence.controllerTerm != state.controllerTerm)
        {
            return ProductStatus::StaleEvidence;
        }

        if (evidence.topologyEpoch != state.plan.topologyEpoch)
        {
            return ProductStatus::StaleEvidence;
        }

        if (evidence.kind == EvidenceKind::ControllerLost)
        {
            const ProductStatus status = journal.append (
                JournalEvent::ControllerLost, state.plan.route,
                ColdMovePhase::Fault, state.plan.digest);

            if (status != ProductStatus::Ok)
            {
                return status;
            }

            state.controllerAvailable = false;
            state.visualMode          = VisualMode::ControllerLost;
            state.power.state         = PowerState::Off;
            state.phase               = ColdMovePhase::Fault;
            return ProductStatus::Ok;
        }

        if (evidence.kind == EvidenceKind::ControllerRecovered)
        {
            if (state.controllerAvailable ||
                evidence.controllerTerm <= state.controllerTerm)
            {
                return ProductStatus::StaleEvidence;
            }

            const ProductStatus status = journal.append (
                JournalEvent::ControllerRecovered, state.plan.route,
                ColdMovePhase::RecoveryWait, state.plan.digest);

            if (status != ProductStatus::Ok)
            {
                return status;
            }

            state.controllerAvailable = true;
            state.controllerTerm      = evidence.controllerTerm;
            state.visualMode          = VisualMode::Attention;
            state.phase               = ColdMovePhase::RecoveryWait;
            state.stableTicks         = 0;
            return ProductStatus::Ok;
        }

        if (!state.controllerAvailable)
        {
            return ProductStatus::ControllerUnavailable;
        }

        if (evidence.kind == EvidenceKind::RealFault ||
            unsafePower (evidence.power))
        {
            return enterFault (
                state, journal, ProductStatus::UnsafePower);
        }

        switch (state.phase)
        {
        case ColdMovePhase::Planned:
            if (evidence.kind != EvidenceKind::Begin)
            {
                return ProductStatus::WrongPhase;
            }

            return changePhase (
                state.plan.hasOldCau ? ColdMovePhase::AwaitingDisconnect
                                     : ColdMovePhase::AwaitingDischarge,
                state, journal);

        case ColdMovePhase::AwaitingDisconnect:
            if (evidence.kind != EvidenceKind::OldCauDisconnected ||
                evidence.cau.value != state.plan.oldCau.value)
            {
                return ProductStatus::WrongPhase;
            }

            return changePhase (
                ColdMovePhase::AwaitingDischarge, state, journal);

        case ColdMovePhase::AwaitingDischarge:
            if (evidence.kind == EvidenceKind::PauDischarging)
            {
                if (evidence.power.state != PowerState::Discharging ||
                    !evidence.power.suppliedByPau)
                {
                    return ProductStatus::UnsafePower;
                }

                const ProductStatus status = appendTransition (
                    state, journal, JournalEvent::EvidenceAccepted);

                if (status != ProductStatus::Ok)
                {
                    return status;
                }

                state.power = evidence.power;
                return ProductStatus::Ok;
            }

            if (evidence.kind != EvidenceKind::PauDischarged ||
                evidence.power.state != PowerState::Discharged ||
                evidence.power.millivolts != 0)
            {
                return ProductStatus::WrongPhase;
            }

            state.power = evidence.power;
            return changePhase (
                ColdMovePhase::AwaitingEpoch, state, journal);

        case ColdMovePhase::AwaitingEpoch:
            if (evidence.kind != EvidenceKind::EpochPersisted)
            {
                return ProductStatus::WrongPhase;
            }

            state.acceptedTopologyEpoch = evidence.topologyEpoch;
            return changePhase (
                ColdMovePhase::AwaitingPower, state, journal);

        case ColdMovePhase::AwaitingPower:
            if (evidence.kind == EvidenceKind::PauPowerAdmitted)
            {
                if (evidence.power.state != PowerState::Admitted ||
                    !evidence.power.suppliedByPau)
                {
                    return ProductStatus::UnsafePower;
                }

                const ProductStatus status = appendTransition (
                    state, journal, JournalEvent::EvidenceAccepted);

                if (status != ProductStatus::Ok)
                {
                    return status;
                }

                state.power = evidence.power;
                return ProductStatus::Ok;
            }

            if (evidence.kind != EvidenceKind::PauPowered ||
                evidence.power.state != PowerState::On ||
                !evidence.power.suppliedByPau)
            {
                return ProductStatus::WrongPhase;
            }

            state.power = evidence.power;
            return changePhase (
                ColdMovePhase::AwaitingTopology, state, journal);

        case ColdMovePhase::AwaitingTopology:
            if (evidence.kind != EvidenceKind::TopologyObserved ||
                !sameTopology (evidence.topology, state.plan.topology))
            {
                return ProductStatus::StaleEvidence;
            }

            state.topologyMatches = true;
            return changePhase (
                ColdMovePhase::AwaitingPresentation, state, journal);

        case ColdMovePhase::AwaitingPresentation:
            if (evidence.kind != EvidenceKind::NewCauPresented ||
                evidence.cau.value != state.plan.newCau.value)
            {
                return ProductStatus::WrongPhase;
            }

            return changePhase (ColdMovePhase::Active, state, journal);

        case ColdMovePhase::Active:
            if (evidence.kind != EvidenceKind::ContractLost)
            {
                return ProductStatus::WrongPhase;
            }

            state.power.state = PowerState::Off;
            state.stableTicks = 0;
            return changePhase (
                state.plan.profile.failurePolicy ==
                        FailurePolicy::AutomaticRecovery
                    ? ColdMovePhase::RecoveryWait
                    : ColdMovePhase::Fault,
                state, journal);

        case ColdMovePhase::RecoveryWait:
            if (state.plan.profile.failurePolicy ==
                    FailurePolicy::ManualRecovery &&
                evidence.kind == EvidenceKind::ManualRecoveryAuthorized)
            {
                state.stableTicks = state.plan.profile.stabilityTicks;
                return changePhase (
                    ColdMovePhase::AwaitingDischarge, state, journal);
            }

            if (state.plan.profile.failurePolicy !=
                    FailurePolicy::AutomaticRecovery ||
                evidence.kind != EvidenceKind::StabilityObserved)
            {
                return ProductStatus::WrongPhase;
            }

            if (evidence.stableTicks < state.plan.profile.stabilityTicks)
            {
                const ProductStatus status = appendTransition (
                    state, journal, JournalEvent::EvidenceAccepted);

                if (status != ProductStatus::Ok)
                {
                    return status;
                }

                state.stableTicks = evidence.stableTicks;
                return ProductStatus::Ok;
            }

            state.stableTicks = evidence.stableTicks;
            return changePhase (
                ColdMovePhase::AwaitingDischarge, state, journal);

        case ColdMovePhase::Fault:
            if (state.plan.profile.failurePolicy !=
                    FailurePolicy::ManualRecovery ||
                evidence.kind != EvidenceKind::ManualRecoveryAuthorized)
            {
                return ProductStatus::WrongPhase;
            }

            return changePhase (
                ColdMovePhase::AwaitingDischarge, state, journal);
        }

        return ProductStatus::WrongPhase;
    }

    const char* coldMovePhaseName (ColdMovePhase phase) noexcept
    {
        switch (phase)
        {
        case ColdMovePhase::Planned: return "planned";
        case ColdMovePhase::AwaitingDisconnect: return "awaiting-disconnect";
        case ColdMovePhase::AwaitingDischarge: return "awaiting-discharge";
        case ColdMovePhase::AwaitingEpoch: return "awaiting-epoch";
        case ColdMovePhase::AwaitingPower: return "awaiting-power";
        case ColdMovePhase::AwaitingTopology: return "awaiting-topology";
        case ColdMovePhase::AwaitingPresentation: return "awaiting-presentation";
        case ColdMovePhase::Active: return "active";
        case ColdMovePhase::RecoveryWait: return "recovery-wait";
        case ColdMovePhase::Fault: return "fault";
        }

        return "unknown";
    }

    const char* visualModeName (VisualMode mode) noexcept
    {
        switch (mode)
        {
        case VisualMode::Startup: return "startup";
        case VisualMode::Normal: return "normal";
        case VisualMode::Night: return "night";
        case VisualMode::Attention: return "attention";
        case VisualMode::Fault: return "fault";
        case VisualMode::Test: return "test";
        case VisualMode::ControllerLost: return "controller-lost";
        case VisualMode::Maintenance: return "maintenance";
        }

        return "unknown";
    }
} // namespace adk::usbmesh::product
