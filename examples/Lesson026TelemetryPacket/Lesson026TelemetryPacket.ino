// Mega 2560, fixture only: D45 drives an evidence LED through 1 kOhm to GND.
// The packet schedule is generated locally. This sketch receives and emits no RF.
#include <Adk.h>
#include <observation_tracker.h>
#include <telemetry_evidence.h>
#include <telemetry_packet.h>

namespace {

    constexpr adk::PinId evidencePin = 45;

    const adk::ObservationTrackerConfig trackerConfig = {adk::Duration (2000),
                                                         adk::Duration (5000)};

    adk::Runtime              runtime;
    adk::MonoLed              acquisitionEvidence (runtime.resources (), LED_BUILTIN);
    adk::MonoLed              packetEvidence      (runtime.resources (), evidencePin);
    adk::TelemetryPacketCodec packetCodec;
    adk::ObservationTracker   tracker (26, trackerConfig);
    adk::TelemetryFixtureSchedule fixtureSchedule;
    adk::TelemetryEvidenceModel   evidenceModel;

    adk::TimePoint               signalStarted;
    adk::PacketValidity          packetValidity = adk::PacketValidity::Valid;
    adk::TelemetryEvidenceSignal signal         = adk::TelemetryEvidenceSignal::Fresh;
    bool                         running        = false;

    bool acquireTelemetryEvidence (adk::TimePoint now);
    bool observePacket            (adk::TimePoint now, adk::TelemetryFixtureDecision& decision);
    bool verifyPacket             (const adk::TelemetryFixtureDecision& decision,
                       adk::TimePoint                       now);
    bool updateFreshness         (adk::TimePoint now);
    bool showObservationEvidence (adk::TimePoint now);

    void chooseEvidence (adk::Status status, adk::TimePoint now);
    void observeFailure (adk::Status status);
    void stopSafely     ();

} // namespace

void setup ()
{
    running = acquireTelemetryEvidence (adk::TimePoint (millis ()));
}

void loop ()
{
    const adk::TimePoint          now (millis ());
    adk::TelemetryFixtureDecision decision;

    if (!running)
    {
        return;
    }

    if (!observePacket (now, decision) || !verifyPacket (decision, now) ||
        !updateFreshness (now) || !showObservationEvidence (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireTelemetryEvidence (adk::TimePoint now)
    {
        const adk::Status trackerStatus = tracker.initialize ();

        if (!trackerStatus.ok ())
        {
            observeFailure (trackerStatus);
            return false;
        }

        const adk::Status fixtureStatus = fixtureSchedule.initialize ();

        if (!fixtureStatus.ok ())
        {
            observeFailure (fixtureStatus);
            stopSafely     ();
            return false;
        }

        const adk::Status acquisitionStatus = acquisitionEvidence.initialize ();

        if (!acquisitionStatus.ok ())
        {
            observeFailure (acquisitionStatus);
            stopSafely     ();
            return false;
        }

        const adk::Status evidenceStatus = packetEvidence.initialize ();

        if (!evidenceStatus.ok ())
        {
            observeFailure (evidenceStatus);
            stopSafely     ();
            return false;
        }

        const adk::Status acquisitionOnStatus = acquisitionEvidence.on ();

        if (!acquisitionOnStatus.ok ())
        {
            observeFailure (acquisitionOnStatus);
            stopSafely     ();
            return false;
        }

        signalStarted = now;
        return true;
    }

    bool observePacket (adk::TimePoint now, adk::TelemetryFixtureDecision& decision)
    {
        const adk::Result<adk::TelemetryFixtureDecision> result =
            fixtureSchedule.update (now);

        if (!result.ok ())
        {
            observeFailure (result.status ());
            return false;
        }

        decision = result.value ();
        return true;
    }

    bool verifyPacket (const adk::TelemetryFixtureDecision& decision,
                       adk::TimePoint                       now)
    {
        if (decision.action == adk::TelemetryFixtureAction::None)
        {
            return true;
        }

        if (decision.action == adk::TelemetryFixtureAction::Silence)
        {
            packetValidity = adk::PacketValidity::Valid;
            return true;
        }

        adk::TelemetrySample fixture = {26,
                                        decision.sequence,
                                        now.milliseconds (),
                                        adk::TelemetryKind::Temperature,
                                        adk::SampleQuality::Valid,
                                        215,
                                        -1};
        uint8_t              packet[adk::TelemetryPacketCodec::size];

        const adk::Result<uint16_t> encoded =
            packetCodec.encode (fixture, {packet, sizeof (packet)});

        if (!encoded.ok ())
        {
            observeFailure (encoded.status ());
            return false;
        }

        if (decision.action == adk::TelemetryFixtureAction::Corrupt)
        {
            packet[sizeof (packet) - 1] ^= 0x01;
        }

        adk::TelemetrySample decoded;
        packetValidity = packetCodec.decode ({packet, sizeof (packet)}, decoded);

        if (packetValidity != adk::PacketValidity::Valid)
        {
            chooseEvidence (adk::StatusCode::Ok, now);
            return true;
        }

        const adk::Status acceptStatus = tracker.accept (decoded, now);

        if (!acceptStatus.ok ())
        {
            observeFailure (acceptStatus);
            return false;
        }

        chooseEvidence (acceptStatus, now);
        return true;
    }

    bool updateFreshness (adk::TimePoint now)
    {
        const adk::Status updateStatus = tracker.update (now);

        if (!updateStatus.ok ())
        {
            observeFailure (updateStatus);
            return false;
        }

        chooseEvidence (updateStatus, now);
        return true;
    }

    void chooseEvidence (adk::Status status, adk::TimePoint now)
    {
        const adk::TelemetryEvidenceSignal next =
            evidenceModel.decide (packetValidity, tracker.state (), status);

        if (next != signal)
        {
            signal        = next;
            signalStarted = now;
        }
    }

    bool showObservationEvidence (adk::TimePoint now)
    {
        const uint32_t phase = now.elapsedSince (signalStarted).milliseconds ();
        bool           active;

        switch (signal)
        {
            case adk::TelemetryEvidenceSignal::Fresh:
                active = phase % 2000 < 1000;
                break;
            case adk::TelemetryEvidenceSignal::GapOrAging:
                active =
                    phase % 1000 < 150 || (phase % 1000 >= 300 && phase % 1000 < 450);
                break;
            case adk::TelemetryEvidenceSignal::Stale:
                active = phase % 1200 < 120 ||
                         (phase % 1200 >= 240 && phase % 1200 < 360) ||
                         (phase % 1200 >= 480 && phase % 1200 < 600);
                break;
            case adk::TelemetryEvidenceSignal::Corrupt:
            case adk::TelemetryEvidenceSignal::Fault:
                active = phase % 1200 < 100 ||
                         (phase % 1200 >= 200 && phase % 1200 < 300) ||
                         (phase % 1200 >= 400 && phase % 1200 < 500) ||
                         (phase % 1200 >= 600 && phase % 1200 < 700);
                break;
        }

        return packetEvidence.set (active).ok ();
    }

    void observeFailure (adk::Status status)
    {
        (void)status;
        signal = adk::TelemetryEvidenceSignal::Fault;
    }

    void stopSafely ()
    {
        packetEvidence.off           ();
        acquisitionEvidence.off      ();
        packetEvidence.shutdown      ();
        acquisitionEvidence.shutdown ();
        fixtureSchedule.shutdown     ();
        tracker.shutdown             ();
        running = false;
    }

} // namespace
