// E0 MAX7219 fixture. This sketch records copied register commands and
// chip-select framing in memory. It owns no SPI endpoint, pin, timer, display,
// module, or powered circuit.
#include <Adk.h>
#include <max7219_presentation_policy.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        Configure,
        SubmitRotatedFrame,
        InjectPartialPrefix,
        Cleanup,
        Shutdown,
        Complete
    };

    struct CommandTraceCell
    {
        uint32_t presentationGeneration;
        uint8_t  operation;
        uint8_t  operationIndex;
        uint8_t  registerAddress;
        uint8_t  data;
        uint8_t  acceptedByteCount;
        uint8_t  chipSelectActiveBefore;
        uint8_t  chipSelectInactiveAfter;
        uint8_t  transportStatus;
        uint8_t  predictionPass;
    };

    struct SnapshotCell
    {
        uint32_t desiredGeneration;
        uint32_t submittedGeneration;
        uint8_t  partialPrefix;
        uint8_t  fault;
        uint8_t  configured;
        uint8_t  blankRequested;
        uint8_t  cleanupPending;
        uint8_t  shutdownAccepted;
        uint8_t  physicallyIndeterminate;
        uint8_t  initialized;
        uint8_t  predictionPass;
    };

    constexpr uint8_t firstLogicalFrame[8] = {0x80, 0x40, 0x20, 0x10,
                                              0x08, 0x04, 0x02, 0x01};
    constexpr uint8_t faultLogicalFrame[8] = {0xff, 0xff, 0xff, 0xff,
                                              0xff, 0xff, 0xff, 0xff};

    const adk::Max7219PresentationConfig
        matrixConfig (590, 59, adk::Max7219Orientation::Rotate90, 1);
    adk::Max7219PresentationPolicy matrixPolicy (matrixConfig);

    constexpr uint8_t maximumTraceCells = 32;

    volatile CommandTraceCell traceCells[maximumTraceCells];
    volatile SnapshotCell     finalSnapshotCell;
    volatile uint8_t          fixtureStatusCell;
    volatile uint8_t          initializeStatusCell;
    volatile uint8_t          replayStageCell;
    volatile uint8_t          recordedCommandCountCell;
    volatile uint8_t          allPredictionsPassCell;

    adk::Max7219Receipt pendingReceipt;
    ReplayStage         replayStage;
    uint32_t            observationTime;
    uint8_t             recordedCommandCount;
    bool                havePendingReceipt;
    bool                faultInjected;

    // clang-format off
    adk::Status acquireReplayFixture ();
    void        configureReplay      ();
    adk::Status startMatrixReplay    ();
    void        observeReplayStage   ();
    adk::Status decideMatrixIntent   (adk::Max7219Command& command);
    void        presentMatrixIntent  (adk::Status serviceStatus,
                                      const adk::Max7219Command& command);
    adk::Status commitFrame          (const uint8_t rows[8],
                                      uint32_t sourceSequence);
    void        recordTransport      (const adk::Max7219Command& command,
                                      bool injectPartialFailure);
    bool        commandPrediction    (const adk::Max7219Command& command,
                                      bool partialFailure);
    void        recordFinalSnapshot  ();
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireReplayFixture ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    configureReplay ();

    const adk::Status initializeStatus = startMatrixReplay ();

    initializeStatusCell = static_cast<uint8_t> (initializeStatus.error ());
}

void loop ()
{
    if (replayStage == ReplayStage::Complete)
    {
        return;
    }

    observeReplayStage ();

    adk::Max7219Command command = {
        0, 0, 0, 0, 0, 0, 0, adk::Max7219Operation::Configure, false};
    const adk::Status serviceStatus = decideMatrixIntent (command);

    presentMatrixIntent (serviceStatus, command);
}

namespace {

    adk::Status acquireReplayFixture ()
    {
        return maximumTraceCells >= 25 ? adk::StatusCode::Ok
                                       : adk::StatusCode::InternalInvariant;
    }

    void configureReplay ()
    {
        replayStage                   = ReplayStage::Configure;
        observationTime               = 59000;
        recordedCommandCount          = 0;
        havePendingReceipt            = false;
        faultInjected                 = false;
        replayStageCell               = static_cast<uint8_t> (replayStage);
        recordedCommandCountCell      = 0;
        allPredictionsPassCell        = 1;
        initializeStatusCell          = 0xff;
        finalSnapshotCell.initialized = 0;
    }

    adk::Status startMatrixReplay ()
    {
        const adk::Status status = matrixPolicy.initialize ();

        if (!status.ok ())
        {
            replayStage = ReplayStage::Complete;
        }
        return status;
    }

    void observeReplayStage ()
    {
        const adk::Max7219PresentationSnapshot snapshot = matrixPolicy.snapshot ();

        if (replayStage == ReplayStage::Configure && snapshot.configured &&
            !snapshot.outstanding)
        {
            replayStage = ReplayStage::SubmitRotatedFrame;
        }
        else if (replayStage == ReplayStage::SubmitRotatedFrame &&
                 snapshot.submittedFrame.generation == 1 && !snapshot.outstanding)
        {
            replayStage = ReplayStage::InjectPartialPrefix;
        }
        else if (replayStage == ReplayStage::InjectPartialPrefix &&
                 snapshot.cleanupPending && !snapshot.outstanding)
        {
            replayStage = ReplayStage::Cleanup;
        }
        else if (replayStage == ReplayStage::Cleanup &&
                 snapshot.shutdownCommandAccepted && !snapshot.outstanding)
        {
            replayStage = ReplayStage::Shutdown;
        }

        replayStageCell = static_cast<uint8_t> (replayStage);
    }

    adk::Status decideMatrixIntent (adk::Max7219Command& command)
    {
        const adk::Max7219PresentationSnapshot snapshot = matrixPolicy.snapshot ();

        if (replayStage == ReplayStage::SubmitRotatedFrame &&
            snapshot.desiredFrame.generation == 0)
        {
            const adk::Status status = commitFrame (firstLogicalFrame, 59001);

            if (!status.ok ())
            {
                return status;
            }
        }
        else if (replayStage == ReplayStage::InjectPartialPrefix &&
                 snapshot.desiredFrame.generation == 1)
        {
            const adk::Status status = commitFrame (faultLogicalFrame, 59002);

            if (!status.ok ())
            {
                return status;
            }
        }
        else if (replayStage == ReplayStage::Shutdown)
        {
            recordFinalSnapshot ();

            matrixPolicy.shutdown ();

            const adk::Max7219PresentationSnapshot shutdownSnapshot =
                matrixPolicy.snapshot ();
            const bool shutdownPrediction =
                finalSnapshotCell.predictionPass != 0 &&
                !shutdownSnapshot.initialized &&
                shutdownSnapshot.shutdownCommandAccepted &&
                shutdownSnapshot.fault == adk::Max7219Fault::Transport;

            finalSnapshotCell.initialized =
                shutdownSnapshot.initialized ? UINT8_C (1) : UINT8_C (0);
            finalSnapshotCell.predictionPass =
                shutdownPrediction ? UINT8_C (1) : UINT8_C (0);
            if (!shutdownPrediction)
            {
                allPredictionsPassCell = 0;
            }

            replayStage     = ReplayStage::Complete;
            replayStageCell = static_cast<uint8_t> (replayStage);
            return adk::StatusCode::Ok;
        }

        const adk::Result<adk::Max7219Command> result =
            havePendingReceipt ? matrixPolicy.service (&pendingReceipt)
                               : matrixPolicy.service ();

        havePendingReceipt = false;
        command            = result.value ();

        return result.status ();
    }

    void presentMatrixIntent (adk::Status                serviceStatus,
                              const adk::Max7219Command& command)
    {
        if (!serviceStatus.ok () && command.emitted)
        {
            allPredictionsPassCell = 0;
        }
        if (!command.emitted)
        {
            return;
        }

        const bool injectPartialFailure =
            replayStage == ReplayStage::InjectPartialPrefix && !faultInjected &&
            command.operation == adk::Max7219Operation::SubmitRow &&
            command.operationIndex == 0;

        recordTransport (command, injectPartialFailure);
        faultInjected = faultInjected || injectPartialFailure;
    }

    adk::Status commitFrame (const uint8_t rows[8], uint32_t sourceSequence)
    {
        adk::Max7219PresentationPreview candidate;
        adk::Status status = matrixPolicy.preview (rows, sourceSequence, candidate);

        if (status.ok ())
        {
            status = matrixPolicy.commit (candidate);
        }
        return status;
    }

    void recordTransport (const adk::Max7219Command& command, bool partialFailure)
    {
        const bool traceCapacity = recordedCommandCount < maximumTraceCells;
        const bool prediction    = commandPrediction (command, partialFailure);

        if (!traceCapacity || !prediction)
        {
            allPredictionsPassCell = 0;
        }
        if (traceCapacity)
        {
            volatile CommandTraceCell& cell = traceCells[recordedCommandCount];

            cell.presentationGeneration  = command.presentationGeneration;
            cell.operation               = static_cast<uint8_t> (command.operation);
            cell.operationIndex          = command.operationIndex;
            cell.registerAddress         = command.registerAddress;
            cell.data                    = command.data;
            cell.acceptedByteCount       = partialFailure ? UINT8_C (1) : UINT8_C (2);
            cell.chipSelectActiveBefore  = 1;
            cell.chipSelectInactiveAfter = 1;
            cell.transportStatus =
                static_cast<uint8_t> (partialFailure ? adk::StatusCode::HardwareFailure
                                                     : adk::StatusCode::Ok);
            cell.predictionPass = prediction ? UINT8_C (1) : UINT8_C (0);
        }

        pendingReceipt.ownerToken             = command.ownerToken;
        pendingReceipt.lifecycleGeneration    = command.lifecycleGeneration;
        pendingReceipt.configurationRevision  = command.configurationRevision;
        pendingReceipt.presentationGeneration = command.presentationGeneration;
        pendingReceipt.operationIndex         = command.operationIndex;
        pendingReceipt.registerAddress        = command.registerAddress;
        pendingReceipt.data                   = command.data;
        pendingReceipt.operation              = command.operation;
        pendingReceipt.acceptedByteCount  = partialFailure ? UINT8_C (1) : UINT8_C (2);
        pendingReceipt.chipSelectInactive = true;
        pendingReceipt.observedAt         = adk::TimePoint (observationTime++);
        pendingReceipt.status =
            partialFailure ? adk::StatusCode::HardwareFailure : adk::StatusCode::Ok;
        havePendingReceipt = true;

        ++recordedCommandCount;
        recordedCommandCountCell = recordedCommandCount;
    }

    bool commandPrediction (const adk::Max7219Command& command, bool partialFailure)
    {
        const bool identity = command.ownerToken == 590 &&
                              command.configurationRevision == 59 &&
                              command.lifecycleGeneration == 1;

        if (!identity)
        {
            return false;
        }
        if (command.operation == adk::Max7219Operation::Configure)
        {
            constexpr uint8_t addresses[14] = {0x0c, 0x0f, 0x09, 0x0b, 0x0a, 1, 2,
                                               3,    4,    5,    6,    7,    8, 0x0c};
            constexpr uint8_t values[14] = {0, 0, 0, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1};

            return command.presentationGeneration == 0 && command.operationIndex < 14 &&
                   command.registerAddress == addresses[command.operationIndex] &&
                   command.data == values[command.operationIndex];
        }
        if (command.operation == adk::Max7219Operation::CleanupShutdown)
        {
            return command.registerAddress == 0x0c && command.data == 0;
        }

        const uint8_t expectedData =
            command.presentationGeneration == 1
                ? static_cast<uint8_t> (
                      UINT8_C (1) << static_cast<uint8_t> (command.registerAddress - 1))
                : UINT8_C (0xff);

        return command.operation == adk::Max7219Operation::SubmitRow &&
               command.registerAddress >= 1 && command.registerAddress <= 8 &&
               command.data == expectedData &&
               (command.presentationGeneration == 1 ||
                (command.presentationGeneration == 2 && partialFailure));
    }

    void recordFinalSnapshot ()
    {
        const adk::Max7219PresentationSnapshot snapshot = matrixPolicy.snapshot ();

        finalSnapshotCell.desiredGeneration   = snapshot.desiredFrame.generation;
        finalSnapshotCell.submittedGeneration = snapshot.submittedFrame.generation;
        finalSnapshotCell.partialPrefix       = snapshot.partialPrefix;
        finalSnapshotCell.fault               = static_cast<uint8_t> (snapshot.fault);
        finalSnapshotCell.configured          = snapshot.configured ? 1 : 0;
        finalSnapshotCell.blankRequested      = snapshot.blankRequested ? 1 : 0;
        finalSnapshotCell.cleanupPending      = snapshot.cleanupPending ? 1 : 0;
        finalSnapshotCell.shutdownAccepted =
            snapshot.shutdownCommandAccepted ? UINT8_C (1) : UINT8_C (0);
        finalSnapshotCell.physicallyIndeterminate =
            snapshot.physicallyIndeterminate ? UINT8_C (1) : UINT8_C (0);
        finalSnapshotCell.initialized = snapshot.initialized ? 1 : 0;

        const bool prediction =
            snapshot.desiredFrame.generation == 2 &&
            snapshot.submittedFrame.generation == 1 && snapshot.partialPrefix == 0 &&
            snapshot.fault == adk::Max7219Fault::Transport && snapshot.configured &&
            snapshot.blankRequested && !snapshot.cleanupPending &&
            snapshot.shutdownCommandAccepted && snapshot.physicallyIndeterminate &&
            snapshot.initialized;

        finalSnapshotCell.predictionPass = prediction ? UINT8_C (1) : UINT8_C (0);
        if (!prediction)
        {
            allPredictionsPassCell = 0;
        }
    }

} // namespace
