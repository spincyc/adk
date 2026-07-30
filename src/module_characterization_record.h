#pragma once

#include "bounded_span.h"
#include "module_characterization.h"

#include <stdint.h>

namespace adk {

    enum struct ModuleBenchScriptStep : uint8_t
    {
        InspectDeclaration,
        ReviewAscending,
        ReviewDescending,
        ReviewVerification,
        PrepareRecord
    };

    struct ModuleCharacterizationRecordImage
    {
        static constexpr uint8_t  version = 1;
        static constexpr uint16_t size    = 192;

        uint8_t bytes[size];
    };

    enum struct ModuleCharacterizationRecordValidity : uint8_t
    {
        Valid,
        BadLength,
        BadFraming,
        BadIntegrity,
        BadSemanticValue
    };

    struct ModuleCompactBracket
    {
        bool     present;
        uint16_t beforeRaw;
        uint16_t afterRaw;
        bool     beforeAsserted;
        bool     afterAsserted;
        uint32_t beforeSequence;
        uint32_t afterSequence;
    };

    struct ModuleCharacterizationRecord
    {
        uint16_t                     recordSchemaRevision;
        uint16_t                     benchRevision;
        uint16_t                     descriptorSchemaRevision;
        uint16_t                     envelopeRevision;
        uint32_t                     lifecycleGeneration;
        uint32_t                     sessionId;
        uint32_t                     descriptorId;
        uint16_t                     descriptorRevision;
        uint32_t                     declaredSpecimenReference;
        uint16_t                     declaredSpecimenRevision;
        uint16_t                     declaredElectricalEvidenceRevision;
        ModuleChannelTopology        channelTopology;
        ModuleComparatorOutputStage  comparatorOutputStage;
        ModulePullRequirement        pullRequirement;
        ModuleDeclaredRail           declaredPullRail;
        ModuleComparatorPolarity     comparatorPolarity;
        ModuleThresholdControlKind   thresholdControlKind;
        ModuleThresholdDirection     thresholdDirection;
        ModuleMillivoltRange         declaredSupplyMillivolts;
        ModuleMillivoltRange         declaredSignalMillivolts;
        ModuleRawDomain              rawDomain;
        ModuleDeclaredDuration       warmup;
        ModuleDeclaredDuration       settling;
        uint32_t                     runId;
        uint32_t                     characterizationLifecycleGeneration;
        uint16_t                     characterizationRevision;
        uint8_t                      sourceId;
        uint16_t                     sourceConfigurationRevision;
        uint8_t                      ascendingCount;
        uint8_t                      descendingCount;
        uint8_t                      verificationCount;
        ModuleCompactBracket         ascendingBracket;
        ModuleCompactBracket         descendingBracket;
        ModuleAnalogInterval         guaranteedInactiveInterval;
        ModuleAnalogInterval         guaranteedActiveInterval;
        ModuleAnalogInterval         ambiguityInterval;
        ModuleComparatorRelation     relation;
        uint32_t                     firstWitnessDigest;
        uint32_t                     lastWitnessDigest;
        uint32_t                     offendingBeforeDigest;
        uint32_t                     offendingAfterDigest;
        uint32_t                     firstSequence;
        uint32_t                     lastSequence;
        uint32_t                     descriptorDigest;
        uint32_t                     evidenceDigest;
        ModuleCharacterizationState  terminalState;
        ModuleCharacterizationReason terminalReason;
        Status                       terminalStatus;
        ModuleBenchScriptStep        scriptStep;
    };

    struct ModuleCharacterizationRecordCodec
    {
        static constexpr uint16_t crcPolynomial   = 0x1021;
        static constexpr uint16_t crcInitialValue = 0xffff;
        static constexpr uint16_t crcFinalXor     = 0x0000;

        Result<uint16_t> encode (const ModuleCharacterizationRecord& record,
                                 MutableByteSpan output) const noexcept;
        ModuleCharacterizationRecordValidity
        decode (ByteSpan image, ModuleCharacterizationRecord& output) const noexcept;
    };

    static_assert (sizeof (ModuleCharacterizationRecordImage::bytes) == 192,
                   "module characterization image is exactly 192 bytes");
} // namespace adk
