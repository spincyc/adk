#include <Adk.h>

namespace {

    constexpr uint8_t  buttonPins[]  = {22, 23, 24, 25};
    constexpr uint8_t  ledPins[]     = {30, 31, 32, 33};
    constexpr uint16_t frequencies[] = {262, 330, 392, 523};
    constexpr uint32_t sequenceSeed  = 0x12345678u;

    adk::Runtime runtime;

    const adk::ButtonConfig buttonOneConfig (buttonPins[0]);

    const adk::ButtonConfig buttonTwoConfig (buttonPins[1]);

    const adk::ButtonConfig buttonThreeConfig (buttonPins[2]);

    const adk::ButtonConfig buttonFourConfig (buttonPins[3]);

    adk::Button buttonOne (runtime.resources (), buttonOneConfig);

    adk::Button buttonTwo (runtime.resources (), buttonTwoConfig);

    adk::Button buttonThree (runtime.resources (), buttonThreeConfig);

    adk::Button buttonFour (runtime.resources (), buttonFourConfig);

    adk::Button* buttons[] = {&buttonOne, &buttonTwo, &buttonThree, &buttonFour};

    adk::MonoLed ledOne (runtime.resources (), ledPins[0]);

    adk::MonoLed ledTwo (runtime.resources (), ledPins[1]);

    adk::MonoLed ledThree (runtime.resources (), ledPins[2]);

    adk::MonoLed ledFour (runtime.resources (), ledPins[3]);

    adk::MonoLed* leds[] = {&ledOne, &ledTwo, &ledThree, &ledFour};

    const adk::RgbLedChannel redChannel   = {5, 330};
    const adk::RgbLedChannel greenChannel = {6, 330};
    const adk::RgbLedChannel blueChannel  = {7, 330};
    adk::RgbLed statusLed (runtime.resources (), redChannel, greenChannel, blueChannel);

    adk::PiezoSounder sounder (runtime.resources (), 11);

    adk::XorShift32CueSource source (sequenceSeed);
    adk::SimonConfig         config;
    adk::Simon               game (config, source);

    adk::SimonPhase previousPhase = adk::SimonPhase::Idle;
    adk::CueId      previousCue   = adk::CueId::One;
    bool            cueVisible    = false;
    bool            halted        = false;

    void shutdownHardware ()
    {
        sounder.shutdown ();

        statusLed.shutdown ();

        for (uint8_t index = adk::Simon::cueCount; index > 0; --index)
        {
            leds[index - 1]->shutdown ();
        }

        for (uint8_t index = adk::Simon::cueCount; index > 0; --index)
        {
            buttons[index - 1]->shutdown ();
        }
    }

    bool initializeHardware ()
    {
        bool ready = true;

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            ready = (buttons[index]->initialize () == adk::Status::Ok) && ready;
        }

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            ready = (leds[index]->initialize () == adk::Status::Ok) && ready;
        }

        ready = (statusLed.initialize () == adk::Status::Ok) && ready;

        ready = (sounder.initialize () == adk::Status::Ok) && ready;

        ready = (game.initialize () == adk::Status::Ok) && ready;

        if (!ready)
        {
            shutdownHardware ();
        }

        return ready;
    }

    adk::SimonInput sampleButtons (adk::TimePoint now)
    {
        adk::SimonInput input;

        for (uint8_t index = 0; index < adk::Simon::cueCount; ++index)
        {
            buttons[index]->update (now);

            const uint8_t mask = static_cast<uint8_t> (1u << index);

            if (buttons[index]->pressed ())
            {
                input.activeMask |= mask;
            }

            if (buttons[index]->pressEvent ())
            {
                input.pressedMask |= mask;
            }

            if (buttons[index]->releaseEvent ())
            {
                input.releasedMask |= mask;
            }
        }

        const adk::SimonPhase phase    = game.snapshot ().phase;
        const bool            canStart = phase == adk::SimonPhase::Idle ||
                                         phase == adk::SimonPhase::GameSuccess ||
                                         phase == adk::SimonPhase::GameFailure;

        input.startEvent = canStart && input.pressedMask != 0;
        return input;
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
            ready             = (leds[index]->set (active) == adk::Status::Ok) && ready;
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
                                 config.cueOnDuration, now) == adk::Status::Ok;
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
            return sounder.play (880, adk::Duration (150), now) == adk::Status::Ok;
        }

        if (snapshot.phase == adk::SimonPhase::GameFailure)
        {
            return sounder.play (110, adk::Duration (300), now) == adk::Status::Ok;
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

        return statusLed.set (color) == adk::Status::Ok;
    }
} // namespace

void setup ()
{
    halted = !initializeHardware ();
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    const adk::SimonInput input = sampleButtons (now);

    const adk::Status status = game.update (now, input);

    if (!(status == adk::Status::Ok))
    {
        shutdownHardware ();
        halted = true;
        return;
    }

    const adk::SimonSnapshot snapshot = game.snapshot ();

    const bool ledsReady = applyLeds (snapshot.ledMask);

    const bool soundReady = applySound (now, snapshot);

    const bool statusReady = applyStatusLed (snapshot.phase);

    previousPhase = snapshot.phase;

    if (!(ledsReady && soundReady && statusReady))
    {
        shutdownHardware ();
        halted = true;
    }
}
