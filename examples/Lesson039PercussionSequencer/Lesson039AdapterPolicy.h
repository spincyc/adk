#pragma once

#include <stdint.h>

struct Lesson039ReplayEvent
{
    uint8_t  attackMask;
    bool     completion;
    uint32_t completionStartedAtMs;
    uint16_t completionIntensity;
    bool     play;
    bool     clear;
};

enum struct Lesson039AdapterFault : uint8_t
{
    None,
    Acquisition,
    Observation,
    Decision,
    Presentation,
    Sound
};

struct Lesson039EvidenceLatch
{
    explicit Lesson039EvidenceLatch (uint32_t durationMs) noexcept
        : startedAtMs_ (0), durationMs_ (durationMs), active_ (false)
    {
    }

    void trigger (uint32_t nowMs) noexcept
    {
        startedAtMs_ = nowMs;
        active_      = true;
    }

    bool active (uint32_t nowMs) noexcept
    {
        if (active_ && nowMs - startedAtMs_ >= durationMs_)
        {
            active_ = false;
        }
        return active_;
    }

  private:
    uint32_t startedAtMs_;
    uint32_t durationMs_;
    bool     active_;
};

struct Lesson039SampleCadence
{
    explicit Lesson039SampleCadence (uint32_t intervalMs) noexcept
        : intervalMs_ (intervalMs), sampledAtMs_ (0), anchored_ (false)
    {
    }

    bool due (uint32_t nowMs) noexcept
    {
        if (!anchored_ || nowMs - sampledAtMs_ >= intervalMs_)
        {
            sampledAtMs_ = nowMs;
            anchored_    = true;
            return true;
        }

        return false;
    }

  private:
    uint32_t intervalMs_;
    uint32_t sampledAtMs_;
    bool     anchored_;
};

struct Lesson039ReplaySchedule
{
    Lesson039ReplaySchedule () noexcept
        : previousAtMs_ (0), phase_ (0), anchored_ (false)
    {
    }

    Lesson039ReplayEvent advance (uint32_t nowMs) noexcept
    {
        Lesson039ReplayEvent event = {0, false, 0, 0, false, false};
        if (!anchored_)
        {
            previousAtMs_ = nowMs;
            phase_        = nowMs % cycleMs;
            anchored_     = true;
            event.clear   = phase_ == 0;
            return event;
        }

        const uint32_t elapsed       = nowMs - previousAtMs_;
        const uint32_t previousPhase = phase_;
        const uint32_t previousAt    = previousAtMs_;
        previousAtMs_                = nowMs;
        phase_                       = (phase_ + elapsed % cycleMs) % cycleMs;

        static constexpr uint16_t attackThresholds[3]     = {600, 1200, 1800};
        static constexpr uint16_t completionThresholds[3] = {660, 1260, 1860};

        uint32_t latestCompletionDistance = 0;
        for (uint8_t surface = 0; surface < 3; ++surface)
        {
            if (crossed (previousPhase, elapsed, attackThresholds[surface]))
            {
                event.attackMask |= static_cast<uint8_t> (1U << surface);
            }

            const uint32_t firstDistance =
                distanceTo (previousPhase, completionThresholds[surface]);
            if (firstDistance <= elapsed)
            {
                const uint32_t latestDistance =
                    firstDistance + ((elapsed - firstDistance) / cycleMs) * cycleMs;
                if (!event.completion || latestDistance > latestCompletionDistance)
                {
                    latestCompletionDistance    = latestDistance;
                    event.completion            = true;
                    event.completionStartedAtMs = previousAt + latestDistance - 100U;
                    event.completionIntensity   = static_cast<uint16_t> (
                        120U + completionThresholds[surface] / 10U);
                }
            }
        }

        event.play  = crossed (previousPhase, elapsed, 4000);
        event.clear = crossed (previousPhase, elapsed, 0);
        return event;
    }

  private:
    static constexpr uint32_t cycleMs = 8000;

    static uint32_t distanceTo (uint32_t phase, uint32_t threshold) noexcept
    {
        const uint32_t distance = (threshold + cycleMs - phase) % cycleMs;
        return distance == 0 ? cycleMs : distance;
    }

    static bool crossed (uint32_t phase, uint32_t elapsed, uint32_t threshold) noexcept
    {
        return elapsed >= distanceTo (phase, threshold);
    }

    uint32_t previousAtMs_;
    uint32_t phase_;
    bool     anchored_;
};

inline uint16_t lesson039NormalizeAdc (uint16_t sample) noexcept
{
    if (sample > 1023)
    {
        sample = 1023;
    }

    return static_cast<uint16_t> ((static_cast<uint32_t> (sample) * 1000U + 511U) /
                                  1023U);
}
