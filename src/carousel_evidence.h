#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {
    // clang-format off

    enum struct CarouselSourceKind : uint8_t
    {
        SyntheticIdentity,
        SyntheticKey,
        SyntheticHome,
        SyntheticStop
    };

    struct CarouselSource
    {
        CarouselSourceKind kind;
        uint8_t            sourceId;
        uint16_t           configurationRevision;
    };

    struct CopiedKeyEvidence
    {
        CarouselSource source;
        TimePoint      observedAt;
        uint32_t       sequence;
        uint8_t        key;
        bool           pressed;
        Status         status;
    };

    struct CopiedBinaryEvidence
    {
        CarouselSource source;
        TimePoint      observedAt;
        uint32_t       sequence;
        bool           active;
        bool           qualified;
        uint32_t       qualificationEpoch;
        Status         status;
    };

    static constexpr uint8_t maximumLocalIdentityBytes = 10;

    struct LocalIdentity
    {
        uint8_t length;
        uint8_t bytes[maximumLocalIdentityBytes];
    };

    struct IdentityEvidence
    {
        CarouselSource source;
        TimePoint      observedAt;
        uint32_t       sequence;
        LocalIdentity  identity;
        Status         status;
    };

    struct CopiedKeyBatch
    {
        CarouselSource source;
        TimePoint      observedAt;
        uint32_t       sequence;
        uint8_t        digitCount;
        uint8_t        digits[4];
        bool           confirm;
        bool           cancel;
        Status         status;
    };

    struct CopiedPresentationStatus
    {
        TimePoint observedAt;
        uint32_t  sequence;
        Status    status;
    };

    // clang-format on
} // namespace adk
