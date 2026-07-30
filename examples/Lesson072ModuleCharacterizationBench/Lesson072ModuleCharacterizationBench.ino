// E0 copied-characterization project replay. This sketch runs the Lesson 070
// declaration and Lesson 071 policy before presenting one envelope to the
// inert bench. It owns no module, pin, ADC, comparator, clock, display,
// storage medium, stimulus, or powered circuit.
#include <Adk.h>
#include <inert_module_characterization_bench.h>

namespace {

    struct PresentationResultCell
    {
        uint32_t lifecycleGeneration;
        uint32_t sessionId;
        uint32_t runId;
        uint32_t descriptorDigest;
        uint32_t evidenceDigest;
        uint8_t  state;
        uint8_t  step;
        uint8_t  relation;
        uint8_t  faultDominant;
        uint8_t  recordPrepared;
        uint8_t  status;
        uint8_t  predictionPass;
    };

    struct ReplayResultCell
    {
        uint8_t fixtureStatus;
        uint8_t characterizationStatus;
        uint8_t benchStatus;
        uint8_t completedSteps;
        uint8_t codecValidity;
        uint8_t predictionsPass;
        uint8_t complete;
    };

    enum struct ReplayStage : uint8_t
    {
        InspectDeclaration,
        ReviewAscending,
        ReviewDescending,
        ReviewVerification,
        PrepareRecord,
        Shutdown,
        Complete
    };

    constexpr uint32_t sessionId                    = 72001UL;
    constexpr uint32_t runId                        = 72002UL;
    constexpr uint8_t  sourceId                     = 72;
    constexpr uint16_t sourceConfigurationRevision  = 1;
    constexpr uint8_t  requiredPointsPerLeg         = 4;
    constexpr uint8_t  controlSourceId              = 73;
    constexpr uint16_t controlConfigurationRevision = 1;
    constexpr uint32_t expectedDescriptorDigest     = 3411307269UL;

    const adk::ModuleThresholdDescriptor descriptor = {
        1,
        72001UL,
        1,
        720001UL,
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
        {adk::ModuleDurationDeclaration::Known, adk::Duration (100)},
        {adk::ModuleDurationDeclaration::Known, adk::Duration (10)}};

    const adk::ModuleCharacterizationConfig characterizationConfig = {
        1, descriptor, requiredPointsPerLeg, adk::Duration (5), adk::Duration (5)};

    const adk::ModuleBenchConfig benchConfig = {
        1,
        1,
        1,
        descriptor.descriptorId,
        descriptor.descriptorRevision,
        descriptor.schemaRevision,
        descriptor.declaredSpecimenRevision,
        descriptor.declaredElectricalEvidenceRevision,
        expectedDescriptorDigest,
        controlSourceId,
        controlConfigurationRevision,
        adk::Duration (5)};

    const uint16_t ascendingRaw[4]         = {0, 500, 700, 1023};
    const bool     ascendingAsserted[4]    = {false, false, true, true};
    const uint16_t descendingRaw[4]        = {1023, 650, 450, 0};
    const bool     descendingAsserted[4]   = {true, true, false, false};
    const uint16_t verificationRaw[4]      = {300, 550, 800, 900};
    const bool     verificationAsserted[4] = {false, false, true, true};

    adk::ModuleCharacterizationPolicy      characterization (characterizationConfig);

    adk::InertModuleCharacterizationBench  bench (benchConfig);
    adk::ModuleCharacterizationEnvelope    workingEnvelope;
    adk::ModuleBenchResult                 workingResult;
    adk::ModuleCharacterizationRecordImage workingImage;

    volatile PresentationResultCell presentationResultCell;
    volatile ReplayResultCell       replayResultCell;
    volatile uint8_t
        characterizationRecordCell[adk::ModuleCharacterizationRecordImage::size];

    ReplayStage replayStage;
    uint32_t    suppliedNow;
    uint32_t    nextSequence;
    uint32_t    nextControlSequence;

    // clang-format off
    adk::Status acquireCopiedFixture      ();
    void        configureBenchReplay      ();
    adk::Status startCharacterization     ();
    adk::Status replayCharacterizationLeg (
        uint32_t legId, adk::ModuleCharacterizationLeg leg,
        adk::ModuleSweepDirection direction, const uint16_t analogRaw[4],
        const bool comparatorAsserted[4]);
    adk::Status startBenchSession         ();

    adk::Status observeReviewControl ();

    adk::Status actuatePreparedRecord () __attribute__ ((noinline));

    bool        decideBenchResult    (
        const adk::ModuleBenchResult& result, adk::Status status);
    void        actuateBenchIntent   (
        const adk::ModuleBenchResult& result, bool prediction);
    void        finishBenchReplay    (adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedFixture ();

    configureBenchReplay ();

    replayResultCell.fixtureStatus = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        finishBenchReplay (fixtureStatus);
        return;
    }

    adk::Status status                      = startCharacterization ();

    replayResultCell.characterizationStatus = static_cast<uint8_t> (status.error ());

    if (status.ok ())
    {
        status = startBenchSession ();
    }
    replayResultCell.benchStatus = static_cast<uint8_t> (status.error ());

    if (!status.ok ())
    {
        finishBenchReplay (status);
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
        const adk::Status status = bench.shutdown (adk::TimePoint (++suppliedNow));

        finishBenchReplay (status);
        return;
    }

    const adk::Status status       = observeReviewControl ();

    const adk::Status resultStatus = bench.result (workingResult);
    const bool        prediction =
        status.ok () && resultStatus.ok () && decideBenchResult (workingResult, status);

    actuateBenchIntent (workingResult, prediction);

    if (!status.ok () || !resultStatus.ok ())
    {
        finishBenchReplay (!status.ok () ? status : resultStatus);
        return;
    }

    replayStage = static_cast<ReplayStage> (static_cast<uint8_t> (replayStage) + 1U);
}

namespace {

    adk::Status acquireCopiedFixture ()
    {
        const adk::Status descriptorStatus =
            adk::validateModuleThresholdDescriptor (descriptor);
        if (!descriptorStatus.ok ())
        {
            return descriptorStatus;
        }
        const adk::Result<bool> declarations =
            adk::moduleDescriptorDeclarationsComplete (descriptor);
        if (!declarations.ok ())
        {
            return declarations.status ();
        }
        if (!declarations.value () || adk::moduleThresholdDescriptorDigest (
                                          descriptor) != expectedDescriptorDigest)
        {
            return adk::StatusCode::InvalidConfiguration;
        }
        return adk::StatusCode::Ok;
    }

    void configureBenchReplay ()
    {
        replayStage                                =
            ReplayStage::InspectDeclaration;
        suppliedNow                                = 100;
        nextSequence                               = 1;
        nextControlSequence                        = 1;
        workingImage                               = {};
        replayResultCell.fixtureStatus             = 0xff;
        replayResultCell.characterizationStatus    = 0xff;
        replayResultCell.benchStatus                = 0xff;
        replayResultCell.completedSteps             = 0;
        replayResultCell.codecValidity              = 0xff;
        replayResultCell.predictionsPass            = 1;
        replayResultCell.complete                   = 0;
        presentationResultCell.lifecycleGeneration = 0;
        presentationResultCell.sessionId           = 0;
        presentationResultCell.runId                = 0;
        presentationResultCell.descriptorDigest     = 0;
        presentationResultCell.evidenceDigest       = 0;
        presentationResultCell.state                = 0;
        presentationResultCell.step                 = 0;
        presentationResultCell.relation             = 0;
        presentationResultCell.faultDominant        = 0;
        presentationResultCell.recordPrepared       = 0;
        presentationResultCell.status               = 0xff;
        presentationResultCell.predictionPass       = 0;
        for (uint16_t index = 0; index < adk::ModuleCharacterizationRecordImage::size;
             ++index)
        {
            characterizationRecordCell[index] = 0;
        }
    }

    adk::Status startCharacterization ()
    {
        adk::Status status = characterization.initialize (adk::TimePoint (suppliedNow));

        if (status.ok ())
        {
            status = characterization.beginSession (adk::TimePoint (++suppliedNow),
                                                    sessionId, runId);
        }
        if (status.ok ())
        {
            status = replayCharacterizationLeg (
                1, adk::ModuleCharacterizationLeg::Ascending,
                adk::ModuleSweepDirection::Increasing, ascendingRaw, ascendingAsserted);
        }
        if (status.ok ())
        {
            status = replayCharacterizationLeg (
                2, adk::ModuleCharacterizationLeg::Descending,
                adk::ModuleSweepDirection::Decreasing, descendingRaw,
                descendingAsserted);
        }
        if (status.ok ())
        {
            status = replayCharacterizationLeg (
                3, adk::ModuleCharacterizationLeg::Verification,
                adk::ModuleSweepDirection::Unordered, verificationRaw,
                verificationAsserted);
        }
        if (status.ok ())
        {
            status = characterization.evidence (workingEnvelope.evidence);
        }
        return status;
    }

    adk::Status replayCharacterizationLeg (uint32_t                       legId,
                                           adk::ModuleCharacterizationLeg leg,
                                           adk::ModuleSweepDirection      direction,
                                           const uint16_t                 analogRaw[4],
                                           const bool comparatorAsserted[4])
    {
        adk::Status status = characterization.beginLeg (adk::TimePoint (++suppliedNow),
                                                        legId, leg, direction);
        for (uint16_t index = 0; status.ok () && index < requiredPointsPerLeg; ++index)
        {
            ++suppliedNow;
            const adk::ModuleThresholdFrame frame = {
                descriptor.schemaRevision,
                descriptor.descriptorId,
                descriptor.descriptorRevision,
                descriptor.declaredSpecimenReference,
                descriptor.declaredSpecimenRevision,
                descriptor.declaredElectricalEvidenceRevision,
                {sourceId, sourceConfigurationRevision, nextSequence++,
                 adk::TimePoint (suppliedNow)},
                analogRaw[index],
                adk::ModuleChannelStatus::Current,
                comparatorAsserted[index],
                adk::ModuleChannelStatus::Current,
                true,
                comparatorAsserted[index],
                true,
                true,
                adk::StatusCode::Ok,
                adk::StatusCode::Ok};
            const adk::ModuleCharacterizationPoint point = {
                sessionId, runId,     legId,    static_cast<uint16_t> (index + 1U),
                leg,       direction, sourceId, sourceConfigurationRevision,
                frame};
            status =
                characterization.observe (adk::TimePoint (suppliedNow), point);
        }
        return status.ok ()
                   ? characterization.finalizeLeg (adk::TimePoint (++suppliedNow))
                   : status;
    }

    adk::Status startBenchSession ()
    {
        workingEnvelope.envelopeRevision = benchConfig.envelopeRevision;
        workingEnvelope.descriptorDigest =
            adk::moduleThresholdDescriptorDigest (descriptor);
        workingEnvelope.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (workingEnvelope.evidence);

        adk::Status status = bench.initialize (adk::TimePoint (++suppliedNow));

        if (status.ok ())
        {
            status = bench.beginSession (adk::TimePoint (++suppliedNow), sessionId,
                                         workingEnvelope);
        }
        return status;
    }

    adk::Status observeReviewControl ()
    {
        if (replayStage == ReplayStage::PrepareRecord)
        {
            adk::Status status =
                bench.prepareRecord (adk::TimePoint (++suppliedNow), workingImage);
            if (!status.ok ())
            {
                return status;
            }
            return actuatePreparedRecord ();
        }

        const adk::ModuleBenchControl control = {controlSourceId,
                                                 controlConfigurationRevision,
                                                 sessionId,
                                                 nextControlSequence++,
                                                 adk::TimePoint (++suppliedNow),
                                                 adk::ModuleBenchCommand::Advance,
                                                 adk::StatusCode::Ok};
        return bench.applyCommand (adk::TimePoint (suppliedNow), control);
    }

    adk::Status actuatePreparedRecord ()
    {
        const adk::ModuleCharacterizationRecordCodec codec;
        adk::ModuleCharacterizationRecord            decodedRecord;
        const adk::ModuleCharacterizationRecordValidity validity =
            codec.decode (
                {workingImage.bytes,
                 adk::ModuleCharacterizationRecordImage::size},
                decodedRecord);

        replayResultCell.codecValidity = static_cast<uint8_t> (validity);
        for (uint16_t index = 0;
             index < adk::ModuleCharacterizationRecordImage::size; ++index)
        {
            characterizationRecordCell[index] = workingImage.bytes[index];
        }
        return validity == adk::ModuleCharacterizationRecordValidity::Valid
                   ? adk::StatusCode::Ok
                   : adk::StatusCode::InternalInvariant;
    }

    bool decideBenchResult (const adk::ModuleBenchResult& result, adk::Status status)
    {
        const adk::ModuleBenchScriptStep expectedStep =
            static_cast<adk::ModuleBenchScriptStep> (
                static_cast<uint8_t> (replayStage) + 1U);
        const bool preparingRecord = replayStage == ReplayStage::PrepareRecord;

        return status.ok () && result.lifecycleGeneration != 0 &&
               result.sessionId == sessionId && result.runId == runId &&
               result.descriptorDigest == workingEnvelope.descriptorDigest &&
               result.evidenceDigest == workingEnvelope.evidenceDigest &&
               result.step == (preparingRecord
                                   ? adk::ModuleBenchScriptStep::PrepareRecord
                                   : expectedStep) &&
               result.state == (preparingRecord
                                    ? adk::ModuleBenchState::RecordPrepared
                                    : adk::ModuleBenchState::ScriptActive) &&
               result.relation == workingEnvelope.evidence.relation &&
               result.presentation.step == result.step &&
               result.presentation.state == result.state &&
               result.presentation.relation == result.relation &&
               !result.presentation.faultDominant &&
               result.recordPrepared == preparingRecord;
    }

    void actuateBenchIntent (const adk::ModuleBenchResult& result, bool prediction)
    {
        presentationResultCell.lifecycleGeneration = result.lifecycleGeneration;
        presentationResultCell.sessionId           = result.sessionId;
        presentationResultCell.runId               = result.runId;
        presentationResultCell.descriptorDigest    = result.descriptorDigest;
        presentationResultCell.evidenceDigest      = result.evidenceDigest;
        presentationResultCell.state    = static_cast<uint8_t> (result.state);
        presentationResultCell.step     = static_cast<uint8_t> (result.step);
        presentationResultCell.relation = static_cast<uint8_t> (result.relation);
        presentationResultCell.faultDominant =
            result.presentation.faultDominant ? 1 : 0;
        presentationResultCell.recordPrepared = result.recordPrepared ? 1 : 0;
        presentationResultCell.status = static_cast<uint8_t> (result.status.error ());
        presentationResultCell.predictionPass = prediction ? 1 : 0;

        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && prediction ? 1 : 0;
        ++replayResultCell.completedSteps;
    }

    void finishBenchReplay (adk::Status status)
    {
        replayResultCell.predictionsPass =
            replayResultCell.predictionsPass && status.ok () ? 1 : 0;
        replayResultCell.complete = 1;
        replayStage               = ReplayStage::Complete;
    }

} // namespace
