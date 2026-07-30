// E0 copied-record replay. This sketch owns no sensor, bus, endpoint, clock,
// interrupt, display, or storage medium. Its fixed fixtures and memory result
// cells do not qualify an MPU6050, QMI8658, or any powered adapter.
#include <Adk.h>
#include <inertial_record.h>

namespace {

    struct ReplayResultCell
    {
        uint8_t  state;
        uint8_t  sourceId;
        uint8_t  dataReady;
        uint8_t  saturation;
        uint8_t  producerStatus;
        uint8_t  normalizedStatus;
        uint8_t  codecValidity;
        uint8_t  roundTripPass;
        uint16_t encodedSize;
        uint32_t observedAt;
        uint32_t sequence;
        int32_t  accelerationXMicroG;
        int32_t  angularRateZMilliDegreesPerSecond;
    };

    constexpr adk::InertialRecordConfig recordConfig = {1, 1};

    const adk::InertialSource copiedFixtureSource = {
        adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel::Synthetic,
        67,
        4,
        9,
        2000000,
        250000000};

    const adk::InertialSample copiedFrames[] = {{copiedFixtureSource,
                                                 {12000, -8000, 999800},
                                                 {40, -30, 1250},
                                                 adk::TimePoint (100),
                                                 1,
                                                 true,
                                                 adk::InertialSaturation::None,
                                                 adk::StatusCode::Ok},
                                                {copiedFixtureSource,
                                                 {0, 0, 0},
                                                 {0, 0, 0},
                                                 adk::TimePoint (110),
                                                 2,
                                                 false,
                                                 adk::InertialSaturation::None,
                                                 adk::StatusCode::Ok},
                                                {copiedFixtureSource,
                                                 {0, 0, 0},
                                                 {0, 0, 0},
                                                 adk::TimePoint (120),
                                                 3,
                                                 false,
                                                 adk::InertialSaturation::None,
                                                 adk::StatusCode::HardwareFailure}};

    constexpr uint8_t copiedFrameCount =
        sizeof (copiedFrames) / sizeof (copiedFrames[0]);

    adk::InertialRecordNormalizer recordNormalizer (recordConfig);
    adk::InertialRecordCodec      recordCodec;

    uint8_t canonicalImages[copiedFrameCount][adk::InertialRecordCodec::size];

    volatile ReplayResultCell replayResultCells[copiedFrameCount];
    volatile uint8_t imageResultCells[copiedFrameCount][adk::InertialRecordCodec::size];
    volatile uint8_t replayCompleteCell;

    uint8_t replayIndex;
    bool    replayActive;

    adk::Status acquireCopiedFixtures   ();
    void        configureReplayResults  ();
    adk::Status startReplay             ();
    adk::Status observeCopiedFrame      (const adk::InertialSample& sample,
                                         adk::InertialRecord&       record);
    bool decideCanonicalRecord (const adk::InertialRecord& record, uint8_t* image,
                                uint16_t& encodedSize, adk::InertialRecord& decoded,
                                adk::InertialRecordValidity& validity);
    bool equalRecord (const adk::InertialRecord& left,
                      const adk::InertialRecord& right);
    void presentReplayResult (uint8_t index, const adk::InertialRecord& record,
                              const uint8_t* image, adk::Status normalizeStatus,
                              uint16_t                    encodedSize,
                              adk::InertialRecordValidity validity, bool roundTripPass);

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedFixtures ();

    configureReplayResults ();

    if (!fixtureStatus.ok ())
    {
        return;
    }

    startReplay ();
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const adk::InertialSample&  copiedFrame      = copiedFrames[replayIndex];
    adk::InertialRecord         normalizedRecord = {};
    adk::InertialRecord         decodedRecord    = {};
    uint16_t                    encodedSize      = 0;
    adk::InertialRecordValidity validity =
        adk::InertialRecordValidity::BadSemanticValue;

    const adk::Status normalizeStatus =
        observeCopiedFrame (copiedFrame, normalizedRecord);
    bool roundTripPass = false;

    if (normalizeStatus.ok ())
    {
        roundTripPass =
            decideCanonicalRecord (normalizedRecord, canonicalImages[replayIndex],
                                   encodedSize, decodedRecord, validity);
    }

    presentReplayResult (replayIndex, normalizedRecord, canonicalImages[replayIndex],
                         normalizeStatus, encodedSize, validity, roundTripPass);

    ++replayIndex;
    replayActive       = replayIndex < copiedFrameCount;
    replayCompleteCell = replayActive ? 0 : 1;
}

namespace {

    adk::Status acquireCopiedFixtures ()
    {
        return copiedFixtureSource.sourceId != 0 &&
                       copiedFixtureSource.configurationRevision != 0 &&
                       copiedFixtureSource.calibrationRevision != 0
                   ? adk::StatusCode::Ok
                   : adk::StatusCode::InternalInvariant;
    }

    void configureReplayResults ()
    {
        replayIndex        = 0;
        replayActive       = false;
        replayCompleteCell = 0;

        for (uint8_t index = 0; index < copiedFrameCount; ++index)
        {
            replayResultCells[index].state                             = 0xff;
            replayResultCells[index].sourceId                          = 0xff;
            replayResultCells[index].dataReady                         = 0xff;
            replayResultCells[index].saturation                        = 0xff;
            replayResultCells[index].producerStatus                    = 0xff;
            replayResultCells[index].normalizedStatus                  = 0xff;
            replayResultCells[index].codecValidity                     = 0xff;
            replayResultCells[index].roundTripPass                     = 0;
            replayResultCells[index].encodedSize                       = 0;
            replayResultCells[index].observedAt                        = 0;
            replayResultCells[index].sequence                          = 0;
            replayResultCells[index].accelerationXMicroG               = 0;
            replayResultCells[index].angularRateZMilliDegreesPerSecond = 0;

            for (uint8_t offset = 0; offset < adk::InertialRecordCodec::size; ++offset)
            {
                canonicalImages[index][offset]  = 0;
                imageResultCells[index][offset] = 0;
            }
        }
    }

    adk::Status startReplay ()
    {
        replayActive = true;
        return adk::StatusCode::Ok;
    }

    adk::Status observeCopiedFrame (const adk::InertialSample& sample,
                                    adk::InertialRecord&       record)
    {
        return recordNormalizer.normalize (sample, record);
    }

    bool decideCanonicalRecord (const adk::InertialRecord& record, uint8_t* image,
                                uint16_t& encodedSize, adk::InertialRecord& decoded,
                                adk::InertialRecordValidity& validity)
    {
        const adk::Result<uint16_t> encoded =
            recordCodec.encode (record, {image, adk::InertialRecordCodec::size});

        if (!encoded.ok ())
        {
            return false;
        }

        encodedSize = encoded.value ();
        validity =
            recordCodec.decode ({image, adk::InertialRecordCodec::size}, decoded);

        return validity == adk::InertialRecordValidity::Valid &&
               equalRecord (record, decoded);
    }

    bool equalRecord (const adk::InertialRecord& left, const adk::InertialRecord& right)
    {
        return left.schemaRevision == right.schemaRevision &&
               left.normalizationRevision == right.normalizationRevision &&
               left.source.kind == right.source.kind &&
               left.source.model == right.source.model &&
               left.source.sourceId == right.source.sourceId &&
               left.source.configurationRevision ==
                   right.source.configurationRevision &&
               left.source.calibrationRevision == right.source.calibrationRevision &&
               left.source.accelerationRangeMicroG ==
                   right.source.accelerationRangeMicroG &&
               left.source.angularRateRangeMilliDegreesPerSecond ==
                   right.source.angularRateRangeMilliDegreesPerSecond &&
               left.accelerationMicroG.x == right.accelerationMicroG.x &&
               left.accelerationMicroG.y == right.accelerationMicroG.y &&
               left.accelerationMicroG.z == right.accelerationMicroG.z &&
               left.angularRateMilliDegreesPerSecond.x ==
                   right.angularRateMilliDegreesPerSecond.x &&
               left.angularRateMilliDegreesPerSecond.y ==
                   right.angularRateMilliDegreesPerSecond.y &&
               left.angularRateMilliDegreesPerSecond.z ==
                   right.angularRateMilliDegreesPerSecond.z &&
               left.observedAt == right.observedAt && left.sequence == right.sequence &&
               left.dataReady == right.dataReady &&
               left.saturation == right.saturation &&
               left.producerStatus == right.producerStatus && left.state == right.state;
    }

    void presentReplayResult (uint8_t index, const adk::InertialRecord& record,
                              const uint8_t* image, adk::Status normalizeStatus,
                              uint16_t                    encodedSize,
                              adk::InertialRecordValidity validity, bool roundTripPass)
    {
        replayResultCells[index].state      = static_cast<uint8_t> (record.state);
        replayResultCells[index].sourceId   = record.source.sourceId;
        replayResultCells[index].dataReady  = record.dataReady ? 1 : 0;
        replayResultCells[index].saturation = static_cast<uint8_t> (record.saturation);
        replayResultCells[index].producerStatus =
            static_cast<uint8_t> (record.producerStatus.error ());
        replayResultCells[index].normalizedStatus =
            static_cast<uint8_t> (normalizeStatus.error ());
        replayResultCells[index].codecValidity = static_cast<uint8_t> (validity);
        replayResultCells[index].roundTripPass = roundTripPass ? 1 : 0;
        replayResultCells[index].encodedSize   = encodedSize;
        replayResultCells[index].observedAt    = record.observedAt.milliseconds ();
        replayResultCells[index].sequence      = record.sequence;
        replayResultCells[index].accelerationXMicroG = record.accelerationMicroG.x;
        replayResultCells[index].angularRateZMilliDegreesPerSecond =
            record.angularRateMilliDegreesPerSecond.z;

        if (roundTripPass)
        {
            for (uint8_t offset = 0; offset < adk::InertialRecordCodec::size; ++offset)
            {
                imageResultCells[index][offset] = image[offset];
            }
        }
    }

} // namespace
