#include "museum_case_monitor.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        uint8_t hazardBit (MuseumHazard hazard) noexcept
        {
            return static_cast<uint8_t> (hazard);
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validHealth (MuseumCaseHealth health) noexcept
        {
            return health >= MuseumCaseHealth::Qualifying &&
                   health <= MuseumCaseHealth::Cooldown;
        }

        bool validProbeQuality (ProbeQuality quality) noexcept
        {
            return quality >= ProbeQuality::Unqualified &&
                   quality <= ProbeQuality::ProducerFault;
        }

        bool validThermalQuality (ThermalQuality quality) noexcept
        {
            return quality >= ThermalQuality::Unqualified &&
                   quality <= ThermalQuality::ProducerFault;
        }

        bool validRadiantQuality (RadiantQuality quality) noexcept
        {
            return quality >= RadiantQuality::Unqualified &&
                   quality <= RadiantQuality::ProducerFault;
        }

        bool validMagneticObservation (const MagneticObservation& observation) noexcept
        {
            const bool validLevel = observation.rawLevel == Level::Low ||
                                    observation.rawLevel == Level::High;
            const bool validQuality =
                observation.quality >= MagneticQuality::Unqualified &&
                observation.quality <= MagneticQuality::AboveQualifiedRange;
            return observation.source == MagneticSource::ContactDigital &&
                   observation.polarity == MagneticPolarity::Unspecified &&
                   observation.raw <= 1 && validLevel && validQuality &&
                   observation.raw ==
                       static_cast<uint16_t> (
                           observation.rawLevel == Level::High ? 1 : 0) &&
                   !(observation.activationEvent && observation.deactivationEvent) &&
                   (!observation.activationEvent || observation.active) &&
                   (!observation.deactivationEvent || !observation.active);
        }

        MuseumCaseIntent inactiveIntent (const MuseumCaseConfig& config,
                                         uint32_t                generation) noexcept
        {
            return {config.ownerToken,
                    generation,
                    config.configurationRevision,
                    MuseumCaseHealth::Qualifying,
                    0,
                    0,
                    true,
                    false,
                    false,
                    true};
        }

        MuseumAuditIntent emptyAuditIntent () noexcept
        {
            return {0,
                    0,
                    0,
                    0,
                    TimePoint (),
                    MuseumCaseHealth::Qualifying,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    TimePoint (),
                    0};
        }

        MuseumAuditReceipt emptyAuditReceipt () noexcept
        {
            return {
                0, 0, 0, 0, TimePoint (), MuseumCaseHealth::Qualifying, 0, 0, 0, 0, 0,
                0, 0, 0, 0, false,        StatusCode::NotInitialized};
        }

        bool sameMagneticObservation (const MagneticObservation& left,
                                      const MagneticObservation& right) noexcept
        {
            return left.source == right.source && left.raw == right.raw &&
                   left.rawLevel == right.rawLevel &&
                   left.observedAt == right.observedAt &&
                   left.polarity == right.polarity &&
                   left.activationEvent == right.activationEvent &&
                   left.deactivationEvent == right.deactivationEvent &&
                   left.active == right.active && left.stableFor == right.stableFor &&
                   left.quality == right.quality && left.status == right.status;
        }

        bool sameReed (const MuseumReedEvidence& left,
                       const MuseumReedEvidence& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sequence == right.sequence &&
                   sameMagneticObservation (left.observation, right.observation);
        }

        bool sameAcknowledge (const MuseumAcknowledgeEvidence& left,
                              const MuseumAcknowledgeEvidence& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt &&
                   left.pressed == right.pressed && left.status == right.status;
        }

        bool sameProbeSample (const ResistiveProbeSample& left,
                              const ResistiveProbeSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt &&
                   left.energizedRaw == right.energizedRaw &&
                   left.dischargedRaw == right.dischargedRaw &&
                   left.excitationOnTime == right.excitationOnTime &&
                   left.cycleTime == right.cycleTime &&
                   left.excitationObservedOffAfterSample ==
                       right.excitationObservedOffAfterSample &&
                   left.status == right.status;
        }

        bool sameProbeObservation (const ResistiveProbeObservation& left,
                                   const ResistiveProbeObservation& right) noexcept
        {
            return sameProbeSample (left.sample, right.sample) &&
                   left.normalizedPermille == right.normalizedPermille &&
                   left.observedCycleDutyPermille ==
                       right.observedCycleDutyPermille &&
                   left.quality == right.quality && left.age == right.age &&
                   left.status == right.status;
        }

        bool sameThermalSample (const ConvertedThermalSample& left,
                                const ConvertedThermalSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt &&
                   left.milliCelsius == right.milliCelsius &&
                   left.uncertaintyMilliCelsius == right.uncertaintyMilliCelsius &&
                   left.saturated == right.saturated && left.status == right.status;
        }

        bool sameCategoricalSample (const CategoricalThresholdSample& left,
                                    const CategoricalThresholdSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.sequence == right.sequence &&
                   left.observedAt == right.observedAt && left.raw == right.raw &&
                   left.state == right.state && left.saturated == right.saturated &&
                   left.status == right.status;
        }

        bool sameEnvironmentObservation (
            const ThermalRadiantObservation& left,
            const ThermalRadiantObservation& right) noexcept
        {
            return sameThermalSample (
                       left.envelope.thermistor, right.envelope.thermistor) &&
                   sameCategoricalSample (left.envelope.digitalTemperature,
                                          right.envelope.digitalTemperature) &&
                   sameCategoricalSample (
                       left.envelope.radiant, right.envelope.radiant) &&
                   left.thermalQuality == right.thermalQuality &&
                   left.radiantQuality == right.radiantQuality &&
                   left.thermistorAge == right.thermistorAge &&
                   left.digitalTemperatureAge == right.digitalTemperatureAge &&
                   left.radiantAge == right.radiantAge &&
                   left.thermalHazard == right.thermalHazard &&
                   left.radiantHazard == right.radiantHazard &&
                   left.status == right.status;
        }

        template <typename Sample, typename Equal>
        bool validOrder (const Sample& sample, const Sample& accepted,
                         Equal equal) noexcept
        {
            const uint32_t delta = sample.sequence - accepted.sequence;
            if (delta == 0)
            {
                return equal (sample, accepted);
            }

            return accepted.sequence != UINT32_MAX && delta < halfRange &&
                   sample.observedAt != accepted.observedAt &&
                   sample.observedAt.elapsedSince (accepted.observedAt)
                           .milliseconds () < halfRange;
        }

        bool validReedOrder (const MuseumReedEvidence& sample,
                             const MuseumReedEvidence& accepted) noexcept
        {
            const uint32_t delta = sample.sequence - accepted.sequence;
            if (delta == 0)
            {
                return sameReed (sample, accepted);
            }

            return accepted.sequence != UINT32_MAX && delta < halfRange &&
                   sample.observation.observedAt != accepted.observation.observedAt &&
                   sample.observation.observedAt
                           .elapsedSince (accepted.observation.observedAt)
                           .milliseconds () < halfRange;
        }

        bool sameDecision (const MuseumAuditIntent& left,
                           const MuseumAuditIntent& right) noexcept
        {
            return left.health == right.health && left.hazardMask == right.hazardMask;
        }

        bool sameReceipt (const MuseumAuditReceipt& left,
                          const MuseumAuditReceipt& right) noexcept
        {
            return left.ownerToken == right.ownerToken &&
                   left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.configurationRevision == right.configurationRevision &&
                   left.recordSequence == right.recordSequence &&
                   left.observedAt == right.observedAt && left.health == right.health &&
                   left.hazardMask == right.hazardMask &&
                   left.liquidSequence == right.liquidSequence &&
                   left.thermistorSequence == right.thermistorSequence &&
                   left.digitalTemperatureSequence ==
                       right.digitalTemperatureSequence &&
                   left.radiantSequence == right.radiantSequence &&
                   left.reedSequence == right.reedSequence &&
                   left.acknowledgeSequence == right.acknowledgeSequence &&
                   left.witnessDigest == right.witnessDigest &&
                   left.attempt == right.attempt && left.accepted == right.accepted &&
                   left.status == right.status;
        }

        bool receiptMatches (const MuseumAuditReceipt& receipt,
                             const MuseumAuditIntent&  intent) noexcept
        {
            return receipt.ownerToken == intent.ownerToken &&
                   receipt.lifecycleGeneration == intent.lifecycleGeneration &&
                   receipt.configurationRevision == intent.configurationRevision &&
                   receipt.recordSequence == intent.recordSequence &&
                   receipt.observedAt == intent.observedAt &&
                   receipt.health == intent.health &&
                   receipt.hazardMask == intent.hazardMask &&
                   receipt.liquidSequence == intent.liquidSequence &&
                   receipt.thermistorSequence == intent.thermistorSequence &&
                   receipt.digitalTemperatureSequence ==
                       intent.digitalTemperatureSequence &&
                   receipt.radiantSequence == intent.radiantSequence &&
                   receipt.reedSequence == intent.reedSequence &&
                   receipt.acknowledgeSequence == intent.acknowledgeSequence &&
                   receipt.witnessDigest == intent.witnessDigest &&
                   receipt.attempt == intent.attempt;
        }

        void hashByte (uint32_t& hash, uint8_t value) noexcept
        {
            hash ^= value;
            hash *= 0x01000193UL;
        }

        void hash16 (uint32_t& hash, uint16_t value) noexcept
        {
            hashByte (hash, static_cast<uint8_t> (value));
            hashByte (hash, static_cast<uint8_t> (value >> 8U));
        }

        void hash32 (uint32_t& hash, uint32_t value) noexcept
        {
            hashByte (hash, static_cast<uint8_t> (value));
            hashByte (hash, static_cast<uint8_t> (value >> 8U));
            hashByte (hash, static_cast<uint8_t> (value >> 16U));
            hashByte (hash, static_cast<uint8_t> (value >> 24U));
        }

        uint32_t witnessDigest (const MuseumAuditIntent& intent) noexcept
        {
            static const char domain[] = "ADK.MUSEUM.AUDIT.V1";
            uint32_t          hash     = 0x811c9dc5UL;
            for (uint8_t index = 0; index < sizeof (domain) - 1U; ++index)
            {
                hashByte (hash, static_cast<uint8_t> (domain[index]));
            }

            hash32   (hash, intent.ownerToken);
            hash32   (hash, intent.lifecycleGeneration);
            hash16   (hash, intent.configurationRevision);
            hash32   (hash, intent.recordSequence);
            hash32   (hash, intent.observedAt.milliseconds ());
            hashByte (hash, static_cast<uint8_t> (intent.health));
            hashByte (hash, intent.hazardMask);
            hash32   (hash, intent.liquidSequence);
            hash32   (hash, intent.thermistorSequence);
            hash32   (hash, intent.digitalTemperatureSequence);
            hash32   (hash, intent.radiantSequence);
            hash32   (hash, intent.reedSequence);
            hash32   (hash, intent.acknowledgeSequence);
            return hash;
        }

        MuseumAuditReceipt receiptFrom (const MuseumAuditIntent& intent, bool accepted,
                                        Status status) noexcept
        {
            return {intent.ownerToken,
                    intent.lifecycleGeneration,
                    intent.configurationRevision,
                    intent.recordSequence,
                    intent.observedAt,
                    intent.health,
                    intent.hazardMask,
                    intent.liquidSequence,
                    intent.thermistorSequence,
                    intent.digitalTemperatureSequence,
                    intent.radiantSequence,
                    intent.reedSequence,
                    intent.acknowledgeSequence,
                    intent.witnessDigest,
                    intent.attempt,
                    accepted,
                    status};
        }
    } // namespace

    MuseumCaseMonitor::MuseumCaseMonitor (const MuseumCaseConfig& config) noexcept
        : config_                      (config),
          intent_                      (inactiveIntent (config, 0)),
          lastEnvelope_                (),
          outstanding_                 (emptyAuditIntent ()),
          dirtySuccessor_              (emptyAuditIntent ()),
          latestDecision_              (emptyAuditIntent ()),
          retiredReceipt_              (emptyAuditReceipt ()),
          lastUpdateAt_                (),
          cooldownSince_               (),
          lifecycleGeneration_         (0),
          nextRecordSequence_          (0),
          initialized_                 (false),
          generationExhausted_         (false),
          recordSequenceExhausted_     (false),
          hasEnvelope_                 (false),
          alarmLatched_                (false),
          cooldownActive_              (false),
          hasDecision_                 (false),
          hasOutstanding_              (false),
          hasDirtySuccessor_           (false),
          hasRetiredReceipt_           (false),
          retryNeedsIssue_             (false),
          auditTerminal_               (false),
          recordingFault_              (false)
    {
    }

    Status MuseumCaseMonitor::initialize (TimePoint now) noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const uint8_t sourceIds[]     = {config_.expectedLiquidSourceId,
                                         config_.expectedThermistorSourceId,
                                         config_.expectedDigitalTemperatureSourceId,
                                         config_.expectedRadiantSourceId,
                                         config_.expectedReedSourceId,
                                         config_.expectedAcknowledgeSourceId};
        bool          duplicateSource = false;
        for (uint8_t left = 0; left < 6; ++left)
        {
            for (uint8_t right = left + 1; right < 6; ++right)
            {
                duplicateSource =
                    duplicateSource || sourceIds[left] == sourceIds[right];
            }
        }

        const uint32_t liquidAge     = config_.maximumLiquidAge.milliseconds ();

        const uint32_t thermistorAge = config_.maximumThermistorAge.milliseconds ();

        const uint32_t digitalAge =
            config_.maximumDigitalTemperatureAge.milliseconds ();

        const uint32_t radiantAge     = config_.maximumRadiantAge.milliseconds ();

        const uint32_t reedAge        = config_.maximumReedAge.milliseconds ();

        const uint32_t acknowledgeAge = config_.maximumAcknowledgeAge.milliseconds ();

        const uint32_t cooldown       = config_.healthyCooldown.milliseconds ();

        const uint32_t deadline       = config_.auditReceiptDeadline.milliseconds ();
        if (config_.ownerToken == 0 || config_.configurationRevision == 0 ||
            duplicateSource || config_.expectedLiquidSourceId == 0 ||
            config_.expectedThermistorSourceId == 0 ||
            config_.expectedDigitalTemperatureSourceId == 0 ||
            config_.expectedRadiantSourceId == 0 || config_.expectedReedSourceId == 0 ||
            config_.expectedAcknowledgeSourceId == 0 ||
            config_.expectedLiquidConfigurationRevision == 0 ||
            config_.expectedLiquidCalibrationRevision == 0 ||
            config_.expectedThermistorConfigurationRevision == 0 ||
            config_.expectedThermistorCalibrationRevision == 0 ||
            config_.expectedDigitalTemperatureConfigurationRevision == 0 ||
            config_.expectedDigitalTemperatureCalibrationRevision == 0 ||
            config_.expectedRadiantConfigurationRevision == 0 ||
            config_.expectedRadiantCalibrationRevision == 0 ||
            config_.expectedReedConfigurationRevision == 0 ||
            config_.expectedAcknowledgeConfigurationRevision == 0 || liquidAge == 0 ||
            liquidAge >= halfRange || thermistorAge == 0 ||
            thermistorAge >= halfRange || digitalAge == 0 || digitalAge >= halfRange ||
            radiantAge == 0 || radiantAge >= halfRange || reedAge == 0 ||
            reedAge >= halfRange || acknowledgeAge == 0 ||
            acknowledgeAge >= halfRange || cooldown == 0 || cooldown >= halfRange ||
            deadline == 0 || deadline >= halfRange ||
            config_.maximumAuditAttempts == 0 || config_.maximumAuditAttempts > 8)
        {
            return StatusCode::InvalidConfiguration;
        }

        if (generationExhausted_ || lifecycleGeneration_ == UINT32_MAX)
        {
            generationExhausted_ = true;
            intent_              = inactiveIntent (config_, lifecycleGeneration_);
            return StatusCode::CapacityExceeded;
        }

        initialized_ = true;
        return reset (now);
    }

    Status MuseumCaseMonitor::reset (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (generationExhausted_ || lifecycleGeneration_ == UINT32_MAX)
        {
            generationExhausted_ = true;
            initialized_         = false;
            intent_              = inactiveIntent (config_, lifecycleGeneration_);
            return StatusCode::CapacityExceeded;
        }

        ++lifecycleGeneration_;
        intent_                  = inactiveIntent       (config_, lifecycleGeneration_);
        lastEnvelope_            = MuseumCaseEnvelope   ();
        outstanding_             = emptyAuditIntent     ();
        dirtySuccessor_          = emptyAuditIntent     ();
        latestDecision_          = emptyAuditIntent     ();
        retiredReceipt_          = emptyAuditReceipt    ();
        lastUpdateAt_            = now;
        cooldownSince_           = TimePoint ();
        hasEnvelope_             = false;
        alarmLatched_            = false;
        cooldownActive_          = false;
        hasDecision_             = false;
        hasOutstanding_          = false;
        hasDirtySuccessor_       = false;
        hasRetiredReceipt_       = false;
        retryNeedsIssue_         = false;
        auditTerminal_           = false;
        recordingFault_          = false;
        recordSequenceExhausted_ = nextRecordSequence_ == UINT32_MAX;
        return StatusCode::Ok;
    }

    Status MuseumCaseMonitor::update (const MuseumCaseEnvelope& envelope,
                                      MuseumCaseResult&         result) noexcept
    {
        result = {intent_,
                  false,
                  emptyAuditIntent (),
                  StatusCode::InvalidArgument};
        if (!initialized_)
        {
            result = {inactiveIntent (config_, lifecycleGeneration_),
                      false,
                      emptyAuditIntent (),
                      StatusCode::NotInitialized};
            return result.status;
        }

        const ResistiveProbeSample&   liquid = envelope.liquid.sample;
        const ConvertedThermalSample& thermistor =
            envelope.environment.envelope.thermistor;
        const CategoricalThresholdSample& digital =
            envelope.environment.envelope.digitalTemperature;
        const CategoricalThresholdSample& radiant =
            envelope.environment.envelope.radiant;
        const uint32_t reedAge =
            envelope.now.elapsedSince (envelope.reed.observation.observedAt)
                .milliseconds ();
        const uint32_t liquidAge =
            envelope.now.elapsedSince (liquid.observedAt).milliseconds ();
        const uint32_t thermistorAge =
            envelope.now.elapsedSince (thermistor.observedAt).milliseconds ();
        const uint32_t digitalAge =
            envelope.now.elapsedSince (digital.observedAt).milliseconds ();
        const uint32_t radiantAge =
            envelope.now.elapsedSince (radiant.observedAt).milliseconds ();
        const uint32_t acknowledgeAge =
            envelope.now.elapsedSince (envelope.acknowledge.observedAt).milliseconds ();

        if (envelope.now.elapsedSince (lastUpdateAt_).milliseconds () >= halfRange ||
            liquidAge >= halfRange || thermistorAge >= halfRange ||
            digitalAge >= halfRange || radiantAge >= halfRange ||
            reedAge >= halfRange || acknowledgeAge >= halfRange ||
            !validStatus         (envelope.liquid.status) ||
            !validStatus         (envelope.environment.status) ||
            !validStatus         (envelope.reed.observation.status) ||
            !validStatus         (envelope.acknowledge.status) ||
            !validStatus         (liquid.status) ||
            !validStatus         (thermistor.status) ||
            !validStatus         (digital.status) ||
            !validStatus         (radiant.status) ||
            !validProbeQuality   (envelope.liquid.quality) ||
            !validThermalQuality (envelope.environment.thermalQuality) ||
            !validRadiantQuality (envelope.environment.radiantQuality) ||
            liquid.sequence == 0 || thermistor.sequence == 0 ||
            digital.sequence == 0 || radiant.sequence == 0 ||
            (envelope.environment.thermalQuality == ThermalQuality::Alarm &&
             !envelope.environment.thermalHazard) ||
            (envelope.environment.radiantHazard &&
             envelope.environment.radiantQuality != RadiantQuality::AbruptChange &&
             envelope.environment.radiantQuality != RadiantQuality::Sustained) ||
            liquid.sourceId != config_.expectedLiquidSourceId ||
            liquid.configurationRevision !=
                config_.expectedLiquidConfigurationRevision ||
            liquid.calibrationRevision != config_.expectedLiquidCalibrationRevision ||
            thermistor.sourceId != config_.expectedThermistorSourceId ||
            thermistor.configurationRevision !=
                config_.expectedThermistorConfigurationRevision ||
            thermistor.calibrationRevision !=
                config_.expectedThermistorCalibrationRevision ||
            digital.sourceId != config_.expectedDigitalTemperatureSourceId ||
            digital.configurationRevision !=
                config_.expectedDigitalTemperatureConfigurationRevision ||
            digital.calibrationRevision !=
                config_.expectedDigitalTemperatureCalibrationRevision ||
            radiant.sourceId != config_.expectedRadiantSourceId ||
            radiant.configurationRevision !=
                config_.expectedRadiantConfigurationRevision ||
            radiant.calibrationRevision != config_.expectedRadiantCalibrationRevision ||
            envelope.reed.sourceId != config_.expectedReedSourceId ||
            envelope.reed.configurationRevision !=
                config_.expectedReedConfigurationRevision ||
            envelope.reed.sequence == 0 ||
            !validMagneticObservation (envelope.reed.observation) ||
            envelope.acknowledge.sourceId != config_.expectedAcknowledgeSourceId ||
            envelope.acknowledge.configurationRevision !=
                config_.expectedAcknowledgeConfigurationRevision ||
            envelope.acknowledge.sequence == 0 ||
            (envelope.hasAuditReceipt &&
             (!validStatus (envelope.auditReceipt.status) ||
              !validHealth (envelope.auditReceipt.health) ||
              envelope.auditReceipt.ownerToken == 0 ||
              envelope.auditReceipt.lifecycleGeneration == 0 ||
              envelope.auditReceipt.configurationRevision == 0 ||
              envelope.auditReceipt.recordSequence == 0 ||
              envelope.auditReceipt.attempt == 0 ||
              envelope.auditReceipt.accepted != envelope.auditReceipt.status.ok ())) ||
            (auditTerminal_ && envelope.hasAuditReceipt))
        {
            return StatusCode::InvalidArgument;
        }

        if (hasEnvelope_)
        {
            const bool validLiquid =
                validOrder (liquid, lastEnvelope_.liquid.sample, sameProbeSample);
            const bool validThermistor =
                validOrder (thermistor, lastEnvelope_.environment.envelope.thermistor,
                            sameThermalSample);
            const bool validDigital = validOrder (
                digital, lastEnvelope_.environment.envelope.digitalTemperature,
                sameCategoricalSample);
            const bool validRadiant =
                validOrder (radiant, lastEnvelope_.environment.envelope.radiant,
                            sameCategoricalSample);
            const bool validReed = validReedOrder (envelope.reed, lastEnvelope_.reed);

            const bool validAcknowledge = validOrder (
                envelope.acknowledge, lastEnvelope_.acknowledge, sameAcknowledge);
            if (!validLiquid || !validThermistor || !validDigital || !validRadiant ||
                !validReed || !validAcknowledge)
            {
                const bool exhaustedChange =
                    (lastEnvelope_.liquid.sample.sequence == UINT32_MAX &&
                     !sameProbeSample (liquid, lastEnvelope_.liquid.sample)) ||
                    (lastEnvelope_.environment.envelope.thermistor.sequence ==
                         UINT32_MAX &&
                     !sameThermalSample (
                         thermistor, lastEnvelope_.environment.envelope.thermistor)) ||
                    (lastEnvelope_.environment.envelope.digitalTemperature.sequence ==
                         UINT32_MAX &&
                     !sameCategoricalSample (
                         digital,
                         lastEnvelope_.environment.envelope.digitalTemperature)) ||
                    (lastEnvelope_.environment.envelope.radiant.sequence ==
                         UINT32_MAX &&
                     !sameCategoricalSample (
                         radiant, lastEnvelope_.environment.envelope.radiant)) ||
                    (lastEnvelope_.reed.sequence == UINT32_MAX &&
                     !sameReed (envelope.reed, lastEnvelope_.reed)) ||
                    (lastEnvelope_.acknowledge.sequence == UINT32_MAX &&
                     !sameAcknowledge (envelope.acknowledge,
                                       lastEnvelope_.acknowledge));
                result.status = exhaustedChange ? StatusCode::CapacityExceeded
                                                : StatusCode::InvalidArgument;
                return result.status;
            }
        }

        const bool retiredReceiptDuplicate =
            envelope.hasAuditReceipt && hasRetiredReceipt_ &&
            sameReceipt (envelope.auditReceipt, retiredReceipt_);
        if (retryNeedsIssue_ && envelope.hasAuditReceipt &&
            !retiredReceiptDuplicate)
        {
            return StatusCode::InvalidArgument;
        }
        if (envelope.hasAuditReceipt && !retiredReceiptDuplicate)
        {
            if (hasOutstanding_)
            {
                if (!receiptMatches (envelope.auditReceipt, outstanding_))
                {
                    return StatusCode::InvalidArgument;
                }
            }
            else
            {
                return StatusCode::InvalidArgument;
            }
        }

        const bool evidenceChanged =
            !hasEnvelope_ ||
            !sameProbeObservation       (envelope.liquid, lastEnvelope_.liquid) ||
            !sameEnvironmentObservation (
                envelope.environment, lastEnvelope_.environment) ||
            !sameReed        (envelope.reed, lastEnvelope_.reed) ||
            !sameAcknowledge (envelope.acknowledge, lastEnvelope_.acknowledge);
        const bool acknowledgeAdvanced =
            !hasEnvelope_ ||
            envelope.acknowledge.sequence != lastEnvelope_.acknowledge.sequence;
        if (hasEnvelope_ && envelope.now == lastUpdateAt_ && evidenceChanged)
        {
            return StatusCode::InvalidArgument;
        }

        bool acceptedReceipt = false;
        bool failedReceipt   = false;
        if (envelope.hasAuditReceipt && hasOutstanding_ &&
            !retiredReceiptDuplicate)
        {
            acceptedReceipt = envelope.auditReceipt.accepted;
            failedReceipt   = !acceptedReceipt;
        }
        const bool deadlineLost = hasOutstanding_ && !envelope.hasAuditReceipt &&
                                  !retryNeedsIssue_ &&
                                  envelope.now.elapsedSince (outstanding_.issuedAt) >
                                      config_.auditReceiptDeadline;

        bool              nextRecordingFault = recordingFault_;
        bool              nextAuditTerminal  = auditTerminal_;
        bool              nextRetryIssue     = retryNeedsIssue_;
        MuseumAuditIntent nextOutstanding    = outstanding_;
        if (acceptedReceipt)
        {
            nextRecordingFault = false;
            nextRetryIssue     = false;
        }
        else if (failedReceipt || deadlineLost)
        {
            nextRecordingFault = true;
            if (nextOutstanding.attempt >= config_.maximumAuditAttempts)
            {
                nextAuditTerminal = true;
                nextRetryIssue    = false;
            }
            else
            {
                ++nextOutstanding.attempt;
                nextRetryIssue = true;
            }
        }

        uint8_t hazardMask = 0;
        bool    qualifying = false;
        bool    warning    = false;
        bool    sensingFault =
            envelope.liquid.quality == ProbeQuality::Stale ||
            envelope.liquid.quality == ProbeQuality::Saturated ||
            envelope.liquid.quality == ProbeQuality::Disconnected ||
            envelope.liquid.quality == ProbeQuality::ExcitationFault ||
            envelope.liquid.quality == ProbeQuality::ProducerFault ||
            envelope.environment.thermalQuality == ThermalQuality::Saturated ||
            envelope.environment.thermalQuality == ThermalQuality::Stale ||
            envelope.environment.thermalQuality == ThermalQuality::ProducerFault ||
            envelope.environment.radiantQuality == RadiantQuality::SaturatedAmbient ||
            envelope.environment.radiantQuality == RadiantQuality::Stale ||
            envelope.environment.radiantQuality == RadiantQuality::ProducerFault ||
            liquidAge > config_.maximumLiquidAge.milliseconds () ||

            thermistorAge > config_.maximumThermistorAge.milliseconds () ||

            digitalAge > config_.maximumDigitalTemperatureAge.milliseconds () ||

            radiantAge > config_.maximumRadiantAge.milliseconds () ||

            !envelope.reed.observation.status.ok () ||
            envelope.reed.observation.quality != MagneticQuality::Valid ||
            reedAge > config_.maximumReedAge.milliseconds () ||

            !envelope.acknowledge.status.ok () ||

            acknowledgeAge > config_.maximumAcknowledgeAge.milliseconds ();
        qualifying =
            envelope.liquid.quality == ProbeQuality::Unqualified ||
            envelope.environment.thermalQuality == ThermalQuality::Unqualified ||
            envelope.environment.radiantQuality == RadiantQuality::Unqualified;

        if (envelope.liquid.quality == ProbeQuality::Wet)
        {
            hazardMask |= hazardBit (MuseumHazard::Liquid);
        }
        else if (envelope.liquid.quality == ProbeQuality::Damp)
        {
            warning = true;
        }
        if (envelope.environment.thermalHazard)
        {
            hazardMask |= hazardBit (MuseumHazard::Thermal);
        }
        else if (envelope.environment.thermalQuality == ThermalQuality::Warning ||
                 envelope.environment.thermalQuality == ThermalQuality::Disagreement)
        {
            warning = true;
        }
        if (envelope.environment.radiantHazard)
        {
            hazardMask |= hazardBit (MuseumHazard::Radiant);
        }
        if (envelope.reed.observation.status.ok () &&
            envelope.reed.observation.quality == MagneticQuality::Valid &&
            !envelope.reed.observation.active)
        {
            hazardMask |= hazardBit (MuseumHazard::Opening);
        }
        if (sensingFault)
        {
            hazardMask |= hazardBit (MuseumHazard::Sensing);
        }
        if (nextRecordingFault || nextAuditTerminal)
        {
            hazardMask |= hazardBit (MuseumHazard::Recording);
        }

        const uint8_t activeHazards =
            hazardMask & static_cast<uint8_t> (hazardBit (MuseumHazard::Liquid) |
                                               hazardBit (MuseumHazard::Thermal) |
                                               hazardBit (MuseumHazard::Radiant) |
                                               hazardBit (MuseumHazard::Opening));
        bool             nextAlarmLatched  = alarmLatched_;
        bool             nextCooldown      = cooldownActive_;
        TimePoint        nextCooldownSince = cooldownSince_;
        MuseumCaseHealth health;
        if (sensingFault || nextAuditTerminal || nextRecordingFault)
        {
            health       = MuseumCaseHealth::Fault;
            nextCooldown = false;
        }
        else if (activeHazards != 0)
        {
            health           = MuseumCaseHealth::Alarm;
            nextAlarmLatched = true;
            nextCooldown     = false;
        }
        else if (qualifying)
        {
            health       = MuseumCaseHealth::Qualifying;
            nextCooldown = false;
        }
        else if (nextAlarmLatched)
        {
            if (warning)
            {
                health       = MuseumCaseHealth::Warning;
                nextCooldown = false;
            }
            else
            {
                if (!nextCooldown && acknowledgeAdvanced &&
                    envelope.acknowledge.pressed)
                {
                    nextCooldown      = true;
                    nextCooldownSince = envelope.now;
                }

                if (nextCooldown && envelope.now.elapsedSince (nextCooldownSince) >=
                                        config_.healthyCooldown)
                {
                    health           = MuseumCaseHealth::Healthy;
                    nextAlarmLatched = false;
                    nextCooldown     = false;
                }
                else
                {
                    health = nextCooldown ? MuseumCaseHealth::Cooldown
                                          : MuseumCaseHealth::Alarm;
                }
            }
        }
        else
        {
            health = warning ? MuseumCaseHealth::Warning : MuseumCaseHealth::Healthy;
        }

        MuseumCaseIntent nextIntent = {config_.ownerToken,
                                       lifecycleGeneration_,
                                       config_.configurationRevision,
                                       health,
                                       hazardMask,
                                       static_cast<uint8_t> (health),
                                       true,
                                       health == MuseumCaseHealth::Alarm,
                                       health == MuseumCaseHealth::Alarm,
                                       health != MuseumCaseHealth::Alarm};

        MuseumAuditIntent current = {config_.ownerToken,
                                     lifecycleGeneration_,
                                     config_.configurationRevision,
                                     0,
                                     envelope.now,
                                     health,
                                     hazardMask,
                                     liquid.sequence,
                                     thermistor.sequence,
                                     digital.sequence,
                                     radiant.sequence,
                                     envelope.reed.sequence,
                                     envelope.acknowledge.sequence,
                                     0,
                                     TimePoint (),
                                     0};

        bool              nextHasOutstanding = hasOutstanding_;
        bool              nextHasDirty       = hasDirtySuccessor_;
        MuseumAuditIntent nextDirty          = dirtySuccessor_;
        if (nextHasOutstanding)
        {
            if (sameDecision (current, nextOutstanding))
            {
                nextDirty    = emptyAuditIntent ();
                nextHasDirty = false;
            }
            else if (nextHasDirty && sameDecision (current, nextDirty))
            {
                nextDirty = current;
            }
            else
            {
                nextDirty    = current;
                nextHasDirty = true;
            }
        }

        if (acceptedReceipt)
        {
            retiredReceipt_ =
                receiptFrom (outstanding_, true, envelope.auditReceipt.status);
            hasRetiredReceipt_ = true;
            nextHasOutstanding = false;
            nextOutstanding    = emptyAuditIntent ();
        }

        bool issueIntent = false;
        const bool idleDecisionChanged =
            !hasDecision_ || !sameDecision (current, latestDecision_);
        if (!nextHasOutstanding && (nextHasDirty || idleDecisionChanged))
        {
            if (recordSequenceExhausted_ || nextRecordSequence_ == UINT32_MAX)
            {
                recordSequenceExhausted_ = true;
                nextAuditTerminal        = true;
                nextRecordingFault       = true;
                nextIntent.health        = MuseumCaseHealth::Fault;
                nextIntent.hazardMask |= hazardBit (MuseumHazard::Recording);
                nextIntent.alarmSoundIntent     = false;
                nextIntent.inertRelayLampIntent = false;
                nextIntent.alarmOutputInactive  = true;
                nextHasDirty                    = nextHasDirty || hasDecision_;
            }
            else
            {
                ++nextRecordSequence_;
                MuseumAuditIntent promoted = nextHasDirty ? nextDirty : current;
                promoted.recordSequence    = nextRecordSequence_;
                promoted.issuedAt          = envelope.now;
                promoted.attempt           = 1;
                promoted.witnessDigest     = witnessDigest (promoted);
                nextOutstanding            = promoted;
                nextHasOutstanding         = true;
                nextHasDirty               = false;
                nextDirty                  = emptyAuditIntent ();
                nextRetryIssue             = false;
                issueIntent                = true;
                recordSequenceExhausted_   = nextRecordSequence_ == UINT32_MAX;
            }
        }
        else if (nextHasOutstanding && retryNeedsIssue_ && nextRetryIssue &&
                 !failedReceipt && !deadlineLost && !nextAuditTerminal)
        {
            nextOutstanding.issuedAt = envelope.now;
            nextRetryIssue           = false;
            issueIntent              = true;
        }
        else if (nextHasOutstanding && !failedReceipt && !deadlineLost &&
                 !retryNeedsIssue_)
        {
            issueIntent = true;
        }

        intent_               = nextIntent;
        lastEnvelope_         = envelope;
        latestDecision_       = current;
        lastUpdateAt_         = envelope.now;
        outstanding_          = nextOutstanding;
        dirtySuccessor_       = nextDirty;
        alarmLatched_         = nextAlarmLatched;
        cooldownActive_       = nextCooldown;
        cooldownSince_        = nextCooldownSince;
        hasEnvelope_          = true;
        hasDecision_          = true;
        hasOutstanding_       = nextHasOutstanding;
        hasDirtySuccessor_    = nextHasDirty;
        retryNeedsIssue_      = nextRetryIssue;
        auditTerminal_        = nextAuditTerminal;
        recordingFault_       = nextRecordingFault;
        result.intent         = intent_;
        result.hasAuditIntent = issueIntent && hasOutstanding_;
        result.auditIntent = result.hasAuditIntent ? outstanding_ : emptyAuditIntent ();
        result.status = auditTerminal_ ? StatusCode::CapacityExceeded : StatusCode::Ok;
        return result.status;
    }

    Status MuseumCaseMonitor::shutdown () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::Ok;
        }

        if (lifecycleGeneration_ == UINT32_MAX)
        {
            generationExhausted_ = true;
        }
        else
        {
            ++lifecycleGeneration_;
        }

        initialized_       = false;
        hasEnvelope_       = false;
        hasOutstanding_    = false;
        hasDirtySuccessor_ = false;
        retryNeedsIssue_   = false;
        auditTerminal_     = false;
        recordingFault_    = false;
        outstanding_       = emptyAuditIntent ();
        dirtySuccessor_    = emptyAuditIntent ();
        intent_            = inactiveIntent   (config_, lifecycleGeneration_);
        return generationExhausted_ ? StatusCode::CapacityExceeded : StatusCode::Ok;
    }

    MuseumCaseIntent MuseumCaseMonitor::snapshot () const noexcept
    {
        return intent_;
    }

    bool MuseumCaseMonitor::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
