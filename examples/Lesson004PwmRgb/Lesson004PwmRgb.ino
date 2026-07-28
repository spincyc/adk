#include <Adk.h>

namespace {

    constexpr unsigned long acquisitionEvidenceMs = 250;
    constexpr unsigned long separationMs          = 750;
    constexpr unsigned long behaviorStepMs        = 1000;

    enum class Phase : uint8_t
    {
        AcquisitionEvidence,
        Separation,
        Behavior,
        Shutdown
    };

    adk::Runtime runtime;
    adk::RgbLed  led                (runtime.resources (), {6, 220}, {5, 220},
                                    {3, 220});
    adk::MonoLed acquisitionEvidence (runtime.resources (), LED_BUILTIN);

    const adk::Rgb colors[]{adk::Rgb (255, 0, 0), adk::Rgb (0, 255, 0),
                            adk::Rgb (0, 0, 255), adk::Rgb (255, 255, 255),
                            adk::Rgb (0, 0, 0),   adk::Rgb (255, 64, 0)};

    constexpr uint8_t colorCount = sizeof (colors) / sizeof (colors[0]);

    bool          ready          = false;
    Phase         phase          = Phase::Shutdown;
    uint8_t       colorIndex     = 0;
    unsigned long phaseStartedAt = 0;

    bool phaseElapsed (unsigned long now, unsigned long duration);
    void enterPhase   (Phase next, unsigned long now);
    void showColor    (uint8_t index, unsigned long now);
    void stopSafely   ();

} // namespace

void setup ()
{
    ready = led.initialize ().ok ();
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
                enterPhase (Phase::Behavior, now);
            }
            break;
        case Phase::Behavior:
            if (phaseElapsed (now, behaviorStepMs))
            {
                if (colorIndex < colorCount)
                {
                    showColor (colorIndex, now);
                }
                else
                {
                    enterPhase (Phase::Shutdown, now);
                }
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
            case Phase::Behavior:
                colorIndex = 0;
                showColor (colorIndex, now);
                return;
            case Phase::Shutdown:
                stopSafely ();
                return;
        }

        if (!status.ok ())
        {
            stopSafely ();
        }
    }

    void showColor (uint8_t index, unsigned long now)
    {
        if (!led.set (colors[index]).ok ())
        {
            stopSafely ();
            return;
        }

        ++colorIndex;
        phaseStartedAt = now;
    }

    void stopSafely ()
    {
        acquisitionEvidence.shutdown ();

        led.shutdown ();
        ready = false;
        phase = Phase::Shutdown;
    }

} // namespace
