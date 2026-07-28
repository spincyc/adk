#include <Adk.h>

#ifndef ADK_LESSON006_FIXED_REPLAY
#define ADK_LESSON006_FIXED_REPLAY 0
#endif

#if ADK_LESSON006_FIXED_REPLAY != 0 && ADK_LESSON006_FIXED_REPLAY != 1
#error "ADK_LESSON006_FIXED_REPLAY must be 0 or 1"
#endif

namespace {

    constexpr uint8_t  buttonPins[]  = {22, 23, 24, 25};
    constexpr uint8_t  ledPins[]     = {30, 31, 32, 33};
    constexpr uint16_t frequencies[] = {262, 330, 392, 523};
    constexpr uint32_t sequenceSeed  = 0x12345678u;
    constexpr uint32_t acquisitionPulseMilliseconds = 250u;
    constexpr uint32_t behaviorStartMilliseconds = 1000u;
    constexpr uint32_t runDurationMilliseconds = 120000u;
    constexpr uint32_t inactiveSettleMilliseconds = 250u;

#if ADK_LESSON006_FIXED_REPLAY
    constexpr adk::CueId publishedReplayCues[] =
    {
        adk::CueId::One,
        adk::CueId::Three
    };
#endif

#if ADK_LESSON006_FIXED_REPLAY
    adk::SimonConfig makePublishedReplayConfig ()
    {
        adk::SimonConfig config;

        config.cueOnDuration  = adk::Duration (100);
        config.cueGapDuration = adk::Duration (50);
        config.inputTimeout   = adk::Duration (500);
        config.resultDuration = adk::Duration (100);
        config.startingLength = 1;
        config.growthPerRound = 1;
        config.maximumLength  = 2;

        return config;
    }
#endif

    adk::Runtime runtime;

    const adk::ButtonConfig buttonOneConfig (buttonPins[0]);

    const adk::ButtonConfig buttonTwoConfig (buttonPins[1]);

    const adk::ButtonConfig buttonThreeConfig (buttonPins[2]);

    const adk::ButtonConfig buttonFourConfig (buttonPins[3]);

    adk::Button buttonOne (runtime.resources (), buttonOneConfig);

    adk::Button buttonTwo (runtime.resources (), buttonTwoConfig);

    adk::Button buttonThree (runtime.resources (), buttonThreeConfig);

    adk::Button buttonFour (runtime.resources (), buttonFourConfig);

    adk::Button* cueButtons[] = {&buttonOne, &buttonTwo, &buttonThree, &buttonFour};

    adk::MonoLed ledOne (runtime.resources (), ledPins[0]);

    adk::MonoLed ledTwo (runtime.resources (), ledPins[1]);

    adk::MonoLed ledThree (runtime.resources (), ledPins[2]);

    adk::MonoLed ledFour (runtime.resources (), ledPins[3]);

    adk::MonoLed* cueLeds[] = {&ledOne, &ledTwo, &ledThree, &ledFour};

    adk::MonoLed acquisitionLed (runtime.resources (), LED_BUILTIN);

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusLed (runtime.resources (), redChannel, greenChannel, blueChannel);

    adk::PiezoSounder sounder (runtime.resources (), 11);

#if ADK_LESSON006_FIXED_REPLAY
    adk::FixedCueSource cueSource (publishedReplayCues, 2);

    const adk::SimonConfig gameConfig = makePublishedReplayConfig ();
#else
    adk::XorShift32CueSource cueSource (sequenceSeed);
    adk::SimonConfig         gameConfig;
#endif

    adk::Simon simon (gameConfig, cueSource);

    adk::SimonPhase previousPhase = adk::SimonPhase::Idle;
    adk::CueId      previousCue   = adk::CueId::One;
    bool            cueVisible    = false;
    bool            halted        = false;
    bool            acquisitionPulseComplete = false;
    bool            shutdownPending = false;
    uint32_t        runStarted      = 0;
    uint32_t        inactiveStarted = 0;

    bool initializeHardware ();

    adk::SimonInput observePlayer (adk::TimePoint now);

    bool updateStartup (adk::TimePoint now);

    bool updateShutdown (adk::TimePoint now);

    void commandInactive ();

    adk::Status decideGame (adk::TimePoint now, const adk::SimonInput& input);

    bool presentGame (adk::TimePoint now, const adk::SimonSnapshot& snapshot);

    void stopSafely ();
} // namespace

void setup ()
{
    halted = !initializeHardware ();

    if (!halted)
    {
        runStarted = millis ();
    }
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (updateStartup (now))
    {
        return;
    }

    if (updateShutdown (now))
    {
        return;
    }

    const adk::SimonInput observation = observePlayer (now);

    const adk::Status decision = decideGame (now, observation);

    if (!decision.ok ())
    {
        stopSafely ();
        return;
    }

    const adk::SimonSnapshot snapshot = simon.snapshot ();

    if (!presentGame (now, snapshot))
    {
        stopSafely ();
    }
}

namespace {

    void shutdownHardware ()
    {
        sounder.shutdown ();

        statusLed.shutdown ();

        acquisitionLed.shutdown ();

        for (uint8_t index = adk::Simon::cueCount; index > 0; --index)
        {
            cueLeds[index - 1]->shutdown ();
        }

        for (uint8_t index = adk::Simon::cueCount; index > 0; --index)
        {
            cueButtons[index - 1]->shutdown ();
        }
    }

    bool initializeHardware ()
    {
        bool ready = true;

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            ready = cueButtons[index]->initialize ().ok () && ready;
        }

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            ready = cueLeds[index]->initialize ().ok () && ready;
        }

        ready = acquisitionLed.initialize ().ok () && ready;

        ready = statusLed.initialize ().ok () && ready;

        ready = sounder.initialize ().ok () && ready;

        ready = simon.initialize ().ok () && ready;

        if (!ready)
        {
            shutdownHardware ();
            return false;
        }

        // D13 is resource-acquisition evidence only; game behavior never drives it.
        if (!acquisitionLed.set (true).ok ())
        {
            shutdownHardware ();
            return false;
        }

        return true;
    }

    adk::SimonInput observePlayer (adk::TimePoint now)
    {
        adk::SimonInput input;

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            cueButtons[index]->update (now);

            const uint8_t mask = static_cast<uint8_t> (1u << index);

            if (cueButtons[index]->pressed ())
            {
                input.activeMask |= mask;
            }

            if (cueButtons[index]->pressEvent ())
            {
                input.pressedMask |= mask;
            }

            if (cueButtons[index]->releaseEvent ())
            {
                input.releasedMask |= mask;
            }
        }

        const adk::SimonPhase phase    = simon.snapshot ().phase;
        const bool            canStart = phase == adk::SimonPhase::Idle ||
                                         phase == adk::SimonPhase::GameSuccess ||
                                         phase == adk::SimonPhase::GameFailure;

        input.startEvent = canStart && input.pressedMask != 0;
        return input;
    }

    bool updateStartup (adk::TimePoint now)
    {
        const uint32_t elapsed =
            static_cast<uint32_t> (now.milliseconds () - runStarted);

        if (!acquisitionPulseComplete &&
            elapsed >= acquisitionPulseMilliseconds)
        {
            if (!acquisitionLed.set (false).ok ())
            {
                stopSafely ();
                return true;
            }

            acquisitionPulseComplete = true;
        }

        return elapsed < behaviorStartMilliseconds;
    }

    void commandInactive ()
    {
        sounder.stop       ();
        statusLed.set      (adk::Rgb ());
        acquisitionLed.set (false);

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            cueLeds[index]->set (false);
        }
    }

    bool updateShutdown (adk::TimePoint now)
    {
        if (!shutdownPending)
        {
            if (static_cast<uint32_t> (now.milliseconds () - runStarted) <
                runDurationMilliseconds)
            {
                return false;
            }

            commandInactive ();
            shutdownPending = true;
            inactiveStarted = now.milliseconds ();
            return true;
        }

        if (static_cast<uint32_t> (now.milliseconds () - inactiveStarted) <
            inactiveSettleMilliseconds)
        {
            return true;
        }

        stopSafely ();
        return true;
    }

    adk::Status decideGame (adk::TimePoint now, const adk::SimonInput& input)
    {
        return simon.update (now, input);
    }

    adk::PiezoSounder::Frequency frequencyFor (adk::CueId cue)
    {
        const uint8_t index = static_cast<uint8_t> (cue);
        return frequencies[index];
    }

    bool applyLeds (uint8_t mask)
    {
        bool ready = true;

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            const bool active = (mask & static_cast<uint8_t> (1u << index)) != 0;
            ready = cueLeds[index]->set (active).ok () && ready;
        }

        return ready;
    }

    bool applySound (adk::TimePoint now, const adk::SimonSnapshot& snapshot)
    {
        sounder.update (now);

        const bool newCue = snapshot.hasDisplayedCue &&
                            (!cueVisible || snapshot.displayedCue != previousCue);

        if (newCue)
        {
            previousCue = snapshot.displayedCue;
            cueVisible  = true;
            return sounder.play (frequencyFor (snapshot.displayedCue),
                                 gameConfig.cueOnDuration,
                                 now).ok ();
        }

        if (!snapshot.hasDisplayedCue)
        {
            cueVisible = false;
        }

        if (snapshot.phase == previousPhase)
        {
            return true;
        }

        if (snapshot.phase == adk::SimonPhase::RoundSuccess ||
            snapshot.phase == adk::SimonPhase::GameSuccess)
        {
            return sounder.play (880, adk::Duration (150), now).ok ();
        }

        if (snapshot.phase == adk::SimonPhase::GameFailure)
        {
            return sounder.play (110, adk::Duration (300), now).ok ();
        }

        return true;
    }

    bool applyStatusLed (adk::SimonPhase phase)
    {
        if (phase == previousPhase)
        {
            return true;
        }

        adk::Rgb color;

        switch (phase)
        {
            case adk::SimonPhase::PlaybackOn:
            case adk::SimonPhase::PlaybackGap: color = adk::Rgb (0, 0, 96); break;

            case adk::SimonPhase::AwaitPress:
            case adk::SimonPhase::AwaitRelease: color = adk::Rgb (96, 32, 0); break;

            case adk::SimonPhase::RoundSuccess:
            case adk::SimonPhase::GameSuccess: color = adk::Rgb (0, 96, 0); break;

            case adk::SimonPhase::GameFailure: color = adk::Rgb (96, 0, 0); break;

            case adk::SimonPhase::Idle: break;
        }

        return statusLed.set (color).ok ();
    }

    bool presentGame (adk::TimePoint now, const adk::SimonSnapshot& snapshot)
    {
        // Cue LEDs expose playback; RGB exposes phase without Serial.
        const bool ledsReady = applyLeds (snapshot.ledMask);

        const bool soundReady = applySound (now, snapshot);

        const bool statusReady = applyStatusLed (snapshot.phase);

        previousPhase = snapshot.phase;
        return ledsReady && soundReady && statusReady;
    }

    void stopSafely ()
    {
        shutdownHardware ();
        halted = true;
    }
} // namespace
