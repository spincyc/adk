#pragma once

#include "magnetic_observation.h"
#include "resistive_probe_observation.h"
#include "status.h"
#include "thermal_radiant_observation.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct MuseumCaseHealth : uint8_t
    {
        Qualifying = 0,
        Healthy    = 1,
        Warning    = 2,
        Alarm      = 3,
        Fault      = 4,
        Cooldown   = 5
    };

    enum struct MuseumHazard : uint8_t
    {
        None      = 0,
        Liquid    = 1,
        Thermal   = 2,
        Radiant   = 4,
        Opening   = 8,
        Sensing   = 16,
        Recording = 32
    };

    struct MuseumReedEvidence
    {
        uint8_t             sourceId;
        uint16_t            configurationRevision;
        uint32_t            sequence;
        MagneticObservation observation;
    };

    struct MuseumAcknowledgeEvidence
    {
        uint8_t   sourceId;
        uint16_t  configurationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
        bool      pressed;
        Status    status;
    };

    struct MuseumAuditIntent
    {
        uint32_t         ownerToken;
        uint32_t         lifecycleGeneration;
        uint16_t         configurationRevision;
        uint32_t         recordSequence;
        TimePoint        observedAt;
        MuseumCaseHealth health;
        uint8_t          hazardMask;
        uint32_t         liquidSequence;
        uint32_t         thermistorSequence;
        uint32_t         digitalTemperatureSequence;
        uint32_t         radiantSequence;
        uint32_t         reedSequence;
        uint32_t         acknowledgeSequence;
        uint32_t         witnessDigest;
        TimePoint        issuedAt;
        uint8_t          attempt;
    };

    struct MuseumAuditReceipt
    {
        uint32_t         ownerToken;
        uint32_t         lifecycleGeneration;
        uint16_t         configurationRevision;
        uint32_t         recordSequence;
        TimePoint        observedAt;
        MuseumCaseHealth health;
        uint8_t          hazardMask;
        uint32_t         liquidSequence;
        uint32_t         thermistorSequence;
        uint32_t         digitalTemperatureSequence;
        uint32_t         radiantSequence;
        uint32_t         reedSequence;
        uint32_t         acknowledgeSequence;
        uint32_t         witnessDigest;
        uint8_t          attempt;
        bool             accepted;
        Status           status;
    };

    struct MuseumCaseConfig
    {
        // Caller-unique correlation value; it is not an authentication secret.
        uint32_t ownerToken;
        uint16_t configurationRevision;
        uint8_t  expectedLiquidSourceId;
        uint16_t expectedLiquidConfigurationRevision;
        uint16_t expectedLiquidCalibrationRevision;
        uint8_t  expectedThermistorSourceId;
        uint16_t expectedThermistorConfigurationRevision;
        uint16_t expectedThermistorCalibrationRevision;
        uint8_t  expectedDigitalTemperatureSourceId;
        uint16_t expectedDigitalTemperatureConfigurationRevision;
        uint16_t expectedDigitalTemperatureCalibrationRevision;
        uint8_t  expectedRadiantSourceId;
        uint16_t expectedRadiantConfigurationRevision;
        uint16_t expectedRadiantCalibrationRevision;
        Duration maximumLiquidAge;
        Duration maximumThermistorAge;
        Duration maximumDigitalTemperatureAge;
        Duration maximumRadiantAge;
        uint8_t  expectedReedSourceId;
        uint16_t expectedReedConfigurationRevision;
        uint8_t  expectedAcknowledgeSourceId;
        uint16_t expectedAcknowledgeConfigurationRevision;
        Duration maximumReedAge;
        Duration maximumAcknowledgeAge;
        Duration healthyCooldown;
        Duration auditReceiptDeadline;
        uint8_t  maximumAuditAttempts;
    };

    struct MuseumCaseEnvelope
    {
        TimePoint                 now;
        ResistiveProbeObservation liquid;
        ThermalRadiantObservation environment;
        MuseumReedEvidence        reed;
        MuseumAcknowledgeEvidence acknowledge;
        bool                      hasAuditReceipt;
        MuseumAuditReceipt        auditReceipt;
    };

    struct MuseumCaseIntent
    {
        uint32_t         ownerToken;
        uint32_t         lifecycleGeneration;
        uint16_t         configurationRevision;
        MuseumCaseHealth health;
        uint8_t          hazardMask;
        uint8_t          rgbBlinkCode;
        bool             lcdShowsAgeOrFault;
        bool             alarmSoundIntent;
        bool             inertRelayLampIntent;
        bool             alarmOutputInactive;
    };

    struct MuseumCaseResult
    {
        MuseumCaseIntent  intent;
        bool              hasAuditIntent;
        MuseumAuditIntent auditIntent;
        Status            status;
    };

    // Pure copied-observation coordinator; all output values are inert intent.
    struct MuseumCaseMonitor
    {
        explicit MuseumCaseMonitor (const MuseumCaseConfig& config) noexcept;

        MuseumCaseMonitor (const MuseumCaseMonitor&)            = delete;
        MuseumCaseMonitor& operator= (const MuseumCaseMonitor&) = delete;
        MuseumCaseMonitor (MuseumCaseMonitor&&)                 = delete;
        MuseumCaseMonitor& operator= (MuseumCaseMonitor&&)      = delete;

        Status initialize (TimePoint now) noexcept;
        Status reset      (TimePoint now) noexcept;
        Status update     (const MuseumCaseEnvelope& envelope,
                           MuseumCaseResult&         result) noexcept;
        Status shutdown   () noexcept;

        MuseumCaseIntent snapshot    () const noexcept;
        bool             initialized () const noexcept;

      private:
        MuseumCaseConfig   config_;
        MuseumCaseIntent   intent_;
        MuseumCaseEnvelope lastEnvelope_;
        MuseumAuditIntent  outstanding_;
        MuseumAuditIntent  dirtySuccessor_;
        MuseumAuditIntent  latestDecision_;
        MuseumAuditReceipt retiredReceipt_;
        TimePoint          lastUpdateAt_;
        TimePoint          cooldownSince_;
        uint32_t           lifecycleGeneration_;
        uint32_t           nextRecordSequence_;
        bool               initialized_;
        bool               generationExhausted_;
        bool               recordSequenceExhausted_;
        bool               hasEnvelope_;
        bool               alarmLatched_;
        bool               cooldownActive_;
        bool               hasDecision_;
        bool               hasOutstanding_;
        bool               hasDirtySuccessor_;
        bool               hasRetiredReceipt_;
        bool               retryNeedsIssue_;
        bool               auditTerminal_;
        bool               recordingFault_;
    };
} // namespace adk
