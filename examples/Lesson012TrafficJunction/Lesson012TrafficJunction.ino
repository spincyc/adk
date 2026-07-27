#include <Adk.h>

// Mega 2560, USB 5 V: D22/D23/D24 drive a 74HC595 and common-cathode
// seven-segment display. D25 is a pull-up request button. D30-D37 drive
// current-limited tabletop signal LEDs. D13 confirms full acquisition.
// This is not a road controller.

namespace {

    const adk::ShiftRegisterPins displayPins = {22, 23, 24};
    const adk::ButtonConfig      requestConfig (25);

    adk::Runtime             runtime;
    adk::MonoLed             acquisitionIndicator (runtime.resources (), 13);
    adk::SevenSegmentDisplay countdown            (runtime.resources (),
                                                   displayPins,
                                                   adk::SevenSegmentPolarity::CommonCathode);
    adk::Button              requestButton         (runtime.resources (), requestConfig);

    adk::MonoLed mainRed        (runtime.resources (), 30);
    adk::MonoLed mainYellow     (runtime.resources (), 31);
    adk::MonoLed mainGreen      (runtime.resources (), 32);
    adk::MonoLed sideRed        (runtime.resources (), 33);
    adk::MonoLed sideYellow     (runtime.resources (), 34);
    adk::MonoLed sideGreen      (runtime.resources (), 35);
    adk::MonoLed pedestrianWalk (runtime.resources (), 36);
    adk::MonoLed pedestrianStop (runtime.resources (), 37);

    adk::TrafficConfig   junctionConfig;
    adk::TrafficJunction junction (junctionConfig);

    bool halted = false;

    bool                 acquireJunction       ();
    adk::TrafficInput    observeJunction       (adk::TimePoint now);
    adk::Status          decideJunction        (adk::TimePoint now,
                                                const adk::TrafficInput& input);
    bool                 actuateJunction       (adk::TimePoint now,
                                                const adk::TrafficSnapshot& decision);
    bool                 showSignals           (const adk::TrafficSignals& signals);
    bool                 showCountdown         (adk::TimePoint now,
                                                const adk::TrafficSnapshot& decision);
    adk::SevenSegmentGlyph countdownGlyph      (uint8_t seconds);
    void                 showAllRed            ();
    void                 stopSafely            ();
} // namespace

void setup ()
{
    halted = !acquireJunction ();
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint       now                             (millis ());
    const adk::TrafficInput    observation = observeJunction   (now);
    const adk::Status          status      = decideJunction    (now, observation);
    const adk::TrafficSnapshot decision    = junction.snapshot ();

    if (status != adk::Status::Ok || !actuateJunction (now, decision))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireJunction ()
    {
        adk::MonoLed* const signals[] =
        {
            &mainRed,
            &mainYellow,
            &mainGreen,
            &sideRed,
            &sideYellow,
            &sideGreen,
            &pedestrianStop,
            &pedestrianWalk
        };

        for (uint8_t index = 0; index < 8; ++index)
        {
            if (signals[index]->initialize () != adk::Status::Ok)
            {
                stopSafely ();
                return false;
            }
        }

        if (requestButton.initialize () != adk::Status::Ok ||
            countdown.initialize            () != adk::Status::Ok ||
            acquisitionIndicator.initialize () != adk::Status::Ok ||
            junction.initialize             () != adk::Status::Ok)
        {
            stopSafely ();
            return false;
        }

        if (acquisitionIndicator.on () != adk::Status::Ok)
        {
            stopSafely ();
            return false;
        }

        return actuateJunction (adk::TimePoint (millis ()), junction.snapshot ());
    }

    adk::TrafficInput observeJunction (adk::TimePoint now)
    {
        requestButton.update     (now);
        return adk::TrafficInput (requestButton.pressEvent (), true);
    }

    adk::Status decideJunction (adk::TimePoint now, const adk::TrafficInput& input)
    {
        return junction.update (now, input);
    }

    bool actuateJunction (adk::TimePoint now, const adk::TrafficSnapshot& decision)
    {
        if (!showSignals (decision.signals))
        {
            return false;
        }

        return showCountdown (now, decision);
    }

    bool showSignals (const adk::TrafficSignals& signals)
    {
        if (mainGreen.off      () != adk::Status::Ok ||
            sideGreen.off      () != adk::Status::Ok ||
            pedestrianWalk.off () != adk::Status::Ok ||
            mainYellow.off     () != adk::Status::Ok ||
            sideYellow.off     () != adk::Status::Ok ||
            mainRed.on         () != adk::Status::Ok ||
            sideRed.on         () != adk::Status::Ok ||
            pedestrianStop.on  () != adk::Status::Ok)
        {
            return false;
        }

        return mainRed.set        (signals.mainRed)        == adk::Status::Ok &&
               mainYellow.set     (signals.mainYellow)     == adk::Status::Ok &&
               mainGreen.set      (signals.mainGreen)      == adk::Status::Ok &&
               sideRed.set        (signals.sideRed)        == adk::Status::Ok &&
               sideYellow.set     (signals.sideYellow)     == adk::Status::Ok &&
               sideGreen.set      (signals.sideGreen)      == adk::Status::Ok &&
               pedestrianStop.set (signals.pedestrianStop) == adk::Status::Ok &&
               pedestrianWalk.set (signals.pedestrianWalk) == adk::Status::Ok;
    }

    bool showCountdown (adk::TimePoint now, const adk::TrafficSnapshot& decision)
    {
        if (!decision.hasDeadline)
        {
            return countdown.blank () == adk::Status::Ok;
        }

        const uint32_t remainingMs = decision.nextDeadline.milliseconds () -
                                     now.milliseconds ();
        uint32_t       seconds     = (remainingMs + 999U) / 1000U;

        if (seconds > 9U)
        {
            seconds = 9U;
        }

        return countdown.show (countdownGlyph (static_cast<uint8_t> (seconds))) ==
               adk::Status::Ok;
    }

    adk::SevenSegmentGlyph countdownGlyph (uint8_t seconds)
    {
        switch (seconds)
        {
            case 0: return adk::SevenSegmentGlyph::Zero;
            case 1: return adk::SevenSegmentGlyph::One;
            case 2: return adk::SevenSegmentGlyph::Two;
            case 3: return adk::SevenSegmentGlyph::Three;
            case 4: return adk::SevenSegmentGlyph::Four;
            case 5: return adk::SevenSegmentGlyph::Five;
            case 6: return adk::SevenSegmentGlyph::Six;
            case 7: return adk::SevenSegmentGlyph::Seven;
            case 8: return adk::SevenSegmentGlyph::Eight;
            case 9: return adk::SevenSegmentGlyph::Nine;
        }

        return adk::SevenSegmentGlyph::Blank;
    }

    void showAllRed ()
    {
        mainGreen.off      ();
        sideGreen.off      ();
        pedestrianWalk.off ();
        mainYellow.off     ();
        sideYellow.off     ();
        mainRed.on         ();
        sideRed.on         ();
        pedestrianStop.on  ();
    }

    void stopSafely ()
    {
        showAllRed      ();
        countdown.blank ();
        halted = true;
    }
} // namespace
