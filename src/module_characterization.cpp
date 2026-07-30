#include "module_characterization.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        template <typename Value>
        bool inClosedRange (Value value, Value first, Value last) noexcept
        {
            return value >= first && value <= last;
        }

        bool forward (uint32_t later, uint32_t earlier) noexcept
        {
            const uint32_t distance = later - earlier;
            return distance != 0 && distance < halfRange;
        }

        bool forwardOrEqual (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool validLeg (ModuleCharacterizationLeg leg) noexcept
        {
            return inClosedRange (leg, ModuleCharacterizationLeg::Ascending,
                                  ModuleCharacterizationLeg::Verification);
        }

        bool validDirection (ModuleSweepDirection direction) noexcept
        {
            return inClosedRange (direction, ModuleSweepDirection::Increasing,
                                  ModuleSweepDirection::Unordered);
        }

        ModuleCompactWitness witness (const ModuleCharacterizationPoint& point) noexcept
        {
            return {true,
                    point.controlOrdinal,
                    point.frame.analogRaw,
                    point.frame.comparatorAsserted,
                    point.frame.provenance.sequence,
                    point.frame.provenance.observedAt};
        }

        ModuleCharacterizationPoint
        pointFromWitness (const ModuleCharacterizationConfig&   config,
                          const ModuleCharacterizationEvidence& evidence,
                          ModuleCharacterizationLeg leg, ModuleSweepDirection direction,
                          const ModuleCompactWitness& compact) noexcept
        {
            const bool levelHigh = config.descriptor.comparatorPolarity ==
                                           ModuleComparatorPolarity::ActiveHigh
                                       ? compact.comparatorAsserted
                                       : !compact.comparatorAsserted;
            return {evidence.sessionId,
                    evidence.runId,
                    evidence.legId,
                    compact.controlOrdinal,
                    leg,
                    direction,
                    evidence.sourceId,
                    evidence.sourceConfigurationRevision,
                    {config.descriptor.schemaRevision,
                     config.descriptor.descriptorId,
                     config.descriptor.descriptorRevision,
                     config.descriptor.declaredSpecimenReference,
                     config.descriptor.declaredSpecimenRevision,
                     config.descriptor.declaredElectricalEvidenceRevision,
                     {evidence.sourceId, evidence.sourceConfigurationRevision,
                      compact.sequence, compact.observedAt},
                     compact.analogRaw,
                     ModuleChannelStatus::Current,
                     levelHigh,
                     ModuleChannelStatus::Current,
                     true,
                     compact.comparatorAsserted,
                     true,
                     true,
                     StatusCode::Ok,
                     StatusCode::Ok}};
        }

        bool frameEqual (const ModuleThresholdFrame& left,
                         const ModuleThresholdFrame& right) noexcept
        {
            return left.schemaRevision == right.schemaRevision &&
                   left.descriptorId == right.descriptorId &&
                   left.descriptorRevision == right.descriptorRevision &&
                   left.declaredSpecimenReference == right.declaredSpecimenReference &&
                   left.declaredSpecimenRevision == right.declaredSpecimenRevision &&
                   left.declaredElectricalEvidenceRevision ==
                       right.declaredElectricalEvidenceRevision &&
                   left.provenance.sourceId == right.provenance.sourceId &&
                   left.provenance.sourceConfigurationRevision ==
                       right.provenance.sourceConfigurationRevision &&
                   left.provenance.sequence == right.provenance.sequence &&
                   left.provenance.observedAt == right.provenance.observedAt &&
                   left.analogRaw == right.analogRaw &&
                   left.analogStatus == right.analogStatus &&
                   left.comparatorLevelHigh == right.comparatorLevelHigh &&
                   left.comparatorStatus == right.comparatorStatus &&
                   left.comparatorPresent == right.comparatorPresent &&
                   left.comparatorAsserted == right.comparatorAsserted &&
                   left.declaredWarmupSatisfied == right.declaredWarmupSatisfied &&
                   left.declaredSettlingSatisfied == right.declaredSettlingSatisfied &&
                   left.analogProducerStatus == right.analogProducerStatus &&
                   left.comparatorProducerStatus == right.comparatorProducerStatus;
        }

        bool pointEqual (const ModuleCharacterizationPoint& left,
                         const ModuleCharacterizationPoint& right) noexcept
        {
            return left.sessionId == right.sessionId && left.runId == right.runId &&
                   left.legId == right.legId &&
                   left.controlOrdinal == right.controlOrdinal &&
                   left.leg == right.leg && left.direction == right.direction &&
                   left.sourceId == right.sourceId &&
                   left.sourceConfigurationRevision ==
                       right.sourceConfigurationRevision &&
                   frameEqual (left.frame, right.frame);
        }

        ModuleThresholdDescriptor emptyDescriptor () noexcept
        {
            return {0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    ModuleChannelTopology::AnalogOnly,
                    ModuleComparatorOutputStage::Unspecified,
                    ModulePullRequirement::Unspecified,
                    ModuleDeclaredRail::Unspecified,
                    {0, 0},
                    {0, 0},
                    {0, 0},
                    ModuleComparatorPolarity::Unspecified,
                    ModuleThresholdControlKind::Unspecified,
                    ModuleThresholdDirection::Unspecified,
                    {ModuleDurationDeclaration::Known, Duration (0)},
                    {ModuleDurationDeclaration::Known, Duration (0)}};
        }

        ModuleThresholdFrame emptyFrame () noexcept
        {
            return {0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    {0, 0, 0, TimePoint (0)},
                    0,
                    ModuleChannelStatus::NotPresent,
                    false,
                    ModuleChannelStatus::NotPresent,
                    false,
                    false,
                    false,
                    false,
                    StatusCode::Ok,
                    StatusCode::Ok};
        }

        ModuleCharacterizationPoint emptyPoint () noexcept
        {
            return {0,
                    0,
                    0,
                    0,
                    ModuleCharacterizationLeg::Ascending,
                    ModuleSweepDirection::Increasing,
                    0,
                    0,
                    emptyFrame ()};
        }

        ModuleCompactWitness emptyWitness () noexcept
        {
            return {false, 0, 0, false, 0, TimePoint (0)};
        }

        void clearEvidence (ModuleCharacterizationEvidence& evidence) noexcept
        {
            const ModuleCharacterizationPoint point    = emptyPoint ();
            const ModuleTransitionBracket     bracket  = {false, point, point};
            const ModuleAnalogInterval        interval = {false, 0, 0};
            evidence.lifecycleGeneration               = 0;
            evidence.sessionId                         = 0;
            evidence.runId                             = 0;
            evidence.legId                             = 0;
            evidence.characterizationRevision          = 0;
            evidence.descriptor                        = emptyDescriptor ();
            evidence.sourceId                          = 0;
            evidence.sourceConfigurationRevision       = 0;
            evidence.state                      = ModuleCharacterizationState::Idle;
            evidence.reason                     = ModuleCharacterizationReason::None;
            evidence.terminalLeg                = ModuleCharacterizationLeg::Ascending;
            evidence.ascendingCount             = 0;
            evidence.descendingCount            = 0;
            evidence.verificationCount          = 0;
            evidence.ascendingBracket           = bracket;
            evidence.descendingBracket          = bracket;
            evidence.guaranteedInactiveInterval = interval;
            evidence.guaranteedActiveInterval   = interval;
            evidence.ambiguityInterval          = interval;
            evidence.relation                   = ModuleComparatorRelation::Unverified;
            evidence.firstWitness               = emptyWitness ();
            evidence.lastWitness                = emptyWitness ();
            evidence.offendingBefore            = emptyWitness ();
            evidence.offendingAfter             = emptyWitness ();
            evidence.status                     = StatusCode::Ok;
        }

        uint8_t& legCount (ModuleCharacterizationEvidence& evidence,
                           ModuleCharacterizationLeg       leg) noexcept
        {
            if (leg == ModuleCharacterizationLeg::Ascending)
            {
                return evidence.ascendingCount;
            }
            if (leg == ModuleCharacterizationLeg::Descending)
            {
                return evidence.descendingCount;
            }
            return evidence.verificationCount;
        }

        ModuleTransitionBracket& legBracket (ModuleCharacterizationEvidence& evidence,
                                             ModuleCharacterizationLeg leg) noexcept
        {
            return leg == ModuleCharacterizationLeg::Ascending
                       ? evidence.ascendingBracket
                       : evidence.descendingBracket;
        }

        void reject (ModuleCharacterizationEvidence& evidence,
                     ModuleCharacterizationReason reason, ModuleCharacterizationLeg leg,
                     const ModuleCharacterizationPoint* before,
                     const ModuleCharacterizationPoint* after) noexcept
        {
            evidence.state       = ModuleCharacterizationState::Rejected;
            evidence.reason      = reason;
            evidence.terminalLeg = leg;
            evidence.status      = StatusCode::Ok;
            if (before != nullptr)
            {
                evidence.offendingBefore = witness (*before);
            }
            if (after != nullptr)
            {
                evidence.offendingAfter = witness (*after);
            }
        }

        void rejectCompact (ModuleCharacterizationEvidence& evidence,
                            ModuleCharacterizationReason    reason,
                            ModuleCharacterizationLeg       leg,
                            const ModuleCompactWitness*     before,
                            const ModuleCompactWitness*     after) noexcept
        {
            evidence.state       = ModuleCharacterizationState::Rejected;
            evidence.reason      = reason;
            evidence.terminalLeg = leg;
            evidence.status      = StatusCode::Ok;
            if (before != nullptr)
            {
                evidence.offendingBefore = *before;
            }
            if (after != nullptr)
            {
                evidence.offendingAfter = *after;
            }
        }

        bool atEndpoint (const ModuleCompactWitness& point, uint16_t endpoint) noexcept
        {
            return point.analogRaw == endpoint;
        }
    } // namespace

    ModuleCharacterizationPolicy::ModuleCharacterizationPolicy (
        const ModuleCharacterizationConfig& config) noexcept
        : config_                (config),
          evidence_              (),
          firstLegWitness_       (),
          previousWitness_       (),
          priorSourceWitness_    (),
          lastSessionId_         (0),
          lastRunId_             (0),
          lastLegId_             (0),
          lastOperationAt_       (),
          initialized_           (false),
          shutdown_              (false),
          sessionActive_         (false),
          legActive_             (false),
          hasPreviousPoint_      (false),
          hasPriorSourceWitness_ (false),
          transitionSeen_        (false),
          relationConsistent_    (false),
          relationAmbiguous_     (false)
    {
        clearEvidence (evidence_);
    }

    Status ModuleCharacterizationPolicy::initialize (TimePoint now) noexcept
    {
        if (shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Status descriptorStatus =
            validateModuleThresholdDescriptor (config_.descriptor);
        const Result<bool> declarations =
            moduleDescriptorDeclarationsComplete (config_.descriptor);
        if (!descriptorStatus.ok () || !declarations.ok () || !declarations.value () ||
            config_.characterizationRevision == 0 || config_.requiredPointsPerLeg < 2 ||
            config_.requiredPointsPerLeg > 16 ||
            config_.maximumAge.milliseconds () == 0 ||
            config_.maximumGap.milliseconds () == 0 ||
            config_.maximumAge.milliseconds () >= halfRange ||
            config_.maximumGap.milliseconds () >= halfRange ||
            config_.descriptor.channelTopology !=
                ModuleChannelTopology::AnalogAndComparator ||
            config_.descriptor.comparatorPolarity ==
                ModuleComparatorPolarity::Unspecified)
        {
            return StatusCode::InvalidConfiguration;
        }

        if (evidence_.lifecycleGeneration == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }

        ++evidence_.lifecycleGeneration;
        evidence_.characterizationRevision = config_.characterizationRevision;
        evidence_.descriptor               = config_.descriptor;
        evidence_.state                    = ModuleCharacterizationState::Idle;
        evidence_.reason                   = ModuleCharacterizationReason::None;
        evidence_.status                   = StatusCode::Ok;
        lastOperationAt_                   = now;
        initialized_                       = true;
        return StatusCode::Ok;
    }

    Status ModuleCharacterizationPolicy::beginSession (TimePoint now,
                                                       uint32_t  sessionId,
                                                       uint32_t  runId) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (evidence_.state == ModuleCharacterizationState::Shutdown)
        {
            return StatusCode::NotInitialized;
        }
        if (sessionActive_ || evidence_.state != ModuleCharacterizationState::Idle ||
            sessionId == 0 || runId == 0 ||
            !forwardOrEqual (now, lastOperationAt_) ||
            (lastSessionId_ != 0 &&
             !forward       (sessionId, lastSessionId_)) ||
            (lastRunId_ != 0 &&
             !forward       (runId, lastRunId_)))
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t generation = evidence_.lifecycleGeneration;
        clearEvidence (evidence_);
        evidence_.lifecycleGeneration      = generation;
        evidence_.sessionId                = sessionId;
        evidence_.runId                    = runId;
        evidence_.characterizationRevision = config_.characterizationRevision;
        evidence_.descriptor               = config_.descriptor;
        evidence_.state                    = ModuleCharacterizationState::Idle;
        evidence_.reason                   = ModuleCharacterizationReason::None;
        evidence_.status                   = StatusCode::Ok;
        lastSessionId_                     = sessionId;
        lastRunId_                         = runId;
        lastLegId_                         = 0;
        lastOperationAt_                   = now;
        sessionActive_                     = true;
        legActive_                         = false;
        hasPreviousPoint_                  = false;
        hasPriorSourceWitness_             = false;
        transitionSeen_                    = false;
        relationConsistent_                = false;
        relationAmbiguous_                 = false;
        return StatusCode::Ok;
    }

    Status
    ModuleCharacterizationPolicy::beginLeg (TimePoint now, uint32_t legId,
                                            ModuleCharacterizationLeg leg,
                                            ModuleSweepDirection direction) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        const bool legIdFollows =
            lastLegId_ == 0 || forward (legId, lastLegId_);
        if (!sessionActive_ || legActive_ || !validLeg (leg) ||
            !validDirection (direction) || legId == 0 ||
            !forwardOrEqual (now, lastOperationAt_) || !legIdFollows)
        {
            return StatusCode::InvalidArgument;
        }

        const ModuleCharacterizationLeg expected =
            evidence_.ascendingCount == 0
                ? ModuleCharacterizationLeg::Ascending
                : (evidence_.descendingCount == 0
                       ? ModuleCharacterizationLeg::Descending
                       : ModuleCharacterizationLeg::Verification);
        const ModuleSweepDirection expectedDirection =
            expected == ModuleCharacterizationLeg::Ascending
                ? ModuleSweepDirection::Increasing
                : (expected == ModuleCharacterizationLeg::Descending
                       ? ModuleSweepDirection::Decreasing
                       : ModuleSweepDirection::Unordered);
        if (leg != expected || direction != expectedDirection)
        {
            return StatusCode::InvalidArgument;
        }

        evidence_.legId       = legId;
        evidence_.terminalLeg = leg;
        evidence_.state       = ModuleCharacterizationState::Collecting;
        lastLegId_            = legId;
        lastOperationAt_      = now;
        legActive_            = true;
        hasPreviousPoint_     = false;
        transitionSeen_       = false;
        return StatusCode::Ok;
    }

    Status ModuleCharacterizationPolicy::observe (
        TimePoint now, const ModuleCharacterizationPoint& point) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!sessionActive_ || !legActive_ ||
            evidence_.state != ModuleCharacterizationState::Collecting ||
            !validLeg       (point.leg) ||
            !validDirection (point.direction) ||
            !forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }

        const ModuleCharacterizationLeg activeLeg = evidence_.terminalLeg;
        const ModuleSweepDirection      activeDirection =
            activeLeg == ModuleCharacterizationLeg::Ascending
                ? ModuleSweepDirection::Increasing
                : (activeLeg == ModuleCharacterizationLeg::Descending
                       ? ModuleSweepDirection::Decreasing
                       : ModuleSweepDirection::Unordered);
        const ModuleCharacterizationPoint previousPoint = pointFromWitness (
            config_, evidence_, activeLeg, activeDirection, previousWitness_);
        uint8_t& count = legCount (evidence_, activeLeg);

        const Status frameStatus =
            validateModuleThresholdFrame (config_.descriptor, point.frame);
        if (!frameStatus.ok ())
        {
            return frameStatus;
        }

        if (point.sessionId != evidence_.sessionId || point.runId != evidence_.runId ||
            point.legId != evidence_.legId || point.leg != activeLeg ||
            point.direction != activeDirection || point.sourceId == 0 ||
            point.sourceConfigurationRevision == 0 ||
            point.sourceId != point.frame.provenance.sourceId ||
            point.sourceConfigurationRevision !=
                point.frame.provenance.sourceConfigurationRevision ||
            (evidence_.sourceId != 0 && (point.sourceId != evidence_.sourceId ||
                                         point.sourceConfigurationRevision !=
                                             evidence_.sourceConfigurationRevision)))
        {
            return StatusCode::InvalidArgument;
        }

        if (hasPreviousPoint_ &&
            point.frame.provenance.sequence == previousWitness_.sequence &&
            point.frame.provenance.observedAt == previousWitness_.observedAt &&
            point.controlOrdinal == previousWitness_.controlOrdinal)
        {
            return pointEqual (point, previousPoint) ? StatusCode::Ok
                                                     : StatusCode::InvalidArgument;
        }

        if (count >= config_.requiredPointsPerLeg)
        {
            return StatusCode::CapacityExceeded;
        }
        if (point.controlOrdinal != static_cast<uint16_t> (count + 1U))
        {
            return StatusCode::InvalidArgument;
        }

        ModuleCharacterizationEvidence& candidate = evidence_;
        if (!point.frame.analogProducerStatus.ok () ||
            !point.frame.comparatorProducerStatus.ok () ||
            point.frame.analogStatus == ModuleChannelStatus::ProducerFault ||
            point.frame.comparatorStatus == ModuleChannelStatus::ProducerFault)
        {
            reject (candidate, ModuleCharacterizationReason::ProducerFault, activeLeg,
                    hasPreviousPoint_ ? &previousPoint : nullptr, &point);
            legActive_     = false;
            sessionActive_ = false;
            return StatusCode::Ok;
        }

        const ModuleCompactWitness currentWitness = witness (point);
        const uint32_t             age =
            now.elapsedSince (point.frame.provenance.observedAt).milliseconds ();
        if (!forwardOrEqual (now, point.frame.provenance.observedAt) ||
            (hasPriorSourceWitness_ &&
             !forward (point.frame.provenance.observedAt.milliseconds (),
                       priorSourceWitness_.observedAt.milliseconds ())) ||
            (hasPriorSourceWitness_ &&
             point.frame.provenance.observedAt
                     .elapsedSince (priorSourceWitness_.observedAt)
                     .milliseconds () > config_.maximumGap.milliseconds ()))
        {
            rejectCompact (
                candidate, ModuleCharacterizationReason::TimestampDiscontinuity,
                activeLeg, hasPriorSourceWitness_ ? &priorSourceWitness_ : nullptr,
                &currentWitness);
        }
        else if (hasPriorSourceWitness_ &&
                 point.frame.provenance.sequence - priorSourceWitness_.sequence != 1U)
        {
            rejectCompact (candidate,
                           ModuleCharacterizationReason::SequenceDiscontinuity,
                           activeLeg, &priorSourceWitness_, &currentWitness);
        }
        else if (age > config_.maximumAge.milliseconds () ||
                 point.frame.analogStatus == ModuleChannelStatus::Stale ||
                 point.frame.comparatorStatus == ModuleChannelStatus::Stale)
        {
            reject (candidate, ModuleCharacterizationReason::Stale, activeLeg,
                    hasPreviousPoint_ ? &previousPoint : nullptr, &point);
        }
        else if (!point.frame.declaredWarmupSatisfied)
        {
            reject (candidate, ModuleCharacterizationReason::WarmupUnsatisfied,
                    activeLeg, hasPreviousPoint_ ? &previousPoint : nullptr, &point);
        }
        else if (!point.frame.declaredSettlingSatisfied)
        {
            reject (candidate, ModuleCharacterizationReason::SettlingUnsatisfied,
                    activeLeg, hasPreviousPoint_ ? &previousPoint : nullptr, &point);
        }
        else if (activeLeg != ModuleCharacterizationLeg::Verification &&
                 hasPreviousPoint_ &&
                 ((activeDirection == ModuleSweepDirection::Increasing &&
                   point.frame.analogRaw < previousWitness_.analogRaw) ||
                  (activeDirection == ModuleSweepDirection::Decreasing &&
                   point.frame.analogRaw > previousWitness_.analogRaw)))
        {
            reject (candidate, ModuleCharacterizationReason::DirectionViolation,
                    activeLeg, &previousPoint, &point);
        }
        else if (activeLeg != ModuleCharacterizationLeg::Verification &&
                 hasPreviousPoint_ &&
                 point.frame.comparatorAsserted != previousWitness_.comparatorAsserted)
        {
            if (point.frame.analogRaw == previousWitness_.analogRaw || transitionSeen_)
            {
                reject (candidate, ModuleCharacterizationReason::Chatter, activeLeg,
                        &previousPoint, &point);
            }
            else
            {
                ModuleTransitionBracket& bracket = legBracket (candidate, activeLeg);
                bracket.present                  = true;
                bracket.before                   = previousPoint;
                bracket.after                    = point;
            }
        }
        else if (activeLeg == ModuleCharacterizationLeg::Verification)
        {
            bool inAmbiguity =
                candidate.ambiguityInterval.present &&
                point.frame.analogRaw >= candidate.ambiguityInterval.lower &&
                point.frame.analogRaw <= candidate.ambiguityInterval.upper;
            bool contradiction = false;
            if (candidate.guaranteedActiveInterval.present &&
                point.frame.analogRaw >= candidate.guaranteedActiveInterval.lower &&
                point.frame.analogRaw <= candidate.guaranteedActiveInterval.upper)
            {
                contradiction       = !point.frame.comparatorAsserted;
                relationConsistent_ = relationConsistent_ || !contradiction;
            }
            else if (candidate.guaranteedInactiveInterval.present &&
                     point.frame.analogRaw >=
                         candidate.guaranteedInactiveInterval.lower &&
                     point.frame.analogRaw <=
                         candidate.guaranteedInactiveInterval.upper)
            {
                contradiction       = point.frame.comparatorAsserted;
                relationConsistent_ = relationConsistent_ || !contradiction;
            }
            else if (inAmbiguity)
            {
                relationAmbiguous_ = true;
            }

            if (contradiction)
            {
                candidate.relation = ModuleComparatorRelation::Disagrees;
                reject (candidate,
                        ModuleCharacterizationReason::AnalogComparatorDisagreement,
                        activeLeg, hasPreviousPoint_ ? &previousPoint : nullptr,
                        &point);
            }
        }

        if (candidate.state == ModuleCharacterizationState::Rejected)
        {
            legActive_     = false;
            sessionActive_ = false;
            return StatusCode::Ok;
        }

        ++legCount (candidate, activeLeg);
        if (!candidate.firstWitness.present)
        {
            candidate.firstWitness                = witness (point);
            candidate.sourceId                    = point.sourceId;
            candidate.sourceConfigurationRevision = point.sourceConfigurationRevision;
        }
        candidate.lastWitness = witness (point);

        if (activeLeg != ModuleCharacterizationLeg::Verification && hasPreviousPoint_ &&
            point.frame.comparatorAsserted != previousWitness_.comparatorAsserted)
        {
            transitionSeen_ = true;
        }

        if (!hasPreviousPoint_)
        {
            firstLegWitness_ = witness (point);
        }
        previousWitness_       = witness (point);
        priorSourceWitness_    = previousWitness_;
        hasPreviousPoint_      = true;
        hasPriorSourceWitness_ = true;
        lastOperationAt_       = now;
        return StatusCode::Ok;
    }

    Status ModuleCharacterizationPolicy::finalizeLeg (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!sessionActive_ || !legActive_ ||
            evidence_.state != ModuleCharacterizationState::Collecting ||
            !forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }

        const ModuleCharacterizationLeg leg = evidence_.terminalLeg;
        if (legCount (evidence_, leg) != config_.requiredPointsPerLeg)
        {
            return StatusCode::InvalidArgument;
        }

        ModuleCharacterizationEvidence& candidate = evidence_;
        if (leg != ModuleCharacterizationLeg::Verification)
        {
            const bool allAtLower =
                atEndpoint (firstLegWitness_, config_.descriptor.rawDomain.minimum) &&
                atEndpoint (previousWitness_, config_.descriptor.rawDomain.minimum);
            const bool allAtUpper =
                atEndpoint (firstLegWitness_, config_.descriptor.rawDomain.maximum) &&
                atEndpoint (previousWitness_, config_.descriptor.rawDomain.maximum);
            if (!legBracket (candidate, leg).present && (allAtLower || allAtUpper))
            {
                rejectCompact (candidate,
                               allAtLower ? ModuleCharacterizationReason::AtLowerRail
                                          : ModuleCharacterizationReason::AtUpperRail,
                               leg, &firstLegWitness_, &previousWitness_);
                legActive_     = false;
                sessionActive_ = false;
                return StatusCode::Ok;
            }

            const bool coversDomain =
                leg == ModuleCharacterizationLeg::Ascending
                    ? atEndpoint (firstLegWitness_,
                                  config_.descriptor.rawDomain.minimum) &&
                          atEndpoint (previousWitness_,
                                      config_.descriptor.rawDomain.maximum)
                    : atEndpoint (firstLegWitness_,
                                  config_.descriptor.rawDomain.maximum) &&
                          atEndpoint (previousWitness_,
                                      config_.descriptor.rawDomain.minimum);
            if (!coversDomain)
            {
                rejectCompact (candidate,
                               ModuleCharacterizationReason::DirectionViolation, leg,
                               &firstLegWitness_, &previousWitness_);
                legActive_     = false;
                sessionActive_ = false;
                return StatusCode::Ok;
            }

            if (!legBracket (candidate, leg).present)
            {
                rejectCompact (
                    candidate,
                    previousWitness_.comparatorAsserted
                        ? ModuleCharacterizationReason::NoObservedTransitionActive
                        : ModuleCharacterizationReason::NoObservedTransitionInactive,
                    leg, &firstLegWitness_, &previousWitness_);
                legActive_     = false;
                sessionActive_ = false;
                return StatusCode::Ok;
            }
        }

        if (leg == ModuleCharacterizationLeg::Descending)
        {
            const ModuleTransitionBracket& ascending  = candidate.ascendingBracket;
            const ModuleTransitionBracket& descending = candidate.descendingBracket;
            const bool lowState = ascending.before.frame.comparatorAsserted;
            if (lowState != descending.after.frame.comparatorAsserted ||
                ascending.after.frame.comparatorAsserted !=
                    descending.before.frame.comparatorAsserted ||
                lowState == ascending.after.frame.comparatorAsserted)
            {
                reject (candidate,
                        ModuleCharacterizationReason::TransitionOrientationMismatch,
                        leg, &descending.before, &descending.after);
                legActive_     = false;
                sessionActive_ = false;
                return StatusCode::Ok;
            }

            const uint16_t lowProved =
                ascending.before.frame.analogRaw < descending.after.frame.analogRaw
                    ? ascending.before.frame.analogRaw
                    : descending.after.frame.analogRaw;
            const uint16_t highProved =
                ascending.after.frame.analogRaw > descending.before.frame.analogRaw
                    ? ascending.after.frame.analogRaw
                    : descending.before.frame.analogRaw;
            const ModuleAnalogInterval lowInterval = {
                true, config_.descriptor.rawDomain.minimum, lowProved};
            const ModuleAnalogInterval highInterval = {
                true, highProved, config_.descriptor.rawDomain.maximum};

            if (lowState)
            {
                candidate.guaranteedActiveInterval   = lowInterval;
                candidate.guaranteedInactiveInterval = highInterval;
            }
            else
            {
                candidate.guaranteedInactiveInterval = lowInterval;
                candidate.guaranteedActiveInterval   = highInterval;
            }

            const uint32_t ambiguityLower = static_cast<uint32_t> (lowProved) + 1U;
            const uint32_t ambiguityUpper =
                highProved == 0 ? UINT32_MAX : static_cast<uint32_t> (highProved) - 1U;
            if (ambiguityLower <= ambiguityUpper && ambiguityUpper <= UINT16_MAX)
            {
                candidate.ambiguityInterval = {true,
                                               static_cast<uint16_t> (ambiguityLower),
                                               static_cast<uint16_t> (ambiguityUpper)};
            }
        }

        if (leg == ModuleCharacterizationLeg::Verification)
        {
            candidate.relation =
                relationConsistent_
                    ? (relationAmbiguous_ ? ModuleComparatorRelation::Ambiguous
                                          : ModuleComparatorRelation::Consistent)
                    : ModuleComparatorRelation::Ambiguous;
            candidate.state  = ModuleCharacterizationState::Complete;
            candidate.reason = ModuleCharacterizationReason::None;
            sessionActive_   = false;
        }
        else
        {
            candidate.state = ModuleCharacterizationState::Idle;
        }

        legActive_        = false;
        hasPreviousPoint_ = false;
        transitionSeen_   = false;
        lastOperationAt_  = now;
        return StatusCode::Ok;
    }

    Status ModuleCharacterizationPolicy::reset (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }
        if (evidence_.lifecycleGeneration == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }

        const uint32_t nextGeneration =
            evidence_.lifecycleGeneration + UINT32_C (1);
        clearEvidence (evidence_);
        evidence_.lifecycleGeneration      = nextGeneration;
        evidence_.characterizationRevision = config_.characterizationRevision;
        evidence_.descriptor               = config_.descriptor;
        evidence_.state                    = ModuleCharacterizationState::Idle;
        evidence_.status                   = StatusCode::Ok;
        sessionActive_                     = false;
        legActive_                         = false;
        hasPreviousPoint_                  = false;
        hasPriorSourceWitness_             = false;
        transitionSeen_                    = false;
        relationConsistent_                = false;
        relationAmbiguous_                 = false;
        lastOperationAt_                   = now;
        return StatusCode::Ok;
    }

    Status ModuleCharacterizationPolicy::shutdown (TimePoint now) noexcept
    {
        if (shutdown_)
        {
            return forwardOrEqual (now, lastOperationAt_) ? StatusCode::Ok
                                                          : StatusCode::InvalidArgument;
        }
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }

        evidence_.state        = ModuleCharacterizationState::Shutdown;
        evidence_.reason       = ModuleCharacterizationReason::None;
        evidence_.status       = StatusCode::Ok;
        sessionActive_         = false;
        legActive_             = false;
        hasPreviousPoint_      = false;
        hasPriorSourceWitness_ = false;
        initialized_           = false;
        shutdown_              = true;
        lastOperationAt_       = now;
        return StatusCode::Ok;
    }

    Status ModuleCharacterizationPolicy::evidence (
        ModuleCharacterizationEvidence& output) const noexcept
    {
        if (!initialized_ && evidence_.state != ModuleCharacterizationState::Shutdown)
        {
            return StatusCode::NotInitialized;
        }

        output = evidence_;
        return StatusCode::Ok;
    }

#if defined(ADK_TESTING)
    void ModuleCharacterizationPolicy::seedLifecycleGenerationForTest (
        uint32_t generation) noexcept
    {
        evidence_.lifecycleGeneration = generation;
    }
#endif
} // namespace adk
