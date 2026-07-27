#include "character_display.h"

#include "board.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr uint8_t pinCount = 6;

        struct ArduinoHd44780Transport final : Hd44780Transport
        {
            Status configureOutput (PinId pin) noexcept override
            {
                digitalWrite (pin, LOW);
                pinMode      (pin, OUTPUT);
                return StatusCode::Ok;
            }

            void release (PinId pin) noexcept override
            {
                pinMode (pin, INPUT);
            }

            Status write (PinId pin, Level level) noexcept override
            {
                digitalWrite (pin, level == Level::High ? HIGH : LOW);
                return StatusCode::Ok;
            }

            void waitMicroseconds (uint16_t duration) noexcept override
            {
                delayMicroseconds (duration);
            }
        };

        ArduinoHd44780Transport arduinoTransport;

        bool due (TimePoint now, TimePoint earlier, uint32_t interval) noexcept
        {
            const uint32_t elapsed = now.elapsedSince (earlier).milliseconds ();
            return elapsed <= static_cast<uint32_t> (INT32_MAX) && elapsed >= interval;
        }

        const char* sampleStateName (ClimateSampleState state) noexcept
        {
            switch (state)
            {
                case ClimateSampleState::Unavailable: return "unavailable";
                case ClimateSampleState::Valid: return "valid";
                case ClimateSampleState::TransportTimeout: return "transport_timeout";
                case ClimateSampleState::ChecksumFailure: return "checksum_failure";
                case ClimateSampleState::TemperatureOutOfRange:
                    return "temperature_range";
                case ClimateSampleState::HumidityOutOfRange: return "humidity_range";
                case ClimateSampleState::Stale: return "stale";
                case ClimateSampleState::InvalidLimits: return "invalid_limits";
                case ClimateSampleState::InvalidTiming: return "invalid_timing";
            }

            return "unknown";
        }

        struct RecordWriter
        {
            char*  output;
            size_t capacity;
            size_t length;
            bool   fits;

            void append (char value) noexcept
            {
                if (length + 1U < capacity)
                {
                    output[length] = value;
                }
                else
                {
                    fits = false;
                }

                ++length;
            }

            void append (const char* text) noexcept
            {
                while (*text != '\0')
                {
                    append (*text++);
                }
            }

            void appendUnsigned (uint32_t value) noexcept
            {
                char    digits[10];
                uint8_t count = 0;

                do
                {
                    digits[count++] =
                        static_cast<char> ('0' + static_cast<char> (value % 10U));
                    value /= 10U;
                }
                while (value != 0U);

                while (count != 0U)
                {
                    append (digits[--count]);
                }
            }

            void appendSigned (int16_t value) noexcept
            {
                if (value < 0)
                {
                    append         ('-');
                    appendUnsigned (
                        static_cast<uint16_t> (-(static_cast<int32_t> (value))));
                }
                else
                {
                    appendUnsigned (static_cast<uint16_t> (value));
                }
            }

            Status finish () noexcept
            {
                if (capacity != 0U)
                {
                    const size_t terminator =
                        length < capacity ? length : capacity - 1U;
                    output[terminator] = '\0';
                }

                return fits ? StatusCode::Ok : StatusCode::CapacityExceeded;
            }
        };
    } // namespace

    CharacterDisplay::~CharacterDisplay () noexcept = default;
    Hd44780Transport::~Hd44780Transport () noexcept = default;

    Hd44780Display::Hd44780Display (ResourceRegistry&  resources,
                                    const Hd44780Pins& pins) noexcept
        : Hd44780Display (resources, pins, arduinoTransport)
    {
    }

    Hd44780Display::Hd44780Display (ResourceRegistry&  resources,
                                    const Hd44780Pins& pins,
                                    Hd44780Transport&  transport) noexcept
        : resources_        (&resources)
        , claims_           ()
        , pins_             (pins)
        , transport_        (&transport)
        , desired_          ()
        , shown_            ()
        , startupAt_        (0)
        , startup_          (Startup::AwaitPower)
        , flushIndex_       (0)
        , initialized_      (false)
        , hasStartupAnchor_ (false)
    {
        clearLines ();
    }

    Hd44780Display::~Hd44780Display () noexcept
    {
        shutdown ();
    }

    Status Hd44780Display::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        Status status = claimPins ();

        if (status.ok ())
        {
            status = configurePins ();
        }

        if (!status.ok ())
        {
            shutdown ();
            return status;
        }

        clearLines ();
        startup_          = Startup::AwaitPower;
        flushIndex_       = 0;
        initialized_      = true;
        hasStartupAnchor_ = false;
        return StatusCode::Ok;
    }

    void Hd44780Display::shutdown () noexcept
    {
        if (initialized_ && ready ())
        {
            writeByte (false, 0x08U);
        }

        const PinId pins[] = {pins_.registerSelect, pins_.enable, pins_.data4,
                              pins_.data5,          pins_.data6,  pins_.data7};

        for (uint8_t index = pinCount; index != 0U; --index)
        {
            if (claims_[index - 1U].active ())
            {
                transport_->write   (pins[index - 1U], Level::Low);
                transport_->release (pins[index - 1U]);

                claims_[index - 1U].release ();
            }
        }

        clearLines ();
        startup_          = Startup::AwaitPower;
        flushIndex_       = 0;
        initialized_      = false;
        hasStartupAnchor_ = false;
    }

    bool Hd44780Display::initialized () const noexcept
    {
        return initialized_;
    }

    Status Hd44780Display::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!ready ())
        {
            return advanceStartup (now);
        }

        return flushCell ();
    }

    bool Hd44780Display::ready () const noexcept
    {
        return initialized_ && startup_ == Startup::Ready;
    }

    Status Hd44780Display::show (uint8_t row, const char* text) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (row >= characterDisplayRows || text == nullptr)
        {
            return StatusCode::InvalidArgument;
        }

        uint8_t column = 0;

        while (column < characterDisplayColumns && text[column] != '\0')
        {
            const uint8_t value = static_cast<uint8_t> (text[column]);

            if (value < 0x20U || value > 0x7eU)
            {
                return StatusCode::InvalidArgument;
            }

            ++column;
        }

        if (column == characterDisplayColumns && text[column] != '\0')
        {
            return StatusCode::CapacityExceeded;
        }

        for (column = 0; column < characterDisplayColumns; ++column)
        {
            desired_[row][column] = text[column] == '\0' ? ' ' : text[column];

            if (text[column] == '\0')
            {
                while (++column < characterDisplayColumns)
                {
                    desired_[row][column] = ' ';
                }

                break;
            }
        }

        return StatusCode::Ok;
    }

    const char* Hd44780Display::line (uint8_t row) const noexcept
    {
        return row < characterDisplayRows ? desired_[row] : nullptr;
    }

    const Hd44780Pins& Hd44780Display::pins () const noexcept
    {
        return pins_;
    }

    Status Hd44780Display::claimPins () noexcept
    {
        const PinId pins[] = {pins_.registerSelect, pins_.enable, pins_.data4,
                              pins_.data5,          pins_.data6,  pins_.data7};

        for (uint8_t index = 0; index < pinCount; ++index)
        {
            if (!Mega2560Board::validPin (pins[index]) ||
                !Mega2560Board::supports (pins[index], PinCapability::DigitalOutput))
            {
                return StatusCode::InvalidPin;
            }

            for (uint8_t earlier = 0; earlier < index; ++earlier)
            {
                if (pins[index] == pins[earlier])
                {
                    return StatusCode::InvalidArgument;
                }
            }
        }

        for (uint8_t index = 0; index < pinCount; ++index)
        {
            const Status status =
                resources_->claim ({ResourceKind::Pin, pins[index]}, claims_[index]);

            if (!status.ok ())
            {
                return status;
            }
        }

        return StatusCode::Ok;
    }

    Status Hd44780Display::configurePins () noexcept
    {
        const PinId pins[] = {pins_.registerSelect, pins_.enable, pins_.data4,
                              pins_.data5,          pins_.data6,  pins_.data7};

        for (uint8_t index = 0; index < pinCount; ++index)
        {
            const Status status = transport_->configureOutput (pins[index]);

            if (!status.ok ())
            {
                return status;
            }
        }

        return StatusCode::Ok;
    }

    Status Hd44780Display::advanceStartup (TimePoint now) noexcept
    {
        if (!hasStartupAnchor_)
        {
            startupAt_        = now;
            hasStartupAnchor_ = true;
            return StatusCode::Ok;
        }

        const uint32_t wait = startup_ == Startup::AwaitPower ? 15U
                              : startup_ == Startup::WakeOne  ? 5U
                              : startup_ == Startup::Clear    ? 2U
                                                              : 1U;

        if (!due (now, startupAt_, wait))
        {
            return StatusCode::Ok;
        }

        Status status = StatusCode::Ok;

        switch (startup_)
        {
            case Startup::AwaitPower: status = writeNibble (false, 0x03U); break;

            case Startup::WakeOne: status = writeNibble (false, 0x03U); break;

            case Startup::WakeTwo: status = writeNibble (false, 0x03U); break;

            case Startup::WakeThree: status = writeNibble (false, 0x02U); break;

            case Startup::FourBit: status = writeByte (false, 0x28U); break;

            case Startup::Function: status = writeByte (false, 0x08U); break;

            case Startup::DisplayOff: status = writeByte (false, 0x01U); break;

            case Startup::Clear: status = writeByte (false, 0x06U); break;

            case Startup::Entry: status = writeByte (false, 0x0cU); break;
            case Startup::DisplayOn: startup_ = Startup::Ready; return StatusCode::Ok;
            case Startup::Ready: return StatusCode::Ok;
        }

        if (status.ok ())
        {
            startup_   = static_cast<Startup> (static_cast<uint8_t> (startup_) + 1U);
            startupAt_ = now;
        }

        return status;
    }

    Status Hd44780Display::flushCell () noexcept
    {
        for (uint8_t offset = 0;
             offset < characterDisplayRows * characterDisplayColumns; ++offset)
        {
            const uint8_t index =
                static_cast<uint8_t> ((flushIndex_ + offset) %
                                      (characterDisplayRows * characterDisplayColumns));
            const uint8_t row    = index / characterDisplayColumns;
            const uint8_t column = index % characterDisplayColumns;

            if (desired_[row][column] == shown_[row][column])
            {
                continue;
            }

            const uint8_t address =
                static_cast<uint8_t> ((row == 0U ? 0x00U : 0x40U) + column);
            Status status = writeByte (false, static_cast<uint8_t> (0x80U | address));

            if (status.ok ())
            {
                status = writeByte (true, static_cast<uint8_t> (desired_[row][column]));
            }

            if (status.ok ())
            {
                shown_[row][column] = desired_[row][column];
                flushIndex_         = static_cast<uint8_t> (
                    (index + 1U) % (characterDisplayRows * characterDisplayColumns));
            }

            return status;
        }

        return StatusCode::Ok;
    }

    Status Hd44780Display::writeByte (bool registerSelect, uint8_t value) noexcept
    {
        Status status =
            writeNibble (registerSelect, static_cast<uint8_t> (value >> 4U));

        if (status.ok ())
        {
            status = writeNibble (registerSelect, static_cast<uint8_t> (value & 0x0fU));
        }

        return status;
    }

    Status Hd44780Display::writeNibble (bool registerSelect, uint8_t value) noexcept
    {
        const PinId dataPins[] = {pins_.data4, pins_.data5, pins_.data6, pins_.data7};
        Status status = transport_->write (pins_.registerSelect,
                                           registerSelect ? Level::High : Level::Low);

        for (uint8_t bit = 0; status.ok () && bit < 4U; ++bit)
        {
            status = transport_->write (dataPins[bit],
                                        (value & static_cast<uint8_t> (1U << bit)) != 0U
                                            ? Level::High
                                            : Level::Low);
        }

        if (status.ok ())
        {
            status = transport_->write (pins_.enable, Level::High);
        }

        if (status.ok ())
        {
            transport_->waitMicroseconds (1);
            status = transport_->write   (pins_.enable, Level::Low);
        }

        return status;
    }

    void Hd44780Display::clearLines () noexcept
    {
        for (uint8_t row = 0; row < characterDisplayRows; ++row)
        {
            for (uint8_t column = 0; column < characterDisplayColumns; ++column)
            {
                desired_[row][column] = ' ';
                shown_[row][column]   = '\0';
            }

            desired_[row][characterDisplayColumns] = '\0';
            shown_[row][characterDisplayColumns]   = '\0';
        }
    }

    Status formatClimateRecord (const ClimateSample& sample, uint32_t sequence,
                                char* output, size_t capacity) noexcept
    {
        if (output == nullptr || capacity == 0U)
        {
            return StatusCode::InvalidArgument;
        }

        RecordWriter writer = {output, capacity, 0, true};
        writer.append         ("seq=");
        writer.appendUnsigned (sequence);
        writer.append         (",state=");
        writer.append         (sampleStateName (sample.state));
        writer.append         (",temp_centi_c=");
        writer.appendSigned   (sample.temperatureCentiCelsius);
        writer.append         (",rh_permille=");
        writer.appendUnsigned (sample.humidityPermille);
        writer.append         (",at_ms=");
        writer.appendUnsigned (sample.observedAt.milliseconds ());
        writer.append         ('\n');

        return writer.finish ();
    }
} // namespace adk
