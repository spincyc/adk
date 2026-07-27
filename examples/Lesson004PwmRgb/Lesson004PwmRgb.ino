#include <Adk.h>

namespace {

    constexpr unsigned long StepIntervalMs = 1000;

    adk::Runtime runtime;
    adk::RgbLed  led (runtime.resources (), {6, 220}, {5, 220}, {3, 220});

    const adk::Rgb colors[]{adk::Rgb (255, 0, 0), adk::Rgb (0, 255, 0),
                            adk::Rgb (0, 0, 255), adk::Rgb (255, 255, 255),
                            adk::Rgb (0, 0, 0),   adk::Rgb (255, 64, 0)};

    constexpr uint8_t ColorCount = sizeof (colors) / sizeof (colors[0]);

    unsigned long nextChangeMs = 0;
    uint8_t       colorIndex   = 0;
    bool          ready        = false;

} // namespace

void setup ()
{
    const adk::Status status = led.initialize ();

    if (status != adk::Status::Ok)
    {
        return;
    }

    ready        = true;
    nextChangeMs = millis ();
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    const unsigned long now = millis ();

    if (static_cast<int32_t> (now - nextChangeMs) < 0)
    {
        return;
    }

    if (led.set (colors[colorIndex]) != adk::Status::Ok)
    {
        led.shutdown ();
        ready = false;
        return;
    }

    colorIndex = static_cast<uint8_t> ((colorIndex + 1) % ColorCount);
    nextChangeMs += StepIntervalMs;
}
