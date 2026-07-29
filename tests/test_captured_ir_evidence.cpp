#include <captured_ir_evidence.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace {

    constexpr uint8_t necPulseCount = 67;

    adk::Pulse pulse (adk::PulseLevel level, uint32_t duration)
    {
        return adk::Pulse (level, adk::MicrosecondDuration (duration));
    }

    adk::IrSourceIdentity
    source (uint8_t sourceId = 1, uint16_t revision = 2, uint32_t epoch = 3,
            adk::IrSourceKind kind = adk::IrSourceKind::SyntheticFixture)
    {
        const adk::IrSourceIdentity result = {kind, sourceId, revision, epoch};
        return result;
    }

    adk::PulseFrame makeNec (adk::Pulse* storage, uint8_t address, uint8_t command,
                             uint32_t sequence)
    {
        const uint32_t bits =
            static_cast<uint32_t> (address) |
            (static_cast<uint32_t> (static_cast<uint8_t> (~address)) << 8U) |
            (static_cast<uint32_t> (command) << 16U) |
            (static_cast<uint32_t> (static_cast<uint8_t> (~command)) << 24U);

        storage[0] = pulse (adk::PulseLevel::Mark, 9000);
        storage[1] = pulse (adk::PulseLevel::Space, 4500);
        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            storage[2 + bit * 2] = pulse (adk::PulseLevel::Mark, 560);
            storage[3 + bit * 2] =
                pulse (adk::PulseLevel::Space,
                       (bits & (UINT32_C (1) << bit)) != 0U ? 1690 : 560);
        }
        storage[66] = pulse (adk::PulseLevel::Mark, 560);

        const adk::PulseFrame frame = {storage, necPulseCount, sequence,
                                       adk::CaptureState::Complete};
        return frame;
    }

    adk::PulseFrame makeRepeat (adk::Pulse* storage, uint32_t sequence)
    {
        storage[0]                  = pulse (adk::PulseLevel::Mark, 9000);
        storage[1]                  = pulse (adk::PulseLevel::Space, 2250);
        storage[2]                  = pulse (adk::PulseLevel::Mark, 560);
        const adk::PulseFrame frame = {storage, 3, sequence,
                                       adk::CaptureState::Complete};
        return frame;
    }

    bool sameSource (const adk::IrSourceIdentity& left,
                     const adk::IrSourceIdentity& right)
    {
        return left.kind == right.kind && left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.sessionEpoch == right.sessionEpoch;
    }

    bool sameSnapshot (const adk::CapturedIrSnapshot& left,
                       const adk::CapturedIrSnapshot& right)
    {
        return left.disposition == right.disposition &&
               left.strength == right.strength &&
               sameSource                              (left.provenance.source, right.provenance.source) &&
               left.provenance.observedAt.microseconds () ==
                   right.provenance.observedAt.microseconds () &&
               left.provenance.captureSequence == right.provenance.captureSequence &&
               left.provenance.captureState == right.provenance.captureState &&
               left.provenance.protocol == right.provenance.protocol &&
               left.provenance.decoderValidity == right.provenance.decoderValidity &&
               left.provenance.sourceStatus == right.provenance.sourceStatus &&
               left.address == right.address && left.command == right.command &&
               left.evidenceGeneration == right.evidenceGeneration &&
               left.pulseCount == right.pulseCount && left.status == right.status;
    }

    struct Fixture
    {
        Fixture (uint8_t capacity = adk::capturedIrPulseCapacity,
                 uint8_t maximum  = adk::capturedIrPulseCapacity) noexcept
            : decoder (), words (),
              evidence (decoder, adk::IrPulseStorage{words, capacity}, maximum)
        {
        }

        adk::InfraredDecoder    decoder;
        uint32_t                words[adk::capturedIrPulseCapacity];
        adk::CapturedIrEvidence evidence;
    };

    void testPublicShapeAndConstructionAreInert ()
    {
        static_assert (adk::capturedIrPulseCapacity == adk::PulseCapture::capacity,
                       "copied evidence preserves the Lesson 025 maximum");
        static_assert (adk::capturedIrMarkMask == UINT32_C (0x80000000),
                       "mark occupies the high bit");
        static_assert (adk::capturedIrDurationMask == UINT32_C (0x7fffffff),
                       "duration occupies the low 31 bits");
        static_assert (!std::is_copy_constructible<adk::CapturedIrEvidence>::value,
                       "evidence is not copy constructible");
        static_assert (!std::is_copy_assignable<adk::CapturedIrEvidence>::value,
                       "evidence is not copy assignable");
        static_assert (!std::is_move_constructible<adk::CapturedIrEvidence>::value,
                       "evidence is not move constructible");
        static_assert (!std::is_move_assignable<adk::CapturedIrEvidence>::value,
                       "evidence is not move assignable");

        Fixture                       fixture;
        const adk::CapturedIrSnapshot snapshot = fixture.evidence.snapshot ();
        assert                                                             (snapshot.disposition == adk::IrCaptureDisposition::None);
        assert                                                             (snapshot.strength == adk::EvidenceStrength::None);
        assert                                                             (snapshot.address == 0 && snapshot.command == 0);
        assert                                                             (snapshot.evidenceGeneration == 0 && snapshot.pulseCount == 0);
        assert                                                             (fixture.evidence.view ().error () == adk::StatusCode::NotInitialized);
    }

    void testConfigurationCapacityMatrix ()
    {
        adk::InfraredDecoder decoder;
        uint32_t             words[adk::capturedIrPulseCapacity + 1] = {};

        const adk::IrPulseStorage stores[] = {
            {nullptr, 0}, {words, 1}, {words, 99}, {words, 100}, {words, 101}};
        const uint8_t maxima[] = {0, 1, 99, 100, 101};

        for (uint8_t storeIndex = 0; storeIndex < 5; ++storeIndex)
        {
            for (uint8_t maximumIndex = 0; maximumIndex < 5; ++maximumIndex)
            {
                adk::CapturedIrEvidence evidence (decoder, stores[storeIndex],
                                                  maxima[maximumIndex]);
                const bool              valid =
                    stores[storeIndex].capacity >= 1 &&
                    stores[storeIndex].capacity <= adk::capturedIrPulseCapacity &&
                    maxima[maximumIndex] >= 1 &&
                    maxima[maximumIndex] <= stores[storeIndex].capacity;
                assert            (evidence.initialize ().ok () == valid);
                evidence.shutdown ();
            }
        }

        adk::CapturedIrEvidence nullStorage (decoder, adk::IrPulseStorage{nullptr, 1},
                                             1);
        assert (nullStorage.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
    }

    void testLifecycleIsIdempotentAndInvalidatesViews ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        assert (fixture.evidence.initialize ().ok ());

        adk::Pulse      pulses[necPulseCount];
        adk::PulseFrame frame = makeNec (pulses, 0x12, 0xa5, 7);
        assert                          (fixture.evidence
                    .admit (frame, source (), adk::Status (),
                            adk::MicrosecondTimePoint (100))
                    .ok ());
        const adk::Result<adk::CapturedIrView> live = fixture.evidence.view ();
        assert                                                              (live.ok ());

        fixture.evidence.shutdown ();
        assert                    (fixture.evidence.view ().error () == adk::StatusCode::NotInitialized);
        assert                    (fixture.evidence.requiredWords (live.value ()).error () ==
                adk::StatusCode::NotInitialized);
        fixture.evidence.shutdown ();

        assert                 (fixture.evidence.initialize ().ok ());
        assert                 (fixture.evidence.snapshot ().evidenceGeneration == 0);
        fixture.evidence.reset ();
        assert                 (fixture.evidence.snapshot ().evidenceGeneration == 0);
    }

    void testKnownFrameCopiesWordsAndFullProvenance ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());

        adk::Pulse                  pulses[necPulseCount];
        adk::PulseFrame             frame    = makeNec (pulses, 0x12, 0xa5, 17);
        const adk::IrSourceIdentity identity = source  (9, 41, 73);
        assert                                         (fixture.evidence
                    .admit (frame, identity, adk::Status (),
                            adk::MicrosecondTimePoint (123456))
                    .ok ());

        const adk::CapturedIrSnapshot snapshot = fixture.evidence.snapshot ();
        assert                                                             (snapshot.disposition == adk::IrCaptureDisposition::KnownValid);
        assert                                                             (snapshot.strength == adk::EvidenceStrength::IntegrityVerified);
        assert                                                             (sameSource (snapshot.provenance.source, identity));
        assert                                                             (snapshot.provenance.observedAt.microseconds () == 123456);
        assert                                                             (snapshot.provenance.captureSequence == 17);
        assert                                                             (snapshot.provenance.captureState == adk::CaptureState::Complete);
        assert                                                             (snapshot.provenance.protocol == adk::InfraredProtocol::Nec);
        assert                                                             (snapshot.provenance.decoderValidity == adk::FrameValidity::Valid);
        assert                                                             (snapshot.provenance.sourceStatus.ok ());
        assert                                                             (snapshot.address == 0x12 && snapshot.command == 0xa5);
        assert                                                             (snapshot.evidenceGeneration != 0);
        assert                                                             (snapshot.pulseCount == necPulseCount);
        assert                                                             (snapshot.status.ok ());

        const adk::Result<adk::CapturedIrView> view = fixture.evidence.view ();
        assert                                                              (view.ok () && view.value ().size == necPulseCount);
        assert                                                              (view.value ().owner == &fixture.evidence);
        assert                                                              (view.value ().evidenceGeneration == snapshot.evidenceGeneration);
        assert                                                              (view.value ().words == fixture.words);
        assert                                                              (view.value ().words[0] == (adk::capturedIrMarkMask | UINT32_C (9000)));
        assert                                                              (view.value ().words[1] == UINT32_C (4500));
        assert                                                              (view.value ().words[66] == (adk::capturedIrMarkMask | UINT32_C (560)));

        pulses[0] = pulse (adk::PulseLevel::Space, 1);
        assert            (view.value ().words[0] == (adk::capturedIrMarkMask | UINT32_C (9000)));
    }

    void testZeroPulseEvidenceIsAValidCopiedRecord ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());

        const adk::PulseFrame frame = {nullptr, 0, 1, adk::CaptureState::Complete};
        assert (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (1))
                .ok    ());

        const adk::CapturedIrSnapshot snapshot = fixture.evidence.snapshot ();
        assert                                                             (snapshot.disposition == adk::IrCaptureDisposition::Truncated);
        assert                                                             (snapshot.strength == adk::EvidenceStrength::None);
        assert                                                             (snapshot.pulseCount == 0);

        const adk::Result<adk::CapturedIrView> view = fixture.evidence.view ();
        assert                                                              (view.ok ());
        assert                                                              (view.value ().words == fixture.words);
        assert                                                              (view.value ().size == 0);
        assert                                                              (fixture.evidence.requiredWords (view.value ()).value () == 0);
        assert                                                              (fixture.evidence.exportWords (view.value (), {nullptr, 0}).ok ());
    }

    void testComponentsUseDisjointCallerOwnedStorage ()
    {
        adk::InfraredDecoder    leftDecoder;
        adk::InfraredDecoder    rightDecoder;
        uint32_t                leftWords[adk::capturedIrPulseCapacity]  = {};
        uint32_t                rightWords[adk::capturedIrPulseCapacity] = {};
        adk::CapturedIrEvidence left (leftDecoder,
                                      {leftWords, adk::capturedIrPulseCapacity},
                                      adk::capturedIrPulseCapacity);
        adk::CapturedIrEvidence right (rightDecoder,
                                       {rightWords, adk::capturedIrPulseCapacity},
                                       adk::capturedIrPulseCapacity);
        assert (left.initialize ().ok ());
        assert (right.initialize ().ok ());

        adk::Pulse            leftPulses[necPulseCount];
        adk::Pulse            rightPulses[necPulseCount];
        const adk::PulseFrame leftFrame  = makeNec (leftPulses, 1, 2, 1);
        const adk::PulseFrame rightFrame = makeNec (rightPulses, 3, 4, 1);
        assert                                     (left.admit (leftFrame, source (1), adk::Status (),
                            adk::MicrosecondTimePoint (1))
                    .ok ());
        assert (right
                    .admit (rightFrame, source (2), adk::Status (),
                            adk::MicrosecondTimePoint (1))
                    .ok ());

        assert (left.view ().value ().words == leftWords);
        assert (right.view ().value ().words == rightWords);
        assert (left.view ().value ().words != right.view ().value ().words);
        assert (left.snapshot ().address == 1 && left.snapshot ().command == 2);
        assert (right.snapshot ().address == 3 && right.snapshot ().command == 4);
    }

    void testRepeatClearsPriorDecodedCommand ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());

        adk::Pulse      pulses[necPulseCount];
        adk::PulseFrame frame = makeNec (pulses, 0x31, 0x72, 1);
        assert                          (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (1))
                .ok    ());
        assert (fixture.evidence.snapshot ().address == 0x31);
        assert (fixture.evidence.snapshot ().command == 0x72);

        frame = makeRepeat (pulses, 2);
        assert             (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (2))
                .ok    ());
        const adk::CapturedIrSnapshot repeat = fixture.evidence.snapshot ();
        assert                                                           (repeat.disposition == adk::IrCaptureDisposition::KnownRepeat);
        assert                                                           (repeat.strength == adk::EvidenceStrength::ShapeRecognized);
        assert                                                           (repeat.address == 0 && repeat.command == 0);
    }

    void testRepeatUnknownAndMalformedCategoriesAreNonAuthoritative ()
    {
        struct Expected
        {
            adk::IrCaptureDisposition disposition;
            adk::EvidenceStrength     strength;
            adk::InfraredProtocol     protocol;
            adk::FrameValidity        validity;
        };

        for (uint8_t caseIndex = 0; caseIndex < 7; ++caseIndex)
        {
            Fixture         fixture;
            adk::Pulse      pulses[necPulseCount + 1] = {};
            adk::PulseFrame frame                     = {};
            Expected        expected                  = {};
            assert (fixture.evidence.initialize ().ok ());

            if (caseIndex == 0)
            {
                frame    = makeRepeat (pulses, 1);
                expected = {adk::IrCaptureDisposition::KnownRepeat,
                            adk::EvidenceStrength::ShapeRecognized,
                            adk::InfraredProtocol::Nec, adk::FrameValidity::Repeat};
            }
            else if (caseIndex == 1)
            {
                pulses[0] = pulse (adk::PulseLevel::Mark, 100);
                pulses[1] = pulse (adk::PulseLevel::Space, 100);
                frame     = {pulses, 2, 1, adk::CaptureState::Complete};
                expected = {adk::IrCaptureDisposition::UnknownProtocol,
                            adk::EvidenceStrength::None, adk::InfraredProtocol::Unknown,
                            adk::FrameValidity::UnknownProtocol};
            }
            else if (caseIndex == 2)
            {
                frame              = makeNec                  (pulses, 1, 2, 1);
                pulses[2].duration = adk::MicrosecondDuration (701);
                expected           = {adk::IrCaptureDisposition::TimingInvalid,
                                      adk::EvidenceStrength::ShapeRecognized,
                                      adk::InfraredProtocol::Nec,
                                      adk::FrameValidity::TimingInvalid};
            }
            else if (caseIndex == 3)
            {
                frame               = makeNec                  (pulses, 0x44, 0x20, 1);
                pulses[19].duration = adk::MicrosecondDuration (560);
                expected            = {adk::IrCaptureDisposition::IntegrityInvalid,
                                       adk::EvidenceStrength::ShapeRecognized,
                                       adk::InfraredProtocol::Nec,
                                       adk::FrameValidity::IntegrityInvalid};
            }
            else if (caseIndex == 4)
            {
                frame      = makeNec (pulses, 1, 2, 1);
                frame.size = 10;
                expected = {adk::IrCaptureDisposition::Truncated,
                            adk::EvidenceStrength::ShapeRecognized,
                            adk::InfraredProtocol::Nec, adk::FrameValidity::Truncated};
            }
            else if (caseIndex == 5)
            {
                frame      = makeNec (pulses, 1, 2, 1);
                pulses[67] = pulse   (adk::PulseLevel::Space, 560);
                frame.size = 68;
                expected   = {adk::IrCaptureDisposition::DecoderOverflow,
                              adk::EvidenceStrength::ShapeRecognized,
                              adk::InfraredProtocol::Nec, adk::FrameValidity::Overflow};
            }
            else
            {
                frame    = {nullptr, 0, 1, adk::CaptureState::Complete};
                expected = {adk::IrCaptureDisposition::Truncated,
                            adk::EvidenceStrength::None, adk::InfraredProtocol::Unknown,
                            adk::FrameValidity::Truncated};
            }

            assert (fixture.evidence
                        .admit (frame, source (), adk::Status (),
                                adk::MicrosecondTimePoint (5))
                        .ok ());
            const adk::CapturedIrSnapshot snapshot = fixture.evidence.snapshot ();
            assert                                                             (snapshot.disposition == expected.disposition);
            assert                                                             (snapshot.strength == expected.strength);
            assert                                                             (snapshot.provenance.protocol == expected.protocol);
            assert                                                             (snapshot.provenance.decoderValidity == expected.validity);
            assert                                                             (snapshot.address == 0 && snapshot.command == 0);
        }
    }

    void testCaptureFaultsAndSourceFaultRemainDistinct ()
    {
        struct CaptureCase
        {
            adk::CaptureState         state;
            adk::IrCaptureDisposition disposition;
            adk::FrameValidity        validity;
        };
        const CaptureCase cases[] = {{adk::CaptureState::Overflow,
                                      adk::IrCaptureDisposition::CaptureOverflow,
                                      adk::FrameValidity::Overflow},
                                     {adk::CaptureState::TimingFault,
                                      adk::IrCaptureDisposition::CaptureTimingFault,
                                      adk::FrameValidity::TimingInvalid}};

        for (uint8_t index = 0; index < 2; ++index)
        {
            Fixture fixture;
            assert                             (fixture.evidence.initialize ().ok ());
            adk::Pulse      pulses[2] = {pulse (adk::PulseLevel::Mark, 9000),
                                         pulse (adk::PulseLevel::Space, 4500)};
            adk::PulseFrame frame     = {pulses, 2, 10, cases[index].state};
            assert (fixture.evidence
                        .admit (frame, source (), adk::Status (),
                                adk::MicrosecondTimePoint (50))
                        .ok ());
            const adk::CapturedIrSnapshot snapshot = fixture.evidence.snapshot ();
            assert                                                             (snapshot.disposition == cases[index].disposition);
            assert                                                             (snapshot.strength == adk::EvidenceStrength::None);
            assert                                                             (snapshot.provenance.decoderValidity == cases[index].validity);
            assert                                                             (snapshot.address == 0 && snapshot.command == 0);
        }

        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        adk::Pulse        pulses[necPulseCount];
        adk::PulseFrame   frame         = makeNec (pulses, 9, 10, 4);
        const adk::Status sourceFailure = adk::StatusCode::HardwareFailure;
        assert (
            fixture.evidence
                .admit (frame, source (), sourceFailure, adk::MicrosecondTimePoint (99))
                .ok    ());
        const adk::CapturedIrSnapshot failed = fixture.evidence.snapshot ();
        assert                                                           (failed.disposition == adk::IrCaptureDisposition::SourceFault);
        assert                                                           (failed.strength == adk::EvidenceStrength::None);
        assert                                                           (failed.provenance.sourceStatus == sourceFailure);
        assert                                                           (failed.address == 0 && failed.command == 0);
    }

    void testPulseCountsAndCompactDurationBoundaries ()
    {
        const uint8_t counts[] = {1, 99, 100};
        for (uint8_t countIndex = 0; countIndex < 3; ++countIndex)
        {
            Fixture fixture;
            assert (fixture.evidence.initialize ().ok ());
            adk::Pulse pulses[adk::capturedIrPulseCapacity];
            for (uint8_t index = 0; index < counts[countIndex]; ++index)
            {
                pulses[index] = pulse ((index & 1U) == 0U ? adk::PulseLevel::Mark
                                                          : adk::PulseLevel::Space,
                                       index == 0 ? 1 : adk::capturedIrDurationMask);
            }
            const adk::PulseFrame frame = {pulses, counts[countIndex], 1,
                                           adk::CaptureState::Overflow};
            assert (fixture.evidence
                        .admit (frame, source (), adk::Status (),
                                adk::MicrosecondTimePoint (1))
                        .ok ());
            const adk::CapturedIrView view = fixture.evidence.view ().value ();
            assert                                                 (view.size == counts[countIndex]);
            assert                                                 (view.words[0] == (adk::capturedIrMarkMask | UINT32_C (1)));
            if (counts[countIndex] > 1)
            {
                assert (view.words[1] == adk::capturedIrDurationMask);
            }
        }

        const uint32_t rejectedDurations[] = {0, adk::capturedIrDurationMask +
                                                     UINT32_C (1)};
        const uint8_t  rejectedPositions[] = {0, 49, 99};
        for (uint8_t durationIndex = 0; durationIndex < 2; ++durationIndex)
        {
            for (uint8_t positionIndex = 0; positionIndex < 3; ++positionIndex)
            {
                Fixture fixture;
                assert (fixture.evidence.initialize ().ok ());
                adk::Pulse      baselinePulses[necPulseCount];
                adk::PulseFrame baseline = makeNec (baselinePulses, 1, 2, 1);
                assert                             (fixture.evidence
                            .admit (baseline, source (), adk::Status (),
                                    adk::MicrosecondTimePoint (1))
                            .ok ());
                const adk::CapturedIrSnapshot before = fixture.evidence.snapshot ();

                adk::Pulse rejectedPulses[adk::capturedIrPulseCapacity];
                for (uint8_t index = 0; index < adk::capturedIrPulseCapacity; ++index)
                {
                    rejectedPulses[index] = pulse (adk::PulseLevel::Mark, index + 1U);
                }
                rejectedPulses[rejectedPositions[positionIndex]].duration =
                    adk::MicrosecondDuration (rejectedDurations[durationIndex]);
                const adk::PulseFrame rejected = {rejectedPulses,
                                                  adk::capturedIrPulseCapacity, 2,
                                                  adk::CaptureState::Overflow};
                assert (!fixture.evidence
                             .admit (rejected, source (), adk::Status (),
                                     adk::MicrosecondTimePoint (2))
                             .ok ());
                assert (sameSnapshot (before, fixture.evidence.snapshot ()));
            }
        }

        Fixture invalidLevel;
        assert                                 (invalidLevel.evidence.initialize ().ok ());
        adk::Pulse            badLevel = pulse (static_cast<adk::PulseLevel> (255), 1);
        const adk::PulseFrame invalid  = {&badLevel, 1, 1, adk::CaptureState::Overflow};
        assert (!invalidLevel.evidence
                     .admit (invalid, source (), adk::Status (),
                             adk::MicrosecondTimePoint (1))
                     .ok ());
        assert (invalidLevel.evidence.snapshot ().evidenceGeneration == 0);
    }

    void testInvalidFramesRejectWithoutMutation ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        adk::Pulse      pulses[necPulseCount];
        adk::PulseFrame baseline = makeNec (pulses, 1, 2, 1);
        assert                             (fixture.evidence
                    .admit (baseline, source (), adk::Status (),
                            adk::MicrosecondTimePoint (1))
                    .ok ());
        const adk::CapturedIrSnapshot before = fixture.evidence.snapshot ();

        adk::PulseFrame invalidFrames[] = {
            {nullptr, 1, 2, adk::CaptureState::Complete},
            {pulses, 101, 2, adk::CaptureState::Overflow},
            {pulses, 1, 2, adk::CaptureState::Idle},
            {pulses, 1, 2, adk::CaptureState::Capturing},
            {pulses, 1, 2, static_cast<adk::CaptureState> (255)}};
        for (uint8_t index = 0; index < 5; ++index)
        {
            assert (!fixture.evidence
                         .admit (invalidFrames[index], source (), adk::Status (),
                                 adk::MicrosecondTimePoint (2))
                         .ok ());
            assert (sameSnapshot (before, fixture.evidence.snapshot ()));
        }

        const adk::IrSourceIdentity invalidSources[] = {
            {adk::IrSourceKind::SyntheticFixture, 0, 1, 1},
            {adk::IrSourceKind::SyntheticFixture, 1, 0, 1},
            {adk::IrSourceKind::SyntheticFixture, 1, 1, 0},
            {static_cast<adk::IrSourceKind> (255), 1, 1, 1}};
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::PulseFrame next = makeNec (pulses, 1, 2, 2);
            assert                         (!fixture.evidence
                         .admit (next, invalidSources[index], adk::Status (),
                                 adk::MicrosecondTimePoint (2))
                         .ok ());
            assert (sameSnapshot (before, fixture.evidence.snapshot ()));
        }
    }

    void testDuplicateSequenceOrderingAndWraparound ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        adk::Pulse                  pulses[necPulseCount];
        const adk::IrSourceIdentity identity = source ();

        adk::PulseFrame frame =
            makeNec (pulses, 3, 4, std::numeric_limits<uint32_t>::max () - 1U);
        assert (
            fixture.evidence
                .admit (frame, identity, adk::Status (), adk::MicrosecondTimePoint (10))
                .ok    ());
        const adk::CapturedIrSnapshot first = fixture.evidence.snapshot ();

        assert (fixture.evidence
                    .admit (frame, identity, adk::Status (),
                            adk::MicrosecondTimePoint (999))
                    .ok ());
        assert (sameSnapshot (first, fixture.evidence.snapshot ()));

        pulses[2].duration = adk::MicrosecondDuration (561);
        assert                                        (!fixture.evidence
                     .admit (frame, identity, adk::Status (),
                             adk::MicrosecondTimePoint (11))
                     .ok ());
        assert                                        (sameSnapshot (first, fixture.evidence.snapshot ()));
        pulses[2].duration = adk::MicrosecondDuration (560);

        const adk::Status changedStatuses[] = {adk::StatusCode::HardwareFailure,
                                               adk::StatusCode::Timeout};
        for (uint8_t index = 0; index < 2; ++index)
        {
            assert (!fixture.evidence
                         .admit (frame, identity, changedStatuses[index],
                                 adk::MicrosecondTimePoint (10))
                         .ok ());
            assert (sameSnapshot (first, fixture.evidence.snapshot ()));
        }

        const adk::IrSourceIdentity changedSources[] = {
            source (2), source (1, 3), source (1, 2, 4),
            source (1, 2, 3, adk::IrSourceKind::QualifiedReceiver)};
        for (uint8_t index = 0; index < 4; ++index)
        {
            assert (!fixture.evidence
                         .admit (frame, changedSources[index], adk::Status (),
                                 adk::MicrosecondTimePoint (10))
                         .ok ());
            assert (sameSnapshot (first, fixture.evidence.snapshot ()));
        }

        adk::Pulse            repeatPulses[3];
        const adk::PulseFrame changedClassification =
            makeRepeat (repeatPulses, frame.sequence);
        assert (!fixture.evidence
                     .admit (changedClassification, identity, adk::Status (),
                             adk::MicrosecondTimePoint (10))
                     .ok ());
        assert (sameSnapshot (first, fixture.evidence.snapshot ()));

        frame = makeNec (pulses, 3, 4, 0);
        assert          (
            fixture.evidence
                .admit (frame, identity, adk::Status (), adk::MicrosecondTimePoint (12))
                .ok    ());
        const adk::CapturedIrSnapshot wrapped = fixture.evidence.snapshot ();
        assert                                                            (wrapped.provenance.captureSequence == 0);
        assert                                                            (wrapped.evidenceGeneration != first.evidenceGeneration);

        const uint32_t rejectedSequences[] = {std::numeric_limits<uint32_t>::max (),
                                              UINT32_C (0x80000000)};
        for (uint8_t index = 0; index < 2; ++index)
        {
            frame = makeNec (pulses, 3, 4, rejectedSequences[index]);
            assert          (!fixture.evidence
                         .admit (frame, identity, adk::Status (),
                                 adk::MicrosecondTimePoint (13))
                         .ok ());
            assert (sameSnapshot (wrapped, fixture.evidence.snapshot ()));
        }
    }

    void testIdentityChangesRequireReset ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        adk::Pulse      pulses[necPulseCount];
        adk::PulseFrame frame = makeNec (pulses, 1, 2, 1);
        assert                          (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (1))
                .ok    ());
        const adk::CapturedIrSnapshot before = fixture.evidence.snapshot ();

        const adk::IrSourceIdentity changed[] = {
            source (2), source (1, 3), source (1, 2, 4),
            source (1, 2, 3, adk::IrSourceKind::QualifiedReceiver)};
        frame = makeNec (pulses, 1, 2, 2);
        for (uint8_t index = 0; index < 4; ++index)
        {
            assert (!fixture.evidence
                         .admit (frame, changed[index], adk::Status (),
                                 adk::MicrosecondTimePoint (2))
                         .ok ());
            assert (sameSnapshot (before, fixture.evidence.snapshot ()));
        }

        fixture.evidence.reset ();
        assert                 (fixture.evidence
                    .admit (frame, changed[0], adk::Status (),
                            adk::MicrosecondTimePoint (2))
                    .ok ());
        assert (
            sameSource (fixture.evidence.snapshot ().provenance.source, changed[0]));
    }

    void testObservationTimeWrapsButRegressionAndHalfRangeReject ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        adk::Pulse pulses[necPulseCount];

        adk::PulseFrame frame = makeNec (pulses, 1, 2, 1);
        assert                          (fixture.evidence
                    .admit (frame, source (), adk::Status (),
                            adk::MicrosecondTimePoint (
                                std::numeric_limits<uint32_t>::max () - 1U))
                    .ok ());

        frame = makeNec (pulses, 1, 2, 2);
        assert          (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (0))
                .ok    ());
        const adk::CapturedIrSnapshot wrapped = fixture.evidence.snapshot ();
        assert                                                            (wrapped.provenance.observedAt.microseconds () == 0);

        frame = makeNec (pulses, 1, 2, 3);
        assert          (!fixture.evidence
                     .admit (frame, source (), adk::Status (),
                             adk::MicrosecondTimePoint (
                                 std::numeric_limits<uint32_t>::max ()))
                     .ok ());
        assert (sameSnapshot (wrapped, fixture.evidence.snapshot ()));

        assert (!fixture.evidence
                     .admit (frame, source (), adk::Status (),
                             adk::MicrosecondTimePoint (UINT32_C (0x80000000)))
                     .ok ());
        assert (sameSnapshot (wrapped, fixture.evidence.snapshot ()));
    }

    void testViewRequiredWordsAndExportAreTransactional ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());
        adk::Pulse      pulses[necPulseCount];
        adk::PulseFrame frame = makeNec (pulses, 0x21, 0x84, 1);
        assert                          (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (1))
                .ok    ());
        const adk::CapturedIrView  view     = fixture.evidence.view          ().value ();
        const adk::Result<uint8_t> required = fixture.evidence.requiredWords (view);
        assert                                                               (required.ok () && required.value () == necPulseCount);

        uint32_t shortStorage[necPulseCount] = {};
        for (uint8_t index = 0; index < necPulseCount; ++index)
        {
            shortStorage[index] = UINT32_C (0x5a5a5a5a);
        }
        assert (fixture.evidence
                    .exportWords (view,
                                  adk::IrPulseStorage{shortStorage, necPulseCount - 1})
                    .error () == adk::StatusCode::CapacityExceeded);
        for (uint8_t index = 0; index < necPulseCount; ++index)
        {
            assert (shortStorage[index] == UINT32_C (0x5a5a5a5a));
        }
        assert (fixture.evidence.exportWords (view, adk::IrPulseStorage{nullptr, 0})
                    .error () == adk::StatusCode::CapacityExceeded);

        uint32_t                   exact[necPulseCount] = {};
        const adk::Result<uint8_t> exactResult          = fixture.evidence.exportWords (
            view, adk::IrPulseStorage{exact, necPulseCount});
        assert (exactResult.ok () && exactResult.value () == necPulseCount);
        for (uint8_t index = 0; index < necPulseCount; ++index)
        {
            assert (exact[index] == view.words[index]);
        }

        uint32_t                   large[adk::capturedIrPulseCapacity] = {};
        const adk::Result<uint8_t> largeResult = fixture.evidence.exportWords (
            view, adk::IrPulseStorage{large, adk::capturedIrPulseCapacity});
        assert (largeResult.ok () && largeResult.value () == necPulseCount);

        adk::CapturedIrView forged = view;
        forged.owner               = nullptr;
        assert (!fixture.evidence.requiredWords (forged).ok ());
        assert (
            !fixture.evidence
                 .exportWords (forged,
                               adk::IrPulseStorage{large, adk::capturedIrPulseCapacity})
                 .ok ());

        Fixture foreign;
        assert (foreign.evidence.initialize ().ok ());
        assert (!foreign.evidence.requiredWords (view).ok ());
        assert (!foreign.evidence
                     .exportWords (
                         view, adk::IrPulseStorage{large, adk::capturedIrPulseCapacity})
                     .ok ());

        frame = makeNec (pulses, 0x21, 0x84, 2);
        assert          (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (2))
                .ok    ());
        assert (!fixture.evidence.requiredWords (view).ok ());
        assert (!fixture.evidence
                     .exportWords (
                         view, adk::IrPulseStorage{large, adk::capturedIrPulseCapacity})
                     .ok ());
    }

    void testMaximumCountAndStorageFailuresPreservePriorRecord ()
    {
        Fixture fixture (99, 99);
        assert          (fixture.evidence.initialize ().ok ());

        adk::Pulse pulses[adk::capturedIrPulseCapacity];
        for (uint8_t index = 0; index < adk::capturedIrPulseCapacity; ++index)
        {
            pulses[index] = pulse (adk::PulseLevel::Mark, 1);
        }
        adk::PulseFrame frame = {pulses, 99, 1, adk::CaptureState::Overflow};
        assert (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (1))
                .ok    ());
        const adk::CapturedIrSnapshot before = fixture.evidence.snapshot ();

        frame.size     = 100;
        frame.sequence = 2;
        assert (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (2))
                .error () == adk::StatusCode::CapacityExceeded);
        assert (sameSnapshot (before, fixture.evidence.snapshot ()));
    }

    void testFailedAdmissionDoesNotAlterBackingStorage ()
    {
        Fixture fixture;
        assert (fixture.evidence.initialize ().ok ());

        adk::Pulse      pulses[necPulseCount];
        adk::PulseFrame frame = makeNec (pulses, 1, 2, 1);
        assert                          (
            fixture.evidence
                .admit (frame, source (), adk::Status (), adk::MicrosecondTimePoint (1))
                .ok    ());

        uint32_t before[adk::capturedIrPulseCapacity] = {};
        for (uint8_t index = 0; index < adk::capturedIrPulseCapacity; ++index)
        {
            before[index] = fixture.words[index];
        }

        frame               = makeNec                  (pulses, 3, 4, 2);
        pulses[40].duration = adk::MicrosecondDuration (0);
        assert                                         (!fixture.evidence
                     .admit (frame, source (), adk::Status (),
                             adk::MicrosecondTimePoint (2))
                     .ok ());

        for (uint8_t index = 0; index < adk::capturedIrPulseCapacity; ++index)
        {
            assert (fixture.words[index] == before[index]);
        }
    }

    void testReplayProducesIdenticalFieldsAndWords ()
    {
        Fixture left;
        Fixture right;
        assert (left.evidence.initialize ().ok ());
        assert (right.evidence.initialize ().ok ());
        adk::Pulse            leftPulses[necPulseCount];
        adk::Pulse            rightPulses[necPulseCount];
        const adk::PulseFrame leftFrame  = makeNec (leftPulses, 0x55, 0xaa, 45);
        const adk::PulseFrame rightFrame = makeNec (rightPulses, 0x55, 0xaa, 45);

        assert (left.evidence
                    .admit (leftFrame, source (), adk::Status (),
                            adk::MicrosecondTimePoint (999))
                    .ok ());
        assert (right.evidence
                    .admit (rightFrame, source (), adk::Status (),
                            adk::MicrosecondTimePoint (999))
                    .ok ());
        assert                                                    (sameSnapshot (left.evidence.snapshot (), right.evidence.snapshot ()));
        const adk::CapturedIrView leftView  = left.evidence.view  ().value ();
        const adk::CapturedIrView rightView = right.evidence.view ().value ();
        assert                                                    (leftView.size == rightView.size);
        for (uint8_t index = 0; index < leftView.size; ++index)
        {
            assert (leftView.words[index] == rightView.words[index]);
        }
    }
} // namespace

int main ()
{
    testPublicShapeAndConstructionAreInert                     ();
    testConfigurationCapacityMatrix                            ();
    testLifecycleIsIdempotentAndInvalidatesViews               ();
    testKnownFrameCopiesWordsAndFullProvenance                 ();
    testZeroPulseEvidenceIsAValidCopiedRecord                  ();
    testComponentsUseDisjointCallerOwnedStorage                ();
    testRepeatClearsPriorDecodedCommand                        ();
    testRepeatUnknownAndMalformedCategoriesAreNonAuthoritative ();
    testCaptureFaultsAndSourceFaultRemainDistinct              ();
    testPulseCountsAndCompactDurationBoundaries                ();
    testInvalidFramesRejectWithoutMutation                     ();
    testDuplicateSequenceOrderingAndWraparound                 ();
    testIdentityChangesRequireReset                            ();
    testObservationTimeWrapsButRegressionAndHalfRangeReject    ();
    testViewRequiredWordsAndExportAreTransactional             ();
    testMaximumCountAndStorageFailuresPreservePriorRecord      ();
    testFailedAdmissionDoesNotAlterBackingStorage              ();
    testReplayProducesIdenticalFieldsAndWords                  ();
}
