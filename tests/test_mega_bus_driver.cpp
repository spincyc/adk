#include "mega_bus_driver.h"

#include <cstdlib>
#include <type_traits>

namespace {

    void require (bool condition)
    {
        if (!condition)
        {
            std::abort ();
        }
    }

    struct RecordingI2cIo final : adk::MegaI2cIo
    {
        adk::Status   configureStatus = adk::StatusCode::Ok;
        adk::Status   transferStatus  = adk::StatusCode::Ok;
        uint8_t       configureCount  = 0;
        uint8_t       disableCount    = 0;
        uint8_t       transferCount   = 0;
        adk::Duration timeout         = adk::Duration ();

        adk::Status configure () noexcept override
        {
            ++configureCount;
            return configureStatus;
        }

        void disable () noexcept override
        {
            ++disableCount;
        }

        adk::Status transfer (uint8_t, const uint8_t*, uint8_t, uint8_t*, uint8_t,
                              adk::Duration transferTimeout) noexcept override
        {
            ++transferCount;
            timeout = transferTimeout;
            return transferStatus;
        }
    };

    enum struct SpiTrace : uint8_t
    {
        ConfigureMaster,
        Disable,
        ConfigureChipSelect,
        ReleaseChipSelect,
        Begin,
        Transfer,
        End
    };

    struct RecordingSpiIo final : adk::MegaSpiIo
    {
        static const uint8_t traceCapacity = 16;

        adk::Status   configureStatus      = adk::StatusCode::Ok;
        adk::Status   chipSelectStatus     = adk::StatusCode::Ok;
        adk::Status   beginStatus          = adk::StatusCode::Ok;
        adk::Status   transferStatus       = adk::StatusCode::Ok;
        adk::Status   endStatus            = adk::StatusCode::Ok;
        SpiTrace      trace[traceCapacity] = {};
        uint8_t       traceSize            = 0;
        adk::PinId    chipSelect           = 0;
        adk::Duration timeout              = adk::Duration ();
        bool          chipSelectConfigured = false;

        void record (SpiTrace event) noexcept
        {
            if (traceSize < traceCapacity)
            {
                trace[traceSize++] = event;
            }
        }

        adk::Status configureMaster () noexcept override
        {
            record (SpiTrace::ConfigureMaster);
            return configureStatus;
        }

        void disable () noexcept override
        {
            record (SpiTrace::Disable);
        }

        adk::Status configureChipSelect (adk::PinId selected) noexcept override
        {
            chipSelect = selected;
            chipSelectConfigured = true;
            record (SpiTrace::ConfigureChipSelect);
            return chipSelectStatus;
        }

        void releaseChipSelect (adk::PinId selected) noexcept override
        {
            chipSelect = selected;
            chipSelectConfigured = false;
            record (SpiTrace::ReleaseChipSelect);
        }

        adk::Status beginTransaction (const adk::SpiSettings&,
                                      adk::PinId selected) noexcept override
        {
            chipSelect = selected;
            record (SpiTrace::Begin);
            return beginStatus;
        }

        adk::Status transfer (const uint8_t*, uint8_t*, uint16_t,
                              adk::Duration transferTimeout) noexcept override
        {
            timeout = transferTimeout;
            record (SpiTrace::Transfer);
            return transferStatus;
        }

        adk::Status endTransaction (adk::PinId selected) noexcept override
        {
            chipSelect = selected;
            record (SpiTrace::End);
            return endStatus;
        }
    };

    void provesI2cOwnershipAndRollback ()
    {
        adk::ResourceRegistry resources;
        RecordingI2cIo        io;
        adk::MegaI2cDriver    driver (resources, io);

        require (driver.configure ().ok ());
        require (driver.configure ().ok ());
        require (io.configureCount == 1);
        require (resources.claimed ({adk::ResourceKind::Pin, 20}));
        require (resources.claimed ({adk::ResourceKind::Pin, 21}));

        driver.disable        ();
        driver.disable        ();
        require               (!resources.claimed ({adk::ResourceKind::Pin, 20}));
        require               (!resources.claimed ({adk::ResourceKind::Pin, 21}));

        io.configureStatus = adk::StatusCode::HardwareFailure;
        require (driver.configure ().error () == adk::StatusCode::HardwareFailure);
        require (!resources.claimed ({adk::ResourceKind::Pin, 20}));
        require (!resources.claimed ({adk::ResourceKind::Pin, 21}));
        require (io.disableCount == 2);
    }

    void provesI2cConflictAndTimeout ()
    {
        adk::ResourceRegistry resources;
        adk::ResourceClaim    blocker;
        RecordingI2cIo        io;
        adk::MegaI2cDriver    driver (resources, io);

        require         (resources.claim ({adk::ResourceKind::Pin, 21}, blocker).ok ());
        require         (driver.configure ().error () == adk::StatusCode::ResourceBusy);
        require         (!resources.claimed ({adk::ResourceKind::Pin, 20}));
        require         (io.configureCount == 0);
        blocker.release ();

        require (driver.configure ().ok ());
        require (driver.transfer (0x20, nullptr, 0, nullptr, 0, adk::Duration (0))
                     .error () == adk::StatusCode::InvalidArgument);
        require (
            driver.transfer (0x20, nullptr, 0, nullptr, 0, adk::Duration (7)).ok ());
        require (io.transferCount == 1);
        require (io.timeout == adk::Duration (7));
    }

    void provesSpiOwnershipAndMasterPolicy ()
    {
        adk::ResourceRegistry resources;
        RecordingSpiIo        io;

        {
            adk::MegaSpiDriver driver  (resources, io);
            require                    (driver.configure ().ok ());
            require                    (driver.configure ().ok ());
            require                    (io.traceSize == 1);
            require                    (io.trace[0] == SpiTrace::ConfigureMaster);

            for (adk::PinId pin = 50; pin <= 53; ++pin)
            {
                require (resources.claimed ({adk::ResourceKind::Pin, pin}));
            }
        }

        for (adk::PinId pin = 50; pin <= 53; ++pin)
        {
            require (!resources.claimed ({adk::ResourceKind::Pin, pin}));
        }
        require (io.trace[1] == SpiTrace::Disable);
    }

    void provesSpiRollbackAndChipSelectPolicy ()
    {
        adk::ResourceRegistry resources;
        adk::ResourceClaim    blocker;
        RecordingSpiIo        io;
        adk::MegaSpiDriver    driver (resources, io);

        require         (resources.claim ({adk::ResourceKind::Pin, 52}, blocker).ok ());
        require         (driver.configure ().error () == adk::StatusCode::ResourceBusy);
        require         (!resources.claimed ({adk::ResourceKind::Pin, 50}));
        require         (!resources.claimed ({adk::ResourceKind::Pin, 51}));
        blocker.release ();

        io.configureStatus = adk::StatusCode::HardwareFailure;
        require (driver.configure ().error () == adk::StatusCode::HardwareFailure);
        require (io.trace[0] == SpiTrace::ConfigureMaster);
        require (io.trace[1] == SpiTrace::Disable);

        io.configureStatus = adk::StatusCode::Ok;
        require (driver.configure ().ok ());
        require (driver.configureChipSelect (53).error () ==
                 adk::StatusCode::InvalidArgument);
        require                  (driver.configureChipSelect (49).ok ());
        driver.disable           ();
        require                  (io.chipSelectConfigured);
        require                  (driver.configure ().ok ());
        driver.releaseChipSelect (49);
        require                  (io.chipSelect == 49);
        require                  (!io.chipSelectConfigured);
    }

    void provesSpiRestorationTraces ()
    {
        adk::ResourceRegistry  resources;
        RecordingSpiIo         io;
        adk::MegaSpiDriver     driver (resources, io);
        const adk::SpiSettings settings = {1000000, 0, true};
        uint8_t                write    = 0x5a;
        uint8_t                read     = 0;

        require (driver.configure ().ok ());
        io.traceSize = 0;
        const adk::SpiSettings tooFast = {8000001, 0, true};
        const adk::SpiSettings rounded = {3000000, 0, true};
        require (
            driver.transfer (tooFast, 49, &write, &read, 1, adk::Duration (4)).error ()
            == adk::StatusCode::Unsupported);
        require (
            driver.transfer (rounded, 49, &write, &read, 1, adk::Duration (4)).error ()
            == adk::StatusCode::Unsupported);
        require (io.traceSize == 0);

        require (
            driver.transfer (settings, 49, &write, &read, 1, adk::Duration (4)).ok ());
        require (io.traceSize == 3);
        require (io.trace[0] == SpiTrace::Begin);
        require (io.trace[1] == SpiTrace::Transfer);
        require (io.trace[2] == SpiTrace::End);
        require (io.timeout == adk::Duration (4));

        io.traceSize      = 0;
        io.transferStatus = adk::StatusCode::HardwareFailure;
        require (driver.transfer (settings, 49, &write, &read, 1, adk::Duration (4))
                     .error () == adk::StatusCode::HardwareFailure);
        require (io.traceSize == 3);
        require (io.trace[2] == SpiTrace::End);

        io.traceSize      = 0;
        io.beginStatus    = adk::StatusCode::HardwareFailure;
        io.transferStatus = adk::StatusCode::Ok;
        require (driver.transfer (settings, 49, &write, &read, 1, adk::Duration (4))
                     .error () == adk::StatusCode::HardwareFailure);
        require (io.traceSize == 2);
        require (io.trace[0] == SpiTrace::Begin);
        require (io.trace[1] == SpiTrace::End);

        io.traceSize   = 0;
        io.beginStatus = adk::StatusCode::Ok;
        io.endStatus   = adk::StatusCode::HardwareFailure;
        require (
            driver.transfer (settings, 49, &write, &read, 1, adk::Duration (4)).error ()
            == adk::StatusCode::HardwareFailure);
        require (!driver.configured ());
        require (io.trace[2] == SpiTrace::End);
        require (io.trace[3] == SpiTrace::Disable);
        require (!resources.claimed ({adk::ResourceKind::Pin, 50}));
        require (
            driver.transfer (settings, 49, &write, &read, 1, adk::Duration (4)).error ()
            == adk::StatusCode::NotInitialized);
    }

    void provesTraits ()
    {
        static_assert (!std::is_copy_constructible<adk::MegaI2cDriver>::value,
                       "I2C adapter owns pin claims");
        static_assert (!std::is_move_constructible<adk::MegaI2cDriver>::value,
                       "I2C adapter address remains stable");
        static_assert (!std::is_copy_constructible<adk::MegaSpiDriver>::value,
                       "SPI adapter owns pin claims");
        static_assert (!std::is_move_constructible<adk::MegaSpiDriver>::value,
                       "SPI adapter address remains stable");
    }
} // namespace

int main ()
{
    provesI2cOwnershipAndRollback ();

    provesI2cConflictAndTimeout ();

    provesSpiOwnershipAndMasterPolicy ();

    provesSpiRollbackAndChipSelectPolicy ();

    provesSpiRestorationTraces ();

    provesTraits ();
}
