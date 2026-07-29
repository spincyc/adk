#include <inert_ir_translator.h>

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace {

    constexpr uint8_t necPulseCount = 67;

    adk::IrSourceIdentity receiveSource () noexcept
    {
        return adk::syntheticIrReceiveSource;
    }

    adk::IrSourceIdentity emitterSource () noexcept
    {
        return {adk::IrSourceKind::SyntheticFixture, 2, 4, 5};
    }

    adk::IrTranslatorConfig config () noexcept
    {
        return {7,
                11,
                adk::syntheticIrMappingDigest,
                adk::MicrosecondDuration (67980),
                receiveSource            (),
                emitterSource            (),
                adk::MicrosecondDuration (2000),
                adk::MicrosecondDuration (10000)};
    }

    adk::Pulse pulse (adk::PulseLevel level, uint32_t duration) noexcept
    {
        return adk::Pulse (level, adk::MicrosecondDuration (duration));
    }

    adk::PulseFrame makeNec (adk::Pulse* storage, uint8_t command,
                             uint32_t sequence) noexcept
    {
        const uint8_t address =
            static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].address);
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
        return {storage, necPulseCount, sequence, adk::CaptureState::Complete};
    }

#if ADK_IR_TRANSLATOR_TEST_PART == 1
    adk::PulseFrame makeRepeat (adk::Pulse* storage, uint32_t sequence) noexcept
    {
        storage[0] = pulse (adk::PulseLevel::Mark, 9000);
        storage[1] = pulse (adk::PulseLevel::Space, 2250);
        storage[2] = pulse (adk::PulseLevel::Mark, 560);
        return {storage, 3, sequence, adk::CaptureState::Complete};
    }
#endif

    adk::CapturedIrProvenance emptyProvenance () noexcept
    {
        return {{adk::IrSourceKind::SyntheticFixture, 0, 0, 0},
                adk::MicrosecondTimePoint (),
                0,
                adk::CaptureState::Idle,
                adk::InfraredProtocol::Unknown,
                adk::FrameValidity::UnknownProtocol,
                adk::Status ()};
    }

    adk::KnownIrEmissionPreview emptyEmissionPreview () noexcept
    {
        return {nullptr,
                0,
                0,
                0,
                0,
                0,
                adk::LocalIrCodeId::StationPing,
                {0, 0},
                0,
                adk::MicrosecondTimePoint (),
                adk::MicrosecondTimePoint (),
                adk::IrEnvelopeIntent::Inactive};
    }

    adk::IrTranslatorPreview emptyTranslatorPreview () noexcept
    {
        return {nullptr,
                0,
                0,
                0,
                {0, 0},
                0,
                0,
                0,
                0,
                emptyProvenance (),
                adk::LocalIrCodeId::StationPing,
                adk::LocalIrCodeId::StationPing,
                emptyEmissionPreview ()};
    }

    adk::IrTranslatorUpdateInput emptyUpdate (uint32_t now) noexcept
    {
        adk::IrTranslatorUpdateInput result;
        result.now                   = adk::MicrosecondTimePoint (now);
        result.cancelPresent         = false;
        result.cancelOperationId     = 0;
        result.commitPresent         = false;
        result.commitPreview         = emptyTranslatorPreview ();
        result.receivePresent        = false;
        result.receive.source = {
            adk::IrSourceKind::SyntheticFixture, 0, 0, 0};
        result.receive.sourceStatus = adk::Status               ();
        result.receive.observedAt   = adk::MicrosecondTimePoint ();
        result.receive.frame = {
            nullptr, 0, 0, adk::CaptureState::Idle};
        result.actualEmissionPresent = false;
        result.actualEmission.source = {
            adk::IrSourceKind::SyntheticFixture, 0, 0, 0};
        result.actualEmission.transactionId = 0;
        result.actualEmission.startedAt     = adk::MicrosecondTimePoint ();
        result.actualEmission.completedAt   = adk::MicrosecondTimePoint ();
        result.actualEmission.status        = adk::Status               ();
        return result;
    }

    adk::IrTranslatorUpdateInput
    receiveUpdate (uint32_t now, adk::PulseFrame frame,
                   adk::IrSourceIdentity source       = receiveSource (),
                   adk::Status           sourceStatus = adk::Status   ()) noexcept
    {
        adk::IrTranslatorUpdateInput result = emptyUpdate (now);
        result.receivePresent               = true;
        result.receive = {source, sourceStatus, adk::MicrosecondTimePoint (now), frame};
        return result;
    }

    bool sameSource (const adk::IrSourceIdentity& left,
                     const adk::IrSourceIdentity& right) noexcept
    {
        return left.kind == right.kind &&
               left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.sessionEpoch == right.sessionEpoch;
    }

    bool sameProvenance (const adk::CapturedIrProvenance& left,
                         const adk::CapturedIrProvenance& right) noexcept
    {
        return sameSource (left.source, right.source) &&
               left.observedAt.microseconds () ==
                   right.observedAt.microseconds () &&
               left.captureSequence == right.captureSequence &&
               left.captureState == right.captureState &&
               left.protocol == right.protocol &&
               left.decoderValidity == right.decoderValidity &&
               left.sourceStatus == right.sourceStatus;
    }

    bool sameCatalog (const adk::KnownIrCatalogIdentity& left,
                      const adk::KnownIrCatalogIdentity& right) noexcept
    {
        return left.revision == right.revision &&
               left.digest == right.digest;
    }

    bool sameSnapshot (const adk::IrTranslatorSnapshot& left,
                       const adk::IrTranslatorSnapshot& right) noexcept
    {
        return left.operationId == right.operationId &&
               left.receivedCode == right.receivedCode &&
               left.transmitCode == right.transmitCode &&
               sameProvenance (left.receiveProvenance,
                               right.receiveProvenance) &&
               sameSource (left.transmitSource, right.transmitSource) &&
               left.transmitTransactionId == right.transmitTransactionId &&
               left.disposition == right.disposition &&
               left.suppressedEchoCount == right.suppressedEchoCount &&
               left.transmitIntent == right.transmitIntent &&
               left.roundTrip.complete == right.roundTrip.complete &&
               left.roundTrip.operationId == right.roundTrip.operationId &&
               left.roundTrip.transmittedCode ==
                   right.roundTrip.transmittedCode &&
               sameCatalog (left.roundTrip.emissionCatalog,
                            right.roundTrip.emissionCatalog) &&
               sameSource (left.roundTrip.transmitSource,
                           right.roundTrip.transmitSource) &&
               left.roundTrip.actualStartedAt.microseconds () ==
                   right.roundTrip.actualStartedAt.microseconds () &&
               left.roundTrip.actualCompletedAt.microseconds () ==
                   right.roundTrip.actualCompletedAt.microseconds () &&
               left.roundTrip.receiveDisposition ==
                   right.roundTrip.receiveDisposition &&
               left.roundTrip.receiveStrength ==
                   right.roundTrip.receiveStrength &&
               sameProvenance (left.roundTrip.receiveProvenance,
                               right.roundTrip.receiveProvenance) &&
               left.roundTrip.receiveEvidenceGeneration ==
                   right.roundTrip.receiveEvidenceGeneration &&
               left.roundTrip.correlationDisposition ==
                   right.roundTrip.correlationDisposition &&
               left.roundTrip.elapsed.microseconds () ==
                   right.roundTrip.elapsed.microseconds () &&
               left.roundTrip.status == right.roundTrip.status &&
               left.status == right.status;
    }

#if ADK_IR_TRANSLATOR_TEST_PART == 1
    uint32_t digestByte (uint32_t digest, uint8_t value) noexcept
    {
        return (digest ^ value) * UINT32_C (0x01000193);
    }

    uint32_t digestLittleEndian (uint32_t digest, uint32_t value,
                                 uint8_t width) noexcept
    {
        for (uint8_t index = 0; index < width; ++index)
        {
            digest = digestByte (digest, static_cast<uint8_t> (value >> (index * 8U)));
        }
        return digest;
    }

    void testCanonicalFixtureDigest ()
    {
        assert (adk::syntheticIrReceiveSource.kind ==
                adk::IrSourceKind::SyntheticFixture);
        assert (adk::syntheticIrReceiveSource.sourceId == 52);
        assert (adk::syntheticIrReceiveSource.configurationRevision ==
                adk::syntheticIrMappingRevision);
        assert (adk::syntheticIrReceiveSource.sessionEpoch == 1);

        uint32_t digest = UINT32_C  (0x811c9dc5);
        digest = digestLittleEndian (digest, adk::syntheticIrMappingRevision, 2);
        digest = digestByte         (digest,
                             static_cast<uint8_t> (adk::syntheticIrReceiveSource.kind));
        digest = digestByte         (digest, adk::syntheticIrReceiveSource.sourceId);
        digest = digestLittleEndian (
            digest, adk::syntheticIrReceiveSource.configurationRevision, 2);
        digest =
            digestLittleEndian (digest, adk::syntheticIrReceiveSource.sessionEpoch, 4);
        for (uint8_t index = 0; index < adk::syntheticIrFixtureCount; ++index)
        {
            const adk::SyntheticIrReceiveFixture& fixture =
                adk::syntheticIrReceiveFixtures[index];
            digest = digestByte         (digest, static_cast<uint8_t> (fixture.protocol));
            digest = digestLittleEndian (digest, fixture.address, 4);
            digest = digestLittleEndian (digest, fixture.command, 4);
            digest = digestByte         (digest, static_cast<uint8_t> (fixture.receivedCode));
            digest =
                digestByte (digest, fixture.transmissionAllowed
                                        ? static_cast<uint8_t> (fixture.translatedCode)
                                        : UINT8_C (0xff));
        }
        assert (digest == UINT32_C (0xa8f94d6b));
        assert (digest == adk::syntheticIrMappingDigest);
    }
#endif

    adk::IrTranslatorPreview admitAndPrepare (adk::InertIrTranslator& translator,
                                              adk::Pulse* pulses, uint8_t command,
                                              uint32_t operationId,
                                              uint32_t now) noexcept
    {
        assert (
            translator
                .update (receiveUpdate (now, makeNec (pulses, command, operationId)))
                .ok     ());
        const adk::Result<adk::IrTranslatorPreview> preview =
            translator.prepareTranslation (operationId,
                                           adk::MicrosecondTimePoint (now));
        assert               (preview.ok ());
        return preview.value ();
    }

    void commit (adk::InertIrTranslator&         translator,
                 const adk::IrTranslatorPreview& preview, uint32_t now) noexcept
    {
        adk::IrTranslatorUpdateInput input = emptyUpdate (now);
        input.commitPresent                = true;
        input.commitPreview                = preview;
        assert (translator.update (input).ok ());
    }

#if ADK_IR_TRANSLATOR_TEST_PART == 1
    void testConstructionConfigurationAndLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::InertIrTranslator>::value,
                       "translator owns stable-address children and candidates");
        static_assert (!std::is_move_constructible<adk::InertIrTranslator>::value,
                       "translator cannot move while previews bind its address");

        uint32_t               storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (config (),
                                           {storage, adk::capturedIrPulseCapacity});
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Idle);
        assert (translator.update (emptyUpdate (0)).error () ==
                adk::StatusCode::NotInitialized);
        assert              (translator.initialize ().ok ());
        assert              (translator.initialize ().ok ());
        translator.shutdown ();
        translator.shutdown ();
        assert              (translator.update (emptyUpdate (1)).error () ==
                adk::StatusCode::NotInitialized);
        assert (translator.initialize ().ok ());

        adk::IrTranslatorConfig invalid = config ();
        invalid.instanceEpoch           = 0;
        adk::InertIrTranslator zeroEpoch (invalid,
                                          {storage, adk::capturedIrPulseCapacity});
        assert (zeroEpoch.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        invalid               = config ();
        invalid.mappingDigest = 1;
        adk::InertIrTranslator badMap (invalid,
                                       {storage, adk::capturedIrPulseCapacity});
        assert                                            (badMap.initialize ().error () == adk::StatusCode::InvalidConfiguration);
        invalid                = config                   ();
        invalid.responseWindow = adk::MicrosecondDuration (0);
        adk::InertIrTranslator noWindow                   (invalid,
                                         {storage, adk::capturedIrPulseCapacity});
        assert (noWindow.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::InertIrTranslator nullStorage (config (),
                                            {nullptr, adk::capturedIrPulseCapacity});
        assert (nullStorage.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::InertIrTranslator shortStorage (
            config (), {storage, adk::capturedIrPulseCapacity - 1});
        assert (shortStorage.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        adk::InertIrTranslator longStorage (
            config (), {storage, adk::capturedIrPulseCapacity + 1});
        assert (longStorage.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
    }

    void testFixedMappingAndEvidenceExclusions ()
    {
        struct Mapping
        {
            uint8_t            command;
            adk::LocalIrCodeId received;
            adk::LocalIrCodeId transmitted;
        };
        const Mapping mappings[] = {
            {static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].command),
             adk::LocalIrCodeId::StationPing, adk::LocalIrCodeId::StationReady},
            {static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[1].command),
             adk::LocalIrCodeId::StationReady, adk::LocalIrCodeId::StationAcknowledge},
            {static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[3].command),
             adk::LocalIrCodeId::StationAcknowledge, adk::LocalIrCodeId::StationPing}};

        for (uint8_t index = 0; index < 3; ++index)
        {
            uint32_t               storage[adk::capturedIrPulseCapacity];
            adk::InertIrTranslator translator (config (),
                                               {storage, adk::capturedIrPulseCapacity});
            adk::Pulse             pulses[necPulseCount];
            assert                                                   (translator.initialize ().ok ());
            const adk::IrTranslatorPreview preview = admitAndPrepare (
                translator, pulses, mappings[index].command, index + 1, 100);
            assert (preview.receivedCode == mappings[index].received);
            assert (preview.transmitCode == mappings[index].transmitted);
            assert (preview.receivedCode != preview.transmitCode);
            assert (preview.mappingDigest == adk::syntheticIrMappingDigest);
        }

        uint32_t               storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (config (),
                                           {storage, adk::capturedIrPulseCapacity});
        adk::Pulse             pulses[necPulseCount];
        assert (translator.initialize ().ok ());
        assert (translator
                    .update (receiveUpdate (
                        10, makeNec (pulses,
                                     static_cast<uint8_t> (
                                         adk::syntheticIrReceiveFixtures[2].command),
                                     1)))
                    .ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Cancelled);
        assert (translator.prepareTranslation (1, adk::MicrosecondTimePoint (10))
                    .error () == adk::StatusCode::InvalidArgument);

        translator.reset ();
        assert           (
            translator.update (receiveUpdate (20, makeNec (pulses, 0x99, 2))).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::UnlistedValidObserved);
        assert (translator.prepareTranslation (2, adk::MicrosecondTimePoint (20))
                    .error () == adk::StatusCode::InvalidArgument);

        assert (translator.update (receiveUpdate (30, makeRepeat (pulses, 3))).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::RepeatRejected);

        const adk::IrSourceIdentity foreign = {adk::IrSourceKind::SyntheticFixture, 9,
                                               2, 3};
        assert (translator
                    .update (receiveUpdate (
                        40,
                        makeNec (pulses,
                                 static_cast<uint8_t> (
                                     adk::syntheticIrReceiveFixtures[0].command),
                                 4),
                        foreign))
                    .error () == adk::StatusCode::InvalidArgument);
    }

    void testCandidateIdentityAndAtomicRejection ()
    {
        uint32_t               firstStorage[adk::capturedIrPulseCapacity];
        uint32_t               secondStorage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator first (config (),
                                      {firstStorage, adk::capturedIrPulseCapacity});
        adk::InertIrTranslator second (config (),
                                       {secondStorage, adk::capturedIrPulseCapacity});
        adk::Pulse             firstPulses[necPulseCount];
        adk::Pulse             secondPulses[necPulseCount];
        assert                                                   (first.initialize ().ok ());
        assert                                                   (second.initialize ().ok ());
        const adk::IrTranslatorPreview preview = admitAndPrepare (
            first, firstPulses,
            static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].command), 17, 100);
        admitAndPrepare (
            second, secondPulses,
            static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].command), 17, 100);
        assert (first.canCommit (preview, adk::MicrosecondTimePoint (100)));
        assert (!second.canCommit (preview, adk::MicrosecondTimePoint (100)));
        assert (!first.canCommit (preview, adk::MicrosecondTimePoint (101)));

        adk::IrTranslatorPreview forged = preview;
        ++forged.inputDigest;
        assert                                                  (!first.canCommit (forged, adk::MicrosecondTimePoint (100)));
        const adk::IrTranslatorSnapshot before = first.snapshot ();
        adk::IrTranslatorUpdateInput    input  = emptyUpdate    (100);
        input.commitPresent                    = true;
        input.commitPreview                    = forged;
        assert (first.update (input).error () == adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, first.snapshot ()));

        commit (first, preview, 100);
        assert (!first.canCommit (preview, adk::MicrosecondTimePoint (100)));
        assert (first.snapshot ().operationId == 17);

        first.reset ();
        assert      (!first.canCommit (preview, adk::MicrosecondTimePoint (100)));
    }

    void testAtomicEnvelopeCancelDominatesCoInputs ()
    {
        uint32_t               storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (config (),
                                           {storage, adk::capturedIrPulseCapacity});
        adk::Pulse             initial[necPulseCount];
        adk::Pulse             malformed[2] = {pulse (adk::PulseLevel::Mark, 9000),
                                               pulse (adk::PulseLevel::Space, 1)};
        assert                                                   (translator.initialize ().ok ());
        const adk::IrTranslatorPreview preview = admitAndPrepare (
            translator, initial,
            static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].command), 21, 100);

        adk::IrTranslatorUpdateInput input = emptyUpdate (100);
        input.cancelPresent                = true;
        input.cancelOperationId            = 21;
        input.commitPresent                = true;
        input.commitPreview                = preview;
        input.receivePresent               = true;
        input.receive = {receiveSource (),
                         adk::Status               (),
                         adk::MicrosecondTimePoint (100),
                         {malformed, 2, 99, adk::CaptureState::Complete}};
        assert (translator.update (input).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Cancelled);
        assert (translator.snapshot ().transmitIntent ==
                adk::IrEnvelopeIntent::Inactive);
        assert (!translator.canCommit (preview, adk::MicrosecondTimePoint (100)));
        assert (translator.receiveSnapshot ().disposition ==
                adk::IrCaptureDisposition::UnknownProtocol);

        adk::IrTranslatorUpdateInput invalid   = emptyUpdate (101);
        invalid.cancelOperationId              = 21;
        const adk::IrTranslatorSnapshot before = translator.snapshot ();
        assert                                                       (translator.update (invalid).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, translator.snapshot ()));
    }

    void testSelfEchoAndExactResponseBounds ()
    {
        const uint32_t completion  = 5000;
        const uint32_t guardEnd    = completion + 2000;
        const uint32_t responseEnd = guardEnd + 10000;

        for (uint8_t boundary = 0; boundary < 4; ++boundary)
        {
            uint32_t               storage[adk::capturedIrPulseCapacity];
            adk::InertIrTranslator translator (config (),
                                               {storage, adk::capturedIrPulseCapacity});
            adk::Pulse             request[necPulseCount];
            adk::Pulse             response[necPulseCount];
            assert                                                   (translator.initialize ().ok ());
            const adk::IrTranslatorPreview preview = admitAndPrepare (
                translator, request,
                static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].command), 31,
                100);
            commit (translator, preview, 100);

            adk::IrTranslatorUpdateInput emission = emptyUpdate (completion);
            emission.actualEmissionPresent        = true;
            emission.actualEmission               = {
                emitterSource             (), 31, adk::MicrosecondTimePoint (1000),
                adk::MicrosecondTimePoint (completion), adk::Status ()};
            assert (translator.update (emission).ok ());

            const uint32_t observed[] = {guardEnd - 1, guardEnd, responseEnd - 1,
                                         responseEnd};
            assert (translator
                        .update (receiveUpdate (
                            observed[boundary],
                            makeNec (response,
                                     static_cast<uint8_t> (
                                         adk::syntheticIrReceiveFixtures[1].command),
                                     boundary + 50)))
                        .ok ());
            const adk::IrTranslatorSnapshot snapshot = translator.snapshot ();
            if (boundary == 0)
            {
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::SelfEchoSuppressed);
                assert (snapshot.suppressedEchoCount == 1);
                assert (!snapshot.roundTrip.complete);
            }
            else if (boundary < 3)
            {
                assert (snapshot.roundTrip.complete);
                assert (snapshot.roundTrip.elapsed.microseconds () ==
                        observed[boundary] - completion);
                assert (snapshot.roundTrip.correlationDisposition ==
                        adk::IrTranslationDisposition::Translated);
            }
            else
            {
                assert (!snapshot.roundTrip.complete);
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::ReceiveTimeout);
            }
        }
    }

    void testAttributionFaultResetAndShutdown ()
    {
        uint32_t               storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (config (),
                                           {storage, adk::capturedIrPulseCapacity});
        adk::Pulse             request[necPulseCount];
        assert                                                   (translator.initialize ().ok ());
        const adk::IrTranslatorPreview preview = admitAndPrepare (
            translator, request,
            static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[0].command), 41, 100);
        commit (translator, preview, 100);

        adk::IrTranslatorUpdateInput mismatch = emptyUpdate (5000);
        mismatch.actualEmissionPresent        = true;
        mismatch.actualEmission = {emitterSource (), 99,
                                   adk::MicrosecondTimePoint (1000),
                                   adk::MicrosecondTimePoint (5000), adk::Status ()};
        assert (translator.update (mismatch).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::AttributionMismatch);
        assert (!translator.snapshot ().roundTrip.complete);

        adk::Pulse response[necPulseCount];
        assert (translator
                    .update (receiveUpdate (
                        7000,
                        makeNec (
                            response,
                            static_cast<uint8_t> (
                                adk::syntheticIrReceiveFixtures[1].command),
                            42)))
                    .ok ());
        assert (!translator.snapshot ().roundTrip.complete);

        translator.reset ();
        assert           (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Idle);
        assert (translator.snapshot ().operationId == 0);
        assert (translator.snapshot ().transmitIntent ==
                adk::IrEnvelopeIntent::Inactive);
        assert (!translator.canCommit (preview, adk::MicrosecondTimePoint (100)));

        const adk::IrTranslatorPreview fresh = admitAndPrepare (
            translator, request,
            static_cast<uint8_t> (adk::syntheticIrReceiveFixtures[1].command), 42,
            6000);
        translator.shutdown ();
        assert              (!translator.canCommit (fresh, adk::MicrosecondTimePoint (6000)));
        assert              (translator.snapshot ().transmitIntent ==
                adk::IrEnvelopeIntent::Inactive);
    }

#else
    void testReceiveFreeTimeoutAndSameEnvelopeAttribution ()
    {
        uint32_t storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (
            config (), {storage, adk::capturedIrPulseCapacity});
        adk::Pulse request[necPulseCount];
        assert                                                   (translator.initialize ().ok ());
        const adk::IrTranslatorPreview preview = admitAndPrepare (
            translator, request,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            51, 100);

        adk::IrTranslatorUpdateInput combined = emptyUpdate (100);
        combined.commitPresent                = true;
        combined.commitPreview                = preview;
        combined.actualEmissionPresent        = true;
        combined.actualEmission = {
            emitterSource (),
            99,
            adk::MicrosecondTimePoint (99),
            adk::MicrosecondTimePoint (100),
            adk::Status               ()};
        assert (translator.update (combined).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::AttributionMismatch);
        assert (!translator.snapshot ().roundTrip.complete);

        adk::IrTranslatorUpdateInput emission = emptyUpdate (5000);
        emission.actualEmissionPresent        = true;
        emission.actualEmission = {
            emitterSource (),
            51,
            adk::MicrosecondTimePoint (1000),
            adk::MicrosecondTimePoint (5000),
            adk::Status               ()};
        assert (translator.update (emission).ok ());

        assert (translator.update (emptyUpdate (16999)).ok ());
        assert (translator.snapshot ().disposition !=
                adk::IrTranslationDisposition::ReceiveTimeout);
        assert (translator.update (emptyUpdate (17000)).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::ReceiveTimeout);
        assert (!translator.snapshot ().roundTrip.complete);
    }

    void testCancellationCannotResurrect ()
    {
        uint32_t storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (
            config (), {storage, adk::capturedIrPulseCapacity});
        adk::Pulse request[necPulseCount];
        adk::Pulse response[necPulseCount];
        assert                                                   (translator.initialize ().ok ());
        const adk::IrTranslatorPreview preview = admitAndPrepare (
            translator, request,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            61, 100);
        commit (translator, preview, 100);

        adk::IrTranslatorUpdateInput cancelled = emptyUpdate (5000);
        cancelled.cancelPresent                = true;
        cancelled.cancelOperationId            = 61;
        cancelled.actualEmissionPresent        = true;
        cancelled.actualEmission = {
            emitterSource (),
            61,
            adk::MicrosecondTimePoint (1000),
            adk::MicrosecondTimePoint (5000),
            adk::Status               ()};
        assert (translator.update (cancelled).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Cancelled);
        assert (translator.snapshot ().transmitIntent ==
                adk::IrEnvelopeIntent::Inactive);

        assert (translator
                    .update (receiveUpdate (
                        7000,
                        makeNec (
                            response,
                            static_cast<uint8_t> (
                                adk::syntheticIrReceiveFixtures[1].command),
                            62)))
                    .ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Cancelled);
        assert (!translator.snapshot ().roundTrip.complete);
        assert (translator
                    .prepareTranslation (
                        62, adk::MicrosecondTimePoint (7000))
                    .error () == adk::StatusCode::InvalidArgument);

        translator.reset                                       ();
        const adk::IrTranslatorPreview fresh = admitAndPrepare (
            translator, request,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            71, 8000);
        commit (translator, fresh, 8000);
        assert (translator
                    .update (receiveUpdate (
                        9000,
                        makeNec (
                            response,
                            static_cast<uint8_t> (
                                adk::syntheticIrReceiveFixtures[2].command),
                            72)))
                    .ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Cancelled);
        assert (translator.snapshot ().transmitIntent ==
                adk::IrEnvelopeIntent::Inactive);
        assert (translator.emissionSnapshot ().disposition ==
                adk::IrEmissionDisposition::Cancelled);

        translator.reset ();
        const adk::IrTranslatorPreview semanticCancelPreview =
            admitAndPrepare (
                translator, request,
                static_cast<uint8_t> (
                    adk::syntheticIrReceiveFixtures[0].command),
                81, 10000);
        adk::IrTranslatorUpdateInput semanticCancel =
            receiveUpdate (
                10000,
                makeNec (
                    response,
                    static_cast<uint8_t> (
                        adk::syntheticIrReceiveFixtures[2].command),
                    82));
        semanticCancel.commitPresent = true;
        semanticCancel.commitPreview = semanticCancelPreview;
        assert (translator.update (semanticCancel).ok ());
        assert (translator.snapshot ().disposition ==
                adk::IrTranslationDisposition::Cancelled);
        assert (translator.snapshot ().transmitIntent ==
                adk::IrEnvelopeIntent::Inactive);
        assert (translator.emissionSnapshot ().disposition ==
                adk::IrEmissionDisposition::Cancelled);
    }

    void testNewReceiveInvalidatesPreparedTranslation ()
    {
        uint32_t storage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator translator (
            config (), {storage, adk::capturedIrPulseCapacity});
        adk::Pulse first[necPulseCount];
        adk::Pulse newer[necPulseCount];
        assert                                                 (translator.initialize ().ok ());
        const adk::IrTranslatorPreview stale = admitAndPrepare (
            translator, first,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            91, 100);

        adk::IrTranslatorUpdateInput collision =
            receiveUpdate (
                100,
                makeNec (
                    newer,
                    static_cast<uint8_t> (
                        adk::syntheticIrReceiveFixtures[1].command),
                    92));
        collision.commitPresent = true;
        collision.commitPreview = stale;
        assert (translator.update (collision).ok ());
        assert (!translator.canCommit (
            stale, adk::MicrosecondTimePoint (100)));
        assert (translator.emissionSnapshot ().disposition ==
                adk::IrEmissionDisposition::Cancelled);
        assert (translator.snapshot ().operationId == 0);
    }

    void testPreviewMutationReplayRolloverAndAbsentFields ()
    {
        uint32_t firstStorage[adk::capturedIrPulseCapacity];
        uint32_t secondStorage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator first (
            config (), {firstStorage, adk::capturedIrPulseCapacity});
        adk::InertIrTranslator second (
            config (), {secondStorage, adk::capturedIrPulseCapacity});
        adk::Pulse firstPulses[necPulseCount];
        adk::Pulse secondPulses[necPulseCount];
        assert                                                   (first.initialize ().ok ());
        assert                                                   (second.initialize ().ok ());
        const adk::IrTranslatorPreview preview = admitAndPrepare (
            first, firstPulses,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            101, 100);
        const adk::IrTranslatorPreview replay = admitAndPrepare (
            second, secondPulses,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            101, 100);
        assert (preview.instanceEpoch == replay.instanceEpoch);
        assert (preview.mappingDigest == replay.mappingDigest);
        assert (preview.inputDigest == replay.inputDigest);
        assert (preview.receivedCode == replay.receivedCode);
        assert (preview.transmitCode == replay.transmitCode);
        assert (sameSnapshot (first.snapshot (), second.snapshot ()));

        adk::IrTranslatorPreview changed = preview;
        changed.owner = &second;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.instanceEpoch;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.configurationRevision;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.mappingDigest;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emissionCatalog.revision;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emissionCatalog.digest;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.parentGeneration;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.operationId;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.inputDigest;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.evidenceGeneration;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.receiveProvenance.source.sourceId;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.receiveProvenance.source.configurationRevision;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.receiveProvenance.source.sessionEpoch;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receiveProvenance.source.kind =
            adk::IrSourceKind::QualifiedReceiver;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receiveProvenance.observedAt =
            adk::MicrosecondTimePoint (
                changed.receiveProvenance.observedAt.microseconds () + 1U);
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.receiveProvenance.captureSequence;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receiveProvenance.captureState = adk::CaptureState::Overflow;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receiveProvenance.protocol = adk::InfraredProtocol::Unknown;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receiveProvenance.decoderValidity =
            adk::FrameValidity::IntegrityInvalid;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receiveProvenance.sourceStatus =
            adk::StatusCode::HardwareFailure;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.receivedCode = adk::LocalIrCodeId::StationReady;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.transmitCode = adk::LocalIrCodeId::StationPing;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.emission.owner = &second;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.configurationRevision;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.instanceEpoch;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.policyGeneration;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.candidateGeneration;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.transactionId;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.emission.codeId = adk::LocalIrCodeId::StationPing;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.catalog.revision;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.catalog.digest;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        ++changed.emission.candidateDigest;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.emission.startAt =
            adk::MicrosecondTimePoint (101);
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.emission.completeAt =
            adk::MicrosecondTimePoint (
                changed.emission.completeAt.microseconds () + 1U);
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));
        changed = preview;
        changed.emission.firstIntent = adk::IrEnvelopeIntent::Inactive;
        assert (!first.canCommit (
            changed, adk::MicrosecondTimePoint (100)));

        const adk::IrTranslatorSnapshot before = first.snapshot ();
        adk::IrTranslatorUpdateInput absent = emptyUpdate       (100);
        absent.receive.source.sourceId       = 1;
        assert (first.update (absent).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, first.snapshot ()));

        adk::IrTranslatorUpdateInput halfRange =
            emptyUpdate (UINT32_C (0x80000000));
        halfRange.actualEmissionPresent = true;
        halfRange.actualEmission = {
            emitterSource (),
            101,
            adk::MicrosecondTimePoint (),
            adk::MicrosecondTimePoint (UINT32_C (0x80000000)),
            adk::Status               ()};
        assert (first.update (halfRange).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, first.snapshot ()));

        uint32_t rolloverStorage[adk::capturedIrPulseCapacity];
        adk::InertIrTranslator rollover (
            config (), {rolloverStorage, adk::capturedIrPulseCapacity});
        adk::Pulse request[necPulseCount];
        adk::Pulse response[necPulseCount];
        assert                                                           (rollover.initialize ().ok ());
        const adk::IrTranslatorPreview rolloverPreview = admitAndPrepare (
            rollover, request,
            static_cast<uint8_t> (
                adk::syntheticIrReceiveFixtures[0].command),
            111, UINT32_C (0xfffff000));
        commit (rollover, rolloverPreview, UINT32_C (0xfffff000));
        adk::IrTranslatorUpdateInput emission =
            emptyUpdate (UINT32_C (0xfffffff0));
        emission.actualEmissionPresent = true;
        emission.actualEmission = {
            emitterSource (),
            111,
            adk::MicrosecondTimePoint (UINT32_C (0xfffff000)),
            adk::MicrosecondTimePoint (UINT32_C (0xfffffff0)),
            adk::Status               ()};
        assert (rollover.update (emission).ok ());
        assert (rollover
                    .update (receiveUpdate (
                        UINT32_C (0x000007c0),
                        makeNec  (
                            response,
                            static_cast<uint8_t> (
                                adk::syntheticIrReceiveFixtures[1].command),
                            112)))
                    .ok ());
        assert (rollover.snapshot ().roundTrip.complete);
    }

    void testAllCanonicalPresenceMasks ()
    {
        for (uint8_t mask = 0; mask < 16; ++mask)
        {
            const bool cancelPresent =
                (mask & UINT8_C (0x01)) != 0;
            const bool commitPresent =
                (mask & UINT8_C (0x02)) != 0;
            const bool receivePresent =
                (mask & UINT8_C (0x04)) != 0;
            const bool actualPresent =
                (mask & UINT8_C (0x08)) != 0;
            const uint32_t operationId = 121;
            uint32_t storage[adk::capturedIrPulseCapacity];
            adk::InertIrTranslator translator (
                config (), {storage, adk::capturedIrPulseCapacity});
            adk::Pulse request[necPulseCount];
            adk::Pulse response[necPulseCount];
            assert (translator.initialize ().ok ());

            adk::IrTranslatorPreview preview =
                emptyTranslatorPreview ();
            if (commitPresent || (cancelPresent && !actualPresent))
            {
                preview = admitAndPrepare (
                    translator, request,
                    static_cast<uint8_t> (
                        adk::syntheticIrReceiveFixtures[0].command),
                    operationId, 100);
            }
            else if (actualPresent)
            {
                preview = admitAndPrepare (
                    translator, request,
                    static_cast<uint8_t> (
                        adk::syntheticIrReceiveFixtures[0].command),
                    operationId, 50);
                commit (translator, preview, 50);
            }

            adk::IrTranslatorUpdateInput input = emptyUpdate (100);
            input.cancelPresent                = cancelPresent;
            input.cancelOperationId =
                cancelPresent ? operationId : 0;
            input.commitPresent = commitPresent;
            if (commitPresent)
            {
                input.commitPreview = preview;
            }
            input.receivePresent = receivePresent;
            if (receivePresent)
            {
                const uint32_t sequence =
                    (commitPresent || cancelPresent || actualPresent)
                        ? operationId + 1U
                        : operationId;
                input.receive = {
                    receiveSource             (),
                    adk::Status               (),
                    adk::MicrosecondTimePoint (100),
                    makeNec                   (
                        response,
                        static_cast<uint8_t> (
                            adk::syntheticIrReceiveFixtures[1].command),
                        sequence)};
            }
            input.actualEmissionPresent = actualPresent;
            if (actualPresent)
            {
                input.actualEmission = {
                    emitterSource (),
                    operationId,
                    adk::MicrosecondTimePoint (99),
                    adk::MicrosecondTimePoint (100),
                    adk::Status               ()};
            }

            assert (translator.update (input).ok ());
            const adk::IrTranslatorSnapshot snapshot =
                translator.snapshot ();
            if (cancelPresent)
            {
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::Cancelled);
                assert (snapshot.transmitIntent ==
                        adk::IrEnvelopeIntent::Inactive);
            }
            else if (commitPresent && receivePresent &&
                     actualPresent)
            {
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::AttributionMismatch);
                assert (snapshot.operationId == 0);
            }
            else if (actualPresent && receivePresent)
            {
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::SelfEchoSuppressed);
                assert (!snapshot.roundTrip.complete);
            }
            else if (commitPresent && receivePresent)
            {
                assert (snapshot.operationId == 0);
                assert (translator.emissionSnapshot ().disposition ==
                        adk::IrEmissionDisposition::Cancelled);
            }
            else if (commitPresent || actualPresent ||
                     receivePresent)
            {
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::Translated);
            }
            else
            {
                assert (snapshot.disposition ==
                        adk::IrTranslationDisposition::Idle);
            }
        }
    }
#endif
} // namespace

int main ()
{
#if ADK_IR_TRANSLATOR_TEST_PART == 1
    testCanonicalFixtureDigest                       ();
    testConstructionConfigurationAndLifecycle        ();
    testFixedMappingAndEvidenceExclusions            ();
    testCandidateIdentityAndAtomicRejection          ();
    testAtomicEnvelopeCancelDominatesCoInputs        ();
    testSelfEchoAndExactResponseBounds               ();
    testAttributionFaultResetAndShutdown             ();
#else
    testReceiveFreeTimeoutAndSameEnvelopeAttribution ();
    testCancellationCannotResurrect                  ();
    testNewReceiveInvalidatesPreparedTranslation     ();
    testPreviewMutationReplayRolloverAndAbsentFields ();
    testAllCanonicalPresenceMasks                    ();
#endif
    return 0;
}
