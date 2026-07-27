#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "telemetry_packet.h"

namespace {

    constexpr uint8_t packetSize = adk::TelemetryPacketCodec::size;

    uint16_t crc (const uint8_t* data, uint8_t size)
    {
        uint16_t value = 0xffff;

        for (uint8_t index = 0; index < size; ++index)
        {
            value ^= static_cast<uint16_t> (static_cast<uint16_t> (data[index]) << 8);

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                value = (value & 0x8000) != 0
                            ? static_cast<uint16_t> ((value << 1) ^ 0x1021)
                            : static_cast<uint16_t> (value << 1);
            }
        }

        return value;
    }

    void refreshCrc (uint8_t* packet)
    {
        const uint16_t value = crc (packet, adk::TelemetryPacketCodec::integrityOffset);
        packet[17]           = static_cast<uint8_t> (value >> 8);
        packet[18]           = static_cast<uint8_t> (value);
    }

    adk::TelemetrySample sample ()
    {
        return {0x1234,
                0xabcd,
                0x01020304,
                adk::TelemetryKind::Temperature,
                adk::SampleQuality::Valid,
                0,
                0};
    }

    void encode (const adk::TelemetrySample& value, uint8_t* packet)
    {
        const adk::Result<uint16_t> result =
            adk::TelemetryPacketCodec ().encode (value, {packet, packetSize});
        assert (result.ok ());
        assert (result.value () == packetSize);
    }

    void assertEqual (const adk::TelemetrySample& left,
                      const adk::TelemetrySample& right)
    {
        assert (left.sourceId == right.sourceId);
        assert (left.sequence == right.sequence);
        assert (left.observedMilliseconds == right.observedMilliseconds);
        assert (left.kind == right.kind);
        assert (left.quality == right.quality);
        assert (left.value == right.value);
        assert (left.decimalExponent == right.decimalExponent);
    }

    void testGoldenLayoutAndCrc ()
    {
        constexpr uint8_t expected[] = {0xad, 0x01, 0x12, 0x34, 0xab, 0xcd, 0x01,
                                        0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0xc8, 0x43};
        uint8_t           packet[packetSize] = {};

        encode (sample (), packet);

        assert (memcmp (packet, expected, sizeof (expected)) == 0);
    }

    void testGoldenValueExtremes ()
    {
        const adk::TelemetrySample values[] = {
            {0, 0, 0, adk::TelemetryKind::Counter, adk::SampleQuality::Valid, INT32_MIN,
             INT8_MIN},
            {UINT16_MAX, UINT16_MAX, UINT32_MAX, adk::TelemetryKind::Contact,
             adk::SampleQuality::StaleAtSource, -1, -1},
            {1, 2, 3, adk::TelemetryKind::Distance, adk::SampleQuality::OutOfRange,
             INT32_MAX, INT8_MAX}};

        for (const adk::TelemetrySample& expected : values)
        {
            uint8_t              packet[packetSize] = {};
            adk::TelemetrySample actual             = sample ();
            encode                                           (expected, packet);

            assert (adk::TelemetryPacketCodec ().decode (
                        {packet, packetSize}, actual) == adk::PacketValidity::Valid);
            assertEqual (actual, expected);
        }

        uint8_t packet[packetSize] = {};
        encode (values[0], packet);
        constexpr uint8_t minimumSigned[] = {0x80, 0x00, 0x00, 0x00, 0x80};
        assert (memcmp (&packet[adk::TelemetryPacketCodec::valueOffset], minimumSigned,
                        sizeof (minimumSigned)) == 0);

        encode (values[1], packet);
        constexpr uint8_t negativeOne[] = {0xff, 0xff, 0xff, 0xff, 0xff};
        assert (memcmp (&packet[adk::TelemetryPacketCodec::valueOffset], negativeOne,
                        sizeof (negativeOne)) == 0);
    }

    void testEveryLengthAndUnalignedStorage ()
    {
        uint8_t storage[packetSize + 2] = {};
        encode (sample (), &storage[1]);

        for (uint8_t size = 0; size < packetSize; ++size)
        {
            adk::TelemetrySample output = sample ();
            assert                               (adk::TelemetryPacketCodec ().decode ({&storage[1], size}, output) ==
                    adk::PacketValidity::BadLength);
            assertEqual (output, sample ());
        }

        adk::TelemetrySample output = {};
        assert (adk::TelemetryPacketCodec ().decode ({&storage[1], packetSize + 1},
                                                     output) ==
                adk::PacketValidity::TrailingData);
        assert (adk::TelemetryPacketCodec ().decode ({nullptr, packetSize}, output) ==
                adk::PacketValidity::BadLength);
        assert (adk::TelemetryPacketCodec ().decode (
                    {&storage[1], packetSize}, output) == adk::PacketValidity::Valid);
        assertEqual (output, sample ());
    }

    void testAllOneBitCorruption ()
    {
        uint8_t original[packetSize] = {};
        encode (sample (), original);

        for (uint8_t byte = 0; byte < packetSize; ++byte)
        {
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                uint8_t packet[packetSize];
                memcpy (packet, original, packetSize);
                packet[byte] ^= static_cast<uint8_t> (1u << bit);

                adk::TelemetrySample output = sample ();
                assert                               (adk::TelemetryPacketCodec ().decode ({packet, packetSize},
                                                             output) !=
                        adk::PacketValidity::Valid);
                assertEqual (output, sample ());
            }
        }
    }

    void testMalformedFields ()
    {
        uint8_t packet[packetSize] = {};
        encode (sample (), packet);

        packet[adk::TelemetryPacketCodec::magicOffset] = 0;
        refreshCrc                           (packet);
        adk::TelemetrySample output = sample ();
        assert                               (adk::TelemetryPacketCodec ().decode ({packet, packetSize}, output) ==
                adk::PacketValidity::BadVersion);

        encode (sample (), packet);
        packet[adk::TelemetryPacketCodec::versionOffset] = 2;
        refreshCrc (packet);
        assert     (adk::TelemetryPacketCodec ().decode ({packet, packetSize}, output) ==
                adk::PacketValidity::BadVersion);

        encode (sample (), packet);
        packet[adk::TelemetryPacketCodec::kindOffset] = 5;
        refreshCrc (packet);
        assert     (adk::TelemetryPacketCodec ().decode ({packet, packetSize}, output) ==
                adk::PacketValidity::BadType);

        encode (sample (), packet);
        packet[adk::TelemetryPacketCodec::qualityOffset] = 4;
        refreshCrc (packet);
        assert     (adk::TelemetryPacketCodec ().decode ({packet, packetSize}, output) ==
                adk::PacketValidity::BadQuality);
        assertEqual (output, sample ());
    }

    void testEncodeFailureNeverMutatesStorage ()
    {
        uint8_t storage[packetSize];

        for (uint8_t capacity = 0; capacity < packetSize; ++capacity)
        {
            memset (storage, 0x5a, sizeof (storage));
            const adk::Result<uint16_t> result =
                adk::TelemetryPacketCodec ().encode (sample (), {storage, capacity});
            assert (result.error () == adk::StatusCode::CapacityExceeded);
            assert (result.value () == 0);

            for (uint8_t index = 0; index < packetSize; ++index)
            {
                assert (storage[index] == 0x5a);
            }
        }

        const adk::Result<uint16_t> nullResult =
            adk::TelemetryPacketCodec ().encode (sample (), {nullptr, packetSize});
        assert (nullResult.error () == adk::StatusCode::CapacityExceeded);

        adk::TelemetrySample invalid = sample ();
        invalid.kind                 = static_cast<adk::TelemetryKind> (255);
        assert (adk::TelemetryPacketCodec ()
                    .encode (invalid, {storage, packetSize})
                    .error  () == adk::StatusCode::InvalidArgument);

        invalid         = sample ();
        invalid.quality = static_cast<adk::SampleQuality> (255);
        assert (adk::TelemetryPacketCodec ()
                    .encode (invalid, {storage, packetSize})
                    .error  () == adk::StatusCode::InvalidArgument);
    }

    void testDeterministicReplay ()
    {
        uint8_t first[packetSize]  = {};
        uint8_t second[packetSize] = {};

        encode (sample (), first);
        encode (sample (), second);

        assert (memcmp (first, second, packetSize) == 0);
    }
} // namespace

int main ()
{
    testGoldenLayoutAndCrc               ();
    testGoldenValueExtremes              ();
    testEveryLengthAndUnalignedStorage   ();
    testAllOneBitCorruption              ();
    testMalformedFields                  ();
    testEncodeFailureNeverMutatesStorage ();
    testDeterministicReplay              ();
}
