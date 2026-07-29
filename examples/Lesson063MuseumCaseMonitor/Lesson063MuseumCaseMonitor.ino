// E0 museum-case fixture. This sketch replays copied observations, supplied
// time, and copied audit receipts into named memory result cells. It owns no
// sensor, clock, bus, storage, display, alarm, relay, lamp, pin, or circuit.
#include <Adk.h>
#include <museum_case_monitor.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        InitialHealthy,
        AcceptHealthy,
        WetAlarm,
        RejectAlarmRecord,
        ReissueAlarmRecord,
        AcceptAlarmRecord,
        RecoverAcknowledged,
        CooldownComplete,
        CaseOpened,
        Shutdown,
        Complete
    };

    struct MuseumResultCell
    {
        uint32_t lifecycleGeneration;
        uint32_t auditRecordSequence;
        uint32_t auditWitnessDigest;
        uint8_t  health;
        uint8_t  hazardMask;
        uint8_t  rgbBlinkCode;
        uint8_t  lcdShowsAgeOrFault;
        uint8_t  alarmSoundIntent;
        uint8_t  inertRelayLampIntent;
        uint8_t  alarmOutputInactive;
        uint8_t  auditIntentPresent;
        uint8_t  auditAttempt;
        uint8_t  status;
        uint8_t  predictionPass;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t liquidInitializeStatus;
        uint8_t environmentInitializeStatus;
        uint8_t monitorInitializeStatus;
        uint8_t completedStages;
        uint8_t predictionsPass;
        uint8_t terminalAssertionPass;
        uint8_t complete;
    };

    constexpr uint8_t  liquidSourceId      = 61;
    constexpr uint8_t  thermistorSourceId  = 62;
    constexpr uint8_t  digitalSourceId     = 63;
    constexpr uint8_t  radiantSourceId     = 64;
    constexpr uint8_t  reedSourceId        = 65;
    constexpr uint8_t  acknowledgeSourceId = 66;
    constexpr uint16_t sourceRevision      = 1;
    constexpr uint16_t calibrationRevision = 1;
    constexpr uint8_t  replayStageCount    = 9;

    const adk::ResistiveProbeConfig liquidConfig = {
        1023, 900, 300, 20, 25, 350, 700, adk::Duration (100), adk::Duration (5), 100};
    const adk::ThermalRadiantConfig environmentConfig = {
        28000, 32000, adk::Duration (100), adk::Duration (10), adk::Duration (30)};
    const adk::MuseumCaseConfig museumConfig = {0x4d555345UL,
                                                63,
                                                liquidSourceId,
                                                sourceRevision,
                                                calibrationRevision,
                                                thermistorSourceId,
                                                sourceRevision,
                                                calibrationRevision,
                                                digitalSourceId,
                                                sourceRevision,
                                                calibrationRevision,
                                                radiantSourceId,
                                                sourceRevision,
                                                calibrationRevision,
                                                adk::Duration (100),
                                                adk::Duration (100),
                                                adk::Duration (100),
                                                adk::Duration (100),
                                                reedSourceId,
                                                sourceRevision,
                                                acknowledgeSourceId,
                                                sourceRevision,
                                                adk::Duration (100),
                                                adk::Duration (100),
                                                adk::Duration (20),
                                                adk::Duration (10),
                                                2};

    adk::ResistiveProbeObservationPolicy liquidPolicy (liquidConfig);

    adk::ThermalRadiantObservationPolicy environmentPolicy (environmentConfig);

    adk::MuseumCaseMonitor museumMonitor (museumConfig);

    volatile MuseumResultCell museumResultCells[replayStageCount];
    volatile ReplayResultCell replayResultCell;

    ReplayStage             replayStage;
    uint32_t                suppliedNow;
    uint32_t                producerSequence;
    adk::MuseumAuditIntent  outstandingAudit;
    adk::MuseumCaseEnvelope workingEnvelope;
    adk::MuseumCaseResult   workingResult;
    bool                    haveOutstandingAudit;

    // clang-format off
    adk::Status acquireCopiedMuseumFixture ();
    void        configureMuseumReplay      ();
    adk::Status startCopiedPolicies        ();

    adk::ResistiveProbeSample copiedLiquidSample (bool wet);

    adk::ThermalRadiantEnvelope copiedEnvironment ();

    adk::MuseumReedEvidence copiedReedEvidence (bool closed);

    adk::MuseumAcknowledgeEvidence copiedAcknowledgement (bool pressed);

    adk::MuseumAuditReceipt emptyAuditReceipt ();

    adk::MuseumAuditReceipt copiedAuditReceipt (bool accepted);

    void observeMuseumFrame ();

    bool decideMuseumIntent (
        ReplayStage stage, const adk::MuseumCaseResult& result);
    void presentMuseumIntent (
        uint8_t index, const adk::MuseumCaseResult& result, bool prediction);
    void finishMuseumReplay ();
    // clang-format on

} // namespace

void setup ()
{
    replayResultCell.liquidInitializeStatus =
        static_cast<uint8_t> (adk::StatusCode::NotInitialized);
    replayResultCell.environmentInitializeStatus =
        static_cast<uint8_t> (adk::StatusCode::NotInitialized);
    replayResultCell.monitorInitializeStatus =
        static_cast<uint8_t> (adk::StatusCode::NotInitialized);

    const adk::Status fixtureStatus = acquireCopiedMuseumFixture ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        finishMuseumReplay ();
        return;
    }

    configureMuseumReplay ();

    const adk::Status initializeStatus = startCopiedPolicies ();
    replayResultCell.monitorInitializeStatus =
        static_cast<uint8_t> (initializeStatus.error ());
    if (!initializeStatus.ok ())
    {
        finishMuseumReplay ();
    }
}

void loop ()
{
    if (replayStage == ReplayStage::Complete)
    {
        return;
    }

    if (replayStage == ReplayStage::Shutdown)
    {
        const adk::Status shutdownStatus = museumMonitor.shutdown ();

        const adk::MuseumCaseIntent inactive = museumMonitor.snapshot ();

        const bool prediction = shutdownStatus.ok () && !museumMonitor.initialized () &&
                                !inactive.alarmSoundIntent &&
                                !inactive.inertRelayLampIntent &&
                                inactive.alarmOutputInactive;

        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
        finishMuseumReplay ();
        return;
    }

    ++producerSequence;
    suppliedNow += replayStage == ReplayStage::CooldownComplete ? 25 : 5;

    observeMuseumFrame ();

    workingResult.intent         = museumMonitor.snapshot ();
    workingResult.hasAuditIntent = false;
    workingResult.auditIntent    = outstandingAudit;
    workingResult.status         = adk::StatusCode::Ok;
    const adk::Status updateStatus =
        museumMonitor.update (workingEnvelope, workingResult);
    workingResult.status = updateStatus;

    const bool prediction = decideMuseumIntent (replayStage, workingResult);

    presentMuseumIntent (static_cast<uint8_t> (replayStage), workingResult, prediction);

    replayResultCell.predictionsPass =
        replayResultCell.predictionsPass && prediction ? 1 : 0;
    ++replayResultCell.completedStages;

    if (workingResult.hasAuditIntent)
    {
        outstandingAudit     = workingResult.auditIntent;
        haveOutstandingAudit = true;
    }

    replayStage = static_cast<ReplayStage> (static_cast<uint8_t> (replayStage) + 1);
}

namespace {

    adk::Status acquireCopiedMuseumFixture ()
    {
        const bool identitiesDistinct =
            liquidSourceId != thermistorSourceId && liquidSourceId != digitalSourceId &&
            liquidSourceId != radiantSourceId && liquidSourceId != reedSourceId &&
            liquidSourceId != acknowledgeSourceId &&
            thermistorSourceId != digitalSourceId &&
            thermistorSourceId != radiantSourceId &&
            thermistorSourceId != reedSourceId &&
            thermistorSourceId != acknowledgeSourceId &&
            digitalSourceId != radiantSourceId && digitalSourceId != reedSourceId &&
            digitalSourceId != acknowledgeSourceId && radiantSourceId != reedSourceId &&
            radiantSourceId != acknowledgeSourceId &&
            reedSourceId != acknowledgeSourceId;

        return identitiesDistinct ? adk::StatusCode::Ok
                                  : adk::StatusCode::InvalidConfiguration;
    }

    void configureMuseumReplay ()
    {
        replayStage                            = ReplayStage::InitialHealthy;
        suppliedNow                            = UINT32_MAX - 30UL;
        producerSequence                       = 0;
        haveOutstandingAudit                   = false;
        replayResultCell.completedStages       = 0;
        replayResultCell.predictionsPass       = 1;
        replayResultCell.terminalAssertionPass = 0;
        replayResultCell.complete              = 0;
    }

    adk::Status startCopiedPolicies ()
    {
        const adk::Status liquidStatus = liquidPolicy.initialize ();
        replayResultCell.liquidInitializeStatus =
            static_cast<uint8_t> (liquidStatus.error ());
        if (!liquidStatus.ok ())
        {
            return liquidStatus;
        }

        const adk::Status environmentStatus = environmentPolicy.initialize ();
        replayResultCell.environmentInitializeStatus =
            static_cast<uint8_t> (environmentStatus.error ());
        if (!environmentStatus.ok ())
        {
            return environmentStatus;
        }

        return museumMonitor.initialize (adk::TimePoint (suppliedNow));
    }

    adk::ResistiveProbeSample copiedLiquidSample (bool wet)
    {
        return {liquidSourceId,
                sourceRevision,
                calibrationRevision,
                producerSequence,
                adk::TimePoint (suppliedNow),
                static_cast<uint16_t> (wet ? 350 : 900),
                3,
                adk::Duration (2),
                adk::Duration (100),
                true,
                adk::StatusCode::Ok};
    }

    adk::ThermalRadiantEnvelope copiedEnvironment ()
    {
        return {{thermistorSourceId, sourceRevision, calibrationRevision,
                 producerSequence, adk::TimePoint (suppliedNow), 23000, 500, false,
                 adk::StatusCode::Ok},
                {digitalSourceId, sourceRevision, calibrationRevision, producerSequence,
                 adk::TimePoint (suppliedNow), 200, adk::ThresholdState::Below, false,
                 adk::StatusCode::Ok},
                {radiantSourceId, sourceRevision, calibrationRevision, producerSequence,
                 adk::TimePoint (suppliedNow), 100, adk::ThresholdState::Below, false,
                 adk::StatusCode::Ok}};
    }

    adk::MuseumReedEvidence copiedReedEvidence (bool closed)
    {
        const bool activation   = closed && replayStage == ReplayStage::InitialHealthy;
        const bool deactivation = !closed;
        const adk::MagneticObservation observation = {
            adk::MagneticSource::ContactDigital,
            static_cast<uint16_t> (closed ? 0 : 1),
            closed ? adk::Level::Low : adk::Level::High,
            adk::TimePoint (suppliedNow),
            adk::MagneticPolarity::Unspecified,
            activation,
            deactivation,
            closed,
            adk::Duration (5),
            adk::MagneticQuality::Valid,
            adk::StatusCode::Ok};

        return {reedSourceId, sourceRevision, producerSequence, observation};
    }

    adk::MuseumAcknowledgeEvidence copiedAcknowledgement (bool pressed)
    {
        return {acknowledgeSourceId,          sourceRevision, producerSequence,
                adk::TimePoint (suppliedNow), pressed,        adk::StatusCode::Ok};
    }

    adk::MuseumAuditReceipt emptyAuditReceipt ()
    {
        return {0,
                0,
                0,
                0,
                adk::TimePoint (),
                adk::MuseumCaseHealth::Qualifying,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                false,
                adk::StatusCode::NotInitialized};
    }

    adk::MuseumAuditReceipt copiedAuditReceipt (bool accepted)
    {
        return {outstandingAudit.ownerToken,
                outstandingAudit.lifecycleGeneration,
                outstandingAudit.configurationRevision,
                outstandingAudit.recordSequence,
                outstandingAudit.observedAt,
                outstandingAudit.health,
                outstandingAudit.hazardMask,
                outstandingAudit.liquidSequence,
                outstandingAudit.thermistorSequence,
                outstandingAudit.digitalTemperatureSequence,
                outstandingAudit.radiantSequence,
                outstandingAudit.reedSequence,
                outstandingAudit.acknowledgeSequence,
                outstandingAudit.witnessDigest,
                outstandingAudit.attempt,
                accepted,
                accepted ? adk::StatusCode::Ok : adk::StatusCode::HardwareFailure};
    }

    void observeMuseumFrame ()
    {
        const bool wet          = replayStage == ReplayStage::WetAlarm ||
                                  replayStage == ReplayStage::RejectAlarmRecord ||
                                  replayStage == ReplayStage::ReissueAlarmRecord ||
                                  replayStage == ReplayStage::AcceptAlarmRecord;
        const bool closed       = replayStage != ReplayStage::CaseOpened;
        const bool acknowledged = replayStage == ReplayStage::RecoverAcknowledged;
        const bool auditReceipt =
            haveOutstandingAudit && (replayStage == ReplayStage::AcceptHealthy ||
                                     replayStage == ReplayStage::RejectAlarmRecord ||
                                     replayStage == ReplayStage::AcceptAlarmRecord ||
                                     replayStage == ReplayStage::CooldownComplete);

        const adk::Status liquidStatus = liquidPolicy.update (
            adk::TimePoint (suppliedNow), copiedLiquidSample (wet));
        const adk::Status environmentStatus = environmentPolicy.update (
            adk::TimePoint (suppliedNow), copiedEnvironment ());

        workingEnvelope.now = adk::TimePoint (suppliedNow);

        workingEnvelope.liquid = liquidPolicy.snapshot ();

        workingEnvelope.environment = environmentPolicy.snapshot ();

        workingEnvelope.reed = copiedReedEvidence (closed);

        workingEnvelope.acknowledge     = copiedAcknowledgement (acknowledged);
        workingEnvelope.hasAuditReceipt = auditReceipt;

        workingEnvelope.auditReceipt = emptyAuditReceipt ();

        if (!liquidStatus.ok ())
        {
            workingEnvelope.liquid.status = liquidStatus;
        }
        if (!environmentStatus.ok ())
        {
            workingEnvelope.environment.status = environmentStatus;
        }
        if (auditReceipt)
        {
            workingEnvelope.auditReceipt =
                copiedAuditReceipt (replayStage != ReplayStage::RejectAlarmRecord);
        }
    }

    bool decideMuseumIntent (ReplayStage stage, const adk::MuseumCaseResult& result)
    {
        const uint8_t liquid    = static_cast<uint8_t> (adk::MuseumHazard::Liquid);
        const uint8_t opening   = static_cast<uint8_t> (adk::MuseumHazard::Opening);
        const uint8_t recording = static_cast<uint8_t> (adk::MuseumHazard::Recording);

        bool stagePrediction = true;
        if (stage == ReplayStage::InitialHealthy ||
            stage == ReplayStage::AcceptHealthy ||
            stage == ReplayStage::CooldownComplete)
        {
            stagePrediction = result.intent.health == adk::MuseumCaseHealth::Healthy;
        }
        else if (stage == ReplayStage::WetAlarm ||
                 stage == ReplayStage::AcceptAlarmRecord)
        {
            stagePrediction = result.intent.health == adk::MuseumCaseHealth::Alarm &&
                              (result.intent.hazardMask & liquid) != 0;
        }
        else if (stage == ReplayStage::RejectAlarmRecord ||
                 stage == ReplayStage::ReissueAlarmRecord)
        {
            stagePrediction = result.intent.health == adk::MuseumCaseHealth::Fault &&
                              (result.intent.hazardMask & liquid) != 0 &&
                              (result.intent.hazardMask & recording) != 0;
        }
        else if (stage == ReplayStage::RecoverAcknowledged)
        {
            stagePrediction = result.intent.health == adk::MuseumCaseHealth::Cooldown;
        }
        else if (stage == ReplayStage::CaseOpened)
        {
            stagePrediction = result.intent.health == adk::MuseumCaseHealth::Alarm &&
                              (result.intent.hazardMask & opening) != 0;
        }

        return result.status.ok () &&
               result.intent.ownerToken == museumConfig.ownerToken &&
               result.intent.configurationRevision ==
                   museumConfig.configurationRevision &&
               result.intent.lcdShowsAgeOrFault &&
               result.intent.alarmSoundIntent ==
                   (result.intent.health == adk::MuseumCaseHealth::Alarm) &&
               result.intent.inertRelayLampIntent ==
                   (result.intent.health == adk::MuseumCaseHealth::Alarm) &&
               result.intent.alarmOutputInactive ==
                   (result.intent.health != adk::MuseumCaseHealth::Alarm) &&
               stagePrediction;
    }

    void presentMuseumIntent (uint8_t index, const adk::MuseumCaseResult& result,
                              bool prediction)
    {
        volatile MuseumResultCell& cell = museumResultCells[index];

        cell.lifecycleGeneration = result.intent.lifecycleGeneration;
        cell.auditRecordSequence =
            result.hasAuditIntent ? result.auditIntent.recordSequence : 0;
        cell.auditWitnessDigest =
            result.hasAuditIntent ? result.auditIntent.witnessDigest : 0;
        cell.health               = static_cast<uint8_t> (result.intent.health);
        cell.hazardMask           = result.intent.hazardMask;
        cell.rgbBlinkCode         = result.intent.rgbBlinkCode;
        cell.lcdShowsAgeOrFault   = result.intent.lcdShowsAgeOrFault ? 1 : 0;
        cell.alarmSoundIntent     = result.intent.alarmSoundIntent ? 1 : 0;
        cell.inertRelayLampIntent = result.intent.inertRelayLampIntent ? 1 : 0;
        cell.alarmOutputInactive  = result.intent.alarmOutputInactive ? 1 : 0;
        cell.auditIntentPresent   = result.hasAuditIntent ? 1 : 0;
        cell.auditAttempt   = result.hasAuditIntent ? result.auditIntent.attempt : 0;
        cell.status         = static_cast<uint8_t> (result.status.error ());
        cell.predictionPass = prediction ? 1 : 0;
    }

    void finishMuseumReplay ()
    {
        const bool terminalAssertion =
            replayResultCell.fixtureStatus ==
                static_cast<uint8_t> (adk::StatusCode::Ok) &&
            replayResultCell.liquidInitializeStatus ==
                static_cast<uint8_t> (adk::StatusCode::Ok) &&
            replayResultCell.environmentInitializeStatus ==
                static_cast<uint8_t> (adk::StatusCode::Ok) &&
            replayResultCell.monitorInitializeStatus ==
                static_cast<uint8_t> (adk::StatusCode::Ok) &&
            replayResultCell.completedStages == replayStageCount &&
            replayResultCell.predictionsPass != 0;

        replayResultCell.terminalAssertionPass = terminalAssertion ? 1 : 0;
        replayResultCell.complete              = 1;
        replayStage                            = ReplayStage::Complete;
    }

} // namespace
