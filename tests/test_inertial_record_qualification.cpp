#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include "inertial_record_qualification.h"

namespace {

    adk::InertialSource source ()
    {
        return {adk::InertialSourceKind::SyntheticFixture,
                adk::InertialModel::Synthetic,
                7,
                3,
                4,
                2000000,
                250000};
    }

    adk::InertialRecordQualificationConfig config ()
    {
        return {9,
                2,
                5,
                source (),
                {adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
                 adk::SignedAxis::PositiveZ},
                3,
                adk::Duration (20),
                adk::Duration (10),
                {0, 0, 1000000},
                {100000, 100000, 100000},
                {50, 50, 50}};
    }

    adk::InertialRecord record (uint32_t sequence, uint32_t observedAt)
    {
        return {2,
                5,
                source (),
                {0, 0, 1000000},
                {0, 0, 0},
                adk::TimePoint (observedAt),
                sequence,
                true,
                adk::InertialSaturation::None,
                adk::StatusCode::Ok,
                adk::InertialRecordState::Recorded};
    }

    adk::InertialQualificationEvidence
    evidence (const adk::InertialRecordQualificationPolicy& policy)
    {
        const adk::InertialVector          zero   = {0, 0, 0};
        const adk::InertialWideVector      wide   = {0, 0, 0};
        const adk::InertialRecord          empty  = record (1, 0);
        adk::InertialQualificationEvidence output = {
            0,
            0,
            0,
            {adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
             adk::SignedAxis::PositiveZ},
            adk::InertialQualificationState::Idle,
            adk::InertialQualificationReason::None,
            0,
            0,
            0,
            adk::TimePoint (),
            adk::TimePoint (),
            adk::Duration  (),
            adk::Duration  (),
            zero,
            zero,
            zero,
            zero,
            zero,
            zero,
            wide,
            wide,
            empty,
            empty,
            adk::Status ()};
        assert (policy.evidence (output).ok ());
        return output;
    }

    void start (adk::InertialRecordQualificationPolicy& policy, uint32_t attempt = 1)
    {
        assert (policy.initialize (adk::TimePoint (0)).ok ());
        assert (policy.begin (adk::TimePoint (0), attempt).ok ());
    }

    bool equalRecord (const adk::InertialRecord& left, const adk::InertialRecord& right)
    {
        uint8_t leftImage[adk::InertialRecordCodec::size]  = {};
        uint8_t rightImage[adk::InertialRecordCodec::size] = {};
        adk::InertialRecordCodec codec;
        assert (codec.encode (left, {leftImage, sizeof leftImage}).ok ());
        assert (codec.encode (right, {rightImage, sizeof rightImage}).ok ());
        for (uint8_t index = 0; index < sizeof leftImage; ++index)
        {
            if (leftImage[index] != rightImage[index])
            {
                return false;
            }
        }
        return true;
    }

    bool equalEvidence (const adk::InertialQualificationEvidence& left,
                        const adk::InertialQualificationEvidence& right)
    {
        return left.attemptId == right.attemptId &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.qualificationRevision == right.qualificationRevision &&
               left.sourceToQualificationFrame.x == right.sourceToQualificationFrame.x &&
               left.sourceToQualificationFrame.y == right.sourceToQualificationFrame.y &&
               left.sourceToQualificationFrame.z == right.sourceToQualificationFrame.z &&
               left.state == right.state && left.reason == right.reason &&
               left.acceptedSampleCount == right.acceptedSampleCount &&
               left.firstSequence == right.firstSequence &&
               left.lastSequence == right.lastSequence &&
               left.firstObservedAt == right.firstObservedAt &&
               left.lastObservedAt == right.lastObservedAt &&
               left.maximumObservedAge == right.maximumObservedAge &&
               left.maximumObservedGap == right.maximumObservedGap &&
               left.meanAccelerationMicroG.x == right.meanAccelerationMicroG.x &&
               left.meanAccelerationMicroG.y == right.meanAccelerationMicroG.y &&
               left.meanAccelerationMicroG.z == right.meanAccelerationMicroG.z &&
               left.meanAngularRateMilliDegreesPerSecond.x ==
                   right.meanAngularRateMilliDegreesPerSecond.x &&
               left.meanAngularRateMilliDegreesPerSecond.y ==
                   right.meanAngularRateMilliDegreesPerSecond.y &&
               left.meanAngularRateMilliDegreesPerSecond.z ==
                   right.meanAngularRateMilliDegreesPerSecond.z &&
               left.minimumAccelerationMicroG.x ==
                   right.minimumAccelerationMicroG.x &&
               left.minimumAccelerationMicroG.y ==
                   right.minimumAccelerationMicroG.y &&
               left.minimumAccelerationMicroG.z ==
                   right.minimumAccelerationMicroG.z &&
               left.maximumAccelerationMicroG.x ==
                   right.maximumAccelerationMicroG.x &&
               left.maximumAccelerationMicroG.y ==
                   right.maximumAccelerationMicroG.y &&
               left.maximumAccelerationMicroG.z ==
                   right.maximumAccelerationMicroG.z &&
               left.minimumAngularRateMilliDegreesPerSecond.x ==
                   right.minimumAngularRateMilliDegreesPerSecond.x &&
               left.minimumAngularRateMilliDegreesPerSecond.y ==
                   right.minimumAngularRateMilliDegreesPerSecond.y &&
               left.minimumAngularRateMilliDegreesPerSecond.z ==
                   right.minimumAngularRateMilliDegreesPerSecond.z &&
               left.maximumAngularRateMilliDegreesPerSecond.x ==
                   right.maximumAngularRateMilliDegreesPerSecond.x &&
               left.maximumAngularRateMilliDegreesPerSecond.y ==
                   right.maximumAngularRateMilliDegreesPerSecond.y &&
               left.maximumAngularRateMilliDegreesPerSecond.z ==
                   right.maximumAngularRateMilliDegreesPerSecond.z &&
               left.accelerationSumsMicroG.x == right.accelerationSumsMicroG.x &&
               left.accelerationSumsMicroG.y == right.accelerationSumsMicroG.y &&
               left.accelerationSumsMicroG.z == right.accelerationSumsMicroG.z &&
               left.angularRateSumsMilliDegreesPerSecond.x ==
                   right.angularRateSumsMilliDegreesPerSecond.x &&
               left.angularRateSumsMilliDegreesPerSecond.y ==
                   right.angularRateSumsMilliDegreesPerSecond.y &&
               left.angularRateSumsMilliDegreesPerSecond.z ==
                   right.angularRateSumsMilliDegreesPerSecond.z &&
               equalRecord (left.terminalRecord, right.terminalRecord) &&
               equalRecord (left.mappedRecord, right.mappedRecord) &&
               left.status == right.status;
    }

    void testMappingDomainIsExactlyTwentyFourProperRotations ()
    {
        uint16_t accepted = 0;
        for (uint8_t x = 0; x < 6; ++x)
        {
            for (uint8_t y = 0; y < 6; ++y)
            {
                for (uint8_t z = 0; z < 6; ++z)
                {
                    const adk::SourceAxisMapping mapping = {
                        static_cast<adk::SignedAxis> (x),
                        static_cast<adk::SignedAxis> (y),
                        static_cast<adk::SignedAxis> (z)};
                    if (!adk::validSignedAxisMapping (mapping))
                    {
                        continue;
                    }
                    ++accepted;
                    int32_t mappedX = 0;
                    int32_t mappedY = 0;
                    int32_t mappedZ = 0;
                    assert (adk::mapSignedAxes (mapping, 11, 22, 33, mappedX, mappedY,
                                                mappedZ));
                    const int32_t absoluteValues[3] = {mappedX < 0 ? -mappedX : mappedX,
                                                       mappedY < 0 ? -mappedY : mappedY,
                                                       mappedZ < 0 ? -mappedZ
                                                                   : mappedZ};
                    assert ((absoluteValues[0] == 11 || absoluteValues[1] == 11 ||
                             absoluteValues[2] == 11) &&
                            (absoluteValues[0] == 22 || absoluteValues[1] == 22 ||
                             absoluteValues[2] == 22) &&
                            (absoluteValues[0] == 33 || absoluteValues[1] == 33 ||
                             absoluteValues[2] == 33));
                }
            }
        }
        assert (accepted == 24);

        const adk::SourceAxisMapping invertX = {adk::SignedAxis::NegativeX,
                                                adk::SignedAxis::PositiveZ,
                                                adk::SignedAxis::PositiveY};
        int32_t                      x       = 71;
        int32_t                      y       = 72;
        int32_t                      z       = 73;
        assert (!adk::mapSignedAxes (invertX, INT32_MIN, 2, 3, x, y, z));
        assert (x == 71 && y == 72 && z == 73);
    }

    void testConfigurationAndLifecycleValidation ()
    {
        adk::InertialRecordQualificationConfig invalid[10];
        for (uint8_t index = 0; index < 10; ++index)
        {
            invalid[index] = config ();
        }
        invalid[0].qualificationRevision                    = 0;
        invalid[1].expectedSchemaRevision                   = 0;
        invalid[2].expectedNormalizationRevision            = 0;
        invalid[3].expectedSource.sourceId                  = 0;
        invalid[4].sourceToQualificationFrame.y             =
            adk::SignedAxis::PositiveX;
        invalid[5].requiredSampleCount                      = 1;
        invalid[6].requiredSampleCount                      = 33;
        invalid[7].maximumAge                               =
            adk::Duration (UINT32_C (0x80000000));
        invalid[8].maximumGap                               =
            adk::Duration (UINT32_C (0x80000000));
        invalid[9].maximumAccelerationDeviationMicroG.x     = -1;
        for (uint8_t index = 0; index < 10; ++index)
        {
            adk::InertialRecordQualificationPolicy policy (invalid[index]);
            assert                                        (policy.initialize (adk::TimePoint ()).error () ==
                    adk::StatusCode::InvalidConfiguration);
            assert (!policy.initialized ());
        }

        adk::InertialRecordQualificationPolicy valid (config ());
        assert                                       (valid.begin (adk::TimePoint (), 1).error () ==
                adk::StatusCode::NotInitialized);
        assert (valid.initialize (adk::TimePoint ()).ok ());
        assert (evidence (valid).lifecycleGeneration == 1);
        assert (valid.begin (adk::TimePoint (), 0).error () ==
                adk::StatusCode::InvalidArgument);
        assert (valid.begin (adk::TimePoint (), 41).ok ());
        assert (valid.begin (adk::TimePoint (), 42).error () ==
                adk::StatusCode::InvalidArgument);
        assert (valid.reset (adk::TimePoint ()).ok ());
        assert (evidence (valid).lifecycleGeneration == 2);
        assert (valid.begin (adk::TimePoint (), 42).ok ());

        adk::InertialRecordQualificationConfig physicalConfig = config ();
        physicalConfig.expectedSource.kind =
            adk::InertialSourceKind::Mpu6050Adapter;
        physicalConfig.expectedSource.model = adk::InertialModel::Mpu6050;
        adk::InertialRecordQualificationPolicy physical (physicalConfig);
        assert                                          (physical.initialize (adk::TimePoint ()).error () ==
                adk::StatusCode::Unsupported);
    }

    void testMappingWindowAndAggregateEvidence ()
    {
        adk::InertialRecordQualificationConfig value = config ();
        value.sourceToQualificationFrame             = {adk::SignedAxis::PositiveZ,
                                                        adk::SignedAxis::PositiveX,
                                                        adk::SignedAxis::PositiveY};
        value.expectedStationaryAccelerationMicroG   = {0, 1000000, 0};
        adk::InertialRecordQualificationPolicy policy (value);
        start                                         (policy, 17);

        adk::InertialRecord first              = record (10, 100);
        first.accelerationMicroG               = {1000000, 0, 0};
        first.angularRateMilliDegreesPerSecond = {10, -20, 30};
        assert (policy.observe (adk::TimePoint (105), first).ok ());
        assert (evidence (policy).state == adk::InertialQualificationState::Collecting);

        adk::InertialRecord second              = first;
        second.sequence                         = 11;
        second.observedAt                       = adk::TimePoint (105);
        second.accelerationMicroG               = {900000, 0, 0};
        second.angularRateMilliDegreesPerSecond = {20, -10, 40};
        assert (policy.observe (adk::TimePoint (125), second).ok ());

        adk::InertialRecord third              = second;
        third.sequence                         = 12;
        third.observedAt                       = adk::TimePoint (115);
        third.accelerationMicroG               = {1100000, 0, 0};
        third.angularRateMilliDegreesPerSecond = {30, 0, 50};
        assert (policy.observe (adk::TimePoint (135), third).ok ());

        const adk::InertialQualificationEvidence result = evidence (policy);
        assert                                                     (result.state == adk::InertialQualificationState::Qualified);
        assert                                                     (result.reason == adk::InertialQualificationReason::None);
        assert                                                     (result.attemptId == 17 && result.acceptedSampleCount == 3);
        assert                                                     (result.lifecycleGeneration == 1);
        assert                                                     (result.sourceToQualificationFrame.x ==
                value.sourceToQualificationFrame.x);
        assert                                                     (result.firstSequence == 10 && result.lastSequence == 12);
        assert                                                     (result.maximumObservedAge == adk::Duration (20));
        assert                                                     (result.maximumObservedGap == adk::Duration (10));
        assert                                                     (result.mappedRecord.accelerationMicroG.y == 1100000);
        assert                                                     (result.mappedRecord.angularRateMilliDegreesPerSecond.x == 50);
        assert                                                     (result.mappedRecord.angularRateMilliDegreesPerSecond.y == 30);
        assert                                                     (result.mappedRecord.angularRateMilliDegreesPerSecond.z == 0);
        assert                                                     (result.meanAccelerationMicroG.x == 0);
        assert                                                     (result.meanAccelerationMicroG.y == 1000000);
        assert                                                     (result.meanAngularRateMilliDegreesPerSecond.x == 40);
        assert                                                     (result.meanAngularRateMilliDegreesPerSecond.y == 20);
        assert                                                     (result.meanAngularRateMilliDegreesPerSecond.z == -10);
        assert                                                     (result.accelerationSumsMicroG.y == 3000000);
        assert                                                     (result.angularRateSumsMilliDegreesPerSecond.x == 120);
        assert                                                     (result.angularRateSumsMilliDegreesPerSecond.y == 60);
        assert                                                     (result.angularRateSumsMilliDegreesPerSecond.z == -30);

        const adk::InertialRecord extra = record (13, 116);
        assert                                   (policy.observe (adk::TimePoint (116), extra).error () ==
                adk::StatusCode::InvalidArgument);
        assert (evidence (policy).lastSequence == 12);
    }

    void testExactBoundsPassAndOneBeyondRejects ()
    {
        adk::InertialRecordQualificationConfig value = config ();
        value.requiredSampleCount                    = 2;
        adk::InertialRecordQualificationPolicy exact  (value);
        start                                         (exact);
        adk::InertialRecord low              = record (1, 10);
        low.accelerationMicroG               = {0, 0, 900000};
        low.angularRateMilliDegreesPerSecond = {-50, 50, 0};
        assert                                         (exact.observe (adk::TimePoint (30), low).ok ());
        adk::InertialRecord high              = record (2, 20);
        high.accelerationMicroG               = {0, 0, 1100000};
        high.angularRateMilliDegreesPerSecond = {50, -50, 50};
        assert (exact.observe (adk::TimePoint (40), high).ok ());
        assert (evidence (exact).state == adk::InertialQualificationState::Qualified);

        adk::InertialRecordQualificationPolicy acceleration (config ());
        start                                               (acceleration);
        adk::InertialRecord outside = record                (1, 10);
        outside.accelerationMicroG  = {0, 0, 1100001};
        assert (acceleration.observe (adk::TimePoint (10), outside).ok ());
        assert (evidence (acceleration).reason ==
                adk::InertialQualificationReason::AccelerationOutsideWindow);

        adk::InertialRecordQualificationPolicy rate       (config ());
        start                                             (rate);
        outside                                  = record (1, 10);
        outside.angularRateMilliDegreesPerSecond = {51, 0, 0};
        assert (rate.observe (adk::TimePoint (10), outside).ok ());
        assert (evidence (rate).reason ==
                adk::InertialQualificationReason::AngularRateOutsideWindow);
    }

    void testMismatchAndTimingPrecedence ()
    {
        adk::InertialRecordQualificationPolicy mismatch (config ());
        start                                           (mismatch);
        adk::InertialRecord wrong              = record (1, 100);
        wrong.schemaRevision                   = 3;
        wrong.producerStatus                   = adk::StatusCode::HardwareFailure;
        wrong.state                            = adk::InertialRecordState::SourceFault;
        wrong.dataReady                        = false;
        wrong.accelerationMicroG               = {0, 0, 0};
        wrong.angularRateMilliDegreesPerSecond = {0, 0, 0};
        assert (mismatch.observe (adk::TimePoint (200), wrong).ok ());
        assert (evidence (mismatch).reason ==
                adk::InertialQualificationReason::ConfigurationMismatch);

        adk::InertialRecordQualificationPolicy stale    (config ());
        start                                           (stale);
        adk::InertialRecord fault              = record (1, 100);
        fault.producerStatus                   = adk::StatusCode::HardwareFailure;
        fault.state                            = adk::InertialRecordState::SourceFault;
        fault.dataReady                        = false;
        fault.accelerationMicroG               = {0, 0, 0};
        fault.angularRateMilliDegreesPerSecond = {0, 0, 0};
        assert (stale.observe (adk::TimePoint (121), fault).ok ());
        assert (evidence (stale).reason == adk::InertialQualificationReason::Stale);

        adk::InertialRecordQualificationPolicy producer (config ());
        start                                           (producer);
        fault.observedAt = adk::TimePoint               (100);
        assert                                          (producer.observe (adk::TimePoint (100), fault).ok ());
        assert                                          (evidence (producer).reason ==
                adk::InertialQualificationReason::ProducerFault);

        adk::InertialRecordQualificationPolicy future (config ());
        start                                         (future);
        assert                                        (future.observe (adk::TimePoint (9), record (1, 10)).ok ());
        assert                                        (evidence (future).reason ==
                adk::InertialQualificationReason::TimestampDiscontinuity);
    }

    void testSignedMeansUseWideExactSums ()
    {
        adk::InertialRecordQualificationConfig value = config ();
        value.expectedStationaryAccelerationMicroG = {0, 0, 0};
        value.maximumAccelerationDeviationMicroG   = {10, 10, 10};
        adk::InertialRecordQualificationPolicy policy (value);
        start                                         (policy);

        const int32_t samples[3] = {-1, -2, -2};
        for (uint8_t index = 0; index < 3; ++index)
        {
            adk::InertialRecord input = record (index + 1, index + 1);
            input.accelerationMicroG  = {samples[index], 0, 0};
            assert (policy.observe (adk::TimePoint (index + 1), input).ok ());
        }
        const adk::InertialQualificationEvidence result = evidence (policy);
        assert                                                     (result.state == adk::InertialQualificationState::Qualified);
        assert                                                     (result.accelerationSumsMicroG.x == -5);
        assert                                                     (result.meanAccelerationMicroG.x == -1);
    }

    void testSequenceBoundaryDuplicateGapAndAtomicMalformedInput ()
    {
        adk::InertialRecordQualificationPolicy wrap (config ());
        start                                       (wrap);
        const adk::InertialRecord before = record   (UINT32_MAX, UINT32_MAX - 1);
        assert                                      (wrap.observe (adk::TimePoint (UINT32_MAX), before).ok ());
        adk::InertialRecord after = record          (0, 2);
        assert                                      (wrap.observe (adk::TimePoint (3), after).ok ());
        assert                                      (evidence (wrap).acceptedSampleCount == 2);

        assert (wrap.observe (adk::TimePoint (4), after).ok ());
        assert (evidence (wrap).acceptedSampleCount == 2);

        adk::InertialRecord changed  = after;
        changed.accelerationMicroG.z = 999000;
        assert (wrap.observe (adk::TimePoint (4), changed).ok ());
        assert (evidence (wrap).reason ==
                adk::InertialQualificationReason::SequenceDiscontinuity);

        assert (wrap.reset (adk::TimePoint ()).ok ());
        assert (wrap.begin (adk::TimePoint (), 2).ok ());
        assert (wrap.observe (adk::TimePoint (10), record (5, 10)).ok ());
        assert (wrap.observe (adk::TimePoint (11), record (7, 11)).ok ());
        assert (evidence (wrap).reason ==
                adk::InertialQualificationReason::SequenceDiscontinuity);

        assert (wrap.reset (adk::TimePoint ()).ok ());
        assert (wrap.begin (adk::TimePoint (), 4).ok ());
        assert (wrap.observe (adk::TimePoint (10), record (20, 10)).ok ());
        assert (wrap.observe (adk::TimePoint (11), record (19, 11)).ok ());
        assert (evidence (wrap).reason ==
                adk::InertialQualificationReason::SequenceDiscontinuity);

        assert (wrap.reset (adk::TimePoint ()).ok ());
        assert (wrap.begin (adk::TimePoint (), 5).ok ());
        assert (wrap.observe (adk::TimePoint (10), record (1, 10)).ok ());
        assert (wrap.observe (adk::TimePoint (11), record (UINT32_C (0x80000001), 11))
                    .ok ());
        assert (evidence (wrap).reason ==
                adk::InertialQualificationReason::SequenceDiscontinuity);

        assert (wrap.reset (adk::TimePoint ()).ok ());
        assert (wrap.begin (adk::TimePoint (), 6).ok ());
        assert (wrap.observe (adk::TimePoint (10), record (1, 10)).ok ());
        assert (wrap.observe (adk::TimePoint (11), record (2, 10)).ok ());
        assert (evidence (wrap).reason ==
                adk::InertialQualificationReason::TimestampDiscontinuity);

        assert                                                        (wrap.reset (adk::TimePoint ()).ok ());
        assert                                                        (wrap.begin (adk::TimePoint (), 3).ok ());
        assert                                                        (wrap.observe (adk::TimePoint (10), record (1, 10)).ok ());
        const adk::InertialQualificationEvidence sentinel  = evidence (wrap);
        adk::InertialRecord                      malformed = record   (2, 11);
        malformed.source.sourceId                          = 0;
        assert (wrap.observe (adk::TimePoint (11), malformed).error () ==
                adk::StatusCode::InvalidArgument);
        const adk::InertialQualificationEvidence unchanged = evidence (wrap);
        assert                                                        (equalEvidence (unchanged, sentinel));
    }

} // namespace

int main ()
{
    testMappingDomainIsExactlyTwentyFourProperRotations     ();
    testConfigurationAndLifecycleValidation                 ();
    testMappingWindowAndAggregateEvidence                   ();
    testExactBoundsPassAndOneBeyondRejects                  ();
    testMismatchAndTimingPrecedence                         ();
    testSignedMeansUseWideExactSums                         ();
    testSequenceBoundaryDuplicateGapAndAtomicMalformedInput ();
}
