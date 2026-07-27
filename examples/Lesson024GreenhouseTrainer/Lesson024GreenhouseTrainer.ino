// Mega 2560, USB only: A0 reads a 10k potentiometer; D30-D35 drive a
// 16x2 LCD; D38, D40, D41, D13, and D5-D7 drive resistor-limited LEDs.
// No relay, pump, heater, water, external supply, or SD card is connected.
#include <Adk.h>
#include <fixed_storage.h>
#include <greenhouse_controller.h>
#include <greenhouse_health_pattern.h>
#include <inert_load_panel.h>
#include <record_sink.h>
#include <rtc.h>
#include <watering_controller.h>

namespace {

    constexpr adk::PinId moisturePin       = 54;
    constexpr adk::PinId fanEvidencePin    = 40;
    constexpr adk::PinId pumpEvidencePin   = 38;
    constexpr adk::PinId heaterEvidencePin = 41;
    constexpr adk::PinId recordEvidencePin = LED_BUILTIN;

    const adk::Hd44780Pins displayPins = {30, 31, 32, 33, 34, 35};

    const adk::MoistureCalibration moistureCalibration = {800, 200, 16};
    const adk::WateringConfig      wateringConfig   = {350, 600, adk::Duration (5000),
                                                       adk::Duration (2000)};
    const adk::GreenhouseConfig    greenhouseConfig = {
        adk::Duration (250), adk::Duration (500), adk::Duration (1000),
        adk::Duration (750)};

    struct SimulatedRtc final : adk::Rtc
    {
        SimulatedRtc () noexcept : initialized_ (false), sequence_ (0)
        {
        }

        ~SimulatedRtc () noexcept override
        {
            shutdown ();
        }

        adk::Status initialize () noexcept override
        {
            initialized_ = true;
            sequence_    = 0;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            initialized_ = false;
        }

        adk::Result<adk::ClockReading> read () noexcept override
        {
            if (!initialized_)
            {
                return adk::Result<adk::ClockReading> (
                    adk::StatusCode::NotInitialized,
                    {0, adk::ClockState::TransportFault});
            }

            const adk::ClockReading reading = {
                static_cast<uint32_t> (1700000000UL + sequence_++),
                adk::ClockState::Valid};
            return adk::Result<adk::ClockReading> (adk::StatusCode::Ok, reading);
        }

        bool     initialized_;
        uint32_t sequence_;
    };

    struct EvidenceRecordSink final : adk::RecordSink
    {
        EvidenceRecordSink (adk::Rtc& rtc, adk::Storage& storage,
                            adk::MonoLed& acknowledgement) noexcept
            : acknowledgement_ (&acknowledgement), rtc_ (&rtc), storage_ (&storage),
              lastClock_          ({0, adk::ClockState::NotSet}),
              initialized_        (false),
              acknowledgementOn_  (false)
        {
        }

        ~EvidenceRecordSink () noexcept override
        {
            shutdown ();
        }

        adk::Status initialize () noexcept override
        {
            if (initialized_)
            {
                return adk::StatusCode::Ok;
            }

            adk::Status status = rtc_->initialize ();

            if (!status.ok ())
            {
                return status;
            }

            status = storage_->initialize ();

            if (!status.ok ())
            {
                rtc_->shutdown ();
                return status;
            }

            status = acknowledgement_->initialize ();

            if (!status.ok ())
            {
                storage_->shutdown ();
                rtc_->shutdown     ();
                return status;
            }

            initialized_       = true;
            acknowledgementOn_ = false;
            return acknowledgement_->off ();
        }

        void shutdown () noexcept override
        {
            if (!initialized_)
            {
                return;
            }

            acknowledgement_->off       ();
            acknowledgement_->shutdown  ();
            storage_->shutdown          ();
            rtc_->shutdown              ();
            initialized_ = false;
        }

        adk::Status append (const adk::StableRecord& record) noexcept override
        {
            if (!initialized_)
            {
                return adk::StatusCode::NotInitialized;
            }

            const adk::Result<adk::ClockReading> clock = rtc_->read ();

            if (!clock.ok () || clock.value ().state != adk::ClockState::Valid)
            {
                return clock.ok () ? adk::StatusCode::HardwareFailure : clock.status ();
            }

            lastClock_ = clock.value ();

            adk::Status status = storage_->append (
                reinterpret_cast<const uint8_t*> (record.text), record.length);

            if (status.ok ())
            {
                status = storage_->sync ();
            }

            if (!status.ok ())
            {
                return status;
            }

            acknowledgementOn_ = !acknowledgementOn_;
            return acknowledgementOn_ ? acknowledgement_->on ()
                                      : acknowledgement_->off ();
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

        adk::MonoLed*     acknowledgement_;
        adk::Rtc*         rtc_;
        adk::Storage*     storage_;
        adk::ClockReading lastClock_;
        bool              initialized_;
        bool              acknowledgementOn_;
    };

    adk::Runtime runtime;

    adk::AnalogInput    moistureInput (runtime.resources (), moisturePin);
    adk::MoistureSensor moisture      (moistureInput, moistureCalibration);

    adk::IndicatorPump      fanIndicator    (runtime.resources (), fanEvidencePin);
    adk::IndicatorPump      pumpIndicator   (runtime.resources (), pumpEvidencePin);
    adk::IndicatorPump      heaterIndicator (runtime.resources (), heaterEvidencePin);
    adk::InertLoadPanel     loadPanel       (fanIndicator, pumpIndicator,
                                             heaterIndicator);
    adk::PanelPumpOutput    pumpOutput      (loadPanel);
    adk::WateringController watering        (wateringConfig, pumpOutput);

    adk::Hd44780Display     display        (runtime.resources (), displayPins);
    adk::MonoLed            recordEvidence (runtime.resources (), recordEvidencePin);
    SimulatedRtc            recordClock;
    adk::FixedStorageMedium recordMedium  (512);
    adk::FixedStorage       recordStorage (recordMedium);
    EvidenceRecordSink      records       (recordClock, recordStorage, recordEvidence);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusEvidence (runtime.resources (), redChannel, greenChannel,
                                blueChannel);
    adk::GreenhouseHealthPattern healthPattern (statusEvidence);

    adk::GreenhouseController greenhouse (greenhouseConfig, moisture, watering, display,
                                          records);

    bool running = false;

    bool acquireGreenhouse    ();
    bool observeGreenhouse    (adk::TimePoint now);
    bool decideGreenhouse     (adk::TimePoint now);
    bool actuateGreenhouse    (adk::TimePoint now);
    bool presentGreenhouse    (adk::TimePoint now);
    bool recordGreenhouse     (adk::TimePoint now);
    bool showGreenhouseHealth (adk::TimePoint now);

    bool wateringAllowed (adk::TimePoint now);
    void stopSafely      ();

} // namespace

void setup ()
{
    running = acquireGreenhouse ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (!observeGreenhouse   (now) || !decideGreenhouse    (now) ||
        !actuateGreenhouse   (now) || !presentGreenhouse   (now) ||
        !recordGreenhouse    (now) || !showGreenhouseHealth (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireGreenhouse ()
    {
        const adk::Status healthStatus = healthPattern.initialize ();

        if (!healthStatus.ok ())
        {
            return false;
        }

        const adk::Status greenhouseStatus = greenhouse.initialize ();

        if (!greenhouseStatus.ok ())
        {
            healthPattern.shutdown ();
            return false;
        }

        const adk::Status startingStatus =
            healthPattern.update (adk::TimePoint (millis ()),
                                  adk::GreenhouseMode::Starting);

        if (!startingStatus.ok ())
        {
            greenhouse.shutdown     ();
            healthPattern.shutdown  ();
            return false;
        }

        return true;
    }

    bool observeGreenhouse (adk::TimePoint now)
    {
        const adk::Status status = greenhouse.observe (now);

        return status.ok ();
    }

    bool decideGreenhouse (adk::TimePoint now)
    {
        const adk::GreenhouseInput input  = {wateringAllowed   (now)};
        const adk::Status          status = greenhouse.decide  (now, input);

        return status.ok ();
    }

    bool actuateGreenhouse (adk::TimePoint now)
    {
        const adk::Status status = greenhouse.actuate (now);

        return status.ok ();
    }

    bool presentGreenhouse (adk::TimePoint now)
    {
        const adk::Status status = greenhouse.present (now);

        return status.ok ();
    }

    bool recordGreenhouse (adk::TimePoint now)
    {
        const adk::Status status = greenhouse.record (now);

        return status.ok ();
    }

    bool showGreenhouseHealth (adk::TimePoint now)
    {
        const adk::GreenhouseMode mode = greenhouse.snapshot ().mode;

        return healthPattern.update (now, mode).ok ();
    }

    bool wateringAllowed (adk::TimePoint now)
    {
        return (now.milliseconds () / 8000U) % 2U == 0U;
    }

    void stopSafely ()
    {
        greenhouse.shutdown     ();
        healthPattern.shutdown  ();
        running = false;
    }
} // namespace
