#include "telemetry_record.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {

    using namespace adk;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    ConsoleSource sourceFor () noexcept
    {
        return {65535,
                TelemetryKind::Temperature,
                SampleQuality::Valid,
                SequenceState::InOrder,
                Freshness::Fresh,
                -2147483647 - 1,
                -9,
                PacketValidity::Valid,
                StatusCode::Ok,
                true};
    }

    void testGoldenAndCapacity ()
    {
        TelemetryRecordEncoder encoder;
        StableRecord           record = {};
        const ConsoleSource     source = sourceFor ();

        require (encoder.encode (TimePoint (4294967295U),
                                 source,
                                 ConsoleHealth::Healthy,
                                 ConsoleRecordReason::HealthTransition,
                                 record)
                     .ok (),
                 "maximum record encodes");

        const char expected[] =
            "TEL1,4294967295,65535,T,V,I,F,-2147483648,-9,H,H\n";

        require (record.length == sizeof (expected) - 1U, "golden length");
        require (record.length == TelemetryRecordEncoder::maximumLength,
                 "maximum length proof is exact");
        require (std::memcmp (record.text, expected, record.length) == 0,
                 "golden bytes");
        require (TelemetryRecordEncoder::maximumLength <= StableRecord::capacity,
                 "record capacity proof");
    }

    void testEveryCode ()
    {
        TelemetryRecordEncoder encoder;
        StableRecord           record = {};
        ConsoleSource          source = sourceFor ();

        for (uint8_t kind = static_cast<uint8_t> (TelemetryKind::Temperature);
             kind <= static_cast<uint8_t> (TelemetryKind::Counter);
             ++kind)
        {
            source.kind = static_cast<TelemetryKind> (kind);
            require (encoder.encode (TimePoint (0),
                                     source,
                                     ConsoleHealth::Healthy,
                                     ConsoleRecordReason::Observation,
                                     record)
                         .ok (),
                     "every kind encodes");
        }

        for (uint8_t quality = static_cast<uint8_t> (SampleQuality::Valid);
             quality <= static_cast<uint8_t> (SampleQuality::StaleAtSource);
             ++quality)
        {
            source.quality = static_cast<SampleQuality> (quality);
            require (encoder.encode (TimePoint (0),
                                     source,
                                     ConsoleHealth::Healthy,
                                     ConsoleRecordReason::Observation,
                                     record)
                         .ok (),
                     "every quality encodes");
        }

        source.quality = SampleQuality::Valid;

        for (uint8_t sequence = static_cast<uint8_t> (SequenceState::First);
             sequence <= static_cast<uint8_t> (SequenceState::Reordered);
             ++sequence)
        {
            source.sequenceState = static_cast<SequenceState> (sequence);
            require (encoder.encode (TimePoint (0),
                                     source,
                                     ConsoleHealth::Healthy,
                                     ConsoleRecordReason::Observation,
                                     record)
                         .ok (),
                     "every sequence state encodes");
        }

        source.sequenceState = SequenceState::InOrder;

        for (uint8_t freshness = static_cast<uint8_t> (Freshness::Fresh);
             freshness <= static_cast<uint8_t> (Freshness::Stale);
             ++freshness)
        {
            source.freshness = static_cast<Freshness> (freshness);
            require (encoder.encode (TimePoint (0),
                                     source,
                                     ConsoleHealth::Healthy,
                                     ConsoleRecordReason::Observation,
                                     record)
                         .ok (),
                     "every freshness encodes");
        }

        source.freshness = Freshness::Fresh;

        for (uint8_t health = static_cast<uint8_t> (ConsoleHealth::Starting);
             health <= static_cast<uint8_t> (ConsoleHealth::Stopped);
             ++health)
        {
            require (encoder.encode (TimePoint (0),
                                     source,
                                     static_cast<ConsoleHealth> (health),
                                     ConsoleRecordReason::Observation,
                                     record)
                         .ok (),
                     "every health encodes");
        }

        for (uint8_t reason =
                 static_cast<uint8_t> (ConsoleRecordReason::None);
             reason <=
             static_cast<uint8_t> (ConsoleRecordReason::HealthTransition);
             ++reason)
        {
            require (encoder.encode (
                         TimePoint (0),
                         source,
                         ConsoleHealth::Healthy,
                         static_cast<ConsoleRecordReason> (reason),
                         record)
                         .ok (),
                     "every record reason encodes");
        }
    }

    void testInvalidLeavesOutputUnchanged ()
    {
        TelemetryRecordEncoder encoder;
        StableRecord           record = {};
        ConsoleSource          source = sourceFor ();

        record.text[0] = 'Q';
        record.length  = 1;
        source.kind    = static_cast<TelemetryKind> (255);

        require (encoder.encode (TimePoint (0),
                                 source,
                                 ConsoleHealth::Healthy,
                                 ConsoleRecordReason::Observation,
                                 record)
                    .error () == StatusCode::InvalidArgument,
                 "invalid enum rejected");
        require (record.length == 1 && record.text[0] == 'Q',
                 "failure leaves output unchanged");

        source                 = sourceFor ();
        source.decimalExponent = 10;
        require (encoder.encode (TimePoint (0),
                                 source,
                                 ConsoleHealth::Healthy,
                                 ConsoleRecordReason::Observation,
                                 record)
                    .error () == StatusCode::InvalidArgument,
                 "invalid exponent rejected");
        require (record.length == 1 && record.text[0] == 'Q',
                 "scale failure leaves output unchanged");
    }

    void testCallerCapacityBoundaries ()
    {
        TelemetryRecordEncoder encoder;
        const ConsoleSource     source = sourceFor ();
        char exact[TelemetryRecordEncoder::maximumLength];
        char shortBuffer[TelemetryRecordEncoder::maximumLength - 1U];

        for (uint8_t index = 0;
             index < TelemetryRecordEncoder::maximumLength - 1U;
             ++index)
        {
            shortBuffer[index] = 'Q';
        }

        const Result<uint16_t> exactResult =
            encoder.encode (TimePoint (4294967295U),
                            source,
                            ConsoleHealth::Healthy,
                            ConsoleRecordReason::HealthTransition,
                            {exact, sizeof (exact)});

        require (exactResult.ok (), "exact capacity succeeds");
        require (exactResult.value () ==
                     TelemetryRecordEncoder::maximumLength,
                 "exact capacity reports bytes");

        const Result<uint16_t> shortResult =
            encoder.encode (TimePoint (4294967295U),
                            source,
                            ConsoleHealth::Healthy,
                            ConsoleRecordReason::HealthTransition,
                            {shortBuffer, sizeof (shortBuffer)});

        require (shortResult.error () == StatusCode::CapacityExceeded,
                 "one-byte-short capacity rejected");
        require (shortResult.value () == 0, "short capacity writes zero bytes");

        for (uint8_t index = 0;
             index < TelemetryRecordEncoder::maximumLength - 1U;
             ++index)
        {
            require (shortBuffer[index] == 'Q',
                     "short output remains unchanged");
        }

        require (encoder.encode (TimePoint (0),
                                 source,
                                 ConsoleHealth::Healthy,
                                 ConsoleRecordReason::Observation,
                                 {nullptr, 0})
                    .error () == StatusCode::CapacityExceeded,
                 "empty output rejected");
    }
} // namespace

int main ()
{
    testGoldenAndCapacity             ();
    testEveryCode                     ();
    testInvalidLeavesOutputUnchanged  ();
    testCallerCapacityBoundaries      ();

    std::cout << "telemetry record tests passed\n";
    return 0;
}
