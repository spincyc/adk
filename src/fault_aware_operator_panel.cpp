#include "fault_aware_operator_panel.h"

#include <string.h>

namespace adk {
    namespace {

        constexpr uint32_t auditMagic   = UINT32_C (0x41444b41);
        constexpr uint16_t auditVersion = UINT16_C (1);
        constexpr uint32_t fnvOffset    = UINT32_C (0x811c9dc5);
        constexpr uint32_t fnvPrime     = UINT32_C (0x01000193);
        constexpr uint32_t halfRange    = UINT32_C (0x80000000);

        void hashByte (uint32_t& hash, uint8_t value) noexcept
        {
            hash = (hash ^ value) * fnvPrime;
        }

        void hash16 (uint32_t& hash, uint16_t value) noexcept
        {
            hashByte (hash, static_cast<uint8_t> (value));
            hashByte (hash, static_cast<uint8_t> (value >> 8));
        }

        void hash32 (uint32_t& hash, uint32_t value) noexcept
        {
            for (uint8_t index = 0; index < 4; ++index)
            {
                hashByte (hash, static_cast<uint8_t> (value >> (index * 8)));
            }
        }

        void hashDomain (uint32_t& hash, const char* domain) noexcept
        {
            do
            {
                hashByte (hash, static_cast<uint8_t> (*domain));
            }
            while (*domain++ != '\0');
        }

        void hashSource (uint32_t& hash, const OperatorSourceIdentity& source) noexcept
        {
            hash16 (hash, source.sourceId);
            hash16 (hash, source.configurationRevision);
            hash32 (hash, source.sessionEpoch);
        }

        void hashPayloadFields (uint32_t& hash, const PanelAuditRecord& record) noexcept
        {
            hash32     (hash, record.operationId);
            hashByte   (hash, static_cast<uint8_t> (record.kind));
            hashByte   (hash, static_cast<uint8_t> (record.diagnostic));
            hash32     (hash, record.diagnosticGeneration);
            hash16     (hash, record.parentConfigurationRevision);
            hash32     (hash, record.parentInstanceEpoch);
            hash32     (hash, record.parentGeneration);
            hash32     (hash, record.clueGeneration);
            hash16     (hash, record.satisfiedRuleMask);
            hash32     (hash, record.policyDigest);
            hashByte   (hash, record.stopPresent ? 1 : 0);
            hashByte   (hash, record.stopAsserted ? 1 : 0);
            hashSource (hash, record.stopSource);
            hash32     (hash, record.stopSourceSequence);
            hash32     (hash, record.stopObservedAt.microseconds ());
            hash32     (hash, record.occurredAt.microseconds ());
        }

        uint32_t payloadDigest (const PanelAuditRecord& record) noexcept
        {
            uint32_t hash = fnvOffset;
            hashDomain        (hash, "ADK.PANEL.PAYLOAD.V1");
            hashPayloadFields (hash, record);
            return hash;
        }

        void hashRecord (uint32_t& hash, const PanelAuditRecord& record,
                         bool includeChecksum) noexcept
        {
            hash32           (hash, record.formatMagic);
            hash16           (hash, record.formatVersion);
            hash16           (hash, record.configurationRevision);
            hash32           (hash, record.instanceEpoch);
            hash32           (hash, record.recordSequence);

            hashPayloadFields (hash, record);

            hash32           (hash, record.payloadDigest);
            if (includeChecksum)
            {
                hash32 (hash, record.checksum);
            }
            hashByte (hash, static_cast<uint8_t> (record.state));
        }

        uint32_t recordChecksum (const PanelAuditRecord& record) noexcept
        {
            uint32_t hash = fnvOffset;
            hashDomain (hash, "ADK.PANEL.RECORD.V1");
            hashRecord (hash, record, false);
            return hash;
        }

        uint32_t imageDigest (const PanelAuditImage& image) noexcept
        {
            uint32_t hash = fnvOffset;
            hashDomain (hash, "ADK.PANEL.IMAGE.V1");
            for (uint8_t index = 0; index < 2; ++index)
            {
                hashByte   (hash, index);
                hashRecord (hash, image.slots[index], true);
            }
            return hash;
        }

        uint32_t imageDigestWithReplacement (
            const PanelAuditImage& image, uint8_t slot,
            const PanelAuditRecord& replacement) noexcept
        {
            uint32_t hash = fnvOffset;
            hashDomain (hash, "ADK.PANEL.IMAGE.V1");
            for (uint8_t index = 0; index < 2; ++index)
            {
                hashByte   (hash, index);
                hashRecord (hash, index == slot ? replacement : image.slots[index],
                            true);
            }
            return hash;
        }

        bool sourceEqual (const OperatorSourceIdentity& left,
                          const OperatorSourceIdentity& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
               left.sessionEpoch == right.sessionEpoch;
        }

        bool sourceZero (const OperatorSourceIdentity& source) noexcept
        {
            return source.sourceId == 0 && source.configurationRevision == 0 &&
                   source.sessionEpoch == 0;
        }

        bool recordEqual (const PanelAuditRecord& left,
                          const PanelAuditRecord& right) noexcept
        {
            return left.formatMagic == right.formatMagic &&
                   left.formatVersion == right.formatVersion &&
                   left.configurationRevision == right.configurationRevision &&
                   left.instanceEpoch == right.instanceEpoch &&
                   left.recordSequence == right.recordSequence &&
                   left.operationId == right.operationId && left.kind == right.kind &&
                   left.diagnostic == right.diagnostic &&
                   left.diagnosticGeneration == right.diagnosticGeneration &&
                   left.parentConfigurationRevision ==
                       right.parentConfigurationRevision &&
                   left.parentInstanceEpoch == right.parentInstanceEpoch &&
                   left.parentGeneration == right.parentGeneration &&
                   left.clueGeneration == right.clueGeneration &&
                   left.satisfiedRuleMask == right.satisfiedRuleMask &&
                   left.policyDigest == right.policyDigest &&
                   left.stopPresent == right.stopPresent &&
                   left.stopAsserted == right.stopAsserted &&
                   sourceEqual (left.stopSource, right.stopSource) &&
                   left.stopSourceSequence == right.stopSourceSequence &&
                   left.stopObservedAt.microseconds () ==
                       right.stopObservedAt.microseconds () &&
                   left.occurredAt.microseconds () ==
                       right.occurredAt.microseconds () &&
                   left.payloadDigest == right.payloadDigest &&
                   left.checksum == right.checksum && left.state == right.state;
        }

        bool emptyRecord (const PanelAuditRecord& record) noexcept
        {
            const bool stopTimeZero =
                record.stopObservedAt.microseconds () == 0;
            const bool occurredTimeZero =
                record.occurredAt.microseconds () == 0;
            return record.formatMagic == 0 && record.formatVersion == 0 &&
                   record.configurationRevision == 0 && record.instanceEpoch == 0 &&
                   record.recordSequence == 0 && record.operationId == 0 &&
                   record.kind == PanelAuditKind::None &&
                   record.diagnostic == PanelDiagnostic::None &&
                   record.diagnosticGeneration == 0 &&
                   record.parentConfigurationRevision == 0 &&
                   record.parentInstanceEpoch == 0 && record.parentGeneration == 0 &&
                   record.clueGeneration == 0 && record.satisfiedRuleMask == 0 &&
                   record.policyDigest == 0 && !record.stopPresent &&
                   !record.stopAsserted && sourceZero (record.stopSource) &&
                   record.stopSourceSequence == 0 && stopTimeZero &&
                   occurredTimeZero &&
                   record.payloadDigest == 0 && record.checksum == 0 &&
                   record.state == PanelAuditSlotState::Empty;
        }

        bool previewZero (const PanelAuditPreview& preview) noexcept
        {
            return preview.ownerToken == 0 && preview.lifecycleGeneration == 0 &&
                   preview.configurationRevision == 0 &&
                   preview.instanceEpoch == 0 && preview.panelGeneration == 0 &&
                   preview.operationId == 0 && preview.slotIndex == 0 &&
                   emptyRecord (preview.record) && preview.imageDigest == 0;
        }

        bool acknowledgeZero (const PanelAcknowledgePreview& preview) noexcept
        {
            return preview.ownerToken == 0 && preview.lifecycleGeneration == 0 &&
                   preview.configurationRevision == 0 &&
                   preview.instanceEpoch == 0 && preview.panelGeneration == 0 &&
                   preview.operationId == 0 &&
                   preview.diagnostic == PanelDiagnostic::None &&
                   preview.diagnosticGeneration == 0 &&
                   previewZero (preview.audit);
        }

        bool newer (uint32_t candidate, uint32_t prior) noexcept
        {
            const uint32_t distance = candidate - prior;
            return distance != 0 && distance < halfRange;
        }

        uint32_t nextRecordSequence (uint32_t prior) noexcept
        {
            return prior == UINT32_MAX ? 1 : prior + 1;
        }

        bool adjacentRecordSequence (uint32_t candidate, uint32_t prior) noexcept
        {
            return candidate == nextRecordSequence (prior);
        }

        bool validStatus (Status status) noexcept
        {
            return static_cast<uint8_t> (status.error ()) <=
                   static_cast<uint8_t> (StatusCode::HardwareFailure);
        }

        bool validDiagnostic (PanelDiagnostic diagnostic) noexcept
        {
            return static_cast<uint8_t> (diagnostic) <=
                   static_cast<uint8_t> (PanelDiagnostic::Stopped);
        }

        bool validAuditKind (PanelAuditKind kind) noexcept
        {
            return static_cast<uint8_t> (kind) <=
                   static_cast<uint8_t> (PanelAuditKind::StopReleased);
        }

        bool validSlotState (PanelAuditSlotState state) noexcept
        {
            return static_cast<uint8_t> (state) <=
                   static_cast<uint8_t> (PanelAuditSlotState::Committed);
        }

        bool admissibleTime (MicrosecondTimePoint now,
                             MicrosecondTimePoint observedAt,
                             MicrosecondDuration maximumAge) noexcept
        {
            const uint32_t age =
                now.microseconds () - observedAt.microseconds ();
            return age < halfRange && age <= maximumAge.microseconds ();
        }

        bool stopEqual (const OperatorStopEvidence& left,
                        const OperatorStopEvidence& right) noexcept
        {
            return left.asserted == right.asserted &&
                   sourceEqual (left.source, right.source) &&
                   left.sourceSequence == right.sourceSequence &&
                   left.observedAt.microseconds () ==
                       right.observedAt.microseconds () &&
                   left.status == right.status;
        }

        bool controlEqual (const OperatorControlEvidence& left,
                           const OperatorControlEvidence& right) noexcept
        {
            return left.pressedMask == right.pressedMask &&
                   sourceEqual (left.source, right.source) &&
                   left.sourceSequence == right.sourceSequence &&
                   left.observedAt.microseconds () ==
                       right.observedAt.microseconds () &&
                   left.status == right.status;
        }

        bool validRecord (const PanelAuditRecord&              record,
                          const FaultAwareOperatorPanelConfig& config) noexcept
        {
            if (emptyRecord (record))
            {
                return true;
            }
            if (record.formatMagic != auditMagic ||
                record.formatVersion != auditVersion ||
                record.configurationRevision != config.configurationRevision ||
                record.instanceEpoch != config.instanceEpoch ||
                record.recordSequence == 0 ||
                !validAuditKind  (record.kind) ||
                !validDiagnostic (record.diagnostic) ||
                !validSlotState  (record.state) ||
                record.state == PanelAuditSlotState::Empty ||
                record.kind == PanelAuditKind::None ||
                record.payloadDigest != payloadDigest (record) ||

                record.checksum != recordChecksum (record))
            {
                return false;
            }

            if (record.operationId == 0)
            {
                return false;
            }
            const bool solve = record.kind == PanelAuditKind::PuzzleSolved;
            const bool allParentFields =
                record.parentConfigurationRevision != 0 &&
                record.parentInstanceEpoch != 0 && record.parentGeneration != 0 &&
                record.clueGeneration != 0 && record.satisfiedRuleMask != 0 &&
                record.policyDigest != 0;
            const bool anyParentFields =
                record.parentConfigurationRevision != 0 ||
                record.parentInstanceEpoch != 0 || record.parentGeneration != 0 ||
                record.clueGeneration != 0 || record.satisfiedRuleMask != 0 ||
                record.policyDigest != 0;
            if ((solve && !allParentFields) || (!solve && anyParentFields))
            {
                return false;
            }

            const bool acknowledged =
                record.kind == PanelAuditKind::AcknowledgedDiagnostic;
            const bool recoverable =
                record.diagnostic == PanelDiagnostic::InputRecovered ||
                record.diagnostic == PanelDiagnostic::PresentationRecovered;
            if (acknowledged != (recoverable && record.diagnosticGeneration != 0))
            {
                return false;
            }
            if (!acknowledged &&
                (record.diagnostic != PanelDiagnostic::None ||
                 record.diagnosticGeneration != 0))
            {
                return false;
            }

            const bool stop = record.kind == PanelAuditKind::StopAsserted ||
                              record.kind == PanelAuditKind::StopReleased;
            if (stop)
            {
                if (!record.stopPresent ||
                    record.stopAsserted !=
                        (record.kind == PanelAuditKind::StopAsserted) ||
                    !sourceEqual (record.stopSource, config.stopSource) ||
                    record.stopSourceSequence == 0)
                {
                    return false;
                }
            }
            else
            {
                const OperatorSourceIdentity emptySource = {};
                if (record.stopPresent || record.stopAsserted ||
                    !sourceEqual (record.stopSource, emptySource) ||
                    record.stopSourceSequence != 0 ||
                    record.stopObservedAt.microseconds () != 0)
                {
                    return false;
                }
            }
            return true;
        }

        PanelPresentationIntent
        makeIntent (const FaultAwareOperatorPanelSnapshot& snapshot) noexcept
        {
            PanelPresentationIntent intent = PanelPresentationIntent ();
            intent.diagnostic              = snapshot.diagnostic;
            intent.selectedCell            = snapshot.selectedCell;
            intent.diagnosticGeneration    = snapshot.diagnosticGeneration;
            intent.acknowledgeAvailable =
                snapshot.diagnostic == PanelDiagnostic::InputRecovered ||
                snapshot.diagnostic == PanelDiagnostic::PresentationRecovered;
            if (snapshot.stopped)
            {
                intent.mode = PanelPresentationMode::Stopped;
            }
            else if (snapshot.diagnostic != PanelDiagnostic::None)
            {
                intent.mode = PanelPresentationMode::Fault;
            }
            else
            {
                intent.mode = PanelPresentationMode::Ready;
            }
            return intent;
        }
    } // namespace

    FaultAwareOperatorPanel::FaultAwareOperatorPanel (
        const FaultAwareOperatorPanelConfig& config) noexcept
        : config_ (config),
          snapshot_                     (),
          image_                        (),
          auditCandidate_               (),
          retainedStop_                 (),
          retainedControl_              (),
          retainedNow_                  (),
          lifecycleGeneration_          (0),
          failedIntentGeneration_       (0),
          acknowledgeOperationId_       (0),
          initialized_                  (false),
          auditCandidateLive_           (false),
          acknowledgeCandidateLive_     (false),
          stopRetained_                 (false),
          controlRetained_              (false),
          stopTransitionPending_        (false),
          presentationFailureRetained_  (false),
          nowRetained_                  (false),
          lifecycleExhausted_           (false),
          projectDiagnosticRetained_    (false),
          presentationPrimaryRetained_  (false),
          auditDiagnosticRetained_      (false)
    {
    }

    FaultAwareOperatorPanel::~FaultAwareOperatorPanel () noexcept
    {
        shutdown ();
    }

    Status FaultAwareOperatorPanel::initialize () noexcept
    {
        if (initialized_ && !lifecycleExhausted_)
        {
            return StatusCode::Ok;
        }
        if (lifecycleExhausted_ || lifecycleGeneration_ == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        if (config_.configurationRevision == 0 || config_.instanceEpoch == 0 ||
            config_.maximumInputAge.microseconds () == 0 ||
            config_.maximumInputAge.microseconds () >= halfRange ||
            config_.selectableCellCount == 0 || config_.selectableCellCount > 12 ||
            config_.controlSource.sourceId == 0 ||
            config_.controlSource.configurationRevision !=
                config_.configurationRevision ||
            config_.controlSource.sessionEpoch == 0 ||
            config_.stopSource.sourceId == 0 ||
            config_.stopSource.configurationRevision !=
                config_.configurationRevision ||
            config_.stopSource.sessionEpoch == 0 ||
            sourceEqual (config_.controlSource, config_.stopSource))
        {
            return StatusCode::InvalidConfiguration;
        }
        ++lifecycleGeneration_;
        initialized_                    = true;
        snapshot_                       = FaultAwareOperatorPanelSnapshot ();
        snapshot_.configurationRevision = config_.configurationRevision;
        snapshot_.instanceEpoch         = config_.instanceEpoch;
        snapshot_.auditDisposition      = PanelAuditDisposition::Empty;
        snapshot_.status                = StatusCode::Ok;
        return StatusCode::Ok;
    }

    void FaultAwareOperatorPanel::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }
        initialized_              = false;
        auditCandidateLive_       = false;
        acknowledgeCandidateLive_ = false;
        stopRetained_             = false;
        controlRetained_          = false;
        stopTransitionPending_    = false;
        presentationFailureRetained_ = false;
        failedIntentGeneration_       = 0;
        nowRetained_                  = false;
        projectDiagnosticRetained_    = false;
        presentationPrimaryRetained_ = false;
        auditDiagnosticRetained_      = false;
        snapshot_                 = FaultAwareOperatorPanelSnapshot ();

        image_                    = PanelAuditImage ();
        if (lifecycleGeneration_ != UINT32_MAX)
        {
            ++lifecycleGeneration_;
        }
        else
        {
            lifecycleExhausted_ = true;
            snapshot_.status = StatusCode::CapacityExceeded;
        }
    }

    void FaultAwareOperatorPanel::reset () noexcept
    {
        auditCandidateLive_             = false;
        acknowledgeCandidateLive_       = false;
        stopRetained_                   = false;
        controlRetained_                = false;
        stopTransitionPending_          = false;
        presentationFailureRetained_    = false;
        failedIntentGeneration_         = 0;
        nowRetained_                    = false;
        projectDiagnosticRetained_      = false;
        presentationPrimaryRetained_   = false;
        auditDiagnosticRetained_        = false;
        image_                          = PanelAuditImage ();

        snapshot_                       = FaultAwareOperatorPanelSnapshot ();
        snapshot_.configurationRevision = config_.configurationRevision;
        snapshot_.instanceEpoch         = config_.instanceEpoch;
        snapshot_.auditDisposition      = PanelAuditDisposition::Empty;
        if (lifecycleGeneration_ == UINT32_MAX)
        {
            lifecycleExhausted_ = true;
            initialized_        = false;
            snapshot_.status = StatusCode::CapacityExceeded;
            return;
        }
        ++lifecycleGeneration_;
    }

    bool FaultAwareOperatorPanel::initialized () const noexcept
    {
        return initialized_;
    }

    Status FaultAwareOperatorPanel::prepareRecord (
        uint32_t operationId, PanelAuditKind kind, PanelDiagnostic diagnostic,
        uint32_t diagnosticGeneration, uint16_t parentConfigurationRevision,
        uint32_t parentInstanceEpoch, uint32_t parentGeneration,
        uint32_t clueGeneration, uint16_t satisfiedRuleMask, uint32_t policyDigest,
        bool stopPresent, bool stopAsserted, const OperatorSourceIdentity& stopSource,
        uint32_t stopSourceSequence, MicrosecondTimePoint stopObservedAt,
        MicrosecondTimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (lifecycleExhausted_)
        {
            return StatusCode::CapacityExceeded;
        }
        const uint32_t preparationNow = now.microseconds ();

        const uint32_t priorNow = retainedNow_.microseconds ();
        if (nowRetained_ && preparationNow != priorNow &&
            !newer (preparationNow, priorNow))
        {
            return StatusCode::InvalidArgument;
        }
        if (stopPresent &&
            !admissibleTime (now, stopObservedAt,
                             MicrosecondDuration (halfRange - 1)))
        {
            return StatusCode::InvalidArgument;
        }
        if (operationId == 0 || auditCandidateLive_ ||
            snapshot_.auditDisposition == PanelAuditDisposition::Corrupt ||
            snapshot_.auditDisposition == PanelAuditDisposition::Indeterminate)
        {
            return StatusCode::InvalidArgument;
        }

        uint8_t    slot     = 0;
        uint32_t   sequence = 1;
        const bool empty0   = emptyRecord (image_.slots[0]);
        const bool empty1   = emptyRecord (image_.slots[1]);
        if (!empty0 || !empty1)
        {
            if (empty0)
            {
                slot     = 0;
                sequence = nextRecordSequence (image_.slots[1].recordSequence);
            }
            else if (empty1)
            {
                slot     = 1;
                sequence = nextRecordSequence (image_.slots[0].recordSequence);
            }
            else
            {
                const bool zeroNewer = newer (image_.slots[0].recordSequence,
                                              image_.slots[1].recordSequence);
                slot                 = zeroNewer ? 1 : 0;
                sequence = nextRecordSequence (
                    zeroNewer ? image_.slots[0].recordSequence
                              : image_.slots[1].recordSequence);
            }
        }

        PanelAuditRecord record            = PanelAuditRecord ();
        record.formatMagic                 = auditMagic;
        record.formatVersion               = auditVersion;
        record.configurationRevision       = config_.configurationRevision;
        record.instanceEpoch               = config_.instanceEpoch;
        record.recordSequence              = sequence;
        record.operationId                 = operationId;
        record.kind                        = kind;
        record.diagnostic                  = diagnostic;
        record.diagnosticGeneration        = diagnosticGeneration;
        record.parentConfigurationRevision = parentConfigurationRevision;
        record.parentInstanceEpoch         = parentInstanceEpoch;
        record.parentGeneration            = parentGeneration;
        record.clueGeneration              = clueGeneration;
        record.satisfiedRuleMask           = satisfiedRuleMask;
        record.policyDigest                = policyDigest;
        record.stopPresent                 = stopPresent;
        record.stopAsserted                = stopAsserted;
        record.stopSource                  = stopSource;
        record.stopSourceSequence          = stopSourceSequence;
        record.stopObservedAt              = stopObservedAt;
        record.occurredAt                  = now;
        record.state                       = PanelAuditSlotState::Prepared;
        record.payloadDigest               = payloadDigest  (record);
        record.checksum                    = recordChecksum (record);

        auditCandidate_.ownerToken            = reinterpret_cast<uintptr_t> (this);
        auditCandidate_.lifecycleGeneration   = lifecycleGeneration_;
        auditCandidate_.configurationRevision = config_.configurationRevision;
        auditCandidate_.instanceEpoch         = config_.instanceEpoch;
        auditCandidate_.panelGeneration       = snapshot_.generation;
        auditCandidate_.operationId           = operationId;
        auditCandidate_.slotIndex             = slot;
        auditCandidate_.record                = record;

        auditCandidate_.imageDigest =
            imageDigestWithReplacement (image_, slot, record);
        auditCandidateLive_           = true;
        return StatusCode::Ok;
    }

    Result<PanelAuditPreview>
    FaultAwareOperatorPanel::prepareAudit (uint32_t operationId, PanelAuditKind kind,
                                           MicrosecondTimePoint now) noexcept
    {
        if (kind != PanelAuditKind::StopAsserted &&
            kind != PanelAuditKind::StopReleased)
        {
            return {StatusCode::InvalidArgument, PanelAuditPreview ()};
        }
        if (kind == PanelAuditKind::StopAsserted ||
            kind == PanelAuditKind::StopReleased)
        {
            const bool asserted = kind == PanelAuditKind::StopAsserted;
            if (!stopRetained_ || !snapshot_.stopped ||
                retainedStop_.asserted != asserted || !stopTransitionPending_)
            {
                return {StatusCode::InvalidArgument, PanelAuditPreview ()};
            }
            const Status status = prepareRecord (
                operationId, kind, PanelDiagnostic::None, 0, 0, 0, 0, 0, 0, 0,
                true, asserted, retainedStop_.source,
                retainedStop_.sourceSequence, retainedStop_.observedAt, now);
            return {status, status.ok () ? auditCandidate_ : PanelAuditPreview ()};
        }
        return {StatusCode::InvalidArgument, PanelAuditPreview ()};
    }

    Result<PanelAuditPreview> FaultAwareOperatorPanel::preparePuzzleSolved (
        uint32_t operationId, uint16_t parentConfigurationRevision,
        uint32_t parentInstanceEpoch, uint32_t parentGeneration,
        uint32_t clueGeneration, uint16_t satisfiedRuleMask, uint32_t policyDigest,
        MicrosecondTimePoint now) noexcept
    {
        const OperatorSourceIdentity source = {};
        if (parentConfigurationRevision == 0 || parentInstanceEpoch == 0 ||
            parentGeneration == 0 || clueGeneration == 0 || satisfiedRuleMask == 0 ||
            policyDigest == 0)
        {
            return {StatusCode::InvalidArgument, PanelAuditPreview ()};
        }
        const Status status = prepareRecord (
            operationId, PanelAuditKind::PuzzleSolved, PanelDiagnostic::None, 0,
            parentConfigurationRevision, parentInstanceEpoch, parentGeneration,
            clueGeneration, satisfiedRuleMask, policyDigest, false, false, source, 0,
            MicrosecondTimePoint (), now);
        return {status, status.ok () ? auditCandidate_ : PanelAuditPreview ()};
    }

    bool FaultAwareOperatorPanel::canAcknowledgeAudit (
        const PanelAuditPreview& preview) const noexcept
    {
        return initialized_ && auditCandidateLive_ &&
               preview.ownerToken == reinterpret_cast<uintptr_t> (this) &&
               preview.lifecycleGeneration == lifecycleGeneration_ &&
               preview.configurationRevision == config_.configurationRevision &&
               preview.instanceEpoch == config_.instanceEpoch &&
               preview.panelGeneration == snapshot_.generation &&
               preview.operationId == auditCandidate_.operationId &&
               preview.slotIndex == auditCandidate_.slotIndex &&
               preview.imageDigest == auditCandidate_.imageDigest &&
               recordEqual (preview.record, auditCandidate_.record);
    }

    Result<PanelAcknowledgePreview>
    FaultAwareOperatorPanel::prepareAcknowledge (uint32_t             operationId,
                                                 MicrosecondTimePoint now) noexcept
    {
        const PanelAcknowledgePreview empty = PanelAcknowledgePreview ();
        const bool                    acknowledgeable =
            snapshot_.diagnostic == PanelDiagnostic::InputRecovered ||
            snapshot_.diagnostic == PanelDiagnostic::PresentationRecovered;
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, empty};
        }
        if (!acknowledgeable || operationId == 0 || acknowledgeCandidateLive_)
        {
            return {StatusCode::InvalidArgument, empty};
        }
        const OperatorSourceIdentity    source = {};
        const Status auditStatus = prepareRecord (
            operationId, PanelAuditKind::AcknowledgedDiagnostic, snapshot_.diagnostic,
            snapshot_.diagnosticGeneration, 0, 0, 0, 0, 0, 0, false, false, source, 0,
            MicrosecondTimePoint (), now);
        if (!auditStatus.ok ())
        {
            return {auditStatus, empty};
        }
        PanelAcknowledgePreview preview = PanelAcknowledgePreview ();
        preview.ownerToken              = reinterpret_cast<uintptr_t> (this);
        preview.lifecycleGeneration     = lifecycleGeneration_;
        preview.configurationRevision   = config_.configurationRevision;
        preview.instanceEpoch           = config_.instanceEpoch;
        preview.panelGeneration         = snapshot_.generation;
        preview.operationId             = operationId;
        preview.diagnostic              = snapshot_.diagnostic;
        preview.diagnosticGeneration    = snapshot_.diagnosticGeneration;
        preview.audit                   = auditCandidate_;
        acknowledgeOperationId_         = operationId;
        acknowledgeCandidateLive_       = true;
        return {StatusCode::Ok, preview};
    }

    Status FaultAwareOperatorPanel::preflightUpdate (
        const FaultAwareOperatorPanelInput& input,
        PreparedUpdate& prepared) const noexcept
    {
        return preflightUpdateInternal (input, false, PanelDiagnostic::None, 0,
                                        true, prepared);
    }

    Status FaultAwareOperatorPanel::preflightProjectUpdate (
        MicrosecondTimePoint now, bool auditImagePresent,
        const PanelAuditImage& auditImage, bool stopPresent,
        const OperatorStopEvidence& stop, bool controlPresent,
        const OperatorControlEvidence& control, bool auditAcknowledgePresent,
        const PanelAuditPreview& auditAcknowledge, bool acknowledgePresent,
        const PanelAcknowledgePreview& acknowledge, bool presentationPresent,
        const PanelPresentationEvidence& presentation,
        PanelDiagnostic derivedDiagnostic, uint32_t derivedGeneration,
        bool puzzleSolveEligible, PreparedUpdate& prepared) const noexcept
    {
        const ProjectUpdateView input = {
            now,
            auditImagePresent,
            auditImage,
            stopPresent,
            stop,
            controlPresent,
            control,
            false,
            PanelDiagnostic::None,
            0,
            auditAcknowledgePresent,
            auditAcknowledge,
            acknowledgePresent,
            acknowledge,
            presentationPresent,
            presentation};
        return preflightUpdateInternal (
            input, true, derivedDiagnostic, derivedGeneration,
            puzzleSolveEligible, prepared);
    }

    template <typename Input>
    Status FaultAwareOperatorPanel::preflightUpdateInternal (
        const Input& input, bool projectOverride,
        PanelDiagnostic derivedDiagnostic, uint32_t derivedGeneration,
        bool puzzleSolveEligible, PreparedUpdate& prepared) const noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (projectOverride &&
            (!validDiagnostic (derivedDiagnostic) ||
             (derivedDiagnostic == PanelDiagnostic::None) !=
                 (derivedGeneration == 0) ||
             derivedDiagnostic == PanelDiagnostic::InputRecovered ||
             derivedDiagnostic == PanelDiagnostic::PresentationRecovered ||
             input.diagnosticPresent))
        {
            return StatusCode::InvalidArgument;
        }
        if (input.auditAcknowledgePresent && input.acknowledgePresent)
        {
            return StatusCode::InvalidArgument;
        }
        const uint32_t inputNow    = input.now.microseconds ();

        const uint32_t retainedNow = retainedNow_.microseconds ();
        if (nowRetained_ && inputNow != retainedNow &&
            !newer (inputNow, retainedNow))
        {
            return StatusCode::InvalidArgument;
        }
        if ((!input.auditImagePresent &&
             (!emptyRecord (input.auditImage.slots[0]) ||
              !emptyRecord (input.auditImage.slots[1]))) ||
            (!input.stopPresent &&
             (input.stop.asserted || !sourceZero (input.stop.source) ||
              input.stop.sourceSequence != 0 ||
              input.stop.observedAt.microseconds () != 0 ||

              !input.stop.status.ok           ())) ||
            (!input.controlPresent &&
             (input.control.pressedMask != 0 ||
              !sourceZero (input.control.source) ||
              input.control.sourceSequence != 0 ||
              input.control.observedAt.microseconds () != 0 ||

              !input.control.status.ok           ())) ||
            (!input.auditAcknowledgePresent &&
             !previewZero (input.auditAcknowledge)) ||
            (!input.acknowledgePresent &&
             !acknowledgeZero (input.acknowledge)) ||
            (!input.presentationPresent &&
             (input.presentation.intentGeneration != 0 ||
              input.presentation.observedAt.microseconds () != 0 ||

              !input.presentation.status.ok           ())))
        {
            return StatusCode::InvalidArgument;
        }
        if (input.diagnosticPresent &&
            (input.diagnostic == PanelDiagnostic::None ||
             input.diagnostic == PanelDiagnostic::InputRecovered ||
             input.diagnostic == PanelDiagnostic::PresentationRecovered))
        {
            return StatusCode::InvalidArgument;
        }
        if (!input.diagnosticPresent && (input.diagnostic != PanelDiagnostic::None ||
                                         input.diagnosticGeneration != 0))
        {
            return StatusCode::InvalidArgument;
        }
        const bool stopSourceInvalid =
            !sourceEqual (input.stop.source, config_.stopSource);
        const bool stopTimeInvalid =
            !admissibleTime (input.now, input.stop.observedAt,
                             config_.maximumInputAge);
        const bool stopInvalid =
            input.stopPresent &&
            (stopSourceInvalid || input.stop.sourceSequence == 0 ||
             !validStatus (input.stop.status) || stopTimeInvalid);
        const bool controlSourceInvalid =
            !sourceEqual (input.control.source, config_.controlSource);
        const bool controlTimeInvalid =
            !admissibleTime (input.now, input.control.observedAt,
                             config_.maximumInputAge);
        const bool controlMaskInvalid =
            (input.control.pressedMask & UINT8_C (0xf0)) != 0;
        const bool controlInvalid =
            input.controlPresent &&
            (controlSourceInvalid || input.control.sourceSequence == 0 ||
             controlMaskInvalid || !validStatus (input.control.status) ||
             controlTimeInvalid);
        const bool presentationInvalid =
            input.presentationPresent &&
            (!validStatus (input.presentation.status) ||
             input.presentation.intentGeneration != snapshot_.generation ||
             !admissibleTime (input.now, input.presentation.observedAt,
                              config_.maximumInputAge));
        if (!validDiagnostic (input.diagnostic) ||
            (input.diagnosticPresent && input.diagnosticGeneration == 0) ||
            stopInvalid || controlInvalid || presentationInvalid)
        {
            return StatusCode::InvalidArgument;
        }
        if (input.stopPresent && stopRetained_)
        {
            if (input.stop.sourceSequence == retainedStop_.sourceSequence)
            {
                if (!stopEqual (input.stop, retainedStop_))
                {
                    return StatusCode::InvalidArgument;
                }
            }
            else
            {
                const bool release =
                    snapshot_.stopped && !input.stop.asserted;
                const bool timeInvalid =
                    release
                        ? !newer (input.stop.observedAt.microseconds (),
                                  retainedStop_.observedAt.microseconds ())
                        : input.stop.observedAt.microseconds () !=
                                  retainedStop_.observedAt.microseconds () &&
                              !newer (input.stop.observedAt.microseconds (),
                                      retainedStop_.observedAt.microseconds ());
                if (!newer (input.stop.sourceSequence,
                            retainedStop_.sourceSequence) ||
                    timeInvalid)
                {
                    return StatusCode::InvalidArgument;
                }
            }
        }
        if (input.controlPresent && controlRetained_)
        {
            if (input.control.sourceSequence == retainedControl_.sourceSequence)
            {
                if (!controlEqual (input.control, retainedControl_))
                {
                    return StatusCode::InvalidArgument;
                }
            }
            else if (!newer (input.control.sourceSequence,
                             retainedControl_.sourceSequence) ||
                     (input.control.observedAt.microseconds () !=
                          retainedControl_.observedAt.microseconds () &&
                      !newer (input.control.observedAt.microseconds (),
                              retainedControl_.observedAt.microseconds ())))
            {
                return StatusCode::InvalidArgument;
            }
        }
        if (input.auditAcknowledgePresent &&
            (!canAcknowledgeAudit (input.auditAcknowledge) ||
             !input.auditImagePresent || input.auditAcknowledge.slotIndex > 1 ||
             !recordEqual (
                 input.auditImage.slots[input.auditAcknowledge.slotIndex],
                 input.auditAcknowledge.record) ||
             imageDigest (input.auditImage) !=
                 input.auditAcknowledge.imageDigest))
        {
            return StatusCode::InvalidArgument;
        }
        if (input.acknowledgePresent)
        {
            const PanelAcknowledgePreview& preview = input.acknowledge;
            if (!acknowledgeCandidateLive_ ||
                preview.ownerToken != reinterpret_cast<uintptr_t> (this) ||
                preview.lifecycleGeneration != lifecycleGeneration_ ||
                preview.configurationRevision != config_.configurationRevision ||
                preview.instanceEpoch != config_.instanceEpoch ||
                preview.panelGeneration != snapshot_.generation ||
                preview.operationId != acknowledgeOperationId_ ||
                preview.diagnostic != snapshot_.diagnostic ||
                preview.diagnosticGeneration != snapshot_.diagnosticGeneration ||
                !canAcknowledgeAudit (preview.audit) ||
                !input.auditImagePresent || preview.audit.slotIndex > 1 ||
                !recordEqual (input.auditImage.slots[preview.audit.slotIndex],
                              preview.audit.record) ||
                imageDigest (input.auditImage) != preview.audit.imageDigest)
            {
                return StatusCode::InvalidArgument;
            }
        }

        prepared.snapshot           = snapshot_;
        prepared.image              = image_;
        prepared.consumeAudit       = false;
        prepared.consumeAcknowledge = false;
        prepared.invalidateCandidates =
            auditCandidateLive_ || acknowledgeCandidateLive_;
        prepared.retainStop         = stopRetained_;
        prepared.stop               = retainedStop_;
        prepared.retainControl      = controlRetained_;
        prepared.control            = retainedControl_;
        prepared.stopTransitionPending = stopTransitionPending_;
        prepared.presentationFailureRetained =
            presentationFailureRetained_;
        prepared.failedIntentGeneration = failedIntentGeneration_;
        prepared.now                     = input.now;
        prepared.retainNow               = true;
        prepared.projectDiagnosticRetained =
            projectDiagnosticRetained_;
        prepared.presentationOnlyDiagnostic = false;
        prepared.presentationPrimaryRetained =
            presentationPrimaryRetained_;
        prepared.auditDiagnosticRetained = auditDiagnosticRetained_;
        if (prepared.auditDiagnosticRetained &&
            prepared.snapshot.diagnostic == PanelDiagnostic::AuditIndeterminate)
        {
            prepared.snapshot.diagnostic           = PanelDiagnostic::None;
            prepared.snapshot.diagnosticGeneration = 0;
        }
        if (prepared.presentationPrimaryRetained &&
            prepared.snapshot.diagnostic == PanelDiagnostic::SourceFault)
        {
            prepared.snapshot.diagnostic           = PanelDiagnostic::None;
            prepared.snapshot.diagnosticGeneration = 0;
        }

        if (input.auditImagePresent)
        {
            if ((!emptyRecord (input.auditImage.slots[0]) &&
                 !admissibleTime (input.now, input.auditImage.slots[0].occurredAt,
                                  MicrosecondDuration (halfRange - 1))) ||
                (!emptyRecord (input.auditImage.slots[1]) &&
                 !admissibleTime (input.now, input.auditImage.slots[1].occurredAt,
                                  MicrosecondDuration (halfRange - 1))))
            {
                return StatusCode::InvalidArgument;
            }
            if (!validRecord (input.auditImage.slots[0], config_) ||
                !validRecord (input.auditImage.slots[1], config_))
            {
                prepared.snapshot.auditDisposition = PanelAuditDisposition::Corrupt;
                if (prepared.snapshot.diagnostic !=
                    PanelDiagnostic::AuditIndeterminate)
                {
                    prepared.snapshot.diagnostic =
                        PanelDiagnostic::AuditIndeterminate;
                    prepared.auditDiagnosticRetained = true;
                }
                prepared.image = input.auditImage;
            }
            else
            {
                const bool empty0 = emptyRecord (input.auditImage.slots[0]);
                const bool empty1 = emptyRecord (input.auditImage.slots[1]);
                const PanelAuditSlotState state0 = input.auditImage.slots[0].state;
                const PanelAuditSlotState state1 = input.auditImage.slots[1].state;
                const bool candidate0 =
                    canAcknowledgeAudit (auditCandidate_) &&
                    auditCandidate_.slotIndex == 0 &&
                    recordEqual (input.auditImage.slots[0],
                                 auditCandidate_.record) &&
                    imageDigest (input.auditImage) ==
                        auditCandidate_.imageDigest;
                const bool candidate1 =
                    canAcknowledgeAudit (auditCandidate_) &&
                    auditCandidate_.slotIndex == 1 &&
                    recordEqual (input.auditImage.slots[1],
                                 auditCandidate_.record) &&
                    imageDigest (input.auditImage) ==
                        auditCandidate_.imageDigest;
                if (empty0 && empty1)
                {
                    prepared.snapshot.auditDisposition =
                        PanelAuditDisposition::PrepareRequired;
                }
                else if ((empty0 && state1 == PanelAuditSlotState::Committed) ||
                         (empty1 && state0 == PanelAuditSlotState::Committed))
                {
                    prepared.snapshot.auditDisposition = PanelAuditDisposition::Ready;
                }
                else if (empty0 && state1 == PanelAuditSlotState::Prepared &&
                         input.auditImage.slots[1].recordSequence == 1 &&
                         candidate1)
                {
                    prepared.snapshot.auditDisposition =
                        PanelAuditDisposition::AcknowledgeRequired;
                }
                else if (empty1 && state0 == PanelAuditSlotState::Prepared &&
                         input.auditImage.slots[0].recordSequence == 1 &&
                         candidate0)
                {
                    prepared.snapshot.auditDisposition =
                        PanelAuditDisposition::AcknowledgeRequired;
                }
                else if (state0 == PanelAuditSlotState::Prepared &&
                         state1 == PanelAuditSlotState::Committed &&
                         adjacentRecordSequence (
                             input.auditImage.slots[0].recordSequence,
                             input.auditImage.slots[1].recordSequence) &&
                         candidate0)
                {
                    prepared.snapshot.auditDisposition =
                        PanelAuditDisposition::AcknowledgeRequired;
                }
                else if (state1 == PanelAuditSlotState::Prepared &&
                         state0 == PanelAuditSlotState::Committed &&
                         adjacentRecordSequence (
                             input.auditImage.slots[1].recordSequence,
                             input.auditImage.slots[0].recordSequence) &&
                         candidate1)
                {
                    prepared.snapshot.auditDisposition =
                        PanelAuditDisposition::AcknowledgeRequired;
                }
                else if (state0 == PanelAuditSlotState::Committed &&
                         state1 == PanelAuditSlotState::Committed &&
                         ((newer (input.auditImage.slots[0].recordSequence,
                                  input.auditImage.slots[1].recordSequence) &&
                           adjacentRecordSequence (
                               input.auditImage.slots[0].recordSequence,
                               input.auditImage.slots[1].recordSequence)) ||
                          (newer (input.auditImage.slots[1].recordSequence,
                                  input.auditImage.slots[0].recordSequence) &&
                           adjacentRecordSequence (
                               input.auditImage.slots[1].recordSequence,
                               input.auditImage.slots[0].recordSequence))))
                {
                    prepared.snapshot.auditDisposition = PanelAuditDisposition::Ready;
                }
                else
                {
                    prepared.snapshot.auditDisposition =
                        PanelAuditDisposition::Indeterminate;
                    if (prepared.snapshot.diagnostic !=
                        PanelDiagnostic::AuditIndeterminate)
                    {
                        prepared.snapshot.diagnostic =
                            PanelDiagnostic::AuditIndeterminate;
                        prepared.auditDiagnosticRetained = true;
                    }
                }
                prepared.image = input.auditImage;

                if (!prepared.retainStop &&
                    (prepared.snapshot.auditDisposition ==
                         PanelAuditDisposition::Ready ||
                     prepared.snapshot.auditDisposition ==
                         PanelAuditDisposition::AcknowledgeRequired))
                {
                    const PanelAuditRecord* latest = nullptr;
                    for (uint8_t index = 0; index < 2; ++index)
                    {
                        const PanelAuditRecord& record =
                            input.auditImage.slots[index];
                        if (record.state == PanelAuditSlotState::Committed &&
                            (record.kind == PanelAuditKind::StopAsserted ||
                             record.kind == PanelAuditKind::StopReleased) &&
                            (latest == nullptr ||
                             newer (record.recordSequence,
                                    latest->recordSequence)))
                        {
                            latest = &record;
                        }
                    }
                    if (latest != nullptr)
                    {
                        prepared.snapshot.stopped =
                            latest->kind == PanelAuditKind::StopAsserted;
                        prepared.stop.asserted =
                            latest->kind == PanelAuditKind::StopAsserted;
                        prepared.stop.source = latest->stopSource;
                        prepared.stop.sourceSequence =
                            latest->stopSourceSequence;
                        prepared.stop.observedAt = latest->stopObservedAt;
                        prepared.stop.status     = StatusCode::Ok;
                        prepared.retainStop      = true;
                    }
                }
            }
        }

        if (input.stopPresent && !stopRetained_ && prepared.retainStop)
        {
            if (input.stop.sourceSequence == prepared.stop.sourceSequence)
            {
                if (!stopEqual (input.stop, prepared.stop))
                {
                    return StatusCode::InvalidArgument;
                }
            }
            else if (!newer (input.stop.sourceSequence,
                             prepared.stop.sourceSequence) ||
                     !newer (input.stop.observedAt.microseconds (),
                             prepared.stop.observedAt.microseconds ()))
            {
                return StatusCode::InvalidArgument;
            }
        }

        const bool retainedStopChanged =
            !stopRetained_ || !stopEqual (input.stop, retainedStop_);
        const bool assertingStop = input.stopPresent && input.stop.status.ok () &&
                                   input.stop.asserted && retainedStopChanged;
        const bool puzzleAuditRequested =
            input.auditAcknowledgePresent &&
            input.auditAcknowledge.record.kind == PanelAuditKind::PuzzleSolved;

        if (input.auditAcknowledgePresent && !assertingStop &&
            !puzzleAuditRequested)
        {
            if (!canAcknowledgeAudit (input.auditAcknowledge) ||
                !input.auditImagePresent || input.auditAcknowledge.slotIndex > 1 ||
                !recordEqual (input.auditImage.slots[input.auditAcknowledge.slotIndex],
                              input.auditAcknowledge.record) ||
                imageDigest (input.auditImage) != input.auditAcknowledge.imageDigest)
            {
                return StatusCode::InvalidArgument;
            }
            PanelAuditRecord committed = input.auditAcknowledge.record;
            committed.state            = PanelAuditSlotState::Committed;
            committed.checksum         = recordChecksum (committed);
            prepared.image             = input.auditImage;
            prepared.image.slots[input.auditAcknowledge.slotIndex] = committed;
            prepared.snapshot.auditDisposition = PanelAuditDisposition::Ready;
            prepared.consumeAudit              = true;
            if (committed.kind == PanelAuditKind::StopAsserted ||
                committed.kind == PanelAuditKind::StopReleased)
            {
                prepared.stopTransitionPending = false;
            }
            if (committed.kind == PanelAuditKind::StopReleased &&
                prepared.retainStop && !prepared.stop.asserted &&
                committed.stopSourceSequence == prepared.stop.sourceSequence &&
                committed.stopObservedAt.microseconds () ==
                    prepared.stop.observedAt.microseconds ())
            {
                prepared.snapshot.stopped = false;
                if (prepared.snapshot.diagnostic == PanelDiagnostic::Stopped)
                {
                    prepared.snapshot.diagnostic = PanelDiagnostic::None;
                    prepared.snapshot.diagnosticGeneration = 0;
                }
            }
        }

        if (input.stopPresent)
        {
            const PanelDiagnostic priorDiagnostic = prepared.snapshot.diagnostic;
            const bool duplicate =
                prepared.retainStop && stopEqual (input.stop, prepared.stop);
            if (!input.stop.status.ok ())
            {
                prepared.snapshot.diagnostic = PanelDiagnostic::SourceFault;
                prepared.snapshot.chordDisposition =
                    OperatorChordDisposition::InvalidEvidence;
            }
            else if (!duplicate)
            {
                bool admitStop = true;
                const bool release =
                    prepared.snapshot.stopped && !input.stop.asserted &&
                    prepared.retainStop &&
                    newer (input.stop.sourceSequence, prepared.stop.sourceSequence) &&
                    newer (input.stop.observedAt.microseconds (),
                           prepared.stop.observedAt.microseconds ());
                if (input.stop.asserted)
                {
                    prepared.snapshot.stopped = true;
                    prepared.stopTransitionPending =
                        !prepared.retainStop || !prepared.stop.asserted;
                    prepared.invalidateCandidates = true;
                }
                else if (release)
                {
                    const PanelAuditRecord* latest = nullptr;
                    for (uint8_t index = 0; index < 2; ++index)
                    {
                        const PanelAuditRecord& record = prepared.image.slots[index];
                        if (record.kind == PanelAuditKind::StopReleased &&
                            record.state == PanelAuditSlotState::Committed &&
                            record.stopSourceSequence == input.stop.sourceSequence &&
                            record.stopObservedAt.microseconds () ==
                                input.stop.observedAt.microseconds ())
                        {
                            latest = &record;
                        }
                    }
                    prepared.snapshot.stopped = latest == nullptr;
                    prepared.stopTransitionPending = latest == nullptr;
                }
                else if (prepared.snapshot.stopped)
                {
                    prepared.snapshot.diagnostic = PanelDiagnostic::TimingFault;
                    prepared.snapshot.chordDisposition =
                        OperatorChordDisposition::InvalidEvidence;
                    admitStop = false;
                }
                if (admitStop)
                {
                    prepared.stop       = input.stop;
                    prepared.retainStop = true;
                    if (!input.stop.asserted &&
                        (priorDiagnostic == PanelDiagnostic::SourceFault ||
                         priorDiagnostic == PanelDiagnostic::TimingFault))
                    {
                        prepared.snapshot.diagnostic =
                            PanelDiagnostic::InputRecovered;
                        ++prepared.snapshot.diagnosticGeneration;
                    }
                }
            }
        }

        if (input.controlPresent)
        {
            const PanelDiagnostic priorDiagnostic = prepared.snapshot.diagnostic;
            const bool duplicate =
                prepared.retainControl &&
                controlEqual (input.control, prepared.control);
            if (!input.control.status.ok () ||
                (input.control.pressedMask & UINT8_C (0xf0)) != 0)
            {
                prepared.snapshot.chordDisposition =
                    OperatorChordDisposition::InvalidEvidence;
                prepared.snapshot.diagnostic = PanelDiagnostic::SourceFault;
            }
            else if (!duplicate)
            {
                const uint8_t mask = input.control.pressedMask;
                if (mask != 0 && (mask & (mask - 1)) != 0)
                {
                    prepared.snapshot.chordDisposition =
                        OperatorChordDisposition::InvalidChord;
                    prepared.snapshot.diagnostic =
                        PanelDiagnostic::OperatorChordInvalid;
                }
                else if (mask != 0)
                {
                    prepared.snapshot.chordDisposition =
                        OperatorChordDisposition::SingleControl;
                    if (!prepared.snapshot.stopped && mask == UINT8_C (1))
                    {
                        prepared.snapshot.selectedCell =
                            static_cast<uint8_t> (
                                prepared.snapshot.selectedCell == 0
                                    ? config_.selectableCellCount - 1
                                    : prepared.snapshot.selectedCell - 1);
                    }
                    else if (!prepared.snapshot.stopped && mask == UINT8_C (2))
                    {
                        prepared.snapshot.selectedCell =
                            static_cast<uint8_t> (
                                (prepared.snapshot.selectedCell + 1) %
                                config_.selectableCellCount);
                    }
                }
                if (priorDiagnostic == PanelDiagnostic::SourceFault ||
                    priorDiagnostic == PanelDiagnostic::TimingFault)
                {
                    prepared.snapshot.diagnostic =
                        PanelDiagnostic::InputRecovered;
                    ++prepared.snapshot.diagnosticGeneration;
                }
                prepared.control       = input.control;
                prepared.retainControl = true;
            }
        }

        if (input.acknowledgePresent && !prepared.snapshot.stopped &&
            !assertingStop)
        {
            const PanelAcknowledgePreview& preview = input.acknowledge;
            if (!acknowledgeCandidateLive_ ||
                preview.ownerToken != reinterpret_cast<uintptr_t> (this) ||
                preview.lifecycleGeneration != lifecycleGeneration_ ||
                preview.panelGeneration != snapshot_.generation ||
                preview.operationId != acknowledgeOperationId_ ||
                preview.diagnostic != snapshot_.diagnostic ||
                preview.diagnosticGeneration != snapshot_.diagnosticGeneration ||
                !canAcknowledgeAudit (preview.audit))
            {
                return StatusCode::InvalidArgument;
            }
            if (!input.auditImagePresent || preview.audit.slotIndex > 1 ||
                !recordEqual (input.auditImage.slots[preview.audit.slotIndex],
                              preview.audit.record) ||
                imageDigest (input.auditImage) != preview.audit.imageDigest)
            {
                return StatusCode::InvalidArgument;
            }
            PanelAuditRecord committed = preview.audit.record;
            committed.state            = PanelAuditSlotState::Committed;
            committed.checksum         = recordChecksum (committed);
            prepared.image             = input.auditImage;
            prepared.image.slots[preview.audit.slotIndex] = committed;
            prepared.snapshot.auditDisposition = PanelAuditDisposition::Ready;
            prepared.snapshot.diagnostic = PanelDiagnostic::None;
            prepared.projectDiagnosticRetained = false;
            prepared.consumeAcknowledge  = true;
            prepared.consumeAudit        = true;
        }

        if (projectOverride)
        {
            const bool priorProjectUnchanged =
                prepared.projectDiagnosticRetained &&
                prepared.snapshot.diagnostic == snapshot_.diagnostic &&
                prepared.snapshot.diagnosticGeneration ==
                    snapshot_.diagnosticGeneration;
            const bool lowerPanelDiagnostic =
                prepared.snapshot.diagnostic == PanelDiagnostic::None ||
                prepared.snapshot.diagnostic ==
                    PanelDiagnostic::OperatorChordInvalid;
            const bool applyDerived =
                priorProjectUnchanged ||
                (derivedDiagnostic != PanelDiagnostic::None &&
                 lowerPanelDiagnostic);
            if (applyDerived)
            {
                prepared.snapshot.diagnostic           = derivedDiagnostic;
                prepared.snapshot.diagnosticGeneration = derivedGeneration;
                prepared.projectDiagnosticRetained =
                    derivedDiagnostic != PanelDiagnostic::None;
                prepared.presentationPrimaryRetained = false;
                prepared.auditDiagnosticRetained = false;
            }
            else
            {
                prepared.projectDiagnosticRetained = false;
            }
        }
        else if (input.diagnosticPresent)
        {
            prepared.snapshot.diagnostic           = input.diagnostic;
            prepared.snapshot.diagnosticGeneration = input.diagnosticGeneration;
            prepared.projectDiagnosticRetained     = false;
            prepared.auditDiagnosticRetained       = false;
        }
        if (input.presentationPresent)
        {
            if (!input.presentation.status.ok ())
            {
                if (prepared.snapshot.diagnostic == PanelDiagnostic::None)
                {
                    prepared.snapshot.diagnostic = PanelDiagnostic::SourceFault;
                    prepared.presentationOnlyDiagnostic = true;
                    prepared.projectDiagnosticRetained = false;
                    prepared.presentationPrimaryRetained = true;
                }
                else
                {
                    prepared.presentationPrimaryRetained = false;
                }
                prepared.presentationFailureRetained = true;
                prepared.failedIntentGeneration =
                    input.presentation.intentGeneration;
            }
            else if (prepared.presentationFailureRetained &&
                     input.presentation.intentGeneration ==
                         prepared.failedIntentGeneration + 1)
            {
                if (prepared.presentationPrimaryRetained &&
                    prepared.snapshot.diagnostic == PanelDiagnostic::None)
                {
                    prepared.snapshot.diagnostic =
                        PanelDiagnostic::PresentationRecovered;
                    prepared.projectDiagnosticRetained = false;
                    ++prepared.snapshot.diagnosticGeneration;
                }
                prepared.presentationFailureRetained = false;
                prepared.presentationPrimaryRetained = false;
                prepared.failedIntentGeneration       = 0;
            }
        }
        else
        {
            if (prepared.presentationPrimaryRetained &&
                prepared.snapshot.diagnostic == PanelDiagnostic::None)
            {
                prepared.snapshot.diagnostic = PanelDiagnostic::SourceFault;
                prepared.presentationOnlyDiagnostic = true;
            }
            else
            {
                prepared.presentationPrimaryRetained = false;
            }
        }
        const bool puzzleAuditEligible =
            puzzleAuditRequested && puzzleSolveEligible &&
            !prepared.snapshot.stopped &&
            prepared.snapshot.diagnostic == PanelDiagnostic::None &&
            prepared.snapshot.chordDisposition !=
                OperatorChordDisposition::InvalidChord &&
            prepared.snapshot.chordDisposition !=
                OperatorChordDisposition::InvalidEvidence &&
            prepared.snapshot.auditDisposition ==
                PanelAuditDisposition::AcknowledgeRequired;
        if (puzzleAuditEligible)
        {
            PanelAuditRecord committed = input.auditAcknowledge.record;
            committed.state            = PanelAuditSlotState::Committed;
            committed.checksum         = recordChecksum (committed);
            prepared.image             = input.auditImage;
            prepared.image.slots[input.auditAcknowledge.slotIndex] = committed;
            prepared.snapshot.auditDisposition = PanelAuditDisposition::Ready;
            prepared.consumeAudit              = true;
        }
        else if (puzzleAuditRequested)
        {
            prepared.image                     = image_;
            prepared.snapshot.auditDisposition = snapshot_.auditDisposition;
        }
        if (prepared.snapshot.auditDisposition ==
                PanelAuditDisposition::Indeterminate ||
            prepared.snapshot.auditDisposition == PanelAuditDisposition::Corrupt)
        {
            if (prepared.snapshot.diagnostic !=
                PanelDiagnostic::AuditIndeterminate)
            {
                prepared.snapshot.diagnostic =
                    PanelDiagnostic::AuditIndeterminate;
                prepared.auditDiagnosticRetained = true;
            }
            if (prepared.auditDiagnosticRetained)
            {
                prepared.projectDiagnosticRetained = false;
                prepared.presentationPrimaryRetained = false;
            }
        }
        else
        {
            prepared.auditDiagnosticRetained = false;
        }
        if (prepared.snapshot.stopped)
        {
            prepared.snapshot.diagnostic = PanelDiagnostic::Stopped;
            prepared.projectDiagnosticRetained = false;
            prepared.presentationPrimaryRetained = false;
            prepared.auditDiagnosticRetained = false;
        }
        ++prepared.snapshot.generation;
        if (prepared.snapshot.generation == 0)
        {
            return StatusCode::CapacityExceeded;
        }
        prepared.snapshot.presentation = makeIntent (prepared.snapshot);
        prepared.snapshot.status       = StatusCode::Ok;
        return StatusCode::Ok;
    }

    void FaultAwareOperatorPanel::applyPreparedUpdate (
        const PreparedUpdate& prepared) noexcept
    {
        snapshot_     = prepared.snapshot;
        image_        = prepared.image;
        retainedStop_ = prepared.stop;
        stopRetained_ = prepared.retainStop;
        retainedControl_ = prepared.control;
        controlRetained_ = prepared.retainControl;
        stopTransitionPending_ = prepared.stopTransitionPending;
        presentationFailureRetained_ =
            prepared.presentationFailureRetained;
        failedIntentGeneration_ = prepared.failedIntentGeneration;
        retainedNow_             = prepared.now;
        nowRetained_             = prepared.retainNow;
        projectDiagnosticRetained_ =
            prepared.projectDiagnosticRetained;
        presentationPrimaryRetained_ =
            prepared.presentationPrimaryRetained;
        auditDiagnosticRetained_ = prepared.auditDiagnosticRetained;
        if (prepared.invalidateCandidates)
        {
            auditCandidateLive_       = false;
            acknowledgeCandidateLive_ = false;
        }
        if (prepared.consumeAudit)
        {
            auditCandidateLive_ = false;
        }
        if (prepared.consumeAcknowledge)
        {
            acknowledgeCandidateLive_ = false;
        }
    }

    void FaultAwareOperatorPanel::invalidatePreparedCandidates () noexcept
    {
        auditCandidateLive_       = false;
        acknowledgeCandidateLive_ = false;
    }

    bool FaultAwareOperatorPanel::solvePreparationEligible () const noexcept
    {
        const bool auditReady =
            snapshot_.auditDisposition == PanelAuditDisposition::Ready ||
            snapshot_.auditDisposition == PanelAuditDisposition::PrepareRequired;
        return initialized_ && !lifecycleExhausted_ && stopRetained_ &&
               !retainedStop_.asserted && !stopTransitionPending_ &&
               !snapshot_.stopped && snapshot_.diagnostic == PanelDiagnostic::None &&
               auditReady;
    }

    Status
    FaultAwareOperatorPanel::update (const FaultAwareOperatorPanelInput& input) noexcept
    {
        PreparedUpdate prepared = PreparedUpdate  ();
        const Status   status   = preflightUpdate (input, prepared);

        if (status.ok ())
        {
            applyPreparedUpdate (prepared);
        }
        return status;
    }

    FaultAwareOperatorPanelSnapshot FaultAwareOperatorPanel::snapshot () const noexcept
    {
        return snapshot_;
    }

    PanelAuditImage FaultAwareOperatorPanel::canonicalAuditImage () const noexcept
    {
        return image_;
    }
} // namespace adk
