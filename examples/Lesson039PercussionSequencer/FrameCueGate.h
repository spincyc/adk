#pragma once

#include <stdint.h>

struct FrameCueGate
{
    FrameCueGate () noexcept : step_ (0), observedAtMs_ (0), armed_ (false)
    {
    }

    bool advance (bool frameValid, uint8_t step, uint32_t observedAtMs,
                  uint32_t cycleDurationMs) noexcept
    {
        if (!frameValid)
        {
            armed_ = false;
            return false;
        }

        const bool cycleElapsed =
            armed_ && observedAtMs - observedAtMs_ >= cycleDurationMs;
        if (armed_ && step == step_ && !cycleElapsed)
        {
            return false;
        }

        step_         = step;
        observedAtMs_ = observedAtMs;
        armed_        = true;
        return true;
    }

  private:
    uint8_t  step_;
    uint32_t observedAtMs_;
    bool     armed_;
};
