#pragma once

#include "bounded_span.h"
#include "inertial_observation.h"

#include <stdint.h>

namespace adk {

    enum struct InertialRecordState : uint8_t
    {
        Recorded,
        NotReady,
        SourceFault
    };

    struct InertialRecordConfig
    {
        uint16_t schemaRevision;
        uint16_t normalizationRevision;
    };

    struct InertialRecord
    {
        uint16_t            schemaRevision;
        uint16_t            normalizationRevision;
        InertialSource      source;
        InertialVector      accelerationMicroG;
        InertialVector      angularRateMilliDegreesPerSecond;
        TimePoint           observedAt;
        uint32_t            sequence;
        bool                dataReady;
        InertialSaturation  saturation;
        Status              producerStatus;
        InertialRecordState state;
    };

    struct InertialRecordNormalizer
    {
        explicit InertialRecordNormalizer (
            const InertialRecordConfig& config) noexcept;

        Status normalize (const InertialSample& sample,
                          InertialRecord&       output) const noexcept;

      private:
        InertialRecordConfig config_;
    };

    enum struct InertialRecordValidity : uint8_t
    {
        Valid,
        BadLength,
        BadFraming,
        BadIntegrity,
        BadSemanticValue
    };

    struct InertialRecordCodec
    {
        static constexpr uint8_t version = 1;
        static constexpr uint8_t size    = 64;

        static constexpr uint8_t magicOffset                 = 0;
        static constexpr uint8_t versionOffset               = 4;
        static constexpr uint8_t lengthOffset                = 5;
        static constexpr uint8_t schemaRevisionOffset        = 6;
        static constexpr uint8_t normalizationRevisionOffset = 8;
        static constexpr uint8_t stateOffset                 = 10;
        static constexpr uint8_t sourceKindOffset            = 11;
        static constexpr uint8_t modelOffset                 = 12;
        static constexpr uint8_t sourceIdOffset              = 13;
        static constexpr uint8_t configurationRevisionOffset = 14;
        static constexpr uint8_t calibrationRevisionOffset   = 16;
        static constexpr uint8_t reservedOffset              = 18;
        static constexpr uint8_t accelerationRangeOffset     = 20;
        static constexpr uint8_t angularRateRangeOffset      = 24;
        static constexpr uint8_t accelerationOffset          = 28;
        static constexpr uint8_t angularRateOffset           = 40;
        static constexpr uint8_t observedAtOffset            = 52;
        static constexpr uint8_t sequenceOffset              = 56;
        static constexpr uint8_t flagsOffset                 = 60;
        static constexpr uint8_t producerStatusOffset        = 61;
        static constexpr uint8_t integrityOffset             = 62;

        static constexpr uint8_t dataReadyFlag       = 0x01;
        static constexpr uint8_t saturationShift     = 1;
        static constexpr uint8_t saturationMask      = 0x06;
        static constexpr uint8_t knownFlagsMask      = 0x07;
        static constexpr uint16_t crcPolynomial      = 0x1021;
        static constexpr uint16_t crcInitialValue    = 0xffff;
        static constexpr uint16_t crcFinalXor        = 0x0000;

        Result<uint16_t> encode (const InertialRecord& record,
                                 MutableByteSpan       output) const noexcept;
        InertialRecordValidity decode (ByteSpan        image,
                                       InertialRecord& output) const noexcept;
    };

    static_assert (InertialRecordCodec::integrityOffset + 2 ==
                       InertialRecordCodec::size,
                   "inertial record image ends after its checksum");
} // namespace adk
