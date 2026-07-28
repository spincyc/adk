#include <Adk.h>

namespace {

    constexpr unsigned long acquisitionEvidenceMs = 250;
    constexpr unsigned long separationMs          = 750;
    constexpr unsigned long behaviorHalfCycleMs   = 500;
    constexpr uint8_t       behaviorCycleCount    = 3;

    enum class Phase : uint8_t
    {
        AcquisitionEvidence,
        Separation,
        BehaviorHigh,
        BehaviorLow,
        Shutdown
    };

    adk::Runtime       runtime;
    adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);
    bool               ready = false;
    Phase              phase = Phase::Shutdown;
    uint8_t            completedCycles = 0;
    unsigned long      phaseStartedAt   = 0;

    bool phaseElapsed (unsigned long now, unsigned long duration);
    void enterPhase   (Phase next, unsigned long now);
    void stopSafely   ();

} // namespace

void setup ()
{
    ready          = led.initialize ().ok ();
    if (ready)
    {
        phaseStartedAt = millis ();

        enterPhase     (Phase::AcquisitionEvidence, phaseStartedAt);
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
                enterPhase (Phase::BehaviorHigh, now);
            }
            break;
        case Phase::BehaviorHigh:
            if (phaseElapsed (now, behaviorHalfCycleMs))
            {
                enterPhase (Phase::BehaviorLow, now);
            }
            break;
        case Phase::BehaviorLow:
            if (phaseElapsed (now, behaviorHalfCycleMs))
            {
                ++completedCycles;
                enterPhase (completedCycles < behaviorCycleCount
                                ? Phase::BehaviorHigh
                                : Phase::Shutdown,
                            now);
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
            case Phase::BehaviorHigh:
                status = led.write (adk::Level::High);
                break;
            case Phase::Separation:
            case Phase::BehaviorLow:
                status = led.write (adk::Level::Low);
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
        led.shutdown ();
        ready = false;
        phase = Phase::Shutdown;
    }

} // namespace
