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

    bool                 acquireJunction  ();
    adk::TrafficInput    observeJunction  (adk::TimePoint now);
    adk::Status          decideJunction   (adk::TimePoint now,
                                           const adk::TrafficInput& input);
    bool                 actuateJunction  (adk::TimePoint now,
                                           const adk::TrafficSnapshot& decision);
    bool                 showSignals      (const adk::TrafficSignals& signals);
    bool                 showCountdown    (adk::TimePoint now,
                                           const adk::TrafficSnapshot& decision);
    adk::SevenSegmentGlyph countdownGlyph (uint8_t seconds);
    bool                 circuitHealthy   ();
    bool                 requestAllRed    ();
    void                 shutdownJunction ();
    void                 haltJunction     ();
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

    if (!status.ok () || !actuateJunction (now, decision))
    {
        haltJunction ();
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
            if (!signals[index]->initialize ().ok ())
            {
                shutdownJunction ();
                return false;
            }
        }

        if (!requestButton.initialize ().ok () ||
            !countdown.initialize            ().ok () ||
            !acquisitionIndicator.initialize ().ok () ||
            !junction.initialize             ().ok ())
        {
            shutdownJunction ();
            return false;
        }

        if (!acquisitionIndicator.on ().ok ())
        {
            shutdownJunction ();
            return false;
        }

        if (!actuateJunction (adk::TimePoint (millis ()), junction.snapshot ()))
        {
            haltJunction ();
            return false;
        }

        return true;
    }

    adk::TrafficInput observeJunction (adk::TimePoint now)
    {
        requestButton.update     (now);
        return adk::TrafficInput (requestButton.pressEvent (), circuitHealthy ());
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
        if (!mainGreen.off      ().ok () ||
            !sideGreen.off      ().ok () ||
            !pedestrianWalk.off ().ok () ||
            !mainYellow.off     ().ok () ||
            !sideYellow.off     ().ok () ||
            !mainRed.on         ().ok () ||
            !sideRed.on         ().ok () ||
            !pedestrianStop.on  ().ok ())
        {
            return false;
        }

        return mainRed.set        (signals.mainRed).ok        () &&
               mainYellow.set     (signals.mainYellow).ok     () &&
               mainGreen.set      (signals.mainGreen).ok      () &&
               sideRed.set        (signals.sideRed).ok        () &&
               sideYellow.set     (signals.sideYellow).ok     () &&
               sideGreen.set      (signals.sideGreen).ok      () &&
               pedestrianStop.set (signals.pedestrianStop).ok () &&
               pedestrianWalk.set (signals.pedestrianWalk).ok ();
    }

    bool showCountdown (adk::TimePoint now, const adk::TrafficSnapshot& decision)
    {
        if (!decision.hasDeadline)
        {
            return countdown.blank ().ok ();
        }

        const uint32_t remainingMs = decision.nextDeadline.milliseconds () -
                                     now.milliseconds ();
        uint32_t       seconds     = (remainingMs + 999U) / 1000U;

        if (seconds > 9U)
        {
            seconds = 9U;
        }

        return countdown.show (countdownGlyph (static_cast<uint8_t> (seconds))).ok ();
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

    bool circuitHealthy ()
    {
        return acquisitionIndicator.initialized () &&
               countdown.initialized             () &&
               requestButton.initialized         () &&
               junction.initialized              () &&
               mainRed.initialized               () &&
               mainYellow.initialized            () &&
               mainGreen.initialized             () &&
               sideRed.initialized               () &&
               sideYellow.initialized            () &&
               sideGreen.initialized             () &&
               pedestrianWalk.initialized        () &&
               pedestrianStop.initialized        ();
    }

    bool requestAllRed ()
    {
        bool complete = true;

        complete = mainGreen.off      ().ok () && complete;
        complete = sideGreen.off      ().ok () && complete;
        complete = pedestrianWalk.off ().ok () && complete;
        complete = mainYellow.off     ().ok () && complete;
        complete = sideYellow.off     ().ok () && complete;
        complete = mainRed.on         ().ok () && complete;
        complete = sideRed.on         ().ok () && complete;
        complete = pedestrianStop.on  ().ok () && complete;

        return complete;
    }

    void shutdownJunction ()
    {
        junction.shutdown               ();
        acquisitionIndicator.shutdown   ();
        countdown.shutdown              ();
        requestButton.shutdown          ();
        pedestrianWalk.shutdown         ();
        pedestrianStop.shutdown         ();
        sideGreen.shutdown              ();
        sideYellow.shutdown             ();
        sideRed.shutdown                ();
        mainGreen.shutdown              ();
        mainYellow.shutdown             ();
        mainRed.shutdown                ();
    }

    void haltJunction ()
    {
        halted = true;
        requestAllRed    ();
        countdown.blank  ();
        shutdownJunction ();
    }
} // namespace
