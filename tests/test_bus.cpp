#include <i2c_bus.h>
#include <spi_bus.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct RecordingI2cDriver final : adk::I2cDriver
    {
        adk::Status configureStatus = adk::StatusCode::Ok;
        adk::Status transferStatus  = adk::StatusCode::Ok;
        uint8_t     configureCount  = 0;
        uint8_t     disableCount    = 0;
        uint8_t     transferCount   = 0;
        uint8_t     address         = 0;
        uint8_t     writeSize       = 0;
        uint8_t     readSize        = 0;
        uint8_t     failReadOffset  = 0xff;
        uint8_t     firstWrite      = 0;
        uint8_t     lastWrite       = 0;
        adk::Duration timeout;

        adk::Status configure () noexcept override
        {
            ++configureCount;
            return configureStatus;
        }

        void disable () noexcept override
        {
            ++disableCount;
        }

        adk::Status transfer (uint8_t, const uint8_t*, uint8_t, uint8_t*,
                              uint8_t, adk::Duration) noexcept override;
    };

    adk::Status RecordingI2cDriver::transfer (
        uint8_t        transferAddress,
        const uint8_t* writeData,
        uint8_t        transferWriteSize,
        uint8_t*       readData,
        uint8_t        transferReadSize,
        adk::Duration  transferTimeout) noexcept
    {
        ++transferCount;
        address   = transferAddress;
        writeSize = transferWriteSize;
        readSize  = transferReadSize;
        timeout   = transferTimeout;
        firstWrite = transferWriteSize == 0 ? 0 : writeData[0];
        lastWrite  = transferWriteSize == 0
            ? 0
            : writeData[transferWriteSize - 1U];

        for (uint8_t index = 0; index < transferReadSize; ++index)
        {
            if (index == failReadOffset)
            {
                return adk::StatusCode::HardwareFailure;
            }

            readData[index] = static_cast<uint8_t> (0xa0U + index);
        }

        return transferStatus;
    }

    struct RecordingSpiDriver final : adk::SpiDriver
    {
        adk::Status      configureStatus = adk::StatusCode::Ok;
        adk::Status      selectStatus    = adk::StatusCode::Ok;
        adk::Status      transferStatus  = adk::StatusCode::Ok;
        uint8_t          configureCount  = 0;
        uint8_t          disableCount    = 0;
        uint8_t          selectCount     = 0;
        uint8_t          releaseCount    = 0;
        uint8_t          transferCount   = 0;
        uint16_t         failReadOffset  = 0xffff;
        adk::PinId       chipSelect      = 0;
        adk::SpiSettings settings        = {};
        adk::Duration    timeout;
        bool             inactiveBefore = false;
        bool             inactiveAfter  = false;
        uint8_t          firstWrite      = 0;
        uint8_t          lastWrite       = 0;

        adk::Status configure () noexcept override
        {
            ++configureCount;
            return configureStatus;
        }

        void disable () noexcept override
        {
            ++disableCount;
        }

        adk::Status configureChipSelect (adk::PinId pin) noexcept override
        {
            ++selectCount;
            chipSelect     = pin;
            inactiveBefore = true;
            return selectStatus;
        }

        void releaseChipSelect (adk::PinId pin) noexcept override
        {
            ++releaseCount;
            chipSelect    = pin;
            inactiveAfter = true;
        }

        adk::Status transfer (const adk::SpiSettings&, adk::PinId,
                              const uint8_t*, uint8_t*, uint16_t,
                              adk::Duration) noexcept override;
    };

    adk::Status RecordingSpiDriver::transfer (
        const adk::SpiSettings& transferSettings,
        adk::PinId              transferChipSelect,
        const uint8_t*          writeData,
        uint8_t*                readData,
        uint16_t                size,
        adk::Duration           transferTimeout) noexcept
    {
        ++transferCount;
        chipSelect      = transferChipSelect;
        settings        = transferSettings;
        timeout         = transferTimeout;
        inactiveBefore  = true;
        inactiveAfter   = false;
        firstWrite      = writeData ? writeData[0] : 0;
        lastWrite       = writeData ? writeData[size - 1U] : 0;

        for (uint16_t index = 0; index < size; ++index)
        {
            if (index == failReadOffset)
            {
                inactiveAfter = true;
                return adk::StatusCode::HardwareFailure;
            }

            if (readData)
            {
                readData[index] = static_cast<uint8_t> (index);
            }
        }

        inactiveAfter = true;
        return transferStatus;
    }

    void provesI2cLifecycleAndAddressOwnership ()
    {
        adk::ResourceRegistry resources;
        RecordingI2cDriver    driver;
        adk::I2cBus            bus          (resources, driver);
        adk::I2cDevice         first        (bus, 0x20);
        adk::I2cDevice         duplicate    (bus, 0x20);
        adk::I2cDevice         reservedLow  (bus, 0x07);
        adk::I2cDevice         reservedHigh (bus, 0x78);

        require (first.initialize ().error () == adk::StatusCode::NotInitialized,
                 "I2C device requires active bus");
        require (bus.initialize ().ok (), "I2C bus initializes");
        require (bus.initialize ().ok (), "I2C bus initialization repeats");
        require (first.initialize ().ok (), "I2C address attaches");
        require (first.initialize ().ok (), "I2C device initialization repeats");
        require (duplicate.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "I2C duplicate address is rejected");
        require (reservedLow.initialize ().error () ==
                     adk::StatusCode::InvalidArgument,
                 "I2C low reserved address is rejected");
        require (reservedHigh.initialize ().error () ==
                     adk::StatusCode::InvalidArgument,
                 "I2C high reserved address is rejected");

        bus.shutdown     ();
        require          (first.initialized (), "I2C address remains attached");
        require          (bus.initialize ().ok (), "I2C bus reinitializes");
        first.shutdown   ();
        require          (duplicate.initialize ().ok (), "I2C address becomes reusable");
    }

    void provesBusRollbackAndDestruction ()
    {
        adk::ResourceRegistry resources;
        RecordingI2cDriver    i2cDriver;
        RecordingSpiDriver    spiDriver;

        i2cDriver.configureStatus = adk::StatusCode::HardwareFailure;

        {
            adk::I2cBus failed (resources, i2cDriver);

            require (failed.initialize ().error () ==
                         adk::StatusCode::HardwareFailure,
                     "I2C reports configuration failure");
            require (!resources.claimed ({adk::ResourceKind::I2cBus, 0}),
                     "I2C rolls back configuration claim");
        }

        i2cDriver.configureStatus = adk::StatusCode::Ok;

        {
            adk::I2cBus active (resources, i2cDriver);

            require (active.initialize ().ok (), "I2C initializes for destruction");
        }

        require (i2cDriver.disableCount == 2,
                 "I2C rollback and destruction disable driver");

        {
            adk::SpiBus active (resources, spiDriver);

            require (active.initialize ().ok (), "SPI initializes for destruction");
        }

        require (spiDriver.disableCount == 1, "SPI destruction disables driver");
        require (!resources.claimed ({adk::ResourceKind::SpiBus, 0}),
                 "SPI destruction releases bus");
    }

    void provesI2cCapacityAndDestruction ()
    {
        adk::ResourceRegistry resources;
        RecordingI2cDriver    driver;
        adk::I2cBus            bus (resources, driver);

        require (bus.initialize ().ok (), "I2C initializes for capacity");

        adk::I2cDevice devices[] = {
            {bus, 0x08}, {bus, 0x11}, {bus, 0x12}, {bus, 0x13},
            {bus, 0x14}, {bus, 0x15}, {bus, 0x16}, {bus, 0x77}
        };

        for (adk::I2cDevice& device : devices)
        {
            require (device.initialize ().ok (), "I2C slot attaches");
        }

        adk::I2cDevice overflow (bus, 0x18);

        require (overflow.initialize ().error () == adk::StatusCode::CapacityExceeded,
                 "I2C capacity is bounded");

        devices[3].shutdown ();

        require (overflow.initialize ().ok (), "I2C released slot is reusable");
    }

    void provesI2cTransfersAndPartialFailure ()
    {
        adk::ResourceRegistry resources;
        RecordingI2cDriver    driver;
        adk::I2cBus            bus    (resources, driver);
        adk::I2cDevice         device (bus, 0x48);
        uint8_t                write[adk::I2cBus::maximumTransferSize] = {};
        uint8_t                read [adk::I2cBus::maximumTransferSize] = {};
        uint8_t                oversized[adk::I2cBus::maximumTransferSize + 1] = {};
        const adk::Duration    timeout (25);

        require (bus.initialize ().ok (), "I2C initializes for transfer");
        require (device.initialize ().ok (), "I2C device initializes");
        write[0]                       = 0x12;
        write[sizeof (write) - 1U]     = 0xef;
        require (device.transfer (write, sizeof (write), read, sizeof (read),
                                  timeout).ok (),
                 "I2C bounded transfer succeeds");
        require (driver.address == 0x48, "I2C forwards owned address");
        require (driver.timeout == timeout, "I2C forwards timeout");
        require (driver.firstWrite == 0x12 && driver.lastWrite == 0xef,
                 "I2C forwards exact bytes");
        require (device.transfer (oversized, sizeof (oversized), nullptr, 0,
                                  timeout).error () ==
                     adk::StatusCode::InvalidArgument,
                 "I2C rejects maximum plus one");

        require (device.transfer (nullptr, 0, nullptr, 0, timeout).error () ==
                     adk::StatusCode::InvalidArgument,
                 "I2C rejects empty transfer");
        require (device.transfer (write, 1, nullptr, 0, adk::Duration ()).error () ==
                     adk::StatusCode::InvalidArgument,
                 "I2C rejects zero timeout");

        for (uint8_t& value : read)
        {
            value = 0;
        }
        driver.failReadOffset = 3;
        require (device.transfer (nullptr, 0, read, 5, timeout).error () ==
                     adk::StatusCode::HardwareFailure,
                 "I2C preserves partial failure");
        require (read[2] == 0xa2 && read[3] == 0, "I2C exposes exact failure offset");

        bus.shutdown ();

        require (device.transfer (write, 1, nullptr, 0, timeout).error () ==
                     adk::StatusCode::NotInitialized,
                 "I2C inactive bus rejects attached device");
    }

    void provesSpiOwnershipSettingsAndRollback ()
    {
        adk::ResourceRegistry resources;
        RecordingSpiDriver    driver;
        adk::SpiBus            bus       (resources, driver);
        const adk::SpiSettings settings  = {8000000, 3, true};
        adk::SpiDevice         device    (bus, 49, settings);
        adk::SpiDevice         duplicate (bus, 49, settings);

        require (device.initialize ().error () == adk::StatusCode::NotInitialized,
                 "SPI device cannot claim before bus owns fixed pins");
        require (bus.initialize ().ok (), "SPI bus initializes");
        require (device.initialize ().ok (), "SPI device initializes inactive");
        require (resources.claimed ({adk::ResourceKind::Pin, 49}),
                 "SPI device persistently owns chip select");
        require (duplicate.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "SPI duplicate chip select is rejected");
        require (device.settings ().clockHz == settings.clockHz,
                 "SPI settings remain immutable");

        bus.shutdown     ();
        require          (device.initialized (), "SPI ownership survives bus shutdown");
        require          (bus.initialize ().ok (), "SPI bus reinitializes");
        device.shutdown  ();
        require          (!resources.claimed ({adk::ResourceKind::Pin, 49}),
                          "SPI shutdown releases chip select");
        require          (driver.inactiveAfter,
                          "SPI shutdown restores inactive chip select");
    }

    void provesSpiTransferTraceAndFailure ()
    {
        adk::ResourceRegistry resources;
        RecordingSpiDriver    driver;
        adk::SpiBus            bus (resources, driver);
        const adk::SpiSettings settings = {4000000, 1, false};
        adk::SpiDevice         device (bus, 48, settings);
        uint8_t                write[adk::SpiBus::maximumTransferSize] = {};
        uint8_t                read [adk::SpiBus::maximumTransferSize] = {};
        uint8_t                oversized[adk::SpiBus::maximumTransferSize + 1] = {};
        const adk::Duration    timeout (12);

        require (bus.initialize ().ok (), "SPI initializes for transfer");
        require (device.initialize ().ok (), "SPI device initializes for transfer");
        write[0]                   = 0x34;
        write[sizeof (write) - 1U] = 0xcd;
        require (device.transfer (write, read, sizeof (write), timeout).ok (),
                 "SPI bounded transfer succeeds");
        require (driver.inactiveBefore && driver.inactiveAfter,
                 "SPI trace brackets transfer with inactive chip select");
        require (driver.settings.mode == 1, "SPI applies device settings");
        require (driver.timeout == timeout, "SPI forwards timeout");
        require (driver.firstWrite == 0x34 && driver.lastWrite == 0xcd,
                 "SPI forwards exact bytes");
        require (device.transfer (oversized, nullptr, sizeof (oversized),
                                  timeout).error () ==
                     adk::StatusCode::InvalidArgument,
                 "SPI rejects maximum plus one");

        for (uint8_t& value : read)
        {
            value = 0;
        }
        driver.failReadOffset = 4;
        require (device.transfer (write, read, 6, timeout).error () ==
                     adk::StatusCode::HardwareFailure,
                 "SPI preserves partial failure");
        require (read[3] == 3 && read[4] == 0, "SPI exposes exact failure offset");
        require (driver.inactiveAfter, "SPI failure restores inactive chip select");

        require (device.transfer (write, nullptr, 0, timeout).error () ==
                     adk::StatusCode::InvalidArgument,
                 "SPI rejects empty transfer");
        require (device.transfer (write, nullptr, 1, adk::Duration ()).error () ==
                     adk::StatusCode::InvalidArgument,
                 "SPI rejects zero timeout");
    }

    void provesSpiInitializationFailures ()
    {
        adk::ResourceRegistry resources;
        RecordingSpiDriver    driver;
        adk::SpiBus            bus (resources, driver);

        driver.configureStatus = adk::StatusCode::HardwareFailure;
        require (bus.initialize ().error () == adk::StatusCode::HardwareFailure,
                 "SPI bus reports driver configuration failure");
        require (!resources.claimed ({adk::ResourceKind::SpiBus, 0}),
                 "SPI bus rolls back failed configuration");

        driver.configureStatus = adk::StatusCode::Ok;
        require (bus.initialize ().ok (), "SPI bus recovers after failure");

        const adk::SpiSettings invalid = {0, 4, true};
        adk::SpiDevice         device  (bus, 47, invalid);

        require (device.initialize ().error () == adk::StatusCode::InvalidArgument,
                 "SPI rejects invalid immutable settings");

        const adk::SpiSettings valid = {1000000, 0, true};
        adk::SpiDevice         failed (bus, 46, valid);
        driver.selectStatus = adk::StatusCode::HardwareFailure;
        require (failed.initialize ().error () == adk::StatusCode::HardwareFailure,
                 "SPI reports chip-select configuration failure");
        require (!resources.claimed ({adk::ResourceKind::Pin, 46}),
                 "SPI rolls back failed chip-select claim");
        require (driver.inactiveAfter, "SPI rollback restores inactive chip select");
    }

    void provesOwnershipTraits ()
    {
        static_assert (!std::is_copy_constructible<adk::I2cBus>::value,
                       "I2C bus is not copyable");
        static_assert (!std::is_move_constructible<adk::I2cDevice>::value,
                       "I2C device is not movable");
        static_assert (!std::is_copy_constructible<adk::SpiBus>::value,
                       "SPI bus is not copyable");
        static_assert (!std::is_move_constructible<adk::SpiDevice>::value,
                       "SPI device is not movable");
    }
}

int main ()
{
    provesI2cLifecycleAndAddressOwnership ();
    provesBusRollbackAndDestruction       ();
    provesI2cCapacityAndDestruction       ();
    provesI2cTransfersAndPartialFailure   ();
    provesSpiOwnershipSettingsAndRollback ();
    provesSpiTransferTraceAndFailure      ();
    provesSpiInitializationFailures       ();
    provesOwnershipTraits                 ();
    return EXIT_SUCCESS;
}
