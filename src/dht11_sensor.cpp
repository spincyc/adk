#include "dht11_sensor.h"

#include "board.h"

#include <Arduino.h>
#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t stabilizationMilliseconds = 1000U;
        constexpr uint32_t acquisitionMilliseconds   = 1000U;
        constexpr uint32_t maximumUnambiguousDuration =
            static_cast<uint32_t> (INT32_MAX);

        constexpr uint16_t requestLowMicroseconds = 18000U;
        constexpr uint16_t responseWaitMaximum    = 120U;
        constexpr uint16_t responsePulseMinimum   = 60U;
        constexpr uint16_t responsePulseMaximum   = 110U;
        constexpr uint16_t responsePulseValidMaximum = 100U;
        constexpr uint16_t bitLowMinimum          = 35U;
        constexpr uint16_t bitLowMaximum          = 70U;
        constexpr uint16_t bitPulseMaximum        = 100U;
        constexpr uint16_t zeroMinimum            = 15U;
        constexpr uint16_t zeroMaximum            = 40U;
        constexpr uint16_t oneMinimum             = 55U;
        constexpr uint16_t oneMaximum             = 85U;

        constexpr ClimateSampleLimits dht11Limits = {0, 5000, 1000};

        ClimateSample unavailableSample () noexcept
        {
            return {0, 0, TimePoint (0), ClimateSampleState::Unavailable};
        }

        ClimateSample sampledFault (ClimateSampleState state, TimePoint now) noexcept
        {
            return {0, 0, now, state};
        }

        bool due (TimePoint now, TimePoint earlier, uint32_t interval) noexcept
        {
            const uint32_t elapsed = now.elapsedSince (earlier).milliseconds ();
            return elapsed <= maximumUnambiguousDuration && elapsed >= interval;
        }

        bool inRange (uint16_t value,
                      uint16_t minimum,
                      uint16_t maximum) noexcept
        {
            return value >= minimum && value <= maximum;
        }
    } // namespace

    Dht11Transport::~Dht11Transport () noexcept = default;

    Dht11Sensor::Dht11Sensor (ResourceRegistry& resources,
                              PinId             dataPin) noexcept
        : resources_         (&resources)
        , claim_             ()
        , transport_         (nullptr)
        , dataPin_           (dataPin)
        , sample_            (unavailableSample ())
        , initializedAt_     (0)
        , lastAcquiredAt_    (0)
        , initialized_       (false)
        , hasScheduleAnchor_ (false)
        , hasAcquired_       (false)
    {
    }

    Dht11Sensor::Dht11Sensor (ResourceRegistry& resources,
                              PinId             dataPin,
                              Dht11Transport&   transport) noexcept
        : resources_         (&resources)
        , claim_             ()
        , transport_         (&transport)
        , dataPin_           (dataPin)
        , sample_            (unavailableSample ())
        , initializedAt_     (0)
        , lastAcquiredAt_    (0)
        , initialized_       (false)
        , hasScheduleAnchor_ (false)
        , hasAcquired_       (false)
    {
    }

    Dht11Sensor::~Dht11Sensor () noexcept
    {
        shutdown ();
    }

    Status Dht11Sensor::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!Mega2560Board::validPin (dataPin_))
        {
            return StatusCode::InvalidPin;
        }

        if (!Mega2560Board::supports (dataPin_, PinCapability::DigitalInput) ||
            !Mega2560Board::supports (dataPin_, PinCapability::DigitalOutput))
        {
            return StatusCode::Unsupported;
        }

        const ResourceId resource = {ResourceKind::Pin, dataPin_};
        const Status     status   = resources_->claim (resource, claim_);

        if (!status.ok ())
        {
            return status;
        }

        pinMode (dataPin_, INPUT);

        sample_        = unavailableSample ();
        initializedAt_ = TimePoint         (0);
        initialized_   = true;
        hasScheduleAnchor_ = false;
        hasAcquired_   = false;
        return StatusCode::Ok;
    }

    void Dht11Sensor::shutdown () noexcept
    {
        if (claim_.active ())
        {
            pinMode        (dataPin_, INPUT);
            claim_.release ();
        }

        sample_       = unavailableSample ();
        initialized_  = false;
        hasScheduleAnchor_ = false;
        hasAcquired_  = false;
    }

    bool Dht11Sensor::initialized () const noexcept
    {
        return initialized_;
    }

    Status Dht11Sensor::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!hasScheduleAnchor_)
        {
            initializedAt_      = now;
            hasScheduleAnchor_ = true;
            return StatusCode::Ok;
        }

        const TimePoint scheduleAnchor =
            hasAcquired_ ? lastAcquiredAt_ : initializedAt_;
        const uint32_t interval =
            hasAcquired_ ? acquisitionMilliseconds : stabilizationMilliseconds;

        if (!due (now, scheduleAnchor, interval))
        {
            const uint32_t elapsed = now.elapsedSince (scheduleAnchor).milliseconds ();
            return elapsed > maximumUnambiguousDuration ? StatusCode::InvalidArgument
                                                        : StatusCode::Ok;
        }

        uint8_t      bytes[5]        = {};
        const Status transportStatus = transact (bytes);

        lastAcquiredAt_ = now;
        hasAcquired_    = true;

        if (!transportStatus.ok ())
        {
            storeFault (ClimateSampleState::TransportTimeout, now);
            return StatusCode::HardwareFailure;
        }

        const uint8_t checksum =
            static_cast<uint8_t> (bytes[0] + bytes[1] + bytes[2] + bytes[3]);

        if (checksum != bytes[4])
        {
            storeFault (ClimateSampleState::ChecksumFailure, now);
            return StatusCode::HardwareFailure;
        }

        const uint16_t humidity = static_cast<uint16_t> (
            static_cast<uint16_t> (bytes[0]) * 10U + bytes[1]);
        const int16_t  temperature =
            static_cast<int16_t> (static_cast<uint16_t> (bytes[2]) * 100U +
                                  static_cast<uint16_t> (bytes[3]) * 10U);

        sample_ = validateClimateSample (temperature, humidity, now, dht11Limits);
        return sample_.state == ClimateSampleState::Valid ? StatusCode::Ok
                                                          : StatusCode::InvalidArgument;
    }

    ClimateSample Dht11Sensor::sample (TimePoint now,
                                       Duration  staleAfter) const noexcept
    {
        ClimateSample current = sample_;

        if (current.state != ClimateSampleState::Valid)
        {
            return current;
        }

        const uint32_t age = now.elapsedSince (current.observedAt).milliseconds ();

        if (staleAfter.milliseconds () > maximumUnambiguousDuration ||
            age > maximumUnambiguousDuration)
        {
            current.state = ClimateSampleState::InvalidTiming;
        }
        else if (age > staleAfter.milliseconds ())
        {
            current.state = ClimateSampleState::Stale;
        }

        return current;
    }

    PinId Dht11Sensor::dataPin () const noexcept
    {
        return dataPin_;
    }

    Status Dht11Sensor::transact (uint8_t (&bytes)[5]) noexcept
    {
        Status status = driveLow ();

        if (status.ok ())
        {
            waitUs               (requestLowMicroseconds);
            status = releaseLine ();
        }

        if (status.ok ())
        {
            status = waitForLevel (Level::Low, responseWaitMaximum);
        }

        uint16_t width = 0;

        if (status.ok ())
        {
            status = measureLevel (Level::Low, responsePulseMaximum, width);

            if (status.ok () &&
                !inRange (width, responsePulseMinimum,
                          responsePulseValidMaximum))
            {
                status = StatusCode::HardwareFailure;
            }
        }

        if (status.ok ())
        {
            status = measureLevel (Level::High, responsePulseMaximum, width);

            if (status.ok () &&
                !inRange (width, responsePulseMinimum,
                          responsePulseValidMaximum))
            {
                status = StatusCode::HardwareFailure;
            }
        }

        for (uint8_t bit = 0; status.ok () && bit < 40U; ++bit)
        {
            status = measureLevel (Level::Low, bitPulseMaximum, width);

            if (status.ok () &&
                !inRange (width, bitLowMinimum, bitLowMaximum))
            {
                status = StatusCode::HardwareFailure;
            }
            else if (status.ok ())
            {
                status = measureLevel (Level::High, bitPulseMaximum, width);
            }

            if (!status.ok ())
            {
                break;
            }

            uint8_t value = 0;

            if (inRange (width, zeroMinimum, zeroMaximum))
            {
                value = 0;
            }
            else if (inRange (width, oneMinimum, oneMaximum))
            {
                value = 1;
            }
            else
            {
                status = StatusCode::HardwareFailure;
                break;
            }

            bytes[bit / 8U] = static_cast<uint8_t> (
                static_cast<uint8_t> (bytes[bit / 8U] << 1U) | value);
        }

        pinMode (dataPin_, INPUT);
        return status;
    }

    Status Dht11Sensor::waitForLevel (Level level, uint16_t timeout) noexcept
    {
        const uint32_t startedAt = nowUs ();

        while (static_cast<uint32_t> (nowUs () - startedAt) <= timeout)
        {
            const Result<Level> observed = readLine ();

            if (!observed.ok ())
            {
                return observed.status ();
            }

            if (observed.value () == level)
            {
                return StatusCode::Ok;
            }

            waitUs (1);
        }

        return StatusCode::HardwareFailure;
    }

    Status Dht11Sensor::measureLevel (Level level, uint16_t timeout,
                                      uint16_t& width) noexcept
    {
        Status status = waitForLevel (level, timeout);

        if (!status.ok ())
        {
            return status;
        }

        const uint32_t startedAt = nowUs ();

        while (static_cast<uint32_t> (nowUs () - startedAt) <= timeout)
        {
            const Result<Level> observed = readLine ();

            if (!observed.ok ())
            {
                return observed.status ();
            }

            if (observed.value () != level)
            {
                width = static_cast<uint16_t> (nowUs () - startedAt);
                return StatusCode::Ok;
            }

            waitUs (1);
        }

        return StatusCode::HardwareFailure;
    }

    Status Dht11Sensor::driveLow () noexcept
    {
        if (transport_ != nullptr)
        {
            return transport_->driveLow (dataPin_);
        }

        digitalWrite (dataPin_, LOW);
        pinMode      (dataPin_, OUTPUT);
        return StatusCode::Ok;
    }

    Status Dht11Sensor::releaseLine () noexcept
    {
        if (transport_ != nullptr)
        {
            return transport_->release (dataPin_);
        }

        pinMode (dataPin_, INPUT);
        return StatusCode::Ok;
    }

    Result<Level> Dht11Sensor::readLine () noexcept
    {
        if (transport_ != nullptr)
        {
            return transport_->read (dataPin_);
        }

        const Level level = digitalRead (dataPin_) == HIGH ? Level::High : Level::Low;
        return Result<Level> (StatusCode::Ok, level);
    }

    void Dht11Sensor::waitUs (uint16_t duration) noexcept
    {
        if (transport_ != nullptr)
        {
            transport_->waitMicroseconds (duration);
        }
        else
        {
            delayMicroseconds (duration);
        }
    }

    uint32_t Dht11Sensor::nowUs () const noexcept
    {
        return transport_ != nullptr ? transport_->microseconds ()
                                     : static_cast<uint32_t> (micros ());
    }

    void Dht11Sensor::storeFault (ClimateSampleState state, TimePoint now) noexcept
    {
        sample_ = sampledFault (state, now);
    }
} // namespace adk
