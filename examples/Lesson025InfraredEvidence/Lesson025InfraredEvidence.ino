// Mega 2560, receive only: active-low receiver OUT=D2; D45 drives LED via 1 kOhm.
// Identify the receiver and verify its datasheet pinout before applying power.
#include <Adk.h>
#include <infrared_decoder.h>
#include <infrared_record.h>
#include <mega_pulse_capture_io.h>
#include <pulse_capture.h>

namespace {

    constexpr adk::PinId capturePin  = 2;
    constexpr adk::PinId evidencePin = 45;

    const adk::PulseCaptureConfig captureConfig = {adk::MicrosecondDuration (12000),
                                                   adk::MicrosecondDuration (100),
                                                   adk::MicrosecondDuration (10000)};

    enum struct EvidenceSignal : uint8_t
    {
        Waiting,
        Valid,
        Unknown,
        Fault
    };

    adk::Runtime            runtime;
    adk::MegaPulseCaptureIo captureIo;
    adk::PulseCapture       capture (runtime.resources (), captureIo, capturePin,
                                     captureConfig);

    adk::MonoLed acquisitionEvidence (runtime.resources (), LED_BUILTIN);
    adk::MonoLed frameEvidence       (runtime.resources (), evidencePin);

    adk::InfraredDecoder       decoder;
    adk::InfraredRecordEncoder recordEncoder;

    adk::TimePoint signalStarted;
    EvidenceSignal signal  = EvidenceSignal::Waiting;
    bool           running = false;

    bool acquireInfraredEvidence ();
    bool observeInfrared         (adk::MicrosecondTimePoint now);
    void decideFrameEvidence     (const adk::InfraredFrame& frame);
    bool recordFrameEvidence     (const adk::InfraredFrame& frame);
    bool showFrameEvidence       (adk::TimePoint now);

    void observeFailure (adk::Status status);
    void stopSafely     ();

} // namespace

void setup ()
{
    Serial.begin (115200);

    running = acquireInfraredEvidence ();
}

void loop ()
{
    if (!running)
    {
        return;
    }

    if (!observeInfrared (adk::MicrosecondTimePoint (micros ())))
    {
        stopSafely ();
        return;
    }

    if (!showFrameEvidence (adk::TimePoint (millis ())))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireInfraredEvidence ()
    {
        const adk::Status captureStatus = capture.initialize ();

        if (!captureStatus.ok ())
        {
            observeFailure (captureStatus);
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

        const adk::Status evidenceStatus = frameEvidence.initialize ();

        if (!evidenceStatus.ok ())
        {
            observeFailure (evidenceStatus);
            stopSafely     ();
            return false;
        }

        signalStarted = adk::TimePoint (millis ());

        const adk::Status acquisitionOnStatus = acquisitionEvidence.on ();

        if (!acquisitionOnStatus.ok ())
        {
            observeFailure (acquisitionOnStatus);
            stopSafely     ();
            return false;
        }

        return true;
    }

    bool observeInfrared (adk::MicrosecondTimePoint now)
    {
        const adk::Status updateStatus = capture.update (now);
        const bool        overrun =
            updateStatus.error () == adk::StatusCode::CapacityExceeded;

        if (!updateStatus.ok () && !overrun)
        {
            observeFailure (updateStatus);
            return false;
        }

        const adk::PulseFrame captured = capture.frame ();

        if (captured.state != adk::CaptureState::Complete &&
            captured.state != adk::CaptureState::Overflow &&
            captured.state != adk::CaptureState::TimingFault)
        {
            if (overrun)
            {
                signal        = EvidenceSignal::Fault;
                signalStarted = adk::TimePoint (millis ());
            }

            return true;
        }

        adk::InfraredFrame frame;
        const adk::Status  decodeStatus = decoder.decode (captured, frame);

        if (!decodeStatus.ok ())
        {
            observeFailure (decodeStatus);
            return false;
        }

        decideFrameEvidence (frame);

        const bool recorded = recordFrameEvidence (frame);

        if (overrun || !recorded)
        {
            signal = EvidenceSignal::Fault;
        }

        const adk::Status acknowledgeStatus = capture.acknowledge (captured.sequence);

        if (!acknowledgeStatus.ok ())
        {
            observeFailure (acknowledgeStatus);
            return false;
        }

        signalStarted = adk::TimePoint (millis ());
        return true;
    }

    void decideFrameEvidence (const adk::InfraredFrame& frame)
    {
        switch (frame.validity)
        {
            case adk::FrameValidity::Valid: signal = EvidenceSignal::Valid; break;
            case adk::FrameValidity::UnknownProtocol:
                signal = EvidenceSignal::Unknown;
                break;
            default: signal = EvidenceSignal::Fault; break;
        }
    }

    bool recordFrameEvidence (const adk::InfraredFrame& frame)
    {
        char record[80];

        const adk::Result<uint16_t> result =
            recordEncoder.encode (frame, {record, sizeof (record)});

        if (!result.ok ())
        {
            observeFailure (result.status ());
            return false;
        }

        Serial.write (reinterpret_cast<const uint8_t*> (record), result.value ());
        return true;
    }

    bool showFrameEvidence (adk::TimePoint now)
    {
        const uint32_t phase = now.elapsedSince (signalStarted).milliseconds ();
        bool           active;

        switch (signal)
        {
            case EvidenceSignal::Waiting: active = false; break;
            case EvidenceSignal::Valid: active = phase < 1000; break;
            case EvidenceSignal::Unknown:
                active = phase < 150 || (phase >= 300 && phase < 450);
                break;
            case EvidenceSignal::Fault:
                active = phase < 100 || (phase >= 200 && phase < 300) ||
                         (phase >= 400 && phase < 500) || (phase >= 600 && phase < 700);
                break;
        }

        return frameEvidence.set (active).ok ();
    }

    void observeFailure (adk::Status status)
    {
        (void)status;
        signal = EvidenceSignal::Fault;
    }

    void stopSafely ()
    {
        frameEvidence.off            ();
        acquisitionEvidence.off      ();
        frameEvidence.shutdown       ();
        acquisitionEvidence.shutdown ();
        capture.shutdown             ();
        running = false;
    }

} // namespace
