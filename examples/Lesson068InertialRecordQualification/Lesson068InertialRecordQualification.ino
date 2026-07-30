// E0 copied-record replay. This sketch owns no sensor, bus, endpoint, clock,
// interrupt, display, or storage medium. Its memory result cells qualify only
// the configured synthetic record stream, never a powered adapter.
#include <Adk.h>
#include <inertial_record.h>
#include <inertial_record_qualification.h>

namespace {

    struct QualificationResultCell
    {
        uint32_t attemptId;
        uint32_t lifecycleGeneration;
        uint32_t firstSequence;
        uint32_t lastSequence;
        int64_t  accelerationSumZMicroG;
        int32_t  meanAccelerationZMicroG;
        int32_t  meanAngularRateZMilliDegreesPerSecond;
        uint8_t  state;
        uint8_t  reason;
        uint8_t  acceptedSampleCount;
        uint8_t  terminalSourceId;
        uint8_t  qualificationFrameZ;
        uint8_t  operationStatus;
        uint8_t  evidenceStatus;
        uint8_t  predictionPass;
    };

    constexpr adk::InertialRecordConfig recordConfig = {1, 1};

    const adk::InertialSource copiedFixtureSource = {
        adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel::Synthetic,
        68,
        4,
        9,
        2000000,
        250000000};

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

    const adk::InertialSample copiedStationaryFrames[] = {
        {copiedFixtureSource,
         {8000, -6000, 999900},
         {40, -30, 120},
         adk::TimePoint (100),
         1,
         true,
         adk::InertialSaturation::None,
         adk::StatusCode::Ok},
        {copiedFixtureSource,
         {-4000, 3000, 1000100},
         {-20, 10, -80},
         adk::TimePoint (110),
         2,
         true,
         adk::InertialSaturation::None,
         adk::StatusCode::Ok},
        {copiedFixtureSource,
         {2000, -1000, 1000000},
         {10, -10, 20},
         adk::TimePoint (120),
         3,
         true,
         adk::InertialSaturation::None,
         adk::StatusCode::Ok}};

    const adk::InertialSample copiedFaultFrame = {copiedFixtureSource,
                                                  {0, 0, 0},
                                                  {0, 0, 0},
                                                  adk::TimePoint (200),
                                                  10,
                                                  false,
                                                  adk::InertialSaturation::None,
                                                  adk::StatusCode::HardwareFailure};

    constexpr uint8_t stationaryFrameCount =
        sizeof (copiedStationaryFrames) / sizeof (copiedStationaryFrames[0]);

    adk::InertialRecordNormalizer          recordNormalizer    (recordConfig);
    adk::InertialRecordQualificationPolicy qualificationPolicy (qualificationConfig);

    volatile QualificationResultCell qualificationResultCells[2];
    volatile uint8_t                 replayCompleteCell;

    uint8_t stationaryFrameIndex;
    uint8_t replayStage;
    bool    replayActive;

    adk::Status acquireCopiedRecordFixtures   ();
    void        configureQualificationResults ();
    adk::Status startQualificationReplay      ();
    adk::Status observeCopiedSample           (adk::TimePoint             now,
                                               const adk::InertialSample& sample);
    bool        decideQualified              (
        const adk::InertialQualificationEvidence& evidence);
    bool decideFaultRejected                 (
        const adk::InertialQualificationEvidence& evidence);
    void presentQualificationResult          (
        uint8_t index, const adk::InertialQualificationEvidence& evidence,
        adk::Status operationStatus, bool prediction);
    void finishReplay                        (adk::Status status);

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedRecordFixtures ();

    configureQualificationResults ();

    if (!fixtureStatus.ok ())
    {
        finishReplay (fixtureStatus);
        return;
    }

    const adk::Status startStatus = startQualificationReplay ();

    if (!startStatus.ok ())
    {
        finishReplay (startStatus);
    }
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    if (replayStage == 0)
    {
        const adk::InertialSample& sample =
            copiedStationaryFrames[stationaryFrameIndex];
        const adk::Status observeStatus = observeCopiedSample (
            adk::TimePoint (105 + stationaryFrameIndex * 10), sample);
        ++stationaryFrameIndex;

        if (!observeStatus.ok ())
        {
            finishReplay (observeStatus);
            return;
        }
        if (stationaryFrameIndex < stationaryFrameCount)
        {
            return;
        }

        adk::InertialQualificationEvidence evidence = {};
        const adk::Status evidenceStatus = qualificationPolicy.evidence (evidence);

        presentQualificationResult (0, evidence, evidenceStatus,
                                    evidenceStatus.ok () && decideQualified (evidence));

        const adk::Status resetStatus =
            qualificationPolicy.reset (adk::TimePoint (190));
        const adk::Status beginStatus =
            resetStatus.ok () ? qualificationPolicy.begin (adk::TimePoint (190), 68002)
                              : resetStatus;
        if (!beginStatus.ok ())
        {
            finishReplay (beginStatus);
            return;
        }
        replayStage = 1;
        return;
    }

    const adk::Status observeStatus =
        observeCopiedSample (adk::TimePoint (205), copiedFaultFrame);
    adk::InertialQualificationEvidence evidence = {};
    const adk::Status evidenceStatus = qualificationPolicy.evidence (evidence);

    const bool prediction = observeStatus.ok () && evidenceStatus.ok () &&
                            decideFaultRejected (evidence);
    presentQualificationResult (
        1, evidence, observeStatus.ok () ? evidenceStatus : observeStatus,
        prediction);
    finishReplay (observeStatus.ok () ? evidenceStatus : observeStatus);
}

namespace {

    adk::Status acquireCopiedRecordFixtures ()
    {
        return copiedFixtureSource.sourceId != 0 &&
                       copiedFixtureSource.configurationRevision != 0 &&
                       copiedFixtureSource.calibrationRevision != 0
                   ? adk::StatusCode::Ok
                   : adk::StatusCode::InternalInvariant;
    }

    void configureQualificationResults ()
    {
        stationaryFrameIndex = 0;
        replayStage          = 0;
        replayActive         = false;
        replayCompleteCell   = 0;

        for (uint8_t index = 0; index < 2; ++index)
        {
            qualificationResultCells[index] = {};
        }
    }

    adk::Status startQualificationReplay ()
    {
        const adk::Status initializeStatus =
            qualificationPolicy.initialize (adk::TimePoint (90));
        if (!initializeStatus.ok ())
        {
            return initializeStatus;
        }

        const adk::Status beginStatus =
            qualificationPolicy.begin (adk::TimePoint (90), 68001);
        replayActive = beginStatus.ok ();
        return beginStatus;
    }

    adk::Status observeCopiedSample (adk::TimePoint             now,
                                     const adk::InertialSample& sample)
    {
        adk::InertialRecord record        = {};
        const adk::Status normalizeStatus = recordNormalizer.normalize (sample, record);

        return normalizeStatus.ok () ? qualificationPolicy.observe (now, record)
                                     : normalizeStatus;
    }

    bool decideQualified (const adk::InertialQualificationEvidence& evidence)
    {
        return evidence.attemptId == 68001 &&
               evidence.lifecycleGeneration == 1 &&
               evidence.state == adk::InertialQualificationState::Qualified &&
               evidence.reason == adk::InertialQualificationReason::None &&
               evidence.acceptedSampleCount == stationaryFrameCount &&
               evidence.firstSequence == 1 && evidence.lastSequence == 3 &&
               evidence.accelerationSumsMicroG.z == 3000000 &&
               evidence.sourceToQualificationFrame.z ==
                   adk::SignedAxis::PositiveZ &&
               evidence.mappedRecord.source.sourceId == copiedFixtureSource.sourceId;
    }

    bool decideFaultRejected (const adk::InertialQualificationEvidence& evidence)
    {
        return evidence.attemptId == 68002 &&
               evidence.lifecycleGeneration == 2 &&
               evidence.state == adk::InertialQualificationState::Rejected &&
               evidence.reason == adk::InertialQualificationReason::ProducerFault &&
               evidence.acceptedSampleCount == 0 &&
               evidence.terminalRecord.source.sourceId == copiedFixtureSource.sourceId;
    }

    void presentQualificationResult (uint8_t                                   index,
                                     const adk::InertialQualificationEvidence& evidence,
                                     adk::Status operationStatus, bool prediction)
    {
        qualificationResultCells[index].attemptId = evidence.attemptId;
        qualificationResultCells[index].lifecycleGeneration =
            evidence.lifecycleGeneration;
        qualificationResultCells[index].firstSequence = evidence.firstSequence;
        qualificationResultCells[index].lastSequence  = evidence.lastSequence;
        qualificationResultCells[index].accelerationSumZMicroG =
            evidence.accelerationSumsMicroG.z;
        qualificationResultCells[index].meanAccelerationZMicroG =
            evidence.meanAccelerationMicroG.z;
        qualificationResultCells[index].meanAngularRateZMilliDegreesPerSecond =
            evidence.meanAngularRateMilliDegreesPerSecond.z;
        qualificationResultCells[index].state  = static_cast<uint8_t> (evidence.state);
        qualificationResultCells[index].reason = static_cast<uint8_t> (evidence.reason);
        qualificationResultCells[index].acceptedSampleCount =
            evidence.acceptedSampleCount;
        qualificationResultCells[index].terminalSourceId =
            evidence.terminalRecord.source.sourceId;
        qualificationResultCells[index].qualificationFrameZ =
            static_cast<uint8_t> (evidence.sourceToQualificationFrame.z);
        qualificationResultCells[index].operationStatus =
            static_cast<uint8_t> (operationStatus.error ());
        qualificationResultCells[index].evidenceStatus =
            static_cast<uint8_t> (evidence.status.error ());
        qualificationResultCells[index].predictionPass = prediction ? 1 : 0;
    }

    void finishReplay (adk::Status status)
    {
        replayActive       = false;
        replayCompleteCell = status.ok () ? 1 : 2;
    }

} // namespace
