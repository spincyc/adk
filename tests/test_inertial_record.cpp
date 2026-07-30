#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "inertial_record.h"

namespace {

    adk::InertialRecord emptyRecord ()
    {
        return {0,
                0,
                {adk::InertialSourceKind::SyntheticFixture,
                 adk::InertialModel::Synthetic, 0, 0, 0, 0, 0},
                {0, 0, 0},
                {0, 0, 0},
                adk::TimePoint (0),
                0,
                false,
                adk::InertialSaturation::None,
                adk::StatusCode::Ok,
                adk::InertialRecordState::NotReady};
    }

    adk::InertialSample sample ()
    {
        return {{adk::InertialSourceKind::SyntheticFixture,
                 adk::InertialModel::Synthetic, 7, 3, 4, 2000000, 250000},
                {-2000000, 17, 2000000},
                {-250000, 19, 250000},
                adk::TimePoint (0x78563412U),
                0xf0e1d2c3U,
                true,
                adk::InertialSaturation::Both,
                adk::StatusCode::Ok};
    }

    adk::InertialRecord record ()
    {
        adk::InertialRecord output = emptyRecord ();

        assert (
            adk::InertialRecordNormalizer ({2, 5}).normalize (sample (), output).ok ());
        return output;
    }

    bool equal (const adk::InertialRecord& left, const adk::InertialRecord& right)
    {
        return left.schemaRevision == right.schemaRevision &&
               left.normalizationRevision == right.normalizationRevision &&
               left.source.kind == right.source.kind &&
               left.source.model == right.source.model &&
               left.source.sourceId == right.source.sourceId &&
               left.source.configurationRevision ==
                   right.source.configurationRevision &&
               left.source.calibrationRevision == right.source.calibrationRevision &&
               left.source.accelerationRangeMicroG ==
                   right.source.accelerationRangeMicroG &&
               left.source.angularRateRangeMilliDegreesPerSecond ==
                   right.source.angularRateRangeMilliDegreesPerSecond &&
               left.accelerationMicroG.x == right.accelerationMicroG.x &&
               left.accelerationMicroG.y == right.accelerationMicroG.y &&
               left.accelerationMicroG.z == right.accelerationMicroG.z &&
               left.angularRateMilliDegreesPerSecond.x ==
                   right.angularRateMilliDegreesPerSecond.x &&
               left.angularRateMilliDegreesPerSecond.y ==
                   right.angularRateMilliDegreesPerSecond.y &&
               left.angularRateMilliDegreesPerSecond.z ==
                   right.angularRateMilliDegreesPerSecond.z &&
               left.observedAt == right.observedAt && left.sequence == right.sequence &&
               left.dataReady == right.dataReady &&
               left.saturation == right.saturation &&
               left.producerStatus == right.producerStatus && left.state == right.state;
    }

    uint16_t crc16 (const uint8_t* bytes, uint8_t length)
    {
        uint16_t crc = adk::InertialRecordCodec::crcInitialValue;
        for (uint8_t index = 0; index < length; ++index)
        {
            crc ^= static_cast<uint16_t> (static_cast<uint16_t> (bytes[index]) << 8);
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                crc = (crc & 0x8000U) != 0U
                          ? static_cast<uint16_t> (
                                (crc << 1) ^ adk::InertialRecordCodec::crcPolynomial)
                          : static_cast<uint16_t> (crc << 1);
            }
        }
        return static_cast<uint16_t> (crc ^ adk::InertialRecordCodec::crcFinalXor);
    }

    void repairCrc (uint8_t* image)
    {
        const uint16_t crc = crc16 (image, adk::InertialRecordCodec::integrityOffset);
        image[adk::InertialRecordCodec::integrityOffset] = static_cast<uint8_t> (crc);
        image[adk::InertialRecordCodec::integrityOffset + 1] =
            static_cast<uint8_t> (crc >> 8);
    }

    void encode (const adk::InertialRecord& value, uint8_t* image)
    {
        const adk::Result<uint16_t> result = adk::InertialRecordCodec ().encode (
            value, {image, adk::InertialRecordCodec::size});
        assert (result.ok ());
        assert (result.value () == adk::InertialRecordCodec::size);
    }

    void testNormalizerPreservesReadyEvidence ()
    {
        const adk::InertialSample input  = sample ();

        adk::InertialRecord       output = emptyRecord ();

        assert (adk::InertialRecordNormalizer ({2, 5}).normalize (input, output).ok ());
        assert (output.schemaRevision == 2);
        assert (output.normalizationRevision == 5);
        assert (output.source.kind == input.source.kind);
        assert (output.source.model == input.source.model);
        assert (output.accelerationMicroG.x == input.accelerationMicroG.x);
        assert (output.angularRateMilliDegreesPerSecond.z ==
                input.angularRateMilliDegreesPerSecond.z);
        assert (output.observedAt == input.observedAt);
        assert (output.sequence == input.sequence);
        assert (output.dataReady);
        assert (output.saturation == adk::InertialSaturation::Both);
        assert (output.producerStatus.ok ());
        assert (output.state == adk::InertialRecordState::Recorded);
    }

    void testNormalizerClassifiesNotReadyAndSourceFault ()
    {
        adk::InertialSample input  = sample ();

        adk::InertialRecord output = emptyRecord ();

        input.dataReady  = false;
        input.saturation = adk::InertialSaturation::None;
        assert (adk::InertialRecordNormalizer ({2, 5}).normalize (input, output).ok ());
        assert (output.state == adk::InertialRecordState::NotReady);
        assert (!output.dataReady);
        assert (output.accelerationMicroG.x == 0);
        assert (output.accelerationMicroG.y == 0);
        assert (output.accelerationMicroG.z == 0);
        assert (output.angularRateMilliDegreesPerSecond.x == 0);
        assert (output.angularRateMilliDegreesPerSecond.y == 0);
        assert (output.angularRateMilliDegreesPerSecond.z == 0);
        assert (output.saturation == adk::InertialSaturation::None);
        assert (output.producerStatus.ok ());

        input.dataReady = true;
        input.status    = adk::StatusCode::HardwareFailure;
        assert (adk::InertialRecordNormalizer ({2, 5}).normalize (input, output).ok ());
        assert (output.state == adk::InertialRecordState::SourceFault);
        assert (!output.dataReady);
        assert (output.accelerationMicroG.x == 0);
        assert (output.angularRateMilliDegreesPerSecond.z == 0);
        assert (output.saturation == adk::InertialSaturation::None);
        assert (output.producerStatus.error () == adk::StatusCode::HardwareFailure);
    }

    void testNormalizerRejectsInvalidConfigurationAtomically ()
    {
        const adk::InertialRecord       sentinel  = record ();
        const adk::InertialRecordConfig invalid[] = {{0, 1}, {1, 0}};

        for (uint8_t index = 0; index < 2; ++index)
        {
            adk::InertialRecord output = sentinel;

            assert (adk::InertialRecordNormalizer (invalid[index])
                        .normalize (sample (), output)
                        .error     () == adk::StatusCode::InvalidConfiguration);
            assert (equal (output, sentinel));
        }
    }

    void testNormalizerRejectsEveryInvalidEnumAtomically ()
    {
        const adk::InertialRecord sentinel = record ();
        for (uint16_t value = 3; value <= UINT8_MAX; ++value)
        {
            adk::InertialSample input  = sample ();
            input.source.kind          = static_cast<adk::InertialSourceKind> (value);
            adk::InertialRecord output = sentinel;

            assert (adk::InertialRecordNormalizer ({2, 5})
                        .normalize (input, output)
                        .error     () == adk::StatusCode::InvalidArgument);
            assert (equal (output, sentinel));

            input              = sample ();
            input.source.model = static_cast<adk::InertialModel> (value);
            output             = sentinel;

            assert (adk::InertialRecordNormalizer ({2, 5})
                        .normalize (input, output)
                        .error     () == adk::StatusCode::InvalidArgument);
            assert (equal (output, sentinel));
        }

        for (uint16_t value = 4; value <= UINT8_MAX; ++value)
        {
            adk::InertialSample input  = sample ();
            input.saturation           = static_cast<adk::InertialSaturation> (value);
            adk::InertialRecord output = sentinel;

            assert (adk::InertialRecordNormalizer ({2, 5})
                        .normalize (input, output)
                        .error     () == adk::StatusCode::InvalidArgument);
            assert (equal (output, sentinel));
        }

        for (uint16_t value = 11; value <= UINT8_MAX; ++value)
        {
            adk::InertialSample input = sample ();

            input.status = adk::Status (static_cast<adk::StatusCode> (value));

            adk::InertialRecord output = sentinel;

            assert (adk::InertialRecordNormalizer ({2, 5})
                        .normalize (input, output)
                        .error     () == adk::StatusCode::InvalidArgument);
            assert (equal (output, sentinel));
        }
    }

    void testSourcePairMatrixAndProducerStatuses ()
    {
        const adk::InertialSourceKind kinds[] = {
            adk::InertialSourceKind::SyntheticFixture,
            adk::InertialSourceKind::Mpu6050Adapter,
            adk::InertialSourceKind::Qmi8658Adapter};
        const adk::InertialModel models[] = {
            adk::InertialModel::Synthetic, adk::InertialModel::Mpu6050,
            adk::InertialModel::Qmi8658UnknownRevision};

        for (uint8_t kind = 0; kind < 3; ++kind)
        {
            for (uint8_t model = 0; model < 3; ++model)
            {
                adk::InertialSample input  = sample ();
                input.source.kind          = kinds[kind];
                input.source.model         = models[model];
                adk::InertialRecord output = record ();
                const adk::Status   result =
                    adk::InertialRecordNormalizer ({2, 5}).normalize (input, output);

                assert (result.ok () == (kind == model));
            }
        }

        for (uint8_t value = 1;
             value <= static_cast<uint8_t> (adk::StatusCode::HardwareFailure); ++value)
        {
            adk::InertialSample input = sample ();

            input.status = adk::Status (static_cast<adk::StatusCode> (value));

            adk::InertialRecord output = emptyRecord ();

            assert (
                adk::InertialRecordNormalizer ({2, 5}).normalize (input, output).ok ());
            assert (output.state == adk::InertialRecordState::SourceFault);
            assert (output.producerStatus.error () ==
                    static_cast<adk::StatusCode> (value));
        }
    }

    void testStructuralBoundariesAreTransactional ()
    {
        const adk::InertialRecord sentinel = record ();
        for (uint8_t field = 0; field < 7; ++field)
        {
            adk::InertialSample input = sample ();
            switch (field)
            {
                case 0: input.source.sourceId = 0; break;
                case 1: input.source.configurationRevision = 0; break;
                case 2: input.source.calibrationRevision = 0; break;
                case 3: input.source.accelerationRangeMicroG = 0; break;
                case 4: input.source.angularRateRangeMilliDegreesPerSecond = 0; break;
                case 5:
                    input.accelerationMicroG.x =
                        static_cast<int32_t> (input.source.accelerationRangeMicroG + 1);
                    break;
                case 6:
                    input.angularRateMilliDegreesPerSecond.x = static_cast<int32_t> (
                        input.source.angularRateRangeMilliDegreesPerSecond + 1);
                    break;
            }
            adk::InertialRecord output = sentinel;

            assert (adk::InertialRecordNormalizer ({2, 5})
                        .normalize (input, output)
                        .error     () == adk::StatusCode::InvalidArgument);
            assert (equal (output, sentinel));
        }
    }

    void testCanonicalLittleEndianImageAndRoundTrip ()
    {
        const adk::InertialRecord input                                  = record ();
        uint8_t                   first[adk::InertialRecordCodec::size]  = {};
        uint8_t                   second[adk::InertialRecordCodec::size] = {};
        encode (input, first);
        encode (input, second);
        assert (memcmp (first, second, sizeof first) == 0);

        assert (memcmp (first, "IR67", 4) == 0);
        assert (first[4] == adk::InertialRecordCodec::version);
        assert (first[5] == adk::InertialRecordCodec::size);
        assert (first[6] == 2 && first[7] == 0);
        assert (first[8] == 5 && first[9] == 0);
        assert (first[10] == static_cast<uint8_t> (adk::InertialRecordState::Recorded));
        assert (first[13] == 7);
        assert (first[18] == 0 && first[19] == 0);
        assert (first[52] == 0x12 && first[53] == 0x34 && first[54] == 0x56 &&
                first[55] == 0x78);
        assert (first[56] == 0xc3 && first[57] == 0xd2 && first[58] == 0xe1 &&
                first[59] == 0xf0);
        assert (first[60] == 0x07);
        assert (first[61] == static_cast<uint8_t> (adk::StatusCode::Ok));
        assert ((static_cast<uint16_t> (first[62]) |
                 static_cast<uint16_t> (static_cast<uint16_t> (first[63]) << 8)) ==
                crc16 (first, 62));

        adk::InertialRecord decoded = emptyRecord ();

        assert (adk::InertialRecordCodec ().decode ({first, sizeof first}, decoded) ==
                adk::InertialRecordValidity::Valid);
        assert (equal (decoded, input));
    }

    void testEncodeCapacityAndSemanticFailuresAreAtomic ()
    {
        uint8_t storage[adk::InertialRecordCodec::size + 1];

        memset (storage, 0xa5, sizeof storage);

        const adk::Result<uint16_t> shortResult = adk::InertialRecordCodec ().encode (
            record (), {storage, adk::InertialRecordCodec::size - 1});
        assert (shortResult.error () == adk::StatusCode::CapacityExceeded);
        assert (shortResult.value () == 0);
        for (uint8_t index = 0; index < sizeof storage; ++index)
        {
            assert (storage[index] == 0xa5);
        }

        adk::InertialRecord invalid = record ();
        invalid.schemaRevision      = 0;
        const adk::Result<uint16_t> semanticResult =
            adk::InertialRecordCodec ().encode (invalid, {storage, sizeof storage});
        assert (semanticResult.error () == adk::StatusCode::InvalidArgument);
        for (uint8_t index = 0; index < sizeof storage; ++index)
        {
            assert (storage[index] == 0xa5);
        }

        const adk::Result<uint16_t> nullResult = adk::InertialRecordCodec ().encode (
            record (), {nullptr, adk::InertialRecordCodec::size});
        assert (nullResult.error () == adk::StatusCode::CapacityExceeded);
    }

    void testDecodeFramingAndIntegrityPrecedenceIsAtomic ()
    {
        uint8_t image[adk::InertialRecordCodec::size] = {};

        encode (record (), image);

        const adk::InertialRecord sentinel = record ();

        adk::InertialRecord output = sentinel;
        assert (adk::InertialRecordCodec ().decode ({nullptr, sizeof image}, output) ==
                adk::InertialRecordValidity::BadLength);
        assert (equal (output, sentinel));
        assert (
            adk::InertialRecordCodec ().decode ({image, sizeof image - 1}, output) ==
            adk::InertialRecordValidity::BadLength);
        assert (equal (output, sentinel));

        image[0] ^= 1;
        assert (adk::InertialRecordCodec ().decode ({image, sizeof image}, output) ==
                adk::InertialRecordValidity::BadFraming);
        assert (equal (output, sentinel));
        image[0] ^= 1;

        image[adk::InertialRecordCodec::versionOffset] ^= 1;
        assert (adk::InertialRecordCodec ().decode ({image, sizeof image}, output) ==
                adk::InertialRecordValidity::BadFraming);
        image[adk::InertialRecordCodec::versionOffset] ^= 1;

        image[adk::InertialRecordCodec::accelerationOffset] ^= 1;
        assert (adk::InertialRecordCodec ().decode ({image, sizeof image}, output) ==
                adk::InertialRecordValidity::BadIntegrity);
        assert (equal (output, sentinel));
    }

    void testDecodeRejectsEverySingleByteCorruptionAtomically ()
    {
        uint8_t canonical[adk::InertialRecordCodec::size] = {};

        encode (record (), canonical);

        const adk::InertialRecord sentinel = record ();

        for (uint8_t index = 0; index < sizeof canonical; ++index)
        {
            uint8_t image[sizeof canonical];
            memcpy (image, canonical, sizeof image);
            image[index] ^= 0x01;
            adk::InertialRecord output = sentinel;
            assert (
                adk::InertialRecordCodec ().decode ({image, sizeof image}, output) !=
                adk::InertialRecordValidity::Valid);
            assert (equal (output, sentinel));
        }
    }

    void testDecodeRejectsCrcRepairedSemanticCorruption ()
    {
        uint8_t canonical[adk::InertialRecordCodec::size] = {};

        encode (record (), canonical);

        const adk::InertialRecord sentinel = record ();
        const uint8_t offsets[] = {adk::InertialRecordCodec::reservedOffset,
                                   adk::InertialRecordCodec::stateOffset,
                                   adk::InertialRecordCodec::sourceKindOffset,
                                   adk::InertialRecordCodec::modelOffset,
                                   adk::InertialRecordCodec::flagsOffset,
                                   adk::InertialRecordCodec::producerStatusOffset};

        for (uint8_t index = 0; index < sizeof offsets; ++index)
        {
            uint8_t image[sizeof canonical];

            memcpy (image, canonical, sizeof image);
            image[offsets[index]] = 0xff;
            repairCrc (image);
            adk::InertialRecord output = sentinel;
            assert (
                adk::InertialRecordCodec ().decode ({image, sizeof image}, output) ==
                adk::InertialRecordValidity::BadSemanticValue);
            assert (equal (output, sentinel));
        }
    }

    void testSignedBoundariesAndInt32MinRejection ()
    {
        adk::InertialRecord input                     = record ();
        input.source.accelerationRangeMicroG          = INT32_MAX;
        input.source.angularRateRangeMilliDegreesPerSecond = INT32_MAX;
        input.accelerationMicroG                           = {-INT32_MAX, 0, INT32_MAX};
        input.angularRateMilliDegreesPerSecond             = {INT32_MAX, -INT32_MAX, -1};
        input.saturation = adk::InertialSaturation::Both;
        uint8_t image[adk::InertialRecordCodec::size] = {};

        encode (input, image);

        adk::InertialRecord output = emptyRecord ();

        assert (adk::InertialRecordCodec ().decode ({image, sizeof image}, output) ==
                adk::InertialRecordValidity::Valid);
        assert (equal (output, input));

        input.accelerationMicroG.x = INT32_MIN;
        memset (image, 0xa5, sizeof image);
        const adk::Result<uint16_t> result =
            adk::InertialRecordCodec ().encode (input, {image, sizeof image});
        assert (result.error () == adk::StatusCode::InvalidArgument);
        for (uint8_t index = 0; index < sizeof image; ++index)
        {
            assert (image[index] == 0xa5);
        }
    }
} // namespace

int main ()
{
    testNormalizerPreservesReadyEvidence                  ();
    testNormalizerClassifiesNotReadyAndSourceFault        ();
    testNormalizerRejectsInvalidConfigurationAtomically   ();
    testNormalizerRejectsEveryInvalidEnumAtomically       ();
    testSourcePairMatrixAndProducerStatuses               ();
    testStructuralBoundariesAreTransactional              ();
    testCanonicalLittleEndianImageAndRoundTrip            ();
    testEncodeCapacityAndSemanticFailuresAreAtomic        ();
    testDecodeFramingAndIntegrityPrecedenceIsAtomic       ();
    testDecodeRejectsEverySingleByteCorruptionAtomically  ();
    testDecodeRejectsCrcRepairedSemanticCorruption        ();
    testSignedBoundariesAndInt32MinRejection              ();
}
