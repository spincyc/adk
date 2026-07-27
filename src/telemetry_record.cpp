#include "telemetry_record.h"

#include <limits.h>

namespace adk {

    namespace {

        static_assert (StableRecord::capacity >=
                           TelemetryRecordEncoder::maximumLength,
                       "StableRecord cannot hold a telemetry record");

        bool appendCharacter (StableRecord& record, char value) noexcept
        {
            if (record.length >= StableRecord::capacity)
            {
                return false;
            }

            record.text[record.length++] = value;
            return true;
        }

        bool appendText (StableRecord& record, const char* text) noexcept
        {
            while (*text != '\0')
            {
                if (!appendCharacter (record, *text++))
                {
                    return false;
                }
            }

            return true;
        }

        bool appendUnsigned (StableRecord& record, uint32_t value) noexcept
        {
            char    digits[10];
            uint8_t count = 0;

            do
            {
                digits[count++] = static_cast<char> ('0' + value % 10U);
                value /= 10U;
            } while (value != 0);

            while (count != 0)
            {
                if (!appendCharacter (record, digits[--count]))
                {
                    return false;
                }
            }

            return true;
        }

        bool appendSigned (StableRecord& record, int32_t value) noexcept
        {
            if (value < 0)
            {
                if (!appendCharacter (record, '-'))
                {
                    return false;
                }

                const uint32_t magnitude =
                    static_cast<uint32_t> (-(value + 1)) + 1U;
                return appendUnsigned (record, magnitude);
            }

            return appendUnsigned (record, static_cast<uint32_t> (value));
        }

        char kindCode (TelemetryKind kind) noexcept
        {
            switch (kind)
            {
                case TelemetryKind::Temperature:      return 'T';
                case TelemetryKind::RelativeHumidity: return 'H';
                case TelemetryKind::Distance:         return 'D';
                case TelemetryKind::Contact:          return 'C';
                case TelemetryKind::Counter:          return 'N';
            }

            return '\0';
        }

        char qualityCode (SampleQuality quality) noexcept
        {
            switch (quality)
            {
                case SampleQuality::Valid:         return 'V';
                case SampleQuality::SensorFault:   return 'F';
                case SampleQuality::OutOfRange:    return 'O';
                case SampleQuality::StaleAtSource: return 'S';
            }

            return '\0';
        }

        char sequenceCode (SequenceState state) noexcept
        {
            switch (state)
            {
                case SequenceState::First:     return 'F';
                case SequenceState::InOrder:   return 'I';
                case SequenceState::Duplicate: return 'D';
                case SequenceState::Gap:       return 'G';
                case SequenceState::Reordered: return 'R';
            }

            return '\0';
        }

        char freshnessCode (Freshness freshness) noexcept
        {
            switch (freshness)
            {
                case Freshness::Fresh: return 'F';
                case Freshness::Aging: return 'A';
                case Freshness::Stale: return 'S';
            }

            return '\0';
        }

        char healthCode (ConsoleHealth health) noexcept
        {
            switch (health)
            {
                case ConsoleHealth::Starting: return 'S';
                case ConsoleHealth::Healthy:  return 'H';
                case ConsoleHealth::Degraded: return 'D';
                case ConsoleHealth::Fault:    return 'F';
                case ConsoleHealth::Stopped:  return 'X';
            }

            return '\0';
        }

        char reasonCode (ConsoleRecordReason reason) noexcept
        {
            switch (reason)
            {
                case ConsoleRecordReason::None:             return 'N';
                case ConsoleRecordReason::Heartbeat:        return 'B';
                case ConsoleRecordReason::Observation:      return 'O';
                case ConsoleRecordReason::Acknowledgement:  return 'A';
                case ConsoleRecordReason::HealthTransition: return 'H';
            }

            return '\0';
        }
    } // namespace

    Result<uint16_t> TelemetryRecordEncoder::encode (
        TimePoint            recordedAt,
        const ConsoleSource& source,
        ConsoleHealth        health,
        ConsoleRecordReason  reason,
        MutableTextSpan      output) const noexcept
    {
        const char kind       = kindCode      (source.kind);
        const char quality    = qualityCode   (source.quality);
        const char sequence   = sequenceCode  (source.sequenceState);
        const char freshness  = freshnessCode (source.freshness);
        const char healthText = healthCode    (health);
        const char reasonText = reasonCode    (reason);

        if (kind == '\0' || quality == '\0' || sequence == '\0' ||
            freshness == '\0' || healthText == '\0' || reasonText == '\0' ||
            source.decimalExponent < -9 || source.decimalExponent > 9)
        {
            return {StatusCode::InvalidArgument, 0};
        }

        StableRecord record = {};

        const bool encoded =
            appendText      (record, "TEL1,") &&
            appendUnsigned  (record, recordedAt.milliseconds ()) &&
            appendCharacter (record, ',') &&
            appendUnsigned  (record, source.sourceId) &&
            appendCharacter (record, ',') &&
            appendCharacter (record, kind) &&
            appendCharacter (record, ',') &&
            appendCharacter (record, quality) &&
            appendCharacter (record, ',') &&
            appendCharacter (record, sequence) &&
            appendCharacter (record, ',') &&
            appendCharacter (record, freshness) &&
            appendCharacter (record, ',') &&
            appendSigned    (record, source.value) &&
            appendCharacter (record, ',') &&
            appendSigned    (record, source.decimalExponent) &&
            appendCharacter (record, ',') &&
            appendCharacter (record, healthText) &&
            appendCharacter (record, ',') &&
            appendCharacter (record, reasonText) &&
            appendCharacter (record, '\n');

        if (!encoded || record.length > maximumLength)
        {
            return {StatusCode::CapacityExceeded, 0};
        }

        if (output.capacity < record.length ||
            (record.length != 0 && output.data == nullptr))
        {
            return {StatusCode::CapacityExceeded, 0};
        }

        for (uint8_t index = 0; index < record.length; ++index)
        {
            output.data[index] = record.text[index];
        }

        return {StatusCode::Ok, record.length};
    }

    Status TelemetryRecordEncoder::encode (
        TimePoint recordedAt, const ConsoleSource& source, ConsoleHealth health,
        ConsoleRecordReason reason, StableRecord& output) const noexcept
    {
        StableRecord candidate = {};
        const Result<uint16_t> result =
            encode (recordedAt,
                    source,
                    health,
                    reason,
                    {candidate.text, StableRecord::capacity});

        if (!result.ok ())
        {
            return result.status ();
        }

        candidate.length = static_cast<uint8_t> (result.value ());
        output           = candidate;
        return StatusCode::Ok;
    }
} // namespace adk
