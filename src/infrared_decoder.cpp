#include "infrared_decoder.h"

namespace adk {

    namespace {

        constexpr uint8_t necPulseCount       = 67;
        constexpr uint8_t necRepeatPulseCount = 3;

        bool pulseMatches (const Pulse& pulse, PulseLevel level, uint32_t minimum,
                           uint32_t maximum) noexcept
        {
            const uint32_t duration = pulse.duration.microseconds ();

            return pulse.level == level && duration >= minimum && duration <= maximum;
        }

        bool leaderMatches (const PulseFrame& capture) noexcept
        {
            return pulseMatches (capture.data[0], PulseLevel::Mark, 8000, 10000) &&
                   pulseMatches (capture.data[1], PulseLevel::Space, 4000, 5000);
        }

        bool repeatMatches (const PulseFrame& capture) noexcept
        {
            return pulseMatches (capture.data[0], PulseLevel::Mark, 8000, 10000) &&
                   pulseMatches (capture.data[1], PulseLevel::Space, 2000, 2500) &&
                   pulseMatches (capture.data[2], PulseLevel::Mark, 400, 700);
        }

        bool decodeBits (const PulseFrame& capture, uint32_t& bits) noexcept
        {
            uint32_t decoded = 0;

            for (uint8_t bit = 0; bit < 32; ++bit)
            {
                const Pulse& mark  = capture.data[2 + bit * 2];
                const Pulse& space = capture.data[3 + bit * 2];

                if (!pulseMatches (mark, PulseLevel::Mark, 400, 700))
                {
                    return false;
                }

                if (pulseMatches (space, PulseLevel::Space, 400, 700))
                {
                    continue;
                }

                if (!pulseMatches (space, PulseLevel::Space, 1400, 1900))
                {
                    return false;
                }

                decoded |= static_cast<uint32_t> (1) << bit;
            }

            if (!pulseMatches (capture.data[66], PulseLevel::Mark, 400, 700))
            {
                return false;
            }

            bits = decoded;
            return true;
        }
    } // namespace

    Status InfraredDecoder::decode (const PulseFrame& capture,
                                    InfraredFrame&    output) const noexcept
    {
        if (capture.size > 0 && capture.data == nullptr)
        {
            return StatusCode::InvalidArgument;
        }

        switch (capture.state)
        {
            case CaptureState::Idle:
            case CaptureState::Capturing:
            case CaptureState::Complete:
            case CaptureState::Overflow:
            case CaptureState::TimingFault: break;
            default: return StatusCode::InvalidArgument;
        }

        InfraredFrame decoded = {InfraredProtocol::Unknown, FrameValidity::Truncated, 0,
                                 0, capture.sequence};

        if (capture.state == CaptureState::Overflow)
        {
            decoded.validity = FrameValidity::Overflow;
            output           = decoded;
            return StatusCode::Ok;
        }

        if (capture.state == CaptureState::TimingFault)
        {
            decoded.validity = FrameValidity::TimingInvalid;
            output           = decoded;
            return StatusCode::Ok;
        }

        if (capture.state != CaptureState::Complete)
        {
            output = decoded;
            return StatusCode::Ok;
        }

        if (capture.size == necRepeatPulseCount && repeatMatches (capture))
        {
            decoded.protocol = InfraredProtocol::Nec;
            decoded.validity = FrameValidity::Repeat;
            output           = decoded;
            return StatusCode::Ok;
        }

        if (capture.size < 2)
        {
            output = decoded;
            return StatusCode::Ok;
        }

        if (!leaderMatches (capture))
        {
            decoded.validity = FrameValidity::UnknownProtocol;
            output           = decoded;
            return StatusCode::Ok;
        }

        decoded.protocol = InfraredProtocol::Nec;

        if (capture.size < necPulseCount)
        {
            decoded.validity = FrameValidity::Truncated;
            output           = decoded;
            return StatusCode::Ok;
        }

        if (capture.size > necPulseCount)
        {
            decoded.validity = FrameValidity::Overflow;
            output           = decoded;
            return StatusCode::Ok;
        }

        uint32_t bits = 0;

        if (!decodeBits (capture, bits))
        {
            decoded.validity = FrameValidity::TimingInvalid;
            output           = decoded;
            return StatusCode::Ok;
        }

        const uint8_t address        = static_cast<uint8_t> (bits);
        const uint8_t inverseAddress = static_cast<uint8_t> (bits >> 8);
        const uint8_t command        = static_cast<uint8_t> (bits >> 16);
        const uint8_t inverseCommand = static_cast<uint8_t> (bits >> 24);

        if (static_cast<uint8_t> (address ^ inverseAddress) != 0xff ||
            static_cast<uint8_t> (command ^ inverseCommand) != 0xff)
        {
            decoded.validity = FrameValidity::IntegrityInvalid;
            output           = decoded;
            return StatusCode::Ok;
        }

        decoded.validity = FrameValidity::Valid;
        decoded.address  = address;
        decoded.command  = command;
        output           = decoded;
        return StatusCode::Ok;
    }
} // namespace adk
