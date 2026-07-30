#include <module_threshold_descriptor.h>

unsigned char moduleThresholdCompositionBytes
    [sizeof (adk::ModuleThresholdDescriptor)
     + sizeof (adk::ModuleThresholdFrame)];
unsigned char moduleThresholdDescriptorBytes
    [sizeof (adk::ModuleThresholdDescriptor)];
unsigned char moduleThresholdFrameBytes[sizeof (adk::ModuleThresholdFrame)];

#if defined (ADK_HAS_LESSON_071)
#include <module_characterization.h>

unsigned char moduleCharacterizationPolicyBytes
    [sizeof (adk::ModuleCharacterizationPolicy)];
unsigned char moduleCharacterizationEvidenceBytes
    [sizeof (adk::ModuleCharacterizationEvidence)];
unsigned char moduleCharacterizationPointBytes
    [sizeof (adk::ModuleCharacterizationPoint)];
#endif

#if defined (ADK_HAS_LESSON_072)
#include <inert_module_characterization_bench.h>
#include <module_characterization_record.h>

unsigned char inertModuleCharacterizationBenchBytes
    [sizeof (adk::InertModuleCharacterizationBench)];
unsigned char moduleCharacterizationRecordImageBytes
    [sizeof (adk::ModuleCharacterizationRecordImage)];
unsigned char moduleCharacterizationSimultaneousImagesBytes
    [2 * sizeof (adk::ModuleCharacterizationRecordImage)];
#endif
