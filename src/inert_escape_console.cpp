#include "inert_escape_console.h"

namespace adk {
    namespace {
        constexpr uint32_t fnvOffset = UINT32_C (0x811c9dc5);
        constexpr uint32_t fnvPrime  = UINT32_C (0x01000193);

        void hashByte (uint32_t& hash, uint8_t value) noexcept
        {
            hash ^= value;
            hash *= fnvPrime;
        }

        void hash16 (uint32_t& hash, uint16_t value) noexcept
        {
            hashByte (hash, static_cast<uint8_t> (value));
            hashByte (hash, static_cast<uint8_t> (value >> 8));
        }

        void hash32 (uint32_t& hash, uint32_t value) noexcept
        {
            hashByte (hash, static_cast<uint8_t> (value));
            hashByte (hash, static_cast<uint8_t> (value >> 8));
            hashByte (hash, static_cast<uint8_t> (value >> 16));
            hashByte (hash, static_cast<uint8_t> (value >> 24));
        }

        void hashDomain (uint32_t& hash, const char* domain) noexcept
        {
            while (*domain != '\0')
            {
                hashByte (hash, static_cast<uint8_t> (*domain));
                ++domain;
            }
            hashByte (hash, 0);
        }

        void hashClueSource (uint32_t& hash, const ClueSourceIdentity& source) noexcept
        {
            hash16 (hash, source.sourceId);
            hash16 (hash, source.configurationRevision);
            hash32 (hash, source.sessionEpoch);
        }

        void hashOperatorSource (uint32_t&                     hash,
                                 const OperatorSourceIdentity& source) noexcept
        {
            hash16 (hash, source.sourceId);
            hash16 (hash, source.configurationRevision);
            hash32 (hash, source.sessionEpoch);
        }

        uint32_t policyDigest (const EscapeConsoleConfig& config) noexcept
        {
            uint32_t hash = fnvOffset;
            hashDomain (hash, "ADK.ESCAPE.POLICY.V1");
            hash16     (hash, config.configurationRevision);
            hash32     (hash, config.instanceEpoch);

            const ClueConstraintConfig& clue = config.clueModel;
            hash16   (hash, clue.configurationRevision);
            hash32   (hash, clue.instanceEpoch);
            hash32   (hash, clue.maximumEvidenceAge.microseconds ());
            hashByte (hash, clue.clueCount);
            hashByte (hash, clue.ruleCount);
            for (uint8_t index = 0; index < 12; ++index)
            {
                hashClueSource (hash, clue.expectedSources[index]);
            }
            for (uint8_t index = 0; index < 12; ++index)
            {
                const ClueRuleDefinition& rule = clue.rules[index];
                hashByte (hash, rule.ruleId);
                hashByte (hash, rule.termCount);
                for (uint8_t term = 0; term < 4; ++term)
                {
                    hashByte (hash, rule.terms[term].clueId);
                    hashByte (hash, static_cast<uint8_t> (rule.terms[term].relation));
                    hashByte (hash, static_cast<uint8_t> (rule.terms[term].category));
                }
                hashByte (hash, rule.prerequisiteCount);
                for (uint8_t prerequisite = 0; prerequisite < 4; ++prerequisite)
                {
                    hashByte (hash, rule.prerequisiteRuleIds[prerequisite]);
                }
            }
            for (uint8_t index = 0; index < 12; ++index)
            {
                hashByte (hash, static_cast<uint8_t> (config.clueFamilies[index]));
            }

            const FaultAwareOperatorPanelConfig& panel = config.panel;
            hash16             (hash, panel.configurationRevision);
            hash32             (hash, panel.instanceEpoch);
            hash32             (hash, panel.maximumInputAge.microseconds ());
            hashByte           (hash, panel.selectableCellCount);
            hashOperatorSource (hash, panel.controlSource);
            hashOperatorSource (hash, panel.stopSource);
            return hash;
        }

        bool validFamilies (const EscapeConsoleConfig& config) noexcept
        {
            uint8_t counts[6] = {};
            for (uint8_t index = 0; index < 12; ++index)
            {
                const uint8_t family =
                    static_cast<uint8_t> (config.clueFamilies[index]);
                if (family >= 6)
                {
                    return false;
                }
                ++counts[family];
            }
            for (uint8_t family = 0; family < 6; ++family)
            {
                if (counts[family] != 2)
                {
                    return false;
                }
            }
            return true;
        }

        bool emptyClueSource (const ClueSourceIdentity& source) noexcept
        {
            return source.sourceId == 0 && source.configurationRevision == 0 &&
                   source.sessionEpoch == 0;
        }

        bool emptyClueObservation (const ClueObservation& observation) noexcept
        {
            return observation.clueId == 0 &&
                   observation.category == ClueCategory::Absent &&
                   observation.quality == ClueQuality::Invalid &&
                   emptyClueSource                  (observation.source) &&
                   observation.sourceSequence == 0 &&
                   observation.observedAt.microseconds () == 0 &&
                   observation.status.ok               ();
        }

        bool emptyClueUpdate (const ClueConstraintUpdate& update) noexcept
        {
            if (update.now.microseconds () != 0 || update.observationMask != 0)
            {
                return false;
            }
            for (uint8_t index = 0; index < 12; ++index)
            {
                if (!emptyClueObservation (update.observations[index]))
                {
                    return false;
                }
            }
            return true;
        }

        bool emptyOperatorSource (const OperatorSourceIdentity& source) noexcept
        {
            return source.sourceId == 0 && source.configurationRevision == 0 &&
                   source.sessionEpoch == 0;
        }

        bool emptyAuditRecord (const PanelAuditRecord& record) noexcept
        {
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
                   !record.stopAsserted && emptyOperatorSource       (record.stopSource) &&
                   record.stopSourceSequence == 0 &&
                   record.stopObservedAt.microseconds () == 0 &&
                   record.occurredAt.microseconds     () == 0 &&
                   record.payloadDigest == 0 && record.checksum == 0 &&
                   record.state == PanelAuditSlotState::Empty;
        }

        bool emptyPanelPreview (const PanelAuditPreview& preview) noexcept
        {
            return preview.ownerToken == 0 && preview.lifecycleGeneration == 0 &&
                   preview.configurationRevision == 0 && preview.instanceEpoch == 0 &&
                   preview.panelGeneration == 0 && preview.operationId == 0 &&
                   preview.slotIndex == 0 && emptyAuditRecord (preview.record) &&
                   preview.imageDigest == 0;
        }

        bool emptyConsolePreview (const EscapeConsolePreview& preview) noexcept
        {
            return preview.ownerToken == 0 && preview.lifecycleGeneration == 0 &&
                   preview.configurationRevision == 0 && preview.instanceEpoch == 0 &&
                   preview.consoleGeneration == 0 && preview.operationId == 0 &&
                   preview.clueGeneration == 0 && preview.satisfiedRuleMask == 0 &&
                   preview.policyDigest == 0 && emptyPanelPreview (preview.audit);
        }

        PanelDiagnostic clueDiagnostic (ClueModelDisposition disposition) noexcept
        {
            switch (disposition)
            {
                case ClueModelDisposition::Incomplete:
                    return PanelDiagnostic::ClueIncomplete;
                case ClueModelDisposition::InvalidEvidence:
                    return PanelDiagnostic::ClueInvalid;
                case ClueModelDisposition::StaleEvidence:
                    return PanelDiagnostic::ClueStale;
                case ClueModelDisposition::ContradictoryEvidence:
                    return PanelDiagnostic::ClueContradictory;
                case ClueModelDisposition::InvalidConfiguration:
                    return PanelDiagnostic::ConfigurationFault;
                case ClueModelDisposition::InternalFault:
                    return PanelDiagnostic::InternalFault;
                case ClueModelDisposition::Uninitialized:
                    return PanelDiagnostic::ConfigurationFault;
                case ClueModelDisposition::Solved: return PanelDiagnostic::None;
            }
            return PanelDiagnostic::InternalFault;
        }

        EscapeConsoleDisposition
        evidenceDisposition (ClueModelDisposition disposition) noexcept
        {
            switch (disposition)
            {
                case ClueModelDisposition::InvalidEvidence:
                    return EscapeConsoleDisposition::InvalidEvidence;
                case ClueModelDisposition::StaleEvidence:
                    return EscapeConsoleDisposition::StaleEvidence;
                case ClueModelDisposition::ContradictoryEvidence:
                    return EscapeConsoleDisposition::ContradictoryEvidence;
                case ClueModelDisposition::InvalidConfiguration:
                    return EscapeConsoleDisposition::ConfigurationFault;
                case ClueModelDisposition::InternalFault:
                    return EscapeConsoleDisposition::InternalFault;
                case ClueModelDisposition::Uninitialized:
                    return EscapeConsoleDisposition::ConfigurationFault;
                case ClueModelDisposition::Incomplete:
                    return EscapeConsoleDisposition::AwaitingClues;
                case ClueModelDisposition::Solved:
                    return EscapeConsoleDisposition::AwaitingOperator;
            }
            return EscapeConsoleDisposition::InternalFault;
        }

        uint8_t qualityRank (ClueQuality quality) noexcept
        {
            switch (quality)
            {
                case ClueQuality::Qualified: return 0;
                case ClueQuality::Degraded: return 1;
                case ClueQuality::Stale: return 2;
                case ClueQuality::Contradictory: return 3;
                case ClueQuality::SourceFault: return 4;
                case ClueQuality::TimingFault: return 5;
                case ClueQuality::Invalid: return 6;
            }
            return 7;
        }

        EscapeConsoleDisposition
        primaryDisposition (const ClueConstraintSnapshot&          clue,
                            const FaultAwareOperatorPanelSnapshot& panel) noexcept
        {
            if (clue.disposition == ClueModelDisposition::InvalidConfiguration ||
                panel.diagnostic == PanelDiagnostic::ConfigurationFault)
            {
                return EscapeConsoleDisposition::ConfigurationFault;
            }
            if (clue.disposition == ClueModelDisposition::InternalFault ||
                panel.diagnostic == PanelDiagnostic::InternalFault)
            {
                return EscapeConsoleDisposition::InternalFault;
            }
            if (panel.stopped || panel.diagnostic == PanelDiagnostic::Stopped)
            {
                return EscapeConsoleDisposition::Stopped;
            }
            if (panel.diagnostic == PanelDiagnostic::SourceFault)
            {
                return EscapeConsoleDisposition::SourceFault;
            }
            if (panel.diagnostic == PanelDiagnostic::TimingFault)
            {
                return EscapeConsoleDisposition::TimingFault;
            }
            const EscapeConsoleDisposition evidence =
                evidenceDisposition (clue.disposition);
            if (evidence != EscapeConsoleDisposition::AwaitingClues &&
                evidence != EscapeConsoleDisposition::AwaitingOperator)
            {
                return evidence;
            }
            if (panel.auditDisposition == PanelAuditDisposition::Corrupt)
            {
                return EscapeConsoleDisposition::InternalFault;
            }
            if (panel.auditDisposition == PanelAuditDisposition::Indeterminate)
            {
                return EscapeConsoleDisposition::AuditIndeterminate;
            }
            if (panel.chordDisposition == OperatorChordDisposition::InvalidChord)
            {
                return EscapeConsoleDisposition::InvalidOperatorChord;
            }
            if (panel.auditDisposition == PanelAuditDisposition::PrepareRequired ||
                panel.auditDisposition == PanelAuditDisposition::AcknowledgeRequired)
            {
                return EscapeConsoleDisposition::AuditPending;
            }
            return evidence;
        }

        EscapeLampIntent lampFor (EscapeConsoleDisposition disposition) noexcept
        {
            switch (disposition)
            {
                case EscapeConsoleDisposition::Uninitialized:
                    return EscapeLampIntent::Off;
                case EscapeConsoleDisposition::AwaitingClues:
                    return EscapeLampIntent::Progress;
                case EscapeConsoleDisposition::AwaitingOperator:
                    return EscapeLampIntent::Confirmation;
                case EscapeConsoleDisposition::AuditPending:
                    return EscapeLampIntent::Ready;
                case EscapeConsoleDisposition::Solved: return EscapeLampIntent::Solved;
                case EscapeConsoleDisposition::Stopped:
                    return EscapeLampIntent::Stopped;
                default: return EscapeLampIntent::Fault;
            }
        }
    } // namespace

    InertEscapeConsole::InertEscapeConsole (const EscapeConsoleConfig& config) noexcept
        : clueModel_ (config.clueModel), panel_ (config.panel),
          configurationRevision_ (config.configurationRevision),
          instanceEpoch_         (config.instanceEpoch), policyDigest_ (0),
          lifecycleGeneration_   (0), operationId_ (0), packedFamilies_ ()
    {
        for (uint8_t index = 0; index < 12; ++index)
        {
            const uint8_t value =
                static_cast<uint8_t> (config.clueFamilies[index]);
            const uint8_t bit   = static_cast<uint8_t> (index * 3U);
            const uint8_t slot  = static_cast<uint8_t> (bit / 8U);
            const uint8_t shift = static_cast<uint8_t> (bit % 8U);
            packedFamilies_[slot] |= static_cast<uint8_t> (value << shift);
            if (shift > 5U)
            {
                packedFamilies_[slot + 1U] |=
                    static_cast<uint8_t> (value >> (8U - shift));
            }
        }
        if (config.configurationRevision != 0 && config.instanceEpoch != 0 &&
            config.clueModel.clueCount == 12 &&
            config.clueModel.ruleCount == 12 && validFamilies (config) &&
            config.policyDigest != 0 &&
            config.policyDigest == policyDigest (config))
        {
            policyDigest_ = config.policyDigest;
        }
        retainDisposition (EscapeConsoleDisposition::Uninitialized);
    }

    InertEscapeConsole::~InertEscapeConsole () noexcept
    {
        shutdown ();
    }

    bool InertEscapeConsole::advanceLifecycle () noexcept
    {
        if (lifecycleExhausted () || lifecycleGeneration_ == UINT32_MAX)
        {
            packedFamilies_[4] = static_cast<uint8_t> (
                (packedFamilies_[4] & UINT8_C (0x0f)) | UINT8_C (0xf0));
            return false;
        }
        ++lifecycleGeneration_;
        return true;
    }

    void InertEscapeConsole::clearSnapshot (EscapeConsoleDisposition disposition,
                                            Status                   status) noexcept
    {
        static_cast<void> (status);
        retainDisposition (disposition);
        if (status.error  () == StatusCode::CapacityExceeded)
        {
            packedFamilies_[4] = static_cast<uint8_t> (
                (packedFamilies_[4] & UINT8_C (0x0f)) | UINT8_C (0xf0));
        }
        operationId_ = 0;
    }

    EscapeConsoleDisposition
    InertEscapeConsole::retainedDisposition () const noexcept
    {
        if (lifecycleExhausted ())
        {
            return EscapeConsoleDisposition::InternalFault;
        }
        return static_cast<EscapeConsoleDisposition> (packedFamilies_[4] >> 4U);
    }

    void InertEscapeConsole::retainDisposition (
        EscapeConsoleDisposition disposition) noexcept
    {
        packedFamilies_[4] = static_cast<uint8_t> (
            (packedFamilies_[4] & UINT8_C (0x0f)) |
            (static_cast<uint8_t> (disposition) << 4U));
    }

    bool InertEscapeConsole::lifecycleExhausted () const noexcept
    {
        return (packedFamilies_[4] & UINT8_C (0xf0)) == UINT8_C (0xf0);
    }

    EscapeClueFamily InertEscapeConsole::familyFor (uint8_t clueId) const noexcept
    {
        const uint8_t bit   = static_cast<uint8_t> (clueId * 3U);
        const uint8_t slot  = static_cast<uint8_t> (bit / 8U);
        const uint8_t shift = static_cast<uint8_t> (bit % 8U);
        uint16_t value      = packedFamilies_[slot];
        if (slot < 4U)
        {
            value |= static_cast<uint16_t> (
                static_cast<uint16_t> (packedFamilies_[slot + 1U]) << 8U);
        }
        return static_cast<EscapeClueFamily> ((value >> shift) & UINT16_C (7));
    }

    Status InertEscapeConsole::initialize () noexcept
    {
        if (panel_.initialized () && !lifecycleExhausted ())
        {
            return StatusCode::Ok;
        }
        if (policyDigest_ == 0)
        {
            clearSnapshot (EscapeConsoleDisposition::ConfigurationFault,
                           StatusCode::InvalidConfiguration);
            return StatusCode::InvalidConfiguration;
        }
        if (!advanceLifecycle ())
        {
            clearSnapshot (EscapeConsoleDisposition::InternalFault,
                           StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }
        Status status = clueModel_.initialize ();
        if (!status.ok                        ())
        {
            clearSnapshot (EscapeConsoleDisposition::ConfigurationFault, status);
            return status;
        }
        status = panel_.initialize ();
        if (!status.ok             ())
        {
            clueModel_.shutdown ();
            clearSnapshot       (EscapeConsoleDisposition::ConfigurationFault, status);
            return status;
        }
        clearSnapshot (EscapeConsoleDisposition::AwaitingClues, StatusCode::Ok);
        return StatusCode::Ok;
    }

    void InertEscapeConsole::shutdown () noexcept
    {
        if (!panel_.initialized ())
        {
            return;
        }
        const bool advanced = advanceLifecycle ();
        panel_.shutdown                        ();
        clueModel_.shutdown                    ();
        clearSnapshot                          (advanced
                                                    ? EscapeConsoleDisposition::Uninitialized
                                                    : EscapeConsoleDisposition::InternalFault,
                                                advanced
                                                    ? Status (StatusCode::NotInitialized)
                                                    : Status (StatusCode::CapacityExceeded));
    }

    void InertEscapeConsole::reset () noexcept
    {
        const bool wasInitialized = panel_.initialized   ();
        const bool advanced       = advanceLifecycle     ();

        panel_.reset                           ();
        clueModel_.reset                       ();

        clearSnapshot                           (
            advanced ? (wasInitialized
                            ? EscapeConsoleDisposition::AwaitingClues
                            : EscapeConsoleDisposition::Uninitialized)
                     : EscapeConsoleDisposition::InternalFault,
            advanced ? Status (wasInitialized ? StatusCode::Ok
                                              : StatusCode::NotInitialized)
                     : Status (StatusCode::CapacityExceeded));
    }

    bool InertEscapeConsole::initialized () const noexcept
    {
        return panel_.initialized ();
    }

    Result<EscapeConsolePreview>
    InertEscapeConsole::prepareSolve (uint32_t             operationId,
                                      MicrosecondTimePoint now) noexcept
    {
        EscapeConsolePreview empty = EscapeConsolePreview ();
        if (!panel_.initialized                           ())
        {
            return {StatusCode::NotInitialized, empty};
        }
        if (lifecycleExhausted ())
        {
            return {StatusCode::CapacityExceeded, empty};
        }
        const ClueConstraintSnapshot          clue  = clueModel_.snapshot ();
        const FaultAwareOperatorPanelSnapshot panel = panel_.snapshot     ();
        if (operationId == 0 ||
            clue.disposition != ClueModelDisposition::Solved || panel.stopped ||
            panel.diagnostic != PanelDiagnostic::None ||
            !panel_.solvePreparationEligible () ||
            (panel.auditDisposition != PanelAuditDisposition::Ready &&
             panel.auditDisposition != PanelAuditDisposition::PrepareRequired))
        {
            return {StatusCode::InvalidArgument, empty};
        }
        Result<PanelAuditPreview> audit = panel_.preparePuzzleSolved (
            operationId, configurationRevision_, instanceEpoch_, panel.generation,
            clue.generation, clue.satisfiedRuleMask, policyDigest_, now);
        if (!audit.ok ())
        {
            return {audit.status (), empty};
        }
        EscapeConsolePreview preview  = EscapeConsolePreview ();
        preview.ownerToken            = reinterpret_cast<uintptr_t> (this);
        preview.lifecycleGeneration   = lifecycleGeneration_;
        preview.configurationRevision = configurationRevision_;
        preview.instanceEpoch         = instanceEpoch_;
        preview.consoleGeneration     = panel.generation;
        preview.operationId           = operationId;
        preview.clueGeneration        = clue.generation;
        preview.satisfiedRuleMask     = clue.satisfiedRuleMask;
        preview.policyDigest          = policyDigest_;
        preview.audit                 = audit.value ();
        return {StatusCode::Ok, preview};
    }

    Result<PanelAuditPreview>
    InertEscapeConsole::preparePanelAudit (uint32_t operationId, PanelAuditKind kind,
                                           MicrosecondTimePoint now) noexcept
    {
        panel_.invalidatePreparedCandidates ();
        if (!panel_.initialized             ())
        {
            return {StatusCode::NotInitialized, PanelAuditPreview ()};
        }
        if (lifecycleExhausted ())
        {
            return {StatusCode::CapacityExceeded, PanelAuditPreview ()};
        }
        return panel_.prepareAudit (operationId, kind, now);
    }

    Result<PanelAcknowledgePreview>
    InertEscapeConsole::preparePanelAcknowledge (uint32_t             operationId,
                                                 MicrosecondTimePoint now) noexcept
    {
        panel_.invalidatePreparedCandidates ();
        if (!panel_.initialized             ())
        {
            return {StatusCode::NotInitialized, PanelAcknowledgePreview ()};
        }
        if (lifecycleExhausted ())
        {
            return {StatusCode::CapacityExceeded, PanelAcknowledgePreview ()};
        }
        return panel_.prepareAcknowledge (operationId, now);
    }

    bool
    InertEscapeConsole::canCommit (const EscapeConsolePreview& preview) const noexcept
    {
        const ClueConstraintSnapshot clue  = clueModel_.snapshot ();
        return panel_.initialized                                () &&
               !lifecycleExhausted () &&
               preview.ownerToken == reinterpret_cast<uintptr_t> (this) &&
               preview.lifecycleGeneration == lifecycleGeneration_ &&
               preview.configurationRevision == configurationRevision_ &&
               preview.instanceEpoch == instanceEpoch_ &&
               preview.consoleGeneration == panel_.snapshot ().generation &&
               preview.operationId != 0 &&
               preview.clueGeneration == clue.generation &&
               preview.satisfiedRuleMask == clue.satisfiedRuleMask &&
               preview.policyDigest == policyDigest_ &&
               preview.audit.record.parentConfigurationRevision ==
                   configurationRevision_ &&
               preview.audit.record.parentInstanceEpoch == instanceEpoch_ &&
               preview.audit.record.parentGeneration == preview.consoleGeneration &&
               preview.audit.record.operationId == preview.operationId &&
               preview.audit.record.clueGeneration == preview.clueGeneration &&
               preview.audit.record.satisfiedRuleMask ==
                   preview.satisfiedRuleMask &&
               preview.audit.record.policyDigest == preview.policyDigest &&
               panel_.canAcknowledgeAudit (preview.audit);
    }

    void InertEscapeConsole::summarizeFamilies (
        EscapeConsoleSnapshot& snapshot) const noexcept
    {
        for (uint8_t family = 0; family < 6; ++family)
        {
            EscapeFamilySnapshot& summary = snapshot.families[family];
            summary.family = static_cast<EscapeClueFamily> (family);
            summary.firstClueId    = UINT8_MAX;
            summary.secondClueId   = UINT8_MAX;
            summary.complete              = true;
            summary.weakestQuality        = ClueQuality::Qualified;
            for (uint8_t clue = 0; clue < 12; ++clue)
            {
                if (familyFor (clue) != summary.family)
                {
                    continue;
                }
                if (summary.firstClueId == UINT8_MAX)
                {
                    summary.firstClueId = clue;
                }
                else
                {
                    summary.secondClueId = clue;
                }
            }
            const uint8_t ids[2]          = {summary.firstClueId, summary.secondClueId};
            for (uint8_t member = 0; member < 2; ++member)
            {
                const Result<ClueEvidenceSnapshot> evidence =
                    clueModel_.evidence (ids[member]);
                if (!evidence.ok () || !evidence.value ().present)
                {
                    summary.complete       = false;
                    summary.weakestQuality = ClueQuality::Invalid;
                    continue;
                }
                const ClueQuality quality = evidence.value ().quality;
                if (qualityRank                            (quality) >
                    qualityRank (summary.weakestQuality))
                {
                    summary.weakestQuality = quality;
                }
                if (quality != ClueQuality::Qualified &&
                    quality != ClueQuality::Degraded)
                {
                    summary.complete = false;
                }
            }
        }
    }

    Status InertEscapeConsole::update (const EscapeConsoleUpdate& input) noexcept
    {
        if (!panel_.initialized ())
        {
            return StatusCode::NotInitialized;
        }
        if (lifecycleExhausted ())
        {
            return StatusCode::CapacityExceeded;
        }
        if (panel_.snapshot ().generation == UINT32_MAX)
        {
            return StatusCode::CapacityExceeded;
        }
        if ((!input.clueUpdatePresent && !emptyClueUpdate (input.clueUpdate)) ||
            (input.clueUpdatePresent &&
             input.clueUpdate.now.microseconds () != input.now.microseconds ()) ||
            (!input.solvePreviewPresent && !emptyConsolePreview (input.solvePreview)) ||
            (input.solvePreviewPresent &&
             (!canCommit (input.solvePreview) || !input.auditAcknowledgePresent ||
              !panel_.canAcknowledgeAudit (input.auditAcknowledge) ||
              input.auditAcknowledge.ownerToken !=
                  input.solvePreview.audit.ownerToken ||
              input.auditAcknowledge.lifecycleGeneration !=
                  input.solvePreview.audit.lifecycleGeneration ||
              input.auditAcknowledge.operationId !=
                  input.solvePreview.audit.operationId ||
              input.auditAcknowledge.imageDigest !=
                  input.solvePreview.audit.imageDigest)))
        {
            return StatusCode::InvalidArgument;
        }

        ClueConstraintModel::PreparedUpdate cluePrepared =
            ClueConstraintModel::PreparedUpdate ();
        ClueConstraintSnapshot proposedClue = clueModel_.snapshot ();
        if (input.clueUpdatePresent)
        {
            const Status clueStatus =
                clueModel_.preflightUpdate (input.clueUpdate, cluePrepared);
            if (!clueStatus.ok ())
            {
                return clueStatus;
            }
            const Result<ClueConstraintSnapshot> prepared =
                clueModel_.preparedSnapshot (cluePrepared);
            if (!prepared.ok ())
            {
                return prepared.status ();
            }
            proposedClue = prepared.value ();
        }

        PanelDiagnostic diagnostic = clueDiagnostic (proposedClue.disposition);
        if (proposedClue.generation == 0 &&
            diagnostic == PanelDiagnostic::ClueIncomplete)
        {
            diagnostic = PanelDiagnostic::None;
        }
        if (proposedClue.disposition == ClueModelDisposition::InvalidEvidence)
        {
            bool sourceFault = false;
            bool timingFault = false;
            for (uint8_t clue = 0; clue < 12; ++clue)
            {
                const Result<ClueEvidenceSnapshot> evidence =
                    input.clueUpdatePresent
                        ? clueModel_.preparedEvidence (cluePrepared, clue)
                        : clueModel_.evidence         (clue);
                if (!evidence.ok () || !evidence.value ().present)
                {
                    continue;
                }
                sourceFault = sourceFault ||
                              evidence.value ().quality == ClueQuality::SourceFault;
                timingFault = timingFault ||
                              evidence.value ().quality == ClueQuality::TimingFault;
            }
            if (sourceFault)
            {
                diagnostic = PanelDiagnostic::SourceFault;
            }
            else if (timingFault)
            {
                diagnostic = PanelDiagnostic::TimingFault;
            }
        }
        FaultAwareOperatorPanel::PreparedUpdate panelPrepared =
            FaultAwareOperatorPanel::PreparedUpdate ();
        const Status panelStatus = panel_.preflightProjectUpdate (
            input.now, input.auditImagePresent, input.auditImage, input.stopPresent,
            input.stop, input.controlPresent, input.control,
            input.auditAcknowledgePresent, input.auditAcknowledge,
            input.acknowledgePresent, input.acknowledge, input.presentationPresent,
            input.presentation, diagnostic,
            diagnostic != PanelDiagnostic::None ? proposedClue.generation : 0,
            !input.clueUpdatePresent &&
                proposedClue.disposition == ClueModelDisposition::Solved,
            panelPrepared);
        if (!panelStatus.ok ())
        {
            return panelStatus;
        }
        const bool solveCommitted =
            input.solvePreviewPresent && panelPrepared.consumeAudit &&
            !panelPrepared.snapshot.stopped &&
            panelPrepared.snapshot.diagnostic == PanelDiagnostic::None &&
            proposedClue.disposition == ClueModelDisposition::Solved &&
            proposedClue.generation == input.solvePreview.clueGeneration &&
            proposedClue.satisfiedRuleMask == input.solvePreview.satisfiedRuleMask;

        if (input.clueUpdatePresent)
        {
            clueModel_.applyPreparedUpdate (cluePrepared);
        }
        panel_.applyPreparedUpdate (panelPrepared);

        const FaultAwareOperatorPanelSnapshot panel = panel_.snapshot ();
        EscapeConsoleDisposition disposition =
            primaryDisposition (clueModel_.snapshot (), panel);
        if (disposition != EscapeConsoleDisposition::Stopped &&
            disposition != EscapeConsoleDisposition::InternalFault &&
            disposition != EscapeConsoleDisposition::ConfigurationFault)
        {
            if (diagnostic == PanelDiagnostic::SourceFault)
            {
                disposition = EscapeConsoleDisposition::SourceFault;
            }
            else if (diagnostic == PanelDiagnostic::TimingFault)
            {
                disposition = EscapeConsoleDisposition::TimingFault;
            }
        }
        if (panelPrepared.presentationOnlyDiagnostic &&
            disposition == EscapeConsoleDisposition::SourceFault)
        {
            FaultAwareOperatorPanelSnapshot primaryPanel = panel;
            primaryPanel.diagnostic                      = PanelDiagnostic::None;
            disposition = primaryDisposition (clueModel_.snapshot (), primaryPanel);
        }
        operationId_ = 0;
        if (solveCommitted)
        {
            operationId_ = input.solvePreview.operationId;
            disposition = EscapeConsoleDisposition::Solved;
        }
        retainDisposition (disposition);
        return StatusCode::Ok;
    }

    EscapeConsoleSnapshot InertEscapeConsole::snapshot () const noexcept
    {
        const FaultAwareOperatorPanelSnapshot panel = panel_.snapshot ();
        EscapeConsoleSnapshot result = EscapeConsoleSnapshot          ();
        result.configurationRevision = configurationRevision_;
        result.instanceEpoch         = instanceEpoch_;
        result.generation            = panel.generation;
        result.operationId           = operationId_;
        result.disposition           = retainedDisposition ();
        result.latchIntent =
            operationId_ != 0 &&
                    result.disposition == EscapeConsoleDisposition::Solved
                ? EscapeLatchIntent::RequestDemonstrationRelease
                : EscapeLatchIntent::Inactive;
        result.lampIntent        = lampFor (result.disposition);
        result.presentation      = panel.presentation;
        result.auditDisposition  = panel.auditDisposition;
        if (lifecycleExhausted ())
        {
            result.status = StatusCode::CapacityExceeded;
        }
        else if (!panel_.initialized () &&
                 result.disposition ==
                     EscapeConsoleDisposition::ConfigurationFault)
        {
            result.status = StatusCode::InvalidConfiguration;
        }
        else if (!panel_.initialized ())
        {
            result.status = StatusCode::NotInitialized;
        }
        else
        {
            result.status = StatusCode::Ok;
        }
        summarizeFamilies (result);
        return result;
    }

    ClueConstraintSnapshot InertEscapeConsole::clueSnapshot () const noexcept
    {
        return clueModel_.snapshot ();
    }

    FaultAwareOperatorPanelSnapshot InertEscapeConsole::panelSnapshot () const noexcept
    {
        return panel_.snapshot ();
    }

    PanelAuditImage InertEscapeConsole::canonicalAuditImage () const noexcept
    {
        return panel_.canonicalAuditImage ();
    }
} // namespace adk
