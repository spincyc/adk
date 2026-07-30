// E0 copied low-side-policy replay. This sketch owns no pin, endpoint, clock,
// transistor, diode, load, supply, or powered circuit. The fictional specimen
// and returned drive levels are software evidence and inert intent only.
#include <Adk.h>
#include <bounded_low_side_driver_policy.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        BoundedRequest,
        RestoreOff,
        OverBudgetRequest,
        Shutdown,
        Complete
    };

    struct SemanticResultCell
    {
        uint32_t descriptorDigest;
        uint32_t sessionId;
        uint32_t runId;
        uint32_t requestId;
        uint32_t admittedLoadUa;
        uint32_t requiredBaseUa;
        uint32_t expiresAt;
        uint8_t  state;
        uint8_t  reason;
        uint8_t  logicalActive;
        uint8_t  outputLevelHigh;
        uint8_t  status;
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

    constexpr uint32_t sessionId                   = 79001UL;
    constexpr uint32_t runId                       = 79002UL;
    constexpr uint8_t  sourceId                    = 79;
    constexpr uint16_t sourceConfigurationRevision = 1;

    const adk::LowSideDriverDescriptor copiedDescriptor = {
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

    adk::BoundedLowSideDriverPolicy driverPolicy (copiedDescriptor);
    adk::LowSideDriveIntent         observedIntent;

    volatile SemanticResultCell sourceCell;
    volatile SemanticResultCell driveCell;
    volatile SemanticResultCell observationCell;
    volatile SemanticResultCell safeStateCell;
    volatile ReplayResultCell   replayResultCell;

    ReplayStage replayStage;
    uint32_t    suppliedNow;
    uint32_t    nextSequence;

    // clang-format off
    adk::Status acquireCopiedDescriptor ();

    void        configureCopiedReplay ();

    adk::Status startPolicySession ();

    adk::Status observeCopiedEvidence ();

    bool        decideExpectedIntent (const adk::LowSideDriveIntent& intent,
                                      adk::Status                    status);

    void actuateSemanticCell (volatile SemanticResultCell&   cell,
                              const adk::LowSideDriveIntent& intent, adk::Status status,
                              bool prediction);

    void finishReplay (adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedDescriptor ();

    configureCopiedReplay ();

    replayResultCell.fixtureStatus  = static_cast<uint8_t> (fixtureStatus.error ());

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

    if (replayStage == ReplayStage::Shutdown)
    {
        driverPolicy.shutdown ();

        observedIntent = driverPolicy.snapshot ();
        ++replayResultCell.completedSteps;
        finishReplay (adk::StatusCode::Ok);
        return;
    }

    const adk::Status status     = observeCopiedEvidence ();

    const bool        prediction = decideExpectedIntent (observedIntent, status);

    volatile SemanticResultCell* resultCell = &driveCell;
    if (replayStage == ReplayStage::RestoreOff)
    {
        resultCell = &safeStateCell;
    }
    else if (replayStage == ReplayStage::OverBudgetRequest)
    {
        resultCell = &observationCell;
    }

    actuateSemanticCell (*resultCell, observedIntent, status, prediction);

    if (!status.ok () || !prediction)
    {
        finishReplay (status);
        return;
    }

    ++replayResultCell.completedSteps;
    replayStage = static_cast<ReplayStage> (static_cast<uint8_t> (replayStage) + 1U);
}

namespace {

    adk::Status acquireCopiedDescriptor ()
    {
        return adk::validateLowSideDriverDescriptor (copiedDescriptor);
    }

    void configureCopiedReplay ()
    {
        replayStage    = ReplayStage::Complete;
        suppliedNow    = 100;
        nextSequence   = 1;
        observedIntent = {};

        replayResultCell.predictionsPass = 1;
    }

    adk::Status startPolicySession ()
    {
        const adk::Status initializeStatus = driverPolicy.initialize ();
        replayResultCell.initializeStatus =
            static_cast<uint8_t> (initializeStatus.error ());
        if (!initializeStatus.ok ())
        {
            return initializeStatus;
        }

        observedIntent = driverPolicy.snapshot ();
        const bool offPrediction =
            observedIntent.state == adk::LowSideDriveState::Off &&
            !observedIntent.logicalActive && !observedIntent.outputLevelHigh;
        actuateSemanticCell (sourceCell, observedIntent, initializeStatus,
                             offPrediction);
        if (!offPrediction)
        {
            return adk::StatusCode::HardwareFailure;
        }

        const adk::Status sessionStatus = driverPolicy.beginSession (sessionId, runId);

        if (sessionStatus.ok ())
        {
            observedIntent = driverPolicy.snapshot ();
            replayStage    = ReplayStage::BoundedRequest;
        }
        return sessionStatus;
    }

    adk::Status observeCopiedEvidence ()
    {
        if (replayStage == ReplayStage::BoundedRequest ||
            replayStage == ReplayStage::OverBudgetRequest)
        {
            const bool bounded = replayStage == ReplayStage::BoundedRequest;
            const adk::LowSideDriveRequest request = {
                sessionId,
                runId,
                static_cast<uint16_t> (nextSequence),
                790100UL + nextSequence,
                sourceId,
                sourceConfigurationRevision,
                nextSequence++,
                adk::TimePoint (suppliedNow += 10),
                observedIntent.lifecycleGeneration,
                true,
                bounded ? 10000UL : 20001UL,
                adk::Duration (50),
                adk::StatusCode::Ok};
            return driverPolicy.apply (request, observedIntent);
        }

        if (replayStage == ReplayStage::RestoreOff)
        {
            const adk::LowSideDriveRequest request = {
                sessionId,
                runId,
                static_cast<uint16_t> (nextSequence),
                790100UL + nextSequence,
                sourceId,
                sourceConfigurationRevision,
                nextSequence++,
                adk::TimePoint (suppliedNow += 10),
                observedIntent.lifecycleGeneration,
                false,
                0,
                adk::Duration (0),
                adk::StatusCode::Ok};
            return driverPolicy.apply (request, observedIntent);
        }

        driverPolicy.shutdown ();

        observedIntent = driverPolicy.snapshot ();
        return adk::StatusCode::Ok;
    }

    bool decideExpectedIntent (const adk::LowSideDriveIntent& intent,
                               adk::Status                    status)
    {
        if (!status.ok ())
        {
            return false;
        }
        if (replayStage == ReplayStage::BoundedRequest)
        {
            return intent.state == adk::LowSideDriveState::Requested &&
                   intent.reason == adk::LowSideDriveReason::None &&
                   intent.logicalActive && intent.outputLevelHigh &&
                   intent.admittedLoadUa == 10000UL;
        }
        if (replayStage == ReplayStage::RestoreOff)
        {
            return intent.state == adk::LowSideDriveState::Off &&
                   !intent.logicalActive && !intent.outputLevelHigh;
        }
        if (replayStage == ReplayStage::OverBudgetRequest)
        {
            return intent.state == adk::LowSideDriveState::Rejected &&
                   intent.reason == adk::LowSideDriveReason::BudgetExceeded &&
                   !intent.logicalActive && !intent.outputLevelHigh;
        }
        return intent.state == adk::LowSideDriveState::Shutdown &&
               !intent.logicalActive && !intent.outputLevelHigh;
    }

    void actuateSemanticCell (volatile SemanticResultCell&   cell,
                              const adk::LowSideDriveIntent& intent, adk::Status status,
                              bool prediction)
    {
        cell.descriptorDigest = intent.driverDescriptorIdentityDigest;
        cell.sessionId        = intent.sessionId;
        cell.runId            = intent.runId;
        cell.requestId        = intent.requestId;
        cell.admittedLoadUa   = intent.admittedLoadUa;
        cell.requiredBaseUa   = intent.requiredBaseUa;
        cell.expiresAt        = intent.expiresAt.milliseconds ();
        cell.state            = static_cast<uint8_t> (intent.state);
        cell.reason           = static_cast<uint8_t> (intent.reason);
        cell.logicalActive    = intent.logicalActive ? 1 : 0;
        cell.outputLevelHigh  = intent.outputLevelHigh ? 1 : 0;
        cell.status           = static_cast<uint8_t> (status.error ());
        cell.predictionPass   = prediction ? 1 : 0;

        replayResultCell.predictionsPass =
            static_cast<uint8_t> (replayResultCell.predictionsPass && prediction);
    }

    void finishReplay (adk::Status status)
    {
        driverPolicy.shutdown ();

        observedIntent = driverPolicy.snapshot ();

        const bool safe = observedIntent.state == adk::LowSideDriveState::Shutdown &&
                          !observedIntent.logicalActive &&
                          !observedIntent.outputLevelHigh;
        actuateSemanticCell (safeStateCell, observedIntent, status, safe);

        replayResultCell.complete = 1;
        replayStage               = ReplayStage::Complete;
    }

} // namespace
