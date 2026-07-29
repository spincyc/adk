// E0 known-emission fixture. This sketch accepts only closed, firmware-authored
// symbols and stores envelope intent in memory. It owns no endpoint, pin,
// timer, interrupt, carrier generator, emitter, or optical power path.
#include <Adk.h>
#include <known_ir_emission_policy.h>

namespace {

    enum struct ReplayAction : uint8_t
    {
        PrepareAndCancel,
        PrepareAndCommit,
        Update,
        CancelActive
    };

    struct ReplayFrame
    {
        uint32_t           now;
        ReplayAction       action;
        adk::LocalIrCodeId codeId;
        uint32_t           transactionId;
    };

    struct EmissionResultCell
    {
        uint32_t transactionId;
        uint32_t terminalTransactionId;
        uint32_t policyGeneration;
        uint8_t  codeId;
        uint8_t  intent;
        uint8_t  disposition;
        uint8_t  terminalCause;
        uint8_t  operationStatus;
        uint8_t  policyStatus;
        uint8_t  catalogPass;
    };

    const adk::KnownIrEmissionConfig emissionConfig = {
        1, 0x05300001UL, adk::MicrosecondDuration (120000)};
    const adk::KnownIrCatalogIdentity expectedCatalog = {1, 0xbc6b6e95UL};

    const ReplayFrame replayFrames[] = {
        {2000, ReplayAction::PrepareAndCancel, adk::LocalIrCodeId::StationCancel, 1},
        {3000, ReplayAction::PrepareAndCommit, adk::LocalIrCodeId::StationAcknowledge,
         2},
        {3001, ReplayAction::Update, adk::LocalIrCodeId::StationAcknowledge, 2},
        {3100, ReplayAction::Update, adk::LocalIrCodeId::StationAcknowledge, 2},
        {3200, ReplayAction::CancelActive, adk::LocalIrCodeId::StationAcknowledge, 2}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::KnownIrEmissionPolicy emissionPolicy (emissionConfig);

    volatile EmissionResultCell resultCells[replayFrameCount];
    volatile uint8_t            fixtureStatusCell;
    volatile uint8_t            initializeStatusCell;
    volatile uint8_t            replayActiveCell;
    volatile uint8_t            completedReplayFramesCell;

    uint8_t replayIndex;
    bool    replayActive;

    // clang-format off
    adk::Status configureReplay        ();
    bool        catalogIdentityMatches ();
    adk::Status observeSymbolicRequest (const ReplayFrame& frame);
    adk::Status decideEmissionIntent   (const ReplayFrame& frame);
    void        presentEmissionIntent  (uint8_t index,
                                        adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = configureReplay ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    const adk::Status initialized = emissionPolicy.initialize ();

    initializeStatusCell = static_cast<uint8_t> (initialized.error ());

    if (!initialized.ok ())
    {
        return;
    }

    if (!catalogIdentityMatches ())
    {
        fixtureStatusCell =
            static_cast<uint8_t> (adk::StatusCode::InvalidConfiguration);
        return;
    }

    replayIndex      = 0;
    replayActive     = true;
    replayActiveCell = 1;
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame& frame = replayFrames[replayIndex];

    adk::Status operationStatus;
    operationStatus = observeSymbolicRequest (frame);

    if (operationStatus.ok ())
    {
        operationStatus = decideEmissionIntent (frame);
    }

    presentEmissionIntent (replayIndex, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;
    replayActive              = replayIndex < replayFrameCount;
    replayActiveCell          = replayActive ? 1 : 0;
}

namespace {

    adk::Status configureReplay ()
    {
        fixtureStatusCell         = 0xff;
        initializeStatusCell      = 0xff;
        replayActiveCell          = 0;
        completedReplayFramesCell = 0;
        replayIndex               = 0;
        replayActive              = false;

        return replayFrameCount == 0 ? adk::StatusCode::InvalidConfiguration
                                     : adk::StatusCode::Ok;
    }

    bool catalogIdentityMatches ()
    {
        const adk::KnownIrCatalogIdentity catalog = emissionPolicy.snapshot ().catalog;
        return catalog.revision == expectedCatalog.revision &&
               catalog.digest == expectedCatalog.digest;
    }

    adk::Status observeSymbolicRequest (const ReplayFrame& frame)
    {
        const uint8_t code = static_cast<uint8_t> (frame.codeId);
        if (code > static_cast<uint8_t> (adk::LocalIrCodeId::StationAcknowledge) ||
            frame.transactionId == 0)
        {
            return adk::StatusCode::InvalidArgument;
        }
        return adk::StatusCode::Ok;
    }

    adk::Status decideEmissionIntent (const ReplayFrame& frame)
    {
        const adk::MicrosecondTimePoint now (frame.now);

        if (frame.action == ReplayAction::PrepareAndCommit)
        {
            const adk::Result<adk::KnownIrEmissionPreview> prepared =
                emissionPolicy.prepare (frame.codeId, frame.transactionId, now);
            if (!prepared.ok ())
            {
                return prepared.status ();
            }
            if (!emissionPolicy.canCommit (prepared.value (), now))
            {
                return adk::StatusCode::InvalidArgument;
            }
            return emissionPolicy.commit (prepared.value (), now);
        }

        if (frame.action == ReplayAction::PrepareAndCancel)
        {
            const adk::Result<adk::KnownIrEmissionPreview> prepared =
                emissionPolicy.prepare (frame.codeId, frame.transactionId, now);
            if (!prepared.ok ())
            {
                return prepared.status ();
            }
            return emissionPolicy.cancel (prepared.value (), now);
        }

        if (frame.action == ReplayAction::CancelActive)
        {
            return emissionPolicy.cancel (frame.transactionId, now);
        }

        return emissionPolicy.update (now);
    }

    void presentEmissionIntent (uint8_t index, adk::Status operationStatus)
    {
        const adk::KnownIrEmissionSnapshot snapshot = emissionPolicy.snapshot ();

        resultCells[index].transactionId         = snapshot.transactionId;
        resultCells[index].terminalTransactionId = snapshot.terminalTransactionId;
        resultCells[index].policyGeneration      = snapshot.policyGeneration;
        resultCells[index].codeId      = static_cast<uint8_t> (snapshot.codeId);
        resultCells[index].intent      = static_cast<uint8_t> (snapshot.intent);
        resultCells[index].disposition = static_cast<uint8_t> (snapshot.disposition);
        resultCells[index].terminalCause =
            static_cast<uint8_t> (snapshot.terminalCause);
        resultCells[index].operationStatus =
            static_cast<uint8_t> (operationStatus.error ());
        resultCells[index].policyStatus =
            static_cast<uint8_t> (snapshot.status.error ());
        resultCells[index].catalogPass =
            snapshot.catalog.revision == expectedCatalog.revision &&
                    snapshot.catalog.digest == expectedCatalog.digest
                ? 1
                : 0;
    }

} // namespace
