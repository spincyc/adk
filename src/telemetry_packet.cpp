#include "telemetry_packet.h"

namespace adk {

    namespace {

        bool validKind (TelemetryKind kind) noexcept
        {
            switch (kind)
            {
                case TelemetryKind::Temperature: return true;
                case TelemetryKind::RelativeHumidity: return true;
                case TelemetryKind::Distance: return true;
                case TelemetryKind::Contact: return true;
                case TelemetryKind::Counter: return true;
            }

            return false;
        }

        bool validQuality (SampleQuality quality) noexcept
        {
            switch (quality)
            {
                case SampleQuality::Valid: return true;
                case SampleQuality::SensorFault: return true;
                case SampleQuality::OutOfRange: return true;
                case SampleQuality::StaleAtSource: return true;
            }

            return false;
        }

        void writeUint16 (uint8_t* output, uint16_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value >> 8);
            output[1] = static_cast<uint8_t> (value);
        }

        void writeUint32 (uint8_t* output, uint32_t value) noexcept
        {
            output[0] = static_cast<uint8_t> (value >> 24);
            output[1] = static_cast<uint8_t> (value >> 16);
            output[2] = static_cast<uint8_t> (value >> 8);
            output[3] = static_cast<uint8_t> (value);
        }

        uint16_t readUint16 (const uint8_t* input) noexcept
        {
            return static_cast<uint16_t> (static_cast<uint16_t> (input[0]) << 8 |
                                          static_cast<uint16_t> (input[1]));
        }

        uint32_t readUint32 (const uint8_t* input) noexcept
        {
            return static_cast<uint32_t> (input[0]) << 24 |
                   static_cast<uint32_t> (input[1]) << 16 |
                   static_cast<uint32_t> (input[2]) << 8 |
                   static_cast<uint32_t> (input[3]);
        }

        int32_t readInt32 (const uint8_t* input) noexcept
        {
            const uint32_t value = readUint32 (input);

            if (value <= INT32_MAX)
            {
                return static_cast<int32_t> (value);
            }

            return -1 - static_cast<int32_t> (~value);
        }

        int8_t readInt8 (uint8_t input) noexcept
        {
            if (input <= INT8_MAX)
            {
                return static_cast<int8_t> (input);
            }

            return static_cast<int8_t> (
                -1 - static_cast<int16_t> (static_cast<uint8_t> (~input)));
        }

        uint16_t packetCrc (const uint8_t* data, uint8_t size) noexcept
        {
            uint16_t crc = TelemetryPacketCodec::crcInitialValue;

            for (uint8_t index = 0; index < size; ++index)
            {
                crc ^= static_cast<uint16_t> (static_cast<uint16_t> (data[index]) << 8);

                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & 0x8000) != 0
                              ? static_cast<uint16_t> (
                                    (crc << 1) ^ TelemetryPacketCodec::crcPolynomial)
                              : static_cast<uint16_t> (crc << 1);
                }
            }

            return static_cast<uint16_t> (crc ^ TelemetryPacketCodec::crcFinalXor);
        }
    } // namespace

    Result<uint16_t>
    TelemetryPacketCodec::encode (const TelemetrySample& sample,
                                  MutableByteSpan        output) const noexcept
    {
        if (!validKind (sample.kind) || !validQuality (sample.quality))
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }

        if (output.data == nullptr || output.capacity < size)
        {
            return Result<uint16_t> (StatusCode::CapacityExceeded, 0);
        }

        uint8_t packet[size];

        packet[magicOffset]   = magic;
        packet[versionOffset] = version;
        writeUint16 (&packet[sourceIdOffset], sample.sourceId);
        writeUint16 (&packet[sequenceOffset], sample.sequence);
        writeUint32 (&packet[observedMillisecondsOffset], sample.observedMilliseconds);
        packet[kindOffset]    = static_cast<uint8_t> (sample.kind);
        packet[qualityOffset] = static_cast<uint8_t> (sample.quality);
        writeUint32 (&packet[valueOffset], static_cast<uint32_t> (sample.value));
        packet[decimalExponentOffset] = static_cast<uint8_t> (sample.decimalExponent);

        const uint16_t crc = packetCrc (packet, integrityOffset);
        writeUint16                    (&packet[integrityOffset], crc);

        for (uint8_t index = 0; index < size; ++index)
        {
            output.data[index] = packet[index];
        }

        return Result<uint16_t> (StatusCode::Ok, size);
    }

    PacketValidity TelemetryPacketCodec::decode (ByteSpan         packet,
                                                 TelemetrySample& output) const noexcept
    {
        if (packet.size < size || packet.data == nullptr)
        {
            return PacketValidity::BadLength;
        }

        if (packet.size > size)
        {
            return PacketValidity::TrailingData;
        }

        if (packet.data[magicOffset] != magic || packet.data[versionOffset] != version)
        {
            return PacketValidity::BadVersion;
        }

        const TelemetryKind kind = static_cast<TelemetryKind> (packet.data[kindOffset]);
        if (!validKind (kind))
        {
            return PacketValidity::BadType;
        }

        const SampleQuality quality =
            static_cast<SampleQuality> (packet.data[qualityOffset]);
        if (!validQuality (quality))
        {
            return PacketValidity::BadQuality;
        }

        const uint16_t expectedCrc = packetCrc  (packet.data, integrityOffset);
        const uint16_t receivedCrc = readUint16 (&packet.data[integrityOffset]);
        if (receivedCrc != expectedCrc)
        {
            return PacketValidity::BadIntegrity;
        }

        const TelemetrySample decoded = {
            readUint16 (&packet.data[sourceIdOffset]),
            readUint16 (&packet.data[sequenceOffset]),
            readUint32 (&packet.data[observedMillisecondsOffset]),
            kind,
            quality,
            readInt32 (&packet.data[valueOffset]),
            readInt8  (packet.data[decimalExponentOffset])};

        output = decoded;
        return PacketValidity::Valid;
    }
} // namespace adk
