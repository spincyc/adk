// Mega 2560, USB 5 V: 74HC595 on D22/D23/D24, common-cathode display.
#include <Adk.h>

namespace {

    constexpr adk::ShiftRegisterPins displayPins = {22, 23, 24};
    constexpr adk::PinId            diagnosticPin = 13;
    constexpr uint32_t              countIntervalMs = 1000;

    adk::Runtime             runtime;
    adk::SevenSegmentDisplay display (runtime.resources (),
                                      displayPins,
                                      adk::SevenSegmentPolarity::CommonCathode);
    adk::MonoLed             diagnosticLed (runtime.resources (), diagnosticPin);

    adk::TimePoint lastCount;
    uint8_t        count = 0;
    bool           ready = false;

    bool acquireCircuit ();

    bool showReady ();

    bool countIsDue (adk::TimePoint now);

    adk::SevenSegmentGlyph chooseDigit ();

    void showDigit (adk::SevenSegmentGlyph digit);

    void stopSafely ();

} // namespace

void setup ()
{
    ready = acquireCircuit () && showReady ();

    if (!ready)
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

    const adk::TimePoint now (millis ());

    if (!countIsDue (now))
    {
        return;
    }

    const adk::SevenSegmentGlyph digit = chooseDigit ();

    showDigit                             (digit);
}

namespace {

    bool acquireCircuit ()
    {
        if (diagnosticLed.initialize () != adk::Status::Ok)
        {
            return false;
        }

        if (display.initialize () != adk::Status::Ok)
        {
            diagnosticLed.shutdown ();
            return false;
        }

        return display.show (adk::SevenSegmentGlyph::Zero) == adk::Status::Ok;
    }

    bool showReady ()
    {
        return diagnosticLed.on () == adk::Status::Ok;
    }

    bool countIsDue (adk::TimePoint now)
    {
        if (now.elapsedSince (lastCount).milliseconds () < countIntervalMs)
        {
            return false;
        }

        lastCount = now;
        return true;
    }

    adk::SevenSegmentGlyph chooseDigit ()
    {
        count = static_cast<uint8_t> ((count + 1U) % 10U);
        return static_cast<adk::SevenSegmentGlyph> (count);
    }

    void showDigit (adk::SevenSegmentGlyph digit)
    {
        if (display.show (digit) != adk::Status::Ok)
        {
            stopSafely ();
        }
    }

    void stopSafely ()
    {
        display      .shutdown ();
        diagnosticLed.shutdown ();
        ready = false;
    }

} // namespace
