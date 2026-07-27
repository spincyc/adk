#pragma once

#include "pulse_capture.h"
#include "status.h"

namespace adk {

    enum struct InfraredProtocol : uint8_t
    {
        Unknown,
        Nec
    };

    enum struct FrameValidity : uint8_t
    {
        Valid,
        Repeat,
        UnknownProtocol,
        TimingInvalid,
        IntegrityInvalid,
        Truncated,
        Overflow
    };

    struct InfraredFrame
    {
        InfraredProtocol protocol;
        FrameValidity    validity;
        uint32_t         address;
        uint32_t         command;
        uint32_t         captureSequence;
    };

    struct InfraredDecoder
    {
        Status decode (const PulseFrame& capture, InfraredFrame& output) const noexcept;
    };
} // namespace adk
