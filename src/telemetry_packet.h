#pragma once

#include <stdint.h>

#include "bounded_span.h"
#include "status.h"

namespace adk {

    enum struct TelemetryKind : uint8_t
    {
        Temperature,
        RelativeHumidity,
        Distance,
        Contact,
        Counter
    };

    enum struct SampleQuality : uint8_t
    {
        Valid,
        SensorFault,
        OutOfRange,
        StaleAtSource
    };

    struct TelemetrySample
    {
        uint16_t      sourceId;
        uint16_t      sequence;
        uint32_t      observedMilliseconds;
        TelemetryKind kind;
        SampleQuality quality;
        int32_t       value;
        int8_t        decimalExponent;
    };

    enum struct PacketValidity : uint8_t
    {
        Valid,
        BadVersion,
        BadLength,
        BadType,
        BadQuality,
        BadIntegrity,
        TrailingData
    };

    struct TelemetryPacketCodec
    {
        static constexpr uint8_t magic   = 0xad;
        static constexpr uint8_t version = 1;
        static constexpr uint8_t size    = 19;

        static constexpr uint8_t magicOffset                = 0;
        static constexpr uint8_t versionOffset              = 1;
        static constexpr uint8_t sourceIdOffset             = 2;
        static constexpr uint8_t sequenceOffset             = 4;
        static constexpr uint8_t observedMillisecondsOffset = 6;
        static constexpr uint8_t kindOffset                 = 10;
        static constexpr uint8_t qualityOffset              = 11;
        static constexpr uint8_t valueOffset                = 12;
        static constexpr uint8_t decimalExponentOffset      = 16;
        static constexpr uint8_t integrityOffset            = 17;

        static constexpr uint16_t crcPolynomial   = 0x1021;
        static constexpr uint16_t crcInitialValue = 0xffff;
        static constexpr uint16_t crcFinalXor     = 0x0000;

        Result<uint16_t> encode (const TelemetrySample& sample,
                                 MutableByteSpan        output) const noexcept;
        PacketValidity decode (ByteSpan packet, TelemetrySample& output) const noexcept;
    };

    static_assert (TelemetryPacketCodec::versionOffset ==
                       TelemetryPacketCodec::magicOffset + 1,
                   "version follows magic");
    static_assert (TelemetryPacketCodec::sourceIdOffset ==
                       TelemetryPacketCodec::versionOffset + 1,
                   "source follows version");
    static_assert (TelemetryPacketCodec::sequenceOffset ==
                       TelemetryPacketCodec::sourceIdOffset + 2,
                   "sequence follows source");
    static_assert (TelemetryPacketCodec::observedMillisecondsOffset ==
                       TelemetryPacketCodec::sequenceOffset + 2,
                   "observation time follows sequence");
    static_assert (TelemetryPacketCodec::kindOffset ==
                       TelemetryPacketCodec::observedMillisecondsOffset + 4,
                   "kind follows observation time");
    static_assert (TelemetryPacketCodec::qualityOffset ==
                       TelemetryPacketCodec::kindOffset + 1,
                   "quality follows kind");
    static_assert (TelemetryPacketCodec::valueOffset ==
                       TelemetryPacketCodec::qualityOffset + 1,
                   "value follows quality");
    static_assert (TelemetryPacketCodec::decimalExponentOffset ==
                       TelemetryPacketCodec::valueOffset + 4,
                   "scale follows value");
    static_assert (TelemetryPacketCodec::integrityOffset ==
                       TelemetryPacketCodec::decimalExponentOffset + 1,
                   "integrity follows scale");
    static_assert (TelemetryPacketCodec::size ==
                       TelemetryPacketCodec::integrityOffset + 2,
                   "packet ends after integrity");
} // namespace adk
