// E0 copied-descriptor replay. This sketch owns no module, pin, ADC,
// comparator, rail, timer, or powered circuit. An accepted declaration is
// software evidence only; it is not permission to connect or energize a board.
#include <Adk.h>
#include <module_threshold_descriptor.h>

namespace {

    enum struct DescriptorDisposition : uint8_t
    {
        AcceptedComplete,
        AcceptedIncomplete,
        RejectedDescriptor,
        RejectedFrame
    };

    struct CopiedDescriptorFixture
    {
        adk::ModuleThresholdDescriptor descriptor;
        adk::ModuleThresholdFrame      frame;
        DescriptorDisposition          expectedDisposition;
    };

    struct DescriptorObservation
    {
        adk::Status descriptorStatus;
        adk::Status frameStatus;
        adk::Status declarationsStatus;
        adk::Status assertionStatus;
        bool        declarationsComplete;
        bool        comparatorAsserted;
    };

    struct DescriptorResultCell
    {
        uint32_t descriptorId;
        uint32_t specimenReference;
        uint32_t sequence;
        uint16_t descriptorRevision;
        uint16_t specimenRevision;
        uint16_t electricalEvidenceRevision;
        uint16_t sourceConfigurationRevision;
        uint16_t rawMinimum;
        uint16_t rawMaximum;
        uint16_t analogRaw;
        uint16_t warmupMilliseconds;
        uint16_t settlingMilliseconds;
        uint8_t  topology;
        uint8_t  outputStage;
        uint8_t  comparatorPolarity;
        uint8_t  pullRequirement;
        uint8_t  pullRail;
        uint8_t  thresholdDirection;
        uint8_t  sourceId;
        uint8_t  analogStatus;
        uint8_t  comparatorStatus;
        uint8_t  analogProducerStatus;
        uint8_t  comparatorProducerStatus;
        uint8_t  declaredWarmupSatisfied;
        uint8_t  declaredSettlingSatisfied;
        uint8_t  descriptorStatus;
        uint8_t  frameStatus;
        uint8_t  assertionStatus;
        uint8_t  declarationsComplete;
        uint8_t  comparatorAsserted;
        uint8_t  disposition;
        uint8_t  predictionPass;
    };

    const CopiedDescriptorFixture copiedFixtures[] = {
        {{1,
          7001,
          1,
          700001,
          1,
          1,
          adk::ModuleChannelTopology::AnalogAndComparator,
          adk::ModuleComparatorOutputStage::PushPull,
          adk::ModulePullRequirement::None,
          adk::ModuleDeclaredRail::Unspecified,
          {3000, 5000},
          {0, 5000},
          {0, 1023},
          adk::ModuleComparatorPolarity::ActiveHigh,
          adk::ModuleThresholdControlKind::Potentiometer,
          adk::ModuleThresholdDirection::IncreasingClockwise,
          {adk::ModuleDurationDeclaration::Known, adk::Duration (250)},
          {adk::ModuleDurationDeclaration::Known, adk::Duration (10)}},
         {1,
          7001,
          1,
          700001,
          1,
          1,
          {70, 1, 1, adk::TimePoint (100)},
          512,
          adk::ModuleChannelStatus::Current,
          true,
          adk::ModuleChannelStatus::Current,
          true,
          true,
          true,
          true,
          adk::StatusCode::Ok,
          adk::StatusCode::Ok},
         DescriptorDisposition::AcceptedComplete},
        {{1,
          7002,
          3,
          700002,
          2,
          4,
          adk::ModuleChannelTopology::AnalogAndComparator,
          adk::ModuleComparatorOutputStage::OpenCollector,
          adk::ModulePullRequirement::PullUp,
          adk::ModuleDeclaredRail::LogicSupply,
          {3000, 3300},
          {0, 3300},
          {100, 900},
          adk::ModuleComparatorPolarity::ActiveLow,
          adk::ModuleThresholdControlKind::Fixed,
          adk::ModuleThresholdDirection::Unspecified,
          {adk::ModuleDurationDeclaration::Unknown, adk::Duration (0)},
          {adk::ModuleDurationDeclaration::Known,   adk::Duration (5)}},
         {1,
          7002,
          3,
          700002,
          2,
          4,
          {70, 2, 2, adk::TimePoint (200)},
          400,
          adk::ModuleChannelStatus::Current,
          false,
          adk::ModuleChannelStatus::Current,
          true,
          true,
          true,
          true,
          adk::StatusCode::Ok,
          adk::StatusCode::Ok},
         DescriptorDisposition::RejectedFrame},
        {{1,
          7003,
          0,
          700003,
          1,
          1,
          adk::ModuleChannelTopology::AnalogOnly,
          adk::ModuleComparatorOutputStage::Unspecified,
          adk::ModulePullRequirement::Unspecified,
          adk::ModuleDeclaredRail::Unspecified,
          {3000, 5000},
          {0, 5000},
          {0, 1023},
          adk::ModuleComparatorPolarity::Unspecified,
          adk::ModuleThresholdControlKind::Unspecified,
          adk::ModuleThresholdDirection::Unspecified,
          {adk::ModuleDurationDeclaration::Known, adk::Duration (0)},
          {adk::ModuleDurationDeclaration::Known, adk::Duration (0)}},
         {1,
          7003,
          0,
          700003,
          1,
          1,
          {70, 3, 3, adk::TimePoint (300)},
          256,
          adk::ModuleChannelStatus::Current,
          false,
          adk::ModuleChannelStatus::NotPresent,
          false,
          false,
          true,
          true,
          adk::StatusCode::Ok,
          adk::StatusCode::Ok},
         DescriptorDisposition::RejectedDescriptor},
        {{1,
          7004,
          2,
          700004,
          1,
          3,
          adk::ModuleChannelTopology::ComparatorOnly,
          adk::ModuleComparatorOutputStage::OpenDrain,
          adk::ModulePullRequirement::PullUp,
          adk::ModuleDeclaredRail::LogicSupply,
          {3000, 3300},
          {0, 3300},
          {0, 1023},
          adk::ModuleComparatorPolarity::ActiveLow,
          adk::ModuleThresholdControlKind::Fixed,
          adk::ModuleThresholdDirection::Unspecified,
          {adk::ModuleDurationDeclaration::Known, adk::Duration (20)},
          {adk::ModuleDurationDeclaration::Known, adk::Duration (4)}},
         {1,
          7004,
          2,
          700004,
          1,
          3,
          {71, 2, 4, adk::TimePoint (400)},
          0,
          adk::ModuleChannelStatus::NotPresent,
          false,
          adk::ModuleChannelStatus::ProducerFault,
          true,
          true,
          true,
          true,
          adk::StatusCode::Ok,
          adk::StatusCode::HardwareFailure},
         DescriptorDisposition::AcceptedComplete},
        {{1,
          7005,
          1,
          700005,
          2,
          2,
          adk::ModuleChannelTopology::ComparatorOnly,
          adk::ModuleComparatorOutputStage::Unspecified,
          adk::ModulePullRequirement::Unspecified,
          adk::ModuleDeclaredRail::Unspecified,
          {3000, 5000},
          {0, 5000},
          {0, 1023},
          adk::ModuleComparatorPolarity::Unspecified,
          adk::ModuleThresholdControlKind::Unspecified,
          adk::ModuleThresholdDirection::Unspecified,
          {adk::ModuleDurationDeclaration::Known, adk::Duration (0)},
          {adk::ModuleDurationDeclaration::Known, adk::Duration (0)}},
         {1,
          7005,
          1,
          700005,
          2,
          2,
          {72, 1, 5, adk::TimePoint (500)},
          0,
          adk::ModuleChannelStatus::NotPresent,
          true,
          adk::ModuleChannelStatus::Current,
          true,
          false,
          true,
          true,
          adk::StatusCode::Ok,
          adk::StatusCode::Ok},
         DescriptorDisposition::AcceptedIncomplete}};

    constexpr uint8_t copiedFixtureCount =
        sizeof (copiedFixtures) / sizeof (copiedFixtures[0]);

    volatile DescriptorResultCell descriptorResultCells[copiedFixtureCount];
    volatile uint8_t              fixtureAcquiredCell;
    volatile uint8_t              resultsInitializedCell;
    volatile uint8_t              replayCompleteCell;

    uint8_t replayIndex;
    bool    replayActive;

    adk::Status acquireCopiedDescriptorFixtures ();
    void        configureDescriptorResults      ();
    adk::Status startDescriptorReplay           ();
    DescriptorObservation
    observeCopiedDescriptor (const CopiedDescriptorFixture& fixture);
    DescriptorDisposition
         decideDescriptorDisposition (const DescriptorObservation& observation);
    void actuateDescriptorResult (uint8_t index, const CopiedDescriptorFixture& fixture,
                                  const DescriptorObservation& observation,
                                  DescriptorDisposition        disposition);

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedDescriptorFixtures ();

    configureDescriptorResults ();

    if (!fixtureStatus.ok ())
    {
        return;
    }

    startDescriptorReplay ();
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const CopiedDescriptorFixture& fixture = copiedFixtures[replayIndex];
    const DescriptorObservation observation =
        observeCopiedDescriptor (fixture);
    const DescriptorDisposition disposition =
        decideDescriptorDisposition (observation);

    actuateDescriptorResult (replayIndex, fixture, observation, disposition);

    ++replayIndex;
    replayActive       = replayIndex < copiedFixtureCount;
    replayCompleteCell = replayActive ? 0 : 1;
}

namespace {

    adk::Status acquireCopiedDescriptorFixtures ()
    {
        fixtureAcquiredCell =
            copiedFixtureCount != 0 && copiedFixtures[0].descriptor.schemaRevision != 0
                ? 1
                : 0;

        return fixtureAcquiredCell != 0 ? adk::StatusCode::Ok
                                        : adk::StatusCode::InternalInvariant;
    }

    void configureDescriptorResults ()
    {
        replayIndex            = 0;
        replayActive           = false;
        resultsInitializedCell = 0;
        replayCompleteCell     = 0;

        for (uint8_t index = 0; index < copiedFixtureCount; ++index)
        {
            descriptorResultCells[index].descriptorId                = 0;
            descriptorResultCells[index].specimenReference           = 0;
            descriptorResultCells[index].sequence                    = 0;
            descriptorResultCells[index].descriptorRevision          = 0;
            descriptorResultCells[index].specimenRevision            = 0;
            descriptorResultCells[index].electricalEvidenceRevision  = 0;
            descriptorResultCells[index].sourceConfigurationRevision = 0;
            descriptorResultCells[index].rawMinimum                  = 0xffff;
            descriptorResultCells[index].rawMaximum                  = 0xffff;
            descriptorResultCells[index].analogRaw                   = 0xffff;
            descriptorResultCells[index].warmupMilliseconds          = 0xffff;
            descriptorResultCells[index].settlingMilliseconds        = 0xffff;
            descriptorResultCells[index].topology                    = 0xff;
            descriptorResultCells[index].outputStage                 = 0xff;
            descriptorResultCells[index].comparatorPolarity          = 0xff;
            descriptorResultCells[index].pullRequirement             = 0xff;
            descriptorResultCells[index].pullRail                    = 0xff;
            descriptorResultCells[index].thresholdDirection          = 0xff;
            descriptorResultCells[index].sourceId                    = 0xff;
            descriptorResultCells[index].analogStatus                = 0xff;
            descriptorResultCells[index].comparatorStatus            = 0xff;
            descriptorResultCells[index].analogProducerStatus        = 0xff;
            descriptorResultCells[index].comparatorProducerStatus    = 0xff;
            descriptorResultCells[index].declaredWarmupSatisfied     = 0xff;
            descriptorResultCells[index].declaredSettlingSatisfied   = 0xff;
            descriptorResultCells[index].descriptorStatus            = 0xff;
            descriptorResultCells[index].frameStatus                 = 0xff;
            descriptorResultCells[index].assertionStatus             = 0xff;
            descriptorResultCells[index].declarationsComplete        = 0xff;
            descriptorResultCells[index].comparatorAsserted          = 0xff;
            descriptorResultCells[index].disposition                 = 0xff;
            descriptorResultCells[index].predictionPass              = 0;
        }

        resultsInitializedCell = 1;
    }

    adk::Status startDescriptorReplay ()
    {
        if (fixtureAcquiredCell == 0 || resultsInitializedCell == 0)
        {
            return adk::StatusCode::NotInitialized;
        }

        replayActive = true;
        return adk::StatusCode::Ok;
    }

    DescriptorObservation
    observeCopiedDescriptor (const CopiedDescriptorFixture& fixture)
    {
        DescriptorObservation observation = {adk::StatusCode::NotInitialized,
                                             adk::StatusCode::NotInitialized,
                                             adk::StatusCode::NotInitialized,
                                             adk::StatusCode::NotInitialized,
                                             false,
                                             false};

        observation.descriptorStatus =
            adk::validateModuleThresholdDescriptor (fixture.descriptor);

        if (!observation.descriptorStatus.ok ())
        {
            return observation;
        }

        observation.frameStatus =
            adk::validateModuleThresholdFrame (fixture.descriptor, fixture.frame);

        const adk::Result<bool> declarations =
            adk::moduleDescriptorDeclarationsComplete (fixture.descriptor);
        observation.declarationsStatus = declarations.status ();

        if (declarations.ok ())
        {
            observation.declarationsComplete = declarations.value ();
        }

        const adk::Result<bool> assertion = adk::moduleComparatorAsserted (
            fixture.descriptor, fixture.frame.comparatorLevelHigh);
        observation.assertionStatus = assertion.status ();

        if (assertion.ok ())
        {
            observation.comparatorAsserted = assertion.value ();
        }

        return observation;
    }

    DescriptorDisposition
    decideDescriptorDisposition (const DescriptorObservation& observation)
    {
        if (!observation.descriptorStatus.ok ())
        {
            return DescriptorDisposition::RejectedDescriptor;
        }

        if (!observation.frameStatus.ok () || !observation.declarationsStatus.ok ())
        {
            return DescriptorDisposition::RejectedFrame;
        }

        return observation.declarationsComplete
                   ? DescriptorDisposition::AcceptedComplete
                   : DescriptorDisposition::AcceptedIncomplete;
    }

    void actuateDescriptorResult (uint8_t index, const CopiedDescriptorFixture& fixture,
                                  const DescriptorObservation& observation,
                                  DescriptorDisposition        disposition)
    {
        const adk::ModuleThresholdDescriptor& descriptor = fixture.descriptor;

        descriptorResultCells[index].descriptorId = descriptor.descriptorId;
        descriptorResultCells[index].specimenReference =
            descriptor.declaredSpecimenReference;
        descriptorResultCells[index].sequence = fixture.frame.provenance.sequence;
        descriptorResultCells[index].descriptorRevision = descriptor.descriptorRevision;
        descriptorResultCells[index].specimenRevision =
            descriptor.declaredSpecimenRevision;
        descriptorResultCells[index].electricalEvidenceRevision =
            descriptor.declaredElectricalEvidenceRevision;
        descriptorResultCells[index].sourceConfigurationRevision =
            fixture.frame.provenance.sourceConfigurationRevision;
        descriptorResultCells[index].rawMinimum = descriptor.rawDomain.minimum;
        descriptorResultCells[index].rawMaximum = descriptor.rawDomain.maximum;
        descriptorResultCells[index].analogRaw  = fixture.frame.analogRaw;
        descriptorResultCells[index].warmupMilliseconds =
            static_cast<uint16_t> (descriptor.warmup.value.milliseconds ());
        descriptorResultCells[index].settlingMilliseconds =
            static_cast<uint16_t> (descriptor.settling.value.milliseconds ());
        descriptorResultCells[index].topology =
            static_cast<uint8_t> (descriptor.channelTopology);
        descriptorResultCells[index].outputStage =
            static_cast<uint8_t> (descriptor.comparatorOutputStage);
        descriptorResultCells[index].comparatorPolarity =
            static_cast<uint8_t> (descriptor.comparatorPolarity);
        descriptorResultCells[index].pullRequirement =
            static_cast<uint8_t> (descriptor.pullRequirement);
        descriptorResultCells[index].pullRail =
            static_cast<uint8_t> (descriptor.declaredPullRail);
        descriptorResultCells[index].thresholdDirection =
            static_cast<uint8_t> (descriptor.thresholdDirection);
        descriptorResultCells[index].sourceId = fixture.frame.provenance.sourceId;
        descriptorResultCells[index].analogStatus =
            static_cast<uint8_t> (fixture.frame.analogStatus);
        descriptorResultCells[index].comparatorStatus =
            static_cast<uint8_t> (fixture.frame.comparatorStatus);
        descriptorResultCells[index].analogProducerStatus =
            static_cast<uint8_t> (fixture.frame.analogProducerStatus.error ());
        descriptorResultCells[index].comparatorProducerStatus =
            static_cast<uint8_t> (fixture.frame.comparatorProducerStatus.error ());
        descriptorResultCells[index].declaredWarmupSatisfied =
            fixture.frame.declaredWarmupSatisfied ? 1 : 0;
        descriptorResultCells[index].declaredSettlingSatisfied =
            fixture.frame.declaredSettlingSatisfied ? 1 : 0;
        descriptorResultCells[index].descriptorStatus =
            static_cast<uint8_t> (observation.descriptorStatus.error ());
        descriptorResultCells[index].frameStatus =
            static_cast<uint8_t> (observation.frameStatus.error ());
        descriptorResultCells[index].assertionStatus =
            static_cast<uint8_t> (observation.assertionStatus.error ());
        descriptorResultCells[index].declarationsComplete =
            observation.declarationsComplete ? 1 : 0;
        descriptorResultCells[index].comparatorAsserted =
            observation.comparatorAsserted ? 1 : 0;
        descriptorResultCells[index].disposition = static_cast<uint8_t> (disposition);
        descriptorResultCells[index].predictionPass =
            disposition == fixture.expectedDisposition ? 1 : 0;
    }

} // namespace
