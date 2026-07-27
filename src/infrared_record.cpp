#include "infrared_record.h"

namespace adk {

    namespace {

        constexpr uint8_t recordCapacity = 80;

        bool protocolName (InfraredProtocol protocol, const char*& name) noexcept
        {
            switch (protocol)
            {
                case InfraredProtocol::Unknown: name = "UNKNOWN"; return true;
                case InfraredProtocol::Nec: name = "NEC"; return true;
            }

            return false;
        }

        bool validityName (FrameValidity validity, const char*& name) noexcept
        {
            switch (validity)
            {
                case FrameValidity::Valid: name = "VALID"; return true;
                case FrameValidity::Repeat: name = "REPEAT"; return true;
                case FrameValidity::UnknownProtocol:
                    name = "UNKNOWN_PROTOCOL";
                    return true;
                case FrameValidity::TimingInvalid: name = "TIMING_INVALID"; return true;
                case FrameValidity::IntegrityInvalid:
                    name = "INTEGRITY_INVALID";
                    return true;
                case FrameValidity::Truncated: name = "TRUNCATED"; return true;
                case FrameValidity::Overflow: name = "OVERFLOW"; return true;
            }

            return false;
        }

        bool appendCharacter (char* storage, uint8_t& size, char value) noexcept
        {
            if (size >= recordCapacity)
            {
                return false;
            }

            storage[size++] = value;
            return true;
        }

        bool appendText (char* storage, uint8_t& size, const char* text) noexcept
        {
            for (uint8_t index = 0; text[index] != '\0'; ++index)
            {
                if (!appendCharacter (storage, size, text[index]))
                {
                    return false;
                }
            }

            return true;
        }

        bool appendDecimal (char* storage, uint8_t& size, uint32_t value) noexcept
        {
            char    reversed[10];
            uint8_t digits = 0;

            do
            {
                reversed[digits++] = static_cast<char> ('0' + value % 10);
                value /= 10;
            }
            while (value != 0);

            while (digits > 0)
            {
                if (!appendCharacter (storage, size, reversed[--digits]))
                {
                    return false;
                }
            }

            return true;
        }

        bool appendHex32 (char* storage, uint8_t& size, uint32_t value) noexcept
        {
            constexpr char digits[] = "0123456789ABCDEF";

            for (int8_t shift = 28; shift >= 0; shift -= 4)
            {
                const uint8_t digit = static_cast<uint8_t> (value >> shift) & 0x0f;

                if (!appendCharacter (storage, size, digits[digit]))
                {
                    return false;
                }
            }

            return true;
        }
    } // namespace

    Result<uint16_t>
    InfraredRecordEncoder::encode (const InfraredFrame& frame,
                                   MutableTextSpan      output) const noexcept
    {
        const char* protocol = nullptr;
        const char* validity = nullptr;

        if (!protocolName (frame.protocol, protocol) ||
            !validityName (frame.validity, validity))
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }

        if ((frame.validity == FrameValidity::Valid &&
             frame.protocol != InfraredProtocol::Nec) ||
            (frame.validity == FrameValidity::Repeat &&
             frame.protocol != InfraredProtocol::Nec))
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }

        const bool     hasNumericMeaning = frame.validity == FrameValidity::Valid;
        const uint32_t address           = hasNumericMeaning ? frame.address : 0;
        const uint32_t command           = hasNumericMeaning ? frame.command : 0;
        char           record[recordCapacity];
        uint8_t        size = 0;

        const bool encoded = appendText      (record, size, "IR1,") &&
                             appendDecimal   (record, size, frame.captureSequence) &&
                             appendCharacter (record, size, ',') &&
                             appendText      (record, size, protocol) &&
                             appendCharacter (record, size, ',') &&
                             appendText      (record, size, validity) &&
                             appendCharacter (record, size, ',') &&
                             appendHex32     (record, size, address) &&
                             appendCharacter (record, size, ',') &&
                             appendHex32     (record, size, command) &&
                             appendCharacter (record, size, '\n');

        if (!encoded)
        {
            return Result<uint16_t> (StatusCode::CapacityExceeded, 0);
        }

        if (output.capacity < size || (size > 0 && output.data == nullptr))
        {
            return Result<uint16_t> (StatusCode::CapacityExceeded, 0);
        }

        for (uint8_t index = 0; index < size; ++index)
        {
            output.data[index] = record[index];
        }

        return Result<uint16_t> (StatusCode::Ok, size);
    }
} // namespace adk
