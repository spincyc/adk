#include "greenhouse_controller.h"

#include <stddef.h>

namespace adk {

    namespace {

        constexpr uint32_t maximumForwardElapsed = 0x7FFFFFFFUL;
        constexpr uint8_t  maximumRecordLength   = 77;

        static_assert (StableRecord::capacity >= maximumRecordLength,
                       "StableRecord cannot hold a greenhouse record");

        const char* sampleToken (MoistureSampleState state) noexcept
        {
            switch (state)
            {
                case MoistureSampleState::Unavailable: return "missing";
                case MoistureSampleState::Valid: return "valid";
                case MoistureSampleState::InputBelowRange: return "below";
                case MoistureSampleState::InputAboveRange: return "above";
                case MoistureSampleState::Stale: return "stale";
            }

            return "missing";
        }

        const char* waterToken (PumpState state) noexcept
        {
            return state == PumpState::On ? "on" : "off";
        }

        const char* reasonToken (WateringReason reason) noexcept
        {
            switch (reason)
            {
                case WateringReason::None: return "none";
                case WateringReason::DryThreshold: return "dry";
                case WateringReason::WetThreshold: return "wet";
                case WateringReason::MaximumOnTime: return "max-time";
                case WateringReason::MinimumOffTime: return "min-time";
                case WateringReason::OperatorInhibit: return "inhibit";
                case WateringReason::InvalidSample: return "invalid";
                case WateringReason::OutputFailure: return "output";
                case WateringReason::Shutdown: return "shutdown";
            }

            return "invalid";
        }

        const char* modeToken (GreenhouseMode mode) noexcept
        {
            switch (mode)
            {
                case GreenhouseMode::Starting: return "starting";
                case GreenhouseMode::Monitoring: return "monitor";
                case GreenhouseMode::Watering: return "watering";
                case GreenhouseMode::Inhibited: return "inhibit";
                case GreenhouseMode::SensorFault: return "sensor";
                case GreenhouseMode::OutputFault: return "output";
                case GreenhouseMode::DisplayFault: return "display";
                case GreenhouseMode::RecordFault: return "record";
                case GreenhouseMode::MultipleFaults: return "multiple";
            }

            return "multiple";
        }

        bool appendText (StableRecord& record, const char* text) noexcept
        {
            while (*text != '\0')
            {
                if (record.length >= StableRecord::capacity)
                {
                    return false;
                }

                record.text[record.length++] = *text++;
            }

            return true;
        }

        bool appendUnsigned (StableRecord& record, uint32_t value) noexcept
        {
            char    digits[10];
            uint8_t count = 0;

            do
            {
                digits[count++] = static_cast<char> ('0' + value % 10U);
                value /= 10U;
            }
            while (value != 0U);

            while (count != 0U)
            {
                if (record.length >= StableRecord::capacity)
                {
                    return false;
                }

                record.text[record.length++] = digits[--count];
            }

            return true;
        }

        void fillLine (char* output, const char* text) noexcept
        {
            uint8_t column = 0;

            while (column < characterDisplayColumns && text[column] != '\0')
            {
                output[column] = text[column];
                ++column;
            }

            while (column < characterDisplayColumns)
            {
                output[column++] = ' ';
            }

            output[column] = '\0';
        }
    }

    GreenhouseController::GreenhouseController (const GreenhouseConfig& config,
                                                MoistureSensor& moisture,
                                                WateringController& watering,
                                                CharacterDisplay& display,
                                                RecordSink& records) noexcept
        : config_             (config)
        , moisture_           (&moisture)
        , watering_           (&watering)
        , display_            (&display)
        , records_            (&records)
        , snapshot_           ()
        , decided_            ()
        , pendingRecord_      ()
        , sampleEpoch_        ()
        , displayEpoch_       ()
        , recordEpoch_        ()
        , decisionAt_         ()
        , initialized_        (false)
        , schedulesAnchored_  (false)
        , decisionReady_      (false)
        , actuationApplied_   (false)
        , pendingRecordReady_ (false)
        , outputFaultLatched_ (false)
    {
        snapshot_.mode           = GreenhouseMode::Starting;
        snapshot_.sensorStatus   = StatusCode::NotInitialized;
        snapshot_.outputStatus   = StatusCode::NotInitialized;
        snapshot_.displayStatus  = StatusCode::NotInitialized;
        snapshot_.recordStatus   = StatusCode::NotInitialized;
        snapshot_.recordSequence = 0;
    }

    GreenhouseController::~GreenhouseController () noexcept
    {
        shutdown ();
    }

    Status GreenhouseController::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!configValid ())
        {
            return StatusCode::InvalidArgument;
        }

        Status status = moisture_->initialize ();

        if (!status.ok ())
        {
            moisture_->shutdown ();
            return status;
        }

        status = watering_->initialize ();

        if (!status.ok ())
        {
            watering_->shutdown ();
            moisture_->shutdown ();
            return status;
        }

        status = display_->initialize ();

        if (!status.ok ())
        {
            display_->shutdown  ();
            watering_->shutdown ();
            moisture_->shutdown ();
            return status;
        }

        status = records_->initialize ();

        if (!status.ok ())
        {
            records_->shutdown  ();
            display_->shutdown  ();
            watering_->shutdown ();
            moisture_->shutdown ();
            return status;
        }

        snapshot_.mode           = GreenhouseMode::Starting;
        snapshot_.sensorStatus   = StatusCode::Ok;
        snapshot_.outputStatus   = StatusCode::Ok;
        snapshot_.displayStatus  = StatusCode::Ok;
        snapshot_.recordStatus   = StatusCode::Ok;
        snapshot_.recordSequence = 0;
        pendingRecord_.length    = 0;
        initialized_             = true;
        schedulesAnchored_       = false;
        decisionReady_           = false;
        actuationApplied_        = false;
        pendingRecordReady_      = false;
        outputFaultLatched_      = false;
        return StatusCode::Ok;
    }

    void GreenhouseController::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        watering_->decide   (decisionAt_, snapshot_.moisture, false);
        watering_->actuate  ();
        records_->shutdown  ();
        display_->shutdown  ();
        watering_->shutdown ();
        moisture_->shutdown ();

        snapshot_.mode          = GreenhouseMode::Starting;
        snapshot_.sensorStatus  = StatusCode::NotInitialized;
        snapshot_.outputStatus  = StatusCode::NotInitialized;
        snapshot_.displayStatus = StatusCode::NotInitialized;
        snapshot_.recordStatus  = StatusCode::NotInitialized;
        initialized_            = false;
        schedulesAnchored_      = false;
        decisionReady_          = false;
        actuationApplied_       = false;
        pendingRecordReady_     = false;
        outputFaultLatched_     = false;
    }

    Status GreenhouseController::observe (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (decisionReady_ && !actuationApplied_)
        {
            return StatusCode::InvalidArgument;
        }

        if (!schedulesAnchored_)
        {
            anchorSchedules (now);
        }
        else if (!due (now, sampleEpoch_, config_.sampleInterval))
        {
            return StatusCode::Ok;
        }

        const Status status = moisture_->update (now);

        snapshot_.sensorStatus = status;
        snapshot_.moisture     = moisture_->sample (now, config_.staleAfter);
        sampleEpoch_           = now;
        refreshMode            ();
        return status;
    }

    Status GreenhouseController::decide (TimePoint now,
                                         const GreenhouseInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (decisionReady_ && !actuationApplied_)
        {
            return StatusCode::InvalidArgument;
        }

        snapshot_.moisture = moisture_->sample (now, config_.staleAfter);

        const bool allowed = input.wateringAllowed &&
                             snapshot_.sensorStatus.ok () &&
                             !outputFaultLatched_;
        const Status status = watering_->decide (now, snapshot_.moisture, allowed);

        snapshot_.watering = watering_->snapshot ();
        decisionAt_        = now;
        decisionReady_     = true;
        actuationApplied_  = false;
        refreshMode        ();
        decided_           = snapshot_;
        return status;
    }

    Status GreenhouseController::actuate (TimePoint) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!decisionReady_)
        {
            return StatusCode::InvalidArgument;
        }

        if (actuationApplied_)
        {
            return snapshot_.outputStatus;
        }

        const Status status = watering_->actuate ();

        snapshot_.outputStatus = status;
        snapshot_.watering     = watering_->snapshot ();

        actuationApplied_ = true;

        if (!status.ok ())
        {
            outputFaultLatched_ = true;
        }

        refreshMode ();
        decided_ = snapshot_;
        return status;
    }

    Status GreenhouseController::present (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!decisionReady_ || !actuationApplied_)
        {
            return StatusCode::InvalidArgument;
        }

        if (!schedulesAnchored_ ||
            !due (now, displayEpoch_, config_.displayInterval))
        {
            return StatusCode::Ok;
        }

        const Status status = prepareView (now);

        snapshot_.displayStatus = status;

        if (status.ok ())
        {
            displayEpoch_ = now;
        }

        refreshMode ();
        return status;
    }

    Status GreenhouseController::record (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!decisionReady_ || !actuationApplied_)
        {
            return StatusCode::InvalidArgument;
        }

        if (!pendingRecordReady_)
        {
            if (!schedulesAnchored_ ||
                !due (now, recordEpoch_, config_.recordInterval))
            {
                return StatusCode::Ok;
            }

            const Status formatStatus = prepareRecord (decisionAt_);

            if (!formatStatus.ok ())
            {
                snapshot_.recordStatus = formatStatus;
                refreshMode ();
                return formatStatus;
            }
        }

        const Status status = records_->append (pendingRecord_);

        snapshot_.recordStatus = status;

        if (status.ok ())
        {
            ++snapshot_.recordSequence;
            recordEpoch_        = now;
            pendingRecordReady_ = false;
        }

        refreshMode ();
        return status;
    }

    GreenhouseSnapshot GreenhouseController::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool GreenhouseController::initialized () const noexcept
    {
        return initialized_;
    }

    bool GreenhouseController::configValid () const noexcept
    {
        const uint32_t sample  = config_.sampleInterval.milliseconds  ();
        const uint32_t display = config_.displayInterval.milliseconds ();
        const uint32_t record  = config_.recordInterval.milliseconds  ();
        const uint32_t stale   = config_.staleAfter.milliseconds      ();

        return sample != 0U && display != 0U && record != 0U &&
               stale >= sample && sample <= maximumForwardElapsed &&
               display <= maximumForwardElapsed &&
               record <= maximumForwardElapsed &&
               stale <= maximumForwardElapsed;
    }

    bool GreenhouseController::due (TimePoint now, TimePoint epoch,
                                    Duration interval) const noexcept
    {
        return now.elapsedSince (epoch) >= interval;
    }

    void GreenhouseController::anchorSchedules (TimePoint now) noexcept
    {
        sampleEpoch_      = now;
        displayEpoch_     = now;
        recordEpoch_      = now;
        schedulesAnchored_ = true;
    }

    void GreenhouseController::refreshMode () noexcept
    {
        const bool sensorFault  = !snapshot_.sensorStatus.ok () ||
                                  snapshot_.moisture.state !=
                                      MoistureSampleState::Valid;
        const bool outputFault  = outputFaultLatched_;
        const bool displayFault = !snapshot_.displayStatus.ok ();
        const bool recordFault  = !snapshot_.recordStatus.ok  ();
        const uint8_t faults = static_cast<uint8_t> (
            static_cast<uint8_t> (sensorFault) +
            static_cast<uint8_t> (outputFault) +
            static_cast<uint8_t> (displayFault) +
            static_cast<uint8_t> (recordFault));

        if (outputFault && faults > 1U)
        {
            snapshot_.mode = GreenhouseMode::MultipleFaults;
        }
        else if (outputFault)
        {
            snapshot_.mode = GreenhouseMode::OutputFault;
        }
        else if (sensorFault)
        {
            snapshot_.mode = GreenhouseMode::SensorFault;
        }
        else if (displayFault)
        {
            snapshot_.mode = GreenhouseMode::DisplayFault;
        }
        else if (recordFault)
        {
            snapshot_.mode = GreenhouseMode::RecordFault;
        }
        else if (snapshot_.watering.reason == WateringReason::OperatorInhibit)
        {
            snapshot_.mode = GreenhouseMode::Inhibited;
        }
        else if (snapshot_.watering.requestedPump == PumpState::On)
        {
            snapshot_.mode = GreenhouseMode::Watering;
        }
        else
        {
            snapshot_.mode = GreenhouseMode::Monitoring;
        }
    }

    Status GreenhouseController::prepareRecord (TimePoint now) noexcept
    {
        StableRecord& record = pendingRecord_;

        record.length = 0;

        const bool valid = decided_.moisture.state == MoistureSampleState::Valid;
        bool success = appendText     (record, "adk-gh,1,") &&
                       appendUnsigned (record, decided_.recordSequence) &&
                       appendText     (record, ",") &&
                       appendUnsigned (record, now.milliseconds ()) &&
                       appendText     (record, ",") &&
                       appendText     (record,
                                       sampleToken (decided_.moisture.state)) &&
                       appendText     (record, ",") &&
                       appendUnsigned (record, decided_.moisture.rawReading) &&
                       appendText     (record, ",");

        if (valid)
        {
            success = success &&
                      appendUnsigned (record, decided_.moisture.moisturePermille);
        }
        else
        {
            success = success && appendText (record, "-");
        }

        success = success && appendText (record, ",") &&
                  appendText (record,
                              waterToken (decided_.watering.requestedPump)) &&
                  appendText (record, ",") &&
                  appendText (record, reasonToken (decided_.watering.reason)) &&
                  appendText (record, ",") &&
                  appendText (record, modeToken (decided_.mode)) &&
                  appendText (record, "\n");

        if (!success)
        {
            record.length = 0;
            return StatusCode::CapacityExceeded;
        }

        pendingRecordReady_ = true;
        return StatusCode::Ok;
    }

    Status GreenhouseController::prepareView (TimePoint now) noexcept
    {
        char lineOne[characterDisplayColumns + 1];
        char lineTwo[characterDisplayColumns + 1];

        fillLine (lineOne, decided_.mode == GreenhouseMode::Monitoring
                               ? "MOISTURE READY"
                               : modeToken (decided_.mode));
        fillLine (lineTwo, decided_.watering.requestedPump == PumpState::On
                               ? "PUMP ON"
                               : "PUMP OFF");

        Status status = display_->show (0, lineOne);

        if (status.ok ())
        {
            status = display_->show (1, lineTwo);
        }

        if (status.ok ())
        {
            status = display_->update (now);
        }

        return status;
    }
} // namespace adk
