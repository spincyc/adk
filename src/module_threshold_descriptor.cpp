#include "module_threshold_descriptor.h"

namespace adk {

    namespace {

        template <typename Value>
        bool inClosedRange (Value value, Value first, Value last) noexcept
        {
            return value >= first && value <= last;
        }

        bool validDuration (const ModuleDeclaredDuration& duration) noexcept
        {
            if (!inClosedRange (duration.declaration,
                                ModuleDurationDeclaration::Known,
                                ModuleDurationDeclaration::Unknown))
            {
                return false;
            }

            return duration.declaration == ModuleDurationDeclaration::Known ||
                   duration.value.milliseconds () == 0;
        }

        bool hasAnalog (ModuleChannelTopology topology) noexcept
        {
            return topology == ModuleChannelTopology::AnalogOnly ||
                   topology == ModuleChannelTopology::AnalogAndComparator;
        }

        bool hasComparator (ModuleChannelTopology topology) noexcept
        {
            return topology == ModuleChannelTopology::ComparatorOnly ||
                   topology == ModuleChannelTopology::AnalogAndComparator;
        }

        bool validStatus (Status status) noexcept
        {
            return inClosedRange (
                status.error (), StatusCode::Ok, StatusCode::HardwareFailure);
        }

        bool validChannel (ModuleChannelStatus channelStatus,
                           Status              producerStatus,
                           bool                present) noexcept
        {
            if (!inClosedRange (channelStatus,
                                ModuleChannelStatus::NotPresent,
                                ModuleChannelStatus::ProducerFault) ||
                !validStatus (producerStatus))
            {
                return false;
            }

            if (!present)
            {
                return channelStatus == ModuleChannelStatus::NotPresent &&
                       producerStatus.ok ();
            }

            if (channelStatus == ModuleChannelStatus::NotPresent)
            {
                return false;
            }

            return channelStatus == ModuleChannelStatus::ProducerFault
                       ? !producerStatus.ok  ()
                       : producerStatus.ok   ();
        }

        bool descriptorEnumsValid (
            const ModuleThresholdDescriptor& descriptor) noexcept
        {
            return inClosedRange (
                       descriptor.channelTopology,
                       ModuleChannelTopology::AnalogOnly,
                       ModuleChannelTopology::AnalogAndComparator) &&
                   inClosedRange (
                       descriptor.comparatorOutputStage,
                       ModuleComparatorOutputStage::Unspecified,
                       ModuleComparatorOutputStage::OpenCollector) &&
                   inClosedRange (
                       descriptor.pullRequirement,
                       ModulePullRequirement::Unspecified,
                       ModulePullRequirement::PullDown) &&
                   inClosedRange (
                       descriptor.declaredPullRail,
                       ModuleDeclaredRail::Unspecified,
                       ModuleDeclaredRail::ModuleSupply) &&
                   inClosedRange (
                       descriptor.comparatorPolarity,
                       ModuleComparatorPolarity::Unspecified,
                       ModuleComparatorPolarity::ActiveLow) &&
                   inClosedRange (
                       descriptor.thresholdControlKind,
                       ModuleThresholdControlKind::Unspecified,
                       ModuleThresholdControlKind::Potentiometer) &&
                   inClosedRange (
                       descriptor.thresholdDirection,
                       ModuleThresholdDirection::Unspecified,
                       ModuleThresholdDirection::IncreasingCounterclockwise);
        }

        bool pullDeclarationCanonical (
            const ModuleThresholdDescriptor& descriptor) noexcept
        {
            const bool declaresPull =
                descriptor.pullRequirement == ModulePullRequirement::PullUp ||
                descriptor.pullRequirement == ModulePullRequirement::PullDown;

            return declaresPull
                       ? descriptor.declaredPullRail !=
                             ModuleDeclaredRail::Unspecified
                       : descriptor.declaredPullRail ==
                             ModuleDeclaredRail::Unspecified;
        }

        bool comparatorDeclarationCanonical (
            const ModuleThresholdDescriptor& descriptor) noexcept
        {
            if (!hasComparator (descriptor.channelTopology))
            {
                return descriptor.comparatorOutputStage ==
                           ModuleComparatorOutputStage::Unspecified &&
                       descriptor.pullRequirement ==
                           ModulePullRequirement::Unspecified &&
                       descriptor.declaredPullRail ==
                           ModuleDeclaredRail::Unspecified &&
                       descriptor.comparatorPolarity ==
                           ModuleComparatorPolarity::Unspecified &&
                       descriptor.thresholdControlKind ==
                           ModuleThresholdControlKind::Unspecified &&
                       descriptor.thresholdDirection ==
                           ModuleThresholdDirection::Unspecified;
            }

            if (!pullDeclarationCanonical (descriptor))
            {
                return false;
            }

            return descriptor.thresholdControlKind ==
                           ModuleThresholdControlKind::Potentiometer
                       ? true
                       : descriptor.thresholdDirection ==
                             ModuleThresholdDirection::Unspecified;
        }
    } // namespace

    Status validateModuleThresholdDescriptor (
        const ModuleThresholdDescriptor& descriptor) noexcept
    {
        if (!descriptorEnumsValid (descriptor) ||
            !validDuration (descriptor.warmup) ||
            !validDuration (descriptor.settling))
        {
            return StatusCode::InvalidArgument;
        }

        if (descriptor.schemaRevision == 0 || descriptor.descriptorId == 0 ||
            descriptor.descriptorRevision == 0 ||
            descriptor.declaredSpecimenReference == 0 ||
            descriptor.declaredSpecimenRevision == 0 ||
            descriptor.declaredElectricalEvidenceRevision == 0 ||
            descriptor.declaredSupplyMillivolts.minimum >
                descriptor.declaredSupplyMillivolts.maximum ||
            descriptor.declaredSignalMillivolts.minimum >
                descriptor.declaredSignalMillivolts.maximum ||
            descriptor.rawDomain.minimum > descriptor.rawDomain.maximum ||
            !comparatorDeclarationCanonical (descriptor))
        {
            return StatusCode::InvalidConfiguration;
        }

        return StatusCode::Ok;
    }

    Result<bool> moduleComparatorAsserted (
        const ModuleThresholdDescriptor& descriptor,
        bool comparatorLevelHigh) noexcept
    {
        const Status validation =
            validateModuleThresholdDescriptor (descriptor);
        if (!validation.ok ())
        {
            return {validation, false};
        }

        if (!hasComparator (descriptor.channelTopology) ||
            descriptor.comparatorPolarity ==
                ModuleComparatorPolarity::Unspecified)
        {
            return {StatusCode::InvalidConfiguration, false};
        }

        return {StatusCode::Ok,
                descriptor.comparatorPolarity ==
                        ModuleComparatorPolarity::ActiveHigh
                    ? comparatorLevelHigh
                    : !comparatorLevelHigh};
    }

    Status validateModuleThresholdFrame (
        const ModuleThresholdDescriptor& descriptor,
        const ModuleThresholdFrame&      frame) noexcept
    {
        const Status descriptorStatus =
            validateModuleThresholdDescriptor (descriptor);
        if (!descriptorStatus.ok ())
        {
            return descriptorStatus;
        }

        if (frame.schemaRevision != descriptor.schemaRevision ||
            frame.descriptorId != descriptor.descriptorId ||
            frame.descriptorRevision != descriptor.descriptorRevision ||
            frame.declaredSpecimenReference !=
                descriptor.declaredSpecimenReference ||
            frame.declaredSpecimenRevision !=
                descriptor.declaredSpecimenRevision ||
            frame.declaredElectricalEvidenceRevision !=
                descriptor.declaredElectricalEvidenceRevision)
        {
            return StatusCode::InvalidArgument;
        }

        if (frame.provenance.sourceId == 0 ||
            frame.provenance.sourceConfigurationRevision == 0)
        {
            return StatusCode::InvalidArgument;
        }

        const bool analogPresent = hasAnalog (descriptor.channelTopology);
        const bool comparatorExpected =
            hasComparator (descriptor.channelTopology);

        if (!validChannel (frame.analogStatus,
                           frame.analogProducerStatus,
                           analogPresent) ||
            !validChannel (frame.comparatorStatus,
                           frame.comparatorProducerStatus,
                           comparatorExpected) ||
            frame.comparatorPresent != comparatorExpected ||
            (!analogPresent && frame.analogRaw != 0) ||
            (analogPresent &&
             (frame.analogRaw < descriptor.rawDomain.minimum ||
              frame.analogRaw > descriptor.rawDomain.maximum)) ||
            (!comparatorExpected &&
             (frame.comparatorLevelHigh || frame.comparatorAsserted)) ||
            (descriptor.warmup.declaration ==
                 ModuleDurationDeclaration::Unknown &&
             frame.declaredWarmupSatisfied) ||
            (descriptor.settling.declaration ==
                 ModuleDurationDeclaration::Unknown &&
             frame.declaredSettlingSatisfied))
        {
            return StatusCode::InvalidArgument;
        }

        if (comparatorExpected)
        {
            const Result<bool> asserted =
                moduleComparatorAsserted (descriptor,
                                          frame.comparatorLevelHigh);

            if (asserted.ok ())
            {
                if (frame.comparatorAsserted != asserted.value ())
                {
                    return StatusCode::InvalidArgument;
                }
            }
            else if (descriptor.comparatorPolarity ==
                         ModuleComparatorPolarity::Unspecified &&
                     frame.comparatorAsserted)
            {
                return StatusCode::InvalidArgument;
            }
        }

        return StatusCode::Ok;
    }

    Result<bool> moduleDescriptorDeclarationsComplete (
        const ModuleThresholdDescriptor& descriptor) noexcept
    {
        const Status validation =
            validateModuleThresholdDescriptor (descriptor);
        if (!validation.ok ())
        {
            return {validation, false};
        }

        const bool comparatorComplete =
            !hasComparator (descriptor.channelTopology) ||
            (descriptor.comparatorOutputStage !=
                 ModuleComparatorOutputStage::Unspecified &&
             descriptor.pullRequirement !=
                 ModulePullRequirement::Unspecified &&
             descriptor.comparatorPolarity !=
                 ModuleComparatorPolarity::Unspecified &&
             descriptor.thresholdControlKind !=
                 ModuleThresholdControlKind::Unspecified &&
             (descriptor.thresholdControlKind !=
                  ModuleThresholdControlKind::Potentiometer ||
              descriptor.thresholdDirection !=
                  ModuleThresholdDirection::Unspecified));
        const bool durationsKnown =
            descriptor.warmup.declaration ==
                ModuleDurationDeclaration::Known &&
            descriptor.settling.declaration ==
                ModuleDurationDeclaration::Known;

        return {StatusCode::Ok, comparatorComplete && durationsKnown};
    }
} // namespace adk
