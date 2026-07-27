#include <Adk.h>

namespace {

    constexpr unsigned long holdMs = 500;

    adk::Runtime       runtime;
    adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);
    bool               ready = false;

    void showLevel (adk::Level level);

} // namespace

void setup ()
{
    ready = led.initialize ().ok ();
}

void loop ()
{
    showLevel (adk::Level::High);
    showLevel (adk::Level::Low);
}

namespace {

    void showLevel (adk::Level level)
    {
        if (!ready)
        {
            return;
        }

        if (!led.write (level).ok ())
        {
            led.shutdown ();
            ready = false;
            return;
        }

        delay (holdMs);
    }

} // namespace
