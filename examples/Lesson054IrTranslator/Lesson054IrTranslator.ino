// E0 translator fixture. This sketch replays copied NEC-shaped evidence and
// stores response intent in memory. It owns no receiver, emitter, pin, timer,
// live clock, or powered circuit, and must not run as hardware evidence.
#include <Adk.h>
#include <inert_ir_translator.h>

namespace {

    struct TranslationCell
    {
        uint32_t operationId;
        uint8_t  receivedCode;
        uint8_t  transmittedCode;
        uint8_t  disposition;
        uint8_t  available;
    };

    struct ReceiveCell
    {
        uint32_t sequence;
        uint8_t  sourceId;
        uint8_t  disposition;
        uint8_t  strength;
        uint8_t  status;
        uint8_t  available;
    };

    struct TransmitCell
    {
        uint32_t transactionId;
        uint8_t  code;
        uint8_t  sourceId;
        uint8_t  intent;
        uint8_t  status;
        uint8_t  available;
    };

    struct FaultCell
    {
        uint8_t receiveStatus;
        uint8_t transmitStatus;
        uint8_t translatorStatus;
        uint8_t active;
        uint8_t available;
    };

    struct SuppressedCell
    {
        uint32_t count;
        uint8_t  disposition;
        uint8_t  available;
    };

    struct RoundTripCell
    {
        uint32_t operationId;
        uint32_t elapsedMicroseconds;
        uint8_t  transmittedCode;
        uint8_t  disposition;
        uint8_t  status;
        uint8_t  complete;
        uint8_t  available;
    };

    constexpr uint8_t  receivePulseCount = 67;
    constexpr uint32_t operationId       = 1;
    constexpr uint32_t translationAt     = 1000;
    constexpr uint32_t emissionEndsAt    = 2000;
    constexpr uint32_t selfEchoAt        = 3999;
    constexpr uint32_t responseAt        = 4000;

    const adk::IrSourceIdentity emitterSource = {adk::IrSourceKind::SyntheticFixture, 2,
                                                 2, 3};
    const adk::IrTranslatorConfig translatorConfig = {7,
                                                      11,
                                                      adk::syntheticIrMappingDigest,
                                                      adk::MicrosecondDuration (67980),
                                                      adk::syntheticIrReceiveSource,
                                                      emitterSource,
                                                      adk::MicrosecondDuration (2000),
                                                      adk::MicrosecondDuration (10000)};

    uint32_t               capturedPulseWords[adk::capturedIrPulseCapacity];
    adk::InertIrTranslator translator (translatorConfig,
                                       {capturedPulseWords,
                                        adk::capturedIrPulseCapacity});
    adk::Pulse             receivePulses[receivePulseCount];

    volatile TranslationCell translationCell      = {0, 0xff, 0xff, 0xff, 0};
    volatile ReceiveCell     receiveCell          = {0, 0, 0xff, 0xff, 0xff, 0};
    volatile TransmitCell    transmitCell         = {0, 0xff, 0, 0xff, 0xff, 0};
    volatile FaultCell       faultCell            = {0xff, 0xff, 0xff, 0, 0};
    volatile SuppressedCell  suppressedCell       = {0, 0xff, 0};
    volatile RoundTripCell   roundTripCell        = {0, 0, 0xff, 0xff, 0xff, 0, 0};
    volatile uint8_t         initializeStatusCell = 0xff;
    volatile uint8_t         fixtureValidCell     = 0;
    volatile uint8_t         prepareStatusCell    = 0xff;
    volatile uint8_t         commitStatusCell     = 0xff;

    bool replayActive = false;

    // clang-format off
    bool                       configureReceiveFixture (
                                   adk::LocalIrCodeId code);
    adk::IrTranslatorUpdateInput observeReceive     (
                                   adk::LocalIrCodeId code, uint32_t sequence,
                                   uint32_t observedAt);
    adk::Result<adk::IrTranslatorPreview>
                               decideResponse      ();
    adk::Status                actuateResponse      (
                                   const adk::IrTranslatorPreview& preview);

    adk::Status                recordEmission      ();

    adk::Status                suppressSelfEcho    ();

    adk::Status                completeRoundTrip   ();

    void                       publishResults       (adk::Status replayStatus);

    void                       setNecBit            (uint8_t bit, bool one);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status acquired = translator.initialize ();

    const bool valid = configureReceiveFixture (adk::LocalIrCodeId::StationPing);

    initializeStatusCell = static_cast<uint8_t> (acquired.error ());
    fixtureValidCell     = valid ? 1 : 0;
    replayActive         = acquired.ok () && valid;
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const adk::IrTranslatorUpdateInput observed =
        observeReceive (adk::LocalIrCodeId::StationPing, 1, translationAt);

    adk::Status replayStatus = translator.update (observed);

    if (!replayStatus.ok ())
    {
        publishResults (replayStatus);
        replayActive = false;
        return;
    }

    const adk::Result<adk::IrTranslatorPreview> response = decideResponse ();

    prepareStatusCell = static_cast<uint8_t> (response.error ());
    replayStatus =
        response.ok () ? actuateResponse (response.value ()) : response.status ();
    commitStatusCell = static_cast<uint8_t> (replayStatus.error ());

    if (replayStatus.ok ())
    {
        replayStatus = recordEmission ();
    }
    if (replayStatus.ok ())
    {
        replayStatus = suppressSelfEcho ();
    }
    if (replayStatus.ok ())
    {
        replayStatus = completeRoundTrip ();
    }

    publishResults (replayStatus);
    replayActive = false;
}

namespace {

    bool configureReceiveFixture (adk::LocalIrCodeId code)
    {
        const uint8_t index = static_cast<uint8_t> (code);
        if (index >= adk::syntheticIrFixtureCount)
        {
            return false;
        }

        const adk::SyntheticIrReceiveFixture& fixture =
            adk::syntheticIrReceiveFixtures[index];
        receivePulses[0] =
            adk::Pulse (adk::PulseLevel::Mark, adk::MicrosecondDuration (9000));
        receivePulses[1] =
            adk::Pulse (adk::PulseLevel::Space, adk::MicrosecondDuration (4500));

        const uint32_t addressBits =
            fixture.address | ((fixture.address ^ UINT32_C (0xff)) << 8);
        const uint32_t commandBits =
            (fixture.command << 16) | ((fixture.command ^ UINT32_C (0xff)) << 24);
        const uint32_t frameBits = addressBits | commandBits;

        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            setNecBit (bit, (frameBits & (UINT32_C (1) << bit)) != 0);
        }

        receivePulses[66] =
            adk::Pulse (adk::PulseLevel::Mark, adk::MicrosecondDuration (560));

        return fixture.protocol == adk::InfraredProtocol::Nec &&
               fixture.receivedCode == code && fixture.address <= UINT8_MAX &&
               fixture.command <= UINT8_MAX;
    }

    adk::IrTranslatorUpdateInput observeReceive (adk::LocalIrCodeId code,
                                                 uint32_t sequence, uint32_t observedAt)
    {
        configureReceiveFixture (code);

        adk::IrTranslatorUpdateInput    input{};
        const adk::Status               sourceStatus;
        const adk::MicrosecondTimePoint observationTime (observedAt);

        input.now            = observationTime;
        input.receivePresent = true;
        input.receive        = {
            adk::syntheticIrReceiveSource,
            sourceStatus,
            observationTime,
            {receivePulses, receivePulseCount, sequence, adk::CaptureState::Complete}};
        return input;
    }

    adk::Result<adk::IrTranslatorPreview> decideResponse ()
    {
        return translator.prepareTranslation (
            operationId, adk::MicrosecondTimePoint (translationAt));
    }

    adk::Status actuateResponse (const adk::IrTranslatorPreview& preview)
    {
        if (!translator.canCommit (preview, adk::MicrosecondTimePoint (translationAt)))
        {
            return adk::StatusCode::InternalInvariant;
        }

        adk::IrTranslatorUpdateInput input{};
        input.now           = adk::MicrosecondTimePoint (translationAt);
        input.commitPresent = true;
        input.commitPreview = preview;
        return translator.update (input);
    }

    adk::Status recordEmission ()
    {
        adk::IrTranslatorUpdateInput    input{};
        const adk::MicrosecondTimePoint startedAt (translationAt);

        const adk::MicrosecondTimePoint completedAt (emissionEndsAt);

        input.now                   = adk::MicrosecondTimePoint (emissionEndsAt);
        input.actualEmissionPresent = true;
        input.actualEmission = {emitterSource, operationId, startedAt, completedAt,
                                adk::Status ()};
        return translator.update (input);
    }

    adk::Status suppressSelfEcho ()
    {
        const adk::IrTranslatorUpdateInput input =
            observeReceive (adk::LocalIrCodeId::StationPing, 2, selfEchoAt);
        const adk::Status status = translator.update (input);

        const adk::IrTranslatorSnapshot snapshot = translator.snapshot ();

        suppressedCell.count       = snapshot.suppressedEchoCount;
        suppressedCell.disposition = static_cast<uint8_t> (snapshot.disposition);
        suppressedCell.available   = 1;
        return status;
    }

    adk::Status completeRoundTrip ()
    {
        const adk::IrTranslatorUpdateInput input =
            observeReceive (adk::LocalIrCodeId::StationReady, 3, responseAt);
        return translator.update (input);
    }

    void publishResults (adk::Status replayStatus)
    {
        const adk::IrTranslatorSnapshot snapshot = translator.snapshot ();

        const adk::CapturedIrSnapshot receive = translator.receiveSnapshot ();

        const adk::KnownIrEmissionSnapshot emission = translator.emissionSnapshot ();

        translationCell.operationId     = snapshot.operationId;
        translationCell.receivedCode    = static_cast<uint8_t> (snapshot.receivedCode);
        translationCell.transmittedCode = static_cast<uint8_t> (snapshot.transmitCode);
        translationCell.disposition     = static_cast<uint8_t> (snapshot.disposition);
        translationCell.available       = 1;

        receiveCell.sequence    = receive.provenance.captureSequence;
        receiveCell.sourceId    = receive.provenance.source.sourceId;
        receiveCell.disposition = static_cast<uint8_t> (receive.disposition);
        receiveCell.strength    = static_cast<uint8_t> (receive.strength);
        receiveCell.status      = static_cast<uint8_t> (receive.status.error ());
        receiveCell.available   = 1;

        transmitCell.transactionId = snapshot.transmitTransactionId;
        transmitCell.code          = static_cast<uint8_t> (snapshot.transmitCode);
        transmitCell.sourceId      = snapshot.transmitSource.sourceId;
        transmitCell.intent        = static_cast<uint8_t> (snapshot.transmitIntent);
        transmitCell.status        = static_cast<uint8_t> (emission.status.error ());
        transmitCell.available     = 1;

        const bool replayFault = !receive.status.ok () || !emission.status.ok () ||
                                 !snapshot.status.ok () || !replayStatus.ok ();

        faultCell.receiveStatus = static_cast<uint8_t> (receive.status.error ());

        faultCell.transmitStatus = static_cast<uint8_t> (emission.status.error ());

        faultCell.translatorStatus = static_cast<uint8_t> (snapshot.status.error ());

        faultCell.active    = replayFault ? 1 : 0;
        faultCell.available = 1;

        suppressedCell.count = snapshot.suppressedEchoCount;

        roundTripCell.operationId         = snapshot.roundTrip.operationId;
        roundTripCell.elapsedMicroseconds = snapshot.roundTrip.elapsed.microseconds ();
        roundTripCell.transmittedCode =
            static_cast<uint8_t> (snapshot.roundTrip.transmittedCode);
        roundTripCell.disposition =
            static_cast<uint8_t> (snapshot.roundTrip.correlationDisposition);
        roundTripCell.status =
            static_cast<uint8_t> (snapshot.roundTrip.status.error ());
        roundTripCell.complete  = snapshot.roundTrip.complete ? 1 : 0;
        roundTripCell.available = 1;
    }

    void setNecBit (uint8_t bit, bool one)
    {
        receivePulses[2 + bit * 2] =
            adk::Pulse (adk::PulseLevel::Mark, adk::MicrosecondDuration (560));
        receivePulses[3 + bit * 2] = adk::Pulse (
            adk::PulseLevel::Space, adk::MicrosecondDuration (one ? 1690 : 560));
    }

} // namespace
