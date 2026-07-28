#include <Adk.h>

namespace {

    constexpr unsigned long acquisitionEvidenceMs = 250;
    constexpr unsigned long separationMs          = 750;
    constexpr adk::PinId                   sounderPin = 6;
    constexpr adk::PiezoSounder::Frequency cueHz      = 440;
    const adk::Duration                    cueLength   (250);

    enum class Phase : uint8_t
    {
        AcquisitionEvidence,
        Separation,
        Cue,
        Shutdown
    };

    adk::Runtime      runtime;
    adk::PiezoSounder sounder             (runtime.resources (), sounderPin);
    adk::MonoLed      acquisitionEvidence (runtime.resources (), LED_BUILTIN);

    bool          ready          = false;
    Phase         phase          = Phase::Shutdown;
    unsigned long phaseStartedAt = 0;

    bool phaseElapsed (unsigned long now, unsigned long duration);
    void enterPhase   (Phase next, unsigned long now);
    void stopSafely   ();

} // namespace

void setup ()
{
    ready = sounder.initialize ().ok ();
    if (ready)
    {
        ready = acquisitionEvidence.initialize ().ok ();
    }

    if (ready)
    {
        phaseStartedAt = millis ();

        enterPhase (Phase::AcquisitionEvidence, phaseStartedAt);
    }
    else
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    const unsigned long now = millis ();
    switch (phase)
    {
        case Phase::AcquisitionEvidence:
            if (phaseElapsed (now, acquisitionEvidenceMs))
            {
                enterPhase (Phase::Separation, now);
            }
            break;
        case Phase::Separation:
            if (phaseElapsed (now, separationMs))
            {
                enterPhase (Phase::Cue, now);
            }
            break;
        case Phase::Cue:
            sounder.update (adk::TimePoint (now));

            if (!sounder.sounding ())
            {
                enterPhase (Phase::Shutdown, now);
            }
            break;
        case Phase::Shutdown:
            stopSafely ();
            break;
    }
}

namespace {

    bool phaseElapsed (unsigned long now, unsigned long duration)
    {
        return now - phaseStartedAt >= duration;
    }

    void enterPhase (Phase next, unsigned long now)
    {
        phase          = next;
        phaseStartedAt = now;

        adk::Status status;
        switch (phase)
        {
            case Phase::AcquisitionEvidence:
                status = acquisitionEvidence.on ();
                break;
            case Phase::Separation:
                status = acquisitionEvidence.off ();
                break;
            case Phase::Cue:
                status = sounder.play (cueHz, cueLength, adk::TimePoint (now));
                break;
            case Phase::Shutdown:
                stopSafely ();
                return;
        }

        if (!status.ok ())
        {
            stopSafely ();
        }
    }

    void stopSafely ()
    {
        acquisitionEvidence.shutdown ();

        sounder.shutdown ();
        ready = false;
        phase = Phase::Shutdown;
    }

} // namespace
