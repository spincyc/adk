#include "inertial_record.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint8_t magic[4] = {'I', 'R', '6', '7'};

        bool validSourceKind (InertialSourceKind kind) noexcept
        {
            switch (kind)
            {
                case InertialSourceKind::SyntheticFixture:
                case InertialSourceKind::Mpu6050Adapter:
                case InertialSourceKind::Qmi8658Adapter: return true;
            }

            return false;
        }

        bool validModel (InertialModel model) noexcept
        {
            switch (model)
            {
                case InertialModel::Synthetic:
                case InertialModel::Mpu6050:
                case InertialModel::Qmi8658UnknownRevision: return true;
            }

            return false;
        }

        bool validSaturation (InertialSaturation saturation) noexcept
        {
            switch (saturation)
            {
                case InertialSaturation::None:
                case InertialSaturation::Acceleration:
                case InertialSaturation::AngularRate:
                case InertialSaturation::Both: return true;
            }

            return false;
        }

        bool validState (InertialRecordState state) noexcept
        {
            switch (state)
            {
                case InertialRecordState::Recorded:
                case InertialRecordState::NotReady:
                case InertialRecordState::SourceFault: return true;
            }

            return false;
        }

        bool validSourcePair (const InertialSource& source) noexcept
        {
            return (source.kind == InertialSourceKind::SyntheticFixture &&
                    source.model == InertialModel::Synthetic) ||
                   (source.kind == InertialSourceKind::Mpu6050Adapter &&
                    source.model == InertialModel::Mpu6050) ||
                   (source.kind == InertialSourceKind::Qmi8658Adapter &&
                    source.model == InertialModel::Qmi8658UnknownRevision);
        }

        uint32_t magnitude (int32_t value) noexcept
        {
            const int64_t widened = value;
            return static_cast<uint32_t> (widened < 0 ? -widened : widened);
        }

        bool anyBeyond (const InertialVector& vector, uint32_t range) noexcept
        {
            return magnitude (vector.x) > range ||
                   magnitude (vector.y) > range ||
                   magnitude (vector.z) > range;
        }

        bool anyAt (const InertialVector& vector, uint32_t range) noexcept
        {
            return magnitude (vector.x) == range ||
                   magnitude (vector.y) == range ||
                   magnitude (vector.z) == range;
        }

        InertialSaturation measuredSaturation (
            const InertialRecord& record) noexcept
        {
            const bool acceleration =
                anyAt (record.accelerationMicroG,
                       record.source.accelerationRangeMicroG);
            const bool angularRate =
                anyAt (record.angularRateMilliDegreesPerSecond,
                       record.source.angularRateRangeMilliDegreesPerSecond);
            return static_cast<InertialSaturation> (
                (acceleration ? 1U : 0U) | (angularRate ? 2U : 0U));
        }

        bool validStatus (Status status) noexcept
        {
            switch (status.error ())
            {
                case StatusCode::Ok:
                case StatusCode::InvalidArgument:
                case StatusCode::InvalidConfiguration:
                case StatusCode::InvalidPin:
                case StatusCode::Unsupported:
                case StatusCode::ResourceBusy:
                case StatusCode::NotInitialized:
                case StatusCode::CapacityExceeded:
                case StatusCode::Timeout:
                case StatusCode::InternalInvariant:
                case StatusCode::HardwareFailure: return true;
            }

            return false;
        }

        InertialRecordState recordState (const InertialSample& sample) noexcept
        {
            if (!sample.status.ok ())
            {
                return InertialRecordState::SourceFault;
            }

            return sample.dataReady ? InertialRecordState::Recorded
                                    : InertialRecordState::NotReady;
        }

        bool validSemanticRecord (const InertialRecord& record) noexcept
        {
            if (record.schemaRevision == 0 ||
                record.normalizationRevision == 0 ||
                !validSourceKind  (record.source.kind) ||
                !validModel       (record.source.model) ||
                !validSaturation  (record.saturation) ||
                !validStatus      (record.producerStatus) ||
                !validState       (record.state) ||
                record.source.sourceId == 0 ||
                record.source.configurationRevision == 0 ||
                record.source.calibrationRevision == 0 ||
                record.source.accelerationRangeMicroG == 0 ||
                record.source.angularRateRangeMilliDegreesPerSecond == 0 ||
                record.source.accelerationRangeMicroG >
                    static_cast<uint32_t> (INT32_MAX) ||
                record.source.angularRateRangeMilliDegreesPerSecond >
                    static_cast<uint32_t> (INT32_MAX) ||
                !validSourcePair  (record.source) ||
                anyBeyond         (record.accelerationMicroG,
                                   record.source.accelerationRangeMicroG) ||
                anyBeyond (
                    record.angularRateMilliDegreesPerSecond,
                    record.source.angularRateRangeMilliDegreesPerSecond) ||
                measuredSaturation (record) != record.saturation)
            {
                return false;
            }

            if (record.state == InertialRecordState::SourceFault)
            {
                return !record.producerStatus.ok () && !record.dataReady &&
                       record.saturation == InertialSaturation::None &&
                       record.accelerationMicroG.x == 0 &&
                       record.accelerationMicroG.y == 0 &&
                       record.accelerationMicroG.z == 0 &&
                       record.angularRateMilliDegreesPerSecond.x == 0 &&
                       record.angularRateMilliDegreesPerSecond.y == 0 &&
                       record.angularRateMilliDegreesPerSecond.z == 0;
            }

            if (record.state == InertialRecordState::NotReady)
            {
                return record.producerStatus.ok () && !record.dataReady &&
                       record.saturation == InertialSaturation::None &&
                       record.accelerationMicroG.x == 0 &&
                       record.accelerationMicroG.y == 0 &&
                       record.accelerationMicroG.z == 0 &&
                       record.angularRateMilliDegreesPerSecond.x == 0 &&
                       record.angularRateMilliDegreesPerSecond.y == 0 &&
                       record.angularRateMilliDegreesPerSecond.z == 0;
            }

            return record.producerStatus.ok () && record.dataReady;
        }

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

        int32_t readSigned32 (const uint8_t* input) noexcept
        {
            const uint32_t value = read32 (input);
            return value <= INT32_MAX ? static_cast<int32_t> (value)
                                      : -1 - static_cast<int32_t> (~value);
        }

        uint16_t crc16 (const uint8_t* bytes, uint8_t length) noexcept
        {
            uint16_t crc = InertialRecordCodec::crcInitialValue;

            for (uint8_t index = 0; index < length; ++index)
            {
                crc ^= static_cast<uint16_t> (
                    static_cast<uint16_t> (bytes[index]) << 8);

                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & 0x8000U) != 0U
                              ? static_cast<uint16_t> (
                                    (crc << 1) ^
                                    InertialRecordCodec::crcPolynomial)
                              : static_cast<uint16_t> (crc << 1);
                }
            }

            return static_cast<uint16_t> (
                crc ^ InertialRecordCodec::crcFinalXor);
        }
    } // namespace

    InertialRecordNormalizer::InertialRecordNormalizer (
        const InertialRecordConfig& config) noexcept
        : config_ (config)
    {
    }

    Status InertialRecordNormalizer::normalize (
        const InertialSample& sample, InertialRecord& output) const noexcept
    {
        if (config_.schemaRevision == 0 ||
            config_.normalizationRevision == 0)
        {
            return StatusCode::InvalidConfiguration;
        }

        const InertialRecordState state = recordState (sample);
        const bool hasSample = state == InertialRecordState::Recorded;
        const InertialVector emptyVector = {0, 0, 0};
        const InertialRecord candidate = {
            config_.schemaRevision,
            config_.normalizationRevision,
            sample.source,
            hasSample ? sample.accelerationMicroG : emptyVector,
            hasSample ? sample.angularRateMilliDegreesPerSecond : emptyVector,
            sample.observedAt,
            sample.sequence,
            hasSample,
            hasSample ? sample.saturation : InertialSaturation::None,
            sample.status,
            state};

        if (!validSemanticRecord (candidate))
        {
            return StatusCode::InvalidArgument;
        }

        output = candidate;
        return StatusCode::Ok;
    }

    Result<uint16_t> InertialRecordCodec::encode (
        const InertialRecord& record, MutableByteSpan output) const noexcept
    {
        if (!validSemanticRecord (record))
        {
            return {StatusCode::InvalidArgument, 0};
        }

        if (output.data == nullptr || output.capacity < size)
        {
            return {StatusCode::CapacityExceeded, 0};
        }

        uint8_t image[size] = {};
        for (uint8_t index = 0; index < sizeof magic; ++index)
        {
            image[magicOffset + index] = magic[index];
        }

        image[versionOffset] = version;
        image[lengthOffset]  = size;
        write16 (&image[schemaRevisionOffset], record.schemaRevision);
        write16 (&image[normalizationRevisionOffset],
                 record.normalizationRevision);
        image[stateOffset]      = static_cast<uint8_t> (record.state);
        image[sourceKindOffset] = static_cast<uint8_t> (record.source.kind);
        image[modelOffset]      = static_cast<uint8_t> (record.source.model);
        image[sourceIdOffset]   = record.source.sourceId;
        write16 (&image[configurationRevisionOffset],
                 record.source.configurationRevision);
        write16 (&image[calibrationRevisionOffset],
                 record.source.calibrationRevision);
        write32 (&image[accelerationRangeOffset],
                 record.source.accelerationRangeMicroG);
        write32 (&image[angularRateRangeOffset],
                 record.source.angularRateRangeMilliDegreesPerSecond);
        write32 (&image[accelerationOffset], static_cast<uint32_t> (
                                                  record.accelerationMicroG.x));
        write32 (&image[accelerationOffset + 4], static_cast<uint32_t> (
                                                      record.accelerationMicroG.y));
        write32 (&image[accelerationOffset + 8], static_cast<uint32_t> (
                                                      record.accelerationMicroG.z));
        write32 (&image[angularRateOffset], static_cast<uint32_t> (
                                               record.angularRateMilliDegreesPerSecond.x));
        write32 (&image[angularRateOffset + 4], static_cast<uint32_t> (
                                                   record.angularRateMilliDegreesPerSecond.y));
        write32 (&image[angularRateOffset + 8], static_cast<uint32_t> (
                                                   record.angularRateMilliDegreesPerSecond.z));
        write32 (&image[observedAtOffset], record.observedAt.milliseconds ());
        write32 (&image[sequenceOffset], record.sequence);
        image[flagsOffset] =
            static_cast<uint8_t> (
                (record.dataReady ? dataReadyFlag : 0U) |
                (static_cast<uint8_t> (record.saturation) <<
                 saturationShift));
        image[producerStatusOffset] =
            static_cast<uint8_t> (record.producerStatus.error ());
        write16 (&image[integrityOffset], crc16 (image, integrityOffset));

        for (uint8_t index = 0; index < size; ++index)
        {
            output.data[index] = image[index];
        }

        return {StatusCode::Ok, size};
    }

    InertialRecordValidity InertialRecordCodec::decode (
        ByteSpan image, InertialRecord& output) const noexcept
    {
        if (image.data == nullptr || image.size != size)
        {
            return InertialRecordValidity::BadLength;
        }

        for (uint8_t index = 0; index < sizeof magic; ++index)
        {
            if (image.data[magicOffset + index] != magic[index])
            {
                return InertialRecordValidity::BadFraming;
            }
        }

        if (image.data[versionOffset] != version ||
            image.data[lengthOffset] != size)
        {
            return InertialRecordValidity::BadFraming;
        }

        if (read16 (&image.data[integrityOffset]) !=
            crc16 (image.data, integrityOffset))
        {
            return InertialRecordValidity::BadIntegrity;
        }

        if (image.data[reservedOffset] != 0 ||
            image.data[reservedOffset + 1] != 0 ||
            (image.data[flagsOffset] & ~knownFlagsMask) != 0)
        {
            return InertialRecordValidity::BadSemanticValue;
        }

        const InertialSaturation saturation =
            static_cast<InertialSaturation> (
                (image.data[flagsOffset] & saturationMask) >>
                saturationShift);
        const InertialRecord candidate = {
            read16 (&image.data[schemaRevisionOffset]),
            read16 (&image.data[normalizationRevisionOffset]),
            {static_cast<InertialSourceKind> (
                 image.data[sourceKindOffset]),
             static_cast<InertialModel> (image.data[modelOffset]),
             image.data[sourceIdOffset],
             read16 (&image.data[configurationRevisionOffset]),
             read16 (&image.data[calibrationRevisionOffset]),
             read32 (&image.data[accelerationRangeOffset]),
             read32 (&image.data[angularRateRangeOffset])},
            {readSigned32 (&image.data[accelerationOffset]),
             readSigned32 (&image.data[accelerationOffset + 4]),
             readSigned32 (&image.data[accelerationOffset + 8])},
            {readSigned32 (&image.data[angularRateOffset]),
             readSigned32 (&image.data[angularRateOffset + 4]),
             readSigned32 (&image.data[angularRateOffset + 8])},
            TimePoint  (read32 (&image.data[observedAtOffset])),
            read32     (&image.data[sequenceOffset]),
            (image.data[flagsOffset] & dataReadyFlag) != 0,
            saturation,
            Status (static_cast<StatusCode> (
                image.data[producerStatusOffset])),
            static_cast<InertialRecordState> (image.data[stateOffset])};

        if (!validSemanticRecord (candidate))
        {
            return InertialRecordValidity::BadSemanticValue;
        }

        output = candidate;
        return InertialRecordValidity::Valid;
    }
} // namespace adk
