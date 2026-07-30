#include "inert_module_characterization_bench.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool validStatus (Status status) noexcept
        {
            return static_cast<uint8_t> (status.error ()) <=
                   static_cast<uint8_t> (StatusCode::HardwareFailure);
        }

        bool validState (ModuleCharacterizationState state) noexcept
        {
            return state == ModuleCharacterizationState::Idle ||
                   state == ModuleCharacterizationState::Collecting ||
                   state == ModuleCharacterizationState::Complete ||
                   state == ModuleCharacterizationState::Rejected ||
                   state == ModuleCharacterizationState::Shutdown;
        }

        bool validReason (ModuleCharacterizationReason reason) noexcept
        {
            return static_cast<uint8_t> (reason) <=
                   static_cast<uint8_t> (
                       ModuleCharacterizationReason::AnalogComparatorDisagreement);
        }

        bool validLeg (ModuleCharacterizationLeg leg) noexcept
        {
            return leg == ModuleCharacterizationLeg::Ascending ||
                   leg == ModuleCharacterizationLeg::Descending ||
                   leg == ModuleCharacterizationLeg::Verification;
        }

        bool validRelation (ModuleComparatorRelation relation) noexcept
        {
            return relation == ModuleComparatorRelation::Unverified ||
                   relation == ModuleComparatorRelation::Consistent ||
                   relation == ModuleComparatorRelation::Ambiguous ||
                   relation == ModuleComparatorRelation::Disagrees;
        }

        bool validConfiguration (const ModuleBenchConfig& config) noexcept
        {
            const uint32_t maximumAge = config.maximumControlAge.milliseconds ();
            return config.benchRevision != 0 && config.envelopeRevision != 0 &&
                   config.recordSchemaRevision != 0 &&
                   config.expectedDescriptorId != 0 &&
                   config.expectedDescriptorRevision != 0 &&
                   config.expectedDescriptorSchemaRevision != 0 &&
                   config.expectedDeclaredSpecimenRevision != 0 &&
                   config.expectedDeclaredElectricalEvidenceRevision != 0 &&
                   config.expectedControlSourceId != 0 &&
                   config.expectedControlSourceConfigurationRevision != 0 &&
                   maximumAge != 0 && maximumAge < halfRange;
        }

        bool forwardOrEqual (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool currentAt (TimePoint now, TimePoint observedAt,
                        Duration maximumAge) noexcept
        {
            const uint32_t age = now.elapsedSince (observedAt).milliseconds ();

            return age < halfRange && age <= maximumAge.milliseconds ();
        }

        bool sameStatus (Status left, Status right) noexcept
        {
            return left.error () == right.error ();
        }

        bool sameControl (const ModuleBenchControl& left,
                          const ModuleBenchControl& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.sourceConfigurationRevision ==
                       right.sourceConfigurationRevision &&
                   left.sessionId == right.sessionId &&
                   left.sequence == right.sequence &&
                   left.observedAt.milliseconds () ==
                       right.observedAt.milliseconds () &&
                   left.command == right.command &&
                   sameStatus (left.producerStatus, right.producerStatus);
        }

        bool zeroFrame (const ModuleThresholdFrame& frame) noexcept
        {
            return frame.schemaRevision == 0 && frame.descriptorId == 0 &&
                   frame.descriptorRevision == 0 &&
                   frame.declaredSpecimenReference == 0 &&
                   frame.declaredSpecimenRevision == 0 &&
                   frame.declaredElectricalEvidenceRevision == 0 &&
                   frame.provenance.sourceId == 0 &&
                   frame.provenance.sourceConfigurationRevision == 0 &&
                   frame.provenance.sequence == 0 &&
                   frame.provenance.observedAt.milliseconds () == 0 &&
                   frame.analogRaw == 0 &&
                   frame.analogStatus == ModuleChannelStatus::NotPresent &&
                   !frame.comparatorLevelHigh &&
                   frame.comparatorStatus == ModuleChannelStatus::NotPresent &&
                   !frame.comparatorPresent && !frame.comparatorAsserted &&
                   !frame.declaredWarmupSatisfied && !frame.declaredSettlingSatisfied &&
                   frame.analogProducerStatus.ok () &&

                   frame.comparatorProducerStatus.ok ();
        }

        bool zeroPoint (const ModuleCharacterizationPoint& point) noexcept
        {
            return point.sessionId == 0 && point.runId == 0 && point.legId == 0 &&
                   point.controlOrdinal == 0 &&
                   point.leg == ModuleCharacterizationLeg::Ascending &&
                   point.direction == ModuleSweepDirection::Increasing &&
                   point.sourceId == 0 && point.sourceConfigurationRevision == 0 &&
                   zeroFrame (point.frame);
        }

        bool zeroWitness (const ModuleCompactWitness& witness) noexcept
        {
            return !witness.present && witness.controlOrdinal == 0 &&
                   witness.analogRaw == 0 && !witness.comparatorAsserted &&
                   witness.sequence == 0 && witness.observedAt.milliseconds () == 0;
        }

        bool validWitness (const ModuleCompactWitness& witness,
                           const ModuleRawDomain&      domain) noexcept
        {
            return witness.present
                       ? witness.controlOrdinal != 0 && witness.sequence != 0 &&
                             witness.analogRaw >= domain.minimum &&
                             witness.analogRaw <= domain.maximum
                       : zeroWitness (witness);
        }

        bool validInterval (const ModuleAnalogInterval& interval,
                            const ModuleRawDomain&      domain) noexcept
        {
            if (!interval.present)
            {
                return interval.lower == 0 && interval.upper == 0;
            }
            return interval.lower <= interval.upper &&
                   interval.lower >= domain.minimum && interval.upper <= domain.maximum;
        }

        bool sameInterval (const ModuleAnalogInterval& interval, bool present,
                           uint16_t lower, uint16_t upper) noexcept
        {
            return interval.present == present &&
                   interval.lower == (present ? lower : 0) &&
                   interval.upper == (present ? upper : 0);
        }

        bool
        validCompleteGeometry (const ModuleCharacterizationEvidence& evidence) noexcept
        {
            const ModuleCharacterizationPoint& ascendingBefore =
                evidence.ascendingBracket.before;
            const ModuleCharacterizationPoint& ascendingAfter =
                evidence.ascendingBracket.after;
            const ModuleCharacterizationPoint& descendingBefore =
                evidence.descendingBracket.before;
            const ModuleCharacterizationPoint& descendingAfter =
                evidence.descendingBracket.after;
            const bool lowState = ascendingBefore.frame.comparatorAsserted;
            if (lowState != descendingAfter.frame.comparatorAsserted ||
                ascendingAfter.frame.comparatorAsserted !=
                    descendingBefore.frame.comparatorAsserted ||
                lowState == ascendingAfter.frame.comparatorAsserted)
            {
                return false;
            }

            const uint16_t lowProved =
                ascendingBefore.frame.analogRaw < descendingAfter.frame.analogRaw
                    ? ascendingBefore.frame.analogRaw
                    : descendingAfter.frame.analogRaw;
            const uint16_t highProved =
                ascendingAfter.frame.analogRaw > descendingBefore.frame.analogRaw
                    ? ascendingAfter.frame.analogRaw
                    : descendingBefore.frame.analogRaw;
            if (lowProved >= highProved)
            {
                return false;
            }

            const ModuleAnalogInterval& low = lowState
                                                  ? evidence.guaranteedActiveInterval
                                                  : evidence.guaranteedInactiveInterval;
            const ModuleAnalogInterval& high = lowState
                                                   ? evidence.guaranteedInactiveInterval
                                                   : evidence.guaranteedActiveInterval;
            const bool hasAmbiguity          = static_cast<uint32_t> (lowProved) + 1U <=
                                               static_cast<uint32_t> (highProved) - 1U;
            return sameInterval (low, true, evidence.descriptor.rawDomain.minimum,
                                 lowProved) &&
                   sameInterval (high, true, highProved,
                                 evidence.descriptor.rawDomain.maximum) &&
                   sameInterval (evidence.ambiguityInterval, hasAmbiguity,
                                 static_cast<uint16_t> (lowProved + 1U),
                                 static_cast<uint16_t> (highProved - 1U));
        }

        bool validPoint (const ModuleCharacterizationPoint&    point,
                         const ModuleCharacterizationEvidence& evidence,
                         ModuleCharacterizationLeg             leg,
                         ModuleSweepDirection                  direction) noexcept
        {
            return point.sessionId == evidence.sessionId &&
                   point.runId == evidence.runId && point.legId != 0 &&
                   point.controlOrdinal != 0 && point.leg == leg &&
                   point.direction == direction &&
                   point.sourceId == evidence.sourceId &&
                   point.sourceConfigurationRevision ==
                       evidence.sourceConfigurationRevision &&
                   point.frame.provenance.sourceId == evidence.sourceId &&
                   point.frame.provenance.sourceConfigurationRevision ==
                       evidence.sourceConfigurationRevision &&
                   validateModuleThresholdFrame (evidence.descriptor, point.frame)
                       .ok ();
        }

        bool validBracket (const ModuleTransitionBracket&        bracket,
                           const ModuleCharacterizationEvidence& evidence,
                           ModuleCharacterizationLeg             leg,
                           ModuleSweepDirection                  direction) noexcept
        {
            if (!bracket.present)
            {
                return zeroPoint (bracket.before) && zeroPoint (bracket.after);
            }
            return validPoint (bracket.before, evidence, leg, direction) &&
                   validPoint (bracket.after, evidence, leg, direction) &&
                   bracket.before.legId == bracket.after.legId &&
                   bracket.before.frame.comparatorAsserted !=
                       bracket.after.frame.comparatorAsserted &&
                   (direction == ModuleSweepDirection::Increasing
                        ? bracket.before.frame.analogRaw < bracket.after.frame.analogRaw
                        : bracket.before.frame.analogRaw >
                              bracket.after.frame.analogRaw) &&
                   bracket.after.controlOrdinal ==
                       static_cast<uint16_t> (bracket.before.controlOrdinal + 1U) &&
                   bracket.after.frame.provenance.sequence -
                              bracket.before.frame.provenance.sequence ==
                       1U;
        }

        bool forwardId (uint32_t later, uint32_t earlier) noexcept
        {
            const uint32_t delta = later - earlier;
            return delta != 0 && delta < halfRange;
        }

        bool validAcceptedEvidence (
            const ModuleCharacterizationEvidence& evidence) noexcept
        {
            const uint32_t totalCount =
                static_cast<uint32_t> (evidence.ascendingCount) +
                static_cast<uint32_t> (evidence.descendingCount) +
                static_cast<uint32_t> (evidence.verificationCount);
            if (totalCount == 0 || !evidence.firstWitness.present ||
                !evidence.lastWitness.present ||
                evidence.firstWitness.controlOrdinal != 1 ||
                evidence.lastWitness.sequence - evidence.firstWitness.sequence !=
                    static_cast<uint32_t> (totalCount - 1U))
            {
                return false;
            }

            uint8_t terminalCount = evidence.verificationCount;
            if (evidence.terminalLeg == ModuleCharacterizationLeg::Ascending)
            {
                terminalCount = evidence.ascendingCount;
            }
            else if (evidence.terminalLeg == ModuleCharacterizationLeg::Descending)
            {
                terminalCount = evidence.descendingCount;
            }
            if (terminalCount != 0)
            {
                return evidence.lastWitness.controlOrdinal == terminalCount;
            }

            const uint8_t priorCount =
                evidence.terminalLeg == ModuleCharacterizationLeg::Verification
                    ? evidence.descendingCount
                    : evidence.ascendingCount;
            return priorCount != 0 &&
                   evidence.lastWitness.controlOrdinal == priorCount;
        }

        bool validBracketCorrelation (
            const ModuleTransitionBracket& bracket, uint8_t count,
            uint16_t sequenceOffset,
            const ModuleCharacterizationEvidence& evidence) noexcept
        {
            if (!bracket.present)
            {
                return true;
            }
            return bracket.after.controlOrdinal <= count &&
                   bracket.before.frame.provenance.sequence -
                           evidence.firstWitness.sequence ==
                       static_cast<uint32_t> (
                           sequenceOffset + bracket.before.controlOrdinal - 1U) &&
                   bracket.after.frame.provenance.sequence -
                           evidence.firstWitness.sequence ==
                       static_cast<uint32_t> (
                           sequenceOffset + bracket.after.controlOrdinal - 1U);
        }

        bool validTerminalCorrelation (
            const ModuleCharacterizationEvidence& evidence) noexcept
        {
            if (!validAcceptedEvidence (evidence) ||
                !validBracketCorrelation (evidence.ascendingBracket,
                                          evidence.ascendingCount, 0, evidence) ||
                !validBracketCorrelation (
                    evidence.descendingBracket, evidence.descendingCount,
                    evidence.ascendingCount, evidence))
            {
                return false;
            }

            if (evidence.ascendingBracket.present &&
                evidence.ascendingBracket.before.legId !=
                    evidence.ascendingBracket.after.legId)
            {
                return false;
            }
            if (evidence.descendingBracket.present &&
                evidence.descendingBracket.before.legId !=
                    evidence.descendingBracket.after.legId)
            {
                return false;
            }

            if (evidence.terminalLeg == ModuleCharacterizationLeg::Ascending)
            {
                return !evidence.descendingBracket.present &&
                       evidence.descendingCount == 0 &&
                       evidence.verificationCount == 0 &&
                       (!evidence.ascendingBracket.present ||
                        evidence.ascendingBracket.before.legId == evidence.legId);
            }
            if (!evidence.ascendingBracket.present ||
                evidence.ascendingCount < 2)
            {
                return false;
            }
            if (evidence.terminalLeg == ModuleCharacterizationLeg::Descending)
            {
                return evidence.verificationCount == 0 &&
                       (!evidence.descendingBracket.present ||
                        evidence.descendingBracket.before.legId == evidence.legId) &&
                       forwardId (
                           evidence.legId,
                           evidence.ascendingBracket.before.legId);
            }
            return evidence.descendingBracket.present &&
                   evidence.descendingCount == evidence.ascendingCount &&
                   forwardId (
                       evidence.descendingBracket.before.legId,
                       evidence.ascendingBracket.before.legId) &&
                   forwardId (evidence.legId,
                              evidence.descendingBracket.before.legId);
        }

        bool validEvidence (const ModuleCharacterizationEvidence& evidence) noexcept
        {
            if (!validateModuleThresholdDescriptor (evidence.descriptor).ok () ||
                evidence.lifecycleGeneration == 0 || evidence.sessionId == 0 ||
                evidence.runId == 0 || evidence.legId == 0 ||
                evidence.characterizationRevision == 0 || evidence.sourceId == 0 ||
                evidence.sourceConfigurationRevision == 0 ||
                !validState (evidence.state) || !validReason (evidence.reason) ||

                !validLeg (evidence.terminalLeg) ||

                !validRelation (evidence.relation) || !validStatus (evidence.status) ||
                evidence.ascendingCount > 16 || evidence.descendingCount > 16 ||
                evidence.verificationCount > 16 ||
                !validBracket (evidence.ascendingBracket, evidence,
                               ModuleCharacterizationLeg::Ascending,
                               ModuleSweepDirection::Increasing) ||
                !validBracket (evidence.descendingBracket, evidence,
                               ModuleCharacterizationLeg::Descending,
                               ModuleSweepDirection::Decreasing) ||
                !validInterval (evidence.guaranteedInactiveInterval,
                                evidence.descriptor.rawDomain) ||
                !validInterval (evidence.guaranteedActiveInterval,
                                evidence.descriptor.rawDomain) ||
                !validInterval (evidence.ambiguityInterval,
                                evidence.descriptor.rawDomain) ||
                !validWitness (evidence.firstWitness, evidence.descriptor.rawDomain) ||
                !validWitness (evidence.lastWitness, evidence.descriptor.rawDomain) ||
                !validWitness (evidence.offendingBefore,
                               evidence.descriptor.rawDomain) ||
                !validWitness (evidence.offendingAfter,
                               evidence.descriptor.rawDomain) ||
                !validTerminalCorrelation (evidence))
            {
                return false;
            }

            if (evidence.state != ModuleCharacterizationState::Complete &&
                evidence.state != ModuleCharacterizationState::Rejected)
            {
                return false;
            }
            if (evidence.state == ModuleCharacterizationState::Complete)
            {
                const ModuleAnalogInterval& inactive =
                    evidence.guaranteedInactiveInterval;
                const ModuleAnalogInterval& active = evidence.guaranteedActiveInterval;
                const ModuleAnalogInterval& ambiguous = evidence.ambiguityInterval;
                const bool                  guaranteedDisjoint =
                    inactive.upper < active.lower || active.upper < inactive.lower;
                const bool ambiguityDisjoint =
                    !ambiguous.present || ((ambiguous.upper < inactive.lower ||
                                            ambiguous.lower > inactive.upper) &&
                                           (ambiguous.upper < active.lower ||
                                            ambiguous.lower > active.upper));
                const uint32_t witnessSequenceDelta =
                    evidence.lastWitness.sequence - evidence.firstWitness.sequence;
                const uint32_t witnessTimeDelta =
                    evidence.lastWitness.observedAt
                        .elapsedSince (evidence.firstWitness.observedAt)
                        .milliseconds ();
                return evidence.reason == ModuleCharacterizationReason::None &&
                       evidence.status.ok () &&
                       evidence.terminalLeg ==
                           ModuleCharacterizationLeg::Verification &&
                       evidence.ascendingCount >= 2 &&
                       evidence.ascendingCount == evidence.descendingCount &&
                       evidence.descendingCount == evidence.verificationCount &&
                       evidence.ascendingBracket.present &&
                       evidence.descendingBracket.present &&
                       validCompleteGeometry (evidence) &&
                       evidence.firstWitness.present && evidence.lastWitness.present &&
                       zeroWitness (evidence.offendingBefore) &&
                       zeroWitness (evidence.offendingAfter) &&
                       inactive.present && active.present && guaranteedDisjoint &&
                       ambiguityDisjoint &&
                       (evidence.relation == ModuleComparatorRelation::Consistent ||
                        evidence.relation == ModuleComparatorRelation::Ambiguous) &&
                       witnessSequenceDelta != 0 && witnessSequenceDelta < halfRange &&
                       witnessTimeDelta < halfRange;
            }
            const bool learningLeg =
                evidence.terminalLeg != ModuleCharacterizationLeg::Verification;
            const bool learningOnlyReason =
                evidence.reason == ModuleCharacterizationReason::DirectionViolation ||
                evidence.reason == ModuleCharacterizationReason::Chatter ||
                evidence.reason ==
                    ModuleCharacterizationReason::NoObservedTransitionActive ||
                evidence.reason ==
                    ModuleCharacterizationReason::NoObservedTransitionInactive ||
                evidence.reason == ModuleCharacterizationReason::AtLowerRail ||
                evidence.reason == ModuleCharacterizationReason::AtUpperRail;
            const bool orientationReason =
                evidence.reason ==
                ModuleCharacterizationReason::TransitionOrientationMismatch;
            const bool disagreementReason =
                evidence.reason ==
                ModuleCharacterizationReason::AnalogComparatorDisagreement;
            const bool noIntervals =
                !evidence.guaranteedInactiveInterval.present &&
                !evidence.guaranteedActiveInterval.present &&
                !evidence.ambiguityInterval.present;
            return evidence.reason != ModuleCharacterizationReason::None &&
                   evidence.status.ok () && evidence.offendingAfter.present &&
                   (learningOnlyReason ? learningLeg : true) &&
                   (orientationReason
                        ? evidence.terminalLeg ==
                              ModuleCharacterizationLeg::Descending
                        : true) &&
                   (disagreementReason
                        ? evidence.terminalLeg ==
                                  ModuleCharacterizationLeg::Verification &&
                              evidence.relation ==
                                  ModuleComparatorRelation::Disagrees
                        : evidence.relation ==
                              ModuleComparatorRelation::Unverified) &&
                   (evidence.terminalLeg ==
                            ModuleCharacterizationLeg::Verification
                        ? validCompleteGeometry (evidence)
                        : noIntervals);
        }

        bool envelopeMatches (const ModuleBenchConfig& config, uint32_t sessionId,
                              const ModuleCharacterizationEnvelope& envelope) noexcept
        {
            const ModuleCharacterizationEvidence& evidence   = envelope.evidence;
            const ModuleThresholdDescriptor&      descriptor = evidence.descriptor;
            return envelope.envelopeRevision == config.envelopeRevision &&
                   sessionId != 0 && evidence.sessionId == sessionId &&
                   descriptor.descriptorId == config.expectedDescriptorId &&
                   descriptor.descriptorRevision == config.expectedDescriptorRevision &&
                   descriptor.schemaRevision ==
                       config.expectedDescriptorSchemaRevision &&
                   descriptor.declaredSpecimenRevision ==
                       config.expectedDeclaredSpecimenRevision &&
                   descriptor.declaredElectricalEvidenceRevision ==
                       config.expectedDeclaredElectricalEvidenceRevision &&
                   envelope.descriptorDigest == config.expectedDescriptorDigest &&
                   envelope.descriptorDigest ==
                       moduleThresholdDescriptorDigest (descriptor) &&
                   envelope.evidenceDigest ==
                       moduleCharacterizationEvidenceDigest (evidence) &&
                   validEvidence (evidence);
        }

        ModuleCompactBracket
        compactBracket (const ModuleTransitionBracket& bracket) noexcept
        {
            if (!bracket.present)
            {
                return {};
            }
            return {true,
                    bracket.before.frame.analogRaw,
                    bracket.after.frame.analogRaw,
                    bracket.before.frame.comparatorAsserted,
                    bracket.after.frame.comparatorAsserted,
                    bracket.before.frame.provenance.sequence,
                    bracket.after.frame.provenance.sequence};
        }

        ModuleCharacterizationRecord
        makeRecord (const ModuleBenchConfig& config, uint32_t generation,
                    uint32_t                              sessionId,
                    const ModuleCharacterizationEnvelope& envelope) noexcept
        {
            const ModuleCharacterizationEvidence& evidence   = envelope.evidence;
            const ModuleThresholdDescriptor&      descriptor = evidence.descriptor;
            return {config.recordSchemaRevision,
                    config.benchRevision,
                    descriptor.schemaRevision,
                    envelope.envelopeRevision,
                    generation,
                    sessionId,
                    descriptor.descriptorId,
                    descriptor.descriptorRevision,
                    descriptor.declaredSpecimenReference,
                    descriptor.declaredSpecimenRevision,
                    descriptor.declaredElectricalEvidenceRevision,
                    descriptor.channelTopology,
                    descriptor.comparatorOutputStage,
                    descriptor.pullRequirement,
                    descriptor.declaredPullRail,
                    descriptor.comparatorPolarity,
                    descriptor.thresholdControlKind,
                    descriptor.thresholdDirection,
                    descriptor.declaredSupplyMillivolts,
                    descriptor.declaredSignalMillivolts,
                    descriptor.rawDomain,
                    descriptor.warmup,
                    descriptor.settling,
                    evidence.runId,
                    evidence.lifecycleGeneration,
                    evidence.characterizationRevision,
                    evidence.sourceId,
                    evidence.sourceConfigurationRevision,
                    evidence.ascendingCount,
                    evidence.descendingCount,
                    evidence.verificationCount,
                    compactBracket (evidence.ascendingBracket),
                    compactBracket (evidence.descendingBracket),
                    evidence.guaranteedInactiveInterval,
                    evidence.guaranteedActiveInterval,
                    evidence.ambiguityInterval,
                    evidence.relation,
                    moduleCompactWitnessDigest (evidence.firstWitness),
                    moduleCompactWitnessDigest (evidence.lastWitness),
                    moduleCompactWitnessDigest (evidence.offendingBefore),
                    moduleCompactWitnessDigest (evidence.offendingAfter),
                    evidence.firstWitness.present ? evidence.firstWitness.sequence : 0,
                    evidence.lastWitness.present ? evidence.lastWitness.sequence : 0,
                    envelope.descriptorDigest,
                    envelope.evidenceDigest,
                    evidence.state,
                    evidence.reason,
                    evidence.status,
                    ModuleBenchScriptStep::InspectDeclaration};
        }

        bool unhealthy (const ModuleCharacterizationEvidence& evidence) noexcept
        {
            return evidence.state == ModuleCharacterizationState::Rejected ||
                   evidence.reason != ModuleCharacterizationReason::None ||
                   !evidence.status.ok () ||
                   evidence.relation == ModuleComparatorRelation::Unverified ||
                   evidence.relation == ModuleComparatorRelation::Disagrees;
        }

        ModuleBenchPresentationIntent presentation (ModuleBenchScriptStep    step,
                                                    ModuleBenchState         state,
                                                    ModuleComparatorRelation relation,
                                                    bool fault) noexcept
        {
            return {step, state, fault, relation};
        }
    } // namespace

    InertModuleCharacterizationBench::InertModuleCharacterizationBench (
        const ModuleBenchConfig& config) noexcept
        : config_ (config), record_ (), preparedImage_{},
          result_{0,
                  0,
                  ModuleBenchState::Inert,
                  ModuleBenchScriptStep::InspectDeclaration,
                  0,
                  0,
                  0,
                  ModuleComparatorRelation::Unverified,
                  presentation (ModuleBenchScriptStep::InspectDeclaration,
                                ModuleBenchState::Inert,
                                ModuleComparatorRelation::Unverified, false),
                  false,
                  StatusCode::NotInitialized},
          lastControl_ (), lastOperationAt_ (), lastSessionId_ (0),
          initialized_ (false), shutdown_ (false), hasRecord_ (false),

          hasControl_ (false)
    {
    }

    Status InertModuleCharacterizationBench::initialize (TimePoint now) noexcept
    {
        if (shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validConfiguration (config_))
        {
            return StatusCode::InvalidConfiguration;
        }
        if (initialized_)
        {
            return StatusCode::InvalidArgument;
        }

        result_          = {1,
                            0,
                            ModuleBenchState::Ready,
                            ModuleBenchScriptStep::InspectDeclaration,
                            0,
                            0,
                            0,
                            ModuleComparatorRelation::Unverified,
                            presentation (ModuleBenchScriptStep::InspectDeclaration,
                                          ModuleBenchState::Ready,
                                          ModuleComparatorRelation::Unverified, false),
                            false,
                            StatusCode::Ok};
        lastOperationAt_ = now;
        initialized_     = true;
        return StatusCode::Ok;
    }

    Status InertModuleCharacterizationBench::beginSession (
        TimePoint now, uint32_t sessionId,
        const ModuleCharacterizationEnvelope& envelope) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (result_.state != ModuleBenchState::Ready ||
            !forwardOrEqual (now, lastOperationAt_) || sessionId == 0 ||
            (lastSessionId_ != 0 && (sessionId - lastSessionId_ == 0 ||
                                     sessionId - lastSessionId_ >= halfRange)) ||
            !envelopeMatches (config_, sessionId, envelope))
        {
            return StatusCode::InvalidArgument;
        }

        const ModuleCharacterizationRecord candidate =
            makeRecord (config_, result_.lifecycleGeneration, sessionId, envelope);
        const bool             fault = unhealthy (envelope.evidence);
        const ModuleBenchState state =
            fault ? ModuleBenchState::Fault : ModuleBenchState::ScriptActive;
        record_                  = candidate;
        result_.sessionId        = sessionId;
        result_.state            = state;
        result_.step             = ModuleBenchScriptStep::InspectDeclaration;
        result_.runId            = envelope.evidence.runId;
        result_.descriptorDigest = envelope.descriptorDigest;
        result_.evidenceDigest   = envelope.evidenceDigest;
        result_.relation         = envelope.evidence.relation;
        result_.presentation =
            presentation (result_.step, state, result_.relation, fault);
        result_.recordPrepared = false;
        result_.status   = fault ? envelope.evidence.status : Status (StatusCode::Ok);
        lastOperationAt_ = now;
        lastSessionId_   = sessionId;
        hasRecord_       = true;
        hasControl_      = false;
        return StatusCode::Ok;
    }

    Status InertModuleCharacterizationBench::applyCommand (
        TimePoint now, const ModuleBenchControl& control) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (result_.state != ModuleBenchState::ScriptActive &&
            result_.state != ModuleBenchState::Ready)
        {
            return StatusCode::InvalidArgument;
        }
        if (!validStatus (control.producerStatus) ||
            control.sourceId != config_.expectedControlSourceId ||
            control.sourceConfigurationRevision !=
                config_.expectedControlSourceConfigurationRevision ||
            control.sessionId != result_.sessionId ||
            !forwardOrEqual (now, lastOperationAt_) ||

            !currentAt (now, control.observedAt, config_.maximumControlAge) ||
            (control.command != ModuleBenchCommand::None &&
             control.command != ModuleBenchCommand::Advance) ||
            (!hasControl_ && control.sequence == 0))
        {
            return StatusCode::InvalidArgument;
        }

        if (hasControl_)
        {
            const uint32_t delta = control.sequence - lastControl_.sequence;
            if (delta == 0)
            {
                return sameControl (control, lastControl_)
                           ? Status (StatusCode::Ok)
                           : Status (StatusCode::InvalidArgument);
            }
            if (delta != 1 || delta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
            if (!forwardOrEqual (control.observedAt, lastControl_.observedAt))
            {
                return StatusCode::InvalidArgument;
            }
        }

        ModuleBenchResult candidate = result_;
        if (!control.producerStatus.ok ())
        {
            candidate.state        = ModuleBenchState::Fault;
            candidate.status       = control.producerStatus;
            candidate.presentation = presentation (candidate.step, candidate.state,
                                                   candidate.relation, true);
        }
        else if (control.command == ModuleBenchCommand::Advance)
        {
            if (candidate.state != ModuleBenchState::ScriptActive)
            {
                return StatusCode::InvalidArgument;
            }
            if (candidate.step != ModuleBenchScriptStep::PrepareRecord)
            {
                candidate.step = static_cast<ModuleBenchScriptStep> (
                    static_cast<uint8_t> (candidate.step) + 1U);
                candidate.presentation = presentation (candidate.step, candidate.state,
                                                       candidate.relation, false);
            }
        }

        result_          = candidate;
        lastControl_     = control;
        lastOperationAt_ = now;
        hasControl_      = true;
        return StatusCode::Ok;
    }

    Status InertModuleCharacterizationBench::prepareRecord (
        TimePoint now, ModuleCharacterizationRecordImage& output) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (!forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }
        if (result_.state == ModuleBenchState::RecordPrepared)
        {
            output = preparedImage_;
            return StatusCode::Ok;
        }
        if (result_.state != ModuleBenchState::ScriptActive ||
            result_.step != ModuleBenchScriptStep::PrepareRecord || !hasRecord_)
        {
            return StatusCode::InvalidArgument;
        }

        ModuleCharacterizationRecord candidate = record_;
        candidate.scriptStep                   = ModuleBenchScriptStep::PrepareRecord;
        ModuleCharacterizationRecordImage staged{};
        const Result<uint16_t> encoded = ModuleCharacterizationRecordCodec ().encode (
            candidate,
            MutableByteSpan{staged.bytes, ModuleCharacterizationRecordImage::size});
        if (!encoded.ok () ||
            encoded.value () != ModuleCharacterizationRecordImage::size)
        {
            return encoded.ok () ? Status (StatusCode::InternalInvariant)
                                 : encoded.status ();
        }

        output                 = staged;
        preparedImage_         = staged;
        record_                = candidate;
        result_.state          = ModuleBenchState::RecordPrepared;
        result_.step           = ModuleBenchScriptStep::PrepareRecord;
        result_.recordPrepared = true;
        result_.status         = StatusCode::Ok;
        result_.presentation =
            presentation (result_.step, result_.state, result_.relation, false);
        lastOperationAt_ = now;
        return StatusCode::Ok;
    }

    Status InertModuleCharacterizationBench::reset (TimePoint now) noexcept
    {
        if (!initialized_ || shutdown_)
        {
            return StatusCode::NotInitialized;
        }
        if (!forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }
        if (result_.lifecycleGeneration == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }

        const uint32_t generation = result_.lifecycleGeneration + 1U;
        record_                   = ModuleCharacterizationRecord ();
        preparedImage_            = {};
        result_          = {generation,
                            0,
                            ModuleBenchState::Ready,
                            ModuleBenchScriptStep::InspectDeclaration,
                            0,
                            0,
                            0,
                            ModuleComparatorRelation::Unverified,
                            presentation (ModuleBenchScriptStep::InspectDeclaration,
                                          ModuleBenchState::Ready,
                                          ModuleComparatorRelation::Unverified, false),
                            false,
                            StatusCode::Ok};
        lastControl_     = ModuleBenchControl ();
        lastOperationAt_ = now;
        hasRecord_       = false;
        hasControl_      = false;
        return StatusCode::Ok;
    }

    Status InertModuleCharacterizationBench::shutdown (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (shutdown_)
        {
            return StatusCode::Ok;
        }
        if (!forwardOrEqual (now, lastOperationAt_))
        {
            return StatusCode::InvalidArgument;
        }

        record_                = ModuleCharacterizationRecord ();
        preparedImage_         = {};
        result_.state          = ModuleBenchState::Shutdown;
        result_.recordPrepared = false;
        result_.status         = StatusCode::Ok;
        result_.presentation =
            presentation (result_.step, result_.state, result_.relation, false);
        lastOperationAt_ = now;
        hasRecord_       = false;
        hasControl_      = false;
        shutdown_        = true;
        return StatusCode::Ok;
    }

    Status
    InertModuleCharacterizationBench::result (ModuleBenchResult& output) const noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        output = result_;
        return StatusCode::Ok;
    }

#if defined(ADK_TESTING)
    void InertModuleCharacterizationBench::seedLifecycleGenerationForTest (
        uint32_t generation) noexcept
    {
        result_.lifecycleGeneration = generation;
    }
#endif
} // namespace adk
