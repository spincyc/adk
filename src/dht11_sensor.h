#pragma once

#include "climate_sensor.h"
#include "digital.h"
#include "resource.h"

#include <stdint.h>

namespace adk {

    struct Dht11Transport
    {
        virtual ~Dht11Transport () noexcept;

        virtual Status        driveLow         (PinId pin) noexcept            = 0;
        virtual Status        release          (PinId pin) noexcept            = 0;
        virtual Result<Level> read             (PinId pin) noexcept            = 0;
        virtual void          waitMicroseconds (uint16_t duration) noexcept    = 0;
        virtual uint32_t      microseconds     () const noexcept               = 0;
    };

    struct Dht11Sensor final : ClimateSensor
    {
        Dht11Sensor  (ResourceRegistry& resources,
                      PinId             dataPin) noexcept;
        Dht11Sensor  (ResourceRegistry& resources,
                      PinId             dataPin,
                      Dht11Transport&   transport) noexcept;
        ~Dht11Sensor () noexcept override;

        Dht11Sensor  (const Dht11Sensor&)            = delete;
        Dht11Sensor& operator= (const Dht11Sensor&) = delete;
        Dht11Sensor  (Dht11Sensor&&)                 = delete;
        Dht11Sensor& operator= (Dht11Sensor&&)      = delete;

        Status        initialize  () noexcept override;
        void          shutdown    () noexcept override;
        bool          initialized () const noexcept override;
        Status        update      (TimePoint now) noexcept override;
        ClimateSample sample      (TimePoint now,
                                   Duration  staleAfter) const noexcept override;

        PinId dataPin () const noexcept;

      private:
        Status        transact     (uint8_t (&bytes)[5]) noexcept;
        Status        waitForLevel (Level level,
                                    uint16_t timeout) noexcept;
        Status        measureLevel (Level level,
                                    uint16_t timeout,
                                    uint16_t& width) noexcept;
        Status        driveLow     () noexcept;
        Status        releaseLine  () noexcept;
        Result<Level> readLine     () noexcept;
        void          waitUs       (uint16_t duration) noexcept;
        uint32_t      nowUs        () const noexcept;
        void          storeFault   (ClimateSampleState state,
                                    TimePoint          now) noexcept;

        ResourceRegistry* resources_;
        ResourceClaim     claim_;
        Dht11Transport*   transport_;
        PinId             dataPin_;
        ClimateSample     sample_;
        TimePoint         initializedAt_;
        TimePoint         lastAcquiredAt_;
        bool              initialized_;
        bool              hasScheduleAnchor_;
        bool              hasAcquired_;
    };
} // namespace adk
