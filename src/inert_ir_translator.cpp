#include "inert_ir_translator.h"

namespace adk {
    const IrSourceIdentity syntheticIrReceiveSource = {
        IrSourceKind::SyntheticFixture, 52, syntheticIrMappingRevision, 1};

    const SyntheticIrReceiveFixture
        syntheticIrReceiveFixtures[syntheticIrFixtureCount] = {
            {InfraredProtocol::Nec, UINT32_C (0x52), UINT32_C (0x10),
             LocalIrCodeId::StationPing, LocalIrCodeId::StationReady, true},
            {InfraredProtocol::Nec, UINT32_C (0x52), UINT32_C (0x20),
             LocalIrCodeId::StationReady, LocalIrCodeId::StationAcknowledge, true},
            {InfraredProtocol::Nec, UINT32_C (0x52), UINT32_C (0x30),
             LocalIrCodeId::StationCancel, LocalIrCodeId::StationPing, false},
            {InfraredProtocol::Nec, UINT32_C (0x52), UINT32_C (0x40),
             LocalIrCodeId::StationAcknowledge, LocalIrCodeId::StationPing, true}};

    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool sameSource (const IrSourceIdentity& left,
                         const IrSourceIdentity& right) noexcept
        {
            return left.kind == right.kind && left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sessionEpoch == right.sessionEpoch;
        }

        bool canonicalSource (const IrSourceIdentity& source) noexcept
        {
            return source.kind == IrSourceKind::SyntheticFixture &&
                   source.sourceId == 0 &&
                   source.configurationRevision == 0 &&
                   source.sessionEpoch == 0;
        }

        bool sameProvenance (const CapturedIrProvenance& left,
                             const CapturedIrProvenance& right) noexcept
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

        bool sameCatalog (const KnownIrCatalogIdentity& left,
                          const KnownIrCatalogIdentity& right) noexcept
        {
            return left.revision == right.revision && left.digest == right.digest;
        }

        bool sameEmissionPreview (const KnownIrEmissionPreview& left,
                                  const KnownIrEmissionPreview& right) noexcept
        {
            return left.owner == right.owner &&
                   left.configurationRevision == right.configurationRevision &&
                   left.instanceEpoch == right.instanceEpoch &&
                   left.policyGeneration == right.policyGeneration &&
                   left.candidateGeneration == right.candidateGeneration &&
                   left.transactionId == right.transactionId &&
                   left.codeId == right.codeId &&
                   sameCatalog (left.catalog, right.catalog) &&
                   left.candidateDigest == right.candidateDigest &&
                   left.startAt.microseconds    () == right.startAt.microseconds () &&
                   left.completeAt.microseconds () ==
                       right.completeAt.microseconds () &&
                   left.firstIntent == right.firstIntent;
        }

        bool canonicalProvenance (
            const CapturedIrProvenance& provenance) noexcept
        {
            return canonicalSource (provenance.source) &&
                   provenance.observedAt.microseconds () == 0 &&
                   provenance.captureSequence == 0 &&
                   provenance.captureState == CaptureState::Idle &&
                   provenance.protocol == InfraredProtocol::Unknown &&
                   provenance.decoderValidity ==
                       FrameValidity::UnknownProtocol &&
                   provenance.sourceStatus.ok ();
        }

        bool canonicalEmissionPreview (
            const KnownIrEmissionPreview& preview) noexcept
        {
            return preview.owner == nullptr &&
                   preview.configurationRevision == 0 &&
                   preview.instanceEpoch == 0 &&
                   preview.policyGeneration == 0 &&
                   preview.candidateGeneration == 0 &&
                   preview.transactionId == 0 &&
                   preview.codeId == LocalIrCodeId::StationPing &&
                   preview.catalog.revision == 0 &&
                   preview.catalog.digest == 0 &&
                   preview.candidateDigest == 0 &&
                   preview.startAt.microseconds    () == 0 &&
                   preview.completeAt.microseconds () == 0 &&
                   preview.firstIntent == IrEnvelopeIntent::Inactive;
        }

        bool canonicalTranslatorPreview (
            const IrTranslatorPreview& preview) noexcept
        {
            return preview.owner == nullptr && preview.instanceEpoch == 0 &&
                   preview.configurationRevision == 0 &&
                   preview.mappingDigest == 0 &&
                   preview.emissionCatalog.revision == 0 &&
                   preview.emissionCatalog.digest == 0 &&
                   preview.parentGeneration == 0 &&
                   preview.operationId == 0 && preview.inputDigest == 0 &&
                   preview.evidenceGeneration == 0 &&
                   canonicalProvenance (preview.receiveProvenance) &&
                   preview.receivedCode == LocalIrCodeId::StationPing &&
                   preview.transmitCode == LocalIrCodeId::StationPing &&
                   canonicalEmissionPreview (preview.emission);
        }

        bool validSource (const IrSourceIdentity& source) noexcept
        {
            return (source.kind == IrSourceKind::SyntheticFixture ||
                    source.kind == IrSourceKind::QualifiedReceiver ||
                    source.kind == IrSourceKind::LocalCatalog ||
                    source.kind == IrSourceKind::QualifiedEmitter) &&
                   source.sourceId != 0 && source.configurationRevision != 0 &&
                   source.sessionEpoch != 0;
        }

        bool validDuration (MicrosecondDuration duration) noexcept
        {
            return duration.microseconds () != 0 &&
                   duration.microseconds () < halfRange;
        }

        bool ordered (MicrosecondTimePoint later, MicrosecondTimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).microseconds () < halfRange;
        }

        uint32_t mix (uint32_t digest, uint32_t value) noexcept
        {
            for (uint8_t byte = 0; byte < 4; ++byte)
            {
                digest ^= static_cast<uint8_t> (value >> (byte * 8U));
                digest *= UINT32_C (0x01000193);
            }
            return digest;
        }

        constexpr uint32_t saturatingIncrement (uint32_t value) noexcept
        {
            return value == UINT32_MAX ? UINT32_MAX : value + 1U;
        }

        static_assert (saturatingIncrement (UINT32_MAX) == UINT32_MAX,
                       "echo suppression count saturates");

        uint32_t previewDigest (uint32_t mappingDigest, uint32_t operationId,
                                uint32_t evidenceGeneration, LocalIrCodeId received,
                                LocalIrCodeId transmitted,
                                uint32_t      childDigest) noexcept
        {
            uint32_t digest = UINT32_C (0x811c9dc5);
            digest          = mix      (digest, mappingDigest);
            digest          = mix      (digest, operationId);
            digest          = mix      (digest, evidenceGeneration);
            digest          = mix      (digest, static_cast<uint8_t> (received));
            digest          = mix      (digest, static_cast<uint8_t> (transmitted));
            return mix                 (digest, childDigest);
        }

        bool mapCommand (const CapturedIrSnapshot& capture, LocalIrCodeId& received,
                         LocalIrCodeId& transmitted) noexcept
        {
            for (uint8_t index = 0; index < syntheticIrFixtureCount; ++index)
            {
                const SyntheticIrReceiveFixture& fixture =
                    syntheticIrReceiveFixtures[index];
                if (fixture.transmissionAllowed &&
                    capture.provenance.protocol == fixture.protocol &&
                    capture.address == fixture.address &&
                    capture.command == fixture.command)
                {
                    received    = fixture.receivedCode;
                    transmitted = fixture.translatedCode;
                    return true;
                }
            }
            return false;
        }

        IrTranslatorSnapshot emptySnapshot () noexcept
        {
            IrTranslatorSnapshot result;
            result.operationId           = 0;
            result.receivedCode          = LocalIrCodeId::StationPing;
            result.transmitCode          = LocalIrCodeId::StationPing;
            result.receiveProvenance     = {{IrSourceKind::SyntheticFixture, 0, 0, 0},
                                            MicrosecondTimePoint (),
                                            0,
                                            CaptureState::Idle,
                                            InfraredProtocol::Unknown,
                                            FrameValidity::UnknownProtocol,
                                            Status ()};
            result.transmitSource        = {IrSourceKind::SyntheticFixture, 0, 0, 0};
            result.transmitTransactionId = 0;
            result.disposition           = IrTranslationDisposition::Idle;
            result.suppressedEchoCount   = 0;
            result.transmitIntent        = IrEnvelopeIntent::Inactive;
            result.roundTrip.complete    = false;
            result.roundTrip.operationId = 0;
            result.roundTrip.transmittedCode = LocalIrCodeId::StationPing;
            result.roundTrip.emissionCatalog = {0, 0};
            result.roundTrip.transmitSource = {IrSourceKind::SyntheticFixture, 0, 0, 0};
            result.roundTrip.actualStartedAt           = MicrosecondTimePoint ();
            result.roundTrip.actualCompletedAt         = MicrosecondTimePoint ();
            result.roundTrip.elapsed                   = MicrosecondDuration  ();
            result.roundTrip.receiveDisposition        = IrCaptureDisposition::None;
            result.roundTrip.receiveStrength           = EvidenceStrength::None;
            result.roundTrip.receiveProvenance         = result.receiveProvenance;
            result.roundTrip.receiveEvidenceGeneration = 0;
            result.roundTrip.correlationDisposition    = IrTranslationDisposition::Idle;
            result.status                              = StatusCode::Ok;
            result.roundTrip.status                    = StatusCode::Ok;
            return result;
        }
    } // namespace

    InertIrTranslator::InertIrTranslator (const IrTranslatorConfig& config,
                                          IrPulseStorage pulseStorage) noexcept
        : config_ (config),
          emissionConfig_ ({config.configurationRevision, config.instanceEpoch,
                            config.maximumEnvelopeDuration}),
          decoder_           (), evidence_ (decoder_, pulseStorage, capturedIrPulseCapacity),
          emission_          (emissionConfig_), snapshot_ (emptySnapshot ()), candidate_ (),
          actualEmission_    (), generation_ (0), lastUpdateAt_ (), initialized_ (false),
          pulseStorageValid_ (pulseStorage.data != nullptr &&
                              pulseStorage.capacity == capturedIrPulseCapacity),
          hasUpdateTime_       (false), hasCandidate_ (false), hasActualEmission_ (false)
          , terminalCancelled_ (false)
    {
    }

    InertIrTranslator::~InertIrTranslator () noexcept
    {
        shutdown ();
    }

    Status InertIrTranslator::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }
        if (!validConfiguration ())
        {
            return StatusCode::InvalidConfiguration;
        }
        Status status = evidence_.initialize ();
        if (!status.ok                       ())
        {
            return status;
        }
        status = emission_.initialize ();
        if (!status.ok                ())
        {
            evidence_.shutdown ();
            return status;
        }
        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void InertIrTranslator::shutdown () noexcept
    {
        if (!initialized_)
        {
            snapshot_.transmitIntent = IrEnvelopeIntent::Inactive;
            return;
        }
        emission_.shutdown ();
        evidence_.shutdown ();
        initialized_             = false;
        hasUpdateTime_           = false;
        hasActualEmission_       = false;
        snapshot_.transmitIntent = IrEnvelopeIntent::Inactive;
        clearCandidate ();
    }

    void InertIrTranslator::reset () noexcept
    {
        if (!initialized_)
        {
            return;
        }
        evidence_.reset                    ();
        emission_.reset                    ();
        snapshot_          = emptySnapshot ();
        hasActualEmission_ = false;
        terminalCancelled_ = false;
        hasUpdateTime_     = false;
        ++generation_;
        if (generation_ == 0)
        {
            ++generation_;
        }
        clearCandidate ();
    }

    Result<IrTranslatorPreview>
    InertIrTranslator::prepareTranslation (uint32_t             operationId,
                                           MicrosecondTimePoint now) noexcept
    {
        IrTranslatorPreview result;
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, result};
        }
        if (operationId == 0 || hasCandidate_ ||
            terminalCancelled_ ||
            (hasUpdateTime_ && now.microseconds () != lastUpdateAt_.microseconds ()))
        {
            return {StatusCode::InvalidArgument, result};
        }

        const CapturedIrSnapshot capture     = evidence_.snapshot ();
        LocalIrCodeId            received    = LocalIrCodeId::StationPing;
        LocalIrCodeId            transmitted = LocalIrCodeId::StationPing;
        if (capture.disposition != IrCaptureDisposition::KnownValid ||
            capture.strength != EvidenceStrength::IntegrityVerified ||
            capture.provenance.captureSequence != operationId ||
            !mapCommand (capture, received, transmitted))
        {
            return {StatusCode::InvalidArgument, result};
        }

        const Result<KnownIrEmissionPreview> child =
            emission_.prepare (transmitted, operationId, now);
        if (!child.ok ())
        {
            return {child.status (), result};
        }

        ++generation_;
        if (generation_ == 0)
        {
            ++generation_;
        }
        result.owner                 = this;
        result.instanceEpoch         = config_.instanceEpoch;
        result.configurationRevision = config_.configurationRevision;
        result.mappingDigest         = config_.mappingDigest;
        result.emissionCatalog       = child.value ().catalog;
        result.parentGeneration      = generation_;
        result.operationId           = operationId;
        result.evidenceGeneration    = capture.evidenceGeneration;
        result.receiveProvenance     = capture.provenance;
        result.receivedCode          = received;
        result.transmitCode          = transmitted;
        result.emission              = child.value ();
        result.inputDigest =
            previewDigest (result.mappingDigest, operationId, result.evidenceGeneration,
                           received, transmitted, result.emission.candidateDigest);
        candidate_    = result;
        hasCandidate_ = true;
        return {StatusCode::Ok, result};
    }

    bool InertIrTranslator::canCommit (const IrTranslatorPreview& preview,
                                       MicrosecondTimePoint       now) const noexcept
    {
        return initialized_ && hasCandidate_ && validPreview (preview, now) &&
               emission_.canCommit (preview.emission, now);
    }

    Status InertIrTranslator::update (const IrTranslatorUpdateInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validUpdate (input))
        {
            return StatusCode::InvalidArgument;
        }

        bool candidateInvalidatedByReceive = false;
        if (input.receivePresent)
        {
            const Status admitted =
                evidence_.admit (input.receive.frame, input.receive.source,
                                 input.receive.sourceStatus, input.receive.observedAt);
            if (!admitted.ok ())
            {
                return admitted;
            }
            classifyReceive ();
            if (hasCandidate_ &&
                evidence_.snapshot ().evidenceGeneration !=
                    candidate_.evidenceGeneration)
            {
                emission_.cancel (candidate_.emission, input.now);
                clearCandidate   ();
                candidateInvalidatedByReceive = true;
            }
            if (snapshot_.disposition == IrTranslationDisposition::Cancelled)
            {
                if (emission_.snapshot ().disposition ==
                    IrEmissionDisposition::Active)
                {
                    emission_.cancel (snapshot_.operationId, input.now);
                }
                clearCandidate ();
                hasActualEmission_       = false;
                terminalCancelled_       = true;
                snapshot_.transmitIntent = IrEnvelopeIntent::Inactive;
            }
        }

        lastUpdateAt_  = input.now;
        hasUpdateTime_ = true;
        bool actualAttributionMismatch = false;

        if (input.cancelPresent)
        {
            if (hasCandidate_)
            {
                emission_.cancel (candidate_.emission, input.now);
            }
            else if (emission_.snapshot ().disposition == IrEmissionDisposition::Active)
            {
                emission_.cancel (input.cancelOperationId, input.now);
            }
            clearCandidate ();
            hasActualEmission_       = false;
            terminalCancelled_       = true;
            snapshot_.disposition    = IrTranslationDisposition::Cancelled;
            snapshot_.transmitIntent = IrEnvelopeIntent::Inactive;
        }

        if (input.actualEmissionPresent && !terminalCancelled_)
        {
            const uint32_t effectiveOperation =
                input.commitPresent && !candidateInvalidatedByReceive
                    ? input.commitPreview.operationId
                    : snapshot_.operationId;
            if (!input.actualEmission.status.ok () ||
                input.actualEmission.transactionId != effectiveOperation)
            {
                actualAttributionMismatch = true;
                snapshot_.disposition = IrTranslationDisposition::AttributionMismatch;
                snapshot_.status      = input.actualEmission.status.ok ()
                                            ? Status (StatusCode::InvalidArgument)
                                            : input.actualEmission.status;
                hasActualEmission_ = false;
            }
            else
            {
                actualEmission_                    = input.actualEmission;
                hasActualEmission_                 = true;
                snapshot_.transmitSource        = input.actualEmission.source;
                snapshot_.transmitTransactionId = input.actualEmission.transactionId;
            }
        }

        if (input.commitPresent && !input.cancelPresent &&
            !terminalCancelled_ && !candidateInvalidatedByReceive)
        {
            if (!canCommit (input.commitPreview, input.now))
            {
                return StatusCode::InvalidArgument;
            }
            const Status status =
                emission_.commit (input.commitPreview.emission, input.now);
            if (!status.ok ())
            {
                return status;
            }
            snapshot_.operationId       = input.commitPreview.operationId;
            snapshot_.receivedCode      = input.commitPreview.receivedCode;
            snapshot_.transmitCode      = input.commitPreview.transmitCode;
            snapshot_.receiveProvenance = input.commitPreview.receiveProvenance;
            snapshot_.transmitTransactionId =
                input.commitPreview.emission.transactionId;
            snapshot_.disposition    = IrTranslationDisposition::Translated;
            snapshot_.transmitIntent = emission_.snapshot ().intent;
            snapshot_.status         = StatusCode::Ok;
            clearCandidate ();
        }
        else if (!input.cancelPresent &&
                 emission_.snapshot ().disposition == IrEmissionDisposition::Active)
        {
            const Status status = emission_.update (input.now);
            if (!status.ok                         ())
            {
                return status;
            }
            snapshot_.transmitIntent = emission_.snapshot ().intent;
        }

        if (input.receivePresent && hasActualEmission_ && !terminalCancelled_)
        {
            correlateReceive ();
        }
        else if (!input.receivePresent && hasActualEmission_ &&
                 snapshot_.disposition != IrTranslationDisposition::Cancelled)
        {
            const uint32_t responseEnd =
                actualEmission_.completedAt
                    .elapsedSince (actualEmission_.startedAt)
                    .microseconds () +
                config_.echoGuard.microseconds      () +
                config_.responseWindow.microseconds ();
            if (input.now.elapsedSince (actualEmission_.startedAt).microseconds () >=
                responseEnd)
            {
                snapshot_.disposition = IrTranslationDisposition::ReceiveTimeout;
            }
        }
        if (terminalCancelled_)
        {
            snapshot_.disposition    = IrTranslationDisposition::Cancelled;
            snapshot_.transmitIntent = IrEnvelopeIntent::Inactive;
        }
        else if (actualAttributionMismatch)
        {
            snapshot_.disposition =
                IrTranslationDisposition::AttributionMismatch;
        }
        return StatusCode::Ok;
    }

    IrTranslatorSnapshot InertIrTranslator::snapshot () const noexcept
    {
        return snapshot_;
    }

    CapturedIrSnapshot InertIrTranslator::receiveSnapshot () const noexcept
    {
        return evidence_.snapshot ();
    }

    KnownIrEmissionSnapshot InertIrTranslator::emissionSnapshot () const noexcept
    {
        return emission_.snapshot ();
    }

    bool InertIrTranslator::validConfiguration () const noexcept
    {
        return pulseStorageValid_ && config_.configurationRevision != 0 &&
               config_.instanceEpoch != 0 &&
               config_.mappingDigest == syntheticIrMappingDigest &&
               validSource (config_.qualifiedReceiveSource) &&
               sameSource  (config_.qualifiedReceiveSource, syntheticIrReceiveSource) &&
               validSource (config_.localEmitterSource) &&
               (config_.localEmitterSource.kind == IrSourceKind::SyntheticFixture ||
                config_.localEmitterSource.kind == IrSourceKind::QualifiedEmitter) &&
               !sameSource (config_.qualifiedReceiveSource,
                            config_.localEmitterSource) &&
               validDuration                  (config_.maximumEnvelopeDuration) &&
               validDuration                  (config_.echoGuard) &&
               validDuration                  (config_.responseWindow) &&
               config_.echoGuard.microseconds () +
                       config_.responseWindow.microseconds () <
                   halfRange;
    }

    bool
    InertIrTranslator::validUpdate (const IrTranslatorUpdateInput& input) const noexcept
    {
        if (hasUpdateTime_ && !ordered (input.now, lastUpdateAt_))
        {
            return false;
        }
        if (!input.cancelPresent && input.cancelOperationId != 0)
        {
            return false;
        }
        if (!input.commitPresent &&
            !canonicalTranslatorPreview (input.commitPreview))
        {
            return false;
        }
        if (input.cancelPresent &&
            (input.cancelOperationId == 0 ||
             (hasCandidate_ && input.cancelOperationId != candidate_.operationId) ||
             (!hasCandidate_ &&
              emission_.snapshot                            ().disposition == IrEmissionDisposition::Active &&
              input.cancelOperationId != emission_.snapshot ().transactionId)))
        {
            return false;
        }
        if (input.commitPresent &&
            (!validPreview (input.commitPreview, input.now) ||
             !emission_.canCommit (input.commitPreview.emission, input.now)))
        {
            return false;
        }
        if (input.receivePresent &&
            (!sameSource (input.receive.source, config_.qualifiedReceiveSource) ||
             input.receive.observedAt.microseconds () != input.now.microseconds ()))
        {
            return false;
        }
        if (!input.receivePresent &&
            (!canonicalSource (input.receive.source) ||
             !input.receive.sourceStatus.ok        () ||
             input.receive.observedAt.microseconds () != 0 ||
             input.receive.frame.data != nullptr ||
             input.receive.frame.size != 0 ||
             input.receive.frame.sequence != 0 ||
             input.receive.frame.state != CaptureState::Idle))
        {
            return false;
        }
        if (input.actualEmissionPresent &&
            (!sameSource (input.actualEmission.source, config_.localEmitterSource) ||
             hasActualEmission_ || input.actualEmission.transactionId == 0 ||
             !ordered (input.actualEmission.completedAt,
                       input.actualEmission.startedAt) ||
             input.actualEmission.completedAt
                     .elapsedSince (input.actualEmission.startedAt)
                     .microseconds () == 0 ||
             input.actualEmission.completedAt
                     .elapsedSince (input.actualEmission.startedAt)
                     .microseconds () >= halfRange ||
             input.actualEmission.completedAt
                         .elapsedSince (input.actualEmission.startedAt)
                         .microseconds () +
                     config_.echoGuard.microseconds      () +
                     config_.responseWindow.microseconds () >=
                 halfRange ||
             !ordered (input.now, input.actualEmission.completedAt)))
        {
            return false;
        }
        if (!input.actualEmissionPresent &&
            (!canonicalSource (input.actualEmission.source) ||
             input.actualEmission.transactionId != 0 ||
             input.actualEmission.startedAt.microseconds   () != 0 ||
             input.actualEmission.completedAt.microseconds () != 0 ||
             !input.actualEmission.status.ok               ()))
        {
            return false;
        }
        return true;
    }

    bool InertIrTranslator::validPreview (const IrTranslatorPreview& preview,
                                          MicrosecondTimePoint       now) const noexcept
    {
        if (!hasCandidate_ || preview.owner != this ||
            preview.instanceEpoch != config_.instanceEpoch ||
            preview.configurationRevision != config_.configurationRevision ||
            preview.mappingDigest != config_.mappingDigest ||
            preview.parentGeneration != candidate_.parentGeneration ||
            preview.operationId != candidate_.operationId ||
            preview.inputDigest != candidate_.inputDigest ||
            preview.evidenceGeneration != candidate_.evidenceGeneration ||
            preview.receivedCode != candidate_.receivedCode ||
            preview.transmitCode != candidate_.transmitCode ||
            !sameCatalog         (preview.emissionCatalog, candidate_.emissionCatalog) ||
            !sameProvenance      (preview.receiveProvenance, candidate_.receiveProvenance) ||
            !sameEmissionPreview (preview.emission, candidate_.emission))
        {
            return false;
        }
        return now.microseconds () == preview.emission.startAt.microseconds ();
    }

    void InertIrTranslator::classifyReceive () noexcept
    {
        const CapturedIrSnapshot capture = evidence_.snapshot ();
        snapshot_.receiveProvenance      = capture.provenance;
        LocalIrCodeId received           = LocalIrCodeId::StationPing;
        LocalIrCodeId transmitted        = LocalIrCodeId::StationPing;

        if (capture.disposition == IrCaptureDisposition::KnownRepeat)
        {
            snapshot_.disposition = IrTranslationDisposition::RepeatRejected;
        }
        else if (capture.disposition == IrCaptureDisposition::UnknownProtocol)
        {
            snapshot_.disposition = IrTranslationDisposition::UnknownObserved;
        }
        else if (capture.disposition != IrCaptureDisposition::KnownValid)
        {
            snapshot_.disposition = IrTranslationDisposition::MalformedObserved;
        }
        else if (capture.provenance.protocol ==
                     syntheticIrReceiveFixtures[2].protocol &&
                 capture.address == syntheticIrReceiveFixtures[2].address &&
                 capture.command == syntheticIrReceiveFixtures[2].command)
        {
            if (snapshot_.operationId == 0)
            {
                snapshot_.receivedCode = LocalIrCodeId::StationCancel;
            }
            snapshot_.disposition = IrTranslationDisposition::Cancelled;
        }
        else if (!mapCommand (capture, received, transmitted))
        {
            snapshot_.disposition = IrTranslationDisposition::UnlistedValidObserved;
        }
        else
        {
            if (snapshot_.operationId == 0)
            {
                snapshot_.receivedCode = received;
                snapshot_.transmitCode = transmitted;
            }
            snapshot_.disposition = IrTranslationDisposition::Translated;
        }
        snapshot_.status = capture.status;
    }

    void InertIrTranslator::correlateReceive () noexcept
    {
        if (terminalCancelled_ || !hasActualEmission_ ||
            !actualEmission_.status.ok () ||
            actualEmission_.transactionId != snapshot_.operationId ||
            !sameSource (actualEmission_.source, config_.localEmitterSource))
        {
            return;
        }
        const CapturedIrSnapshot capture = evidence_.snapshot ();
        const uint32_t           actualDuration =
            actualEmission_.completedAt.elapsedSince (actualEmission_.startedAt)
                .microseconds ();
        const uint32_t observedSinceStart =
            capture.provenance.observedAt.elapsedSince (actualEmission_.startedAt)
                .microseconds ();
        const uint32_t responseStart =
            actualDuration + config_.echoGuard.microseconds ();
        const uint32_t responseEnd =
            responseStart + config_.responseWindow.microseconds ();

        if (observedSinceStart < responseStart)
        {
            snapshot_.suppressedEchoCount =
                saturatingIncrement (snapshot_.suppressedEchoCount);
            snapshot_.disposition = IrTranslationDisposition::SelfEchoSuppressed;
            return;
        }
        if (observedSinceStart >= responseEnd)
        {
            snapshot_.disposition = IrTranslationDisposition::ReceiveTimeout;
            return;
        }
        LocalIrCodeId received    = LocalIrCodeId::StationPing;
        LocalIrCodeId transmitted = LocalIrCodeId::StationPing;
        if (capture.disposition != IrCaptureDisposition::KnownValid ||
            capture.strength != EvidenceStrength::IntegrityVerified ||
            !mapCommand (capture, received, transmitted) ||
            received != snapshot_.transmitCode)
        {
            return;
        }

        snapshot_.roundTrip.complete          = true;
        snapshot_.roundTrip.operationId       = snapshot_.operationId;
        snapshot_.roundTrip.transmittedCode   = snapshot_.transmitCode;
        snapshot_.roundTrip.emissionCatalog   = emission_.snapshot ().catalog;
        snapshot_.roundTrip.transmitSource    = actualEmission_.source;
        snapshot_.roundTrip.actualStartedAt   = actualEmission_.startedAt;
        snapshot_.roundTrip.actualCompletedAt = actualEmission_.completedAt;
        snapshot_.roundTrip.elapsed =
            capture.provenance.observedAt.elapsedSince (actualEmission_.completedAt);
        snapshot_.roundTrip.receiveDisposition        = capture.disposition;
        snapshot_.roundTrip.receiveStrength           = capture.strength;
        snapshot_.roundTrip.receiveProvenance         = capture.provenance;
        snapshot_.roundTrip.receiveEvidenceGeneration = capture.evidenceGeneration;
        snapshot_.roundTrip.correlationDisposition =
            IrTranslationDisposition::Translated;
        snapshot_.roundTrip.status = capture.status;
        snapshot_.disposition      = IrTranslationDisposition::Translated;
    }

    void InertIrTranslator::clearCandidate () noexcept
    {
        hasCandidate_ = false;
    }
} // namespace adk
