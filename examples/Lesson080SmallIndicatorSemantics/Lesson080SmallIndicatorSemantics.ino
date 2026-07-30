// E0 copied-indicator-semantics replay. This sketch owns no pin, endpoint,
// clock, indicator, driver, supply, observation instrument, or powered
// circuit. Every drive and observation value below is fictional software
// evidence; a passing replay makes no hardware-behavior claim.
#include <Adk.h>
#include <bounded_low_side_driver_policy.h>
#include <small_indicator_semantics_policy.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        ActiveGreen,
        RestoreInactive,
        Cancel,
        Shutdown,
        Complete
    };

    struct SemanticResultCell
    {
        uint32_t observationId;
        uint8_t  disposition;
        uint8_t  reason;
        uint8_t  observationState;
        uint8_t  semanticActiveMask;
        uint8_t  semanticActive;
        uint8_t  safeStateSatisfied;
        uint8_t  autonomousBehaviorObserved;
        uint8_t  producerStatus;
        uint8_t  predictionPass;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t initializeStatus;
        uint8_t sessionStatus;
        uint8_t completedSteps;
        uint8_t predictionsPass;
        uint8_t complete;
    };

    constexpr uint32_t sessionId                   = 80001UL;
    constexpr uint32_t runId                       = 80002UL;
    constexpr uint8_t  sourceId                    = 80;
    constexpr uint16_t sourceConfigurationRevision = 1;
    constexpr uint8_t  trafficLightChannels = adk::SmallIndicatorChannels::Red |
                                              adk::SmallIndicatorChannels::Amber |
                                              adk::SmallIndicatorChannels::Green;

    const adk::LowSideDriverDescriptor copiedDriverDescriptor = {
        1,
        79000UL,
        79001UL,
        1,
        1,
        7900001UL,
        7901UL,
        sourceConfigurationRevision,
        adk::LowSideLoadEnergy::ResistiveIndicator,
        adk::LowSideFlybackRequirement::NotRequired,
        adk::LowSideFlybackDeclaration::Absent,
        true,
        0,
        0,
        0,
        0,
        0,
        0,
        {100000UL, 10000UL, 20000UL, 30000UL, 5000UL, 10UL, 1UL, 1000UL, 100, 4500,
         5000, 900, 500, adk::Duration (100), adk::Duration (1000), 500}};

    const adk::SmallIndicatorDescriptor copiedIndicatorDescriptor = {
        1,
        80000UL,
        80001UL,
        1,
        1,
        8000001UL,
        8001UL,
        1,
        copiedDriverDescriptor.specimenReference,
        copiedDriverDescriptor.specimenRevision,
        copiedDriverDescriptor.electricalEvidenceRevision,
        copiedDriverDescriptor.configurationId,
        copiedDriverDescriptor.configurationRevision,
        adk::lowSideDriverDescriptorIdentityDigest (copiedDriverDescriptor),
        adk::SmallIndicatorKind::TrafficLight,
        adk::SmallIndicatorAutonomy::FollowsDrive,
        adk::SmallIndicatorSafeState::DriveInactive,
        true,
        true,
        true,
        false,
        trafficLightChannels,
        adk::Duration (0),
        adk::Duration (0),
        adk::Duration (50)};

    adk::SmallIndicatorSemanticsPolicy indicatorPolicy (copiedIndicatorDescriptor);
    adk::LowSideDriveIntent            copiedDrive;
    adk::SmallIndicatorSemanticRequest copiedRequest;
    adk::SmallIndicatorObservation     copiedObservation;
    adk::SmallIndicatorSemanticResult  semanticResult;

    volatile SemanticResultCell activeCell;
    volatile SemanticResultCell safeStateCell;
    volatile SemanticResultCell cancelCell;
    volatile ReplayResultCell   replayResultCell;

    ReplayStage replayStage;
    uint32_t    suppliedNow;
    uint32_t    nextSequence;

    adk::Status acquireCopiedDescriptors ();

    void        configureCopiedReplay ();

    adk::Status startPolicySession ();

    void observeCopiedEvidence ();

    bool decideExpectedSemantics (const adk::SmallIndicatorSemanticResult& result,
                                  adk::Status                              status);
    void actuateSemanticCell (volatile SemanticResultCell&             cell,
                              const adk::SmallIndicatorSemanticResult& result,
                              adk::Status status, bool prediction);

    void cancelCopiedReplay ();

    void finishReplay (adk::Status status);

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedDescriptors ();

    configureCopiedReplay ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        finishReplay (fixtureStatus);
        return;
    }

    const adk::Status sessionStatus = startPolicySession ();

    replayResultCell.sessionStatus  = static_cast<uint8_t> (sessionStatus.error ());

    if (!sessionStatus.ok ())
    {
        finishReplay (sessionStatus);
    }
}

void loop ()
{
    if (replayStage == ReplayStage::Complete)
    {
        return;
    }

    if (replayStage == ReplayStage::Cancel)
    {
        cancelCopiedReplay ();
        return;
    }

    if (replayStage == ReplayStage::Shutdown)
    {
        indicatorPolicy.shutdown ();

        semanticResult = indicatorPolicy.snapshot ();

        const bool prediction =
            semanticResult.disposition == adk::SmallIndicatorDisposition::Shutdown &&
            !semanticResult.semanticActive && semanticResult.semanticActiveMask == 0;
        actuateSemanticCell (cancelCell, semanticResult, adk::StatusCode::Ok,
                             prediction);
        ++replayResultCell.completedSteps;
        finishReplay (prediction ? adk::StatusCode::Ok
                                 : adk::StatusCode::HardwareFailure);
        return;
    }

    observeCopiedEvidence ();

    const adk::Status status = indicatorPolicy.apply (
        copiedDrive, copiedRequest, copiedObservation,
        adk::TimePoint (suppliedNow), semanticResult);
    const bool prediction = decideExpectedSemantics (semanticResult, status);

    volatile SemanticResultCell& cell =
        replayStage == ReplayStage::ActiveGreen ? activeCell : safeStateCell;
    actuateSemanticCell (cell, semanticResult, status, prediction);

    if (!status.ok () || !prediction)
    {
        finishReplay (status.ok () ? adk::StatusCode::HardwareFailure : status);
        return;
    }

    ++replayResultCell.completedSteps;
    replayStage = static_cast<ReplayStage> (static_cast<uint8_t> (replayStage) + 1U);
}

namespace {

    adk::Status acquireCopiedDescriptors ()
    {
        const adk::Status driverStatus =
            adk::validateLowSideDriverDescriptor (copiedDriverDescriptor);
        if (!driverStatus.ok ())
        {
            return driverStatus;
        }
        return adk::validateSmallIndicatorDescriptor (copiedIndicatorDescriptor);
    }

    void configureCopiedReplay ()
    {
        replayStage       = ReplayStage::Complete;
        suppliedNow       = 100;
        nextSequence      = 1;
        copiedDrive       = {};
        copiedRequest     = {};
        copiedObservation = {};
        semanticResult    = {};

        replayResultCell.predictionsPass = 1;
    }

    adk::Status startPolicySession ()
    {
        const adk::Status initializeStatus = indicatorPolicy.initialize ();
        replayResultCell.initializeStatus =
            static_cast<uint8_t> (initializeStatus.error ());
        if (!initializeStatus.ok ())
        {
            return initializeStatus;
        }

        const adk::Status sessionStatus = indicatorPolicy.beginSession (
            sessionId, runId, adk::TimePoint (suppliedNow));
        if (sessionStatus.ok ())
        {
            semanticResult = indicatorPolicy.snapshot ();
            replayStage    = ReplayStage::ActiveGreen;
        }
        return sessionStatus;
    }

    void observeCopiedEvidence ()
    {
        const bool     active = replayStage == ReplayStage::ActiveGreen;
        const uint8_t  selectedMask =
            active ? adk::SmallIndicatorChannels::Green : 0;
        const uint8_t  observedMask =
            active ? adk::SmallIndicatorChannels::Green : 0;
        const uint32_t requestId     = 800100UL + nextSequence;
        const uint32_t observationId = 800200UL + nextSequence;

        suppliedNow += 10;
        copiedDrive = {semanticResult.lifecycleGeneration,
                       copiedIndicatorDescriptor.expectedDriverDescriptorIdentityDigest,
                       copiedIndicatorDescriptor.driverSpecimenReference,
                       copiedIndicatorDescriptor.driverSpecimenRevision,
                       copiedIndicatorDescriptor.driverElectricalEvidenceRevision,
                       copiedIndicatorDescriptor.driverPolicyConfigurationId,
                       copiedIndicatorDescriptor.driverPolicyConfigurationRevision,
                       sessionId,
                       runId,
                       static_cast<uint16_t> (nextSequence),
                       requestId,
                       active ? adk::LowSideDriveState::Requested
                              : adk::LowSideDriveState::Off,
                       adk::LowSideDriveReason::None,
                       active,
                       active,
                       active ? 1000UL : 0,
                       active ? 1000UL : 0,
                       active ? 10000UL : 0,
                       adk::TimePoint (active ? suppliedNow + 25 : suppliedNow),
                       adk::StatusCode::Ok};

        copiedRequest = {semanticResult.lifecycleGeneration,
                         sessionId,
                         runId,
                         static_cast<uint16_t> (nextSequence),
                         requestId,
                         sourceId,
                         sourceConfigurationRevision,
                         nextSequence,
                         nextSequence,
                         adk::TimePoint (suppliedNow),
                         selectedMask,
                         adk::StatusCode::Ok};

        copiedObservation = {semanticResult.lifecycleGeneration,
                             sessionId,
                             runId,
                             static_cast<uint16_t> (nextSequence),
                             requestId,
                             observationId,
                             static_cast<uint8_t> (sourceId + 1U),
                             sourceConfigurationRevision,
                             nextSequence,
                             adk::TimePoint (suppliedNow),
                             active ? adk::SmallIndicatorObservationState::Active
                                    : adk::SmallIndicatorObservationState::Inactive,
                             observedMask,
                             false,
                             false,
                             !active,
                             adk::StatusCode::Ok};
        ++nextSequence;
    }

    bool decideExpectedSemantics (const adk::SmallIndicatorSemanticResult& result,
                                  adk::Status                              status)
    {
        if (!status.ok ())
        {
            return false;
        }

        if (replayStage == ReplayStage::ActiveGreen)
        {
            return result.disposition == adk::SmallIndicatorDisposition::Accepted &&
                   result.reason == adk::SmallIndicatorReason::None &&
                   result.semanticActive &&
                   result.semanticActiveMask == adk::SmallIndicatorChannels::Green &&
                   !result.safeStateSatisfied;
        }

        return result.disposition == adk::SmallIndicatorDisposition::Accepted &&
               result.reason == adk::SmallIndicatorReason::None &&
               !result.semanticActive && result.semanticActiveMask == 0 &&
               result.safeStateSatisfied;
    }

    void actuateSemanticCell (volatile SemanticResultCell&             cell,
                              const adk::SmallIndicatorSemanticResult& result,
                              adk::Status status, bool prediction)
    {
        cell.observationId      = result.observationId;
        cell.disposition        = static_cast<uint8_t> (result.disposition);
        cell.reason             = static_cast<uint8_t> (result.reason);
        cell.observationState   = static_cast<uint8_t> (result.observationState);
        cell.semanticActiveMask = result.semanticActiveMask;
        cell.semanticActive     = result.semanticActive ? 1 : 0;
        cell.safeStateSatisfied = result.safeStateSatisfied ? 1 : 0;
        cell.autonomousBehaviorObserved = result.autonomousBehaviorObserved ? 1 : 0;
        cell.producerStatus = static_cast<uint8_t> (result.producerStatus.error ());
        cell.predictionPass = prediction ? 1 : 0;

        if (!status.ok () || !prediction)
        {
            replayResultCell.predictionsPass = 0;
        }
    }

    void cancelCopiedReplay ()
    {
        const adk::SmallIndicatorControl control = {
            semanticResult.lifecycleGeneration,
            sessionId,
            runId,
            static_cast<uint16_t> (nextSequence),
            800200UL + nextSequence,
            sourceId,
            sourceConfigurationRevision,
            nextSequence++,
            adk::TimePoint (++suppliedNow),
            true,
            adk::StatusCode::Ok};

        const adk::Status status = indicatorPolicy.cancel (control, semanticResult);
        const bool        prediction =
            status.ok () &&
            semanticResult.disposition == adk::SmallIndicatorDisposition::Cancelled &&
            semanticResult.reason == adk::SmallIndicatorReason::Cancelled &&
            !semanticResult.semanticActive && semanticResult.semanticActiveMask == 0 &&
            semanticResult.safeStateSatisfied;
        actuateSemanticCell (cancelCell, semanticResult, status, prediction);

        if (!status.ok () || !prediction)
        {
            finishReplay (status.ok () ? adk::StatusCode::HardwareFailure : status);
            return;
        }

        ++replayResultCell.completedSteps;
        replayStage = ReplayStage::Shutdown;
    }

    void finishReplay (adk::Status status)
    {
        if (!status.ok ())
        {
            replayResultCell.predictionsPass = 0;
        }
        replayResultCell.complete = 1;
        replayStage               = ReplayStage::Complete;
    }

} // namespace
