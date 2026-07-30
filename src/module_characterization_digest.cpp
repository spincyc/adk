#include "module_characterization_digest.h"

namespace adk {

    namespace {

        struct DigestEncoder
        {
            uint32_t value;

            explicit DigestEncoder (const char* domain) noexcept
                : value (UINT32_C (0xffffffff))
            {
                while (*domain != '\0')
                {
                    byte (static_cast<uint8_t> (*domain));
                    ++domain;
                }
            }

            void byte (uint8_t input) noexcept
            {
                value ^= input;
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    value = (value >> 1U) ^
                            ((value & 1U) != 0 ? UINT32_C (0xedb88320) : 0U);
                }
            }

            void word (uint16_t input) noexcept
            {
                byte (static_cast<uint8_t> (input));
                byte (static_cast<uint8_t> (input >> 8U));
            }

            void doubleWord (uint32_t input) noexcept
            {
                byte (static_cast<uint8_t> (input));
                byte (static_cast<uint8_t> (input >> 8U));
                byte (static_cast<uint8_t> (input >> 16U));
                byte (static_cast<uint8_t> (input >> 24U));
            }

            uint32_t finish () const noexcept
            {
                return value ^ UINT32_C (0xffffffff);
            }
        };

        void encodeDuration (DigestEncoder&                encoder,
                             const ModuleDeclaredDuration& duration) noexcept
        {
            encoder.byte (static_cast<uint8_t> (duration.declaration));

            encoder.doubleWord (duration.value.milliseconds ());
        }

        void encodeDescriptor (DigestEncoder&                   encoder,
                               const ModuleThresholdDescriptor& descriptor) noexcept
        {
            encoder.word (descriptor.schemaRevision);

            encoder.doubleWord (descriptor.descriptorId);

            encoder.word (descriptor.descriptorRevision);

            encoder.doubleWord (descriptor.declaredSpecimenReference);

            encoder.word (descriptor.declaredSpecimenRevision);
            encoder.word (descriptor.declaredElectricalEvidenceRevision);
            encoder.byte (static_cast<uint8_t> (descriptor.channelTopology));
            encoder.byte (static_cast<uint8_t> (descriptor.comparatorOutputStage));
            encoder.byte (static_cast<uint8_t> (descriptor.pullRequirement));
            encoder.byte (static_cast<uint8_t> (descriptor.declaredPullRail));
            encoder.word (descriptor.declaredSupplyMillivolts.minimum);
            encoder.word (descriptor.declaredSupplyMillivolts.maximum);
            encoder.word (descriptor.declaredSignalMillivolts.minimum);
            encoder.word (descriptor.declaredSignalMillivolts.maximum);
            encoder.word (descriptor.rawDomain.minimum);
            encoder.word (descriptor.rawDomain.maximum);
            encoder.byte (static_cast<uint8_t> (descriptor.comparatorPolarity));
            encoder.byte (static_cast<uint8_t> (descriptor.thresholdControlKind));
            encoder.byte (static_cast<uint8_t> (descriptor.thresholdDirection));

            encodeDuration (encoder, descriptor.warmup);
            encodeDuration (encoder, descriptor.settling);
        }

        void encodeFrame (DigestEncoder&              encoder,
                          const ModuleThresholdFrame& frame) noexcept
        {
            encoder.word (frame.schemaRevision);

            encoder.doubleWord (frame.descriptorId);

            encoder.word (frame.descriptorRevision);

            encoder.doubleWord (frame.declaredSpecimenReference);

            encoder.word (frame.declaredSpecimenRevision);
            encoder.word (frame.declaredElectricalEvidenceRevision);
            encoder.byte (frame.provenance.sourceId);
            encoder.word (frame.provenance.sourceConfigurationRevision);

            encoder.doubleWord (frame.provenance.sequence);
            encoder.doubleWord (frame.provenance.observedAt.milliseconds ());

            encoder.word (frame.analogRaw);
            encoder.byte (static_cast<uint8_t> (frame.analogStatus));
            encoder.byte (frame.comparatorLevelHigh ? 1U : 0U);
            encoder.byte (static_cast<uint8_t> (frame.comparatorStatus));
            encoder.byte (frame.comparatorPresent ? 1U : 0U);
            encoder.byte (frame.comparatorAsserted ? 1U : 0U);
            encoder.byte (frame.declaredWarmupSatisfied ? 1U : 0U);
            encoder.byte (frame.declaredSettlingSatisfied ? 1U : 0U);
            encoder.byte (static_cast<uint8_t> (frame.analogProducerStatus.error ()));
            encoder.byte (
                static_cast<uint8_t> (frame.comparatorProducerStatus.error ()));
        }

        void encodePoint (DigestEncoder&                     encoder,
                          const ModuleCharacterizationPoint& point) noexcept
        {
            encoder.doubleWord (point.sessionId);
            encoder.doubleWord (point.runId);
            encoder.doubleWord (point.legId);

            encoder.word (point.controlOrdinal);
            encoder.byte (static_cast<uint8_t> (point.leg));
            encoder.byte (static_cast<uint8_t> (point.direction));
            encoder.byte (point.sourceId);
            encoder.word (point.sourceConfigurationRevision);

            encodeFrame (encoder, point.frame);
        }

        void encodeZeroPoint (DigestEncoder& encoder) noexcept
        {
            encoder.doubleWord (0);
            encoder.doubleWord (0);
            encoder.doubleWord (0);

            encoder.word (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.word (0);
            encoder.word (0);

            encoder.doubleWord (0);

            encoder.word (0);

            encoder.doubleWord (0);

            encoder.word (0);
            encoder.word (0);
            encoder.byte (0);
            encoder.word (0);

            encoder.doubleWord (0);
            encoder.doubleWord (0);

            encoder.word (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
            encoder.byte (0);
        }

        void encodeBracket (DigestEncoder&                 encoder,
                            const ModuleTransitionBracket& bracket) noexcept
        {
            encoder.byte (bracket.present ? 1U : 0U);
            if (bracket.present)
            {
                encodePoint (encoder, bracket.before);
                encodePoint (encoder, bracket.after);
            }
            else
            {
                encodeZeroPoint (encoder);
                encodeZeroPoint (encoder);
            }
        }

        void encodeInterval (DigestEncoder&              encoder,
                             const ModuleAnalogInterval& interval) noexcept
        {
            encoder.byte (interval.present ? 1U : 0U);
            encoder.word (interval.present ? interval.lower : 0U);
            encoder.word (interval.present ? interval.upper : 0U);
        }

        void encodeWitness (DigestEncoder&              encoder,
                            const ModuleCompactWitness& witness) noexcept
        {
            encoder.byte (witness.present ? 1U : 0U);
            encoder.word (witness.present ? witness.controlOrdinal : 0U);
            encoder.word (witness.present ? witness.analogRaw : 0U);
            encoder.byte (witness.present && witness.comparatorAsserted ? 1U : 0U);

            encoder.doubleWord (witness.present ? witness.sequence : 0U);
            encoder.doubleWord (witness.present ? witness.observedAt.milliseconds ()
                                                : 0U);
        }
    } // namespace

    uint32_t moduleThresholdDescriptorDigest (
        const ModuleThresholdDescriptor& descriptor) noexcept
    {
        DigestEncoder encoder ("ADK-MOD-DESC-1");

        encodeDescriptor (encoder, descriptor);

        return encoder.finish ();
    }

    uint32_t moduleCompactWitnessDigest (const ModuleCompactWitness& witness) noexcept
    {
        DigestEncoder encoder ("ADK-MOD-WIT-1");

        encodeWitness (encoder, witness);

        return encoder.finish ();
    }

    uint32_t moduleCharacterizationEvidenceDigest (
        const ModuleCharacterizationEvidence& evidence) noexcept
    {
        DigestEncoder encoder ("ADK-MOD-EVID-1");

        encoder.doubleWord (evidence.lifecycleGeneration);
        encoder.doubleWord (evidence.sessionId);
        encoder.doubleWord (evidence.runId);
        encoder.doubleWord (evidence.legId);

        encoder.word (evidence.characterizationRevision);

        encodeDescriptor (encoder, evidence.descriptor);

        encoder.byte (evidence.sourceId);
        encoder.word (evidence.sourceConfigurationRevision);
        encoder.byte (static_cast<uint8_t> (evidence.state));
        encoder.byte (static_cast<uint8_t> (evidence.reason));
        encoder.byte (static_cast<uint8_t> (evidence.terminalLeg));
        encoder.byte (evidence.ascendingCount);
        encoder.byte (evidence.descendingCount);
        encoder.byte (evidence.verificationCount);

        encodeBracket (encoder, evidence.ascendingBracket);
        encodeBracket (encoder, evidence.descendingBracket);

        encodeInterval (encoder, evidence.guaranteedInactiveInterval);
        encodeInterval (encoder, evidence.guaranteedActiveInterval);
        encodeInterval (encoder, evidence.ambiguityInterval);

        encoder.byte (static_cast<uint8_t> (evidence.relation));

        encodeWitness (encoder, evidence.firstWitness);
        encodeWitness (encoder, evidence.lastWitness);
        encodeWitness (encoder, evidence.offendingBefore);
        encodeWitness (encoder, evidence.offendingAfter);

        encoder.byte (static_cast<uint8_t> (evidence.status.error ()));

        return encoder.finish ();
    }
} // namespace adk
