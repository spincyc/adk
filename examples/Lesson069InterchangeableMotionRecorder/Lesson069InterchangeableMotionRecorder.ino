// E0 copied-record project replay. This sketch stores canonical record images
// and semantic presentation intent in named memory cells. It owns no sensor,
// bus, clock, button, display, RGB output, storage medium, or powered circuit.
#include <Adk.h>
#include <inertial_record.h>
#include <inertial_record_qualification.h>
#include <qualified_motion_recorder.h>

namespace {

    struct MotionResultCell
    {
        uint8_t  mode;
        uint8_t  health;
        uint8_t  scriptStep;
        uint8_t  displayToken;
        uint8_t  rgbRed;
        uint8_t  rgbGreen;
        uint8_t  rgbBlue;
        uint8_t  orientationValid;
        int16_t  pitchTenthsDegree;
        int16_t  rollTenthsDegree;
        uint8_t  exportRequested;
        uint8_t  status;
        uint8_t  predictionPass;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t initializeStatus;
        uint8_t qualificationStatus;
        uint8_t beginStatus;
        uint8_t completedFrames;
        uint8_t predictionsPass;
        uint8_t complete;
    };

    constexpr uint16_t recordCapacity = 6;
    constexpr uint32_t traceToken     = UINT32_C (0x4d4f544e);

    const adk::InertialSource copiedFixtureSource = {
        adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel::Synthetic,
        69,
        5,
        9,
        2000000,
        250000000};

    const adk::InertialRecordConfig recordConfig = {1, 1};

    const adk::InertialRecordQualificationConfig qualificationConfig = {
        1,
        recordConfig.schemaRevision,
        recordConfig.normalizationRevision,
        copiedFixtureSource,
        {adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
         adk::SignedAxis::PositiveZ},
        3,
        adk::Duration (25),
        adk::Duration (15),
        {0, 0, 1000000},
        {30000, 30000, 30000},
        {2000, 2000, 2000}};

    const adk::OrientationConfig orientationConfig = {{adk::SignedAxis::PositiveX,
                                                       adk::SignedAxis::PositiveY,
                                                       adk::SignedAxis::PositiveZ},
                                                      900000,
                                                      1100000,
                                                      2000,
                                                      5000,
                                                      60000};

    const adk::MotionRecorderConfig recorderConfig = {
        recordConfig.schemaRevision,
        recordConfig.normalizationRevision,
        qualificationConfig.qualificationRevision,
        1,
        recordCapacity,
        traceToken,
        adk::Duration (25),
        adk::Duration (4),
        copiedFixtureSource,
        orientationConfig};

    adk::InertialRecordNormalizer          recordNormalizer    (recordConfig);
    adk::InertialRecordQualificationPolicy qualificationPolicy (qualificationConfig);
    adk::QualifiedMotionRecorder           recorder            (recorderConfig);

    adk::MotionRecordImage recordImages[recordCapacity];
    adk::MotionRecorderResult recorderResult;

    volatile MotionResultCell motionResultCell;
    volatile ReplayResultCell replayResultCell;

    uint8_t  replayFrame;
    uint32_t suppliedNow;
    uint32_t qualificationDigest;

    // clang-format off
    adk::Status         acquireCopiedMotionFixture ();
    void                configureMotionReplay      ();
    adk::Status         startMotionReplay          ();

    adk::InertialSample copiedSample        (
        uint32_t sequence, uint32_t observedAt, int32_t rightMicroG,
        int32_t forwardMicroG, int32_t upMicroG);
    adk::Status         qualifyCopiedSource ();
    adk::Status         observeMotionRecord ();
    bool                decideMotionResult  (
        const adk::MotionRecorderResult& result) noexcept;
    void                actuateMotionIntent (
        const adk::MotionRecorderResult& result, bool prediction);
    void                finishMotionReplay (
        adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedMotionFixture ();

    configureMotionReplay ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        finishMotionReplay (fixtureStatus);
        return;
    }

    const adk::Status startStatus = startMotionReplay ();

    if (!startStatus.ok ())
    {
        finishMotionReplay (startStatus);
    }
}

void loop ()
{
    if (replayFrame >= recordCapacity)
    {
        return;
    }

    const adk::Status updateStatus = observeMotionRecord ();

    if (!updateStatus.ok ())
    {
        finishMotionReplay (updateStatus);
        return;
    }

    const adk::Status resultStatus = recorder.result (recorderResult);

    const bool prediction =
        resultStatus.ok () && decideMotionResult (recorderResult);

    actuateMotionIntent (recorderResult, prediction);
    ++replayResultCell.completedFrames;
    ++replayFrame;

    if (recorderResult.mode == adk::MotionRecorderMode::Complete ||
        replayFrame >= recordCapacity)
    {
        finishMotionReplay (resultStatus);
    }
}

namespace {

    adk::Status acquireCopiedMotionFixture ()
    {
        return copiedFixtureSource.sourceId != 0 &&
                       copiedFixtureSource.configurationRevision != 0 &&
                       copiedFixtureSource.calibrationRevision != 0
                   ? adk::StatusCode::Ok
                   : adk::StatusCode::InternalInvariant;
    }

    void configureMotionReplay ()
    {
        replayFrame                      = 0;
        suppliedNow                      = 200;
        qualificationDigest              = 0;
        replayResultCell                 = {};
        replayResultCell.predictionsPass = 1;

        for (uint8_t index = 0; index < recordCapacity; ++index)
        {
            recordImages[index]      = {};
        }
        motionResultCell = {};
    }

    adk::Status startMotionReplay ()
    {
        adk::Status status =
            qualificationPolicy.initialize (adk::TimePoint (90));
        if (status.ok ())
        {
            status = qualificationPolicy.begin (adk::TimePoint (90), 69001);
        }
        if (status.ok ())
        {
            status = recorder.initialize (adk::TimePoint (90), recordCapacity);
        }
        replayResultCell.initializeStatus = static_cast<uint8_t> (status.error ());

        if (!status.ok ())
        {
            return status;
        }

        status = qualifyCopiedSource ();

        replayResultCell.qualificationStatus = static_cast<uint8_t> (status.error ());

        if (!status.ok ())
        {
            return status;
        }

        status = recorder.begin (adk::TimePoint (195), 69001);

        replayResultCell.beginStatus = static_cast<uint8_t> (status.error ());

        return status;
    }

    adk::InertialSample copiedSample (uint32_t sequence, uint32_t observedAt,
                                      int32_t rightMicroG, int32_t forwardMicroG,
                                      int32_t upMicroG)
    {
        return {copiedFixtureSource,
                {rightMicroG, forwardMicroG, upMicroG},
                {20, -10, 30},
                adk::TimePoint (observedAt),
                sequence,
                true,
                adk::InertialSaturation::None,
                adk::StatusCode::Ok};
    }

    adk::Status qualifyCopiedSource ()
    {
        const adk::InertialSample stationarySamples[] = {
            copiedSample (1, 100, 8000, -6000, 999900),
            copiedSample (2, 110, -4000, 3000, 1000100),
            copiedSample (3, 120, 2000, -1000, 1000000)};

        for (uint8_t index = 0; index < 3; ++index)
        {
            adk::InertialRecord record = {};
            adk::Status status =
                recordNormalizer.normalize (stationarySamples[index], record);
            if (status.ok ())
            {
                status = qualificationPolicy.observe (
                    adk::TimePoint (105 + index * 10), record);
            }
            if (!status.ok ())
            {
                return status;
            }
        }

        adk::InertialQualificationEvidence& evidence =
            recorderResult.qualification;
        adk::Status status = qualificationPolicy.evidence (evidence);

        if (!status.ok ())
        {
            return status;
        }

        qualificationDigest = adk::motionQualificationDigest (
            recorderResult.qualification);

        return recorder.qualify (adk::TimePoint (125), evidence);
    }

    adk::Status observeMotionRecord ()
    {
        static const int32_t rightMicroG[recordCapacity] = {
            0, 0, 0, 500000, -500000, 0};
        static const int32_t forwardMicroG[recordCapacity] = {
            0, 500000, -500000, 0, 0, 0};
        static const int32_t upMicroG[recordCapacity] = {
            1000000, 866025, 866025, 866025, 866025, 1000000};

        suppliedNow += 5;
        const adk::InertialSample sample =
            copiedSample (4 + replayFrame, suppliedNow, rightMicroG[replayFrame],
                          forwardMicroG[replayFrame], upMicroG[replayFrame]);
        adk::InertialRecord record = {};
        adk::Status         status = recordNormalizer.normalize (sample, record);

        if (!status.ok ())
        {
            return status;
        }

        const adk::MotionRecorderControl control      = {
            copiedFixtureSource.sourceId,
            static_cast<uint32_t> (replayFrame + 4U),
            adk::TimePoint (suppliedNow),
            qualificationConfig.qualificationRevision,
            1,
            69001,
            qualificationDigest,
            traceToken,
            adk::MotionRecorderCommand::Advance,
            adk::StatusCode::Ok};

        return recorder.update (adk::TimePoint (suppliedNow), record, control,
                                recordImages, recordCapacity);
    }

    bool decideMotionResult (const adk::MotionRecorderResult& result) noexcept
    {
        const bool validTrace =
            result.sessionId == 69001 && result.recordCount == replayFrame + 1U &&
            result.recordCount <= result.recordCapacity &&
            result.latestRecord.source.sourceId == copiedFixtureSource.sourceId;
        const bool validPresentation =
            result.presentation.health == result.health &&
            result.presentation.token != adk::MotionDisplayToken::Fault &&
            result.status.ok ();
        return validTrace && validPresentation;
    }

    void actuateMotionIntent (const adk::MotionRecorderResult& result,
                              bool prediction)
    {
        volatile MotionResultCell& cell = motionResultCell;

        cell.mode              = static_cast<uint8_t> (result.mode);
        cell.health            = static_cast<uint8_t> (result.health);
        cell.scriptStep        = static_cast<uint8_t> (result.scriptStep);
        cell.displayToken      = static_cast<uint8_t> (result.presentation.token);
        cell.rgbRed            = result.presentation.rgbRed;
        cell.rgbGreen          = result.presentation.rgbGreen;
        cell.rgbBlue           = result.presentation.rgbBlue;
        cell.orientationValid  = result.presentation.orientationValid ? 1 : 0;
        cell.pitchTenthsDegree = result.presentation.pitchTenthsDegree;
        cell.rollTenthsDegree  = result.presentation.rollTenthsDegree;
        cell.exportRequested   = result.exportRequested ? 1 : 0;
        cell.status            = static_cast<uint8_t> (result.status.error ());
        cell.predictionPass    = prediction ? 1 : 0;

        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
    }

    void finishMotionReplay (adk::Status status)
    {
        replayFrame               = recordCapacity;
        replayResultCell.complete = status.ok () ? 1 : 0;
    }

} // namespace
