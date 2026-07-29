#include "inert_parts_carousel.h"

#include <limits.h>
#include <stddef.h>

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange         = UINT32_C (0x80000000);
        uint32_t           nextCarouselOwner = 1;

        uint32_t allocateOwner () noexcept
        {
            const uint32_t owner = nextCarouselOwner++;
            if (nextCarouselOwner == 0)
            {
                nextCarouselOwner = 1;
            }
            return owner;
        }

        uint16_t crc16 (const uint8_t* bytes, uint8_t length) noexcept
        {
            uint16_t crc = 0xffffU;
            for (uint8_t index = 0; index < length; ++index)
            {
                const uint16_t highByte = static_cast<uint16_t> (
                    static_cast<uint16_t> (bytes[index]) * UINT16_C (256));
                crc = static_cast<uint16_t> (crc ^ highByte);
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & 0x8000U) != 0
                              ? static_cast<uint16_t> (
                                    static_cast<uint32_t> (crc) * UINT32_C (2) ^
                                    UINT32_C                               (0x1021))
                              : static_cast<uint16_t> (
                                    static_cast<uint32_t> (crc) * UINT32_C (2));
                }
            }
            return crc;
        }

        void put16 (uint8_t* bytes, uint8_t offset, uint16_t value) noexcept
        {
            bytes[offset]     = static_cast<uint8_t> (value);
            bytes[offset + 1] = static_cast<uint8_t> (value >> 8U);
        }

        void put32 (uint8_t* bytes, uint8_t offset, uint32_t value) noexcept
        {
            for (uint8_t index = 0; index < 4; ++index)
            {
                bytes[offset + index] = static_cast<uint8_t> (value >> (index * 8U));
            }
        }

        uint16_t get16 (const uint8_t* bytes, uint8_t offset) noexcept
        {
            return static_cast<uint16_t> (
                bytes[offset] | static_cast<uint16_t> (bytes[offset + 1]) << 8U);
        }

        uint32_t get32 (const uint8_t* bytes, uint8_t offset) noexcept
        {
            uint32_t value = 0;
            for (uint8_t index = 0; index < 4; ++index)
            {
                value |= static_cast<uint32_t> (bytes[offset + index]) << (index * 8U);
            }
            return value;
        }

        bool bytesEqual (const uint8_t* left, const uint8_t* right,
                         uint8_t length) noexcept
        {
            for (uint8_t index = 0; index < length; ++index)
            {
                if (left[index] != right[index])
                {
                    return false;
                }
            }
            return true;
        }

        bool erased (const uint8_t* bytes) noexcept
        {
            for (uint8_t index = 0; index < carouselAuditRecordBytes; ++index)
            {
                if (bytes[index] != 0xffU)
                {
                    return false;
                }
            }
            return true;
        }

        bool rangesOverlap (const uint8_t* left, uint16_t leftLength,
                            const uint8_t* right, uint16_t rightLength) noexcept
        {
            const uintptr_t leftStart  = reinterpret_cast<uintptr_t> (left);
            const uintptr_t rightStart = reinterpret_cast<uintptr_t> (right);
            return leftStart < rightStart + rightLength &&
                   rightStart < leftStart + leftLength;
        }

        bool presentOrPast (TimePoint now, TimePoint observedAt) noexcept
        {
            return now.elapsedSince (observedAt).milliseconds () < halfRange;
        }

        bool current (TimePoint now, TimePoint observedAt, Duration maximumAge) noexcept
        {
            return presentOrPast (now, observedAt) &&
                   now.elapsedSince (observedAt).milliseconds () <=
                       maximumAge.milliseconds ();
        }

        bool withinSkew (TimePoint left, TimePoint right, Duration maximum) noexcept
        {
            const uint32_t forward = left.elapsedSince  (right).milliseconds ();
            const uint32_t reverse = right.elapsedSince (left).milliseconds ();
            const uint32_t delta   = forward < reverse ? forward : reverse;
            return delta < halfRange && delta <= maximum.milliseconds ();
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool sourceShape (CarouselSource source, CarouselSourceKind kind) noexcept
        {
            return source.kind == kind && source.sourceId != 0 &&
                   source.configurationRevision != 0;
        }

        bool zeroSource (CarouselSource source, CarouselSourceKind kind) noexcept
        {
            return source.kind == kind && source.sourceId == 0 &&
                   source.configurationRevision == 0;
        }

        bool zeroIdentity (const IdentityEvidence& evidence) noexcept
        {
            if (!zeroSource (evidence.source, CarouselSourceKind::SyntheticIdentity) ||
                evidence.observedAt != TimePoint                     () || evidence.sequence != 0 ||
                evidence.identity.length != 0 || !evidence.status.ok ())
            {
                return false;
            }
            for (uint8_t index = 0; index < maximumLocalIdentityBytes; ++index)
            {
                if (evidence.identity.bytes[index] != 0)
                {
                    return false;
                }
            }
            return true;
        }

        bool zeroKey (const CopiedKeyBatch& key) noexcept
        {
            if (!zeroSource (key.source, CarouselSourceKind::SyntheticKey) ||
                key.observedAt != TimePoint                                        () || key.sequence != 0 ||
                key.digitCount != 0 || key.confirm || key.cancel || !key.status.ok ())
            {
                return false;
            }
            for (uint8_t index = 0; index < 4; ++index)
            {
                if (key.digits[index] != 0)
                {
                    return false;
                }
            }
            return true;
        }

        StepperSequenceConfig stepperConfig (const CarouselConfig& config,
                                             BoundedHomingPolicy&  homing) noexcept
        {
            HomingExcursionBounds bounds  = homing.excursionBounds ();
            int32_t               minimum = bounds.minimum;
            int32_t               maximum = bounds.maximum;
            for (uint8_t index = 0;
                 index < config.binCount && index < maximumCarouselBins; ++index)
            {
                if (config.binPositions[index] < minimum)
                {
                    minimum = config.binPositions[index];
                }
                if (config.binPositions[index] > maximum)
                {
                    maximum = config.binPositions[index];
                }
            }
            return StepperSequenceConfig (
                config.logicalStepInterval, config.logicalStepInterval,
                config.maximumStepCommandAge, minimum, maximum, false);
        }

        CarouselSnapshot emptySnapshot (Status status) noexcept
        {
            CarouselAuditRecord record = {0, 0,
                                          0, 0,
                                          0, 0,
                                          0, TimePoint (),
                                          0, CarouselPhase::Uninitialized,
                                          0, 0,
                                          0, 0,
                                          0, CarouselAuditStatus::Success,
                                          0};
            return {CarouselPhase::Uninitialized,
                    CarouselFault::None,
                    IdentityDisposition::None,
                    HomingFault::None,
                    0,
                    false,
                    false,
                    0,
                    {0, CarouselGateIntent::Closed, TimePoint (), 0,
                     CarouselAuditStatus::Success},
                    false,
                    record,
                    false,
                    false,
                    0,
                    status};
        }

        bool validPhase (uint8_t value) noexcept
        {
            return value <= static_cast<uint8_t> (CarouselPhase::Fault);
        }

        bool validAuditStatus (uint8_t value) noexcept
        {
            return value <=
                   static_cast<uint8_t> (CarouselAuditStatus::RecoveredInterrupted);
        }

        bool decodeRecord (const uint8_t* bytes, uint32_t projectId, uint8_t binCount,
                           CarouselAuditRecord& record, bool& unsupported) noexcept
        {
            if (get16 (bytes, 0) != carouselAuditMagic ||
                bytes[3] != carouselAuditRecordBytes || get32 (bytes, 4) != projectId ||
                get32                                         (bytes, 8) == 0 || get16 (bytes, 12) == 0 ||
                bytes[22] >= binCount || !validPhase          (bytes[21]) ||
                !validAuditStatus                             (bytes[23]) || bytes[20] < 1 || bytes[20] > 3 ||
                get16                                         (bytes, 26) == 0 || get32 (bytes, 28) == 0 || bytes[36] != 0 ||
                bytes[37] != 0 || crc16                       (bytes, 38) != get16 (bytes, 38))
            {
                unsupported = false;
                return false;
            }
            unsupported = bytes[2] != carouselAuditVersion;
            if (unsupported)
            {
                return false;
            }
            record = {get16 (bytes, 0),  bytes[2],
                      bytes[3],          get32 (bytes, 4),
                      get32                    (bytes, 8),  get16 (bytes, 12),
                      get16                    (bytes, 14), TimePoint (get32 (bytes, 16)),
                      bytes[20],         static_cast<CarouselPhase> (bytes[21]),
                      bytes[22],         get16 (bytes, 24),
                      get16                    (bytes, 26), get32 (bytes, 28),
                      get32                    (bytes, 32), static_cast<CarouselAuditStatus> (bytes[23]),
                      get16                    (bytes, 38)};
            if (record.recordKind == 1)
            {
                return record.auditStatus == CarouselAuditStatus::Success &&
                       record.homeEpoch == 0 &&
                       (record.phase == CarouselPhase::AwaitingConfirmation ||
                        record.phase == CarouselPhase::Homing);
            }
            if (record.recordKind == 3)
            {
                return record.phase == CarouselPhase::Fault &&
                       record.auditStatus ==
                           CarouselAuditStatus::RecoveredInterrupted &&
                       record.homeEpoch == 0;
            }
            if (record.auditStatus == CarouselAuditStatus::RecoveredInterrupted)
            {
                return false;
            }
            if (record.phase == CarouselPhase::Complete)
            {
                return record.auditStatus == CarouselAuditStatus::Success &&
                       record.homeEpoch != 0;
            }
            if (record.phase == CarouselPhase::Cancelled)
            {
                return record.auditStatus == CarouselAuditStatus::Cancelled ||
                       record.auditStatus ==
                           CarouselAuditStatus::AuthorizationExpired ||
                       record.auditStatus == CarouselAuditStatus::ConfirmationConflict;
            }
            if (record.phase == CarouselPhase::Stopped)
            {
                return record.auditStatus == CarouselAuditStatus::Stopped;
            }
            return record.phase == CarouselPhase::Fault &&
                   record.auditStatus != CarouselAuditStatus::Success &&
                   record.auditStatus != CarouselAuditStatus::Cancelled &&
                   record.auditStatus != CarouselAuditStatus::Stopped;
        }

        bool pairMatches (const CarouselAuditRecord& start,
                          const CarouselAuditRecord& terminal) noexcept
        {
            const uint16_t delta =
                static_cast<uint16_t> (terminal.recordSequence - start.recordSequence);
            return start.recordKind == 1 && terminal.recordKind != 1 && delta > 0 &&
                   delta < 0x8000U &&
                   start.projectConfigurationId == terminal.projectConfigurationId &&
                   start.operationId == terminal.operationId &&
                   start.authorizationEpoch == terminal.authorizationEpoch &&
                   start.binId == terminal.binId &&
                   start.identityDigest == terminal.identityDigest &&
                   start.bindingRevision == terminal.bindingRevision &&
                   start.identityImageGeneration == terminal.identityImageGeneration;
        }

        void encodeRecord (const CarouselAuditRecord& record, uint8_t* bytes) noexcept
        {
            for (uint8_t index = 0; index < carouselAuditRecordBytes; ++index)
            {
                bytes[index] = 0;
            }
            put16 (bytes, 0, carouselAuditMagic);
            bytes[2] = carouselAuditVersion;
            bytes[3] = carouselAuditRecordBytes;
            put32 (bytes, 4, record.projectConfigurationId);
            put32 (bytes, 8, record.operationId);
            put16 (bytes, 12, record.authorizationEpoch);
            put16 (bytes, 14, record.recordSequence);
            put32 (bytes, 16, record.occurredAt.milliseconds ());
            bytes[20] = record.recordKind;
            bytes[21] = static_cast<uint8_t> (record.phase);
            bytes[22] = record.binId;
            bytes[23] = static_cast<uint8_t> (record.auditStatus);
            put16 (bytes, 24, record.identityDigest);
            put16 (bytes, 26, record.bindingRevision);
            put32 (bytes, 28, record.identityImageGeneration);
            put32 (bytes, 32, record.homeEpoch);
            put16 (bytes, 38, crc16 (bytes, 38));
        }

        uint16_t identityDigest (const LocalIdentity& identity) noexcept
        {
            uint8_t bytes[maximumLocalIdentityBytes + 1] = {};
            bytes[0]                                     = identity.length;
            for (uint8_t index = 0; index < identity.length; ++index)
            {
                bytes[index + 1] = identity.bytes[index];
            }
            return crc16 (bytes, static_cast<uint8_t> (identity.length + 1U));
        }

        bool stepperLive (const StepperSequenceSnapshot& snapshot) noexcept
        {
            return snapshot.phase == StepSequencePhase::Moving ||
                   snapshot.phase == StepSequencePhase::Holding ||
                   snapshot.coilIntent != 0;
        }
    } // namespace

    InertPartsCarousel::InertPartsCarousel (
        const CarouselConfig& config, LocalIdentityRegistry& identityRegistry,
        BoundedHomingPolicy& homingPolicy, uint8_t* auditSlotBytes,
        uint16_t auditSlotByteExtent, uint8_t auditSlotStride, uint8_t auditCapacity,
        uint8_t* auditCandidateBytes, uint8_t auditCandidateCapacity) noexcept
        : config_                  (&config),
          identity_                (&identityRegistry),
          homing_                  (&homingPolicy),
          auditBytes_              (auditSlotBytes),
          candidateBytes_          (auditCandidateBytes),
          stepper_                 (stepperConfig (config, homingPolicy)),
          pendingHoming_           (),
          pendingStepCommand_      ({0, TimePoint (), StepDirection::Stopped, 0,
                                     Duration (), false, StatusCode::Ok}),
          snapshot_                 (emptySnapshot (StatusCode::NotInitialized)),
          candidate_                (),
          confirmationStartedAt_    (),
          lastAcceptedAt_           (),
          cancellationAt_           (),
          identityDigest_           (0),
          bindingRevision_          (0),
          identityImageGeneration_  (0),
          owner_                    (allocateOwner ()),
          candidateGeneration_      (0),
          lastFrameSequence_        (0),
          nextRecordSequence_       (1),
          authorizationEpoch_       (0),
          auditExtent_              (auditSlotByteExtent),
          lastInstalledBytes_       (),
          confirmationDigits_       (),
          auditStride_              (auditSlotStride),
          auditCapacity_            (auditCapacity),
          candidateCapacity_        (auditCandidateCapacity),
          auditCount_               (0),
          confirmationDigitCount_   (0),
          initialized_              (false),
          hasLastFrame_             (false),
          candidatePending_         (false),
          reconciliationRequired_   (false),
          admissionDurable_         (false),
          homingStepPending_        (false),
          startCancellationPending_ (false),
          shutdownCancellation_     (false)
    {
    }

    Status InertPartsCarousel::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }
        if (reconciliationRequired_ && candidatePending_)
        {
            snapshot_.intent.coilBits      = 0;
            snapshot_.intent.gate          = CarouselGateIntent::Closed;
            snapshot_.authorizationCurrent = false;
            snapshot_.status               = StatusCode::HardwareFailure;
            return snapshot_.status;
        }
        const uint32_t durations[] = {config_->confirmationWindow.milliseconds (),
                                      config_->gateIntentDuration.milliseconds    (),
                                      config_->logicalStepInterval.milliseconds   (),
                                      config_->maximumStepCommandAge.milliseconds (),
                                      config_->maximumFrameAge.milliseconds       ()};
        bool           valid =
            config_->binCount > 0 && config_->binCount <= maximumCarouselBins &&
            config_->projectConfigurationId != 0 && config_->confirmationDigits > 0 &&
            config_->confirmationDigits <= 4 &&
            config_->maximumInputSkew.milliseconds () < halfRange &&
            auditBytes_ != nullptr && candidateBytes_ != nullptr &&
            (auditCapacity_ == 2 || auditCapacity_ == 4 || auditCapacity_ == 6 ||
             auditCapacity_ == 8) &&
            auditExtent_ ==
                static_cast<uint16_t> (auditCapacity_) * carouselAuditRecordBytes &&
            auditStride_ == carouselAuditRecordBytes &&
            candidateCapacity_ == carouselAuditRecordBytes &&
            !rangesOverlap (auditBytes_, auditExtent_, candidateBytes_,
                            candidateCapacity_);
        for (uint8_t index = 0; index < 5; ++index)
        {
            valid = valid && durations[index] > 0 && durations[index] < halfRange;
        }
        uint32_t codeLimit = 1;
        for (uint8_t index = 0; index < config_->confirmationDigits; ++index)
        {
            codeLimit *= 10U;
        }
        int32_t minimum = 0;
        int32_t maximum = 0;
        for (uint8_t index = 0; index < config_->binCount; ++index)
        {
            valid   = valid && config_->binConfirmationCodes[index] < codeLimit;
            minimum = config_->binPositions[index] < minimum
                          ? config_->binPositions[index]
                          : minimum;
            maximum = config_->binPositions[index] > maximum
                          ? config_->binPositions[index]
                          : maximum;
            for (uint8_t other = 0; other < index; ++other)
            {
                valid = valid &&
                        config_->binPositions[index] != config_->binPositions[other];
            }
        }
        for (uint8_t index = 0; index < 10; ++index)
        {
            valid = valid && config_->digitKeys[index] != config_->confirmKey &&
                    config_->digitKeys[index] != config_->cancelKey;
            for (uint8_t other = 0; other < index; ++other)
            {
                valid = valid && config_->digitKeys[index] != config_->digitKeys[other];
            }
        }
        valid = valid && config_->confirmKey != config_->cancelKey &&
                config_->logicalStepInterval <= config_->maximumStepCommandAge &&
                minimum < 0 && maximum > 0;
        if (!valid)
        {
            snapshot_       = emptySnapshot (StatusCode::InvalidConfiguration);
            snapshot_.fault = CarouselFault::InvalidFrame;
            return snapshot_.status;
        }

        Status status = identity_->initialize ();
        if (!status.ok                        ())
        {
            snapshot_.fault  = CarouselFault::IdentityStorageFault;
            snapshot_.status = status;
            return status;
        }
        status = homing_->initialize ();
        if (!status.ok               ())
        {
            identity_->shutdown ();
            snapshot_.fault  = CarouselFault::HomingFault;
            snapshot_.status = status;
            return status;
        }
        status = stepper_.initialize ();
        if (!status.ok               ())
        {
            homing_->shutdown   ();
            identity_->shutdown ();
            snapshot_.fault  = CarouselFault::PositionFault;
            snapshot_.status = status;
            return status;
        }

        CarouselAuditRecord previous             = {0, 0,
                                                    0, 0,
                                                    0, 0,
                                                    0, TimePoint (),
                                                    0, CarouselPhase::Uninitialized,
                                                    0, 0,
                                                    0, 0,
                                                    0, CarouselAuditStatus::Success,
                                                    0};
        bool                hasPrevious          = false;
        uint32_t            recoveredOperationId = 0;
        auditCount_                              = 0;
        for (uint8_t slot = 0; slot < auditCapacity_; ++slot)
        {
            const uint8_t* bytes = auditBytes_ + slot * auditStride_;
            if (erased (bytes))
            {
                for (uint8_t tail = slot + 1; tail < auditCapacity_; ++tail)
                {
                    if (!erased (auditBytes_ + tail * auditStride_))
                    {
                        snapshot_.fault  = CarouselFault::AuditCorrupt;
                        snapshot_.status = StatusCode::InternalInvariant;
                        stepper_.reset      ();
                        homing_->shutdown   ();
                        identity_->shutdown ();
                        return snapshot_.status;
                    }
                }
                break;
            }
            CarouselAuditRecord record;
            bool                unsupported = false;
            if (!decodeRecord (bytes, config_->projectConfigurationId,
                               config_->binCount, record, unsupported))
            {
                snapshot_.fault  = unsupported ? CarouselFault::AuditUnsupported
                                               : CarouselFault::AuditCorrupt;
                snapshot_.status = unsupported ? StatusCode::Unsupported
                                               : StatusCode::InternalInvariant;
                stepper_.reset      ();
                homing_->shutdown   ();
                identity_->shutdown ();
                return snapshot_.status;
            }
            if (hasPrevious)
            {
                if (previous.recordKind == 1)
                {
                    if (!pairMatches (previous, record))
                    {
                        snapshot_.fault  = CarouselFault::AuditCorrupt;
                        snapshot_.status = StatusCode::InternalInvariant;
                        stepper_.reset      ();
                        homing_->shutdown   ();
                        identity_->shutdown ();
                        return snapshot_.status;
                    }
                }
                else
                {
                    const uint16_t delta = static_cast<uint16_t> (
                        record.recordSequence - previous.recordSequence);
                    if (record.recordKind != 1 || delta == 0 || delta >= 0x8000U)
                    {
                        snapshot_.fault  = CarouselFault::AuditCorrupt;
                        snapshot_.status = StatusCode::InternalInvariant;
                        stepper_.reset      ();
                        homing_->shutdown   ();
                        identity_->shutdown ();
                        return snapshot_.status;
                    }
                }
            }
            previous             = record;
            recoveredOperationId = record.operationId;
            hasPrevious          = true;
            auditCount_          = static_cast<uint8_t> (slot + 1U);
            nextRecordSequence_  = static_cast<uint16_t> (record.recordSequence + 1U);
        }

        initialized_              = true;
        snapshot_                 = emptySnapshot (StatusCode::Ok);
        snapshot_.phase           = CarouselPhase::Idle;
        snapshot_.operationId     = recoveredOperationId;
        admissionDurable_         = false;
        homingStepPending_        = false;
        startCancellationPending_ = false;
        if (hasPrevious && previous.recordKind == 1)
        {
            if (auditCount_ >= auditCapacity_ ||
                !erased (auditBytes_ + auditCount_ * auditStride_))
            {
                snapshot_.phase  = CarouselPhase::Fault;
                snapshot_.fault  = CarouselFault::AuditCorrupt;
                snapshot_.status = StatusCode::InternalInvariant;
                return snapshot_.status;
            }
            snapshot_.operationId                   = previous.operationId;
            authorizationEpoch_                     = previous.authorizationEpoch;
            identityDigest_                         = previous.identityDigest;
            bindingRevision_                        = previous.bindingRevision;
            identityImageGeneration_                = previous.identityImageGeneration;
            snapshot_.requestedBin                  = previous.binId;
            snapshot_.phase                         = CarouselPhase::Fault;
            snapshot_.fault                         = CarouselFault::AuditIndeterminate;
            snapshot_.status                        = StatusCode::HardwareFailure;
            snapshot_.terminalReconciliationPending = true;
            CarouselAuditRecord recovery = {carouselAuditMagic,
                                            carouselAuditVersion,
                                            carouselAuditRecordBytes,
                                            config_->projectConfigurationId,
                                            previous.operationId,
                                            previous.authorizationEpoch,
                                            nextRecordSequence_,
                                            previous.occurredAt,
                                            3,
                                            CarouselPhase::Fault,
                                            previous.binId,
                                            previous.identityDigest,
                                            previous.bindingRevision,
                                            previous.identityImageGeneration,
                                            0,
                                            CarouselAuditStatus::RecoveredInterrupted,
                                            0};
            snapshot_.auditRecord        = recovery;
            encodeRecord                           (recovery, candidateBytes_);
            snapshot_.auditRecord.checksum = get16 (candidateBytes_, 38);
            candidate_                     = {owner_,
                                              ++candidateGeneration_,
                                              previous.operationId,
                                              auditCount_,
                                              3,
                                              snapshot_.auditRecord.checksum};
            candidatePending_              = true;
            reconciliationRequired_        = true;
        }
        return snapshot_.status;
    }

    void InertPartsCarousel::shutdown () noexcept
    {
        snapshot_.intent.coilBits      = 0;
        snapshot_.intent.gate          = CarouselGateIntent::Closed;
        snapshot_.intent.gateExpiresAt = TimePoint ();
        snapshot_.authorizationCurrent = false;
        if (candidatePending_)
        {
            reconciliationRequired_ = true;
            if (candidate_.recordKind == 1)
            {
                startCancellationPending_ = true;
                cancellationAt_           = lastAcceptedAt_;
                shutdownCancellation_     = true;
            }
        }
        stepper_.reset      ();
        homing_->shutdown   ();
        identity_->shutdown ();
        initialized_      = false;
        admissionDurable_ = false;
        if (!reconciliationRequired_)
        {
            candidatePending_ = false;
        }
        snapshot_.phase  = CarouselPhase::Uninitialized;
        snapshot_.status = StatusCode::NotInitialized;
    }

    void InertPartsCarousel::prepareTerminal (TimePoint now, CarouselPhase phase,
                                              CarouselAuditStatus auditStatus,
                                              CarouselFault       fault,
                                              Status              status) noexcept
    {
        snapshot_.intent.coilBits      = 0;
        snapshot_.intent.gate          = CarouselGateIntent::Closed;
        snapshot_.intent.gateExpiresAt = TimePoint ();
        snapshot_.authorizationCurrent = false;
        snapshot_.phase                = phase;
        snapshot_.fault                = fault;
        snapshot_.status               = status;
        snapshot_.intent.statusCode    = auditStatus;
        homingStepPending_             = false;

        if (!admissionDurable_ || candidatePending_ || auditCount_ >= auditCapacity_)
        {
            return;
        }
        const HomingSnapshot home   = homing_->snapshot ();
        CarouselAuditRecord  record = {carouselAuditMagic,
                                       carouselAuditVersion,
                                       carouselAuditRecordBytes,
                                       config_->projectConfigurationId,
                                       snapshot_.operationId,
                                       authorizationEpoch_,
                                       nextRecordSequence_,
                                       now,
                                       2,
                                       phase,
                                       snapshot_.requestedBin,
                                       identityDigest_,
                                       bindingRevision_,
                                       identityImageGeneration_,
                                       home.positionKnown ? home.homeEpoch : 0,
                                       auditStatus,
                                       0};
        encodeRecord                                    (record, candidateBytes_);
        record.checksum                         = get16 (candidateBytes_, 38);
        snapshot_.auditRecord                   = record;
        snapshot_.hasAuditRecord                = true;
        snapshot_.terminalReconciliationPending = true;
        candidate_                              = {
            owner_, ++candidateGeneration_, snapshot_.operationId, auditCount_,
            2,      record.checksum};
        candidatePending_ = true;
    }

    Status InertPartsCarousel::update (TimePoint                 now,
                                       const CarouselInputFrame& frame) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        const bool stopShape =
            sourceShape (frame.stop.source, CarouselSourceKind::SyntheticStop) &&
            frame.stop.sequence != 0 && frame.stop.qualificationEpoch != 0 &&
            validStatus (frame.stop.status) &&
            current     (now, frame.stop.observedAt, config_->maximumFrameAge);
        if (!stopShape)
        {
            return StatusCode::InvalidArgument;
        }

        const bool stopActive =
            frame.stop.status.ok () && frame.stop.qualified && frame.stop.active;
        HomingInput homingInput = {frame.observedAt, frame.sequence, frame.home,
                                   frame.stop};
        if (stopActive)
        {
            lastAcceptedAt_ = now;
            if (candidatePending_ && candidate_.recordKind == 1)
            {
                startCancellationPending_         = true;
                reconciliationRequired_           = true;
                cancellationAt_                   = now;
                shutdownCancellation_             = false;
                snapshot_.durableAdmissionPending = false;
                admissionDurable_                 = false;
            }
            HomingPreview stopPreview;
            Status        status =
                homing_->preview (now, homingInput, {false, false, 0}, stopPreview);
            if (!status.ok () || !homing_->canCommit (stopPreview))
            {
                return status.ok () ? StatusCode::InternalInvariant : status;
            }
            const StepperSequenceSnapshot child = stepper_.snapshot ();
            if (stepperLive                                         (child) && !stepper_.stop (now).ok ())
            {
                stepper_.reset ();
                snapshot_.phase  = CarouselPhase::Fault;
                snapshot_.fault  = CarouselFault::PositionFault;
                snapshot_.status = StatusCode::InternalInvariant;
            }
            homing_->commit                                    (stopPreview);
            const HomingSnapshot stopped   = homing_->snapshot ();
            snapshot_.intent.coilBits      = 0;
            snapshot_.intent.gate          = CarouselGateIntent::Closed;
            snapshot_.authorizationCurrent = false;
            snapshot_.positionKnown        = stopped.positionKnown;
            snapshot_.logicalPosition      = stopped.logicalPosition;
            snapshot_.homingFault          = stopped.fault;
            if (snapshot_.phase != CarouselPhase::Fault)
            {
                prepareTerminal (now, CarouselPhase::Stopped,
                                 CarouselAuditStatus::Stopped, CarouselFault::None,
                                 StatusCode::Ok);
            }
            return snapshot_.status;
        }
        if (reconciliationRequired_)
        {
            return StatusCode::HardwareFailure;
        }
        if (candidatePending_ && candidate_.recordKind != 1)
        {
            return StatusCode::ResourceBusy;
        }

        bool frameValid =
            frame.sequence != 0 &&
            current (now, frame.observedAt, config_->maximumFrameAge) &&
            (!hasLastFrame_ || (frame.sequence - lastFrameSequence_ > 0 &&
                                frame.sequence - lastFrameSequence_ < halfRange)) &&
            sourceShape (frame.home.source, CarouselSourceKind::SyntheticHome) &&
            frame.home.sequence != 0 && frame.home.qualificationEpoch != 0 &&
            validStatus (frame.home.status) &&
            current     (now, frame.home.observedAt, config_->maximumFrameAge) &&
            withinSkew  (frame.observedAt, frame.home.observedAt,
                        config_->maximumInputSkew) &&
            withinSkew (frame.observedAt, frame.stop.observedAt,
                        config_->maximumInputSkew) &&
            frame.presentation.sequence != 0 &&
            validStatus (frame.presentation.status) &&
            current     (now, frame.presentation.observedAt, config_->maximumFrameAge) &&
            withinSkew  (frame.observedAt, frame.presentation.observedAt,
                        config_->maximumInputSkew);
        frameValid = frameValid &&
                     (frame.hasIdentity
                          ? sourceShape (frame.identity.source,
                                         CarouselSourceKind::SyntheticIdentity) &&
                                frame.identity.sequence != 0 &&
                                validStatus (frame.identity.status) &&
                                current     (now, frame.identity.observedAt,
                                         config_->maximumFrameAge) &&
                                withinSkew (frame.observedAt, frame.identity.observedAt,
                                            config_->maximumInputSkew)
                          : zeroIdentity (frame.identity));
        frameValid =
            frameValid &&
            (frame.hasKey
                 ? sourceShape (frame.key.source, CarouselSourceKind::SyntheticKey) &&
                       frame.key.sequence != 0 && frame.key.digitCount <= 4 &&
                       validStatus (frame.key.status) &&
                       current     (now, frame.key.observedAt, config_->maximumFrameAge) &&
                       withinSkew  (frame.observedAt, frame.key.observedAt,
                                   config_->maximumInputSkew)
                 : zeroKey (frame.key));
        if (frame.hasKey)
        {
            for (uint8_t index = 0; index < 4; ++index)
            {
                frameValid = frameValid && (index < frame.key.digitCount
                                                ? frame.key.digits[index] <= 9
                                                : frame.key.digits[index] == 0);
            }
        }
        if (!frameValid)
        {
            return StatusCode::InvalidArgument;
        }
        if (snapshot_.phase == CarouselPhase::Fault)
        {
            return snapshot_.status;
        }
        const bool presentationFailed = !frame.presentation.status.ok ();
        if (!frame.home.status.ok                                     () || !frame.home.qualified ||
            !frame.stop.status.ok () || !frame.stop.qualified)
        {
            snapshot_.phase = CarouselPhase::Fault;
            snapshot_.fault = CarouselFault::EvidenceFault;
            snapshot_.status =
                !frame.home.status.ok () ? frame.home.status : frame.stop.status;
            return snapshot_.status;
        }
        lastFrameSequence_ = frame.sequence;
        hasLastFrame_      = true;
        lastAcceptedAt_    = now;

        if (snapshot_.phase == CarouselPhase::GateIntent)
        {
            if (presentOrPast (now, snapshot_.intent.gateExpiresAt))
            {
                prepareTerminal (now, CarouselPhase::Complete,
                                 CarouselAuditStatus::Success, CarouselFault::None,
                                 StatusCode::Ok);
            }
            return StatusCode::Ok;
        }

        if (snapshot_.phase == CarouselPhase::AwaitingConfirmation &&
            now.elapsedSince (confirmationStartedAt_).milliseconds () >=
                config_->confirmationWindow.milliseconds ())
        {
            prepareTerminal (now, CarouselPhase::Cancelled,
                             CarouselAuditStatus::AuthorizationExpired,
                             CarouselFault::AuthorizationExpired, StatusCode::Ok);
            return StatusCode::Ok;
        }

        if (frame.hasIdentity)
        {
            if (snapshot_.phase != CarouselPhase::Idle)
            {
                return StatusCode::InvalidArgument;
            }
            Status status = identity_->observe (now, frame.identity);
            if (!status.ok                     ())
            {
                snapshot_.fault  = CarouselFault::IdentityStorageFault;
                snapshot_.status = status;
                return status;
            }
            const IdentityRegistrySnapshot identity = identity_->snapshot ();
            if (identity.disposition != IdentityDisposition::Known)
            {
                snapshot_.identityDisposition = identity.disposition;
                snapshot_.fault = identity.disposition == IdentityDisposition::LockedOut
                                      ? CarouselFault::IdentityLocked
                                      : CarouselFault::IdentityUnknown;
                snapshot_.status = identity.status;
                return identity.status;
            }
            snapshot_.requestedBin        = identity.selectedBin;
            snapshot_.identityDisposition = identity.disposition;
            snapshot_.intent.selectedBin  = identity.selectedBin;
            bindingRevision_              = identity.matchedBindingRevision;
            identityImageGeneration_      = identity.imageGeneration;
            identityDigest_               = identityDigest (frame.identity.identity);
            confirmationStartedAt_        = now;
            confirmationDigitCount_       = 0;
            snapshot_.phase               = CarouselPhase::AwaitingConfirmation;
            snapshot_.fault               = CarouselFault::None;
            snapshot_.status              = StatusCode::Ok;
            return StatusCode::Ok;
        }

        if (snapshot_.phase == CarouselPhase::AwaitingConfirmation && frame.hasKey)
        {
            if (frame.key.cancel || (frame.key.cancel && frame.key.confirm))
            {
                prepareTerminal (now, CarouselPhase::Cancelled,
                                 frame.key.confirm
                                     ? CarouselAuditStatus::ConfirmationConflict
                                     : CarouselAuditStatus::Cancelled,
                                 frame.key.confirm ? CarouselFault::ConfirmationConflict
                                                   : CarouselFault::None,
                                 StatusCode::Ok);
                return StatusCode::Ok;
            }
            if (confirmationDigitCount_ + frame.key.digitCount >
                config_->confirmationDigits)
            {
                prepareTerminal (now, CarouselPhase::Cancelled,
                                 CarouselAuditStatus::ConfirmationConflict,
                                 CarouselFault::ConfirmationConflict, StatusCode::Ok);
                return StatusCode::Ok;
            }
            for (uint8_t index = 0; index < frame.key.digitCount; ++index)
            {
                confirmationDigits_[confirmationDigitCount_++] =
                    frame.key.digits[index];
            }
            if (!frame.key.confirm)
            {
                return StatusCode::Ok;
            }
            uint16_t code = 0;
            for (uint8_t index = 0; index < confirmationDigitCount_; ++index)
            {
                code = static_cast<uint16_t> (code * 10U + confirmationDigits_[index]);
            }
            if (confirmationDigitCount_ != config_->confirmationDigits ||
                code != config_->binConfirmationCodes[snapshot_.requestedBin])
            {
                prepareTerminal (now, CarouselPhase::Cancelled,
                                 CarouselAuditStatus::ConfirmationConflict,
                                 CarouselFault::ConfirmationConflict, StatusCode::Ok);
                return StatusCode::Ok;
            }
            if (auditCount_ + 2U > auditCapacity_)
            {
                snapshot_.phase  = CarouselPhase::Fault;
                snapshot_.fault  = CarouselFault::AuditFull;
                snapshot_.status = StatusCode::CapacityExceeded;
                return snapshot_.status;
            }
            ++snapshot_.operationId;
            if (snapshot_.operationId == 0)
            {
                ++snapshot_.operationId;
            }
            ++authorizationEpoch_;
            if (authorizationEpoch_ == 0)
            {
                ++authorizationEpoch_;
            }
            CarouselAuditRecord record = {carouselAuditMagic,
                                          carouselAuditVersion,
                                          carouselAuditRecordBytes,
                                          config_->projectConfigurationId,
                                          snapshot_.operationId,
                                          authorizationEpoch_,
                                          nextRecordSequence_,
                                          now,
                                          1,
                                          CarouselPhase::Homing,
                                          snapshot_.requestedBin,
                                          identityDigest_,
                                          bindingRevision_,
                                          identityImageGeneration_,
                                          0,
                                          CarouselAuditStatus::Success,
                                          0};
            encodeRecord                              (record, candidateBytes_);
            record.checksum                   = get16 (candidateBytes_, 38);
            snapshot_.auditRecord             = record;
            snapshot_.hasAuditRecord          = true;
            snapshot_.durableAdmissionPending = true;
            candidate_                        = {
                owner_, ++candidateGeneration_, snapshot_.operationId, auditCount_,
                1,      record.checksum};
            candidatePending_ = true;
            return StatusCode::Ok;
        }

        if (!admissionDurable_)
        {
            return StatusCode::Ok;
        }

        if (homingStepPending_)
        {
            StepperSequencePreview stepPreview;
            Status                 stepStatus =
                stepper_.preview (now, pendingStepCommand_, stepPreview);
            if (!stepStatus.ok () || !stepper_.canCommit (stepPreview) ||
                !homing_->canCommit (pendingHoming_))
            {
                stepper_.reset  ();
                prepareTerminal (
                    now, CarouselPhase::Fault, CarouselAuditStatus::PositionFault,
                    CarouselFault::PositionFault,
                    stepStatus.ok () ? StatusCode::InternalInvariant : stepStatus);
                return snapshot_.status;
            }
            const int32_t before                = stepper_.snapshot ().logicalPosition;
            stepStatus                          = stepper_.commit   (stepPreview);
            const StepperSequenceSnapshot after = stepper_.snapshot ();
            if (stepStatus.ok                                       () && after.phase == StepSequencePhase::Moving)
            {
                snapshot_.intent.coilBits = after.coilIntent;
                return StatusCode::Ok;
            }
            const int32_t expected =
                pendingStepCommand_.direction == StepDirection::Forward ? before + 1
                                                                        : before - 1;
            if (!stepStatus.ok () || after.phase != StepSequencePhase::Complete ||
                after.logicalPosition != expected)
            {
                stepper_.reset  ();
                prepareTerminal (
                    now, CarouselPhase::Fault, CarouselAuditStatus::PositionFault,
                    CarouselFault::PositionFault, StatusCode::InternalInvariant);
                return snapshot_.status;
            }
            Status homeStatus  = homing_->commit (pendingHoming_);
            homingStepPending_ = false;
            if (!homeStatus.ok ())
            {
                prepareTerminal (now, CarouselPhase::Fault,
                                 CarouselAuditStatus::HomingFault,
                                 CarouselFault::HomingFault, homeStatus);
                return snapshot_.status;
            }
            const HomingSnapshot advanced = homing_->snapshot ();
            snapshot_.positionKnown       = advanced.positionKnown;
            snapshot_.logicalPosition     = advanced.logicalPosition;
            snapshot_.homingFault         = advanced.fault;
            snapshot_.intent.coilBits     = after.coilIntent;
            snapshot_.phase               = advanced.phase == HomingPhase::Homed
                                                ? CarouselPhase::ReadyAtBin
                                                : (advanced.phase == HomingPhase::Moving
                                                       ? CarouselPhase::Positioning
                                                       : CarouselPhase::Homing);
        }

        HomingCommand command = {homing_->snapshot ().phase ==
                                     HomingPhase::PositionUnknown,
                                 homing_->snapshot ().phase == HomingPhase::Homed ||
                                     homing_->snapshot ().phase == HomingPhase::Moving,
                                 config_->binPositions[snapshot_.requestedBin]};
        HomingPreview homingPreview;
        Status status = homing_->preview (now, homingInput, command, homingPreview);
        if (!status.ok                   () || !homing_->canCommit (homingPreview))
        {
            snapshot_.homingFault = homingPreview.snapshot.fault;
            prepareTerminal (now, CarouselPhase::Fault,
                             CarouselAuditStatus::HomingFault,
                             CarouselFault::HomingFault,
                             status.ok () ? StatusCode::InternalInvariant : status);
            return snapshot_.status;
        }
        if (homingPreview.snapshot.positionKnown &&
            homingPreview.snapshot.logicalPosition == 0 &&
            homing_->snapshot ().homeEpoch == 0)
        {
            if (stepperLive (stepper_.snapshot ()) && !stepper_.stop (now).ok ())
            {
                stepper_.reset  ();
                prepareTerminal (
                    now, CarouselPhase::Fault, CarouselAuditStatus::PositionFault,
                    CarouselFault::PositionFault, StatusCode::InternalInvariant);
                return snapshot_.status;
            }
            stepper_.reset                                          ();
            const StepperSequenceSnapshot child = stepper_.snapshot ();
            if (child.logicalPosition != 0 ||
                child.phase != StepSequencePhase::Inactive || child.coilIntent != 0)
            {
                prepareTerminal (
                    now, CarouselPhase::Fault, CarouselAuditStatus::PositionFault,
                    CarouselFault::PositionFault, StatusCode::InternalInvariant);
                return snapshot_.status;
            }
            homing_->commit (homingPreview);
        }
        else if (homingPreview.snapshot.stepRequested)
        {
            const int8_t   direction   = homingPreview.snapshot.requestedStepDirection;
            StepperCommand stepCommand = {frame.sequence,
                                          now,
                                          direction > 0 ? StepDirection::Forward
                                                        : StepDirection::Reverse,
                                          1,
                                          config_->logicalStepInterval,
                                          false,
                                          StatusCode::Ok};
            StepperSequencePreview stepPreview;
            status = stepper_.preview (now, stepCommand, stepPreview);
            if (!status.ok            () || !stepper_.canCommit (stepPreview))
            {
                prepareTerminal (now, CarouselPhase::Fault,
                                 CarouselAuditStatus::PositionFault,
                                 CarouselFault::PositionFault,
                                 status.ok () ? StatusCode::InternalInvariant : status);
                return snapshot_.status;
            }
            status = stepper_.commit (stepPreview);
            if (!status.ok           ())
            {
                prepareTerminal (now, CarouselPhase::Fault,
                                 CarouselAuditStatus::PositionFault,
                                 CarouselFault::PositionFault, status);
                return snapshot_.status;
            }
            pendingHoming_            = homingPreview;
            pendingStepCommand_       = stepCommand;
            homingStepPending_        = true;
            snapshot_.intent.coilBits = stepper_.snapshot ().coilIntent;
            snapshot_.phase = homingPreview.snapshot.phase == HomingPhase::Moving
                                  ? CarouselPhase::Positioning
                                  : CarouselPhase::Homing;
            return StatusCode::Ok;
        }
        else
        {
            homing_->commit (homingPreview);
        }

        const HomingSnapshot home           = homing_->snapshot ();
        snapshot_.homingFault               = home.fault;
        const StepperSequenceSnapshot child = stepper_.snapshot ();
        snapshot_.positionKnown             = home.positionKnown;
        snapshot_.logicalPosition           = home.logicalPosition;
        snapshot_.intent.coilBits           = child.coilIntent;
        snapshot_.phase = home.phase == HomingPhase::Homed ? CarouselPhase::ReadyAtBin
                                                           : CarouselPhase::Homing;
        if (home.positionKnown &&
            home.logicalPosition == config_->binPositions[snapshot_.requestedBin])
        {
            snapshot_.phase                = CarouselPhase::GateIntent;
            snapshot_.intent.gate          = CarouselGateIntent::Open;
            snapshot_.intent.gateExpiresAt = TimePoint (
                now.milliseconds () + config_->gateIntentDuration.milliseconds ());
            snapshot_.intent.coilBits      = 0;
            snapshot_.authorizationCurrent = true;
        }
        if (presentationFailed)
        {
            snapshot_.fault  = CarouselFault::PresentationFault;
            snapshot_.status = frame.presentation.status;
        }
        return StatusCode::Ok;
    }

    Result<DurableAuditCandidate>
    InertPartsCarousel::previewAuditWrite () const noexcept
    {
        return {candidatePending_ ? StatusCode::Ok : StatusCode::NotInitialized,
                candidate_};
    }

    Result<AuditRecordView> InertPartsCarousel::previewAuditExport (
        const DurableAuditCandidate& candidate) const noexcept
    {
        const bool valid = candidatePending_ && candidate.owner == candidate_.owner &&
                           candidate.generation == candidate_.generation &&
                           candidate.operationId == candidate_.operationId &&
                           candidate.slot == candidate_.slot &&
                           candidate.recordKind == candidate_.recordKind &&
                           candidate.checksum == candidate_.checksum;
        return {valid ? StatusCode::Ok : StatusCode::InvalidArgument,
                {valid ? candidateBytes_ : nullptr,
                 valid ? carouselAuditRecordBytes : static_cast<uint8_t> (0),
                 candidate.slot, candidate.operationId}};
    }

    Status InertPartsCarousel::acknowledgeAuditWrite (
        const DurableAuditCandidate&      candidate,
        const AuditDurableCommitEvidence& evidence) noexcept
    {
        if (candidate.slot < auditCount_ && candidate.recordKind == 1 &&
            candidatePending_ && candidate_.recordKind == 2 &&
            candidate.owner == owner_ && candidate.owner == candidate_.owner &&
            candidate.generation + 1U == candidate_.generation &&
            candidate.operationId == candidate_.operationId &&
            candidate.slot + 1U == candidate_.slot &&
            evidence.owner == candidate.owner &&
            evidence.generation == candidate.generation &&
            evidence.operationId == candidate.operationId &&
            evidence.slot == candidate.slot &&
            evidence.reconciledRecord.bytes ==
                auditBytes_ + candidate.slot * auditStride_ &&
            evidence.reconciledRecord.length == carouselAuditRecordBytes &&
            evidence.reconciledRecord.slot == candidate.slot &&
            evidence.reconciledRecord.operationId == candidate.operationId &&
            evidence.synchronized && evidence.rereadValidated &&
            evidence.durableStatus.ok () &&
            candidate.checksum ==
                get16 (evidence.reconciledRecord.bytes, 38) &&
            bytesEqual (evidence.reconciledRecord.bytes, lastInstalledBytes_,
                        carouselAuditRecordBytes))
        {
            return StatusCode::Ok;
        }
        const bool identity =
            candidate.owner == candidate_.owner &&
            candidate.generation == candidate_.generation &&
            candidate.operationId == candidate_.operationId &&
            candidate.slot == candidate_.slot &&
            candidate.recordKind == candidate_.recordKind &&
            candidate.checksum == candidate_.checksum &&
            evidence.owner == candidate.owner &&
            evidence.generation == candidate.generation &&
            evidence.operationId == candidate.operationId &&
            evidence.slot == candidate.slot &&
            evidence.reconciledRecord.bytes != nullptr &&
            evidence.reconciledRecord.length == carouselAuditRecordBytes &&
            evidence.reconciledRecord.slot == candidate.slot &&
            evidence.reconciledRecord.operationId == candidate.operationId;
        if (!identity)
        {
            return StatusCode::InvalidArgument;
        }
        uint8_t* installed = auditBytes_ + candidate.slot * auditStride_;
        if (evidence.reconciledRecord.bytes != installed)
        {
            return StatusCode::InvalidArgument;
        }
        if (!evidence.synchronized || !evidence.rereadValidated ||
            !evidence.durableStatus.ok ())
        {
            if (!candidatePending_)
            {
                return StatusCode::InvalidArgument;
            }
            reconciliationRequired_ = true;
            snapshot_.fault         = evidence.durableStatus.ok ()
                                          ? CarouselFault::AuditIndeterminate
                                          : CarouselFault::AuditStorageFault;
            snapshot_.status        = StatusCode::HardwareFailure;
            return snapshot_.status;
        }
        if (candidatePending_ && candidate.recordKind == 1 &&
            startCancellationPending_ && erased (installed))
        {
            candidatePending_                 = false;
            reconciliationRequired_           = false;
            startCancellationPending_         = false;
            snapshot_.durableAdmissionPending = false;
            snapshot_.hasAuditRecord          = false;
            snapshot_.authorizationCurrent    = false;
            snapshot_.phase = shutdownCancellation_ ? CarouselPhase::Uninitialized
                                                    : CarouselPhase::Stopped;
            snapshot_.fault = CarouselFault::None;
            snapshot_.status =
                shutdownCancellation_ ? StatusCode::NotInitialized : StatusCode::Ok;
            shutdownCancellation_ = false;
            return StatusCode::Ok;
        }
        if (!candidatePending_)
        {
            return bytesEqual (installed, candidateBytes_, carouselAuditRecordBytes)
                       ? StatusCode::Ok
                       : StatusCode::InvalidArgument;
        }
        if (!bytesEqual (installed, candidateBytes_, carouselAuditRecordBytes))
        {
            reconciliationRequired_ = true;
            snapshot_.fault         = CarouselFault::AuditIndeterminate;
            snapshot_.status        = StatusCode::HardwareFailure;
            return snapshot_.status;
        }
        candidatePending_       = false;
        reconciliationRequired_ = false;
        for (uint8_t index = 0; index < carouselAuditRecordBytes; ++index)
        {
            lastInstalledBytes_[index] = installed[index];
        }
        auditCount_             = static_cast<uint8_t> (candidate.slot + 1U);
        ++nextRecordSequence_;
        snapshot_.hasAuditRecord = true;
        if (candidate.recordKind == 1)
        {
            admissionDurable_                 = true;
            snapshot_.durableAdmissionPending = false;
            if (startCancellationPending_)
            {
                startCancellationPending_      = false;
                snapshot_.authorizationCurrent = false;
                prepareTerminal (cancellationAt_, CarouselPhase::Stopped,
                                 CarouselAuditStatus::Stopped, CarouselFault::None,
                                 StatusCode::Ok);
                if (shutdownCancellation_)
                {
                    snapshot_.phase  = CarouselPhase::Uninitialized;
                    snapshot_.status = StatusCode::NotInitialized;
                }
                shutdownCancellation_ = false;
            }
            else
            {
                snapshot_.authorizationCurrent = true;
                snapshot_.phase                = CarouselPhase::Homing;
            }
        }
        else
        {
            snapshot_.terminalReconciliationPending = false;
            admissionDurable_                       = false;
        }
        if (!initialized_)
        {
            snapshot_.phase  = CarouselPhase::Uninitialized;
            snapshot_.status = StatusCode::NotInitialized;
        }
        return StatusCode::Ok;
    }

    CarouselSnapshot InertPartsCarousel::snapshot () const noexcept
    {
        return snapshot_;
    }
    // clang-format on
} // namespace adk
