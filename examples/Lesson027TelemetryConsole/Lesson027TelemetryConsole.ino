// Mega 2560, USB only: D30 and D31 use pull-up buttons; D5-D7 drive an
// RGB LED through one 330 ohm resistor per channel; D13 shows record acceptance.
#include <Adk.h>
#include <record_sink.h>
#include <telemetry_console.h>
#include <telemetry_console_project.h>

namespace {

    constexpr adk::PinId nextButtonPin        = 30;
    constexpr adk::PinId acknowledgeButtonPin = 31;
    constexpr adk::PinId recordEvidencePin    = LED_BUILTIN;

    const adk::ButtonConfig nextButtonConfig        (nextButtonPin);
    const adk::ButtonConfig acknowledgeButtonConfig (acknowledgeButtonPin);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};

    const adk::ConsoleSourceConfig sourceConfigs[] = {
        {101, adk::TelemetryKind::Temperature},
        {202, adk::TelemetryKind::RelativeHumidity},
        {303, adk::TelemetryKind::Contact}};
    const adk::TelemetryConsoleConfig consoleConfig = {
        sourceConfigs,
        3,
        adk::Duration (3000),
        adk::Duration (5000),
        adk::Duration (500),
        2};

    struct ConsoleButtons final : adk::TelemetryConsoleOperator
    {
        ConsoleButtons (adk::ResourceRegistry& resources) noexcept
            : next_         (resources, nextButtonConfig),
              acknowledge_  (resources, acknowledgeButtonConfig),
              initialized_  (false)
        {
        }

        ~ConsoleButtons () noexcept override
        {
            shutdown ();
        }

        adk::Status initialize () noexcept override
        {
            if (initialized_)
            {
                return adk::StatusCode::Ok;
            }

            adk::Status status = next_.initialize ();

            if (!status.ok ())
            {
                return status;
            }

            status = acknowledge_.initialize ();

            if (!status.ok ())
            {
                next_.shutdown ();
                return status;
            }

            initialized_ = true;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            acknowledge_.shutdown ();
            next_.shutdown        ();
            initialized_ = false;
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

        adk::Status update (adk::TimePoint now) noexcept override
        {
            if (!initialized_)
            {
                return adk::StatusCode::NotInitialized;
            }

            next_.update        (now);
            acknowledge_.update (now);
            return adk::StatusCode::Ok;
        }

        bool nextPressEvent () const noexcept override
        {
            return next_.pressEvent ();
        }

        bool acknowledgePressEvent () const noexcept override
        {
            return acknowledge_.pressEvent ();
        }

      private:
        adk::Button next_;
        adk::Button acknowledge_;
        bool        initialized_;
    };

    struct ConsoleEvidence final : adk::TelemetryConsolePresentation
    {
        explicit ConsoleEvidence (adk::ResourceRegistry& resources) noexcept
            : health_ (resources, redChannel, greenChannel, blueChannel),
              initialized_ (false)
        {
        }

        ~ConsoleEvidence () noexcept override
        {
            shutdown ();
        }

        adk::Status initialize () noexcept override
        {
            const adk::Status status = health_.initialize ();

            initialized_ = status.ok ();
            return status;
        }

        void shutdown () noexcept override
        {
            health_.off      ();
            health_.shutdown ();
            initialized_ = false;
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

        adk::Status present (adk::TimePoint            now,
                             const adk::ConsoleOutput& output,
                             const adk::ConsoleSource&) noexcept override
        {
            if (!initialized_)
            {
                return adk::StatusCode::NotInitialized;
            }

            const bool visible = cadenceVisible (now, output.health);

            if (!visible)
            {
                return health_.off ();
            }

            return health_.set (healthColor (output.health));
        }

      private:
        static bool cadenceVisible (adk::TimePoint now,
                                    adk::ConsoleHealth health) noexcept
        {
            uint16_t interval = 0;

            switch (health)
            {
                case adk::ConsoleHealth::Starting: interval = 500; break;
                case adk::ConsoleHealth::Healthy: return true;
                case adk::ConsoleHealth::Degraded: interval = 250; break;
                case adk::ConsoleHealth::Fault: interval = 100; break;
                case adk::ConsoleHealth::Stopped: return false;
            }

            return ((now.milliseconds () / interval) & 1U) == 0U;
        }

        static adk::Rgb healthColor (adk::ConsoleHealth health) noexcept
        {
            switch (health)
            {
                case adk::ConsoleHealth::Starting: return adk::Rgb (0, 0, 96);
                case adk::ConsoleHealth::Healthy:  return adk::Rgb (0, 96, 0);
                case adk::ConsoleHealth::Degraded: return adk::Rgb (96, 40, 0);
                case adk::ConsoleHealth::Fault:    return adk::Rgb (96, 0, 0);
                case adk::ConsoleHealth::Stopped:  return adk::Rgb ();
            }

            return adk::Rgb ();
        }

        adk::RgbLed health_;
        bool        initialized_;
    };

    struct EvidenceRecordSink final : adk::RecordSink
    {
        explicit EvidenceRecordSink (adk::ResourceRegistry& resources) noexcept
            : accepted_ (resources, recordEvidencePin), last_ {},
              initialized_ (false), evidenceOn_ (false)
        {
        }

        ~EvidenceRecordSink () noexcept override
        {
            shutdown ();
        }

        adk::Status initialize () noexcept override
        {
            const adk::Status status = accepted_.initialize ();

            initialized_ = status.ok ();
            evidenceOn_  = false;
            return status;
        }

        void shutdown () noexcept override
        {
            accepted_.off      ();
            accepted_.shutdown ();
            initialized_ = false;
            evidenceOn_  = false;
        }

        adk::Status append (const adk::StableRecord& record) noexcept override
        {
            if (!initialized_)
            {
                return adk::StatusCode::NotInitialized;
            }

            last_       = record;
            evidenceOn_ = !evidenceOn_;
            return accepted_.set (evidenceOn_);
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

      private:
        adk::MonoLed     accepted_;
        adk::StableRecord last_;
        bool             initialized_;
        bool             evidenceOn_;
    };

    adk::Runtime runtime;

    ConsoleButtons                operatorInput (runtime.resources ());
    ConsoleEvidence               presentation  (runtime.resources ());
    EvidenceRecordSink            records       (runtime.resources ());
    adk::TelemetryConsole         console       (consoleConfig);
    adk::TelemetryConsoleProject  project       (console, operatorInput, presentation,
                                                 records);

    adk::ConsoleSource observations[3] = {};
    uint8_t            observationCount = 0;
    bool               running          = false;

    bool acquireConsole          ();
    void observeTelemetrySources (adk::TimePoint now);
    void decideConsoleState      (adk::TimePoint now);
    void presentConsoleState     ();
    void recordConsoleEvidence   ();
    void stopSafely              ();

    uint8_t            buildFixture  (adk::TimePoint now);
    adk::ConsoleSource fixtureSource (uint16_t           sourceId,
                                      adk::TelemetryKind kind,
                                      int32_t            value,
                                      int8_t             exponent,
                                      adk::Freshness     freshness,
                                      adk::SequenceState sequence) noexcept;

} // namespace

void setup ()
{
    running = acquireConsole ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    observeTelemetrySources (now);
    decideConsoleState      (now);
    presentConsoleState     ();
    recordConsoleEvidence   ();
}

namespace {

    bool acquireConsole ()
    {
        return project.initialize ().ok ();
    }

    void observeTelemetrySources (adk::TimePoint now)
    {
        observationCount = buildFixture (now);
    }

    void decideConsoleState (adk::TimePoint now)
    {
        const adk::Status status =
            project.update (now, observations, observationCount);

        if (!status.ok () && !status.transient ())
        {
            stopSafely ();
        }
    }

    void presentConsoleState ()
    {
        const adk::TelemetryConsoleProjectSnapshot state = project.snapshot ();

        if (!state.presentationStatus.ok () &&
            !state.presentationStatus.transient ())
        {
            stopSafely ();
        }
    }

    void recordConsoleEvidence ()
    {
        const adk::TelemetryConsoleProjectSnapshot state = project.snapshot ();

        if (!state.recordStatus.ok () && !state.recordStatus.transient ())
        {
            stopSafely ();
        }
    }

    void stopSafely ()
    {
        project.shutdown ();
        running = false;
    }

    uint8_t buildFixture (adk::TimePoint now)
    {
        const uint32_t phase = now.milliseconds ();
        uint8_t        count = 0;

        if (phase >= 2000)
        {
            observations[count++] = fixtureSource (
                101, adk::TelemetryKind::Temperature, 217, -1,
                adk::Freshness::Fresh, adk::SequenceState::InOrder);
        }

        if (phase >= 4000)
        {
            const adk::Freshness humidityFreshness =
                phase >= 12000 && phase < 15000 ? adk::Freshness::Stale
                                                : phase >= 9000 && phase < 12000
                                                      ? adk::Freshness::Aging
                                                      : adk::Freshness::Fresh;
            observations[count++] = fixtureSource (
                202, adk::TelemetryKind::RelativeHumidity, 483, -1,
                humidityFreshness, adk::SequenceState::InOrder);
        }

        if (phase >= 6000)
        {
            observations[count++] = fixtureSource (
                303, adk::TelemetryKind::Contact, 1, 0,
                adk::Freshness::Fresh, adk::SequenceState::InOrder);
        }

        return count;
    }

    adk::ConsoleSource fixtureSource (uint16_t           sourceId,
                                      adk::TelemetryKind kind,
                                      int32_t            value,
                                      int8_t             exponent,
                                      adk::Freshness     freshness,
                                      adk::SequenceState sequence) noexcept
    {
        return {sourceId,
                kind,
                adk::SampleQuality::Valid,
                sequence,
                freshness,
                value,
                exponent,
                adk::PacketValidity::Valid,
                adk::StatusCode::Ok,
                true};
    }

} // namespace
