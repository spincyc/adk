#include <inert_escape_console.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

#ifndef ADK_ESCAPE_CONSOLE_TEST_PART
#define ADK_ESCAPE_CONSOLE_TEST_PART 1
#endif

namespace adk {
    struct InertEscapeConsoleTestAccess
    {
        static void lifecycleGeneration (InertEscapeConsole& console,
                                         uint32_t            generation) noexcept
        {
            console.lifecycleGeneration_ = generation;
        }

        static void allLifecycleGenerations (InertEscapeConsole& console,
                                             uint32_t            generation) noexcept
        {
            console.lifecycleGeneration_            = generation;
            console.panel_.lifecycleGeneration_     = generation;
            console.clueModel_.lifecycleGeneration_ = generation;
        }

        static uint32_t lifecycleGeneration (const InertEscapeConsole& console) noexcept
        {
            return console.lifecycleGeneration_;
        }

        static uint32_t
        panelLifecycleGeneration (const InertEscapeConsole& console) noexcept
        {
            return console.panel_.lifecycleGeneration_;
        }

        static uint32_t
        clueLifecycleGeneration (const InertEscapeConsole& console) noexcept
        {
            return console.clueModel_.lifecycleGeneration_;
        }

        static void snapshotGeneration (InertEscapeConsole& console,
                                        uint32_t            generation) noexcept
        {
            console.panel_.snapshot_.generation = generation;
        }

        static void disposition (InertEscapeConsole&      console,
                                 EscapeConsoleDisposition disposition) noexcept
        {
            console.retainDisposition (disposition);
        }
    };
} // namespace adk

namespace {

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

    adk::ClueSourceIdentity clueSource (uint8_t index) noexcept
    {
        return {static_cast<uint16_t> (100U + index), 55,
                static_cast<uint32_t> (1000U + index)};
    }

    adk::OperatorSourceIdentity controlSource () noexcept
    {
        return {201, 56, 2001};
    }

    adk::OperatorSourceIdentity stopSource () noexcept
    {
        return {202, 56, 2002};
    }

    adk::ClueRuleDefinition rule (uint8_t index) noexcept
    {
        adk::ClueRuleDefinition value = {};
        value.ruleId                  = index;
        value.termCount               = 1;
        value.terms[0]                = {index, adk::ClueTermRelation::Equals,
                                         adk::ClueCategory::Active};
        return value;
    }

    void hashSource (Fnv32& hash, const adk::ClueSourceIdentity& source) noexcept
    {
        hash.u16 (source.sourceId);
        hash.u16 (source.configurationRevision);
        hash.u32 (source.sessionEpoch);
    }

    void hashSource (Fnv32& hash, const adk::OperatorSourceIdentity& source) noexcept
    {
        hash.u16 (source.sourceId);
        hash.u16 (source.configurationRevision);
        hash.u32 (source.sessionEpoch);
    }

    uint32_t policyDigest (const adk::EscapeConsoleConfig& config) noexcept
    {
        Fnv32      hash;
        const char domain[] = "ADK.ESCAPE.POLICY.V1";
        hash.bytes (domain, sizeof domain);

        hash.u16 (config.configurationRevision);
        hash.u32 (config.instanceEpoch);
        hash.u16 (config.clueModel.configurationRevision);
        hash.u32 (config.clueModel.instanceEpoch);

        hash.u32 (config.clueModel.maximumEvidenceAge.microseconds ());

        hash.byte (config.clueModel.clueCount);
        hash.byte (config.clueModel.ruleCount);
        for (uint8_t index = 0; index < 12; ++index)
        {
            hashSource (hash, config.clueModel.expectedSources[index]);
        }
        for (uint8_t index = 0; index < 12; ++index)
        {
            const adk::ClueRuleDefinition& value = config.clueModel.rules[index];
            hash.byte (value.ruleId);
            hash.byte (value.termCount);
            for (uint8_t termIndex = 0; termIndex < 4; ++termIndex)
            {
                hash.byte (value.terms[termIndex].clueId);
                hash.byte (static_cast<uint8_t> (value.terms[termIndex].relation));
                hash.byte (static_cast<uint8_t> (value.terms[termIndex].category));
            }
            hash.byte (value.prerequisiteCount);
            for (uint8_t prerequisiteIndex = 0; prerequisiteIndex < 4;
                 ++prerequisiteIndex)
            {
                hash.byte (value.prerequisiteRuleIds[prerequisiteIndex]);
            }
        }
        for (uint8_t index = 0; index < 12; ++index)
        {
            hash.byte (static_cast<uint8_t> (config.clueFamilies[index]));
        }
        hash.u16 (config.panel.configurationRevision);
        hash.u32 (config.panel.instanceEpoch);

        hash.u32 (config.panel.maximumInputAge.microseconds ());

        hash.byte (config.panel.selectableCellCount);

        hashSource (hash, config.panel.controlSource);
        hashSource (hash, config.panel.stopSource);

        return hash.value ();
    }

    void hashIdentity (Fnv32& hash, const adk::OperatorSourceIdentity& source) noexcept
    {
        hash.u16 (source.sourceId);
        hash.u16 (source.configurationRevision);
        hash.u32 (source.sessionEpoch);
    }

    void hashAuditPayload (Fnv32& hash, const adk::PanelAuditRecord& record) noexcept
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

    void hashAuditRecord (Fnv32& hash, const adk::PanelAuditRecord& record) noexcept
    {
        hash.u32 (record.formatMagic);
        hash.u16 (record.formatVersion);
        hash.u16 (record.configurationRevision);
        hash.u32 (record.instanceEpoch);
        hash.u32 (record.recordSequence);

        hashAuditPayload (hash, record);

        hash.u32 (record.payloadDigest);

        hash.byte (static_cast<uint8_t> (record.state));
    }

    [[maybe_unused]] void sealRecord (adk::PanelAuditRecord& record) noexcept
    {
        Fnv32      payload;
        const char payloadDomain[] = "ADK.PANEL.PAYLOAD.V1";
        payload.bytes (payloadDomain, sizeof payloadDomain);

        hashAuditPayload (payload, record);

        record.payloadDigest = payload.value ();

        Fnv32      checksum;
        const char recordDomain[] = "ADK.PANEL.RECORD.V1";
        checksum.bytes (recordDomain, sizeof recordDomain);

        hashAuditRecord (checksum, record);

        record.checksum = checksum.value ();
    }

    adk::EscapeConsoleConfig config () noexcept
    {
        adk::EscapeConsoleConfig value;
        std::memset (static_cast<void*> (&value), 0, sizeof value);
        value.configurationRevision           = 57;
        value.instanceEpoch                   = 5701;
        value.clueModel.configurationRevision = 55;
        value.clueModel.instanceEpoch         = 5501;
        value.clueModel.maximumEvidenceAge    = adk::MicrosecondDuration (100);
        value.clueModel.clueCount             = 12;
        value.clueModel.ruleCount             = 12;
        value.panel                           = {
            56,           5601, adk::MicrosecondDuration (100), 12, controlSource (),

            stopSource ()};
        for (uint8_t index = 0; index < 12; ++index)
        {
            value.clueModel.expectedSources[index] = clueSource (index);

            value.clueModel.rules[index] = rule (index);
            value.clueFamilies[index] = static_cast<adk::EscapeClueFamily> (index / 2U);
        }
        value.policyDigest = policyDigest (value);
        return value;
    }

    adk::PanelAuditRecord zeroRecord () noexcept
    {
        adk::PanelAuditRecord result;
        std::memset (static_cast<void*> (&result), 0, sizeof result);
        return result;
    }

    adk::PanelAuditImage zeroImage () noexcept
    {
        return {{zeroRecord (), zeroRecord ()}};
    }

    adk::ClueConstraintUpdate zeroClueUpdate (uint32_t now) noexcept
    {
        adk::ClueConstraintUpdate result;
        std::memset (static_cast<void*> (&result), 0, sizeof result);

        result.now = adk::MicrosecondTimePoint (now);
        return result;
    }

    [[maybe_unused]] adk::EscapeConsolePreview zeroPreview () noexcept
    {
        adk::EscapeConsolePreview result;
        std::memset (static_cast<void*> (&result), 0, sizeof result);
        return result;
    }

    adk::EscapeConsoleUpdate emptyUpdate (uint32_t now) noexcept
    {
        adk::EscapeConsoleUpdate result;
        std::memset (static_cast<void*> (&result), 0, sizeof result);

        result.now = adk::MicrosecondTimePoint (now);

        result.clueUpdate = zeroClueUpdate (0);
        return result;
    }

    adk::ClueObservation observation (uint8_t clueId, adk::ClueQuality quality,
                                      uint32_t sequence, uint32_t now) noexcept
    {
        return {clueId,        adk::ClueCategory::Active,
                quality,       clueSource (clueId),

                sequence,      adk::MicrosecondTimePoint (now),

                adk::Status ()};
    }

    adk::ClueObservation zeroObservation () noexcept
    {
        return {0,
                adk::ClueCategory::Absent,
                adk::ClueQuality::Invalid,
                {0, 0, 0},
                0,
                adk::MicrosecondTimePoint (),

                adk::Status ()};
    }

    adk::OperatorStopEvidence stop (bool asserted, uint32_t sequence,
                                    uint32_t now) noexcept
    {
        return {asserted, stopSource (), sequence, adk::MicrosecondTimePoint (now),
                adk::Status ()};
    }

    [[maybe_unused]] adk::OperatorControlEvidence control (uint8_t mask,
                                                           uint32_t sequence,
                                          uint32_t now) noexcept
    {
        return {mask, controlSource (), sequence, adk::MicrosecondTimePoint (now),
                adk::Status ()};
    }

    adk::EscapeConsoleUpdate solvedUpdate (uint32_t now, uint32_t sequence) noexcept
    {
        adk::EscapeConsoleUpdate result = emptyUpdate (now);
        result.auditImagePresent        = true;
        result.auditImage               = zeroImage ();
        result.clueUpdatePresent        = true;
        result.clueUpdate.now           = adk::MicrosecondTimePoint (now);

        result.clueUpdate.observationMask = UINT16_C (0x0fff);
        for (uint8_t index = 0; index < 12; ++index)
        {
            result.clueUpdate.observations[index] =
                observation (index, adk::ClueQuality::Qualified, sequence, now);
        }
        result.stopPresent = true;
        result.stop        = stop (false, sequence, now);
        return result;
    }

    [[maybe_unused]] adk::EscapeConsoleUpdate
    clueMaskUpdate (uint16_t mask, uint32_t now, uint32_t sequence) noexcept
    {
        adk::EscapeConsoleUpdate result   = solvedUpdate (now, sequence);
        result.clueUpdate.observationMask = mask;
        for (uint8_t index = 0; index < 12; ++index)
        {
            if ((mask & static_cast<uint16_t> (UINT16_C (1) << index)) == 0)
            {
                result.clueUpdate.observations[index] = zeroObservation ();
            }
        }
        return result;
    }

    [[maybe_unused]] bool
    sameSnapshot (const adk::EscapeConsoleSnapshot& left,
                  const adk::EscapeConsoleSnapshot& right) noexcept
    {
        return std::memcmp (&left, &right, sizeof left) == 0;
    }

    [[maybe_unused]] adk::EscapeConsolePreview
    prepareSolved (adk::InertEscapeConsole& console,
                                             uint32_t                 operationId,
                                             uint32_t                 now) noexcept
    {
        const adk::Result<adk::EscapeConsolePreview> prepared =
            console.prepareSolve (operationId, adk::MicrosecondTimePoint (now));

        assert (prepared.ok ());

        return prepared.value ();
    }

    [[maybe_unused]] adk::PanelAuditImage
    commitSolved (adk::InertEscapeConsole&         console,
                                       const adk::EscapeConsolePreview& preview,
                                       uint32_t                         now) noexcept
    {
        adk::PanelAuditImage image           = console.canonicalAuditImage ();
        image.slots[preview.audit.slotIndex] = preview.audit.record;

        adk::EscapeConsoleUpdate commit = emptyUpdate (now);
        commit.auditImagePresent        = true;
        commit.auditImage               = image;
        commit.auditAcknowledgePresent  = true;
        commit.auditAcknowledge         = preview.audit;
        commit.solvePreviewPresent      = true;
        commit.solvePreview             = preview;

        assert (console.update (commit).ok ());

        return console.canonicalAuditImage ();
    }

    [[maybe_unused]] adk::PanelAuditImage
    commitPanelAudit (adk::InertEscapeConsole&      console,
                                           const adk::PanelAuditPreview& preview,
                                           uint32_t                      now) noexcept
    {
        adk::PanelAuditImage image     = console.canonicalAuditImage ();
        image.slots[preview.slotIndex] = preview.record;

        adk::EscapeConsoleUpdate commit = emptyUpdate (now);
        commit.auditImagePresent        = true;
        commit.auditImage               = image;
        commit.auditAcknowledgePresent  = true;
        commit.auditAcknowledge         = preview;

        assert (console.update (commit).ok ());

        return console.canonicalAuditImage ();
    }

#if ADK_ESCAPE_CONSOLE_TEST_PART == 1
    void testConfigurationAndLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::InertEscapeConsole>::value,
                       "console must not copy");
        static_assert (!std::is_move_constructible<adk::InertEscapeConsole>::value,
                       "console must not move");

        adk::EscapeConsoleConfig invalid = config ();

        invalid.policyDigest ^= UINT32_C (1);
        adk::InertEscapeConsole rejected (invalid);

        assert (rejected.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
        assert (!rejected.initialized ());

        assert (rejected.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        adk::InertEscapeConsole console (config ());

        assert (!console.initialized ());

        assert (console.initialize ().ok ());

        const adk::EscapeConsoleSnapshot initialized = console.snapshot ();

        assert (initialized.disposition ==
                adk::EscapeConsoleDisposition::AwaitingClues);
        assert (initialized.latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (console.initialize ().ok ());

        assert (sameSnapshot (initialized, console.snapshot ()));

        console.reset ();

        assert (console.initialized ());

        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        console.shutdown ();

        assert (!console.initialized ());

        const adk::EscapeConsoleSnapshot shutdown = console.snapshot ();

        console.shutdown ();

        assert (sameSnapshot (shutdown, console.snapshot ()));

        assert (console.initialize ().ok ());
    }

    void testResetBeforeInitializationCoordinatesLifecycle ()
    {
        adk::InertEscapeConsole console (config ());

        assert (!console.initialized ());

        assert (adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console) == 0);

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                0);
        assert (adk::InertEscapeConsoleTestAccess::clueLifecycleGeneration (console) ==
                0);

        console.reset ();

        assert (!console.initialized ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::Uninitialized);
        assert (console.snapshot ().status.error () == adk::StatusCode::NotInitialized);

        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console) == 1);

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                1);
        assert (adk::InertEscapeConsoleTestAccess::clueLifecycleGeneration (console) ==
                1);
        assert (!console.prepareSolve (1, adk::MicrosecondTimePoint (1)).ok ());

        console.reset ();

        assert (!console.initialized ());

        assert (adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console) == 2);

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                2);
        assert (adk::InertEscapeConsoleTestAccess::clueLifecycleGeneration (console) ==
                2);

        const uint32_t beforeShutdown =
            adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console);
        console.shutdown ();
        console.shutdown ();

        assert (adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console) ==
                beforeShutdown);

        assert (console.initialize ().ok ());

        assert (console.initialized ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::AwaitingClues);
        assert (console.snapshot ().status.ok ());

        assert (adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console) == 3);

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                3);
        assert (adk::InertEscapeConsoleTestAccess::clueLifecycleGeneration (console) ==
                3);

        console.reset ();

        assert (console.initialized ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::AwaitingClues);
        assert (console.snapshot ().status.ok ());

        assert (adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console) == 4);

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                4);
        assert (adk::InertEscapeConsoleTestAccess::clueLifecycleGeneration (console) ==
                4);
    }

    void testResetBeforeInitializationCanExhaust ()
    {
        adk::InertEscapeConsole console (config ());

        adk::InertEscapeConsoleTestAccess::allLifecycleGenerations (console,
                                                                    UINT32_MAX);
        console.reset ();

        assert (!console.initialized ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::InternalFault);
        assert (console.snapshot ().status.error () ==
                adk::StatusCode::CapacityExceeded);
        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (console.initialize ().error () == adk::StatusCode::CapacityExceeded);

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                UINT32_MAX);
        assert (adk::InertEscapeConsoleTestAccess::clueLifecycleGeneration (console) ==
                UINT32_MAX);

        const uint32_t beforeShutdown =
            adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console);
        console.shutdown ();
        console.shutdown ();

        assert (adk::InertEscapeConsoleTestAccess::panelLifecycleGeneration (console) ==
                beforeShutdown);
    }

    void testSolvedEvidenceAndFamilies ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());

        const adk::EscapeConsoleSnapshot snapshot = console.snapshot ();

        assert (console.clueSnapshot ().disposition ==
                adk::ClueModelDisposition::Solved);
        assert (console.clueSnapshot ().satisfiedRuleMask == UINT16_C (0x0fff));
        for (uint8_t index = 0; index < 6; ++index)
        {
            assert (snapshot.families[index].family ==
                    static_cast<adk::EscapeClueFamily> (index));
            assert (snapshot.families[index].firstClueId == index * 2U);
            assert (snapshot.families[index].secondClueId == index * 2U + 1U);
            assert (snapshot.families[index].complete);
            assert (snapshot.families[index].weakestQuality ==
                    adk::ClueQuality::Qualified);
        }
        assert (snapshot.latchIntent == adk::EscapeLatchIntent::Inactive);
    }

    void testEveryCluePresenceMask ()
    {
        for (uint16_t mask = 0; mask < UINT16_C (0x1000); ++mask)
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            assert (console.update (clueMaskUpdate (mask, 10, 1)).ok ());

            const adk::ClueConstraintSnapshot clues = console.clueSnapshot ();

            assert (clues.satisfiedRuleMask == mask);

            assert (clues.disposition == (mask == UINT16_C (0x0fff)
                                              ? adk::ClueModelDisposition::Solved
                                              : adk::ClueModelDisposition::Incomplete));
            for (uint8_t family = 0; family < 6; ++family)
            {
                const uint16_t pairMask = static_cast<uint16_t> (
                    UINT16_C (3) << static_cast<uint8_t> (family * 2U));
                assert (console.snapshot ().families[family].complete ==
                        ((mask & pairMask) == pairMask));
            }
        }
    }

    void testFamilyAndDigestConfigurationMutations ()
    {
        for (uint8_t slot = 0; slot < 12; ++slot)
        {
            adk::EscapeConsoleConfig invalid = config ();
            invalid.clueFamilies[slot] =
                static_cast<adk::EscapeClueFamily> (UINT8_C (6));
            invalid.policyDigest = policyDigest (invalid);

            adk::InertEscapeConsole console (invalid);

            assert (console.initialize ().error () ==
                    adk::StatusCode::InvalidConfiguration);
        }
        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            adk::EscapeConsoleConfig invalid = config ();

            invalid.policyDigest ^= UINT32_C (1) << bit;

            adk::InertEscapeConsole console (invalid);

            assert (console.initialize ().error () ==
                    adk::StatusCode::InvalidConfiguration);
        }
    }

    void assertFamilyMapping (const adk::InertEscapeConsole&  console,
                              const adk::EscapeConsoleConfig& value) noexcept
    {
        const adk::EscapeConsoleSnapshot snapshot = console.snapshot ();
        for (uint8_t family = 0; family < 6; ++family)
        {
            uint8_t first  = UINT8_MAX;
            uint8_t second = UINT8_MAX;
            for (uint8_t clue = 0; clue < 12; ++clue)
            {
                if (value.clueFamilies[clue] ==
                    static_cast<adk::EscapeClueFamily> (family))
                {
                    if (first == UINT8_MAX)
                    {
                        first = clue;
                    }
                    else
                    {
                        second = clue;
                    }
                }
            }
            assert (snapshot.families[family].family ==
                    static_cast<adk::EscapeClueFamily> (family));
            assert (snapshot.families[family].firstClueId == first);
            assert (snapshot.families[family].secondClueId == second);
        }
    }

    void testPackedFamilyMappingsAcrossLifecycle ()
    {
        for (uint8_t layout = 0; layout < 12; ++layout)
        {
            adk::EscapeConsoleConfig value = config ();
            for (uint8_t clue = 0; clue < 12; ++clue)
            {
                const uint8_t base =
                    layout < 6 ? clue % 6U : static_cast<uint8_t> (clue / 2U);
                value.clueFamilies[clue] = static_cast<adk::EscapeClueFamily> (
                    (base + static_cast<uint8_t> (layout % 6U)) % 6U);
            }
            value.policyDigest = policyDigest (value);

            adk::InertEscapeConsole console (value);

            assertFamilyMapping (console, value);

            assert (console.initialize ().ok ());

            assertFamilyMapping (console, value);
            for (uint8_t disposition = 0; disposition < 15; ++disposition)
            {
                adk::InertEscapeConsoleTestAccess::disposition (
                    console, static_cast<adk::EscapeConsoleDisposition> (disposition));
                assertFamilyMapping (console, value);
            }
            console.reset ();

            adk::InertEscapeConsole terminal (value);

            assert (terminal.initialize ().ok ());

            adk::InertEscapeConsoleTestAccess::disposition (
                terminal, adk::EscapeConsoleDisposition::InternalFault);
            assertFamilyMapping (terminal, value);

            assert (console.update (clueMaskUpdate (UINT16_C (0x0555), 10, 1)).ok ());

            assertFamilyMapping (console, value);

            console.reset ();

            assertFamilyMapping (console, value);

            console.shutdown ();

            assertFamilyMapping (console, value);

            assert (console.initialize ().ok ());

            assert (console.update (solvedUpdate (20, 2)).ok ());

            assertFamilyMapping (console, value);
            for (uint8_t family = 0; family < 6; ++family)
            {
                assert (console.snapshot ().families[family].complete);
            }
        }

        adk::EscapeConsoleConfig duplicate = config ();
        duplicate.clueFamilies[0]          = adk::EscapeClueFamily::Pattern;
        duplicate.policyDigest             = policyDigest (duplicate);

        adk::InertEscapeConsole rejected (duplicate);

        assert (rejected.initialize ().error () ==
                adk::StatusCode::InvalidConfiguration);
    }

#endif

#if ADK_ESCAPE_CONSOLE_TEST_PART == 4
    void testStructuralRejectionIsAtomic ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());

        const adk::EscapeConsoleSnapshot prior = console.snapshot ();

        const adk::ClueConstraintSnapshot          priorClues = console.clueSnapshot ();
        const adk::FaultAwareOperatorPanelSnapshot priorPanel =
            console.panelSnapshot ();
        const adk::PanelAuditImage priorImage = console.canonicalAuditImage ();

        adk::EscapeConsoleUpdate malformed = emptyUpdate (11);
        malformed.solvePreview             = zeroPreview ();
        malformed.solvePreview.operationId = 7;
        assert (console.update (malformed).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (prior, console.snapshot ()));

        const adk::ClueConstraintSnapshot          afterClues = console.clueSnapshot ();
        const adk::FaultAwareOperatorPanelSnapshot afterPanel =
            console.panelSnapshot ();
        assert (std::memcmp (&priorClues, &afterClues, sizeof priorClues) == 0);

        assert (std::memcmp (&priorPanel, &afterPanel, sizeof priorPanel) == 0);

        const adk::PanelAuditImage after = console.canonicalAuditImage ();

        assert (std::memcmp (&priorImage, &after, sizeof after) == 0);
    }
#endif

#if ADK_ESCAPE_CONSOLE_TEST_PART == 2
    void testStopDominatesCollision ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        adk::EscapeConsoleUpdate input = solvedUpdate (20, 1);

        input.stop                               = stop (true, 1, 20);
        input.controlPresent                     = true;
        input.control                            = control (UINT8_C (3), 1, 20);
        input.clueUpdate.observations[0].quality = adk::ClueQuality::Contradictory;
        assert (console.update (input).ok ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::Stopped);
        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (console.snapshot ().lampIntent == adk::EscapeLampIntent::Stopped);

        assert (console.panelSnapshot ().chordDisposition ==
                adk::OperatorChordDisposition::InvalidChord);
        assert (console.clueSnapshot ().disposition ==
                adk::ClueModelDisposition::ContradictoryEvidence);
    }

    void testEvidencePrecedenceAndBoundaries ()
    {
        const adk::ClueQuality qualities[] = {
            adk::ClueQuality::SourceFault, adk::ClueQuality::TimingFault,
            adk::ClueQuality::Stale, adk::ClueQuality::Contradictory};
        const adk::EscapeConsoleDisposition dispositions[] = {
            adk::EscapeConsoleDisposition::SourceFault,
            adk::EscapeConsoleDisposition::TimingFault,
            adk::EscapeConsoleDisposition::StaleEvidence,
            adk::EscapeConsoleDisposition::ContradictoryEvidence};
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input           = solvedUpdate (100, 1);
            input.clueUpdate.observations[0].quality = qualities[index];
            input.controlPresent                     = true;
            input.control                            = control (UINT8_C (3), 1, 100);

            assert (console.update (input).ok ());

            assert (console.snapshot ().disposition == dispositions[index]);

            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
            assert (console.snapshot ().lampIntent == adk::EscapeLampIntent::Fault);
        }

        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        adk::EscapeConsoleUpdate exact = solvedUpdate (UINT32_MAX, 1);
        for (uint8_t index = 0; index < 12; ++index)
        {
            exact.clueUpdate.observations[index].observedAt =
                adk::MicrosecondTimePoint (UINT32_MAX - 100U);
        }
        exact.stop.observedAt = adk::MicrosecondTimePoint (UINT32_MAX - 100U);

        assert (console.update (exact).ok ());

        assert (console.clueSnapshot ().disposition ==
                adk::ClueModelDisposition::Solved);
    }

    void testEveryOperatorMaskWithStopDominance ()
    {
        for (uint8_t mask = 0; mask < 16; ++mask)
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input = solvedUpdate (20, 1);

            input.stop           = stop (true, 1, 20);
            input.controlPresent = true;
            input.control        = control (mask, 1, 20);

            assert (console.update (input).ok ());

            assert (console.snapshot ().disposition ==
                    adk::EscapeConsoleDisposition::Stopped);
            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
        }
    }

    void testPreviewOwnershipAndInvalidation ()
    {
        adk::InertEscapeConsole first (config ());

        adk::InertEscapeConsole second (config ());

        assert (first.initialize ().ok ());

        assert (second.initialize ().ok ());

        assert (first.update (solvedUpdate (10, 1)).ok ());

        assert (second.update (solvedUpdate (10, 1)).ok ());

        assert (first.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::PrepareRequired);
        const adk::Result<adk::EscapeConsolePreview> prepared =
            first.prepareSolve (501, adk::MicrosecondTimePoint (11));
        assert (prepared.ok ());

        assert (prepared.value ().audit.record.recordSequence == 1);

        assert (first.canCommit (prepared.value ()));

        assert (first.initialize ().ok ());

        assert (first.canCommit (prepared.value ()));

        assert (!second.canCommit (prepared.value ()));

        adk::EscapeConsolePreview changed = prepared.value ();
        ++changed.operationId;
        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.ownerToken;
        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.lifecycleGeneration;
        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.configurationRevision;
        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.instanceEpoch;
        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.consoleGeneration;
        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.clueGeneration;
        assert (!first.canCommit (changed));

        changed = prepared.value ();

        changed.satisfiedRuleMask ^= UINT16_C (1);

        assert (!first.canCommit (changed));

        changed = prepared.value ();

        changed.policyDigest ^= UINT32_C (1);

        assert (!first.canCommit (changed));

        changed = prepared.value ();
        ++changed.audit.ownerToken;
        assert (!first.canCommit (changed));

        changed = prepared.value ();

        changed.audit.imageDigest ^= UINT32_C (1);

        assert (!first.canCommit (changed));

        first.reset ();

        assert (!first.canCommit (prepared.value ()));

        assert (first.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);
    }

    void testPreparedAuditAndCommit ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());

        assert (console.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::PrepareRequired);
        const adk::Result<adk::EscapeConsolePreview> prepared =
            console.prepareSolve (700, adk::MicrosecondTimePoint (11));
        assert (prepared.ok ());

        adk::EscapeConsolePreview preview = prepared.value ();

        assert (console.canCommit (preview));

        adk::PanelAuditImage image           = console.canonicalAuditImage ();
        image.slots[preview.audit.slotIndex] = preview.audit.record;
        adk::EscapeConsoleUpdate commit      = emptyUpdate (12);
        commit.auditImagePresent             = true;
        commit.auditImage                    = image;
        commit.auditAcknowledgePresent       = true;
        commit.auditAcknowledge              = preview.audit;
        commit.solvePreviewPresent           = true;
        commit.solvePreview                  = preview;
        assert (console.update (commit).ok ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::Solved);
        assert (console.snapshot ().operationId == 700);

        assert (console.snapshot ().latchIntent ==
                adk::EscapeLatchIntent::RequestDemonstrationRelease);
        assert (console.snapshot ().lampIntent == adk::EscapeLampIntent::Solved);

        assert (!console.canCommit (preview));

        const adk::EscapeConsoleSnapshot solved = console.snapshot ();

        assert (console.update (commit).error () == adk::StatusCode::InvalidArgument);

        assert (sameSnapshot (solved, console.snapshot ()));
    }

    void testInvalidChordPreventsSolveCommit ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());

        const adk::Result<adk::EscapeConsolePreview> prepared =
            console.prepareSolve (701, adk::MicrosecondTimePoint (11));

        assert (prepared.ok ());

        const adk::EscapeConsolePreview preview = prepared.value ();

        adk::PanelAuditImage image           = console.canonicalAuditImage ();
        image.slots[preview.audit.slotIndex] = preview.audit.record;

        adk::EscapeConsoleUpdate collision = emptyUpdate (12);
        collision.auditImagePresent        = true;
        collision.auditImage               = image;
        collision.controlPresent           = true;
        collision.control                  = control (UINT8_C (3), 2, 12);
        collision.auditAcknowledgePresent  = true;
        collision.auditAcknowledge         = preview.audit;
        collision.solvePreviewPresent      = true;
        collision.solvePreview             = preview;

        assert (console.update (collision).ok ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::InvalidOperatorChord);
        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (console.snapshot ().operationId == 0);

        assert (console.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::PrepareRequired);
    }
#endif

#if ADK_ESCAPE_CONSOLE_TEST_PART == 3
    adk::EscapeConsoleUpdate
    solveCommitEnvelope (adk::InertEscapeConsole&         console,
                         const adk::EscapeConsolePreview& preview,
                         uint32_t                         now) noexcept
    {
        adk::PanelAuditImage image           = console.canonicalAuditImage ();
        image.slots[preview.audit.slotIndex] = preview.audit.record;
        adk::EscapeConsoleUpdate result      = emptyUpdate (now);
        result.auditImagePresent             = true;
        result.auditImage                    = image;
        result.auditAcknowledgePresent       = true;
        result.auditAcknowledge              = preview.audit;
        result.solvePreviewPresent           = true;
        result.solvePreview                  = preview;
        return result;
    }

    void testSolvePairCannotCommitUnderProposedChildChanges ()
    {
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            assert (console.update (solvedUpdate (10, 1)).ok ());

            const adk::EscapeConsolePreview preview = prepareSolved (console, 750, 11);

            const adk::PanelAuditImage before = console.canonicalAuditImage ();
            adk::EscapeConsoleUpdate   collision =
                solveCommitEnvelope (console, preview, 12);
            collision.stopPresent = true;
            collision.stop        = stop (true, 2, 12);

            assert (console.update (collision).ok ());

            assert (console.snapshot ().disposition ==
                    adk::EscapeConsoleDisposition::Stopped);
            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
            assert (console.panelSnapshot ().auditDisposition ==
                    adk::PanelAuditDisposition::PrepareRequired);
            const adk::PanelAuditImage after = console.canonicalAuditImage ();

            assert (std::memcmp (&before, &after, sizeof before) == 0);

            assert (console.snapshot ().operationId == 0);
        }
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            assert (console.update (solvedUpdate (10, 1)).ok ());

            const adk::EscapeConsolePreview preview = prepareSolved (console, 751, 11);

            const adk::PanelAuditImage before = console.canonicalAuditImage ();
            adk::EscapeConsoleUpdate   collision =
                solveCommitEnvelope (console, preview, 12);
            collision.clueUpdatePresent = true;
            collision.clueUpdate        = solvedUpdate (12, 2).clueUpdate;
            collision.clueUpdate.observations[0].quality =
                adk::ClueQuality::Contradictory;
            assert (console.update (collision).ok ());

            assert (console.snapshot ().disposition ==
                    adk::EscapeConsoleDisposition::ContradictoryEvidence);
            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
            assert (console.panelSnapshot ().auditDisposition ==
                    adk::PanelAuditDisposition::PrepareRequired);
            const adk::PanelAuditImage after = console.canonicalAuditImage ();

            assert (std::memcmp (&before, &after, sizeof before) == 0);

            assert (console.snapshot ().operationId == 0);
        }
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            assert (console.update (solvedUpdate (10, 1)).ok ());

            const adk::EscapeConsolePreview preview = prepareSolved (console, 752, 11);

            const adk::PanelAuditImage before = console.canonicalAuditImage ();
            adk::EscapeConsoleUpdate   collision =
                solveCommitEnvelope (console, preview, 12);
            collision.presentationPresent = true;
            collision.presentation = {console.panelSnapshot ().generation,
                                      adk::MicrosecondTimePoint (12),

                                      adk::Status (adk::StatusCode::HardwareFailure)};
            assert (console.update (collision).ok ());

            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
            assert (console.panelSnapshot ().auditDisposition ==
                    adk::PanelAuditDisposition::PrepareRequired);
            const adk::PanelAuditImage after = console.canonicalAuditImage ();

            assert (std::memcmp (&before, &after, sizeof before) == 0);

            assert (console.snapshot ().operationId == 0);
        }
    }

    void testAuditImagesAcrossRestart ()
    {
        adk::InertEscapeConsole source (config ());

        assert (source.initialize ().ok ());

        assert (source.update (solvedUpdate (10, 1)).ok ());

        const adk::EscapeConsolePreview preview = prepareSolved (source, 801, 11);

        const adk::PanelAuditImage committed = commitSolved (source, preview, 12);

        adk::EscapeConsoleUpdate recover = emptyUpdate (20);
        recover.auditImagePresent        = true;
        recover.auditImage               = committed;
        adk::InertEscapeConsole restarted (config ());

        assert (restarted.initialize ().ok ());

        const adk::Status restartStatus = restarted.update (recover);

        assert (restartStatus.ok ());

        assert (restarted.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Ready);
        assert (restarted.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (restarted.update (recover).ok ());

        adk::PanelAuditImage prepared           = zeroImage ();
        prepared.slots[preview.audit.slotIndex] = preview.audit.record;
        adk::InertEscapeConsole pending (config ());

        assert (pending.initialize ().ok ());
        recover.auditImage = prepared;
        assert (pending.update (recover).ok ());

        assert (pending.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);
        assert (pending.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::AuditIndeterminate);
        assert (pending.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        adk::PanelAuditImage duplicate = committed;
        duplicate.slots[1]             = duplicate.slots[0];
        adk::InertEscapeConsole indeterminate (config ());

        assert (indeterminate.initialize ().ok ());
        recover.auditImage = duplicate;
        assert (indeterminate.update (recover).ok ());

        assert (indeterminate.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Indeterminate);
        assert (indeterminate.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::AuditIndeterminate);

        adk::PanelAuditImage corrupt = committed;
        corrupt.slots[preview.audit.slotIndex].checksum ^= UINT32_C (1);

        adk::InertEscapeConsole rejected (config ());

        assert (rejected.initialize ().ok ());
        recover.auditImage = corrupt;
        assert (rejected.update (recover).ok ());

        assert (rejected.panelSnapshot ().auditDisposition ==
                adk::PanelAuditDisposition::Corrupt);
        assert (rejected.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);
    }

    void testSolveAdmissionRequiresQualifiedReleasedStop ()
    {
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input = solvedUpdate (10, 1);
            input.stopPresent              = false;
            input.stop                     = emptyUpdate (0).stop;

            assert (console.update (input).ok ());

            assert (!console.prepareSolve (900, adk::MicrosecondTimePoint (11)).ok ());
        }
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input = solvedUpdate (10, 1);

            input.stop = stop (true, 1, 10);

            assert (console.update (input).ok ());

            assert (!console.prepareSolve (901, adk::MicrosecondTimePoint (11)).ok ());
        }
        const adk::ClueQuality rejectedQualities[] = {
            adk::ClueQuality::Stale, adk::ClueQuality::SourceFault,
            adk::ClueQuality::TimingFault, adk::ClueQuality::Contradictory};
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input           = solvedUpdate (10, 1);
            input.clueUpdate.observations[0].quality = rejectedQualities[index];
            assert (console.update (input).ok ());

            assert (!console
                         .prepareSolve (static_cast<uint32_t> (910U + index),
                                        adk::MicrosecondTimePoint (11))
                         .ok ());
        }

        const adk::OperatorStopEvidence malformedStops[] = {
            {false, {999, 56, 2002}, 1, adk::MicrosecondTimePoint (10), adk::Status ()},

            stop (false, 1, UINT32_C (0x8000000a)),

            stop (false, 1, 11)};
        const uint32_t malformedNow[] = {10, 10, 10};
        for (uint8_t index = 0; index < 3; ++index)
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input = solvedUpdate (malformedNow[index], 1);
            input.stop                     = malformedStops[index];
            assert (console.update (input).error () ==
                    adk::StatusCode::InvalidArgument);
            assert (!console
                         .prepareSolve (static_cast<uint32_t> (930U + index),
                                        adk::MicrosecondTimePoint (12))
                         .ok ());
        }

        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            adk::EscapeConsoleUpdate input = solvedUpdate (10, 1);

            input.stop.status = adk::Status (adk::StatusCode::HardwareFailure);

            assert (console.update (input).ok ());

            assert (!console.prepareSolve (940, adk::MicrosecondTimePoint (11)).ok ());
        }
    }

    void testShutdownReinitializeRejectsStalePreview ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());

        const adk::EscapeConsolePreview preview = prepareSolved (console, 950, 11);

        console.shutdown ();

        assert (!console.canCommit (preview));

        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (console.initialize ().ok ());

        assert (!console.canCommit (preview));

        assert (console.update (solvedUpdate (20, 2)).ok ());

        const adk::EscapeConsolePreview fresh = prepareSolved (console, 951, 21);

        assert (fresh.lifecycleGeneration != preview.lifecycleGeneration);

        assert (console.canCommit (fresh));
    }

#endif

#if ADK_ESCAPE_CONSOLE_TEST_PART == 5
    void testStopAuditRestartAndQualifiedRelease ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        adk::EscapeConsoleUpdate asserted = emptyUpdate (100);
        asserted.auditImagePresent        = true;
        asserted.auditImage               = zeroImage ();
        asserted.stopPresent              = true;
        asserted.stop                     = stop (true, 10, 100);

        assert (console.update (asserted).ok ());

        assert (console.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::Stopped);

        const adk::Result<adk::PanelAuditPreview> assertPreview =
            console.preparePanelAudit (1001, adk::PanelAuditKind::StopAsserted,
                                       adk::MicrosecondTimePoint (101));
        assert (assertPreview.ok ());
        const adk::PanelAuditImage assertedImage =
            commitPanelAudit (console, assertPreview.value (), 102);

        adk::InertEscapeConsole restarted (config ());

        assert (restarted.initialize ().ok ());

        adk::EscapeConsoleUpdate recover = emptyUpdate (110);
        recover.auditImagePresent        = true;
        recover.auditImage               = assertedImage;
        assert (restarted.update (recover).ok ());

        assert (restarted.snapshot ().disposition ==
                adk::EscapeConsoleDisposition::Stopped);
        assert (restarted.panelSnapshot ().stopped);

        adk::EscapeConsoleUpdate clues = clueMaskUpdate (UINT16_C (0x0fff), 111, 1);
        clues.auditImagePresent        = false;
        clues.auditImage               = zeroImage ();
        clues.stopPresent              = false;
        clues.stop                     = emptyUpdate (0).stop;

        assert (restarted.update (clues).ok ());

        const adk::OperatorStopEvidence invalidReleases[] = {
            stop (false, 10, 100), stop (false, 11, 100), stop (false, 10, 101),

            stop (false, UINT32_C (0x8000000a), 101),

            stop (false, 11, UINT32_C (0x80000064))};
        for (uint8_t index = 0; index < 5; ++index)
        {
            adk::EscapeConsoleUpdate invalid        = emptyUpdate (112);
            invalid.stopPresent                     = true;
            invalid.stop                            = invalidReleases[index];
            const adk::EscapeConsoleSnapshot before = restarted.snapshot ();

            const adk::Status invalidStatus = restarted.update (invalid);

            assert (invalidStatus.error () == adk::StatusCode::InvalidArgument);

            assert (sameSnapshot (before, restarted.snapshot ()));
        }

        adk::EscapeConsoleUpdate released = emptyUpdate (120);
        released.stopPresent              = true;
        released.stop                     = stop (false, 11, 101);

        assert (restarted.update (released).ok ());

        assert (restarted.panelSnapshot ().stopped);

        assert (!restarted.prepareSolve (1003, adk::MicrosecondTimePoint (120)).ok ());

        const adk::Result<adk::PanelAuditPreview> releasePreview =
            restarted.preparePanelAudit (1002, adk::PanelAuditKind::StopReleased,
                                         adk::MicrosecondTimePoint (121));
        assert (releasePreview.ok ());
        const adk::PanelAuditImage releasedImage =
            commitPanelAudit (restarted, releasePreview.value (), 122);
        assert (!restarted.panelSnapshot ().stopped);

        assert (restarted.prepareSolve (1004, adk::MicrosecondTimePoint (123)).ok ());

        adk::InertEscapeConsole releasedRestart (config ());

        assert (releasedRestart.initialize ().ok ());

        recover.now        = adk::MicrosecondTimePoint (130);
        recover.auditImage = releasedImage;
        assert (releasedRestart.update (recover).ok ());

        assert (!releasedRestart.panelSnapshot ().stopped);

        assert (releasedRestart.snapshot ().latchIntent ==
                adk::EscapeLatchIntent::Inactive);
    }

    adk::PanelAuditDisposition
    classifyRestartImage (const adk::PanelAuditImage& image) noexcept
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        adk::EscapeConsoleUpdate input = emptyUpdate (200);
        input.auditImagePresent        = true;
        input.auditImage               = image;
        assert (console.update (input).ok ());

        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        return console.panelSnapshot ().auditDisposition;
    }

    void testSealedAuditSequenceForms ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());
        const adk::PanelAuditImage first =
            commitSolved (console, prepareSolved (console, 1101, 11), 12);

        adk::EscapeConsoleUpdate refreshed = solvedUpdate (20, 2);
        refreshed.auditImagePresent        = true;
        refreshed.auditImage               = first;
        assert (console.update (refreshed).ok ());
        const adk::PanelAuditImage pair =
            commitSolved (console, prepareSolved (console, 1102, 21), 22);
        assert (pair.slots[0].state == adk::PanelAuditSlotState::Committed);
        assert (pair.slots[1].state == adk::PanelAuditSlotState::Committed);

        adk::PanelAuditImage gap    = pair;
        gap.slots[1].recordSequence = gap.slots[0].recordSequence + 2U;
        sealRecord (gap.slots[1]);

        assert (classifyRestartImage (gap) ==
                adk::PanelAuditDisposition::Indeterminate);

        adk::PanelAuditImage half = pair;
        half.slots[1].recordSequence =
            half.slots[0].recordSequence + UINT32_C (0x80000000);
        sealRecord (half.slots[1]);

        assert (classifyRestartImage (half) ==
                adk::PanelAuditDisposition::Indeterminate);

        adk::PanelAuditImage rollover    = pair;
        rollover.slots[0].recordSequence = UINT32_MAX;
        rollover.slots[1].recordSequence = 1;
        sealRecord (rollover.slots[0]);
        sealRecord (rollover.slots[1]);

        assert (classifyRestartImage (rollover) == adk::PanelAuditDisposition::Ready);
    }

    void testPresentationFailureCannotChangeAuditOrRelease ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        assert (console.update (solvedUpdate (10, 1)).ok ());

        const adk::PanelAuditImage          before = console.canonicalAuditImage ();
        const adk::EscapeConsoleDisposition beforeDisposition =
            console.snapshot ().disposition;

        adk::EscapeConsoleUpdate failed = emptyUpdate (11);
        failed.presentationPresent      = true;
        failed.presentation = {console.panelSnapshot ().generation,
                               adk::MicrosecondTimePoint (11),

                               adk::Status (adk::StatusCode::HardwareFailure)};
        assert (console.update (failed).ok ());

        assert (console.snapshot ().latchIntent == adk::EscapeLatchIntent::Inactive);

        assert (console.snapshot ().disposition == beforeDisposition);

        const adk::PanelAuditImage after = console.canonicalAuditImage ();

        assert (std::memcmp (&before, &after, sizeof before) == 0);
    }
#endif

#if ADK_ESCAPE_CONSOLE_TEST_PART == 4
    void testTwoConsoleReplayIsFieldwiseStable ()
    {
        adk::InertEscapeConsole first (config ());

        adk::InertEscapeConsole second (config ());

        assert (first.initialize ().ok ());

        assert (second.initialize ().ok ());

        const uint16_t masks[] = {UINT16_C (0x003), UINT16_C (0x0cf),
                                  UINT16_C (0x0fff)};
        for (uint8_t index = 0; index < 3; ++index)
        {
            const adk::EscapeConsoleUpdate input =
                clueMaskUpdate (masks[index], static_cast<uint32_t> (10U + index),
                                static_cast<uint32_t> (1U + index));
            assert (first.update (input).ok ());

            assert (second.update (input).ok ());

            assert (sameSnapshot (first.snapshot (), second.snapshot ()));

            const adk::PanelAuditImage firstImage = first.canonicalAuditImage ();

            const adk::PanelAuditImage secondImage = second.canonicalAuditImage ();

            assert (std::memcmp (&firstImage, &secondImage, sizeof firstImage) == 0);
        }
    }

    void
    assertMalformedEnvelopeRejected (adk::InertEscapeConsole&        console,
                                     const adk::EscapeConsoleUpdate& malformed) noexcept
    {
        const adk::EscapeConsoleSnapshot before = console.snapshot ();

        const adk::PanelAuditImage beforeImage = console.canonicalAuditImage ();

        assert (console.update (malformed).error () ==
                adk::StatusCode::InvalidArgument);
        assert (sameSnapshot (before, console.snapshot ()));

        const adk::PanelAuditImage afterImage = console.canonicalAuditImage ();

        assert (std::memcmp (&beforeImage, &afterImage, sizeof beforeImage) == 0);
    }

    void testEveryAbsentPayloadMustBeCanonical ()
    {
        adk::InertEscapeConsole console (config ());

        assert (console.initialize ().ok ());

        adk::EscapeConsoleUpdate malformed        = emptyUpdate (1);
        malformed.auditImage.slots[0].operationId = 1;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed                            = emptyUpdate (1);
        malformed.clueUpdate.observationMask = 1;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed               = emptyUpdate (1);
        malformed.stop.asserted = true;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed                     = emptyUpdate (1);
        malformed.control.pressedMask = 1;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed                              = emptyUpdate (1);
        malformed.auditAcknowledge.operationId = 1;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed                         = emptyUpdate (1);
        malformed.acknowledge.operationId = 1;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed                               = emptyUpdate (1);
        malformed.presentation.intentGeneration = 1;
        assertMalformedEnvelopeRejected (console, malformed);

        malformed                          = emptyUpdate (1);
        malformed.solvePreview.operationId = 1;
        assertMalformedEnvelopeRejected (console, malformed);
    }

    void testForcedGenerationAndLifecycleExhaustion ()
    {
        {
            adk::InertEscapeConsole console (config ());

            adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console,
                                                                    UINT32_MAX);
            assert (console.initialize ().error () ==
                    adk::StatusCode::CapacityExceeded);
            assert (!console.initialized ());

            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
        }
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            assert (console.update (solvedUpdate (10, 1)).ok ());

            const adk::EscapeConsolePreview preview = prepareSolved (console, 1201, 11);

            adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console,
                                                                    UINT32_MAX);
            console.reset ();

            assert (!console.canCommit (preview));

            assert (console.snapshot ().status.error () ==
                    adk::StatusCode::CapacityExceeded);
            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
            assert (!console.prepareSolve (1202, adk::MicrosecondTimePoint (12)).ok ());
        }
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            const adk::EscapeConsoleSnapshot before = console.snapshot ();

            adk::InertEscapeConsoleTestAccess::snapshotGeneration (console, UINT32_MAX);

            const adk::Status status = console.update (emptyUpdate (1));

            assert (status.error () == adk::StatusCode::CapacityExceeded);

            assert (console.snapshot ().generation == UINT32_MAX);

            assert (console.snapshot ().latchIntent == before.latchIntent);
        }
        {
            adk::InertEscapeConsole console (config ());

            assert (console.initialize ().ok ());

            assert (console.update (solvedUpdate (10, 1)).ok ());

            const adk::EscapeConsolePreview preview = prepareSolved (console, 1203, 11);

            adk::InertEscapeConsoleTestAccess::lifecycleGeneration (console,
                                                                    UINT32_MAX);
            console.shutdown ();

            assert (!console.initialized ());

            assert (!console.canCommit (preview));

            assert (console.snapshot ().status.error () ==
                    adk::StatusCode::CapacityExceeded);
            assert (console.snapshot ().latchIntent ==
                    adk::EscapeLatchIntent::Inactive);
            assert (console.initialize ().error () ==
                    adk::StatusCode::CapacityExceeded);
        }
    }
#endif

} // namespace

int main ()
{
#if ADK_ESCAPE_CONSOLE_TEST_PART == 1
    testConfigurationAndLifecycle ();

    testResetBeforeInitializationCoordinatesLifecycle ();

    testResetBeforeInitializationCanExhaust ();

    testSolvedEvidenceAndFamilies ();

    testEveryCluePresenceMask ();

    testFamilyAndDigestConfigurationMutations ();

    testPackedFamilyMappingsAcrossLifecycle ();

#elif ADK_ESCAPE_CONSOLE_TEST_PART == 2

    testStopDominatesCollision ();

    testEvidencePrecedenceAndBoundaries ();

    testEveryOperatorMaskWithStopDominance ();

    testPreviewOwnershipAndInvalidation ();

    testPreparedAuditAndCommit ();

    testInvalidChordPreventsSolveCommit ();
#elif ADK_ESCAPE_CONSOLE_TEST_PART == 3

    testSolvePairCannotCommitUnderProposedChildChanges ();

    testAuditImagesAcrossRestart ();

    testSolveAdmissionRequiresQualifiedReleasedStop ();

    testShutdownReinitializeRejectsStalePreview ();
#elif ADK_ESCAPE_CONSOLE_TEST_PART == 4

    testStructuralRejectionIsAtomic ();

    testTwoConsoleReplayIsFieldwiseStable ();

    testEveryAbsentPayloadMustBeCanonical ();

    testForcedGenerationAndLifecycleExhaustion ();
#elif ADK_ESCAPE_CONSOLE_TEST_PART == 5

    testStopAuditRestartAndQualifiedRelease ();

    testSealedAuditSequenceForms ();

    testPresentationFailureCannotChangeAuditOrRelease ();
#else
#error "ADK_ESCAPE_CONSOLE_TEST_PART must be 1, 2, 3, 4, or 5"
#endif
}
