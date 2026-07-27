#include "packet_receiver.h"

namespace adk {

    PacketReceiver::PacketReceiver () noexcept
        : packet_{}, packetSize_ (0), receivedAt_ (), captureSequence_ (0),
          status_      (StatusCode::NotInitialized), initialized_ (false),
          observation_ (false)
    {
    }

    PacketReceiver::~PacketReceiver () noexcept
    {
        shutdown ();
    }

    Status PacketReceiver::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        packetSize_      = 0;
        receivedAt_      = TimePoint ();
        captureSequence_ = 0;
        status_          = StatusCode::Ok;
        initialized_     = true;
        observation_     = false;
        return StatusCode::Ok;
    }

    void PacketReceiver::shutdown () noexcept
    {
        packetSize_      = 0;
        receivedAt_      = TimePoint ();
        captureSequence_ = 0;
        status_          = StatusCode::NotInitialized;
        initialized_     = false;
        observation_     = false;
    }

    bool PacketReceiver::initialized () const noexcept
    {
        return initialized_;
    }

    Status PacketReceiver::submit (ByteSpan packet, TimePoint receivedAt,
                                   uint32_t captureSequence) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (packet.size > capacity || (packet.size > 0 && packet.data == nullptr))
        {
            return StatusCode::InvalidArgument;
        }

        if (observation_)
        {
            status_ = StatusCode::CapacityExceeded;
            return status_;
        }

        for (uint16_t index = 0; index < packet.size; ++index)
        {
            packet_[index] = packet.data[index];
        }

        packetSize_      = packet.size;
        receivedAt_      = receivedAt;
        captureSequence_ = captureSequence;
        status_          = StatusCode::Ok;
        observation_     = true;
        return StatusCode::Ok;
    }

    Status PacketReceiver::observeFailure (Status status) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (status.ok ())
        {
            return StatusCode::InvalidArgument;
        }

        status_ = status;
        return StatusCode::Ok;
    }

    Status PacketReceiver::update (TimePoint now) noexcept
    {
        static_cast<void> (now);

        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        return status_;
    }

    bool PacketReceiver::observation () const noexcept
    {
        return initialized_ && observation_;
    }

    PacketObservation PacketReceiver::latest () const noexcept
    {
        if (!observation ())
        {
            return {{nullptr, 0}, TimePoint (), 0};
        }

        return {{packet_, packetSize_}, receivedAt_, captureSequence_};
    }

    Status PacketReceiver::acknowledge (uint32_t captureSequence) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!observation_ || captureSequence != captureSequence_)
        {
            return StatusCode::InvalidArgument;
        }

        packetSize_  = 0;
        status_      = StatusCode::Ok;
        observation_ = false;
        return StatusCode::Ok;
    }
} // namespace adk
