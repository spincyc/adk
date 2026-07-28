// E0 balance-table fixture. This sketch replays only copied values and stores
// complete presentation intent in memory. It owns no endpoint, bus, live
// clock, or powered circuit, and must not run on a powered board.
#include <Adk.h>
#include <balance_table_instrument.h>

namespace {

    constexpr uint8_t resultCellCount = 25;

    const adk::InertialSource syntheticSource = {
        adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel::Synthetic,
        7,
        3,
        5,
        2000000,
        250000000};

    const adk::OrientationConfig orientationConfig = {{adk::SignedAxis::PositiveX,
                                                       adk::SignedAxis::PositiveY,
                                                       adk::SignedAxis::PositiveZ},
                                                      900000,
                                                      1100000,
                                                      1000,
                                                      5000,
                                                      60000};

    const adk::BalancePresentationConfig presentationConfig = {{0, 1000, 0, false},
                                                               {1000, 0, 0, false},
                                                               {0, 1000, 0, false},
                                                               {0, 0, 1000, false},
                                                               {1000, 1000, 0, false},
                                                               {0, 0, 300, false},
                                                               {0, 0, 700, false},
                                                               {1000, 500, 0, true},
                                                               {1000, 0, 1000, true},
                                                               60000,
                                                               100,
                                                               1000,
                                                               1000,
                                                               20};

    const adk::BalanceInstrumentConfig instrumentConfig = {
        200,
        1000,
        200,
        adk::Duration (100),
        1,
        adk::Duration (200),
        adk::Duration (20),
        {adk::BalanceDirection::None,
         {0, 200, 0, false},
         {false, 0, 0},
         adk::StatusCode::Ok},
        {adk::BalanceDirection::None,
         {300, 200, 0, false},
         {false, 0, 0},
         adk::StatusCode::Ok},
        {adk::BalanceDirection::None,
         {1000, 0, 0, true},
         {false, 0, 0},
         adk::StatusCode::HardwareFailure},
        {adk::BalanceDirection::None,
         {0, 0, 0, false},
         {false, 0, 0},
         adk::StatusCode::NotInitialized}};

    adk::BalanceFrameStorage replayStorage = {};
    adk::BalanceInstrument   instrument (instrumentConfig, orientationConfig,
                                         presentationConfig, replayStorage);

    struct EvidenceCell
    {
        uint8_t  sourceKind;
        uint8_t  sourceModel;
        uint8_t  sourceId;
        uint16_t configurationRevision;
        uint16_t calibrationRevision;
        uint32_t accelerationRangeMicroG;
        uint32_t angularRateRangeMilliDegreesPerSecond;
        uint32_t observedAt;
        uint32_t sequence;
        uint8_t  quality;
        uint32_t maximumAge;
        uint16_t freshnessRevision;
        uint8_t  saturation;
        uint8_t  acceptedDataReady;
        uint8_t  latestDataReady;
        uint8_t  status;
        int32_t  pitchMilliDegrees;
        int32_t  rollMilliDegrees;
        uint8_t  orientationQuality;
        uint8_t  orientationStatus;
        uint8_t  available;
    };

    volatile uint8_t      updateStatusCells[resultCellCount];
    volatile uint8_t      modeCells[resultCellCount];
    volatile uint8_t      outputStatusCells[resultCellCount];
    volatile uint8_t      inertialStatusCells[resultCellCount];
    volatile uint8_t      joystickStatusCells[resultCellCount];
    volatile uint8_t      buttonStatusCells[resultCellCount];
    volatile uint8_t      liveAvailableCells[resultCellCount];
    volatile uint8_t      liveQualityCells[resultCellCount];
    volatile uint8_t      frozenAvailableCells[resultCellCount];
    volatile uint8_t      frozenQualityCells[resultCellCount];
    volatile uint8_t      directionCells[resultCellCount];
    volatile uint16_t     redCells[resultCellCount];
    volatile uint16_t     greenCells[resultCellCount];
    volatile uint16_t     blueCells[resultCellCount];
    volatile uint8_t      lightFaultCells[resultCellCount];
    volatile uint8_t      toneEnabledCells[resultCellCount];
    volatile uint16_t     toneFrequencyCells[resultCellCount];
    volatile uint16_t     toneDurationCells[resultCellCount];
    volatile uint16_t     sensitivityCells[resultCellCount];
    volatile uint32_t     acceptedFrameCells[resultCellCount];
    volatile uint32_t     acceptedTimeCells[resultCellCount];
    volatile uint8_t      replayAvailableCells[resultCellCount];
    volatile EvidenceCell liveEvidenceCells[4];
    volatile EvidenceCell frozenEvidenceCells[4];

    void configureResultCells ();
    adk::BalanceInstrumentInput
    copiedFrame (uint32_t frameAt, uint32_t frameSequence, uint32_t inertialSequence,
                 uint32_t joystickSequence, uint32_t buttonSequence,
                 int32_t rightMicroG, int32_t forwardMicroG, int32_t upMicroG);
    void replayFrame (uint8_t index, const adk::BalanceInstrumentInput& input);

    void presentResult (uint8_t index, adk::Status updateStatus);

    void copyEvidence (volatile EvidenceCell&                 destination,
                       const adk::BalanceMeasurementEvidence& source);

} // namespace

void setup ()
{
    configureResultCells ();

    const adk::Status acquired = instrument.initialize ();

    presentResult (0, acquired);

    if (!acquired.ok ())
    {
        return;
    }

    adk::BalanceInstrumentInput frame = copiedFrame (0, 1, 1, 1, 1, 0, 0, 1000000);

    replayFrame (1, frame);

    frame = copiedFrame (1, 2, 2, 2, 2, 500000, 0, 866025);

    replayFrame (2, frame);

    frame = copiedFrame (2, 3, 3, 3, 3, 0, 500000, 866025);

    replayFrame (3, frame);

    frame = copiedFrame (3, 4, 4, 4, 4, -500000, 0, 866025);

    replayFrame (4, frame);

    frame = copiedFrame (4, 5, 5, 5, 5, 0, -500000, 866025);

    replayFrame (5, frame);

    frame                    = copiedFrame (5, 6, 6, 6, 6, 0, 0, 1000000);
    frame.joystick.xPermille = 1000;
    frame.joystick.event     = adk::SensitivityEvent::Increase;
    replayFrame (6, frame);

    frame                         = copiedFrame (6, 7, 7, 7, 7, 500000, 0, 866025);
    frame.freezeButton.pressed    = true;
    frame.freezeButton.pressEvent = true;
    replayFrame (7, frame);

    frame                           = copiedFrame (7, 8, 8, 8, 8, 0, 500000, 866025);
    frame.freezeButton.releaseEvent = true;
    replayFrame (8, frame);

    frame                         = copiedFrame (8, 9, 9, 9, 9, 0, 500000, 866025);
    frame.freezeButton.pressed    = true;
    frame.freezeButton.pressEvent = true;
    replayFrame (9, frame);

    frame                            = copiedFrame (9, 10, 10, 10, 10, 2000000, 0, 0);
    frame.inertial.sample.saturation = adk::InertialSaturation::Acceleration;
    frame.inertial.quality           = adk::InertialSampleQuality::Saturated;
    replayFrame (10, frame);

    frame = copiedFrame (10, 11, 11, 11, 11, 0, 0, 0);

    replayFrame (11, frame);

    frame                      = copiedFrame (111, 12, 11, 12, 12, 0, 0, 0);
    frame.inertial.sample      = replayStorage.previous.inertial.sample;
    frame.inertial.age         = adk::Duration (101);
    frame.inertial.quality     = adk::InertialSampleQuality::Stale;
    frame.inertial.sequenceGap = 0;
    replayFrame (12, frame);

    frame                     = copiedFrame (112, 13, 12, 13, 13, 0, 0, 1000000);
    frame.freezeButton.status = adk::StatusCode::HardwareFailure;
    replayFrame (13, frame);

    frame = copiedFrame (113, 14, 13, 14, 14, 0, 0, 1000000);

    replayFrame (14, frame);

    const adk::Status acknowledged = instrument.acknowledgeFault ();

    presentResult (15, acknowledged);

    frame = copiedFrame (114, 15, 14, 15, 15, 0, 0, 1000000);

    replayFrame (16, frame);

    frame                    = copiedFrame (115, 16, 15, 16, 16, 0, 0, 1000000);
    frame.joystick.xPermille = 1000;
    frame.joystick.event     = adk::SensitivityEvent::Increase;
    replayFrame (17, frame);

    frame                    = copiedFrame (116, 17, 16, 17, 17, 0, 0, 1000000);
    frame.joystick.xPermille = 1000;
    frame.joystick.event     = adk::SensitivityEvent::Increase;
    replayFrame (18, frame);

    for (uint8_t index = 19; index < 24; ++index)
    {
        const uint32_t sequence = static_cast<uint32_t> (index - 1);
        frame = copiedFrame (sequence + 100, sequence, sequence - 1, sequence, sequence,
                             0, 0, 1000000);
        frame.joystick.xPermille = -1000;
        frame.joystick.event     = adk::SensitivityEvent::Decrease;
        replayFrame (index, frame);
    }

    instrument.shutdown ();

    presentResult (24, adk::StatusCode::Ok);
}

void loop ()
{
}

namespace {

    void configureResultCells ()
    {
        for (uint8_t index = 0; index < resultCellCount; ++index)
        {
            updateStatusCells[index]    = 0xff;
            modeCells[index]            = 0xff;
            outputStatusCells[index]    = 0xff;
            inertialStatusCells[index]  = 0xff;
            joystickStatusCells[index]  = 0xff;
            buttonStatusCells[index]    = 0xff;
            liveAvailableCells[index]   = 0xff;
            liveQualityCells[index]     = 0xff;
            frozenAvailableCells[index] = 0xff;
            frozenQualityCells[index]   = 0xff;
            directionCells[index]       = 0xff;
            redCells[index]             = 0xffff;
            greenCells[index]           = 0xffff;
            blueCells[index]            = 0xffff;
            lightFaultCells[index]      = 0xff;
            toneEnabledCells[index]     = 0xff;
            toneFrequencyCells[index]   = 0xffff;
            toneDurationCells[index]    = 0xffff;
            sensitivityCells[index]     = 0xffff;
            acceptedFrameCells[index]   = 0xffffffffUL;
            acceptedTimeCells[index]    = 0xffffffffUL;
            replayAvailableCells[index] = 0xff;
        }
    }

    adk::BalanceInstrumentInput
    copiedFrame (uint32_t frameAt, uint32_t frameSequence, uint32_t inertialSequence,
                 uint32_t joystickSequence, uint32_t buttonSequence,
                 int32_t rightMicroG, int32_t forwardMicroG, int32_t upMicroG)
    {
        const adk::InertialSample      sample = {syntheticSource,
                                                 {rightMicroG, forwardMicroG, upMicroG},
                                                 {0, 0, 0},
                                                 adk::TimePoint (frameAt),
                                                 inertialSequence,
                                                 true,
                                                 adk::InertialSaturation::None,
                                                 adk::StatusCode::Ok};
        const adk::InertialObservation inertial = {
            sample,
            adk::InertialSampleQuality::Current,
            true,
            adk::Duration (),
            instrumentConfig.inertialMaximumAge,
            instrumentConfig.inertialFreshnessContractRevision,
            0,
            adk::StatusCode::Ok};

        return {inertial,
                {0, 0, adk::SensitivityEvent::None, adk::TimePoint (frameAt),
                 joystickSequence, adk::StatusCode::Ok},
                {false, false, false, adk::TimePoint (frameAt), buttonSequence,
                 adk::StatusCode::Ok},
                adk::TimePoint (frameAt),
                frameSequence};
    }

    void replayFrame (uint8_t index, const adk::BalanceInstrumentInput& input)
    {
        presentResult (index, instrument.update (input));
    }

    void presentResult (uint8_t index, adk::Status updateStatus)
    {
        const adk::BalanceInstrumentOutput output = instrument.snapshot ();

        updateStatusCells[index] = static_cast<uint8_t> (updateStatus.error ());
        modeCells[index]         = static_cast<uint8_t> (output.mode);
        outputStatusCells[index] = static_cast<uint8_t> (output.status.error ());
        inertialStatusCells[index] =
            static_cast<uint8_t> (output.inertialStatus.error ());
        joystickStatusCells[index] =
            static_cast<uint8_t> (output.joystickStatus.error ());
        buttonStatusCells[index]  = static_cast<uint8_t> (output.buttonStatus.error ());
        liveAvailableCells[index] = output.liveEvidence.available ? 1 : 0;
        liveQualityCells[index] =
            static_cast<uint8_t> (output.liveEvidence.provenance.quality);
        frozenAvailableCells[index] = output.frozenEvidence.available ? 1 : 0;
        frozenQualityCells[index] =
            static_cast<uint8_t> (output.frozenEvidence.provenance.quality);
        directionCells[index]   = static_cast<uint8_t> (output.presentation.direction);
        redCells[index]         = output.presentation.light.redPermille;
        greenCells[index]       = output.presentation.light.greenPermille;
        blueCells[index]        = output.presentation.light.bluePermille;
        lightFaultCells[index]  = output.presentation.light.fault ? 1 : 0;
        toneEnabledCells[index] = output.presentation.tone.enabled ? 1 : 0;
        toneFrequencyCells[index]   = output.presentation.tone.frequencyHertz;
        toneDurationCells[index]    = output.presentation.tone.durationMilliseconds;
        sensitivityCells[index]     = output.sensitivityPermille;
        acceptedFrameCells[index]   = output.acceptedFrameSequence;
        acceptedTimeCells[index]    = output.acceptedFrameAt.milliseconds ();
        replayAvailableCells[index] = replayStorage.available ? 1 : 0;

        if (index == 7 || index == 8 || index == 12 || index == 13)
        {
            const uint8_t evidenceIndex = index == 7    ? 0
                                          : index == 8  ? 1
                                          : index == 12 ? 2
                                                        : 3;
            copyEvidence (liveEvidenceCells[evidenceIndex], output.liveEvidence);
            copyEvidence (frozenEvidenceCells[evidenceIndex], output.frozenEvidence);
        }
    }

    void copyEvidence (volatile EvidenceCell&                 destination,
                       const adk::BalanceMeasurementEvidence& source)
    {
        destination.sourceKind  = static_cast<uint8_t> (source.provenance.source.kind);
        destination.sourceModel = static_cast<uint8_t> (source.provenance.source.model);
        destination.sourceId    = source.provenance.source.sourceId;
        destination.configurationRevision =
            source.provenance.source.configurationRevision;
        destination.calibrationRevision = source.provenance.source.calibrationRevision;
        destination.accelerationRangeMicroG =
            source.provenance.source.accelerationRangeMicroG;
        destination.angularRateRangeMilliDegreesPerSecond =
            source.provenance.source.angularRateRangeMilliDegreesPerSecond;
        destination.observedAt = source.provenance.observedAt.milliseconds ();
        destination.sequence   = source.provenance.sequence;
        destination.quality    = static_cast<uint8_t> (source.provenance.quality);
        destination.maximumAge = source.provenance.maximumAge.milliseconds ();
        destination.freshnessRevision = source.provenance.freshnessContractRevision;
        destination.saturation = static_cast<uint8_t> (source.provenance.saturation);
        destination.acceptedDataReady = source.provenance.acceptedDataReady ? 1 : 0;
        destination.latestDataReady   = source.provenance.latestDataReady ? 1 : 0;
        destination.status = static_cast<uint8_t> (source.provenance.status.error ());
        destination.pitchMilliDegrees  = source.estimate.pitchMilliDegrees;
        destination.rollMilliDegrees   = source.estimate.rollMilliDegrees;
        destination.orientationQuality = static_cast<uint8_t> (source.estimate.quality);
        destination.orientationStatus =
            static_cast<uint8_t> (source.estimate.status.error ());
        destination.available = source.available ? 1 : 0;
    }

} // namespace
