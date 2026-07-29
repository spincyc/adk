#include <fault_aware_operator_panel.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

#ifndef ADK_FAULT_AWARE_PANEL_TEST_PART
#define ADK_FAULT_AWARE_PANEL_TEST_PART 0
#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART < 0 || ADK_FAULT_AWARE_PANEL_TEST_PART > 4
#error "ADK_FAULT_AWARE_PANEL_TEST_PART must be 0, 1, 2, 3, or 4"
#endif

namespace adk {
    struct InertEscapeConsole
    {
        static Result<PanelAuditPreview>
        preparePuzzleSolved (FaultAwareOperatorPanel& panel, uint32_t operationId,
                             uint16_t parentConfigurationRevision,
                             uint32_t parentInstanceEpoch, uint32_t parentGeneration,
                             uint32_t clueGeneration, uint16_t satisfiedRuleMask,
                             uint32_t policyDigest, MicrosecondTimePoint now) noexcept
        {
            return panel.preparePuzzleSolved (
                operationId, parentConfigurationRevision, parentInstanceEpoch,
                parentGeneration, clueGeneration, satisfiedRuleMask, policyDigest, now);
        }

        static void setLifecycleGeneration (FaultAwareOperatorPanel& panel,
                                            uint32_t generation) noexcept
        {
            panel.lifecycleGeneration_ = generation;
        }

        static uint32_t
        lifecycleGeneration (const FaultAwareOperatorPanel& panel) noexcept
        {
            return panel.lifecycleGeneration_;
        }

        static Status projectUpdate (FaultAwareOperatorPanel&            panel,
                                     const FaultAwareOperatorPanelInput& input,
                                     PanelDiagnostic derivedDiagnostic,
                                     uint32_t        derivedGeneration,
                                     bool            puzzleSolveEligible) noexcept
        {
            FaultAwareOperatorPanel::PreparedUpdate prepared;
            const Status status = panel.preflightProjectUpdate (
                input.now, input.auditImagePresent, input.auditImage, input.stopPresent,
                input.stop, input.controlPresent, input.control,
                input.auditAcknowledgePresent, input.auditAcknowledge,
                input.acknowledgePresent, input.acknowledge, input.presentationPresent,
                input.presentation, derivedDiagnostic, derivedGeneration,
                puzzleSolveEligible, prepared);
            if (status.ok ())
            {
                panel.applyPreparedUpdate (prepared);
            }
            return status;
        }

        static void invalidateCandidates (FaultAwareOperatorPanel& panel) noexcept
        {
            panel.invalidatePreparedCandidates ();
        }
    };
} // namespace adk

namespace {

    adk::OperatorSourceIdentity controlSource () noexcept
    {
        return {11, 7, 101};
    }

    adk::OperatorSourceIdentity stopSource () noexcept
    {
        return {12, 7, 102};
    }

    adk::FaultAwareOperatorPanelConfig config () noexcept
    {
        return {
            7, 41, adk::MicrosecondDuration (100), 12, controlSource (), stopSource ()};
    }

    adk::PanelAuditRecord zeroRecord () noexcept
    {
        return {0,
                0,
                0,
                0,
                0,
                0,
                adk::PanelAuditKind::None,
                adk::PanelDiagnostic::None,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                false,
                false,
                {0, 0, 0},
                0,
                adk::MicrosecondTimePoint (),

                adk::MicrosecondTimePoint (),
                0,
                0,
                adk::PanelAuditSlotState::Empty};
    }

    adk::PanelAuditImage zeroImage () noexcept
    {
        return {{zeroRecord (), zeroRecord ()}};
    }

    adk::PanelAuditPreview zeroAuditPreview () noexcept
    {
        return {0, 0, 0, 0, 0, 0, 0, zeroRecord (), 0};
    }

    adk::PanelAcknowledgePreview zeroAcknowledgePreview () noexcept
    {
        return {0, 0, 0, 0, 0, 0, adk::PanelDiagnostic::None, 0, zeroAuditPreview ()};
    }

    adk::FaultAwareOperatorPanelInput emptyInput (uint32_t now) noexcept
    {
        return {adk::MicrosecondTimePoint (now),
                false,
                zeroImage (),
                false,
                {false, {0, 0, 0}, 0, adk::MicrosecondTimePoint (), adk::Status ()},
                false,
                {0, {0, 0, 0}, 0, adk::MicrosecondTimePoint (), adk::Status ()},
                false,
                adk::PanelDiagnostic::None,
                0,
                false,
                zeroAuditPreview (),
                false,
                zeroAcknowledgePreview (),
                false,
                {0, adk::MicrosecondTimePoint (), adk::Status ()}};
    }

    adk::FaultAwareOperatorPanelInput
    imageInput (uint32_t now, const adk::PanelAuditImage& image) noexcept
    {
        adk::FaultAwareOperatorPanelInput result = emptyInput (now);
        result.auditImagePresent                 = true;
        result.auditImage                        = image;
        return result;
    }

    adk::OperatorStopEvidence stopEvidence (bool asserted, uint32_t sequence,
                                            uint32_t observedAt) noexcept
    {
        return {asserted, stopSource (), sequence,
                adk::MicrosecondTimePoint (observedAt), adk::Status ()};
    }

    adk::OperatorControlEvidence controlEvidence (uint8_t mask, uint32_t sequence,
                                                  uint32_t observedAt) noexcept
    {
        return {mask, controlSource (), sequence,
                adk::MicrosecondTimePoint (observedAt), adk::Status ()};
    }

    bool zeroRecordValue (const adk::PanelAuditRecord& value) noexcept
    {
        const adk::PanelAuditRecord zero = zeroRecord ();

        return std::memcmp (&value, &zero, sizeof value) == 0;
    }

    bool zeroAuditValue (const adk::PanelAuditPreview& value) noexcept
    {
        return value.ownerToken == 0 && value.lifecycleGeneration == 0 &&
               value.configurationRevision == 0 && value.instanceEpoch == 0 &&
               value.panelGeneration == 0 && value.operationId == 0 &&
               value.slotIndex == 0 && zeroRecordValue (value.record) &&
               value.imageDigest == 0;
    }

    bool zeroAcknowledgeValue (const adk::PanelAcknowledgePreview& value) noexcept
    {
        return value.ownerToken == 0 && value.lifecycleGeneration == 0 &&
               value.configurationRevision == 0 && value.instanceEpoch == 0 &&
               value.panelGeneration == 0 && value.operationId == 0 &&
               value.diagnostic == adk::PanelDiagnostic::None &&
               value.diagnosticGeneration == 0 && zeroAuditValue (value.audit);
    }

    bool sameRecord (const adk::PanelAuditRecord& left,
                     const adk::PanelAuditRecord& right) noexcept
    {
        return left.formatMagic == right.formatMagic &&
               left.formatVersion == right.formatVersion &&
               left.configurationRevision == right.configurationRevision &&
               left.instanceEpoch == right.instanceEpoch &&
               left.recordSequence == right.recordSequence &&
               left.operationId == right.operationId && left.kind == right.kind &&
               left.diagnostic == right.diagnostic &&
               left.diagnosticGeneration == right.diagnosticGeneration &&
               left.parentConfigurationRevision == right.parentConfigurationRevision &&
               left.parentInstanceEpoch == right.parentInstanceEpoch &&
               left.parentGeneration == right.parentGeneration &&
               left.clueGeneration == right.clueGeneration &&
               left.satisfiedRuleMask == right.satisfiedRuleMask &&
               left.policyDigest == right.policyDigest &&
               left.stopPresent == right.stopPresent &&
               left.stopAsserted == right.stopAsserted &&
               left.stopSource.sourceId == right.stopSource.sourceId &&
               left.stopSource.configurationRevision ==
                   right.stopSource.configurationRevision &&
               left.stopSource.sessionEpoch == right.stopSource.sessionEpoch &&
               left.stopSourceSequence == right.stopSourceSequence &&
               left.stopObservedAt.microseconds () ==
                   right.stopObservedAt.microseconds () &&
               left.occurredAt.microseconds () == right.occurredAt.microseconds () &&
               left.payloadDigest == right.payloadDigest &&
               left.checksum == right.checksum && left.state == right.state;
    }

    bool sameImage (const adk::PanelAuditImage& left,
                    const adk::PanelAuditImage& right) noexcept
    {
        return sameRecord (left.slots[0], right.slots[0]) &&
               sameRecord (left.slots[1], right.slots[1]);
    }

    bool sameSnapshot (const adk::FaultAwareOperatorPanelSnapshot& left,
                       const adk::FaultAwareOperatorPanelSnapshot& right) noexcept
    {
        return left.configurationRevision == right.configurationRevision &&
               left.instanceEpoch == right.instanceEpoch &&
               left.generation == right.generation && left.stopped == right.stopped &&
               left.selectedCell == right.selectedCell &&
               left.chordDisposition == right.chordDisposition &&
               left.diagnostic == right.diagnostic &&
               left.diagnosticGeneration == right.diagnosticGeneration &&
               left.auditDisposition == right.auditDisposition &&
               left.presentation.mode == right.presentation.mode &&
               left.presentation.diagnostic == right.presentation.diagnostic &&
               left.presentation.selectedCell == right.presentation.selectedCell &&
               left.presentation.diagnosticGeneration ==
                   right.presentation.diagnosticGeneration &&
               left.presentation.acknowledgeAvailable ==
                   right.presentation.acknowledgeAvailable &&
               left.status == right.status;
    }

    class Fnv32
    {
      public:
        Fnv32 () noexcept : value_ (UINT32_C (0x811c9dc5))
        {
        }

        void byte (uint8_t value) noexcept
        {
            value_ ^= value;
            value_ *= UINT32_C (0x01000193);
        }

        void bytes (const char* values, uint8_t count) noexcept
        {
            for (uint8_t index = 0; index < count; ++index)
            {
                byte (static_cast<uint8_t> (values[index]));
            }
        }

        void u16 (uint16_t value) noexcept
        {
            byte (static_cast<uint8_t> (value));

            byte (static_cast<uint8_t> (value >> 8U));
        }

        void u32 (uint32_t value) noexcept
        {
            u16 (static_cast<uint16_t> (value));

            u16 (static_cast<uint16_t> (value >> 16U));
        }

        uint32_t value () const noexcept
        {
            return value_;
        }

      private:
        uint32_t value_;
    };

    void hashIdentity (Fnv32& hash, const adk::OperatorSourceIdentity& source) noexcept
    {
        hash.u16 (source.sourceId);

        hash.u16 (source.configurationRevision);

        hash.u32 (source.sessionEpoch);
    }

    void hashPayloadFields (Fnv32& hash, const adk::PanelAuditRecord& record) noexcept
    {
        hash.u32 (record.operationId);

        hash.byte (static_cast<uint8_t> (record.kind));

        hash.byte (static_cast<uint8_t> (record.diagnostic));

        hash.u32 (record.diagnosticGeneration);

        hash.u16 (record.parentConfigurationRevision);

        hash.u32 (record.parentInstanceEpoch);

        hash.u32 (record.parentGeneration);

        hash.u32 (record.clueGeneration);

        hash.u16 (record.satisfiedRuleMask);

        hash.u32 (record.policyDigest);

        hash.byte (record.stopPresent ? 1 : 0);

        hash.byte (record.stopAsserted ? 1 : 0);

        hashIdentity (hash, record.stopSource);

        hash.u32 (record.stopSourceSequence);

        hash.u32 (record.stopObservedAt.microseconds ());

        hash.u32 (record.occurredAt.microseconds ());
    }

    uint32_t payloadDigest (const adk::PanelAuditRecord& record) noexcept
    {
        Fnv32             hash;
        static const char domain[] = "ADK.PANEL.PAYLOAD.V1";
        hash.bytes (domain, sizeof domain);

        hashPayloadFields (hash, record);

        return hash.value ();
    }

    void hashRecordFields (Fnv32& hash, const adk::PanelAuditRecord& record,
                           bool includeChecksum) noexcept
    {
        hash.u32 (record.formatMagic);

        hash.u16 (record.formatVersion);

        hash.u16 (record.configurationRevision);

        hash.u32 (record.instanceEpoch);

        hash.u32 (record.recordSequence);

        hashPayloadFields (hash, record);

        hash.u32 (record.payloadDigest);

        if (includeChecksum)
        {
            hash.u32 (record.checksum);
        }
        hash.byte (static_cast<uint8_t> (record.state));
    }

    uint32_t recordChecksum (const adk::PanelAuditRecord& record) noexcept
    {
        Fnv32             hash;
        static const char domain[] = "ADK.PANEL.RECORD.V1";
        hash.bytes (domain, sizeof domain);

        hashRecordFields (hash, record, false);

        return hash.value ();
    }

    uint32_t imageDigest (const adk::PanelAuditImage& image) noexcept
    {
        Fnv32             hash;
        static const char domain[] = "ADK.PANEL.IMAGE.V1";
        hash.bytes (domain, sizeof domain);

        for (uint8_t index = 0; index < 2; ++index)
        {
            hash.byte (index);

            hashRecordFields (hash, image.slots[index], true);
        }
        return hash.value ();
    }

    void sealRecord (adk::PanelAuditRecord& record) noexcept
    {
        record.payloadDigest = payloadDigest (record);

        record.checksum = recordChecksum (record);
    }

    adk::Result<adk::PanelAuditPreview>
    prepareSolved (adk::FaultAwareOperatorPanel& panel, uint32_t operationId,
                   uint32_t now) noexcept
    {
        return adk::InertEscapeConsole::preparePuzzleSolved (
            panel, operationId, 19, 23, 29, 31, UINT16_C (0x0a55),

            UINT32_C (0x12345678), adk::MicrosecondTimePoint (now));
    }

    void commitAudit (adk::FaultAwareOperatorPanel& panel, adk::PanelAuditImage& image,
                      const adk::PanelAuditPreview& preview, uint32_t now) noexcept
    {
        image.slots[preview.slotIndex]          = preview.record;
        adk::FaultAwareOperatorPanelInput input = imageInput (now, image);
        input.auditAcknowledgePresent           = true;
        input.auditAcknowledge                  = preview;
        assert (panel.update (input).ok ());

        image = panel.canonicalAuditImage ();
    }

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 1
    void testConfigurationAndLifecycle ()
    {
        adk::FaultAwareOperatorPanelConfig invalid = config ();
        invalid.selectableCellCount                = 0;
        adk::FaultAwareOperatorPanel bad (invalid);

        assert (bad.initialize ().error () == adk::StatusCode::InvalidConfiguration);

        assert (!bad.initialized ());

        adk::FaultAwareOperatorPanel panel (config ());

        assert (!panel.initialized ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Empty);
        assert (panel.initialize ().ok ());

        const adk::FaultAwareOperatorPanelSnapshot initialized = panel.snapshot ();

        assert (initialized.presentation.mode == adk::PanelPresentationMode::Blank);

        assert (panel.initialize ().ok ());

        assert (sameSnapshot (initialized, panel.snapshot ()));

        panel.reset ();

        assert (panel.initialized ());

        assert (panel.snapshot ().presentation.mode ==
                adk::PanelPresentationMode::Blank);
        panel.shutdown ();

        assert (!panel.initialized ());

        const adk::FaultAwareOperatorPanelSnapshot stopped = panel.snapshot ();

        panel.shutdown ();

        assert (sameSnapshot (stopped, panel.snapshot ()));

        assert (panel.initialize ().ok ());

        assert (panel.initialized ());
    }

    void testDigestBootstrapAndImageForms ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (10, image)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::PrepareRequired);

        const adk::Result<adk::PanelAuditPreview> prepared =
            prepareSolved (panel, 501, 20);
        assert (prepared.ok ());

        const adk::PanelAuditPreview preview = prepared.value ();

        assert (preview.ownerToken != 0);

        assert (preview.lifecycleGeneration != 0);

        assert (preview.record.formatMagic == UINT32_C (0x41444b41));

        assert (preview.record.formatVersion == UINT16_C (1));

        assert (preview.record.recordSequence == 1);

        assert (preview.record.kind == adk::PanelAuditKind::PuzzleSolved);

        assert (preview.record.payloadDigest == payloadDigest (preview.record));

        assert (preview.record.payloadDigest == UINT32_C (0x3e83e665));

        assert (preview.record.checksum == recordChecksum (preview.record));

        assert (preview.record.checksum == UINT32_C (0xfa3e8a9a));

        adk::PanelAuditImage candidate     = image;
        candidate.slots[preview.slotIndex] = preview.record;
        assert (preview.imageDigest == imageDigest (candidate));

        assert (preview.imageDigest == UINT32_C (0xc9209c5b));

        commitAudit (panel, image, preview, 21);

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Ready);
        assert (image.slots[preview.slotIndex].state ==
                adk::PanelAuditSlotState::Committed);

        adk::PanelAuditImage corrupt = image;
        corrupt.slots[preview.slotIndex].formatMagic ^= UINT32_C (1);

        assert (panel.update (imageInput (22, corrupt)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Corrupt);

        corrupt = image;
        corrupt.slots[preview.slotIndex].payloadDigest ^= UINT32_C (1);

        assert (panel.update (imageInput (23, corrupt)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Corrupt);
    }

    void testKindAuthorityAndFailedResults ()
    {
        adk::FaultAwareOperatorPanel panel (config ());
        const adk::PanelAuditKind    kinds[] = {
            adk::PanelAuditKind::None, adk::PanelAuditKind::AcknowledgedDiagnostic,
            adk::PanelAuditKind::PuzzleSolved, adk::PanelAuditKind::StopAsserted,
            adk::PanelAuditKind::StopReleased};
        for (const adk::PanelAuditKind kind : kinds)
        {
            const adk::Result<adk::PanelAuditPreview> result =
                panel.prepareAudit (1, kind, adk::MicrosecondTimePoint (1));
            assert (!result.ok ());

            assert (zeroAuditValue (result.value ()));
        }
        assert (panel.initialize ().ok ());

        for (const adk::PanelAuditKind kind : kinds)
        {
            const adk::Result<adk::PanelAuditPreview> result =
                panel.prepareAudit (2, kind, adk::MicrosecondTimePoint (2));
            assert (!result.ok ());

            assert (zeroAuditValue (result.value ()));
        }
        const adk::Result<adk::PanelAcknowledgePreview> acknowledge =
            panel.prepareAcknowledge (3, adk::MicrosecondTimePoint (3));
        assert (!acknowledge.ok ());

        assert (zeroAcknowledgeValue (acknowledge.value ()));
    }

    void testOwnerLifecycleAndAtomicRejection ()
    {
        adk::FaultAwareOperatorPanel first (config ());

        adk::FaultAwareOperatorPanel second (config ());

        assert (first.initialize ().ok ());

        assert (second.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (first.update (imageInput (10, image)).ok ());

        assert (second.update (imageInput (10, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> prepared =
            prepareSolved (first, 600, 20);
        assert (prepared.ok ());

        assert (!second.canAcknowledgeAudit (prepared.value ()));

        assert (first.canAcknowledgeAudit (prepared.value ()));
        adk::PanelAuditImage foreignImage               = image;
        foreignImage.slots[prepared.value ().slotIndex] = prepared.value ().record;

        adk::FaultAwareOperatorPanelInput foreignCommit = imageInput (20, foreignImage);
        foreignCommit.auditAcknowledgePresent           = true;
        foreignCommit.auditAcknowledge                  = prepared.value ();

        const adk::FaultAwareOperatorPanelSnapshot secondBefore = second.snapshot ();

        assert (second.update (foreignCommit).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (secondBefore, second.snapshot ()));

#define ASSERT_MUTATED_PREVIEW_REJECTS(expression)                                     \
    do                                                                                 \
    {                                                                                  \
        adk::PanelAuditPreview changed = prepared.value ();                            \
        expression;                                                                    \
        assert (!first.canAcknowledgeAudit (changed));                                 \
    }                                                                                  \
    while (false)
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.ownerToken ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.lifecycleGeneration ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.configurationRevision ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.instanceEpoch ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.panelGeneration ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.operationId ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.slotIndex ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.imageDigest ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.formatMagic ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.formatVersion ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.configurationRevision ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.instanceEpoch ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.recordSequence ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.operationId ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.kind =
                                            adk::PanelAuditKind::None);
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.diagnostic =
                                            adk::PanelDiagnostic::ClueInvalid);
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.diagnosticGeneration ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.parentConfigurationRevision ^=
                                        1);
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.parentInstanceEpoch ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.parentGeneration ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.clueGeneration ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.satisfiedRuleMask ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.policyDigest ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.stopPresent = true);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.stopAsserted = true);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.stopSource.sourceId ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (
            changed.record.stopSource.configurationRevision ^= 1);
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.stopSource.sessionEpoch ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.stopSourceSequence ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.stopObservedAt =
                                            adk::MicrosecondTimePoint (1));
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.occurredAt =
                                            adk::MicrosecondTimePoint (1));
        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.payloadDigest ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.checksum ^= 1);

        ASSERT_MUTATED_PREVIEW_REJECTS (changed.record.state =
                                            adk::PanelAuditSlotState::Committed);
#undef ASSERT_MUTATED_PREVIEW_REJECTS

        adk::FaultAwareOperatorPanelInput conflicting = imageInput (21, image);
        conflicting.auditAcknowledgePresent           = true;
        conflicting.auditAcknowledge                  = prepared.value ();
        conflicting.acknowledgePresent                = true;
        conflicting.acknowledge                       = zeroAcknowledgePreview ();

        const adk::FaultAwareOperatorPanelSnapshot before = first.snapshot ();

        const adk::PanelAuditImage beforeImage = first.canonicalAuditImage ();

        assert (first.update (conflicting).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, first.snapshot ()));

        assert (sameImage (beforeImage, first.canonicalAuditImage ()));

        assert (first.canAcknowledgeAudit (prepared.value ()));

        adk::PanelAuditPreview mutated = prepared.value ();

        mutated.operationId ^= UINT32_C (1);

        assert (!first.canAcknowledgeAudit (mutated));

        first.reset ();

        assert (!first.canAcknowledgeAudit (prepared.value ()));

        assert (first.initialize ().ok ());
    }

    void testDiagnosticAcknowledgement ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        adk::FaultAwareOperatorPanelInput failed = imageInput (10, image);
        failed.controlPresent                    = true;
        failed.control                           = controlEvidence (UINT8_C (2), 1, 10);
        failed.control.status                    = adk::StatusCode::HardwareFailure;
        assert (panel.update (failed).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::SourceFault);

        adk::FaultAwareOperatorPanelInput recovered = imageInput (20, image);
        recovered.controlPresent                    = true;
        recovered.control = controlEvidence (UINT8_C (2), 2, 20);

        assert (panel.update (recovered).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::InputRecovered);

        const uint32_t diagnosticGeneration = panel.snapshot ().diagnosticGeneration;

        assert (diagnosticGeneration != 0);

        const adk::Result<adk::PanelAcknowledgePreview> prepared =
            panel.prepareAcknowledge (700, adk::MicrosecondTimePoint (21));
        assert (prepared.ok ());

        assert (prepared.value ().audit.record.kind ==
                adk::PanelAuditKind::AcknowledgedDiagnostic);
        assert (prepared.value ().audit.record.diagnostic ==
                adk::PanelDiagnostic::InputRecovered);
        assert (prepared.value ().audit.record.diagnosticGeneration ==
                diagnosticGeneration);
        assert (prepared.value ().audit.record.payloadDigest ==
                payloadDigest (prepared.value ().audit.record));
        assert (prepared.value ().audit.record.checksum ==
                recordChecksum (prepared.value ().audit.record));

        image.slots[prepared.value ().audit.slotIndex] = prepared.value ().audit.record;

        adk::FaultAwareOperatorPanelInput acknowledge = imageInput (22, image);
        acknowledge.acknowledgePresent                = true;
        acknowledge.acknowledge                       = prepared.value ();

        assert (panel.update (acknowledge).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::None);

        image = panel.canonicalAuditImage ();

        assert (image.slots[prepared.value ().audit.slotIndex].state ==
                adk::PanelAuditSlotState::Committed);

        const adk::FaultAwareOperatorPanelSnapshot before = panel.snapshot ();

        assert (panel.update (acknowledge).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, panel.snapshot ()));
    }

#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 2
    void testStopAuditAndQualifiedRelease ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        adk::FaultAwareOperatorPanelInput asserted = imageInput (10, image);
        asserted.stopPresent                       = true;
        asserted.stop                              = stopEvidence (true, 10, 10);

        assert (panel.update (asserted).ok ());

        assert (panel.snapshot ().stopped);

        const adk::Result<adk::PanelAuditPreview> assertion = panel.prepareAudit (
            800, adk::PanelAuditKind::StopAsserted, adk::MicrosecondTimePoint (11));
        assert (assertion.ok ());

        commitAudit (panel, image, assertion.value (), 12);

        panel.shutdown ();

        assert (panel.initialize ().ok ());

        assert (panel.update (imageInput (12, image)).ok ());

        assert (panel.snapshot ().stopped);

        adk::FaultAwareOperatorPanelInput staleRelease = imageInput (13, image);
        staleRelease.stopPresent                       = true;
        staleRelease.stop                              = stopEvidence (false, 10, 13);

        assert (panel.update (staleRelease).error () ==
                adk::StatusCode::InvalidArgument);
        assert (panel.snapshot ().stopped);

        adk::FaultAwareOperatorPanelInput release = imageInput (20, image);
        release.stopPresent                       = true;
        release.stop                              = stopEvidence (false, 11, 20);

        assert (panel.update (release).ok ());

        assert (panel.snapshot ().stopped);

        const adk::Result<adk::PanelAuditPreview> releaseAudit = panel.prepareAudit (
            801, adk::PanelAuditKind::StopReleased, adk::MicrosecondTimePoint (21));
        assert (releaseAudit.ok ());

        image.slots[releaseAudit.value ().slotIndex] = releaseAudit.value ().record;

        adk::FaultAwareOperatorPanelInput releaseCommit = imageInput (22, image);
        releaseCommit.stopPresent                       = true;
        releaseCommit.stop                              = stopEvidence (false, 11, 20);
        releaseCommit.auditAcknowledgePresent           = true;
        releaseCommit.auditAcknowledge                  = releaseAudit.value ();

        assert (panel.update (releaseCommit).ok ());

        assert (!panel.snapshot ().stopped);

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::None);

        image = panel.canonicalAuditImage ();

        assert (image.slots[releaseAudit.value ().slotIndex].state ==
                adk::PanelAuditSlotState::Committed);
        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Ready);

        adk::PanelAuditImage duplicate    = image;
        duplicate.slots[1].recordSequence = duplicate.slots[0].recordSequence;
        sealRecord (duplicate.slots[1]);

        assert (panel.update (imageInput (23, duplicate)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);

        adk::PanelAuditImage gap    = image;
        gap.slots[1].recordSequence = gap.slots[0].recordSequence + UINT32_C (2);

        sealRecord (gap.slots[1]);

        assert (panel.update (imageInput (24, gap)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);

        adk::PanelAuditImage twoPrepared = image;
        twoPrepared.slots[0].state       = adk::PanelAuditSlotState::Prepared;
        twoPrepared.slots[1].state       = adk::PanelAuditSlotState::Prepared;
        sealRecord (twoPrepared.slots[0]);

        sealRecord (twoPrepared.slots[1]);

        assert (panel.update (imageInput (25, twoPrepared)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);

        adk::PanelAuditImage invalidKind = image;
        invalidKind.slots[0].kind        = adk::PanelAuditKind::None;
        sealRecord (invalidKind.slots[0]);

        assert (panel.update (imageInput (26, invalidKind)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Corrupt);
    }

    void testControlStopAndReplay ()
    {
        adk::FaultAwareOperatorPanel first (config ());

        adk::FaultAwareOperatorPanel second (config ());

        assert (first.initialize ().ok ());

        assert (second.initialize ().ok ());

        adk::PanelAuditImage firstImage  = zeroImage ();
        adk::PanelAuditImage secondImage = firstImage;
        assert (first.update (imageInput (1, firstImage)).ok ());

        assert (second.update (imageInput (1, secondImage)).ok ());

        adk::FaultAwareOperatorPanelInput next = imageInput (10, firstImage);
        next.controlPresent                    = true;
        next.control                           = controlEvidence (UINT8_C (2), 1, 10);

        assert (first.update (next).ok ());
        adk::FaultAwareOperatorPanelInput nextCopy = next;
        nextCopy.auditImage                        = secondImage;
        assert (second.update (nextCopy).ok ());

        assert (sameSnapshot (first.snapshot (), second.snapshot ()));

        assert (first.snapshot ().selectedCell == 1);

        adk::FaultAwareOperatorPanelInput invalidChord = imageInput (20, firstImage);
        invalidChord.controlPresent                    = true;
        invalidChord.control = controlEvidence (UINT8_C (3), 2, 20);

        assert (first.update (invalidChord).ok ());

        assert (first.snapshot ().chordDisposition ==
                adk::OperatorChordDisposition::InvalidChord);

        adk::FaultAwareOperatorPanelInput stop = imageInput (30, firstImage);
        stop.stopPresent                       = true;
        stop.stop                              = stopEvidence (true, 3, 30);
        stop.controlPresent                    = true;
        stop.control                           = controlEvidence (UINT8_C (2), 3, 30);

        assert (first.update (stop).ok ());

        assert (first.snapshot ().stopped);

        assert (first.snapshot ().presentation.mode ==
                adk::PanelPresentationMode::Stopped);
        assert (first.snapshot ().selectedCell == 1);

        const adk::FaultAwareOperatorPanelSnapshot before = first.snapshot ();

        const adk::PanelAuditImage beforeImage = first.canonicalAuditImage ();

        adk::FaultAwareOperatorPanelInput malformed = emptyInput (31);
        malformed.stop.asserted                     = true;
        assert (first.update (malformed).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (before, first.snapshot ()));

        assert (sameImage (beforeImage, first.canonicalAuditImage ()));
    }

    void testEveryControlMask ()
    {
        for (uint16_t mask = 0; mask <= UINT8_MAX; ++mask)
        {
            adk::FaultAwareOperatorPanel panel (config ());

            assert (panel.initialize ().ok ());

            adk::PanelAuditImage image = zeroImage ();

            assert (panel.update (imageInput (1, image)).ok ());

            adk::FaultAwareOperatorPanelInput input = imageInput (10, image);
            input.controlPresent                    = true;
            input.control = controlEvidence (static_cast<uint8_t> (mask), 1, 10);

            if ((mask & UINT16_C (0xf0)) != 0)
            {
                const adk::FaultAwareOperatorPanelSnapshot before = panel.snapshot ();

                assert (panel.update (input).error () ==
                        adk::StatusCode::InvalidArgument);
                assert (sameSnapshot (before, panel.snapshot ()));
                continue;
            }
            assert (panel.update (input).ok ());
            const adk::OperatorChordDisposition expected =
                mask == 0 ? adk::OperatorChordDisposition::None
                : (mask & (mask - 1)) == 0
                    ? adk::OperatorChordDisposition::SingleControl
                    : adk::OperatorChordDisposition::InvalidChord;
            assert (panel.snapshot ().chordDisposition == expected);
        }
    }

    void testPresentationEvidenceBinding ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        const uint32_t intentGeneration = panel.snapshot ().generation;

        adk::FaultAwareOperatorPanelInput mismatch = imageInput (10, image);
        mismatch.presentationPresent               = true;
        mismatch.presentation = {intentGeneration + 1, adk::MicrosecondTimePoint (10),
                                 adk::Status ()};
        const adk::FaultAwareOperatorPanelSnapshot before = panel.snapshot ();

        assert (panel.update (mismatch).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (before, panel.snapshot ()));

        adk::FaultAwareOperatorPanelInput failure = imageInput (11, image);
        failure.presentationPresent               = true;
        failure.presentation = {intentGeneration, adk::MicrosecondTimePoint (11),
                                adk::StatusCode::HardwareFailure};
        assert (panel.update (failure).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::SourceFault);

        adk::FaultAwareOperatorPanelInput recovered = imageInput (12, image);
        recovered.presentationPresent               = true;
        recovered.presentation = {panel.snapshot ().generation,
                                  adk::MicrosecondTimePoint (12), adk::Status ()};
        assert (panel.update (recovered).ok ());

        assert (panel.snapshot ().diagnostic ==
                adk::PanelDiagnostic::PresentationRecovered);
    }

    void testStopInvalidatesAcknowledge ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        adk::FaultAwareOperatorPanelInput failed = imageInput (2, image);
        failed.controlPresent                    = true;
        failed.control                           = controlEvidence (UINT8_C (1), 1, 2);
        failed.control.status                    = adk::StatusCode::HardwareFailure;
        assert (panel.update (failed).ok ());

        adk::FaultAwareOperatorPanelInput recovered = imageInput (3, image);
        recovered.controlPresent                    = true;
        recovered.control = controlEvidence (UINT8_C (1), 2, 3);

        assert (panel.update (recovered).ok ());
        const adk::Result<adk::PanelAcknowledgePreview> acknowledge =
            panel.prepareAcknowledge (900, adk::MicrosecondTimePoint (4));
        assert (acknowledge.ok ());

        adk::FaultAwareOperatorPanelInput stop = imageInput (5, image);
        stop.stopPresent                       = true;
        stop.stop                              = stopEvidence (true, 1, 5);
        stop.presentationPresent               = true;
        stop.presentation                      = {panel.snapshot ().generation,
                                                  adk::MicrosecondTimePoint (5),
                                                  adk::StatusCode::HardwareFailure};
        assert (panel.update (stop).ok ());

        assert (panel.snapshot ().stopped);

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::Stopped);

        image.slots[acknowledge.value ().audit.slotIndex] =
            acknowledge.value ().audit.record;
        adk::FaultAwareOperatorPanelInput stale = imageInput (6, image);
        stale.acknowledgePresent                = true;
        stale.acknowledge                       = acknowledge.value ();

        assert (panel.update (stale).error () == adk::StatusCode::InvalidArgument);
    }

    void testSequenceAndTimeOrdering ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (UINT32_MAX - 20, image)).ok ());

        adk::FaultAwareOperatorPanelInput beforeWrap =
            imageInput (UINT32_MAX - 10, image);
        beforeWrap.controlPresent = true;
        beforeWrap.control =
            controlEvidence (UINT8_C (2), UINT32_MAX - 1, UINT32_MAX - 10);
        assert (panel.update (beforeWrap).ok ());

        adk::FaultAwareOperatorPanelInput atWrap = imageInput (UINT32_MAX - 5, image);
        atWrap.controlPresent                    = true;
        atWrap.control = controlEvidence (UINT8_C (2), UINT32_MAX, UINT32_MAX - 5);

        assert (panel.update (atWrap).ok ());

        adk::FaultAwareOperatorPanelInput wrapped = imageInput (2, image);
        wrapped.controlPresent                    = true;
        wrapped.control                           = controlEvidence (UINT8_C (2), 1, 2);

        assert (panel.update (wrapped).ok ());

        const adk::FaultAwareOperatorPanelSnapshot accepted = panel.snapshot ();

        adk::FaultAwareOperatorPanelInput duplicate = imageInput (3, image);
        duplicate.controlPresent                    = true;
        duplicate.control = controlEvidence (UINT8_C (1), 1, 2);

        assert (panel.update (duplicate).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (accepted, panel.snapshot ()));

        adk::FaultAwareOperatorPanelInput halfRange = imageInput (3, image);
        halfRange.controlPresent                    = true;
        halfRange.control = controlEvidence (UINT8_C (1), UINT32_C (0x80000001), 3);

        assert (panel.update (halfRange).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (accepted, panel.snapshot ()));

        adk::FaultAwareOperatorPanelInput backward = imageInput (4, image);
        backward.controlPresent                    = true;
        backward.control = controlEvidence (UINT8_C (1), UINT32_MAX, 4);

        assert (panel.update (backward).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (accepted, panel.snapshot ()));
    }

    void testStopReleasedClearsOnlyStopOwnedStopped ()
    {
        testStopAuditAndQualifiedRelease ();
    }
#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 3
    void testPreparedRestartRequiresLiveAttribution ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> prepared =
            prepareSolved (panel, 1000, 2);
        assert (prepared.ok ());

        image.slots[prepared.value ().slotIndex] = prepared.value ().record;

        panel.shutdown ();

        assert (panel.initialize ().ok ());

        assert (panel.update (imageInput (3, image)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);
        assert (!panel.canAcknowledgeAudit (prepared.value ()));

        assert (panel.snapshot ().diagnostic ==
                adk::PanelDiagnostic::AuditIndeterminate);
    }

    void testEveryTornRecordFieldFailsClosed ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> prepared =
            prepareSolved (panel, 1100, 2);
        assert (prepared.ok ());

        commitAudit (panel, image, prepared.value (), 3);

        const uint8_t slot = prepared.value ().slotIndex;

#define ASSERT_TORN_FIELD_CORRUPT(expression)                                          \
    do                                                                                 \
    {                                                                                  \
        adk::PanelAuditImage torn = image;                                             \
        expression;                                                                    \
        assert (panel.update (imageInput (10, torn)).ok ());                           \
        assert (panel.snapshot ().auditDisposition ==                                  \
                adk::PanelAuditDisposition::Corrupt);                                  \
    }                                                                                  \
    while (false)
        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].formatMagic ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].formatVersion ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].configurationRevision ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].instanceEpoch ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].recordSequence ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].operationId ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].kind = adk::PanelAuditKind::None);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].diagnostic =
                                       adk::PanelDiagnostic::ClueInvalid);
        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].diagnosticGeneration ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].parentConfigurationRevision ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].parentInstanceEpoch ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].parentGeneration ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].clueGeneration ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].satisfiedRuleMask ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].policyDigest ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopPresent = true);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopAsserted = true);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopSource.sourceId ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopSource.configurationRevision ^=
                                   1);
        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopSource.sessionEpoch ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopSourceSequence ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].stopObservedAt =
                                       adk::MicrosecondTimePoint (1));
        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].occurredAt =
                                       adk::MicrosecondTimePoint (1));
        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].payloadDigest ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].checksum ^= 1);

        ASSERT_TORN_FIELD_CORRUPT (torn.slots[slot].state =
                                       adk::PanelAuditSlotState::Prepared);
#undef ASSERT_TORN_FIELD_CORRUPT
    }

    void testIndeterminateCannotRestoreOrClearStop ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        adk::FaultAwareOperatorPanelInput stop = imageInput (2, image);
        stop.stopPresent                       = true;
        stop.stop                              = stopEvidence (true, 1, 2);

        assert (panel.update (stop).ok ());

        const adk::Result<adk::PanelAuditPreview> assertion = panel.prepareAudit (
            1001, adk::PanelAuditKind::StopAsserted, adk::MicrosecondTimePoint (3));
        assert (assertion.ok ());

        commitAudit (panel, image, assertion.value (), 4);

        assert (panel.snapshot ().stopped);

        const uint8_t firstSlot               = assertion.value ().slotIndex;
        const uint8_t otherSlot               = static_cast<uint8_t> (firstSlot ^ 1U);
        image.slots[otherSlot]                = image.slots[firstSlot];
        image.slots[otherSlot].recordSequence = 2;
        image.slots[otherSlot].operationId    = 1002;
        image.slots[otherSlot].occurredAt     = adk::MicrosecondTimePoint (5);

        sealRecord (image.slots[otherSlot]);

        assert (panel.update (imageInput (6, image)).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Ready);
        adk::PanelAuditImage duplicate    = image;
        duplicate.slots[1].recordSequence = duplicate.slots[0].recordSequence;
        sealRecord (duplicate.slots[1]);

        adk::FaultAwareOperatorPanelInput collision = imageInput (7, duplicate);
        collision.controlPresent                    = true;
        collision.control              = controlEvidence (UINT8_C (3), 1, 7);
        collision.diagnosticPresent    = true;
        collision.diagnostic           = adk::PanelDiagnostic::ClueInvalid;
        collision.diagnosticGeneration = 1;
        collision.presentationPresent  = true;
        collision.presentation         = {panel.snapshot ().generation,
                                          adk::MicrosecondTimePoint (7),
                                          adk::StatusCode::HardwareFailure};
        assert (panel.update (collision).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);
        assert (panel.snapshot ().stopped);

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::Stopped);

        adk::FaultAwareOperatorPanelInput release = imageInput (8, duplicate);
        release.stopPresent                       = true;
        release.stop                              = stopEvidence (false, 2, 8);

        assert (panel.update (release).ok ());

        assert (panel.snapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);
        assert (panel.snapshot ().stopped);
    }

    void testPresentationTimingAndIntervention ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        const uint32_t generation = panel.snapshot ().generation;

        adk::FaultAwareOperatorPanelInput delayed = imageInput (102, image);
        delayed.presentationPresent               = true;
        delayed.presentation = {generation, adk::MicrosecondTimePoint (1),
                                adk::Status ()};
        const adk::FaultAwareOperatorPanelSnapshot before = panel.snapshot ();

        assert (panel.update (delayed).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (before, panel.snapshot ()));

        adk::FaultAwareOperatorPanelInput failure = imageInput (2, image);
        failure.presentationPresent               = true;
        failure.presentation = {generation, adk::MicrosecondTimePoint (2),
                                adk::StatusCode::HardwareFailure};
        assert (panel.update (failure).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::SourceFault);

        adk::FaultAwareOperatorPanelInput intervention = imageInput (3, image);
        intervention.controlPresent                    = true;
        intervention.control = controlEvidence (UINT8_C (2), 1, 3);

        assert (panel.update (intervention).ok ());

        adk::FaultAwareOperatorPanelInput staleSuccess = imageInput (4, image);
        staleSuccess.presentationPresent               = true;
        staleSuccess.presentation = {generation, adk::MicrosecondTimePoint (4),
                                     adk::Status ()};
        assert (panel.update (staleSuccess).error () ==
                adk::StatusCode::InvalidArgument);
        assert (panel.snapshot ().diagnostic !=
                adk::PanelDiagnostic::PresentationRecovered);
    }

    void testLifecycleMaximumAndExhaustion ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::InertEscapeConsole::setLifecycleGeneration (panel, UINT32_MAX);

        assert (panel.initialize ().ok ());
        const adk::Result<adk::PanelAuditPreview> prepared =
            prepareSolved (panel, 1200, 1);
        assert (prepared.ok ());

        assert (prepared.value ().lifecycleGeneration == UINT32_MAX);

        panel.reset ();

        assert (panel.snapshot ().status.error () == adk::StatusCode::CapacityExceeded);
        const adk::Result<adk::PanelAuditPreview> exhausted =
            prepareSolved (panel, 1201, 2);
        assert (!exhausted.ok ());

        assert (zeroAuditValue (exhausted.value ()));

        assert (panel.initialize ().error () == adk::StatusCode::CapacityExceeded);
    }

    void testUninitializedResetLifecycleContract ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (!panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 0);

        panel.reset ();

        assert (!panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 1);

        panel.reset ();

        assert (!panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 2);

        assert (panel.initialize ().ok ());

        assert (panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 3);
        const adk::Result<adk::PanelAuditPreview> candidate =
            prepareSolved (panel, 1700, 1);
        assert (candidate.ok ());

        panel.reset ();

        assert (panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 4);

        assert (!panel.canAcknowledgeAudit (candidate.value ()));

        panel.shutdown ();

        assert (!panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 5);

        panel.shutdown ();

        assert (!panel.initialized ());

        assert (adk::InertEscapeConsole::lifecycleGeneration (panel) == 5);

        adk::FaultAwareOperatorPanel exhausted (config ());

        adk::InertEscapeConsole::setLifecycleGeneration (exhausted, UINT32_MAX);

        exhausted.reset ();

        assert (!exhausted.initialized ());

        assert (exhausted.snapshot ().status.error () ==
                adk::StatusCode::CapacityExceeded);
        assert (exhausted.initialize ().error () == adk::StatusCode::CapacityExceeded);
    }

    void testRetainedNowOrdering ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (20, image)).ok ());

        const adk::FaultAwareOperatorPanelSnapshot accepted = panel.snapshot ();

        assert (panel.update (imageInput (15, image)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (accepted, panel.snapshot ()));

        assert (panel.update (imageInput (UINT32_C (0x80000014), image)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (accepted, panel.snapshot ()));

        adk::FaultAwareOperatorPanel wrap (config ());

        assert (wrap.initialize ().ok ());

        assert (wrap.update (imageInput (UINT32_MAX - 2, image)).ok ());

        assert (wrap.update (imageInput (3, image)).ok ());
    }

    void testRecordSequenceMaximumRollsToOne ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> first =
            prepareSolved (panel, 1300, 2);
        assert (first.ok ());

        commitAudit (panel, image, first.value (), 3);

        const uint8_t slot               = first.value ().slotIndex;
        image.slots[slot].recordSequence = UINT32_MAX;
        sealRecord (image.slots[slot]);

        assert (panel.update (imageInput (4, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> wrapped =
            prepareSolved (panel, 1301, 5);
        assert (wrapped.ok ());

        assert (wrapped.value ().record.recordSequence == 1);
    }

#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 4
    void testProjectDerivedDiagnosticReplacement ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, imageInput (1, image), adk::PanelDiagnostic::ClueIncomplete,
                    10, false)
                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::ClueIncomplete);

        assert (panel.snapshot ().diagnosticGeneration == 10);

        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, imageInput (2, image), adk::PanelDiagnostic::ClueInvalid, 11,
                    false)
                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::ClueInvalid);

        assert (panel.snapshot ().diagnosticGeneration == 11);

        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, imageInput (3, image), adk::PanelDiagnostic::None, 0, false)

                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::None);
    }

    void testFailedParentForwardingInvalidatesNestedCandidate ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> first =
            prepareSolved (panel, 1400, 2);
        assert (first.ok ());

        assert (panel.canAcknowledgeAudit (first.value ()));

        adk::InertEscapeConsole::invalidateCandidates (panel);

        assert (!panel.canAcknowledgeAudit (first.value ()));
        const adk::Result<adk::PanelAuditPreview> replacement =
            prepareSolved (panel, 1401, 3);
        assert (replacement.ok ());
    }

    void testPuzzleSolvedSuppressionCollision ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());
        const adk::Result<adk::PanelAuditPreview> solved =
            prepareSolved (panel, 1500, 2);
        assert (solved.ok ());

        image.slots[solved.value ().slotIndex] = solved.value ().record;

        adk::FaultAwareOperatorPanelInput collision = imageInput (3, image);
        collision.auditAcknowledgePresent           = true;
        collision.auditAcknowledge                  = solved.value ();
        collision.controlPresent                    = true;
        collision.control = controlEvidence (UINT8_C (3), 1, 3);

        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, collision, adk::PanelDiagnostic::None, 0, false)
                    .ok ());
        assert (panel.snapshot ().auditDisposition !=
                adk::PanelAuditDisposition::Ready);
        assert (panel.snapshot ().chordDisposition ==
                adk::OperatorChordDisposition::InvalidChord);
        assert (panel.canonicalAuditImage ().slots[solved.value ().slotIndex].state !=
                adk::PanelAuditSlotState::Committed);
    }

    void testPresentationOwnedSourceFaultTransfersToGenuineSourceFault ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        assert (panel.update (imageInput (1, image)).ok ());

        adk::FaultAwareOperatorPanelInput presentation = imageInput (2, image);
        presentation.presentationPresent               = true;
        presentation.presentation = {panel.snapshot ().generation,
                                     adk::MicrosecondTimePoint (2),
                                     adk::StatusCode::HardwareFailure};
        assert (panel.update (presentation).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::SourceFault);

        adk::FaultAwareOperatorPanelInput genuine = imageInput (3, image);
        genuine.controlPresent                    = true;
        genuine.control                           = controlEvidence (UINT8_C (1), 1, 3);
        genuine.control.status                    = adk::StatusCode::HardwareFailure;
        assert (panel.update (genuine).ok ());

        assert (panel.update (imageInput (4, image)).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::SourceFault);
    }

    void testSubordinatePresentationNeverOverwritesProjectDiagnostic ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage image = zeroImage ();

        adk::FaultAwareOperatorPanelInput failed = imageInput (1, image);
        failed.presentationPresent               = true;
        failed.presentation                      = {panel.snapshot ().generation,
                                                    adk::MicrosecondTimePoint (1),
                                                    adk::StatusCode::HardwareFailure};
        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, failed, adk::PanelDiagnostic::ClueIncomplete, 20, false)
                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::ClueIncomplete);

        adk::FaultAwareOperatorPanelInput healthy = imageInput (2, image);
        healthy.presentationPresent               = true;
        healthy.presentation = {panel.snapshot ().generation,
                                adk::MicrosecondTimePoint (2), adk::Status ()};
        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, healthy, adk::PanelDiagnostic::ClueIncomplete, 20, false)
                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::ClueIncomplete);

        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, imageInput (3, image), adk::PanelDiagnostic::ClueInvalid, 21,
                    false)
                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::ClueInvalid);

        assert (adk::InertEscapeConsole::projectUpdate (
                    panel, imageInput (4, image), adk::PanelDiagnostic::None, 0, false)

                    .ok ());
        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::None);
    }

    void testReadyRecoveryClearsOnlyAuditOwnedDiagnostic ()
    {
        adk::FaultAwareOperatorPanel panel (config ());

        assert (panel.initialize ().ok ());

        adk::PanelAuditImage ready = zeroImage ();

        assert (panel.update (imageInput (1, ready)).ok ());
        adk::PanelAuditImage corrupt = ready;
        corrupt.slots[0].formatMagic = 1;
        assert (panel.update (imageInput (2, corrupt)).ok ());

        assert (panel.snapshot ().diagnostic ==
                adk::PanelDiagnostic::AuditIndeterminate);
        assert (panel.update (imageInput (3, ready)).ok ());

        assert (panel.snapshot ().diagnostic == adk::PanelDiagnostic::None);

        adk::FaultAwareOperatorPanelInput independent = imageInput (4, ready);
        independent.diagnosticPresent                 = true;
        independent.diagnostic           = adk::PanelDiagnostic::AuditIndeterminate;
        independent.diagnosticGeneration = 30;
        assert (panel.update (independent).ok ());

        assert (panel.update (imageInput (5, ready)).ok ());

        assert (panel.snapshot ().diagnostic ==
                adk::PanelDiagnostic::AuditIndeterminate);
        assert (panel.snapshot ().diagnosticGeneration == 30);
    }

#endif
} // namespace

int main ()
{
    (void)stopEvidence;
    (void)zeroAcknowledgeValue;
    (void)imageDigest;
    (void)sameImage;
    (void)sameSnapshot;
    (void)sealRecord;
    (void)prepareSolved;
    (void)commitAudit;

    static_assert (!std::is_copy_constructible<adk::FaultAwareOperatorPanel>::value,
                   "panel must not copy");
    static_assert (!std::is_move_constructible<adk::FaultAwareOperatorPanel>::value,
                   "panel must not move");

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 1
    testConfigurationAndLifecycle ();

    testDigestBootstrapAndImageForms ();

    testKindAuthorityAndFailedResults ();

    testOwnerLifecycleAndAtomicRejection ();

    testDiagnosticAcknowledgement ();
#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 2
    testStopAuditAndQualifiedRelease ();

    testControlStopAndReplay ();

    testEveryControlMask ();

    testPresentationEvidenceBinding ();

    testStopInvalidatesAcknowledge ();

    testSequenceAndTimeOrdering ();

    testStopReleasedClearsOnlyStopOwnedStopped ();
#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 3
    testPreparedRestartRequiresLiveAttribution ();

    testEveryTornRecordFieldFailsClosed ();

    testIndeterminateCannotRestoreOrClearStop ();

    testPresentationTimingAndIntervention ();

    testLifecycleMaximumAndExhaustion ();

    testUninitializedResetLifecycleContract ();

    testRetainedNowOrdering ();

    testRecordSequenceMaximumRollsToOne ();
#endif

#if ADK_FAULT_AWARE_PANEL_TEST_PART == 0 || ADK_FAULT_AWARE_PANEL_TEST_PART == 4
    testProjectDerivedDiagnosticReplacement ();

    testFailedParentForwardingInvalidatesNestedCandidate ();

    testPuzzleSolvedSuppressionCollision ();

    testPresentationOwnedSourceFaultTransfersToGenuineSourceFault ();

    testSubordinatePresentationNeverOverwritesProjectDiagnostic ();

    testReadyRecoveryClearsOnlyAuditOwnedDiagnostic ();

#endif
}
