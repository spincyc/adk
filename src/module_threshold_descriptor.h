#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct ModuleChannelTopology : uint8_t
    {
        AnalogOnly,
        ComparatorOnly,
        AnalogAndComparator
    };

    enum struct ModuleComparatorOutputStage : uint8_t
    {
        Unspecified,
        PushPull,
        OpenDrain,
        OpenCollector
    };

    enum struct ModulePullRequirement : uint8_t
    {
        Unspecified,
        None,
        PullUp,
        PullDown
    };

    enum struct ModuleDeclaredRail : uint8_t
    {
        Unspecified,
        Ground,
        LogicSupply,
        ModuleSupply
    };

    enum struct ModuleComparatorPolarity : uint8_t
    {
        Unspecified,
        ActiveHigh,
        ActiveLow
    };

    enum struct ModuleThresholdControlKind : uint8_t
    {
        Unspecified,
        Fixed,
        Potentiometer
    };

    enum struct ModuleThresholdDirection : uint8_t
    {
        Unspecified,
        IncreasingClockwise,
        IncreasingCounterclockwise
    };

    enum struct ModuleDurationDeclaration : uint8_t
    {
        Known,
        Unknown
    };

    struct ModuleDeclaredDuration
    {
        ModuleDurationDeclaration declaration;
        Duration                  value;
    };

    struct ModuleMillivoltRange
    {
        uint16_t minimum;
        uint16_t maximum;
    };

    struct ModuleRawDomain
    {
        uint16_t minimum;
        uint16_t maximum;
    };

    struct ModuleThresholdDescriptor
    {
        uint16_t                    schemaRevision;
        uint32_t                    descriptorId;
        uint16_t                    descriptorRevision;
        uint32_t                    declaredSpecimenReference;
        uint16_t                    declaredSpecimenRevision;
        uint16_t                    declaredElectricalEvidenceRevision;
        ModuleChannelTopology       channelTopology;
        ModuleComparatorOutputStage comparatorOutputStage;
        ModulePullRequirement       pullRequirement;
        ModuleDeclaredRail          declaredPullRail;
        ModuleMillivoltRange        declaredSupplyMillivolts;
        ModuleMillivoltRange        declaredSignalMillivolts;
        ModuleRawDomain             rawDomain;
        ModuleComparatorPolarity    comparatorPolarity;
        ModuleThresholdControlKind  thresholdControlKind;
        ModuleThresholdDirection    thresholdDirection;
        ModuleDeclaredDuration      warmup;
        ModuleDeclaredDuration      settling;
    };

    enum struct ModuleChannelStatus : uint8_t
    {
        NotPresent,
        Current,
        Stale,
        ProducerFault
    };

    struct ModuleFrameProvenance
    {
        uint8_t   sourceId;
        uint16_t  sourceConfigurationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
    };

    struct ModuleThresholdFrame
    {
        uint16_t              schemaRevision;
        uint32_t              descriptorId;
        uint16_t              descriptorRevision;
        uint32_t              declaredSpecimenReference;
        uint16_t              declaredSpecimenRevision;
        uint16_t              declaredElectricalEvidenceRevision;
        ModuleFrameProvenance provenance;
        uint16_t              analogRaw;
        ModuleChannelStatus   analogStatus;
        bool                  comparatorLevelHigh;
        ModuleChannelStatus   comparatorStatus;
        bool                  comparatorPresent;
        bool                  comparatorAsserted;
        bool                  declaredWarmupSatisfied;
        bool                  declaredSettlingSatisfied;
        Status                analogProducerStatus;
        Status                comparatorProducerStatus;
    };

    Status validateModuleThresholdDescriptor (
        const ModuleThresholdDescriptor& descriptor) noexcept;
    Status validateModuleThresholdFrame (
        const ModuleThresholdDescriptor& descriptor,
        const ModuleThresholdFrame&      frame) noexcept;
    Result<bool> moduleComparatorAsserted (
        const ModuleThresholdDescriptor& descriptor,
        bool comparatorLevelHigh) noexcept;
    Result<bool> moduleDescriptorDeclarationsComplete (
        const ModuleThresholdDescriptor& descriptor) noexcept;
} // namespace adk
