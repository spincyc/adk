#include "module_characterization_record.h"

namespace adk {

    constexpr uint8_t  ModuleCharacterizationRecordImage::version;
    constexpr uint16_t ModuleCharacterizationRecordImage::size;
    constexpr uint16_t ModuleCharacterizationRecordCodec::crcPolynomial;
    constexpr uint16_t ModuleCharacterizationRecordCodec::crcInitialValue;
    constexpr uint16_t ModuleCharacterizationRecordCodec::crcFinalXor;

    namespace {

        constexpr uint8_t  magic[4]        = {'A', 'D', 'M', 'C'};
        constexpr uint16_t integrityOffset = 190;

        void write16 (uint8_t* output, uint16_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value);
            output[1] = static_cast<uint8_t> (value >> 8);
        }

        void write32 (uint8_t* output, uint32_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value);
            output[1] = static_cast<uint8_t> (value >> 8);
            output[2] = static_cast<uint8_t> (value >> 16);
            output[3] = static_cast<uint8_t> (value >> 24);
        }

        uint16_t read16 (const uint8_t* input) noexcept
        {
            return static_cast<uint16_t> (
                static_cast<uint16_t> (input[0]) |
                static_cast<uint16_t> (static_cast<uint16_t> (input[1]) << 8));
        }

        uint32_t read32 (const uint8_t* input) noexcept
        {
            return static_cast<uint32_t> (input[0]) |
                   static_cast<uint32_t> (input[1]) << 8 |
                   static_cast<uint32_t> (input[2]) << 16 |
                   static_cast<uint32_t> (input[3]) << 24;
        }

        uint16_t crc16 (const uint8_t* bytes, uint16_t length) noexcept
        {
            uint16_t crc = ModuleCharacterizationRecordCodec::crcInitialValue;
            for (uint16_t index = 0; index < length; ++index)
            {
                crc ^=
                    static_cast<uint16_t> (static_cast<uint16_t> (bytes[index]) << 8);
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & 0x8000U) != 0U
                              ? static_cast<uint16_t> (
                                    (crc << 1) ^
                                    ModuleCharacterizationRecordCodec::crcPolynomial)
                              : static_cast<uint16_t> (crc << 1);
                }
            }
            return static_cast<uint16_t> (
                crc ^ ModuleCharacterizationRecordCodec::crcFinalXor);
        }

        bool validStatus (Status status) noexcept
        {
            return static_cast<uint8_t> (status.error ()) <=
                   static_cast<uint8_t> (StatusCode::HardwareFailure);
        }

        bool validState (ModuleCharacterizationState state) noexcept
        {
            return state == ModuleCharacterizationState::Complete ||
                   state == ModuleCharacterizationState::Rejected;
        }

        bool validReason (ModuleCharacterizationReason reason) noexcept
        {
            return static_cast<uint8_t> (reason) <=
                   static_cast<uint8_t> (
                       ModuleCharacterizationReason::AnalogComparatorDisagreement);
        }

        bool validRelation (ModuleComparatorRelation relation) noexcept
        {
            return static_cast<uint8_t> (relation) <=
                   static_cast<uint8_t> (ModuleComparatorRelation::Disagrees);
        }

        bool validBracket (const ModuleCompactBracket& bracket,
                           const ModuleRawDomain& domain, bool ascending) noexcept
        {
            if (!bracket.present)
            {
                return bracket.beforeRaw == 0 && bracket.afterRaw == 0 &&
                       !bracket.beforeAsserted && !bracket.afterAsserted &&
                       bracket.beforeSequence == 0 && bracket.afterSequence == 0;
            }
            const uint32_t sequenceDelta =
                bracket.afterSequence - bracket.beforeSequence;
            return bracket.beforeRaw >= domain.minimum &&
                   bracket.beforeRaw <= domain.maximum &&
                   bracket.afterRaw >= domain.minimum &&
                   bracket.afterRaw <= domain.maximum &&
                   (ascending ? bracket.beforeRaw < bracket.afterRaw
                              : bracket.beforeRaw > bracket.afterRaw) &&
                   bracket.beforeAsserted != bracket.afterAsserted &&
                   bracket.beforeSequence != 0 && bracket.afterSequence != 0 &&
                   sequenceDelta != 0 && sequenceDelta < 0x80000000UL;
        }

        bool validInterval (const ModuleAnalogInterval& interval,
                            const ModuleRawDomain&      domain) noexcept
        {
            if (!interval.present)
            {
                return interval.lower == 0 && interval.upper == 0;
            }
            return interval.lower >= domain.minimum &&
                   interval.lower <= interval.upper && interval.upper <= domain.maximum;
        }

        bool disjointIntervals (const ModuleAnalogInterval& left,
                                const ModuleAnalogInterval& right) noexcept
        {
            return !left.present || !right.present || left.upper < right.lower ||
                   right.upper < left.lower;
        }

        bool sameInterval (const ModuleAnalogInterval& interval, bool present,
                           uint16_t lower, uint16_t upper) noexcept
        {
            return interval.present == present &&
                   interval.lower == (present ? lower : 0) &&
                   interval.upper == (present ? upper : 0);
        }

        bool validCompleteGeometry (
            const ModuleCharacterizationRecord& record) noexcept
        {
            const ModuleCompactBracket& ascending  = record.ascendingBracket;
            const ModuleCompactBracket& descending = record.descendingBracket;
            const bool                  lowState    = ascending.beforeAsserted;
            if (lowState != descending.afterAsserted ||
                ascending.afterAsserted != descending.beforeAsserted ||
                lowState == ascending.afterAsserted)
            {
                return false;
            }

            const uint16_t lowProved =
                ascending.beforeRaw < descending.afterRaw
                    ? ascending.beforeRaw
                    : descending.afterRaw;
            const uint16_t highProved =
                ascending.afterRaw > descending.beforeRaw
                    ? ascending.afterRaw
                    : descending.beforeRaw;
            if (lowProved >= highProved)
            {
                return false;
            }

            const ModuleAnalogInterval& low =
                lowState ? record.guaranteedActiveInterval
                         : record.guaranteedInactiveInterval;
            const ModuleAnalogInterval& high =
                lowState ? record.guaranteedInactiveInterval
                         : record.guaranteedActiveInterval;
            const bool hasAmbiguity =
                static_cast<uint32_t> (lowProved) + 1U <=
                static_cast<uint32_t> (highProved) - 1U;
            return sameInterval (low, true, record.rawDomain.minimum, lowProved) &&
                   sameInterval (high, true, highProved,
                                 record.rawDomain.maximum) &&
                   sameInterval (record.ambiguityInterval, hasAmbiguity,
                                 static_cast<uint16_t> (lowProved + 1U),
                                 static_cast<uint16_t> (highProved - 1U));
        }

        bool validSemanticRecord (const ModuleCharacterizationRecord& record) noexcept
        {
            const uint32_t pointCount =
                static_cast<uint32_t> (record.ascendingCount) +
                static_cast<uint32_t> (record.descendingCount) +
                static_cast<uint32_t> (record.verificationCount);
            const uint32_t sequenceDelta = record.lastSequence - record.firstSequence;
            const ModuleThresholdDescriptor descriptor = {
                record.descriptorSchemaRevision,
                record.descriptorId,
                record.descriptorRevision,
                record.declaredSpecimenReference,
                record.declaredSpecimenRevision,
                record.declaredElectricalEvidenceRevision,
                record.channelTopology,
                record.comparatorOutputStage,
                record.pullRequirement,
                record.declaredPullRail,
                record.declaredSupplyMillivolts,
                record.declaredSignalMillivolts,
                record.rawDomain,
                record.comparatorPolarity,
                record.thresholdControlKind,
                record.thresholdDirection,
                record.warmup,
                record.settling};

            if (!validateModuleThresholdDescriptor (descriptor).ok () ||
                record.recordSchemaRevision == 0 || record.benchRevision == 0 ||
                record.envelopeRevision == 0 || record.lifecycleGeneration == 0 ||
                record.sessionId == 0 || record.runId == 0 ||
                record.characterizationLifecycleGeneration == 0 ||
                record.characterizationRevision == 0 || record.sourceId == 0 ||
                record.sourceConfigurationRevision == 0 || record.ascendingCount > 16 ||
                record.descendingCount > 16 || record.verificationCount > 16 ||
                !validBracket (record.ascendingBracket, record.rawDomain, true) ||
                !validBracket (record.descendingBracket, record.rawDomain, false) ||

                !validInterval (record.guaranteedInactiveInterval, record.rawDomain) ||
                !validInterval (record.guaranteedActiveInterval, record.rawDomain) ||
                !validInterval (record.ambiguityInterval, record.rawDomain) ||

                !disjointIntervals (record.guaranteedInactiveInterval,
                                    record.guaranteedActiveInterval) ||
                !disjointIntervals (record.guaranteedInactiveInterval,
                                    record.ambiguityInterval) ||
                !disjointIntervals (record.guaranteedActiveInterval,
                                    record.ambiguityInterval) ||
                !validRelation (record.relation) ||

                !validState (record.terminalState) ||

                !validReason (record.terminalReason) ||
                !validStatus (record.terminalStatus) ||
                record.scriptStep != ModuleBenchScriptStep::PrepareRecord)
            {
                return false;
            }
            if ((pointCount == 0 &&
                 (record.firstSequence != 0 || record.lastSequence != 0)) ||
                (pointCount != 0 &&
                 (record.firstSequence == 0 || record.lastSequence == 0 ||
                  sequenceDelta >= 0x80000000UL)) ||
                (record.ascendingBracket.present && record.ascendingCount < 2) ||
                (record.descendingBracket.present && record.descendingCount < 2) ||
                ((record.guaranteedInactiveInterval.present ||
                  record.guaranteedActiveInterval.present ||
                  record.ambiguityInterval.present) &&
                 (!record.ascendingBracket.present ||
                  !record.descendingBracket.present)))
            {
                return false;
            }

            if (record.terminalState == ModuleCharacterizationState::Complete)
            {
                return record.terminalReason == ModuleCharacterizationReason::None &&
                       record.terminalStatus.ok () && record.ascendingCount >= 2 &&
                       record.descendingCount >= 2 && record.verificationCount >= 2 &&
                       record.ascendingBracket.present &&
                       record.descendingBracket.present &&
                       record.guaranteedInactiveInterval.present &&
                       record.guaranteedActiveInterval.present &&
                       validCompleteGeometry (record) &&
                       record.firstSequence != 0 && record.lastSequence != 0 &&
                       sequenceDelta != 0 &&
                       (record.relation == ModuleComparatorRelation::Consistent ||
                        (record.relation == ModuleComparatorRelation::Ambiguous &&
                         record.ambiguityInterval.present));
            }

            return record.terminalReason != ModuleCharacterizationReason::None;
        }

        void writeBracket (uint8_t*                    output,
                           const ModuleCompactBracket& bracket) noexcept
        {
            output[0] = bracket.present ? 1U : 0U;
            write16 (&output[1], bracket.beforeRaw);
            write16 (&output[3], bracket.afterRaw);
            output[5] = bracket.beforeAsserted ? 1U : 0U;
            output[6] = bracket.afterAsserted ? 1U : 0U;
            write32 (&output[7], bracket.beforeSequence);
            write32 (&output[11], bracket.afterSequence);
        }

        ModuleCompactBracket readBracket (const uint8_t* input) noexcept
        {
            return {input[0] != 0,      read16 (&input[1]), read16 (&input[3]),
                    input[5] != 0,      input[6] != 0,      read32 (&input[7]),

                    read32 (&input[11])};
        }

        void writeInterval (uint8_t*                    output,
                            const ModuleAnalogInterval& interval) noexcept
        {
            output[0] = interval.present ? 1U : 0U;
            write16 (&output[1], interval.lower);
            write16 (&output[3], interval.upper);
        }

        ModuleAnalogInterval readInterval (const uint8_t* input) noexcept
        {
            return {input[0] != 0, read16 (&input[1]), read16 (&input[3])};
        }
    } // namespace

    Result<uint16_t> ModuleCharacterizationRecordCodec::encode (
        const ModuleCharacterizationRecord& record,
        MutableByteSpan                     output) const noexcept
    {
        if (!validSemanticRecord (record))
        {
            return {StatusCode::InvalidArgument, 0};
        }
        if (output.data == nullptr ||
            output.capacity < ModuleCharacterizationRecordImage::size)
        {
            return {StatusCode::CapacityExceeded, 0};
        }

        uint8_t image[ModuleCharacterizationRecordImage::size] = {};
        for (uint8_t index = 0; index < sizeof magic; ++index)
        {
            image[index] = magic[index];
        }
        image[4] = ModuleCharacterizationRecordImage::version;
        write16 (&image[5], ModuleCharacterizationRecordImage::size);
        write16 (&image[7], record.recordSchemaRevision);
        write16 (&image[9], record.benchRevision);
        write32 (&image[11], record.lifecycleGeneration);
        write32 (&image[15], record.sessionId);
        write32 (&image[19], record.descriptorId);
        write16 (&image[23], record.descriptorRevision);
        write32 (&image[25], record.declaredSpecimenReference);
        write16 (&image[29], record.declaredSpecimenRevision);
        write16 (&image[31], record.declaredElectricalEvidenceRevision);
        image[33] = static_cast<uint8_t> (record.channelTopology);
        image[34] = static_cast<uint8_t> (record.comparatorOutputStage);
        image[35] = static_cast<uint8_t> (record.pullRequirement);
        image[36] = static_cast<uint8_t> (record.declaredPullRail);
        image[37] = static_cast<uint8_t> (record.comparatorPolarity);
        image[38] = static_cast<uint8_t> (record.thresholdControlKind);
        image[39] = static_cast<uint8_t> (record.thresholdDirection);
        write16 (&image[40], record.declaredSupplyMillivolts.minimum);
        write16 (&image[42], record.declaredSupplyMillivolts.maximum);
        write16 (&image[44], record.declaredSignalMillivolts.minimum);
        write16 (&image[46], record.declaredSignalMillivolts.maximum);
        write16 (&image[48], record.rawDomain.minimum);
        write16 (&image[50], record.rawDomain.maximum);
        image[52] = static_cast<uint8_t> (record.warmup.declaration);
        write32 (&image[53], record.warmup.value.milliseconds ());
        image[57] = static_cast<uint8_t> (record.settling.declaration);
        write32 (&image[58], record.settling.value.milliseconds ());
        write32 (&image[62], record.runId);
        write32 (&image[66], record.characterizationLifecycleGeneration);
        write16 (&image[70], record.characterizationRevision);
        image[72] = record.ascendingCount;
        image[73] = record.descendingCount;
        image[74] = record.verificationCount;
        writeBracket (&image[75], record.ascendingBracket);
        writeBracket (&image[90], record.descendingBracket);

        writeInterval (&image[105], record.guaranteedInactiveInterval);
        writeInterval (&image[110], record.guaranteedActiveInterval);
        writeInterval (&image[115], record.ambiguityInterval);
        image[120] = static_cast<uint8_t> (record.relation);
        write32 (&image[121], record.firstWitnessDigest);
        write32 (&image[125], record.lastWitnessDigest);
        write32 (&image[129], record.offendingBeforeDigest);
        write32 (&image[133], record.offendingAfterDigest);
        write32 (&image[137], record.firstSequence);
        write32 (&image[141], record.lastSequence);
        write32 (&image[145], record.descriptorDigest);
        write32 (&image[149], record.evidenceDigest);
        image[153] = static_cast<uint8_t> (record.terminalState);
        image[154] = static_cast<uint8_t> (record.terminalReason);
        image[155] = static_cast<uint8_t> (record.terminalStatus.error ());
        image[156] = static_cast<uint8_t> (record.scriptStep);
        write16 (&image[157], record.descriptorSchemaRevision);
        write16 (&image[159], record.envelopeRevision);
        image[161] = record.sourceId;
        write16 (&image[162], record.sourceConfigurationRevision);
        write16 (&image[integrityOffset], crc16 (image, integrityOffset));

        for (uint16_t index = 0; index < ModuleCharacterizationRecordImage::size;
             ++index)
        {
            output.data[index] = image[index];
        }
        return {StatusCode::Ok, ModuleCharacterizationRecordImage::size};
    }

    ModuleCharacterizationRecordValidity ModuleCharacterizationRecordCodec::decode (
        ByteSpan image, ModuleCharacterizationRecord& output) const noexcept
    {
        if (image.size != ModuleCharacterizationRecordImage::size)
        {
            return ModuleCharacterizationRecordValidity::BadLength;
        }
        if (image.data == nullptr)
        {
            return ModuleCharacterizationRecordValidity::BadSemanticValue;
        }
        for (uint8_t index = 0; index < sizeof magic; ++index)
        {
            if (image.data[index] != magic[index])
            {
                return ModuleCharacterizationRecordValidity::BadFraming;
            }
        }
        if (image.data[4] != ModuleCharacterizationRecordImage::version ||
            read16 (&image.data[5]) != ModuleCharacterizationRecordImage::size)
        {
            return ModuleCharacterizationRecordValidity::BadFraming;
        }
        if (read16 (&image.data[integrityOffset]) !=
            crc16 (image.data, integrityOffset))
        {
            return ModuleCharacterizationRecordValidity::BadIntegrity;
        }
        for (uint16_t index = 164; index < integrityOffset; ++index)
        {
            if (image.data[index] != 0)
            {
                return ModuleCharacterizationRecordValidity::BadSemanticValue;
            }
        }
        if (image.data[75] > 1 || image.data[80] > 1 || image.data[81] > 1 ||
            image.data[90] > 1 || image.data[95] > 1 || image.data[96] > 1 ||
            image.data[105] > 1 || image.data[110] > 1 || image.data[115] > 1)
        {
            return ModuleCharacterizationRecordValidity::BadSemanticValue;
        }

        const ModuleCharacterizationRecord candidate = {
            read16 (&image.data[7]),
            read16 (&image.data[9]),
            read16 (&image.data[157]),
            read16 (&image.data[159]),
            read32 (&image.data[11]),
            read32 (&image.data[15]),
            read32 (&image.data[19]),
            read16 (&image.data[23]),
            read32 (&image.data[25]),
            read16 (&image.data[29]),
            read16 (&image.data[31]),
            static_cast<ModuleChannelTopology> (image.data[33]),
            static_cast<ModuleComparatorOutputStage> (image.data[34]),
            static_cast<ModulePullRequirement> (image.data[35]),
            static_cast<ModuleDeclaredRail> (image.data[36]),
            static_cast<ModuleComparatorPolarity> (image.data[37]),
            static_cast<ModuleThresholdControlKind> (image.data[38]),
            static_cast<ModuleThresholdDirection> (image.data[39]),
            {read16 (&image.data[40]), read16 (&image.data[42])},
            {read16 (&image.data[44]), read16 (&image.data[46])},
            {read16 (&image.data[48]), read16 (&image.data[50])},
            {static_cast<ModuleDurationDeclaration> (image.data[52]),
             Duration (read32 (&image.data[53]))},
            {static_cast<ModuleDurationDeclaration> (image.data[57]),
             Duration (read32 (&image.data[58]))},
            read32 (&image.data[62]),
            read32 (&image.data[66]),
            read16 (&image.data[70]),
            image.data[161],
            read16 (&image.data[162]),
            image.data[72],
            image.data[73],
            image.data[74],
            readBracket (&image.data[75]),
            readBracket (&image.data[90]),

            readInterval (&image.data[105]),
            readInterval (&image.data[110]),
            readInterval (&image.data[115]),
            static_cast<ModuleComparatorRelation> (image.data[120]),
            read32 (&image.data[121]),
            read32 (&image.data[125]),
            read32 (&image.data[129]),
            read32 (&image.data[133]),
            read32 (&image.data[137]),
            read32 (&image.data[141]),
            read32 (&image.data[145]),
            read32 (&image.data[149]),
            static_cast<ModuleCharacterizationState> (image.data[153]),
            static_cast<ModuleCharacterizationReason> (image.data[154]),
            Status (static_cast<StatusCode> (image.data[155])),
            static_cast<ModuleBenchScriptStep> (image.data[156])};

        if (!validSemanticRecord (candidate))
        {
            return ModuleCharacterizationRecordValidity::BadSemanticValue;
        }
        output = candidate;
        return ModuleCharacterizationRecordValidity::Valid;
    }
} // namespace adk
