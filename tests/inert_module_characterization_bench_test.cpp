#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "inert_module_characterization_bench.h"
#include "module_characterization_digest.h"

namespace {

    uint32_t crc32 (const char* text)
    {
        uint32_t value = UINT32_C (0xffffffff);
        while (*text != '\0')
        {
            value ^= static_cast<uint8_t> (*text);
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                value =
                    (value >> 1U) ^ ((value & 1U) != 0U ? UINT32_C (0xedb88320) : 0U);
            }
            ++text;
        }
        return value ^ UINT32_C (0xffffffff);
    }

    uint32_t crc32 (const uint8_t* bytes, uint16_t length)
    {
        uint32_t value = UINT32_C (0xffffffff);
        for (uint16_t index = 0; index < length; ++index)
        {
            value ^= bytes[index];
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                value =
                    (value >> 1U) ^ ((value & 1U) != 0U ? UINT32_C (0xedb88320) : 0U);
            }
        }
        return value ^ UINT32_C (0xffffffff);
    }

    struct CanonicalBytes
    {
        uint8_t  bytes[512];
        uint16_t length;

        CanonicalBytes () : bytes{}, length (0)
        {
        }

        void byte (uint8_t value)
        {
            bytes[length++] = value;
        }

        void word (uint16_t value)
        {
            byte (static_cast<uint8_t> (value));
            byte (static_cast<uint8_t> (value >> 8U));
        }

        void doubleWord (uint32_t value)
        {
            byte (static_cast<uint8_t> (value));
            byte (static_cast<uint8_t> (value >> 8U));
            byte (static_cast<uint8_t> (value >> 16U));
            byte (static_cast<uint8_t> (value >> 24U));
        }

        void domain (const char* value)
        {
            while (*value != '\0')
            {
                byte (static_cast<uint8_t> (*value++));
            }
        }
    };

    void appendDuration (CanonicalBytes&                    bytes,
                         const adk::ModuleDeclaredDuration& duration)
    {
        bytes.byte       (static_cast<uint8_t> (duration.declaration));
        bytes.doubleWord (duration.value.milliseconds ());
    }

    void appendDescriptor (CanonicalBytes&                       bytes,
                           const adk::ModuleThresholdDescriptor& value)
    {
        bytes.word       (value.schemaRevision);
        bytes.doubleWord (value.descriptorId);
        bytes.word       (value.descriptorRevision);
        bytes.doubleWord (value.declaredSpecimenReference);
        bytes.word       (value.declaredSpecimenRevision);
        bytes.word       (value.declaredElectricalEvidenceRevision);
        bytes.byte       (static_cast<uint8_t> (value.channelTopology));
        bytes.byte       (static_cast<uint8_t> (value.comparatorOutputStage));
        bytes.byte       (static_cast<uint8_t> (value.pullRequirement));
        bytes.byte       (static_cast<uint8_t> (value.declaredPullRail));
        bytes.word       (value.declaredSupplyMillivolts.minimum);
        bytes.word       (value.declaredSupplyMillivolts.maximum);
        bytes.word       (value.declaredSignalMillivolts.minimum);
        bytes.word       (value.declaredSignalMillivolts.maximum);
        bytes.word       (value.rawDomain.minimum);
        bytes.word       (value.rawDomain.maximum);
        bytes.byte       (static_cast<uint8_t> (value.comparatorPolarity));
        bytes.byte       (static_cast<uint8_t> (value.thresholdControlKind));
        bytes.byte       (static_cast<uint8_t> (value.thresholdDirection));
        appendDuration   (bytes, value.warmup);
        appendDuration   (bytes, value.settling);
    }

    void appendFrame (CanonicalBytes& bytes, const adk::ModuleThresholdFrame& value)
    {
        bytes.word       (value.schemaRevision);
        bytes.doubleWord (value.descriptorId);
        bytes.word       (value.descriptorRevision);
        bytes.doubleWord (value.declaredSpecimenReference);
        bytes.word       (value.declaredSpecimenRevision);
        bytes.word       (value.declaredElectricalEvidenceRevision);
        bytes.byte       (value.provenance.sourceId);
        bytes.word       (value.provenance.sourceConfigurationRevision);
        bytes.doubleWord (value.provenance.sequence);
        bytes.doubleWord (value.provenance.observedAt.milliseconds ());
        bytes.word       (value.analogRaw);
        bytes.byte       (static_cast<uint8_t> (value.analogStatus));
        bytes.byte       (value.comparatorLevelHigh ? 1U : 0U);
        bytes.byte       (static_cast<uint8_t> (value.comparatorStatus));
        bytes.byte       (value.comparatorPresent ? 1U : 0U);
        bytes.byte       (value.comparatorAsserted ? 1U : 0U);
        bytes.byte       (value.declaredWarmupSatisfied ? 1U : 0U);
        bytes.byte       (value.declaredSettlingSatisfied ? 1U : 0U);
        bytes.byte       (static_cast<uint8_t> (value.analogProducerStatus.error ()));
        bytes.byte       (static_cast<uint8_t> (value.comparatorProducerStatus.error ()));
    }

    void appendPoint (CanonicalBytes&                         bytes,
                      const adk::ModuleCharacterizationPoint& value)
    {
        bytes.doubleWord (value.sessionId);
        bytes.doubleWord (value.runId);
        bytes.doubleWord (value.legId);
        bytes.word       (value.controlOrdinal);
        bytes.byte       (static_cast<uint8_t> (value.leg));
        bytes.byte       (static_cast<uint8_t> (value.direction));
        bytes.byte       (value.sourceId);
        bytes.word       (value.sourceConfigurationRevision);
        appendFrame      (bytes, value.frame);
    }

    void appendZeroPoint (CanonicalBytes& bytes)
    {
        for (uint8_t index = 0; index < 53; ++index)
        {
            bytes.byte (0);
        }
    }

    void appendBracket (CanonicalBytes&                     bytes,
                        const adk::ModuleTransitionBracket& value)
    {
        bytes.byte (value.present ? 1U : 0U);
        if (value.present)
        {
            appendPoint (bytes, value.before);
            appendPoint (bytes, value.after);
        }
        else
        {
            appendZeroPoint (bytes);
            appendZeroPoint (bytes);
        }
    }

    void appendInterval (CanonicalBytes& bytes, const adk::ModuleAnalogInterval& value)
    {
        bytes.byte (value.present ? 1U : 0U);
        bytes.word (value.present ? value.lower : 0U);
        bytes.word (value.present ? value.upper : 0U);
    }

    void appendWitness (CanonicalBytes& bytes, const adk::ModuleCompactWitness& value)
    {
        bytes.byte       (value.present ? 1U : 0U);
        bytes.word       (value.present ? value.controlOrdinal : 0U);
        bytes.word       (value.present ? value.analogRaw : 0U);
        bytes.byte       (value.present && value.comparatorAsserted ? 1U : 0U);
        bytes.doubleWord (value.present ? value.sequence : 0U);
        bytes.doubleWord (value.present ? value.observedAt.milliseconds () : 0U);
    }

    CanonicalBytes descriptorBytes (const adk::ModuleThresholdDescriptor& value)
    {
        CanonicalBytes bytes;
        bytes.domain     ("ADK-MOD-DESC-1");
        appendDescriptor (bytes, value);
        return bytes;
    }

    CanonicalBytes evidenceBytes (const adk::ModuleCharacterizationEvidence& value)
    {
        CanonicalBytes bytes;
        bytes.domain     ("ADK-MOD-EVID-1");
        bytes.doubleWord (value.lifecycleGeneration);
        bytes.doubleWord (value.sessionId);
        bytes.doubleWord (value.runId);
        bytes.doubleWord (value.legId);
        bytes.word       (value.characterizationRevision);
        appendDescriptor (bytes, value.descriptor);
        bytes.byte       (value.sourceId);
        bytes.word       (value.sourceConfigurationRevision);
        bytes.byte       (static_cast<uint8_t> (value.state));
        bytes.byte       (static_cast<uint8_t> (value.reason));
        bytes.byte       (static_cast<uint8_t> (value.terminalLeg));
        bytes.byte       (value.ascendingCount);
        bytes.byte       (value.descendingCount);
        bytes.byte       (value.verificationCount);
        appendBracket    (bytes, value.ascendingBracket);
        appendBracket    (bytes, value.descendingBracket);
        appendInterval   (bytes, value.guaranteedInactiveInterval);
        appendInterval   (bytes, value.guaranteedActiveInterval);
        appendInterval   (bytes, value.ambiguityInterval);
        bytes.byte       (static_cast<uint8_t> (value.relation));
        appendWitness    (bytes, value.firstWitness);
        appendWitness    (bytes, value.lastWitness);
        appendWitness    (bytes, value.offendingBefore);
        appendWitness    (bytes, value.offendingAfter);
        bytes.byte       (static_cast<uint8_t> (value.status.error ()));
        return bytes;
    }

    CanonicalBytes witnessBytes (const adk::ModuleCompactWitness& value)
    {
        CanonicalBytes bytes;
        bytes.domain  ("ADK-MOD-WIT-1");
        appendWitness (bytes, value);
        return bytes;
    }

    template <uint16_t Size>
    void assertCanonicalVector (const CanonicalBytes& actual,
                                const uint8_t (&expected)[Size])
    {
        assert (actual.length == Size);
        assert (memcmp (actual.bytes, expected, Size) == 0);
    }

    uint16_t crc16 (const uint8_t* bytes, uint16_t length)
    {
        uint16_t value = 0xffff;
        for (uint16_t index = 0; index < length; ++index)
        {
            value = static_cast<uint16_t> (
                value ^
                static_cast<uint16_t> (static_cast<uint16_t> (bytes[index]) << 8U));
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                value = (value & 0x8000U) != 0U
                            ? static_cast<uint16_t> ((value << 1U) ^ 0x1021U)
                            : static_cast<uint16_t> (value << 1U);
            }
        }
        return value;
    }

    void repairIntegrity (uint8_t* bytes)
    {
        const uint16_t value = crc16 (bytes, UINT16_C (190));
        bytes[190]           = static_cast<uint8_t> (value);
        bytes[191]           = static_cast<uint8_t> (value >> 8U);
    }

    void write32 (uint8_t* bytes, uint32_t value)
    {
        bytes[0] = static_cast<uint8_t> (value);
        bytes[1] = static_cast<uint8_t> (value >> 8U);
        bytes[2] = static_cast<uint8_t> (value >> 16U);
        bytes[3] = static_cast<uint8_t> (value >> 24U);
    }

    void assertRecordsEqual (const adk::ModuleCharacterizationRecord& left,
                             const adk::ModuleCharacterizationRecord& right)
    {
        adk::ModuleCharacterizationRecordCodec codec;
        uint8_t                                leftImage[192]  = {};
        uint8_t                                rightImage[192] = {};

        assert (codec.encode (left, {leftImage, sizeof leftImage}).ok ());
        assert (codec.encode (right, {rightImage, sizeof rightImage}).ok ());
        assert (memcmp (leftImage, rightImage, sizeof leftImage) == 0);
    }

    void assertResultsEqual (const adk::ModuleBenchResult& left,
                             const adk::ModuleBenchResult& right)
    {
        assert (left.lifecycleGeneration == right.lifecycleGeneration);
        assert (left.sessionId == right.sessionId);
        assert (left.state == right.state);
        assert (left.step == right.step);
        assert (left.runId == right.runId);
        assert (left.descriptorDigest == right.descriptorDigest);
        assert (left.evidenceDigest == right.evidenceDigest);
        assert (left.relation == right.relation);
        assert (left.presentation.step == right.presentation.step);
        assert (left.presentation.state == right.presentation.state);
        assert (left.presentation.faultDominant == right.presentation.faultDominant);
        assert (left.presentation.relation == right.presentation.relation);
        assert (left.recordPrepared == right.recordPrepared);
        assert (left.status.error () == right.status.error ());
    }

    adk::ModuleThresholdDescriptor descriptor ()
    {
        return {1,
                0x12345678UL,
                2,
                0x87654321UL,
                3,
                4,
                adk::ModuleChannelTopology::AnalogAndComparator,
                adk::ModuleComparatorOutputStage::OpenDrain,
                adk::ModulePullRequirement::PullUp,
                adk::ModuleDeclaredRail::LogicSupply,
                {3300, 5000},
                {0, 3300},
                {0, 1023},
                adk::ModuleComparatorPolarity::ActiveLow,
                adk::ModuleThresholdControlKind::Potentiometer,
                adk::ModuleThresholdDirection::IncreasingClockwise,
                {adk::ModuleDurationDeclaration::Known, adk::Duration (0)},
                {adk::ModuleDurationDeclaration::Known, adk::Duration (0)}};
    }

    adk::ModuleThresholdFrame frame (uint32_t sequence, uint32_t observedAt,
                                     uint16_t raw, bool asserted)
    {
        const adk::ModuleThresholdDescriptor value = descriptor ();
        return {value.schemaRevision,
                value.descriptorId,
                value.descriptorRevision,
                value.declaredSpecimenReference,
                value.declaredSpecimenRevision,
                value.declaredElectricalEvidenceRevision,
                {7, 8, sequence, adk::TimePoint (observedAt)},
                raw,
                adk::ModuleChannelStatus::Current,
                !asserted,
                adk::ModuleChannelStatus::Current,
                true,
                asserted,
                true,
                true,
                adk::StatusCode::Ok,
                adk::StatusCode::Ok};
    }

    adk::ModuleCharacterizationPoint point (uint32_t legId, uint16_t ordinal,
                                            adk::ModuleCharacterizationLeg leg,
                                            adk::ModuleSweepDirection      direction,
                                            uint32_t sequence, uint32_t observedAt,
                                            uint16_t raw, bool asserted)
    {
        return {11,      12,  legId,
                ordinal, leg, direction,
                7,       8,   frame (sequence, observedAt, raw, asserted)};
    }

    adk::ModuleCharacterizationEvidence completeEvidence ()
    {
        adk::ModuleCharacterizationConfig config = {
            5, descriptor (), 3, adk::Duration (20), adk::Duration (10)};
        adk::ModuleCharacterizationPolicy policy (config);

        assert (policy.initialize (adk::TimePoint (0)).ok ());
        assert (policy.beginSession (adk::TimePoint (0), 11, 12).ok ());

        assert (policy
                    .beginLeg (adk::TimePoint (0), 21,
                               adk::ModuleCharacterizationLeg::Ascending,
                               adk::ModuleSweepDirection::Increasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 0,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 101, 2, 400,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (3),
                              point (21, 3, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 102, 3,
                                     1023, true))
                    .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (3)).ok ());
        assert (policy
                    .beginLeg (adk::TimePoint (4), 22,
                               adk::ModuleCharacterizationLeg::Descending,
                               adk::ModuleSweepDirection::Decreasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (4),
                              point (22, 1, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 103, 4,
                                     1023, true))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (5),
                              point (22, 2, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 104, 5, 500,
                                     true))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (6),
                              point (22, 3, adk::ModuleCharacterizationLeg::Descending,
                                     adk::ModuleSweepDirection::Decreasing, 105, 6, 0,
                                     false))
                    .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (6)).ok ());
        assert (policy
                    .beginLeg (adk::TimePoint (7), 23,
                               adk::ModuleCharacterizationLeg::Verification,
                               adk::ModuleSweepDirection::Unordered)
                    .ok ());
        assert (
            policy
                .observe (adk::TimePoint (7),
                          point (23, 1, adk::ModuleCharacterizationLeg::Verification,
                                 adk::ModuleSweepDirection::Unordered, 106, 7, 250,
                                 false))
                .ok ());
        assert (policy
                    .observe (
                        adk::TimePoint (8),

                        point (23, 2, adk::ModuleCharacterizationLeg::Verification,
                               adk::ModuleSweepDirection::Unordered, 107, 8, 550, true))
                    .ok ());
        assert (
            policy
                .observe (adk::TimePoint (9),
                          point (23, 3, adk::ModuleCharacterizationLeg::Verification,
                                 adk::ModuleSweepDirection::Unordered, 108, 9, 450,
                                 false))
                .ok ());
        assert (policy.finalizeLeg (adk::TimePoint (9)).ok ());
        adk::ModuleCharacterizationEvidence value;
        assert (policy.evidence (value).ok ());
        return value;
    }

    adk::ModuleCharacterizationEvidence rejectedEvidence ()
    {
        adk::ModuleCharacterizationConfig config = {
            5, descriptor (), 3, adk::Duration (20), adk::Duration (10)};
        adk::ModuleCharacterizationPolicy policy (config);

        assert (policy.initialize (adk::TimePoint (0)).ok ());
        assert (policy.beginSession (adk::TimePoint (0), 11, 12).ok ());
        assert (policy
                    .beginLeg (adk::TimePoint (0), 21,
                               adk::ModuleCharacterizationLeg::Ascending,
                               adk::ModuleSweepDirection::Increasing)
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (1),
                              point (21, 1, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 100, 1, 100,
                                     false))
                    .ok ());
        assert (policy
                    .observe (adk::TimePoint (2),
                              point (21, 2, adk::ModuleCharacterizationLeg::Ascending,
                                     adk::ModuleSweepDirection::Increasing, 102, 2, 200,
                                     false))
                    .ok ());
        adk::ModuleCharacterizationEvidence value;
        assert (policy.evidence (value).ok ());
        assert (value.state == adk::ModuleCharacterizationState::Rejected);
        return value;
    }

    adk::ModuleCharacterizationRecord validRecord ()
    {
        return {1,
                2,
                1,
                3,
                4,
                5,
                0x12345678UL,
                2,
                0x87654321UL,
                3,
                4,
                adk::ModuleChannelTopology::AnalogAndComparator,
                adk::ModuleComparatorOutputStage::OpenDrain,
                adk::ModulePullRequirement::PullUp,
                adk::ModuleDeclaredRail::LogicSupply,
                adk::ModuleComparatorPolarity::ActiveLow,
                adk::ModuleThresholdControlKind::Potentiometer,
                adk::ModuleThresholdDirection::IncreasingClockwise,
                {3300, 5000},
                {0, 3300},
                {0, 1023},
                {adk::ModuleDurationDeclaration::Known, adk::Duration (0)},
                {adk::ModuleDurationDeclaration::Known, adk::Duration (0)},
                11,
                12,
                5,
                7,
                8,
                3,
                3,
                3,
                {true, 400, 1023, false, true, 101, 102},
                {true, 500, 0, true, false, 104, 105},
                {true, 0, 0},
                {true, 1023, 1023},
                {true, 1, 1022},
                adk::ModuleComparatorRelation::Consistent,
                0x11111111UL,
                0x22222222UL,
                0,
                0,
                100,
                108,
                0x33333333UL,
                0x44444444UL,
                adk::ModuleCharacterizationState::Complete,
                adk::ModuleCharacterizationReason::None,
                adk::StatusCode::Ok,
                adk::ModuleBenchScriptStep::PrepareRecord};
    }

    adk::ModuleCharacterizationEnvelope envelope ()
    {
        adk::ModuleCharacterizationEnvelope value;
        value.envelopeRevision = 3;
        value.evidence         = completeEvidence ();
        value.descriptorDigest =
            adk::moduleThresholdDescriptorDigest (value.evidence.descriptor);
        value.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (value.evidence);
        return value;
    }

    adk::ModuleBenchConfig
    benchConfig (const adk::ModuleCharacterizationEnvelope& value)
    {
        return {2,
                value.envelopeRevision,
                1,
                value.evidence.descriptor.descriptorId,
                value.evidence.descriptor.descriptorRevision,
                value.evidence.descriptor.schemaRevision,
                value.evidence.descriptor.declaredSpecimenRevision,
                value.evidence.descriptor.declaredElectricalEvidenceRevision,
                value.descriptorDigest,
                9,
                10,
                adk::Duration (20)};
    }

    adk::ModuleBenchControl
    control (uint32_t sequence, uint32_t observedAt,
             adk::ModuleBenchCommand command = adk::ModuleBenchCommand::Advance)
    {
        return {9,
                10,
                11,
                sequence,
                adk::TimePoint (observedAt),
                command,
                adk::StatusCode::Ok};
    }

    void assertAdmissionRejected (const adk::ModuleCharacterizationEnvelope& candidate)
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::InertModuleCharacterizationBench bench (benchConfig (admitted));
        adk::ModuleBenchResult                before;
        adk::ModuleBenchResult                after;

        assert (bench.initialize (adk::TimePoint (0)).ok ());
        assert (bench.result (before).ok ());
        assert (bench.beginSession (adk::TimePoint (0), 11, candidate).error () ==
                adk::StatusCode::InvalidArgument);
        assert             (bench.result (after).ok ());
        assertResultsEqual (after, before);
    }

    void assertEvidenceRejected (adk::ModuleCharacterizationEnvelope candidate)
    {
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);
    }

    void testCodecWireAndAtomicity ()
    {
        adk::ModuleCharacterizationRecordCodec codec;
        adk::ModuleCharacterizationRecord      record = validRecord ();
        struct Guarded
        {
            uint8_t before;
            uint8_t bytes[192];
            uint8_t after;
        } guarded = {0xa5, {}, 0x5a};

        assert (codec.encode (record, {guarded.bytes, sizeof guarded.bytes}).value () ==
                192);
        assert (guarded.before == 0xa5 && guarded.after == 0x5a);
        assert (guarded.bytes[0] == 'A' && guarded.bytes[1] == 'D' &&
                guarded.bytes[2] == 'M' && guarded.bytes[3] == 'C');
        assert (guarded.bytes[4] == 1);
        assert (guarded.bytes[5] == 192 && guarded.bytes[6] == 0);
        assert (guarded.bytes[190] == 0x9f && guarded.bytes[191] == 0xc0);
        const uint8_t golden[192] = {
            0x41, 0x44, 0x4d, 0x43, 0x01, 0xc0, 0x00, 0x01, 0x00, 0x02, 0x00, 0x04,
            0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12, 0x02,
            0x00, 0x21, 0x43, 0x65, 0x87, 0x03, 0x00, 0x04, 0x00, 0x02, 0x02, 0x02,
            0x02, 0x02, 0x02, 0x01, 0xe4, 0x0c, 0x88, 0x13, 0x00, 0x00, 0xe4, 0x0c,
            0x00, 0x00, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x05, 0x00,
            0x03, 0x03, 0x03, 0x01, 0x90, 0x01, 0xff, 0x03, 0x00, 0x01, 0x65, 0x00,
            0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x01, 0xf4, 0x01, 0x00, 0x00, 0x01,
            0x00, 0x68, 0x00, 0x00, 0x00, 0x69, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x01, 0xff, 0x03, 0xff, 0x03, 0x01, 0x01, 0x00, 0xfe, 0x03,
            0x01, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x00,
            0x00, 0x33, 0x33, 0x33, 0x33, 0x44, 0x44, 0x44, 0x44, 0x02, 0x00, 0x00,
            0x04, 0x01, 0x00, 0x03, 0x00, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9f, 0xc0};
        assert (memcmp (guarded.bytes, golden, sizeof golden) == 0);
        for (uint16_t index = 164; index < 190; ++index)
        {
            assert (guarded.bytes[index] == 0);
        }

        adk::ModuleCharacterizationRecord decoded;
        assert (codec.decode ({guarded.bytes, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::Valid);
        assert (decoded.sessionId == record.sessionId);
        assert (decoded.descriptorId == record.descriptorId);
        assert (decoded.evidenceDigest == record.evidenceDigest);

        const adk::ModuleCharacterizationRecord sentinel = decoded;
        for (uint16_t index = 0; index < 192; ++index)
        {
            uint8_t corrupt[192];
            for (uint16_t copy = 0; copy < 192; ++copy)
            {
                corrupt[copy] = guarded.bytes[copy];
            }
            corrupt[index] ^= 0x01;
            adk::ModuleCharacterizationRecord unchanged = sentinel;
            assert (codec.decode ({corrupt, 192}, unchanged) !=
                    adk::ModuleCharacterizationRecordValidity::Valid);
            assertRecordsEqual (unchanged, sentinel);
        }

        uint8_t tooSmall[191] = {};
        tooSmall[0]           = 0xcc;
        assert (codec.encode (record, {tooSmall, sizeof tooSmall}).status ().error () ==
                adk::StatusCode::CapacityExceeded);
        assert (tooSmall[0] == 0xcc);
        assert (codec.decode ({guarded.bytes, 191}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadLength);
        assert (codec.decode ({guarded.bytes, 0}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadLength);
        assert (codec.decode ({nullptr, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadSemanticValue);

        uint8_t semantic[192];
        for (uint16_t index = 0; index < 192; ++index)
        {
            semantic[index] = guarded.bytes[index];
        }
        semantic[164] = 1;
        repairIntegrity (semantic);

        assert (codec.decode ({semantic, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadSemanticValue);
        semantic[0] = 0;
        assert (codec.decode ({semantic, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadFraming);
        semantic[0] = 'A';
        semantic[1] ^= 1;
        assert (codec.decode ({semantic, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadFraming);

        uint8_t framing[192];
        memcpy (framing, golden, sizeof framing);
        ++framing[4];
        repairIntegrity (framing);
        assert          (codec.decode ({framing, sizeof framing}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadFraming);
        memcpy (framing, golden, sizeof framing);
        framing[5] = 191;
        framing[6] = 0;
        repairIntegrity (framing);
        assert          (codec.decode ({framing, sizeof framing}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadFraming);

        uint8_t integrity[192];
        memcpy (integrity, golden, sizeof integrity);
        integrity[100] ^= 1;
        assert (codec.decode ({integrity, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadIntegrity);

        uint8_t typedSemantic[192];
        memcpy (typedSemantic, golden, sizeof typedSemantic);
        typedSemantic[33] = 0xff;
        repairIntegrity (typedSemantic);

        assert (codec.decode ({typedSemantic, 192}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::BadSemanticValue);

        uint8_t oversized[193] = {};
        oversized[192]         = 0x5a;
        assert (codec.encode (record, {oversized, sizeof oversized}).ok ());
        assert (oversized[192] == 0x5a);

        adk::ModuleCharacterizationRecord invalid = record;
        invalid.ascendingCount                    = 17;
        tooSmall[0]                               = 0xcc;
        assert (
            codec.encode (invalid, {tooSmall, sizeof tooSmall}).status ().error () ==
            adk::StatusCode::InvalidArgument);
        assert (tooSmall[0] == 0xcc);

        record.firstWitnessDigest = 0;
        record.lastWitnessDigest  = 0;
        record.descriptorDigest   = 0;
        record.evidenceDigest     = 0;
        const adk::Result<uint16_t> zeroDigestEncode =
            codec.encode (record, {guarded.bytes, 192});

        assert (zeroDigestEncode.ok ());
        assert (guarded.bytes[121] == 0 && guarded.bytes[125] == 0);
        assert (guarded.bytes[145] == 0 && guarded.bytes[149] == 0);

        adk::ModuleCharacterizationRecord zeroDigestDecoded = validRecord ();

        assert (codec.decode ({guarded.bytes, 192}, zeroDigestDecoded) ==
                adk::ModuleCharacterizationRecordValidity::Valid);
        assert (zeroDigestDecoded.firstWitnessDigest == 0);
        assert (zeroDigestDecoded.lastWitnessDigest == 0);
        assert (zeroDigestDecoded.descriptorDigest == 0);
        assert (zeroDigestDecoded.evidenceDigest == 0);
    }

    void testIntegerExtremaAndCanonicalAbsence ()
    {
        adk::ModuleCharacterizationRecordCodec codec;
        adk::ModuleCharacterizationRecord      extreme    = validRecord ();
        uint8_t                                image[192] = {};

        extreme.recordSchemaRevision                = UINT16_MAX;
        extreme.benchRevision                       = UINT16_MAX;
        extreme.descriptorSchemaRevision            = UINT16_MAX;
        extreme.envelopeRevision                    = UINT16_MAX;
        extreme.lifecycleGeneration                 = UINT32_MAX;
        extreme.sessionId                           = UINT32_MAX;
        extreme.descriptorId                        = UINT32_MAX;
        extreme.descriptorRevision                  = UINT16_MAX;
        extreme.declaredSpecimenReference           = UINT32_MAX;
        extreme.declaredSpecimenRevision            = UINT16_MAX;
        extreme.declaredElectricalEvidenceRevision  = UINT16_MAX;
        extreme.runId                               = UINT32_MAX;
        extreme.characterizationLifecycleGeneration = UINT32_MAX;
        extreme.characterizationRevision            = UINT16_MAX;
        extreme.sourceId                            = UINT8_MAX;
        extreme.sourceConfigurationRevision         = UINT16_MAX;
        extreme.warmup.value                        = adk::Duration (UINT32_MAX);
        extreme.settling.value                      = adk::Duration (UINT32_MAX);
        extreme.ascendingBracket.beforeSequence     = UINT32_MAX - 1U;
        extreme.ascendingBracket.afterSequence      = UINT32_MAX;
        extreme.descendingBracket.beforeSequence    = UINT32_MAX - 1U;
        extreme.descendingBracket.afterSequence     = UINT32_MAX;
        extreme.firstWitnessDigest                  = UINT32_MAX;
        extreme.lastWitnessDigest                   = UINT32_MAX;
        extreme.offendingBeforeDigest               = UINT32_MAX;
        extreme.offendingAfterDigest                = UINT32_MAX;
        extreme.firstSequence                       = UINT32_MAX - 1U;
        extreme.lastSequence                        = UINT32_MAX;
        extreme.descriptorDigest                    = UINT32_MAX;
        extreme.evidenceDigest                      = UINT32_MAX;

        const adk::Result<uint16_t> extremeEncode =
            codec.encode (extreme, {image, sizeof image});

        assert (extremeEncode.ok ());

        adk::ModuleCharacterizationRecord decoded = validRecord ();

        assert (codec.decode ({image, sizeof image}, decoded) ==
                adk::ModuleCharacterizationRecordValidity::Valid);
        assertRecordsEqual (decoded, extreme);

        adk::ModuleCharacterizationRecord absent = validRecord ();
        absent.terminalState = adk::ModuleCharacterizationState::Rejected;
        absent.terminalReason =
            adk::ModuleCharacterizationReason::NoObservedTransitionActive;
        absent.ascendingCount             = 0;
        absent.descendingCount            = 0;
        absent.verificationCount          = 0;
        absent.ascendingBracket           = {false, 0, 0, false, false, 0, 0};
        absent.descendingBracket          = {false, 0, 0, false, false, 0, 0};
        absent.guaranteedInactiveInterval = {false, 0, 0};
        absent.guaranteedActiveInterval   = {false, 0, 0};
        absent.ambiguityInterval          = {false, 0, 0};
        absent.firstSequence              = 0;
        absent.lastSequence               = 0;
        assert (codec.encode (absent, {image, sizeof image}).ok ());

        const uint16_t absentOffsets[] = {76, 80, 82, 91, 95, 97, 106, 111, 116};
        for (uint8_t mutation = 0;
             mutation < sizeof absentOffsets / sizeof absentOffsets[0]; ++mutation)
        {
            uint8_t noncanonical[192];
            memcpy (noncanonical, image, sizeof noncanonical);
            noncanonical[absentOffsets[mutation]] = 1;
            repairIntegrity (noncanonical);
            decoded = extreme;
            assert (codec.decode ({noncanonical, sizeof noncanonical}, decoded) ==
                    adk::ModuleCharacterizationRecordValidity::BadSemanticValue);
            assertRecordsEqual (decoded, extreme);
        }
    }

    void testDigestGoldenSeedsAndEvidence ()
    {
        const uint8_t descriptorVector[] = {
            0x41, 0x44, 0x4b, 0x2d, 0x4d, 0x4f, 0x44, 0x2d, 0x44, 0x45, 0x53, 0x43,
            0x2d, 0x31, 0x01, 0x00, 0x78, 0x56, 0x34, 0x12, 0x02, 0x00, 0x21, 0x43,
            0x65, 0x87, 0x03, 0x00, 0x04, 0x00, 0x02, 0x02, 0x02, 0x02, 0xe4, 0x0c,
            0x88, 0x13, 0x00, 0x00, 0xe4, 0x0c, 0x00, 0x00, 0xff, 0x03, 0x02, 0x02,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t evidenceVector[] = {
            0x41, 0x44, 0x4b, 0x2d, 0x4d, 0x4f, 0x44, 0x2d, 0x45, 0x56, 0x49, 0x44,
            0x2d, 0x31, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00,
            0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x78, 0x56,
            0x34, 0x12, 0x02, 0x00, 0x21, 0x43, 0x65, 0x87, 0x03, 0x00, 0x04, 0x00,
            0x02, 0x02, 0x02, 0x02, 0xe4, 0x0c, 0x88, 0x13, 0x00, 0x00, 0xe4, 0x0c,
            0x00, 0x00, 0xff, 0x03, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0x00, 0x02, 0x00, 0x02, 0x03,
            0x03, 0x03, 0x01, 0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x15,
            0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x08, 0x00, 0x01, 0x00,
            0x78, 0x56, 0x34, 0x12, 0x02, 0x00, 0x21, 0x43, 0x65, 0x87, 0x03, 0x00,
            0x04, 0x00, 0x07, 0x08, 0x00, 0x65, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
            0x00, 0x90, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00,
            0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x00, 0x07, 0x08, 0x00, 0x01, 0x00, 0x78, 0x56, 0x34,
            0x12, 0x02, 0x00, 0x21, 0x43, 0x65, 0x87, 0x03, 0x00, 0x04, 0x00, 0x07,
            0x08, 0x00, 0x66, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xff, 0x03,
            0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x0b, 0x00,
            0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x02, 0x00,
            0x01, 0x01, 0x07, 0x08, 0x00, 0x01, 0x00, 0x78, 0x56, 0x34, 0x12, 0x02,
            0x00, 0x21, 0x43, 0x65, 0x87, 0x03, 0x00, 0x04, 0x00, 0x07, 0x08, 0x00,
            0x68, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xf4, 0x01, 0x01, 0x00,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x0c,
            0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x01, 0x07,
            0x08, 0x00, 0x01, 0x00, 0x78, 0x56, 0x34, 0x12, 0x02, 0x00, 0x21, 0x43,
            0x65, 0x87, 0x03, 0x00, 0x04, 0x00, 0x07, 0x08, 0x00, 0x69, 0x00, 0x00,
            0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00,
            0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x03,
            0xff, 0x03, 0x01, 0x01, 0x00, 0xfe, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x00, 0xc2, 0x01, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t presentWitnessVector[] = {
            0x41, 0x44, 0x4b, 0x2d, 0x4d, 0x4f, 0x44, 0x2d, 0x57,
            0x49, 0x54, 0x2d, 0x31, 0x01, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
        const uint8_t absentWitnessVector[] = {0x41, 0x44, 0x4b, 0x2d, 0x4d, 0x4f, 0x44,
                                               0x2d, 0x57, 0x49, 0x54, 0x2d, 0x31, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        assert                                                    (crc32 ("ADK-MOD-DESC-1") == UINT32_C (0xcb58c79d));
        assert                                                    (crc32 ("ADK-MOD-EVID-1") == UINT32_C (0x1daf5814));
        assert                                                    (crc32 ("ADK-MOD-WIT-1") == UINT32_C (0xb1b83fae));
        const CanonicalBytes descriptorEncoding = descriptorBytes (descriptor ());
        assertCanonicalVector                                     (descriptorEncoding, descriptorVector);
        assert                                                    (adk::moduleThresholdDescriptorDigest (descriptor ()) ==
                crc32 (descriptorVector, sizeof descriptorVector));
        const adk::ModuleCharacterizationEvidence value = completeEvidence ();

        const CanonicalBytes evidenceEncoding = evidenceBytes (value);
        assertCanonicalVector                                 (evidenceEncoding, evidenceVector);
        assert                                                (adk::moduleCharacterizationEvidenceDigest (value) ==
                crc32 (evidenceVector, sizeof evidenceVector));
        const CanonicalBytes presentWitnessEncoding = witnessBytes (value.firstWitness);
        assertCanonicalVector                                      (presentWitnessEncoding, presentWitnessVector);
        assert                                                     (adk::moduleCompactWitnessDigest (value.firstWitness) ==
                crc32 (presentWitnessVector, sizeof presentWitnessVector));
        const adk::ModuleCompactWitness absent = {false, 0, 0,
                                                  false, 0, adk::TimePoint (0)};
        const CanonicalBytes            absentWitnessEncoding = witnessBytes (absent);
        assertCanonicalVector                                                (absentWitnessEncoding, absentWitnessVector);
        assert                                                               (adk::moduleCompactWitnessDigest (absent) ==
                crc32 (absentWitnessVector, sizeof absentWitnessVector));
    }

    void testCrcValidSemanticBoundaries ()
    {
        adk::ModuleCharacterizationRecordCodec codec;
        adk::ModuleCharacterizationRecord      output;
        uint8_t                                base[192] = {};

        assert (codec.encode (validRecord (), {base, sizeof base}).ok ());
        const uint16_t offsets[] = {33, 72, 75, 164};
        const uint8_t  values[]  = {0xff, 17, 0, 1};
        for (uint8_t mutation = 0; mutation < 4; ++mutation)
        {
            uint8_t bytes[192];
            for (uint16_t index = 0; index < 192; ++index)
            {
                bytes[index] = base[index];
            }
            bytes[offsets[mutation]] = values[mutation];
            repairIntegrity (bytes);

            assert (codec.decode ({bytes, sizeof bytes}, output) ==
                    adk::ModuleCharacterizationRecordValidity::BadSemanticValue);
        }

        uint8_t overlap[192];
        for (uint16_t index = 0; index < 192; ++index)
        {
            overlap[index] = base[index];
        }
        overlap[110] = 1;
        overlap[111] = 0x90;
        overlap[112] = 0x01;
        overlap[113] = 0xff;
        overlap[114] = 0x03;
        repairIntegrity (overlap);

        assert (codec.decode ({overlap, sizeof overlap}, output) ==
                adk::ModuleCharacterizationRecordValidity::BadSemanticValue);

        uint8_t halfRangeSequence[192];
        for (uint16_t index = 0; index < 192; ++index)
        {
            halfRangeSequence[index] = base[index];
        }
        write32 (&halfRangeSequence[82], 1);
        write32 (&halfRangeSequence[86], UINT32_C (0x80000001));

        repairIntegrity (halfRangeSequence);

        assert (codec.decode ({halfRangeSequence, 192}, output) ==
                adk::ModuleCharacterizationRecordValidity::BadSemanticValue);
    }

    void testEnvelopeSemanticAdmission ()
    {
        adk::ModuleCharacterizationEnvelope forged = envelope ();
        ++forged.evidence.ascendingCount;
        forged.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (forged.evidence);
        adk::ModuleBenchConfig config = benchConfig (forged);

        adk::InertModuleCharacterizationBench bench (config);

        assert (bench.initialize (adk::TimePoint (0)).ok ());
        assert (bench.beginSession (adk::TimePoint (0), 11, forged).error () ==
                adk::StatusCode::InvalidArgument);

        adk::ModuleCharacterizationEnvelope rejected = envelope ();

        rejected.evidence = rejectedEvidence ();
        rejected.descriptorDigest =
            adk::moduleThresholdDescriptorDigest (rejected.evidence.descriptor);
        rejected.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (rejected.evidence);
        adk::InertModuleCharacterizationBench faultBench (benchConfig (rejected));

        assert (faultBench.initialize (adk::TimePoint (0)).ok ());
        assert (faultBench.beginSession (adk::TimePoint (0), 11, rejected).ok ());
        adk::ModuleBenchResult result;

        assert (faultBench.result (result).ok ());
        assert (result.state == adk::ModuleBenchState::Fault);
        assert (result.presentation.faultDominant);
        adk::ModuleCharacterizationRecordImage image = {};
        assert (faultBench.prepareRecord (adk::TimePoint (0), image).error () ==
                adk::StatusCode::InvalidArgument);
        assert (faultBench.reset (adk::TimePoint (1)).ok ());
        assert (faultBench.result (result).ok ());
        assert (result.state == adk::ModuleBenchState::Ready);
    }

    void testEnvelopeIdentityAndCorrelationMatrix ()
    {
        adk::ModuleCharacterizationEnvelope candidate = envelope ();

        ++candidate.envelopeRevision;
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.descriptor.descriptorRevision;
        candidate.descriptorDigest =
            adk::moduleThresholdDescriptorDigest (candidate.evidence.descriptor);
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate                = envelope ();
        candidate.evidence.state = adk::ModuleCharacterizationState::Collecting;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.runId;
        candidate.evidence.ascendingBracket.before.runId  = candidate.evidence.runId;
        candidate.evidence.ascendingBracket.after.runId   = candidate.evidence.runId;
        candidate.evidence.descendingBracket.before.runId = candidate.evidence.runId;
        candidate.evidence.descendingBracket.after.runId  = candidate.evidence.runId;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        adk::InertModuleCharacterizationBench changedRunBench (
            benchConfig (envelope ()));
        assert (changedRunBench.initialize (adk::TimePoint (0)).ok ());
        assert (changedRunBench.beginSession (adk::TimePoint (0), 11, candidate).ok ());

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.sessionId;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.runId;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.sourceId;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.sourceConfigurationRevision;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.frame.provenance.sourceId;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.frame.provenance
              .sourceConfigurationRevision;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.frame.descriptorRevision;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.after.legId;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.after.controlOrdinal;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        candidate.evidence.ascendingBracket.before.direction =
            adk::ModuleSweepDirection::Decreasing;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        candidate.evidence.ascendingBracket.before.frame.comparatorAsserted = true;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        candidate.evidence.ascendingBracket.after.frame.analogRaw =
            candidate.evidence.descriptor.rawDomain.minimum;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        candidate.evidence.lastWitness.sequence =
            candidate.evidence.firstWitness.sequence + UINT32_C (0x80000000);
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);

        candidate = envelope ();
        candidate.evidence.guaranteedInactiveInterval.lower =
            candidate.evidence.guaranteedActiveInterval.lower;
        candidate.evidenceDigest =
            adk::moduleCharacterizationEvidenceDigest (candidate.evidence);
        assertAdmissionRejected (candidate);
    }

    void testTerminalAndRejectedCorrelationMatrix ()
    {
        adk::ModuleCharacterizationEnvelope candidate = envelope ();

        ++candidate.evidence.ascendingCount;
        assertEvidenceRejected (candidate);

        candidate                                      = envelope ();
        candidate.evidence.firstWitness.controlOrdinal = 2;
        assertEvidenceRejected (candidate);

        candidate = envelope ();
        --candidate.evidence.lastWitness.controlOrdinal;
        assertEvidenceRejected (candidate);

        candidate = envelope ();
        candidate.evidence.ascendingBracket.after.controlOrdinal =
            static_cast<uint16_t> (candidate.evidence.ascendingCount + 1U);
        assertEvidenceRejected (candidate);

        candidate = envelope ();
        ++candidate.evidence.ascendingBracket.before.frame.provenance.sequence;
        ++candidate.evidence.ascendingBracket.after.frame.provenance.sequence;
        assertEvidenceRejected (candidate);

        candidate                      = envelope ();
        candidate.evidence.terminalLeg = adk::ModuleCharacterizationLeg::Descending;
        assertEvidenceRejected (candidate);

        candidate = envelope ();
        candidate.evidence.descendingBracket.before.legId =
            candidate.evidence.ascendingBracket.before.legId;
        candidate.evidence.descendingBracket.after.legId =
            candidate.evidence.ascendingBracket.before.legId;
        assertEvidenceRejected (candidate);

        candidate                         = envelope ();
        candidate.evidence.offendingAfter = candidate.evidence.lastWitness;
        assertEvidenceRejected (candidate);

        const adk::ModuleCharacterizationEvidence rejected = rejectedEvidence ();

        candidate          = envelope ();
        candidate.evidence = rejected;
        candidate.descriptorDigest =
            adk::moduleThresholdDescriptorDigest (candidate.evidence.descriptor);

        adk::ModuleCharacterizationEnvelope rejectedMutation = candidate;
        rejectedMutation.evidence.status = adk::StatusCode::HardwareFailure;
        assertEvidenceRejected (rejectedMutation);

        rejectedMutation                         = candidate;
        rejectedMutation.evidence.offendingAfter = {false, 0, 0,
                                                    false, 0, adk::TimePoint (0)};
        assertEvidenceRejected (rejectedMutation);

        rejectedMutation = candidate;
        rejectedMutation.evidence.reason =
            adk::ModuleCharacterizationReason::TransitionOrientationMismatch;
        assertEvidenceRejected (rejectedMutation);

        rejectedMutation = candidate;
        rejectedMutation.evidence.reason =
            adk::ModuleCharacterizationReason::AnalogComparatorDisagreement;
        assertEvidenceRejected (rejectedMutation);

        rejectedMutation                   = candidate;
        rejectedMutation.evidence.relation = adk::ModuleComparatorRelation::Consistent;
        assertEvidenceRejected (rejectedMutation);

        rejectedMutation                                     = candidate;
        rejectedMutation.evidence.guaranteedInactiveInterval = {true, 0, 100};
        assertEvidenceRejected (rejectedMutation);
    }

    void testConfigurationAndSessionAdmissionMatrix ()
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::ModuleBenchConfig config = benchConfig (admitted);

        ++config.expectedDescriptorId;
        adk::InertModuleCharacterizationBench descriptorBench (config);

        assert (descriptorBench.initialize (adk::TimePoint (0)).ok ());
        assert (
            descriptorBench.beginSession (adk::TimePoint (0), 11, admitted).error () ==
            adk::StatusCode::InvalidArgument);

        config = benchConfig (admitted);
        ++config.envelopeRevision;
        adk::InertModuleCharacterizationBench envelopeBench (config);

        assert (envelopeBench.initialize (adk::TimePoint (0)).ok ());
        assert (
            envelopeBench.beginSession (adk::TimePoint (0), 11, admitted).error () ==
            adk::StatusCode::InvalidArgument);

        adk::InertModuleCharacterizationBench sessionBench (benchConfig (admitted));

        assert (sessionBench.initialize (adk::TimePoint (0)).ok ());
        assert (sessionBench.beginSession (adk::TimePoint (0), 12, admitted).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testBenchLifecycleAtomicityAndCanaries ()
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::InertModuleCharacterizationBench bench (benchConfig (admitted));
        adk::ModuleBenchResult result = {UINT32_MAX,
                                         UINT32_MAX,
                                         adk::ModuleBenchState::Fault,
                                         adk::ModuleBenchScriptStep::PrepareRecord,
                                         UINT32_MAX,
                                         UINT32_MAX,
                                         UINT32_MAX,
                                         adk::ModuleComparatorRelation::Disagrees,
                                         {adk::ModuleBenchScriptStep::PrepareRecord,
                                          adk::ModuleBenchState::Fault, true,
                                          adk::ModuleComparatorRelation::Disagrees},
                                         true,
                                         adk::StatusCode::HardwareFailure};
        const adk::ModuleBenchResult sentinel = result;

        assert             (bench.result (result).error () == adk::StatusCode::NotInitialized);
        assertResultsEqual (result, sentinel);
        assert             (bench.initialize (adk::TimePoint (0)).ok ());
        assert             (bench.result (result).ok ());
        assert             (result.state == adk::ModuleBenchState::Ready);

        adk::ModuleCharacterizationEnvelope changed = admitted;
        ++changed.evidence.runId;
        assert (bench.beginSession (adk::TimePoint (0), 11, changed).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.result (result).ok ());
        assert (result.state == adk::ModuleBenchState::Ready);
        assert (bench.beginSession (adk::TimePoint (0), 11, admitted).ok ());

        struct GuardedImage
        {
            uint8_t                                before;
            adk::ModuleCharacterizationRecordImage image;
            uint8_t                                after;
        } output              = {0xa5, {}, 0x5a};
        output.image.bytes[0] = 0xcc;
        assert (bench.prepareRecord (adk::TimePoint (0), output.image).error () ==
                adk::StatusCode::InvalidArgument);
        assert (output.image.bytes[0] == 0xcc);

        assert (bench.applyCommand (adk::TimePoint (0), control (0, 0)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.applyCommand (adk::TimePoint (1), control (1, 2)).error () ==
                adk::StatusCode::InvalidArgument);
        for (uint32_t sequence = 1; sequence <= 4; ++sequence)
        {
            const adk::ModuleBenchControl next = control (sequence, sequence);

            assert (bench.applyCommand (adk::TimePoint (sequence), next).ok ());
        }
        assert (bench.applyCommand (adk::TimePoint (4), control (4, 4)).ok ());

        adk::ModuleBenchControl changedDuplicate = control (4, 4);

        changedDuplicate.command = adk::ModuleBenchCommand::None;
        assert (bench.applyCommand (adk::TimePoint (4), changedDuplicate).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.applyCommand (adk::TimePoint (4), control (6, 4)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.result (result).ok ());
        assert (result.step == adk::ModuleBenchScriptStep::PrepareRecord);
        assert (bench.prepareRecord (adk::TimePoint (4), output.image).ok ());
        assert (output.before == 0xa5 && output.after == 0x5a);
        const adk::ModuleCharacterizationRecordImage first = output.image;
        output.image.bytes[0]                              = 0;
        assert (bench.prepareRecord (adk::TimePoint (4), output.image).ok ());
        for (uint16_t index = 0; index < 192; ++index)
        {
            assert (output.image.bytes[index] == first.bytes[index]);
        }
        assert (bench.reset (adk::TimePoint (5)).ok ());
        assert (bench.result (result).ok ());
        assert (result.lifecycleGeneration == 2);
        assert (result.state == adk::ModuleBenchState::Ready);
        assert (bench.shutdown (adk::TimePoint (6)).ok ());
        assert (bench.shutdown (adk::TimePoint (6)).ok ());
    }

    void testControlFailureAndExhaustion ()
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::InertModuleCharacterizationBench bench (benchConfig (admitted));

        assert (bench.initialize (adk::TimePoint (0)).ok ());
        assert (bench.beginSession (adk::TimePoint (0), 11, admitted).ok ());

        adk::ModuleBenchControl failed = control (1, 1);

        failed.producerStatus = adk::StatusCode::HardwareFailure;

        assert (bench.applyCommand (adk::TimePoint (1), failed).ok ());
        adk::ModuleBenchResult result;

        assert (bench.result (result).ok ());
        assert (result.state == adk::ModuleBenchState::Fault);
        assert (result.presentation.faultDominant);
        assert (result.status.error () == adk::StatusCode::HardwareFailure);
        assert (bench.reset (adk::TimePoint (2)).ok ());

        bench.seedLifecycleGenerationForTest (UINT32_MAX);

        assert (bench.reset (adk::TimePoint (2)).error () ==
                adk::StatusCode::CapacityExceeded);
        assert (bench.result (result).ok ());
        assert (result.lifecycleGeneration == UINT32_MAX);
    }

    void testControlChronologyAndSessionReuse ()
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::InertModuleCharacterizationBench bench (benchConfig (admitted));

        assert (bench.initialize (adk::TimePoint (0)).ok ());
        assert (bench.beginSession (adk::TimePoint (0), 11, admitted).ok ());

        assert (
            bench
                .applyCommand (adk::TimePoint (1),
                               control (UINT32_MAX, 1, adk::ModuleBenchCommand::None))
                .ok ());
        assert (bench
                    .applyCommand (adk::TimePoint (2),
                                   control (0, 2, adk::ModuleBenchCommand::None))
                    .ok ());
        assert (bench
                    .applyCommand (adk::TimePoint (3),
                                   control (1, 1, adk::ModuleBenchCommand::None))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (bench
                    .applyCommand (adk::TimePoint (23),
                                   control (1, 2, adk::ModuleBenchCommand::None))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (bench
                    .applyCommand (adk::TimePoint (3),
                                   control (1, 4, adk::ModuleBenchCommand::None))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (bench
                    .applyCommand (adk::TimePoint (3),

                                   control (1, UINT32_C (0x80000003),
                                            adk::ModuleBenchCommand::None))
                    .error () == adk::StatusCode::InvalidArgument);
        assert (bench
                    .applyCommand (adk::TimePoint (3),
                                   control (UINT32_C (0x80000000), 3,
                                            adk::ModuleBenchCommand::None))
                    .error () == adk::StatusCode::InvalidArgument);

        assert (bench.reset (adk::TimePoint (4)).ok ());
        assert (bench.beginSession (adk::TimePoint (4), 11, admitted).error () ==
                adk::StatusCode::InvalidArgument);
        adk::ModuleCharacterizationRecordImage image = {};
        assert (bench.prepareRecord (adk::TimePoint (4), image).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench
                    .applyCommand (adk::TimePoint (4),
                                   control (1, 4, adk::ModuleBenchCommand::Advance))
                    .error () == adk::StatusCode::InvalidArgument);
    }

    void testControlIdentityStateAndStepMatrix ()
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::InertModuleCharacterizationBench bench (benchConfig (admitted));
        adk::ModuleBenchResult                result;

        assert (bench.initialize (adk::TimePoint (0)).ok ());
        adk::ModuleBenchControl readyNone =
            control (1, 0, adk::ModuleBenchCommand::None);
        readyNone.sessionId = 0;
        assert (bench.applyCommand (adk::TimePoint (0), readyNone).ok ());

        adk::ModuleBenchControl readyAdvance = control (2, 0);

        readyAdvance.sessionId = 0;
        assert (bench.applyCommand (adk::TimePoint (0), readyAdvance).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.beginSession (adk::TimePoint (0), 11, admitted).ok ());

        adk::ModuleBenchControl mismatch = control (1, 1);
        ++mismatch.sourceId;
        assert (bench.applyCommand (adk::TimePoint (1), mismatch).error () ==
                adk::StatusCode::InvalidArgument);
        mismatch = control (1, 1);
        ++mismatch.sourceConfigurationRevision;
        assert (bench.applyCommand (adk::TimePoint (1), mismatch).error () ==
                adk::StatusCode::InvalidArgument);
        mismatch = control (1, 1);
        ++mismatch.sessionId;
        assert (bench.applyCommand (adk::TimePoint (1), mismatch).error () ==
                adk::StatusCode::InvalidArgument);
        mismatch         = control (1, 1);
        mismatch.command = static_cast<adk::ModuleBenchCommand> (0xff);
        assert (bench.applyCommand (adk::TimePoint (1), mismatch).error () ==
                adk::StatusCode::InvalidArgument);
        mismatch.producerStatus = static_cast<adk::StatusCode> (0xff);
        assert (bench.applyCommand (adk::TimePoint (1), mismatch).error () ==
                adk::StatusCode::InvalidArgument);

        const adk::ModuleBenchScriptStep expectedSteps[] = {
            adk::ModuleBenchScriptStep::ReviewAscending,
            adk::ModuleBenchScriptStep::ReviewDescending,
            adk::ModuleBenchScriptStep::ReviewVerification,
            adk::ModuleBenchScriptStep::PrepareRecord};
        for (uint32_t sequence = 1; sequence <= 4; ++sequence)
        {
            assert (bench
                        .applyCommand (adk::TimePoint (sequence),
                                       control (sequence, sequence))
                        .ok ());
            assert (bench.result (result).ok ());
            assert (result.step == expectedSteps[sequence - 1]);
            assert (result.presentation.step == result.step);
            assert (result.presentation.state == result.state);
        }

        adk::ModuleCharacterizationRecordImage image = {};
        assert (bench.prepareRecord (adk::TimePoint (4), image).ok ());
        assert (bench.applyCommand (adk::TimePoint (5), control (5, 5)).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.result (result).ok ());
        assert (result.state == adk::ModuleBenchState::RecordPrepared);

        adk::InertModuleCharacterizationBench faultBench (benchConfig (admitted));

        assert (faultBench.initialize (adk::TimePoint (0)).ok ());
        assert (faultBench.beginSession (adk::TimePoint (0), 11, admitted).ok ());

        adk::ModuleBenchControl collision = control (1, 1);
        collision.producerStatus          = adk::StatusCode::HardwareFailure;
        assert (faultBench.applyCommand (adk::TimePoint (1), collision).ok ());
        assert (faultBench.result (result).ok ());
        assert (result.state == adk::ModuleBenchState::Fault);
        assert (result.step == adk::ModuleBenchScriptStep::InspectDeclaration);
        assert (faultBench.applyCommand (adk::TimePoint (2), control (2, 2)).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testAgeBoundaryAtomicityAndShutdown ()
    {
        const adk::ModuleCharacterizationEnvelope admitted = envelope ();

        adk::InertModuleCharacterizationBench bench (benchConfig (admitted));
        adk::ModuleBenchResult                before;
        adk::ModuleBenchResult                after;

        assert (bench.initialize (adk::TimePoint (0)).ok ());
        assert (bench.beginSession (adk::TimePoint (0), 11, admitted).ok ());
        assert (bench.applyCommand (adk::TimePoint (21), control (1, 1)).ok ());
        assert (bench.result (before).ok ());

        adk::ModuleBenchControl stale = control (2, 1);

        assert (bench.applyCommand (adk::TimePoint (22), stale).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.result (after).ok ());
        assert (after.state == before.state);
        assert (after.step == before.step);
        assert (after.status.error () == before.status.error ());

        adk::ModuleBenchControl duplicate = control (1, 1);
        duplicate.command                 = adk::ModuleBenchCommand::None;
        assert (bench.applyCommand (adk::TimePoint (21), duplicate).error () ==
                adk::StatusCode::InvalidArgument);
        assert (bench.result (after).ok ());
        assert (after.step == before.step);

        assert (bench.shutdown (adk::TimePoint (21)).ok ());
        assert (bench.result (after).ok ());
        assert (after.state == adk::ModuleBenchState::Shutdown);
        assert (!after.recordPrepared);
        assert (after.status.ok ());
        assert (after.presentation.state == adk::ModuleBenchState::Shutdown);
        assert (!after.presentation.faultDominant);
        assert (bench.beginSession (adk::TimePoint (22), 12, admitted).error () ==
                adk::StatusCode::NotInitialized);
        assert (bench.applyCommand (adk::TimePoint (22), control (2, 22)).error () ==
                adk::StatusCode::NotInitialized);
        adk::ModuleCharacterizationRecordImage image;
        memset (image.bytes, 0xa5, sizeof image.bytes);
        assert (bench.prepareRecord (adk::TimePoint (22), image).error () ==
                adk::StatusCode::NotInitialized);
        for (uint16_t index = 0; index < sizeof image.bytes; ++index)
        {
            assert (image.bytes[index] == 0xa5);
        }
        assert (bench.reset (adk::TimePoint (22)).error () ==
                adk::StatusCode::NotInitialized);
    }
} // namespace

int main ()
{
    testCodecWireAndAtomicity                  ();
    testIntegerExtremaAndCanonicalAbsence      ();
    testDigestGoldenSeedsAndEvidence           ();
    testCrcValidSemanticBoundaries             ();
    testEnvelopeSemanticAdmission              ();
    testEnvelopeIdentityAndCorrelationMatrix   ();
    testTerminalAndRejectedCorrelationMatrix   ();
    testConfigurationAndSessionAdmissionMatrix ();
    testBenchLifecycleAtomicityAndCanaries     ();
    testControlFailureAndExhaustion            ();
    testControlChronologyAndSessionReuse       ();
    testControlIdentityStateAndStepMatrix      ();
    testAgeBoundaryAtomicityAndShutdown        ();

    return 0;
}
