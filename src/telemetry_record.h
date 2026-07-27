#pragma once

#include "bounded_span.h"
#include "record_sink.h"
#include "telemetry_console.h"

namespace adk {

    struct TelemetryRecordEncoder
    {
        static constexpr uint8_t maximumLength = 49;

        Result<uint16_t> encode (TimePoint            recordedAt,
                                 const ConsoleSource& source,
                                 ConsoleHealth        health,
                                 ConsoleRecordReason  reason,
                                 MutableTextSpan      output) const noexcept;
        Status encode (TimePoint            recordedAt,
                       const ConsoleSource& source,
                       ConsoleHealth        health,
                       ConsoleRecordReason  reason,
                       StableRecord&        output) const noexcept;
    };
} // namespace adk
