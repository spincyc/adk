#pragma once

#include "module_characterization.h"

#include <stdint.h>

namespace adk {

    uint32_t moduleThresholdDescriptorDigest (
        const ModuleThresholdDescriptor& descriptor) noexcept;
    uint32_t moduleCompactWitnessDigest (const ModuleCompactWitness& witness) noexcept;

    uint32_t moduleCharacterizationEvidenceDigest (
        const ModuleCharacterizationEvidence& evidence) noexcept;
} // namespace adk
