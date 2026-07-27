#pragma once

#include <stdint.h>

#include "bounded_span.h"
#include "status.h"
#include "time.h"

namespace adk {

    struct PacketObservation
    {
        ByteSpan  packet;
        TimePoint receivedAt;
        uint32_t  captureSequence;
    };

    struct PacketReceiver
    {
        static constexpr uint8_t capacity = 19;

        PacketReceiver  () noexcept;
        ~PacketReceiver () noexcept;

        PacketReceiver (const PacketReceiver&)            = delete;
        PacketReceiver& operator= (const PacketReceiver&) = delete;

        Status            initialize  () noexcept;
        void              shutdown    () noexcept;
        bool              initialized () const noexcept;
        Status            submit      (ByteSpan packet, TimePoint receivedAt,
                                  uint32_t captureSequence) noexcept;
        Status            observeFailure (Status status) noexcept;
        Status            update         (TimePoint now) noexcept;
        bool              observation    () const noexcept;
        PacketObservation latest         () const noexcept;
        Status            acknowledge    (uint32_t captureSequence) noexcept;

      private:
        uint8_t   packet_[capacity];
        uint16_t  packetSize_;
        TimePoint receivedAt_;
        uint32_t  captureSequence_;
        Status    status_;
        bool      initialized_;
        bool      observation_;
    };
} // namespace adk
