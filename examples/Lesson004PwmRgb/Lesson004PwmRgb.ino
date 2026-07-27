#include <Adk.h>

namespace {

    constexpr unsigned long stepIntervalMs = 1000;

    adk::Runtime runtime;
    adk::RgbLed  led (runtime.resources (), {6, 220}, {5, 220}, {3, 220});

    const adk::Rgb colors[]{adk::Rgb (255, 0, 0), adk::Rgb (0, 255, 0),
                            adk::Rgb (0, 0, 255), adk::Rgb (255, 255, 255),
                            adk::Rgb (0, 0, 0),   adk::Rgb (255, 64, 0)};

    constexpr uint8_t colorCount = sizeof (colors) / sizeof (colors[0]);

    unsigned long nextChangeMs = 0;
    uint8_t       colorIndex   = 0;
    bool          ready        = false;

    bool changeDue (unsigned long now);

    void showNextColor ();

} // namespace

void setup ()
{
    ready = led.initialize () == adk::Status::Ok;

    nextChangeMs = millis ();
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    const unsigned long now = millis ();

    if (!changeDue (now))
    {
        return;
    }

    showNextColor ();
}

namespace {

    bool changeDue (unsigned long now)
    {
        return static_cast<int32_t> (now - nextChangeMs) >= 0;
    }

    void showNextColor ()
    {
        if (led.set (colors[colorIndex]) != adk::Status::Ok)
        {
            led.shutdown ();
            ready = false;
            return;
        }

        colorIndex = static_cast<uint8_t> ((colorIndex + 1) % colorCount);
        nextChangeMs += stepIntervalMs;
    }

} // namespace
