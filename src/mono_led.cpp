#include "mono_led.h"

namespace adk {

    MonoLed::MonoLed (ResourceRegistry& resources,
                      PinId            pin,
                      bool             activeHigh) noexcept
        : output_     (resources,
                      pin,
                      activeHigh ? Level::Low : Level::High)
        , activeHigh_ (activeHigh)
        , active_     (false)
    {
    }

    MonoLed::~MonoLed () noexcept
    {
        shutdown ();
    }

    Status MonoLed::initialize () noexcept
    {
        Status status = output_.initialize ();

        if (status == Status::Ok)
        {
            active_ = false;
        }

        return status;
    }

    void MonoLed::shutdown () noexcept
    {
        if (output_.initialized ())
        {
            off              ();
            output_.shutdown ();
        }

        active_ = false;
    }

    Status MonoLed::set (bool active) noexcept
    {
        Status status = output_.write (physicalLevel (active));

        if (status == Status::Ok)
        {
            active_ = active;
        }

        return status;
    }

    Status MonoLed::on () noexcept
    {
        return set (true);
    }

    Status MonoLed::off () noexcept
    {
        return set (false);
    }

    bool MonoLed::active () const noexcept
    {
        return active_;
    }

    bool MonoLed::activeHigh () const noexcept
    {
        return activeHigh_;
    }

    bool MonoLed::initialized () const noexcept
    {
        return output_.initialized ();
    }

    PinId MonoLed::pin () const noexcept
    {
        return output_.pin ();
    }

    Level MonoLed::physicalLevel (bool active) const noexcept
    {
        return active == activeHigh_ ? Level::High : Level::Low;
    }
}
