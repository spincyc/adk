// Mega 2560, USB 5 V: 74HC595 on D22/D23/D24, common-cathode display.
#include <Adk.h>

namespace {

    constexpr adk::ShiftRegisterPins displayPins = {22, 23, 24};
    constexpr adk::PinId            diagnosticPin = 13;
    constexpr uint32_t              countIntervalMs = 1000;
    constexpr uint32_t              acquisitionPulseMs = 250;
    constexpr uint32_t              separatorMs = 750;
    constexpr uint32_t              runLimitMs = 120000;
    constexpr uint32_t              inactiveEvidenceMs = 250;

    adk::Runtime             runtime;
    adk::SevenSegmentDisplay display (runtime.resources (),
                                      displayPins,
                                      adk::SevenSegmentPolarity::CommonCathode);
    adk::MonoLed             diagnosticLed (runtime.resources (), diagnosticPin);

    adk::TimePoint acquiredAt;
    adk::TimePoint lastCount;
    adk::TimePoint inactiveAt;
    uint8_t        count = 0;
    bool           ready = false;
    bool           acquisitionPulseOn = false;
    bool           counting = false;
    bool           stopping = false;

    bool acquireCircuit ();

    bool showReady ();

    bool acquisitionIsComplete (adk::TimePoint now);

    bool countIsDue (adk::TimePoint now);

    adk::SevenSegmentGlyph chooseDigit ();

    void showDigit (adk::SevenSegmentGlyph digit);

    bool finishIfDue (adk::TimePoint now);

    void stopSafely ();

} // namespace

void setup ()
{
    ready = acquireCircuit () && showReady ();

    if (!ready)
    {
        stopSafely ();
        return;
    }

    acquiredAt = adk::TimePoint (millis ());
    lastCount  = acquiredAt;
}

void loop ()
{
    if (!ready)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (finishIfDue (now))
    {
        return;
    }

    if (!acquisitionIsComplete (now))
    {
        return;
    }

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
        if (!diagnosticLed.initialize ().ok ())
        {
            return false;
        }

        if (!display.initialize ().ok ())
        {
            diagnosticLed.shutdown ();
            return false;
        }

        return display.show (adk::SevenSegmentGlyph::Zero).ok ();
    }

    bool showReady ()
    {
        if (!diagnosticLed.on ().ok ())
        {
            return false;
        }

        acquisitionPulseOn = true;
        return true;
    }

    bool acquisitionIsComplete (adk::TimePoint now)
    {
        const uint32_t elapsedMs = now.elapsedSince (acquiredAt).milliseconds ();

        if (acquisitionPulseOn && elapsedMs >= acquisitionPulseMs)
        {
            if (!diagnosticLed.off ().ok ())
            {
                stopSafely ();
                return false;
            }

            acquisitionPulseOn = false;
        }

        if (!counting && elapsedMs >= acquisitionPulseMs + separatorMs)
        {
            counting  = true;
            lastCount = now;
        }

        return counting;
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
        if (!display.show (digit).ok ())
        {
            stopSafely ();
        }
    }

    bool finishIfDue (adk::TimePoint now)
    {
        if (!stopping
            && now.elapsedSince (acquiredAt).milliseconds () >= runLimitMs)
        {
            if (!display.blank ().ok () || !diagnosticLed.off ().ok ())
            {
                stopSafely ();
                return true;
            }

            inactiveAt = now;
            stopping   = true;
        }

        if (stopping
            && now.elapsedSince (inactiveAt).milliseconds ()
                   >= inactiveEvidenceMs)
        {
            stopSafely ();
        }

        return stopping;
    }

    void stopSafely ()
    {
        display      .shutdown ();
        diagnosticLed.shutdown ();
        ready              = false;
        acquisitionPulseOn = false;
        counting           = false;
        stopping           = false;
    }

} // namespace
