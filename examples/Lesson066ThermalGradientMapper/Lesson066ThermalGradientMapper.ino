// E0 thermal-gradient fixture. This sketch replays copied qualified probe
// sets, copied controls, and supplied time into named memory result cells. It
// owns no probe, bus, clock, display, LED, storage, pin, or powered circuit.
#include <Adk.h>
#include <thermal_gradient_mapper.h>

namespace {

    enum struct ReplayStage : uint8_t
    {
        MixedGradient,
        MiddleProbeFault,
        Recovered,
        NextPage,
        Record,
        Shutdown,
        Complete
    };

    struct PageResultCell
    {
        uint8_t health;
        uint8_t pageKind;
        uint8_t pageIndex;
        uint8_t selectedSlot;
        uint8_t probeCount;
        uint8_t gradientCount;
        uint8_t faultMask;
        uint8_t ledSelectionMask;
        uint8_t outputsInactive;
        uint8_t status;
        uint8_t predictionPass;
    };

    struct GradientResultCell
    {
        int32_t lowerRawSixteenths;
        int32_t upperRawSixteenths;
        uint8_t quality;
        uint8_t faultMask;
    };

    struct RecordResultCell
    {
        uint32_t sequence;
        uint32_t setCycleSequence;
        uint32_t witnessDigest;
        uint8_t  probeCount;
        uint8_t  gradientCount;
        uint8_t  health;
        uint8_t  faultMask;
        uint8_t  present;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t initializeStatus;
        uint8_t completedStages;
        uint8_t predictionsPass;
        uint8_t complete;
    };

    constexpr uint8_t  setSourceId       = 65;
    constexpr uint8_t  controlSourceId   = 66;
    constexpr uint16_t sourceRevision    = 1;
    constexpr uint32_t mapperOwnerToken  = 0x54484d50UL;
    constexpr uint32_t oneWireOwnerToken = UINT32_C (650065);
    constexpr uint16_t oneWireRevision   = 64;
    constexpr uint8_t  receiptSourceId   = 7;

    const adk::OneWireRomCode probeRoms[4] = {
        {{0x28, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29}},
        {{0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70}},
        {{0x28, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47}},
        {{0x28, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc2}}};

    const adk::ThermalMapperConfig mapperConfig = {
        mapperOwnerToken,
        sourceRevision,
        setSourceId,
        sourceRevision,
        {probeRoms[0], probeRoms[1], probeRoms[2], probeRoms[3]},
        {probeRoms[0], probeRoms[1], probeRoms[2], probeRoms[3]},
        4,
        mapperOwnerToken,
        controlSourceId,
        sourceRevision,
        adk::Duration (20),
        8};

    adk::OneWireTransactionPolicy
        fixtureOneWirePolicy ({oneWireOwnerToken,
                               oneWireRevision,
                               receiptSourceId,
                               oneWireRevision,
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

    adk::Qualified18B20ProbeSetPolicy fixtureProbeSetPolicy (
        {setSourceId,
         sourceRevision,
         oneWireOwnerToken,
         oneWireRevision,
         {{probeRoms[0], adk::Ds18b20Resolution::Bits9, adk::Duration (94),
           adk::Duration (1000), -880, 2000, 160},
          {probeRoms[1], adk::Ds18b20Resolution::Bits10, adk::Duration (188),
           adk::Duration (1000), -880, 2000, 160},
          {probeRoms[2], adk::Ds18b20Resolution::Bits11, adk::Duration (375),
           adk::Duration (1000), -880, 2000, 160},
          {probeRoms[3], adk::Ds18b20Resolution::Bits12, adk::Duration (750),
           adk::Duration (1000), -880, 2000, 160}}});
    adk::Ds18b20CycleBuilder   fixtureBuilder;
    adk::ThermalGradientMapper fixtureMapper (mapperConfig);

    volatile PageResultCell     pageResultCell;
    volatile GradientResultCell gradientResultCells[3];
    volatile RecordResultCell   recordResultCell;
    volatile ReplayResultCell   replayResultCell;

    ReplayStage replayStage;
    uint32_t    suppliedNow;
    uint32_t    probeObservedAt;
    uint32_t    cycleSequence;
    uint32_t    controlSequence;

    // clang-format off
    adk::Status acquireCopiedThermalFixture ();
    void        configureThermalReplay      ();
    adk::Status startThermalMapper          ();

    adk::QualifiedDs18b20Snapshot observeQualifiedProbeSet (
        ReplayStage stage);
    adk::ThermalMapperControl observeMapperControl (ReplayStage stage);

    adk::Status updateThermalMap ();
    bool decideMapperResult      (
        ReplayStage stage, const adk::ThermalMapperResult& result);
    void actuateMapperIntent (
        const adk::ThermalMapperResult& result, bool prediction);
    void finishThermalReplay (adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedThermalFixture ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());
    if (!fixtureStatus.ok                                                      ())
    {
        finishThermalReplay (fixtureStatus);
        return;
    }

    configureThermalReplay ();

    const adk::Status initializeStatus = startThermalMapper ();
    replayResultCell.initializeStatus =
        static_cast<uint8_t> (initializeStatus.error ());
    if (!initializeStatus.ok ())
    {
        finishThermalReplay (initializeStatus);
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
        const adk::Status shutdownStatus = fixtureMapper.shutdown ();

        adk::ThermalGradientIntent inactiveIntent;
        const adk::Status snapshotStatus = fixtureMapper.snapshot (inactiveIntent);
        const bool        prediction = shutdownStatus.ok          () && snapshotStatus.ok () &&
                                       !fixtureMapper.initialized () &&
                                       inactiveIntent.outputsInactive;

        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
        finishThermalReplay (shutdownStatus);
        return;
    }

    ++controlSequence;
    suppliedNow += 5;
    if (replayStage == ReplayStage::MixedGradient ||
        replayStage == ReplayStage::MiddleProbeFault ||
        replayStage == ReplayStage::Recovered)
    {
        ++cycleSequence;
        probeObservedAt = suppliedNow;
    }

    const adk::Status updateStatus = updateThermalMap ();
    if (!updateStatus.ok                              ())
    {
        finishThermalReplay (updateStatus);
        return;
    }
    ++replayResultCell.completedStages;
    replayStage = static_cast<ReplayStage> (static_cast<uint8_t> (replayStage) + 1U);
}

namespace {

    adk::Status acquireCopiedThermalFixture ()
    {
        for (uint8_t left = 0; left < 4; ++left)
        {
            for (uint8_t right = static_cast<uint8_t> (left + 1U); right < 4; ++right)
            {
                bool equal = true;
                for (uint8_t byte = 0; byte < 8; ++byte)
                {
                    equal = equal &&
                            probeRoms[left].bytes[byte] == probeRoms[right].bytes[byte];
                }
                if (equal)
                {
                    return adk::StatusCode::InvalidConfiguration;
                }
            }
        }
        return adk::StatusCode::Ok;
    }

    void configureThermalReplay ()
    {
        replayStage                      = ReplayStage::MixedGradient;
        suppliedNow                      = UINT32_MAX - 25UL;
        probeObservedAt                  = suppliedNow;
        cycleSequence                    = 0;
        controlSequence                  = 0;
        replayResultCell.completedStages = 0;
        replayResultCell.predictionsPass = 1;
        replayResultCell.complete        = 0;
        recordResultCell.present         = 0;
    }

    adk::Status startThermalMapper ()
    {
        adk::OneWireStepIntent releaseIntent;
        adk::Status            status = fixtureOneWirePolicy.initialize (
            adk::MicrosecondTimePoint (0), releaseIntent);
        if (!status.ok ())
        {
            return status;
        }

        status = fixtureProbeSetPolicy.initialize ();
        if (!status.ok                            ())
        {
            return status;
        }

        return fixtureMapper.initialize (adk::TimePoint (suppliedNow));
    }

    adk::QualifiedDs18b20Snapshot observeQualifiedProbeSet (ReplayStage stage)
    {
        adk::QualifiedDs18b20Snapshot snapshot;

        snapshot.sourceId              = setSourceId;
        snapshot.configurationRevision = sourceRevision;
        snapshot.cycleSequence         = cycleSequence;
        snapshot.observedAt            = adk::TimePoint (probeObservedAt);
        snapshot.validCount            = 4;
        snapshot.presentMask           = 0x0f;
        snapshot.faultMask             = 0;
        snapshot.quality               = adk::Ds18b20SetQuality::Complete;
        snapshot.status                = adk::StatusCode::Ok;

        const int16_t rawValues[4] = {320, 336, 328, 368};
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::QualifiedDs18b20Probe& probe = snapshot.probes[index];

            probe.rom                       = probeRoms[index];
            probe.cycleSequence             = cycleSequence;
            probe.conversionGeneration      = cycleSequence * 4U + index + 1U;
            probe.readTransactionGeneration = cycleSequence * 4U + index + 101U;
            probe.observedAt                = adk::TimePoint (probeObservedAt);
            probe.freshThrough              = adk::TimePoint (probeObservedAt + 10U);
            probe.rawSixteenths             = rawValues[index];
            probe.lowerRawSixteenths        = rawValues[index];
            probe.upperRawSixteenths        = rawValues[index];
            probe.resolution                = adk::Ds18b20Resolution::Bits12;
            probe.quality                   = adk::Ds18b20ProbeQuality::Current;
            probe.age                       = adk::Duration (0);
            probe.status                    = adk::StatusCode::Ok;
        }

        if (stage == ReplayStage::MiddleProbeFault)
        {
            adk::QualifiedDs18b20Probe& middle = snapshot.probes[1];
            middle.quality                     = adk::Ds18b20ProbeQuality::Missing;
            middle.status                      = adk::StatusCode::Ok;
            snapshot.validCount                = 3;
            snapshot.presentMask               = 0x0d;
            snapshot.faultMask                 = 0x02;
            snapshot.quality                   = adk::Ds18b20SetQuality::Missing;
        }

        return snapshot;
    }

    adk::ThermalMapperControl observeMapperControl (ReplayStage stage)
    {
        adk::ThermalMapperControl control;

        control.ownerToken            = mapperOwnerToken;
        control.sourceId              = controlSourceId;
        control.configurationRevision = sourceRevision;
        control.sequence              = controlSequence;
        control.observedAt            = adk::TimePoint (suppliedNow);
        control.nextEdge              = stage == ReplayStage::NextPage;
        control.recordEdge            = stage == ReplayStage::Record;
        control.status                = adk::StatusCode::Ok;
        return control;
    }

    adk::Status updateThermalMap ()
    {
        const adk::ThermalMapperEnvelope envelope = {
            adk::TimePoint       (suppliedNow), observeQualifiedProbeSet (replayStage),
            observeMapperControl (replayStage)};
        adk::ThermalMapperResult result;

        const adk::Status status = fixtureMapper.update (envelope, result);
        result.status            = status;
        if (!status.ok ())
        {
            return status;
        }

        const bool prediction = decideMapperResult (replayStage, result);
        actuateMapperIntent                        (result, prediction);
        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
        return adk::StatusCode::Ok;
    }

    bool decideMapperResult (ReplayStage stage, const adk::ThermalMapperResult& result)
    {
        if (!result.status.ok () || result.intent.probeCount != 4 ||
            result.intent.gradientCount != 3)
        {
            return false;
        }

        if (stage == ReplayStage::MiddleProbeFault)
        {
            return result.intent.health == adk::ThermalGradientHealth::Fault &&
                   result.intent.overallFaultMask == 0x02 &&
                   result.intent.gradients[0].quality ==
                       adk::ThermalGradientQuality::Fault &&
                   result.intent.gradients[1].quality ==
                       adk::ThermalGradientQuality::Fault;
        }

        if (stage == ReplayStage::Record)
        {
            return result.hasRecord && result.record.probeCount == 4 &&
                   result.record.gradientCount == 3 &&
                   result.record.setCycleSequence == cycleSequence;
        }

        return result.intent.overallFaultMask == 0 &&
               result.intent.health != adk::ThermalGradientHealth::Fault &&
               !result.hasRecord;
    }

    void actuateMapperIntent (const adk::ThermalMapperResult& result, bool prediction)
    {
        pageResultCell.health           = static_cast<uint8_t> (result.intent.health);
        pageResultCell.pageKind         = static_cast<uint8_t> (result.intent.pageKind);
        pageResultCell.pageIndex        = result.intent.pageIndex;
        pageResultCell.selectedSlot     = result.intent.selectedSlot;
        pageResultCell.probeCount       = result.intent.probeCount;
        pageResultCell.gradientCount    = result.intent.gradientCount;
        pageResultCell.faultMask        = result.intent.overallFaultMask;
        pageResultCell.ledSelectionMask = result.intent.ledSelectionMask;
        pageResultCell.outputsInactive  = result.intent.outputsInactive ? 1 : 0;
        pageResultCell.status           = static_cast<uint8_t> (result.status.error ());
        pageResultCell.predictionPass   = prediction ? 1 : 0;

        for (uint8_t index = 0; index < 3; ++index)
        {
            gradientResultCells[index].lowerRawSixteenths =
                result.intent.gradients[index].lowerRawSixteenths;
            gradientResultCells[index].upperRawSixteenths =
                result.intent.gradients[index].upperRawSixteenths;
            gradientResultCells[index].quality =
                static_cast<uint8_t> (result.intent.gradients[index].quality);
            gradientResultCells[index].faultMask =
                result.intent.gradients[index].faultMask;
        }

        if (result.hasRecord)
        {
            recordResultCell.sequence         = result.record.recordSequence;
            recordResultCell.setCycleSequence = result.record.setCycleSequence;
            recordResultCell.witnessDigest    = result.record.witnessDigest;
            recordResultCell.probeCount       = result.record.probeCount;
            recordResultCell.gradientCount    = result.record.gradientCount;
            recordResultCell.health    = static_cast<uint8_t> (result.record.health);
            recordResultCell.faultMask = result.record.faultMask;
            recordResultCell.present   = 1;
        }
    }

    void finishThermalReplay (adk::Status status)
    {
        pageResultCell.status     = static_cast<uint8_t> (status.error ());
        replayResultCell.complete = 1;
        replayStage               = ReplayStage::Complete;
    }

} // namespace
