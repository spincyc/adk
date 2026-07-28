// Mega 2560, USB 5 V only. Before power is applied, identify the encoder
// module's A/CLK, B/DT, SW, GND, and rated supply terminals from its markings.
// Do not infer pin order from board colour or a catalog alias. After confirming
// logic levels stay within 0-5 V and SW is confirmed to close to GND, connect
// A to D24, B to D25, SW to D26, and the inspected supply and GND terminals
// to their rated rail and Mega GND. D27-D30 each drive an LED through 1 kOhm
// to GND. D5-D7 drive a common-cathode RGB LED through one 1 kOhm resistor
// per channel. TP-A is D24
// and TP-B is D25, each measured relative to Mega GND.
#include <Adk.h>
#include <quadrature_encoder.h>

namespace {

    constexpr uint8_t  positionLedCount            = 4;
    constexpr uint16_t readyPulseMilliseconds      = 500;
    constexpr uint16_t transitionPulseMilliseconds = 120;

    enum struct TransitionEvidence : uint8_t
    {
        Idle,
        Valid,
        Invalid
    };

    const adk::QuadratureEncoderConfig encoderConfig     (24, 25);
    const adk::ButtonConfig            resetButtonConfig (26);

    adk::Runtime runtime;

    adk::QuadratureEncoder encoder     (runtime.resources (), encoderConfig);
    adk::Button            resetButton (runtime.resources (), resetButtonConfig);

    adk::MonoLed positionLeds[positionLedCount] = {{runtime.resources (), 27},
                                                   {runtime.resources (), 28},
                                                   {runtime.resources (), 29},
                                                   {runtime.resources (), 30}};

    adk::RgbLed stateLed (runtime.resources (), {5, 1000}, {6, 1000}, {7, 1000});

    adk::QuadratureEncoderSnapshot observedEncoder;
    uint16_t                       previousInvalidTransitions = 0;
    uint32_t                       readyStartedAt             = 0;
    uint32_t                       transitionStartedAt        = 0;
    TransitionEvidence             transitionEvidence = TransitionEvidence::Idle;
    bool                           running            = false;

    bool acquireEncoderPanel   ();
    bool configureEncoderPanel ();
    void startEncoderPanel     ();
    bool observeEncoder        (adk::TimePoint now);
    void decidePosition        (adk::TimePoint now);
    bool actuateEvidence       (adk::TimePoint now);
    bool showPositionNibble    ();
    bool showTransitionState   (adk::TimePoint now);
    void stopSafely            ();

} // namespace

void setup ()
{
    if (acquireEncoderPanel () && configureEncoderPanel ())
    {
        startEncoderPanel ();
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (!observeEncoder (now))
    {
        stopSafely ();
        return;
    }

    decidePosition (now);

    if (!actuateEvidence (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireEncoderPanel ()
    {
        if (!encoder.initialize ().ok () || !resetButton.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        for (uint8_t index = 0; index < positionLedCount; ++index)
        {
            if (!positionLeds[index].initialize ().ok ())
            {
                stopSafely ();
                return false;
            }
        }

        if (!stateLed.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    bool configureEncoderPanel ()
    {
        for (uint8_t index = 0; index < positionLedCount; ++index)
        {
            if (!positionLeds[index].off ().ok ())
            {
                return false;
            }
        }

        return stateLed.set (adk::Rgb (0, 0, 96)).ok ();
    }

    void startEncoderPanel ()
    {
        observedEncoder            = encoder.snapshot ();
        previousInvalidTransitions = observedEncoder.invalidTransitions;
        readyStartedAt             = millis ();
        running                    = true;
    }

    bool observeEncoder (adk::TimePoint now)
    {
        if (!encoder.update ().ok ())
        {
            return false;
        }

        resetButton.update                 (now);
        observedEncoder = encoder.snapshot ();
        return observedEncoder.status.ok   ();
    }

    void decidePosition (adk::TimePoint now)
    {
        if (resetButton.pressEvent ())
        {
            encoder.resetPosition              ();
            observedEncoder = encoder.snapshot ();
        }

        if (observedEncoder.invalidTransitions != previousInvalidTransitions)
        {
            transitionEvidence  = TransitionEvidence::Invalid;
            transitionStartedAt = now.milliseconds ();
        }
        else if (observedEncoder.delta != 0)
        {
            transitionEvidence  = TransitionEvidence::Valid;
            transitionStartedAt = now.milliseconds ();
        }
    }

    bool actuateEvidence (adk::TimePoint now)
    {
        const bool shown = showPositionNibble () && showTransitionState (now);

        previousInvalidTransitions = observedEncoder.invalidTransitions;
        return shown;
    }

    bool showPositionNibble ()
    {
        const uint8_t nibble = static_cast<uint8_t> (
            static_cast<uint32_t> (observedEncoder.position) & 0x0fU);

        for (uint8_t index = 0; index < positionLedCount; ++index)
        {
            if (!positionLeds[index].set ((nibble & (1U << index)) != 0).ok ())
            {
                return false;
            }
        }

        return true;
    }

    bool showTransitionState (adk::TimePoint now)
    {
        const adk::Rgb blue  (0, 0, 96);
        const adk::Rgb red   (96, 0, 0);
        const adk::Rgb green (0, 96, 0);
        const adk::Rgb amber (80, 32, 0);

        if (now.milliseconds () - readyStartedAt < readyPulseMilliseconds)
        {
            return stateLed.set (blue).ok ();
        }

        if (now.milliseconds () - transitionStartedAt >= transitionPulseMilliseconds)
        {
            transitionEvidence = TransitionEvidence::Idle;
        }

        if (transitionEvidence == TransitionEvidence::Invalid)
        {
            return stateLed.set (red).ok ();
        }

        if (transitionEvidence == TransitionEvidence::Valid)
        {
            return stateLed.set (green).ok ();
        }

        return stateLed.set (amber).ok ();
    }

    void stopSafely ()
    {
        running = false;
        stateLed.set (adk::Rgb (96, 0, 0));

        for (uint8_t index = 0; index < positionLedCount; ++index)
        {
            positionLeds[index].off ();
        }

        encoder.shutdown     ();
        resetButton.shutdown ();
    }

} // namespace
