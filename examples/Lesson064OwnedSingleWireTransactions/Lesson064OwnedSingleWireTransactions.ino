// E0 single-wire transaction fixture. This sketch replays copied intent and
// receipt values at supplied microsecond times. It owns no pin, timer,
// interrupt, pull-up, probe, bus, power path, or powered circuit.
#include <Adk.h>
#include <one_wire_transaction_policy.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        ConfirmInitialRelease,
        RejectParasitePower,
        BeginPresenceTransaction,
        ApplyPresenceStep,
        ConfirmPresenceRelease,
        BeginTimeoutTransaction,
        TriggerTimeout,
        ConfirmRollbackRelease,
        BeginShutdown,
        ConfirmShutdown,
        Complete
    };

    struct TransactionResultCell
    {
        uint32_t requestSequence;
        uint32_t transactionGeneration;
        uint16_t slotIndex;
        uint8_t  stage;
        uint8_t  operation;
        uint8_t  phase;
        uint8_t  lineIntent;
        uint8_t  quality;
        uint8_t  status;
        uint8_t  sampleRequired;
        uint8_t  releaseRequested;
        uint8_t  releaseConfirmed;
        uint8_t  initialized;
        uint8_t  receiptProvenancePass;
        uint8_t  predictionPass;
    };

    struct OwnershipResultCell
    {
        uint32_t ownerToken;
        uint32_t lifecycleGeneration;
        uint16_t configurationRevision;
        uint8_t  predictionPass;
    };

    constexpr uint32_t fixtureOwnerToken = UINT32_C (640064);
    constexpr uint16_t fixtureConfigurationRevision = 64;
    constexpr uint8_t  fixtureReceiptSourceId = 7;
    constexpr uint16_t fixtureReceiptConfigurationRevision = 64;

    adk::OneWireTransactionPolicy fixturePolicy ({
        fixtureOwnerToken,
        fixtureConfigurationRevision,
        fixtureReceiptSourceId,
        fixtureReceiptConfigurationRevision,
        true,
        adk::MicrosecondDuration (480),
        adk::MicrosecondDuration (960),
        adk::MicrosecondDuration (15),
        adk::MicrosecondDuration (60),
        adk::MicrosecondDuration (15),
        adk::MicrosecondDuration (60),
        adk::MicrosecondDuration (60),
        adk::MicrosecondDuration (240),
        adk::MicrosecondDuration (60),
        adk::MicrosecondDuration (120),
        adk::MicrosecondDuration (1),
        adk::MicrosecondDuration (15),
        adk::MicrosecondDuration (1),
        adk::MicrosecondDuration (15),
        adk::MicrosecondDuration (15),
        adk::MicrosecondDuration (45),
        adk::MicrosecondDuration (60),
        adk::MicrosecondDuration (120),
        adk::MicrosecondDuration (1),
        adk::MicrosecondDuration (20),
        adk::MicrosecondDuration (20000),
        64});
    adk::OneWireOperationRequest  fixtureRequest = {
        0,
        adk::OneWireOperation::ResetPresence,
        {{0, 0, 0, 0, 0, 0, 0, 0}},
        {{{0, 0, 0, 0, 0, 0, 0, 0}}, 0, false},
        adk::MicrosecondTimePoint (),
        adk::OneWireSupplyMode::ExternallyPowered,
        adk::StatusCode::Ok};
    adk::OneWireSearchState fixtureSearchState = {{{0, 0, 0, 0, 0, 0, 0, 0}}, 0, false};
    adk::OneWireStepIntent  fixtureIntent      = {0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  adk::OneWireOperation::ResetPresence,
                                                  adk::OneWirePhase::Inert,
                                                  0,
                                                  0,
                                                  false,
                                                  adk::OneWireLineIntent::Release,
                                                  false,
                                                  adk::MicrosecondTimePoint (),
                                                  adk::MicrosecondTimePoint (),
                                                  {{0, 0, 0, 0, 0, 0, 0, 0}}};
    adk::OneWireStepReceipt fixtureReceipt     = {0,
                                                  0,
                                                  0,
                                                  adk::MicrosecondTimePoint (),
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  adk::OneWireOperation::ResetPresence,
                                                  adk::OneWirePhase::Inert,
                                                  0,
                                                  0,
                                                  adk::OneWireLineIntent::Release,
                                                  true,
                                                  false,
                                                  adk::StatusCode::NotInitialized};
    adk::OneWireTransactionSnapshot fixtureSnapshot = {
        adk::OneWireOperation::ResetPresence,
        adk::OneWirePhase::Inert,
        adk::OneWireTransactionQuality::Unqualified,
        {0,
         adk::OneWireOperation::ResetPresence,
         {{0, 0, 0, 0, 0, 0, 0, 0}},
         {{{0, 0, 0, 0, 0, 0, 0, 0}}, 0, false},
         adk::MicrosecondTimePoint (),
         adk::OneWireSupplyMode::ExternallyPowered,
         adk::StatusCode::Ok},
        {{{0, 0, 0, 0, 0, 0, 0, 0}}, 0, false},
        {{0, 0, 0, 0, 0, 0, 0, 0}},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        0,
        0,
        false,
        false,
        false,
        adk::MicrosecondTimePoint (),
        adk::StatusCode::NotInitialized,
        0,
        0,
        0,
        0};

    constexpr uint8_t resultCellCount = 10;

    volatile TransactionResultCell resultCells[resultCellCount];
    volatile OwnershipResultCell   ownershipResultCell;
    volatile uint8_t               fixtureStatusCell;
    volatile uint8_t               initializeStatusCell;
    volatile uint8_t               completedResultCellsCell;
    volatile uint8_t               allPredictionsPassCell;
    volatile uint8_t               replayActiveCell;

    ReplayStage replayStage;
    uint32_t    receiptSequence;
    uint8_t     resultIndex;
    bool        receiptProvenancePass;
    bool        replayActive;

    // clang-format off
    adk::Status acquireCopiedTransactionFixture  ();
    void        configureTransactionReplay       ();
    adk::Status startTransactionReplay           ();
    void        loadRequest                      (
        uint32_t sequence, adk::OneWireSupplyMode supplyMode,
        adk::MicrosecondTimePoint startedAt);
    void        copyReceiptForIntent             (
        bool sampledHigh, adk::Status producerStatus);
    adk::Status decideTransactionStep            ();
    void        presentTransactionIntent         (
        ReplayStage completedStage, adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedTransactionFixture ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    configureTransactionReplay ();

    const adk::Status initializeStatus = startTransactionReplay ();

    initializeStatusCell = static_cast<uint8_t> (initializeStatus.error ());
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayStage completedStage  = replayStage;
    const adk::Status operationStatus = decideTransactionStep ();

    presentTransactionIntent (completedStage, operationStatus);
}

namespace {

    adk::Status acquireCopiedTransactionFixture ()
    {
        return fixtureOwnerToken != 0 && fixtureConfigurationRevision != 0 &&
                       fixtureReceiptSourceId != 0
                   ? adk::StatusCode::Ok
                   : adk::StatusCode::InternalInvariant;
    }

    void configureTransactionReplay ()
    {
        fixtureSearchState.rom = {{0x28, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}};
        fixtureSearchState.lastDiscrepancy = 0;
        fixtureSearchState.lastDevice      = false;

        receiptSequence          = 0;
        resultIndex              = 0;
        completedResultCellsCell = 0;
        allPredictionsPassCell   = 1;
        replayActiveCell         = 0;
        replayActive             = false;
        receiptProvenancePass    = true;
        replayStage              = ReplayStage::ConfirmInitialRelease;
    }

    adk::Status startTransactionReplay ()
    {
        const adk::Status status =
            fixturePolicy.initialize (adk::MicrosecondTimePoint (100), fixtureIntent);

        if (status.ok ())
        {
            replayActive     = true;
            replayActiveCell = 1;
        }

        return status;
    }

    void loadRequest (uint32_t sequence, adk::OneWireSupplyMode supplyMode,
                      adk::MicrosecondTimePoint startedAt)
    {
        fixtureRequest.requestSequence = sequence;
        fixtureRequest.operation       = adk::OneWireOperation::ResetPresence;
        fixtureRequest.addressedRom    = fixtureSearchState.rom;
        fixtureRequest.search          = fixtureSearchState;
        fixtureRequest.startedAt       = startedAt;
        fixtureRequest.supplyMode      = supplyMode;
        fixtureRequest.status          = adk::StatusCode::Ok;
    }

    void copyReceiptForIntent (bool sampledHigh, adk::Status producerStatus)
    {
        ++receiptSequence;
        fixtureReceipt.sourceId              = fixtureReceiptSourceId;
        fixtureReceipt.configurationRevision = fixtureReceiptConfigurationRevision;
        fixtureReceipt.sequence              = receiptSequence;
        fixtureReceipt.observedAt            = fixtureIntent.earliestAt;
        fixtureReceipt.ownerToken            = fixtureIntent.ownerToken;
        fixtureReceipt.lifecycleGeneration   = fixtureIntent.lifecycleGeneration;
        fixtureReceipt.requestSequence       = fixtureIntent.requestSequence;
        fixtureReceipt.transactionGeneration = fixtureIntent.transactionGeneration;
        fixtureReceipt.operation             = fixtureIntent.operation;
        fixtureReceipt.phase                 = fixtureIntent.phase;
        fixtureReceipt.phaseSequence         = fixtureIntent.phaseSequence;
        fixtureReceipt.slotIndex             = fixtureIntent.slotIndex;
        fixtureReceipt.appliedIntent         = fixtureIntent.lineIntent;
        fixtureReceipt.sampledHigh           = sampledHigh;
        fixtureReceipt.accepted              = producerStatus.ok ();
        fixtureReceipt.status                = producerStatus;
        receiptProvenancePass =
            fixtureReceipt.sourceId == fixtureReceiptSourceId &&
            fixtureReceipt.configurationRevision ==
                fixtureReceiptConfigurationRevision &&
            fixtureReceipt.ownerToken == fixtureIntent.ownerToken &&
            fixtureReceipt.lifecycleGeneration == fixtureIntent.lifecycleGeneration &&
            fixtureReceipt.requestSequence == fixtureIntent.requestSequence &&
            fixtureReceipt.transactionGeneration ==
                fixtureIntent.transactionGeneration &&
            fixtureReceipt.operation == fixtureIntent.operation &&
            fixtureReceipt.phase == fixtureIntent.phase &&
            fixtureReceipt.phaseSequence == fixtureIntent.phaseSequence &&
            fixtureReceipt.slotIndex == fixtureIntent.slotIndex &&
            fixtureReceipt.appliedIntent == fixtureIntent.lineIntent;
    }

    adk::Status decideTransactionStep ()
    {
        switch (replayStage)
        {
            case ReplayStage::ConfirmInitialRelease:
                copyReceiptForIntent (true, adk::StatusCode::Ok);
                replayStage = ReplayStage::RejectParasitePower;
                return fixturePolicy.confirmCleanup (fixtureReceipt.observedAt,
                                                     fixtureReceipt);

            case ReplayStage::RejectParasitePower:
                loadRequest (1, adk::OneWireSupplyMode::ParasitePower,
                             adk::MicrosecondTimePoint (1000));
                replayStage = ReplayStage::BeginPresenceTransaction;
                return fixturePolicy.begin (fixtureRequest.startedAt, fixtureRequest,
                                            fixtureIntent);

            case ReplayStage::BeginPresenceTransaction:
                loadRequest (2, adk::OneWireSupplyMode::ExternallyPowered,
                             adk::MicrosecondTimePoint (2000));
                replayStage = ReplayStage::ApplyPresenceStep;
                return fixturePolicy.begin (fixtureRequest.startedAt, fixtureRequest,
                                            fixtureIntent);

            case ReplayStage::ApplyPresenceStep:
            {
                const bool sampledHigh =
                    fixtureIntent.phase == adk::OneWirePhase::PresenceWindow &&
                            fixtureIntent.sampleRequired
                        ? fixtureSnapshot.presenceSeen
                        : true;
                copyReceiptForIntent (sampledHigh, adk::StatusCode::Ok);

                const adk::Status status = fixturePolicy.update (
                    fixtureReceipt.observedAt, fixtureReceipt, fixtureIntent);
                fixturePolicy.snapshot (fixtureSnapshot);
                if (fixtureSnapshot.quality ==
                        adk::OneWireTransactionQuality::ReleaseUnconfirmed &&
                    fixtureSnapshot.releaseRequested)
                {
                    replayStage = ReplayStage::ConfirmPresenceRelease;
                }
                return status;
            }

            case ReplayStage::ConfirmPresenceRelease:
                copyReceiptForIntent (true, adk::StatusCode::Ok);
                replayStage = ReplayStage::BeginTimeoutTransaction;
                return fixturePolicy.confirmCleanup (fixtureReceipt.observedAt,
                                                     fixtureReceipt);

            case ReplayStage::BeginTimeoutTransaction:
                loadRequest (3, adk::OneWireSupplyMode::ExternallyPowered,
                             adk::MicrosecondTimePoint (30000));
                replayStage = ReplayStage::TriggerTimeout;
                return fixturePolicy.begin (fixtureRequest.startedAt, fixtureRequest,
                                            fixtureIntent);

            case ReplayStage::TriggerTimeout:
                replayStage = ReplayStage::ConfirmRollbackRelease;
                return fixturePolicy.advance (adk::MicrosecondTimePoint (50001),
                                              fixtureIntent);

            case ReplayStage::ConfirmRollbackRelease:
                copyReceiptForIntent (true, adk::StatusCode::Ok);
                replayStage = ReplayStage::BeginShutdown;
                return fixturePolicy.confirmCleanup (fixtureReceipt.observedAt,
                                                     fixtureReceipt);

            case ReplayStage::BeginShutdown:
                replayStage = ReplayStage::ConfirmShutdown;
                return fixturePolicy.shutdown (adk::MicrosecondTimePoint (60000),
                                               fixtureIntent);

            case ReplayStage::ConfirmShutdown:
                copyReceiptForIntent (true, adk::StatusCode::Ok);
                replayStage = ReplayStage::Complete;
                return fixturePolicy.confirmCleanup (fixtureReceipt.observedAt,
                                                     fixtureReceipt);

            case ReplayStage::Complete: return adk::StatusCode::Ok;
        }

        return adk::StatusCode::InternalInvariant;
    }

    void presentTransactionIntent (ReplayStage completedStage,
                                   adk::Status operationStatus)
    {
        fixturePolicy.snapshot (fixtureSnapshot);

        const uint8_t cellIndex = static_cast<uint8_t> (completedStage);
        if (cellIndex < resultCellCount)
        {
            volatile TransactionResultCell& cell = resultCells[cellIndex];

            cell.requestSequence       = fixtureIntent.requestSequence;
            cell.transactionGeneration = fixtureIntent.transactionGeneration;
            cell.slotIndex             = fixtureIntent.slotIndex;
            cell.stage                 = static_cast<uint8_t> (completedStage);
            cell.operation  = static_cast<uint8_t> (fixtureSnapshot.operation);
            cell.phase      = static_cast<uint8_t> (fixtureSnapshot.phase);
            cell.lineIntent = static_cast<uint8_t> (fixtureIntent.lineIntent);
            cell.quality    = static_cast<uint8_t> (fixtureSnapshot.quality);
            cell.status     = static_cast<uint8_t> (operationStatus.error ());
            cell.sampleRequired =
                fixtureIntent.sampleRequired ? UINT8_C (1) : UINT8_C (0);
            cell.releaseRequested =
                fixtureSnapshot.releaseRequested ? UINT8_C (1) : UINT8_C (0);
            cell.releaseConfirmed =
                fixtureSnapshot.releaseConfirmed ? UINT8_C (1) : UINT8_C (0);
            cell.initialized = fixturePolicy.initialized () ? UINT8_C (1) : UINT8_C (0);
            cell.receiptProvenancePass =
                receiptProvenancePass ? UINT8_C (1) : UINT8_C (0);

            bool predictionPass = fixtureIntent.ownerToken == 640064 &&
                                  fixtureIntent.configurationRevision == 64;
            if (completedStage == ReplayStage::RejectParasitePower)
            {
                predictionPass =
                    operationStatus.error () == adk::StatusCode::Unsupported;
            }
            else if (completedStage == ReplayStage::TriggerTimeout)
            {
                predictionPass =
                    operationStatus.error () == adk::StatusCode::Timeout &&
                    fixtureIntent.lineIntent == adk::OneWireLineIntent::Release &&
                    fixtureSnapshot.quality ==
                        adk::OneWireTransactionQuality::ReleaseUnconfirmed;
            }
            else if (completedStage == ReplayStage::ConfirmPresenceRelease)
            {
                predictionPass =
                    operationStatus.ok () &&
                    fixtureSnapshot.phase == adk::OneWirePhase::Complete &&
                    fixtureSnapshot.quality == adk::OneWireTransactionQuality::Complete &&
                    fixtureSnapshot.releaseRequested &&
                    fixtureSnapshot.releaseConfirmed;
            }
            else if (completedStage == ReplayStage::ConfirmShutdown)
            {
                predictionPass = operationStatus.ok () &&
                                 !fixturePolicy.initialized () &&
                                 fixtureSnapshot.phase == adk::OneWirePhase::Inert;
            }
            else
            {
                predictionPass = operationStatus.ok ();
            }

            cell.predictionPass = predictionPass ? UINT8_C (1) : UINT8_C (0);

            ownershipResultCell.ownerToken          = fixtureSnapshot.ownerToken;
            ownershipResultCell.lifecycleGeneration =
                fixtureSnapshot.lifecycleGeneration;
            ownershipResultCell.configurationRevision =
                fixtureSnapshot.configurationRevision;
            const bool ownershipPrediction =
                completedStage == ReplayStage::ConfirmShutdown
                    ? fixtureSnapshot.ownerToken == 0 &&
                          fixtureSnapshot.lifecycleGeneration == 0 &&
                          fixtureSnapshot.configurationRevision == 0 &&
                          fixtureSnapshot.transactionGeneration == 0
                    : fixtureSnapshot.ownerToken == fixtureOwnerToken &&
                          fixtureSnapshot.lifecycleGeneration ==
                              fixtureIntent.lifecycleGeneration &&
                          fixtureSnapshot.configurationRevision ==
                              fixtureConfigurationRevision &&
                          fixtureSnapshot.transactionGeneration ==
                              fixtureIntent.transactionGeneration;
            ownershipResultCell.predictionPass =
                ownershipPrediction ? UINT8_C (1) : UINT8_C (0);

            if (!predictionPass || !receiptProvenancePass ||
                ownershipResultCell.predictionPass == 0)
            {
                allPredictionsPassCell = 0;
            }

            if (cellIndex >= resultIndex)
            {
                resultIndex              = static_cast<uint8_t> (cellIndex + 1U);
                completedResultCellsCell = resultIndex;
            }
        }

        if (replayStage == ReplayStage::Complete)
        {
            replayActive     = false;
            replayActiveCell = 0;
        }
    }

} // namespace
