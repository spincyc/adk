// Mega 2560, USB only: D12 drives a fault LED through 1 kOhm.
// D13 acknowledges deterministic fake transactions; no bus waveform is generated.
#include <fixed_storage.h>
#include <i2c_bus.h>
#include <moisture_sensor.h>
#include <mono_led.h>
#include <rtc.h>
#include <runtime.h>
#include <spi_bus.h>

namespace {

    constexpr adk::PinId faultEvidencePin = 12;
    constexpr adk::PinId moistureInputPin = 54;
    constexpr adk::PinId spiChipSelectPin = 49;

    constexpr uint32_t transactionIntervalMilliseconds  = 1500;
    constexpr uint32_t minimumActivityPulseMilliseconds = 100;
    constexpr uint32_t activityPulseRangeMilliseconds   = 800;

    const adk::Duration            transactionTimeout  (20);
    const adk::Duration            sampleStaleAfter    (3000);
    const adk::SpiSettings         storageSettings     = {1000000, 0, true};
    const adk::MoistureCalibration moistureCalibration = {200, 800, 32};

    struct SimulatedRtc final : adk::Rtc
    {
        adk::Status                    initialize () noexcept override;
        void                           shutdown   () noexcept override;
        adk::Result<adk::ClockReading> read       () noexcept override;

        bool     initialized = false;
        uint32_t readCount   = 0;
    };

    struct RecordingI2cDriver : adk::I2cDriver
    {
        adk::Status configure () noexcept override;
        void        disable   () noexcept override;
        adk::Status transfer  (uint8_t address, const uint8_t* writeData,
                              uint8_t writeSize, uint8_t* readData, uint8_t readSize,
                              adk::Duration timeout) noexcept override;

        bool     configured = false;
        uint16_t transfers  = 0;
    };

    struct RecordingSpiDriver : adk::SpiDriver
    {
        adk::Status configure           () noexcept override;
        void        disable             () noexcept override;
        adk::Status configureChipSelect (adk::PinId chipSelect) noexcept override;
        void        releaseChipSelect   (adk::PinId chipSelect) noexcept override;
        adk::Status transfer            (const adk::SpiSettings& settings,
                                         adk::PinId chipSelect,
                              const uint8_t* writeData, uint8_t* readData,
                              uint16_t size, adk::Duration timeout) noexcept override;

        bool       configured           = false;
        bool       chipSelectConfigured = false;
        adk::PinId configuredChipSelect = 0;
        uint16_t   transfers            = 0;
    };

    adk::Runtime runtime;

    RecordingI2cDriver i2cDriver;
    RecordingSpiDriver spiDriver;
    SimulatedRtc       rtc;

    adk::I2cBus i2cBus (runtime.resources (), i2cDriver);
    adk::SpiBus spiBus (runtime.resources (), spiDriver);

    adk::I2cDevice metadataDevice (i2cBus, 0x50);
    adk::SpiDevice recordDevice   (spiBus, spiChipSelectPin, storageSettings);

    adk::AnalogInput    moistureInput  (runtime.resources (), moistureInputPin);
    adk::MoistureSensor moistureSensor (moistureInput, moistureCalibration);

    adk::FixedStorageMedium storageMedium (adk::FixedStorageMedium::maximumCapacity);
    adk::FixedStorage       storage       (storageMedium);

    adk::MonoLed activityEvidence (runtime.resources (), LED_BUILTIN);
    adk::MonoLed faultEvidence    (runtime.resources (), faultEvidencePin);

    adk::TimePoint lastTransaction;
    adk::TimePoint activityStarted;

    bool     running                   = false;
    bool     activityActive            = false;
    uint8_t  recordsSinceRestart       = 0;
    uint32_t activityPulseMilliseconds = minimumActivityPulseMilliseconds;

    bool acquireBusOwners     ();
    bool observeMoisture      (adk::TimePoint now, adk::MoistureSample& sample);
    bool persistMoisture      (adk::TimePoint now, const adk::MoistureSample& sample);
    bool proveStorageRestart  ();
    bool acknowledgeMoisture  (adk::TimePoint now, const adk::MoistureSample& sample);
    bool clearAcknowledgement (adk::TimePoint now);
    void observeFailure       (adk::Status status);
    void stopSafely           ();

} // namespace

void setup ()
{
    running         = acquireBusOwners ();
    lastTransaction = adk::TimePoint   (millis ());

    if (!running)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (!clearAcknowledgement (now))
    {
        stopSafely ();
        return;
    }

    if (now.elapsedSince (lastTransaction).milliseconds () <
        transactionIntervalMilliseconds)
    {
        return;
    }

    adk::MoistureSample sample;

    if (!observeMoisture (now, sample) || !persistMoisture (now, sample) ||
        !acknowledgeMoisture (now, sample))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireBusOwners ()
    {
        const adk::Status activityStatus = activityEvidence.initialize ();

        if (!activityStatus.ok ())
        {
            return false;
        }

        const adk::Status faultStatus = faultEvidence.initialize ();

        if (!faultStatus.ok ())
        {
            observeFailure (faultStatus);
            return false;
        }

        const adk::Status i2cStatus = i2cBus.initialize ();

        if (!i2cStatus.ok ())
        {
            observeFailure (i2cStatus);
            return false;
        }

        const adk::Status spiStatus = spiBus.initialize ();

        if (!spiStatus.ok ())
        {
            observeFailure (spiStatus);
            return false;
        }

        const adk::Status metadataStatus = metadataDevice.initialize ();

        if (!metadataStatus.ok ())
        {
            observeFailure (metadataStatus);
            return false;
        }

        const adk::Status recordStatus = recordDevice.initialize ();

        if (!recordStatus.ok ())
        {
            observeFailure (recordStatus);
            return false;
        }

        const adk::Status moistureStatus = moistureSensor.initialize ();

        if (!moistureStatus.ok ())
        {
            observeFailure (moistureStatus);
            return false;
        }

        const adk::Status rtcStatus = rtc.initialize ();

        if (!rtcStatus.ok ())
        {
            observeFailure (rtcStatus);
            return false;
        }

        const adk::Status storageStatus = storage.initialize ();

        if (!storageStatus.ok ())
        {
            observeFailure (storageStatus);
            return false;
        }

        return true;
    }

    bool observeMoisture (adk::TimePoint now, adk::MoistureSample& sample)
    {
        const adk::Status status = moistureSensor.update (now);

        if (!status.ok ())
        {
            observeFailure (status);
            return false;
        }

        sample = moistureSensor.sample (now, sampleStaleAfter);

        if (sample.state != adk::MoistureSampleState::Valid)
        {
            observeFailure (adk::StatusCode::InvalidArgument);
            return false;
        }

        return true;
    }

    bool persistMoisture (adk::TimePoint now, const adk::MoistureSample& sample)
    {
        const adk::Result<adk::ClockReading> clockResult = rtc.read ();

        if (!clockResult.ok () || clockResult.value ().state != adk::ClockState::Valid)
        {
            observeFailure (clockResult.ok () ? adk::StatusCode::InvalidArgument
                                              : clockResult.status ());
            return false;
        }

        const uint8_t metadataRequest[] = {0x10};
        uint8_t       metadataToken     = 0;

        const adk::Status i2cStatus = metadataDevice.transfer (
            metadataRequest, sizeof (metadataRequest), &metadataToken,
            sizeof (metadataToken), transactionTimeout);

        if (!i2cStatus.ok ())
        {
            observeFailure (i2cStatus);
            return false;
        }

        const uint32_t unixSeconds = clockResult.value ().unixSeconds;
        const uint8_t  record[] = {static_cast<uint8_t> (unixSeconds >> 24),
                                   static_cast<uint8_t> (unixSeconds >> 16),
                                   static_cast<uint8_t> (unixSeconds >> 8),
                                   static_cast<uint8_t> (unixSeconds),
                                   static_cast<uint8_t> (clockResult.value ().state),
                                   static_cast<uint8_t> (sample.rawReading >> 8),
                                   static_cast<uint8_t> (sample.rawReading),
                                   static_cast<uint8_t> (sample.moisturePermille >> 8),
                                   static_cast<uint8_t> (sample.moisturePermille),
                                   static_cast<uint8_t> (sample.state),
                                   metadataToken};

        const adk::Status spiStatus = recordDevice.transfer (
            record, nullptr, sizeof (record), transactionTimeout);

        if (!spiStatus.ok ())
        {
            observeFailure (spiStatus);
            return false;
        }

        const adk::Status appendStatus = storage.append (record, sizeof (record));

        if (!appendStatus.ok ())
        {
            observeFailure (appendStatus);
            return false;
        }

        const adk::Status syncStatus = storage.sync ();

        if (!syncStatus.ok ())
        {
            observeFailure (syncStatus);
            return false;
        }

        ++recordsSinceRestart;

        if (recordsSinceRestart == 8 && !proveStorageRestart ())
        {
            return false;
        }

        lastTransaction = now;
        return true;
    }

    bool proveStorageRestart ()
    {
        const uint16_t durableSize = storageMedium.durableSize ();

        storage.shutdown ();

        const adk::Status status = storage.initialize ();

        if (!status.ok () || storage.stagedSize () != durableSize)
        {
            observeFailure (status.ok () ? adk::StatusCode::HardwareFailure : status);
            return false;
        }

        recordsSinceRestart = 0;
        return true;
    }

    bool acknowledgeMoisture (adk::TimePoint now, const adk::MoistureSample& sample)
    {
        const adk::Status status = activityEvidence.on ();

        if (!status.ok ())
        {
            observeFailure (status);
            return false;
        }

        activityPulseMilliseconds = minimumActivityPulseMilliseconds +
                                    static_cast<uint32_t> (sample.moisturePermille) *
                                        activityPulseRangeMilliseconds / 1000U;
        activityStarted           = now;
        activityActive            = true;
        return true;
    }

    bool clearAcknowledgement (adk::TimePoint now)
    {
        if (!activityActive || now.elapsedSince (activityStarted).milliseconds () <
                                   activityPulseMilliseconds)
        {
            return true;
        }

        const adk::Status status = activityEvidence.off ();

        if (!status.ok ())
        {
            observeFailure (status);
            return false;
        }

        activityActive = false;
        return true;
    }

    void observeFailure (adk::Status status)
    {
        (void)status;
        faultEvidence.on ();
    }

    void stopSafely ()
    {
        storage       .shutdown ();
        rtc           .shutdown ();
        moistureSensor.shutdown ();
        recordDevice  .shutdown ();
        metadataDevice.shutdown ();
        spiBus        .shutdown ();
        i2cBus        .shutdown ();

        activityEvidence.off      ();
        activityEvidence.shutdown ();

        if (!faultEvidence.on ().ok ())
        {
            faultEvidence.shutdown ();
        }

        running = false;
    }

    adk::Status SimulatedRtc::initialize () noexcept
    {
        initialized = true;
        return adk::StatusCode::Ok;
    }

    void SimulatedRtc::shutdown () noexcept
    {
        initialized = false;
    }

    adk::Result<adk::ClockReading> SimulatedRtc::read () noexcept
    {
        if (!initialized)
        {
            return adk::Result<adk::ClockReading> (
                adk::StatusCode::NotInitialized, {0, adk::ClockState::TransportFault});
        }

        const adk::ClockReading reading = {
            static_cast<uint32_t> (1700000000UL + readCount), adk::ClockState::Valid};

        ++readCount;
        return adk::Result<adk::ClockReading> (adk::StatusCode::Ok, reading);
    }

    adk::Status RecordingI2cDriver::configure () noexcept
    {
        configured = true;
        return adk::StatusCode::Ok;
    }

    void RecordingI2cDriver::disable () noexcept
    {
        configured = false;
    }

    adk::Status RecordingI2cDriver::transfer (uint8_t address, const uint8_t* writeData,
                                              uint8_t writeSize, uint8_t* readData,
                                              uint8_t       readSize,
                                              adk::Duration timeout) noexcept
    {
        if (!configured)
        {
            return adk::StatusCode::NotInitialized;
        }

        ++transfers;

        if (timeout.milliseconds () == 0)
        {
            return adk::StatusCode::InvalidArgument;
        }

        if (readSize != 0)
        {
            readData[0] = static_cast<uint8_t> (address + writeData[0] + writeSize);
        }

        return adk::StatusCode::Ok;
    }

    adk::Status RecordingSpiDriver::configure () noexcept
    {
        configured = true;
        return adk::StatusCode::Ok;
    }

    void RecordingSpiDriver::disable () noexcept
    {
        configured = false;
    }

    adk::Status RecordingSpiDriver::configureChipSelect (adk::PinId chipSelect) noexcept
    {
        configuredChipSelect = chipSelect;
        chipSelectConfigured = true;
        return adk::StatusCode::Ok;
    }

    void RecordingSpiDriver::releaseChipSelect (adk::PinId chipSelect) noexcept
    {
        if (chipSelect == configuredChipSelect)
        {
            chipSelectConfigured = false;
        }
    }

    adk::Status RecordingSpiDriver::transfer (const adk::SpiSettings& settings,
                                              adk::PinId              chipSelect,
                                              const uint8_t*          writeData,
                                              uint8_t* readData, uint16_t size,
                                              adk::Duration timeout) noexcept
    {
        if (!configured || !chipSelectConfigured || chipSelect != configuredChipSelect)
        {
            return adk::StatusCode::NotInitialized;
        }

        ++transfers;

        if (timeout.milliseconds () == 0)
        {
            return adk::StatusCode::InvalidArgument;
        }

        for (uint16_t index = 0; index < size; ++index)
        {
            if (readData != nullptr)
            {
                const uint8_t written = writeData == nullptr ? 0 : writeData[index];
                readData[index] =
                    static_cast<uint8_t> (written ^ settings.mode ^ chipSelect);
            }
        }

        return adk::StatusCode::Ok;
    }

} // namespace
