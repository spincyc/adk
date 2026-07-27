#include "greenhouse_health_pattern.h"

namespace adk {

    namespace {

        bool pulse (uint32_t phase, uint8_t count) noexcept
        {
            const uint32_t pulseWindow = static_cast<uint32_t> (count) * 200U;

            return phase < pulseWindow && phase % 200U < 100U;
        }
    } // namespace

    GreenhouseHealthPattern::GreenhouseHealthPattern (RgbLed& led) noexcept
        : led_ (&led), mode_ (GreenhouseMode::Starting), color_ (), modeSince_ (),
          lastUpdate_ (), initialized_ (false), hasMode_ (false), hasUpdate_ (false)
    {
    }

    GreenhouseHealthPattern::~GreenhouseHealthPattern () noexcept
    {
        shutdown ();
    }

    Status GreenhouseHealthPattern::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Status status = led_->initialize ();

        if (!status.ok ())
        {
            led_->shutdown ();
            return status;
        }

        color_       = Rgb ();
        initialized_ = true;
        hasMode_     = false;
        hasUpdate_   = false;
        return StatusCode::Ok;
    }

    void GreenhouseHealthPattern::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        led_->off      ();
        led_->shutdown ();

        color_       = Rgb ();
        initialized_ = false;
        hasMode_     = false;
        hasUpdate_   = false;
    }

    Status GreenhouseHealthPattern::update (TimePoint now, GreenhouseMode mode) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (hasUpdate_ && now == lastUpdate_ && hasMode_ && mode == mode_)
        {
            return StatusCode::Ok;
        }

        if (!hasMode_ || mode != mode_)
        {
            mode_      = mode;
            modeSince_ = now;
            hasMode_   = true;
        }

        const Rgb next = chooseColor (now.elapsedSince (modeSince_), mode);

        if (next != color_)
        {
            const Status status = led_->set (next);

            if (!status.ok ())
            {
                return status;
            }

            color_ = next;
        }

        lastUpdate_ = now;
        hasUpdate_  = true;
        return StatusCode::Ok;
    }

    bool GreenhouseHealthPattern::initialized () const noexcept
    {
        return initialized_;
    }

    Rgb GreenhouseHealthPattern::chooseColor (Duration       elapsed,
                                              GreenhouseMode mode) const noexcept
    {
        const uint32_t phase = elapsed.milliseconds () % 1000U;

        switch (mode)
        {
            case GreenhouseMode::Starting:
                return elapsed.milliseconds () % 2000U < 1000U
                           ? Rgb (0, 0, 160)
                           : Rgb ();
            case GreenhouseMode::Monitoring:
                return pulse (phase, 1) ? Rgb (0, 160, 0) : Rgb ();
            case GreenhouseMode::Watering:
                return pulse (phase, 1) ? Rgb (0, 128, 128) : Rgb ();
            case GreenhouseMode::Inhibited: return Rgb (160, 80, 0);
            case GreenhouseMode::SensorFault:
                return pulse (phase, 2) ? Rgb (160, 0, 0) : Rgb ();
            case GreenhouseMode::OutputFault: return Rgb (160, 0, 0);
            case GreenhouseMode::DisplayFault:
                return pulse (phase, 3) ? Rgb (160, 80, 0) : Rgb ();
            case GreenhouseMode::RecordFault:
                return pulse (phase, 4) ? Rgb (128, 0, 128) : Rgb ();
            case GreenhouseMode::MultipleFaults:
                return phase < 500U ? Rgb (160, 0, 0) : Rgb (128, 0, 128);
        }

        return Rgb ();
    }
} // namespace adk
