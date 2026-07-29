// E0 copied-IR fixture. This sketch replays checked-in pulse traces and stores
// decoded evidence in memory. It owns no receiver, emitter, pin, timer,
// interrupt, carrier output, or optical power path. Compilation and result
// cells are not physical acceptance.
#include <Adk.h>
#include <captured_ir_evidence.h>

namespace {

    struct ReplayFrame
    {
        const adk::Pulse*         pulses;
        uint8_t                   pulseCount;
        uint32_t                  sequence;
        uint32_t                  observedAtUs;
        adk::IrCaptureDisposition expectedDisposition;
        adk::EvidenceStrength     expectedStrength;
        adk::Status               sourceStatus;
    };

    struct EvidenceResultCell
    {
        uint32_t captureSequence;
        uint32_t evidenceGeneration;
        uint32_t address;
        uint32_t command;
        uint32_t firstPulseWord;
        uint32_t lastPulseWord;
        uint8_t  disposition;
        uint8_t  strength;
        uint8_t  pulseCount;
        uint8_t  operationStatus;
        uint8_t  evidenceStatus;
        uint8_t  viewStatus;
        uint8_t  predictionPass;
    };

    adk::Pulse fixturePulse (adk::PulseLevel level, uint32_t duration)
    {
        return {level, adk::MicrosecondDuration (duration)};
    }

    const adk::Pulse validNecTrace[] = {fixturePulse (adk::PulseLevel::Mark, 9000),
                                        fixturePulse (adk::PulseLevel::Space, 4500),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 1690),
                                        fixturePulse (adk::PulseLevel::Mark, 560),
                                        fixturePulse (adk::PulseLevel::Space, 560),
                                        fixturePulse (adk::PulseLevel::Mark, 560)};

    const adk::Pulse repeatTrace[] = {fixturePulse (adk::PulseLevel::Mark, 9000),
                                      fixturePulse (adk::PulseLevel::Space, 2250),
                                      fixturePulse (adk::PulseLevel::Mark, 560)};

    const adk::Pulse unknownTrace[] = {fixturePulse (adk::PulseLevel::Space, 9000),
                                       fixturePulse (adk::PulseLevel::Mark, 2250),
                                       fixturePulse (adk::PulseLevel::Space, 560)};

    const ReplayFrame replayFrames[] = {
        {validNecTrace,
         static_cast<uint8_t> (sizeof (validNecTrace) / sizeof (validNecTrace[0])), 1,
         1000, adk::IrCaptureDisposition::KnownValid,
         adk::EvidenceStrength::IntegrityVerified, adk::StatusCode::Ok},
        {repeatTrace,
         static_cast<uint8_t> (sizeof (repeatTrace) / sizeof (repeatTrace[0])), 2, 2000,
         adk::IrCaptureDisposition::KnownRepeat, adk::EvidenceStrength::ShapeRecognized,
         adk::StatusCode::Ok},
        {unknownTrace,
         static_cast<uint8_t> (sizeof (unknownTrace) / sizeof (unknownTrace[0])), 3,
         3000, adk::IrCaptureDisposition::UnknownProtocol, adk::EvidenceStrength::None,
         adk::StatusCode::Ok},
        {validNecTrace, 10, 4, 4000, adk::IrCaptureDisposition::Truncated,
         adk::EvidenceStrength::ShapeRecognized, adk::StatusCode::Ok},
        {validNecTrace,
         static_cast<uint8_t> (sizeof (validNecTrace) / sizeof (validNecTrace[0])), 5,
         5000, adk::IrCaptureDisposition::SourceFault, adk::EvidenceStrength::None,
         adk::StatusCode::HardwareFailure}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    const adk::IrSourceIdentity fixtureSource = {adk::IrSourceKind::SyntheticFixture, 1,
                                                 1, 1};

    adk::InfraredDecoder    decoder;
    uint32_t                copiedPulseWords[adk::capturedIrPulseCapacity];
    adk::CapturedIrEvidence evidence (
        decoder,
        {copiedPulseWords, static_cast<uint8_t> (sizeof (copiedPulseWords) /
                                                 sizeof (copiedPulseWords[0]))},
        adk::capturedIrPulseCapacity);

    volatile EvidenceResultCell resultCells[replayFrameCount];
    volatile uint8_t            fixtureStatusCell;
    volatile uint8_t            initializeStatusCell;
    volatile uint8_t            replayActiveCell;
    volatile uint8_t            completedReplayFramesCell;

    uint8_t replayIndex;
    bool    replayActive;

    // clang-format off
    adk::Status     acquireEvidenceFixture  ();
    void            configureReplay         ();
    adk::Status     startEvidenceReplay     ();
    adk::PulseFrame observeCheckedInTrace   (const ReplayFrame& frame);
    adk::Status     decideCapturedEvidence  (const ReplayFrame&     frame,
                                             const adk::PulseFrame& capture);
    void            presentCapturedEvidence (uint8_t            index,
                                             const ReplayFrame& frame,
                                             adk::Status        operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireEvidenceFixture ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    configureReplay ();

    const adk::Status initializationStatus = startEvidenceReplay ();

    initializeStatusCell = static_cast<uint8_t> (initializationStatus.error ());

    if (!initializationStatus.ok ())
    {
        return;
    }
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame&    frame   = replayFrames[replayIndex];
    const adk::PulseFrame capture = observeCheckedInTrace (frame);

    const adk::Status operationStatus = decideCapturedEvidence (frame, capture);

    presentCapturedEvidence (replayIndex, frame, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;
    replayActive              = replayIndex < replayFrameCount;
    replayActiveCell          = replayActive ? 1 : 0;
}

namespace {

    adk::Status acquireEvidenceFixture ()
    {
        if (replayFrameCount == 0 ||
            sizeof (validNecTrace) / sizeof (validNecTrace[0]) >
                adk::capturedIrPulseCapacity)
        {
            return adk::StatusCode::InvalidConfiguration;
        }

        return adk::StatusCode::Ok;
    }

    void configureReplay ()
    {
        initializeStatusCell      = 0xff;
        replayActiveCell          = 0;
        completedReplayFramesCell = 0;
        replayIndex               = 0;
        replayActive              = false;

        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            resultCells[index] = {0, 0, 0, 0, 0, 0, 0xff, 0xff, 0, 0xff, 0xff, 0xff, 0};
        }
    }

    adk::Status startEvidenceReplay ()
    {
        const adk::Status status = evidence.initialize ();

        if (status.ok ())
        {
            replayActive     = true;
            replayActiveCell = 1;
        }

        return status;
    }

    adk::PulseFrame observeCheckedInTrace (const ReplayFrame& frame)
    {
        return {frame.pulses, frame.pulseCount, frame.sequence,
                adk::CaptureState::Complete};
    }

    adk::Status decideCapturedEvidence (const ReplayFrame&     frame,
                                        const adk::PulseFrame& capture)
    {
        return evidence.admit (capture, fixtureSource, frame.sourceStatus,
                               adk::MicrosecondTimePoint (frame.observedAtUs));
    }

    void presentCapturedEvidence (uint8_t index, const ReplayFrame& frame,
                                  adk::Status operationStatus)
    {
        const adk::CapturedIrSnapshot snapshot = evidence.snapshot ();

        const adk::Result<adk::CapturedIrView> viewResult = evidence.view ();

        volatile EvidenceResultCell& result = resultCells[index];

        result.captureSequence    = snapshot.provenance.captureSequence;
        result.evidenceGeneration = snapshot.evidenceGeneration;
        result.address            = snapshot.address;
        result.command            = snapshot.command;
        result.firstPulseWord     = viewResult.ok () && viewResult.value ().size != 0
                                        ? viewResult.value ().words[0]
                                        : 0;
        result.lastPulseWord =
            viewResult.ok () && viewResult.value ().size != 0
                ? viewResult.value ().words[viewResult.value ().size - 1]
                : 0;
        result.disposition     = static_cast<uint8_t> (snapshot.disposition);
        result.strength        = static_cast<uint8_t> (snapshot.strength);
        result.pulseCount      = snapshot.pulseCount;
        result.operationStatus = static_cast<uint8_t> (operationStatus.error ());

        result.evidenceStatus = static_cast<uint8_t> (snapshot.status.error ());

        result.viewStatus = static_cast<uint8_t> (viewResult.status ().error ());
        result.predictionPass =
            operationStatus.ok () && snapshot.status == frame.sourceStatus &&
                    viewResult.ok () &&
                    snapshot.disposition == frame.expectedDisposition &&
                    snapshot.strength == frame.expectedStrength
                ? 1
                : 0;
    }

} // namespace
