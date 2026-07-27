#pragma once

#include "climate_sensor.h"
#include "digital.h"
#include "resource.h"
#include "status.h"
#include "time.h"

#include <stddef.h>
#include <stdint.h>

namespace adk {

    constexpr uint8_t characterDisplayRows    = 2;
    constexpr uint8_t characterDisplayColumns = 16;

    struct CharacterDisplay
    {
        virtual ~CharacterDisplay () noexcept;

        virtual Status      initialize  () noexcept                        = 0;
        virtual void        shutdown    () noexcept                        = 0;
        virtual bool        initialized () const noexcept                  = 0;
        virtual Status      update      (TimePoint now) noexcept           = 0;
        virtual bool        ready       () const noexcept                  = 0;
        virtual Status      show        (uint8_t row,
                                         const char* text) noexcept        = 0;
        virtual const char* line        (uint8_t row) const noexcept       = 0;
    };

    struct Hd44780Pins
    {
        PinId registerSelect;
        PinId enable;
        PinId data4;
        PinId data5;
        PinId data6;
        PinId data7;
    };

    struct Hd44780Transport
    {
        virtual ~Hd44780Transport () noexcept;

        virtual Status configureOutput  (PinId pin) noexcept          = 0;
        virtual void   release          (PinId pin) noexcept          = 0;
        virtual Status write            (PinId pin,
                                         Level level) noexcept        = 0;
        virtual void   waitMicroseconds (uint16_t duration) noexcept = 0;
    };

    struct Hd44780Display final : CharacterDisplay
    {
        Hd44780Display (ResourceRegistry& resources, const Hd44780Pins& pins) noexcept;
        Hd44780Display (ResourceRegistry& resources, const Hd44780Pins& pins,
                        Hd44780Transport& transport) noexcept;
        ~Hd44780Display () noexcept override;

        Hd44780Display (const Hd44780Display&)            = delete;
        Hd44780Display& operator= (const Hd44780Display&) = delete;
        Hd44780Display (Hd44780Display&&)                 = delete;
        Hd44780Display& operator= (Hd44780Display&&)      = delete;

        Status      initialize  () noexcept override;
        void        shutdown    () noexcept override;
        bool        initialized () const noexcept override;
        Status      update      (TimePoint now) noexcept override;
        bool        ready       () const noexcept override;
        Status      show        (uint8_t row,
                                 const char* text) noexcept override;
        const char* line        (uint8_t row) const noexcept override;

        const Hd44780Pins& pins () const noexcept;

      private:
        enum struct Startup : uint8_t
        {
            AwaitPower,
            WakeOne,
            WakeTwo,
            WakeThree,
            FourBit,
            Function,
            DisplayOff,
            Clear,
            Entry,
            DisplayOn,
            Ready
        };

        Status claimPins      () noexcept;
        Status configurePins  () noexcept;
        Status advanceStartup (TimePoint now) noexcept;
        Status flushCell      () noexcept;
        Status writeByte      (bool    registerSelect,
                               uint8_t value) noexcept;
        Status writeNibble    (bool    registerSelect,
                               uint8_t value) noexcept;
        void   clearLines     () noexcept;

        ResourceRegistry* resources_;
        ResourceClaim     claims_[6];
        Hd44780Pins       pins_;
        Hd44780Transport* transport_;
        char              desired_[characterDisplayRows][characterDisplayColumns + 1];
        char              shown_[characterDisplayRows][characterDisplayColumns + 1];
        TimePoint         startupAt_;
        Startup           startup_;
        uint8_t           flushIndex_;
        bool              initialized_;
        bool              hasStartupAnchor_;
    };

    Status formatClimateRecord (const ClimateSample& sample, uint32_t sequence,
                                char* output, size_t capacity) noexcept;
} // namespace adk
