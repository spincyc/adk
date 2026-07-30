#pragma once

#include "module_characterization_digest.h"
#include "module_characterization_record.h"

#include <stdint.h>

namespace adk {

    struct ModuleCharacterizationEnvelope
    {
        uint16_t                       envelopeRevision;
        ModuleCharacterizationEvidence evidence;
        uint32_t                       descriptorDigest;
        uint32_t                       evidenceDigest;
    };

    enum struct ModuleBenchState : uint8_t
    {
        Inert,
        Ready,
        ScriptActive,
        RecordPrepared,
        Fault,
        Shutdown
    };

    enum struct ModuleBenchCommand : uint8_t
    {
        None,
        Advance
    };

    struct ModuleBenchControl
    {
        uint8_t            sourceId;
        uint16_t           sourceConfigurationRevision;
        uint32_t           sessionId;
        uint32_t           sequence;
        TimePoint          observedAt;
        ModuleBenchCommand command;
        Status             producerStatus;
    };

    struct ModuleBenchPresentationIntent
    {
        ModuleBenchScriptStep    step;
        ModuleBenchState         state;
        bool                     faultDominant;
        ModuleComparatorRelation relation;
    };

    struct ModuleBenchConfig
    {
        uint16_t benchRevision;
        uint16_t envelopeRevision;
        uint16_t recordSchemaRevision;
        uint32_t expectedDescriptorId;
        uint16_t expectedDescriptorRevision;
        uint16_t expectedDescriptorSchemaRevision;
        uint16_t expectedDeclaredSpecimenRevision;
        uint16_t expectedDeclaredElectricalEvidenceRevision;
        uint32_t expectedDescriptorDigest;
        uint8_t  expectedControlSourceId;
        uint16_t expectedControlSourceConfigurationRevision;
        Duration maximumControlAge;
    };

    struct ModuleBenchResult
    {
        uint32_t                      lifecycleGeneration;
        uint32_t                      sessionId;
        ModuleBenchState              state;
        ModuleBenchScriptStep         step;
        uint32_t                      runId;
        uint32_t                      descriptorDigest;
        uint32_t                      evidenceDigest;
        ModuleComparatorRelation      relation;
        ModuleBenchPresentationIntent presentation;
        bool                          recordPrepared;
        Status                        status;
    };

    struct InertModuleCharacterizationBench
    {
        explicit InertModuleCharacterizationBench (
            const ModuleBenchConfig& config) noexcept;

        InertModuleCharacterizationBench (const InertModuleCharacterizationBench&) =
            delete;
        InertModuleCharacterizationBench&
        operator= (const InertModuleCharacterizationBench&)                   = delete;
        InertModuleCharacterizationBench (InertModuleCharacterizationBench&&) = delete;
        InertModuleCharacterizationBench&
        operator= (InertModuleCharacterizationBench&&) = delete;

        Status initialize (TimePoint now) noexcept;

        Status beginSession (TimePoint now, uint32_t sessionId,
                             const ModuleCharacterizationEnvelope& envelope) noexcept;
        Status applyCommand (TimePoint now, const ModuleBenchControl& control) noexcept;

        Status prepareRecord (TimePoint                          now,
                              ModuleCharacterizationRecordImage& output) noexcept;
        Status reset (TimePoint now) noexcept;

        Status shutdown (TimePoint now) noexcept;

        Status result (ModuleBenchResult& output) const noexcept;

#if defined(ADK_TESTING)
        void seedLifecycleGenerationForTest (uint32_t generation) noexcept;
#endif

      private:
        ModuleBenchConfig                 config_;
        ModuleCharacterizationRecord      record_;
        ModuleCharacterizationRecordImage preparedImage_;
        ModuleBenchResult                 result_;
        ModuleBenchControl                lastControl_;
        TimePoint                         lastOperationAt_;
        uint32_t                          lastSessionId_;
        bool                              initialized_;
        bool                              shutdown_;
        bool                              hasRecord_;
        bool                              hasControl_;
    };
} // namespace adk
