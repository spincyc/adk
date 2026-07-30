#include <assert.h>
#include <stdint.h>

#include "qualified_motion_recorder.h"

namespace {

    uint16_t imageCrc (const uint8_t* bytes)
    {
        uint16_t crc = UINT16_C (0xffff);
        for (uint16_t index = 0; index < 126; ++index)
        {
            crc ^= static_cast<uint16_t> (
                static_cast<uint16_t> (bytes[index]) << 8);
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                crc = (crc & UINT16_C (0x8000)) != 0
                          ? static_cast<uint16_t> (
                                (crc << 1) ^ UINT16_C (0x1021))
                          : static_cast<uint16_t> (crc << 1);
            }
        }
        return crc;
    }

    void repairImageCrc (adk::MotionRecordImage& image)
    {
        const uint16_t crc = imageCrc (image.bytes);
        image.bytes[126]   = static_cast<uint8_t> (crc >> 8);
        image.bytes[127]   = static_cast<uint8_t> (crc);
    }

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

    adk::MotionRecorderConfig config (uint16_t maximumRecordCount)
    {
        return {2,
                5,
                9,
                1,
                maximumRecordCount,
                UINT32_C (0x12345678),

                adk::Duration (20),
                adk::Duration (5),
                source        (),
                {{adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
                  adk::SignedAxis::PositiveZ},
                 800000,
                 1200000,
                 5000,
                 3000,
                 60000}};
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

    adk::InertialQualificationEvidence qualification ()
    {
        const adk::InertialRecord terminal = record (3, 3);
        return {11,
                1,
                9,
                {adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
                 adk::SignedAxis::PositiveZ},
                adk::InertialQualificationState::Qualified,
                adk::InertialQualificationReason::None,
                3,
                1,
                3,
                adk::TimePoint (1),
                adk::TimePoint (3),
                adk::Duration  (1),
                adk::Duration  (1),
                {0, 0, 1000000},
                {0, 0, 0},
                {0, 0, 1000000},
                {0, 0, 1000000},
                {0, 0, 0},
                {0, 0, 0},
                {0, 0, 3000000},
                {0, 0, 0},
                terminal,
                terminal,
                adk::StatusCode::Ok};
    }

    adk::MotionRecorderControl control (const adk::InertialRecord& input)
    {
        return {input.source.sourceId,
                input.sequence,
                input.observedAt,
                9,
                1,
                11,
                adk::motionQualificationDigest (qualification ()),

                UINT32_C (0x12345678),
                adk::MotionRecorderCommand::None,
                adk::StatusCode::Ok};
    }

    adk::MotionRecorderResult result (const adk::QualifiedMotionRecorder& recorder)
    {
        const adk::InertialRecord empty  = record (1, 1);
        adk::MotionRecorderResult output = {0,
                                            0,
                                            adk::MotionRecorderMode::Inert,
                                            adk::MotionRecorderHealth::Unknown,
                                            adk::MotionScriptStep::Rest,
                                            0,
                                            0,
                                            empty,
                                            qualification (),
                                            {adk::MotionDisplayToken::QualifySource,
                                             adk::MotionRecorderHealth::Unknown, 0, 0,
                                             0, false, 0, 0},
                                            false,
                                            adk::StatusCode::Ok};
        assert (recorder.result (output).ok ());
        return output;
    }

    void prepare (adk::QualifiedMotionRecorder& recorder, uint16_t capacity)
    {
        assert (recorder.initialize (adk::TimePoint (0), capacity).ok ());
        assert (result (recorder).mode ==
                adk::MotionRecorderMode::AwaitingQualification);
        assert (recorder.qualify (adk::TimePoint (3), qualification ()).ok ());
        assert (result (recorder).mode == adk::MotionRecorderMode::Ready);
        assert (recorder.begin (adk::TimePoint (4), 17).ok ());
        assert (result (recorder).mode == adk::MotionRecorderMode::Recording);
    }

    void assertQualificationRejected (
        const adk::InertialQualificationEvidence& evidence)
    {
        adk::QualifiedMotionRecorder recorder (config (1));

        assert (recorder.initialize (adk::TimePoint (0), 1).ok ());
        assert (!recorder.qualify (adk::TimePoint (3), evidence).ok ());
        assert (result (recorder).mode ==
                adk::MotionRecorderMode::AwaitingQualification);
    }

    void testQualificationCorrelationDomain ()
    {
        adk::InertialQualificationEvidence changed = qualification ();
        changed.attemptId                           = 0;
        assertQualificationRejected (changed);

        changed                     = qualification ();
        changed.lifecycleGeneration = 0;
        assertQualificationRejected (changed);

        changed                       = qualification ();
        changed.qualificationRevision = 10;
        assertQualificationRejected (changed);

        changed        = qualification ();
        changed.state  = adk::InertialQualificationState::Rejected;
        changed.reason = adk::InertialQualificationReason::ProducerFault;
        assertQualificationRejected (changed);

        changed                               = qualification ();
        changed.terminalRecord.schemaRevision = 3;
        assertQualificationRejected (changed);

        changed                                      = qualification ();
        changed.mappedRecord.normalizationRevision   = 6;
        assertQualificationRejected (changed);

        changed                              = qualification ();
        ++changed.mappedRecord.source.sourceId;
        assertQualificationRejected (changed);

        changed                       = qualification ();
        ++changed.mappedRecord.sequence;
        assertQualificationRejected (changed);

        changed                         = qualification  ();
        changed.mappedRecord.observedAt = adk::TimePoint (4);

        assertQualificationRejected (changed);

        changed = qualification ();
        ++changed.mappedRecord.accelerationMicroG.x;
        assertQualificationRejected (changed);

        changed = qualification ();
        changed.sourceToQualificationFrame.x =
            adk::SignedAxis::NegativeX;
        assertQualificationRejected (changed);

        changed = qualification ();
        changed.acceptedSampleCount = 2;
        assertQualificationRejected (changed);

        changed = qualification ();
        changed.firstSequence = 2;
        assertQualificationRejected (changed);

        changed = qualification ();

        changed.firstObservedAt = adk::TimePoint (3);

        assertQualificationRejected (changed);

        changed = qualification ();

        changed.maximumObservedGap = adk::Duration (3);

        assertQualificationRejected (changed);

        changed = qualification ();
        ++changed.meanAccelerationMicroG.z;
        assertQualificationRejected (changed);

        changed = qualification ();
        --changed.accelerationSumsMicroG.z;
        assertQualificationRejected (changed);

        changed = qualification ();
        changed.maximumAccelerationMicroG.z = 999999;
        assertQualificationRejected (changed);
    }

    void testLifecycleAndExportHandshake ()
    {
        adk::MotionRecordImage       images[2] = {};
        adk::QualifiedMotionRecorder recorder (config (2));

        prepare (recorder, 2);

        adk::InertialRecord        input   = record  (4, 5);
        adk::MotionRecorderControl buttons = control (input);
        buttons.command                     = adk::MotionRecorderCommand::RequestExport;

        assert (recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        adk::MotionRecorderResult current = result (recorder);

        assert (current.sessionId == 17);
        assert (current.recordCount == 1);
        assert (current.health == adk::MotionRecorderHealth::Recording);
        assert (current.presentation.orientationValid);
        assert (images[0].bytes[0] == 'Q');
        assert (current.exportRequested);
        assert (recorder.acknowledgeExport (adk::TimePoint (5)).ok ());
        assert (!result (recorder).exportRequested);

        adk::DecodedMotionRecord decoded = {
            0,
            0,
            0,
            0,
            adk::MotionScriptStep::Rest,
            adk::MotionRecorderHealth::Unknown,
            0,
            0,
            0,
            0,
            0,
            0,
            {adk::SignedAxis::PositiveX,
             adk::SignedAxis::PositiveY,
             adk::SignedAxis::PositiveZ},
            record (1, 1),
            {0,
             0,
             adk::OrientationQuality::Invalid,
             adk::StatusCode::InvalidArgument}};
        adk::MotionRecordCodec codec;
        assert (codec.decode (images[0], decoded) ==
                adk::MotionRecordValidity::Valid);
        assert (decoded.sessionId == 17);
        assert (decoded.ordinal == 0);
        assert (decoded.qualificationRevision == 9);
        assert (decoded.qualificationLifecycleGeneration == 1);
        assert (decoded.qualificationAttemptId == 11);
        assert (decoded.mappedRecord.sequence == 4);

        adk::MotionRecordImage badLength = images[0];
        badLength.bytes[5] = 127;

        repairImageCrc (badLength);

        assert (codec.decode (badLength, decoded) ==
                adk::MotionRecordValidity::BadLength);

        for (uint16_t index = 0; index < adk::MotionRecordImage::capacity;
             ++index)
        {
            adk::MotionRecordImage corrupted = images[0];
            corrupted.bytes[index] ^= UINT8_C (0x01);

            assert (codec.decode (corrupted, decoded) !=
                    adk::MotionRecordValidity::Valid);
        }

        const uint32_t decodedSession = decoded.sessionId;
        const uint8_t semanticOffsets[] = {25, 92, 93, 110};
        const uint8_t semanticValues[] = {
            static_cast<uint8_t> (adk::MotionRecorderHealth::Ready),
            static_cast<uint8_t> (adk::OrientationQuality::Invalid),
            static_cast<uint8_t> (adk::StatusCode::HardwareFailure),
            1};
        for (uint8_t index = 0; index < sizeof semanticOffsets; ++index)
        {
            adk::MotionRecordImage malformed = images[0];
            malformed.bytes[semanticOffsets[index]] = semanticValues[index];
            repairImageCrc (malformed);

            assert (codec.decode (malformed, decoded) ==
                    adk::MotionRecordValidity::BadSemanticValue);
            assert (decoded.sessionId == decodedSession);
        }

        input                  = record  (5, 10);
        buttons                = control (input);
        buttons.command = adk::MotionRecorderCommand::Advance;

        assert (recorder.update (adk::TimePoint (10), input, buttons, images, 2).ok ());

        current = result (recorder);

        assert (current.recordCount == 2);
        assert (current.scriptStep == adk::MotionScriptStep::TiltForward);
        assert (!current.exportRequested);

        assert (recorder.shutdown (adk::TimePoint (11)).ok ());
        assert (recorder.shutdown (adk::TimePoint (11)).ok ());

        current = result (recorder);

        assert (current.mode == adk::MotionRecorderMode::Shutdown);
        assert (!current.presentation.orientationValid);
    }

    void testCapacityFailureIsAtomic ()
    {
        adk::MotionRecordImage       images[1] = {};
        adk::QualifiedMotionRecorder recorder (config (1));

        prepare (recorder, 1);

        adk::InertialRecord        first   = record  (4, 5);
        adk::MotionRecorderControl buttons = control (first);

        assert (recorder.update (adk::TimePoint (5), first, buttons, images, 1).ok ());

        const adk::MotionRecordImage saved = images[0];

        adk::InertialRecord second = record  (5, 6);
        buttons                    = control (second);

        assert (
            !recorder.update (adk::TimePoint (6), second, buttons, images, 1).ok ());

        const adk::MotionRecorderResult current = result (recorder);

        assert (current.recordCount == 1);
        assert (current.mode == adk::MotionRecorderMode::Fault);
        assert (current.health == adk::MotionRecorderHealth::CapacityExhausted);
        for (uint16_t index = 0; index < adk::MotionRecordImage::capacity; ++index)
        {
            assert (images[0].bytes[index] == saved.bytes[index]);
        }
    }

    void testCorrelationAndFaultPresentation ()
    {
        adk::MotionRecordImage       images[2] = {};
        adk::QualifiedMotionRecorder recorder (config (2));

        prepare (recorder, 2);

        adk::InertialRecord        input   = record  (4, 5);
        adk::MotionRecorderControl buttons = control (input);

        ++buttons.sequence;

        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());
        assert (result (recorder).recordCount == 0);

        buttons = control (input);
        ++buttons.sourceId;
        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        buttons = control (input);

        buttons.observedAt = adk::TimePoint (6);

        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        buttons = control (input);
        ++buttons.qualificationRevision;
        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        buttons = control (input);
        ++buttons.qualificationLifecycleGeneration;
        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        buttons = control (input);
        ++buttons.qualificationAttemptId;
        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        buttons = control (input);
        ++buttons.qualificationDigest;
        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());

        buttons = control (input);
        ++buttons.traceToken;
        assert (!recorder.update (adk::TimePoint (5), input, buttons, images, 2).ok ());
        assert (result (recorder).recordCount == 0);

        assert (recorder.reset (adk::TimePoint (6)).ok ());
        assert (result (recorder).mode ==
                adk::MotionRecorderMode::AwaitingQualification);
        assert (recorder.qualify (adk::TimePoint (7), qualification ()).ok ());
        assert (recorder.begin (adk::TimePoint (8), 18).ok ());

        input                    = record (4, 9);
        input.producerStatus     = adk::StatusCode::HardwareFailure;
        input.state              = adk::InertialRecordState::SourceFault;
        input.dataReady          = false;
        input.accelerationMicroG = {0, 0, 0};
        buttons                  = control (input);

        assert (!recorder.update (adk::TimePoint (9), input, buttons, images, 2).ok ());

        const adk::MotionRecorderResult current = result (recorder);

        assert (current.health == adk::MotionRecorderHealth::SourceFault);
        assert (!current.presentation.orientationValid);
        assert (current.presentation.token == adk::MotionDisplayToken::Fault);
        assert (current.recordCount == 0);
    }

    void testResetDominatesAndReplayIsDeterministic ()
    {
        adk::MotionRecordImage       leftImages[1]  = {};
        adk::MotionRecordImage       rightImages[1] = {};
        adk::QualifiedMotionRecorder left  (config (1));
        adk::QualifiedMotionRecorder right (config (1));

        prepare                        (left, 1);
        prepare                        (right, 1);

        const adk::InertialRecord  input   = record  (4, 5);
        adk::MotionRecorderControl buttons = control (input);
        buttons.command = adk::MotionRecorderCommand::Reset;

        assert (left.update (adk::TimePoint (5), input, buttons, leftImages, 1).ok ());
        assert (
            right.update (adk::TimePoint (5), input, buttons, rightImages, 1).ok ());

        const adk::MotionRecorderResult leftResult  = result (left);
        const adk::MotionRecorderResult rightResult = result (right);

        assert (leftResult.mode == adk::MotionRecorderMode::AwaitingQualification);
        assert (rightResult.mode == leftResult.mode);
        assert (leftResult.recordCount == 0);
        assert (!leftResult.exportRequested);

        assert  (left.qualify (adk::TimePoint (6), qualification ()).ok ());
        assert  (right.qualify (adk::TimePoint (6), qualification ()).ok ());
        assert  (left.begin (adk::TimePoint (7), 19).ok ());
        assert  (right.begin (adk::TimePoint (7), 19).ok ());

        buttons = control (input);

        assert (left.update (adk::TimePoint (8), input, buttons, leftImages, 1).ok ());
        assert (
            right.update (adk::TimePoint (8), input, buttons, rightImages, 1).ok ());
        for (uint16_t index = 0; index < adk::MotionRecordImage::capacity;
             ++index)
        {
            assert (leftImages[0].bytes[index] == rightImages[0].bytes[index]);
        }
    }

    void testDuplicateAndOrientationFailureAreAtomic ()
    {
        struct GuardedImages
        {
            uint32_t               before;
            adk::MotionRecordImage images[2];
            uint32_t               after;
        };

        GuardedImages guarded = {UINT32_C (0x13579bdf), {}, UINT32_C (0x2468ace0)};

        adk::QualifiedMotionRecorder recorder (config (2));

        prepare (recorder, 2);

        const adk::InertialRecord  input   = record  (4, 5);
        adk::MotionRecorderControl buttons = control (input);

        assert (
            recorder.update (adk::TimePoint (5), input, buttons, guarded.images, 2).ok ());

        const adk::MotionRecordImage saved = guarded.images[0];

        assert (
            recorder.update (adk::TimePoint (5), input, buttons, guarded.images, 2).ok ());
        assert (result (recorder).recordCount == 1);
        assert (guarded.images[0].bytes[0] == saved.bytes[0]);

        buttons         = control (input);
        buttons.command = adk::MotionRecorderCommand::Advance;

        const adk::Status commandChanged = recorder.update (
            adk::TimePoint (10), input, buttons, guarded.images, 2);
        assert (!commandChanged.ok ());
        assert (result (recorder).recordCount == 1);

        adk::InertialRecord changed = input;
        ++changed.accelerationMicroG.x;
        buttons = control (changed);

        const adk::Status changedStatus = recorder.update (
            adk::TimePoint (5), changed, buttons, guarded.images, 2);

        assert (!changedStatus.ok ());
        assert (result (recorder).recordCount == 1);
        assert (guarded.before == UINT32_C (0x13579bdf));
        assert (guarded.after == UINT32_C (0x2468ace0));
        for (uint16_t index = 0; index < adk::MotionRecordImage::capacity; ++index)
        {
            assert (guarded.images[0].bytes[index] == saved.bytes[index]);
        }
    }

    void testTimestampAndSessionSerialOrdering ()
    {
        adk::MotionRecordImage       wrapImages[2] = {};
        adk::QualifiedMotionRecorder wrapRecorder (config (2));

        assert (wrapRecorder.initialize (
            adk::TimePoint (UINT32_MAX - 1), 2).ok ());

        adk::InertialQualificationEvidence wrapQualification = qualification ();
        wrapQualification.acceptedSampleCount       = 2;
        wrapQualification.firstSequence             = UINT32_MAX - 2;
        wrapQualification.lastSequence              = UINT32_MAX - 1;
        wrapQualification.firstObservedAt           =
            adk::TimePoint (UINT32_MAX - 2);
        wrapQualification.lastObservedAt            =
            adk::TimePoint (UINT32_MAX - 1);
        wrapQualification.accelerationSumsMicroG.z  = 2000000;
        wrapQualification.terminalRecord.sequence   = UINT32_MAX - 1;
        wrapQualification.mappedRecord.sequence     = UINT32_MAX - 1;
        wrapQualification.terminalRecord.observedAt =
            adk::TimePoint (UINT32_MAX - 1);
        wrapQualification.mappedRecord.observedAt   =
            adk::TimePoint (UINT32_MAX - 1);
        assert (wrapRecorder
                    .qualify (adk::TimePoint (UINT32_MAX - 1),
                              wrapQualification)
                    .ok ());
        assert (wrapRecorder.begin (
            adk::TimePoint (UINT32_MAX - 1), 17).ok ());

        adk::InertialRecord        first   = record  (UINT32_MAX, UINT32_MAX);
        adk::MotionRecorderControl buttons = control (first);
        buttons.qualificationDigest =
            adk::motionQualificationDigest (wrapQualification);

        adk::Status updateStatus = wrapRecorder.update (
            adk::TimePoint (0), first, buttons, wrapImages, 2);
        assert (updateStatus.ok ());

        adk::InertialRecord second = record  (0, 0);
        buttons                    = control (second);
        buttons.qualificationDigest =
            adk::motionQualificationDigest (wrapQualification);

        updateStatus = wrapRecorder.update (
            adk::TimePoint (1), second, buttons, wrapImages, 2);
        assert (updateStatus.ok ());
        assert (result (wrapRecorder).recordCount == 2);

        adk::MotionRecordImage       orderImages[2] = {};
        adk::QualifiedMotionRecorder orderRecorder (config (2));

        prepare (orderRecorder, 2);

        first   = record  (4, 5);
        buttons = control (first);

        updateStatus = orderRecorder.update (
            adk::TimePoint (5), first, buttons, orderImages, 2);
        assert (updateStatus.ok ());

        second  = record  (5, 4);
        buttons = control (second);

        updateStatus = orderRecorder.update (
            adk::TimePoint (4), second, buttons, orderImages, 2);
        assert (!updateStatus.ok ());

        second            = record  (5, UINT32_C (0x80000005));
        buttons           = control (second);

        updateStatus = orderRecorder.update (
            adk::TimePoint (UINT32_C (0x80000005)), second, buttons,
            orderImages, 2);
        assert (!updateStatus.ok ());
        assert (result (orderRecorder).recordCount == 1);

        assert (orderRecorder.reset (adk::TimePoint (6)).ok ());
        assert (orderRecorder.qualify (adk::TimePoint (7), qualification ()).ok ());
        assert (!orderRecorder.begin (adk::TimePoint (8), 17).ok ());
        assert (!orderRecorder.begin (adk::TimePoint (8), 16).ok ());

        const adk::Status halfRangeSession = orderRecorder.begin (
            adk::TimePoint (8), UINT32_C (0x80000011));

        assert (!halfRangeSession.ok ());
        assert (orderRecorder.begin (adk::TimePoint (8), 18).ok ());
    }
} // namespace

int main ()
{
    testQualificationCorrelationDomain ();

    testLifecycleAndExportHandshake ();

    testCapacityFailureIsAtomic ();

    testCorrelationAndFaultPresentation ();

    testResetDominatesAndReplayIsDeterministic ();

    testDuplicateAndOrientationFailureAreAtomic ();

    testTimestampAndSessionSerialOrdering ();
    return 0;
}
