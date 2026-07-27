#pragma once

#include "bounded_span.h"
#include "infrared_decoder.h"
#include "status.h"

namespace adk {

    struct InfraredRecordEncoder
    {
        Result<uint16_t> encode (const InfraredFrame& frame,
                                 MutableTextSpan      output) const noexcept;
    };
} // namespace adk
