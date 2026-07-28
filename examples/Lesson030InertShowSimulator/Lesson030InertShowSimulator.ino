// Mega 2560, USB 5 V only. D22-D29 each drive an LED through 1 kOhm
// to GND. D30-D38 buttons connect to GND. D5-D7 drive a common-cathode
// RGB LED through one 1 kOhm resistor per channel. TP28 is D35; TP29 is
// the selected D22-D29 output measured relative to Mega GND.
#include <Adk.h>

namespace {

    constexpr uint8_t  channelCount       = adk::InertChannelAssessor::capacity;
    constexpr uint8_t  auditCapacity      = 96;
    constexpr uint32_t startupDurationMs  = 2000;
    constexpr uint32_t sessionDurationMs  = 120000;

    const adk::ButtonConfig reviewButtonConfig    (30);
    const adk::ButtonConfig runButtonConfig       (31);
    const adk::ButtonConfig confirmButtonConfig   (32);
    const adk::ButtonConfig skipButtonConfig      (33);
    const adk::ButtonConfig cancelButtonConfig    (34);
    const adk::ButtonConfig channelButtonConfig   (35);
    const adk::ButtonConfig primaryButtonConfig   (36);
    const adk::ButtonConfig redundantButtonConfig (37);
    const adk::ButtonConfig applyButtonConfig     (38);

    adk::Runtime runtime;

    adk::Button reviewButton    (runtime.resources (), reviewButtonConfig);
    adk::Button runButton       (runtime.resources (), runButtonConfig);
    adk::Button confirmButton   (runtime.resources (), confirmButtonConfig);
    adk::Button skipButton      (runtime.resources (), skipButtonConfig);
    adk::Button cancelButton    (runtime.resources (), cancelButtonConfig);
    adk::Button channelButton   (runtime.resources (), channelButtonConfig);
    adk::Button primaryButton   (runtime.resources (), primaryButtonConfig);
    adk::Button redundantButton (runtime.resources (), redundantButtonConfig);
    adk::Button applyButton     (runtime.resources (), applyButtonConfig);

    adk::MonoLed cueLeds[channelCount] = {
        {runtime.resources (), 22}, {runtime.resources (), 23},
        {runtime.resources (), 24}, {runtime.resources (), 25},
        {runtime.resources (), 26}, {runtime.resources (), 27},
        {runtime.resources (), 28}, {runtime.resources (), 29}};

    adk::RgbLed stateLed (runtime.resources (), {5, 1000}, {6, 1000}, {7, 1000});

    adk::InertChannelAssessor assessor (adk::Duration (5000));

    const adk::InertCueSchedulerConfig schedulerConfig = {
        {{{3,  adk::Duration (2000),  adk::Duration (2000)},
          {7,  adk::Duration (7000),  adk::Duration (2000)},
          {12, adk::Duration (12000), adk::Duration (2000)},
          {29, adk::Duration (17000), adk::Duration (2000)}},
         4},
        adk::Duration (3000)};

    adk::CueAuditEntry     auditStorage[auditCapacity];
    adk::CueAuditBuffer    audit     (auditStorage, auditCapacity);
    adk::InertCueScheduler scheduler (schedulerConfig, audit);

    const adk::InertCueChannelMap cueChannelMap = {{5, 2, 7, 0}, 4};

    adk::InertShowSimulator simulator (cueChannelMap, assessor, scheduler, audit);

    adk::InertChannelObservation observations[channelCount];
    adk::InertChannelId          selectedChannel   = 0;
    adk::InertObservation        selectedPrimary   = adk::InertObservation::Closed;
    adk::InertObservation        selectedRedundant = adk::InertObservation::Closed;
    adk::TimePoint               lastSampleAt;
    adk::TimePoint               startedAt;
    bool                         hasLastSample = false;
    bool                         running       = false;

    bool acquireSimulatorPanel  ();
    void configureClosedFrame   (adk::TimePoint now);
    void startSimulatorPanel    (adk::TimePoint now);
    void observeSimulatorInput  (adk::TimePoint now);
    bool decideInertShow        (adk::TimePoint now);
    bool actuateInertCues       ();
    bool actuateSimulatorState  (adk::TimePoint now);
    bool showStartupProgress    (adk::TimePoint now);
    bool setStateColor          (const adk::Rgb& color);
    bool statePulse             (adk::TimePoint now, uint16_t onMilliseconds,
                                 uint16_t periodMilliseconds);
    void stopSafely             ();

} // namespace

void setup ()
{
    const adk::TimePoint now (millis ());

    if (acquireSimulatorPanel ())
    {
        configureClosedFrame (now);
        startSimulatorPanel  (now);
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (now.elapsedSince (startedAt).milliseconds () >= sessionDurationMs)
    {
        stopSafely ();
        return;
    }

    if (now.elapsedSince (startedAt).milliseconds () < startupDurationMs)
    {
        if (!showStartupProgress (now))
        {
            stopSafely ();
        }

        return;
    }

    if (hasLastSample && now == lastSampleAt)
    {
        return;
    }

    lastSampleAt  = now;
    hasLastSample = true;

    observeSimulatorInput (now);

    if (!decideInertShow (now) || !actuateInertCues () || !actuateSimulatorState (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireSimulatorPanel ()
    {
        if (!reviewButton.initialize    ().ok () ||
            !runButton.initialize       ().ok () ||
            !confirmButton.initialize   ().ok () ||
            !skipButton.initialize      ().ok () ||
            !cancelButton.initialize    ().ok () ||
            !channelButton.initialize   ().ok () ||
            !primaryButton.initialize   ().ok () ||
            !redundantButton.initialize ().ok () ||
            !applyButton.initialize     ().ok ())
        {
            stopSafely ();
            return false;
        }

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            if (!cueLeds[channel].initialize ().ok () || !cueLeds[channel].off ().ok ())
            {
                stopSafely ();
                return false;
            }
        }

        if (!stateLed.initialize ().ok () || !stateLed.off ().ok () ||
            !simulator.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    void configureClosedFrame (adk::TimePoint now)
    {
        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            observations[channel] = {channel, adk::InertObservation::Closed,
                                     adk::InertObservation::Closed, now};
        }
    }

    void startSimulatorPanel (adk::TimePoint now)
    {
        startedAt      = now;
        hasLastSample  = false;
        running        = true;
    }

    void observeSimulatorInput (adk::TimePoint now)
    {
        reviewButton.update    (now);
        runButton.update       (now);
        confirmButton.update   (now);
        skipButton.update      (now);
        cancelButton.update    (now);
        channelButton.update   (now);
        primaryButton.update   (now);
        redundantButton.update (now);
        applyButton.update     (now);

        if (channelButton.pressEvent ())
        {
            selectedChannel   = (selectedChannel + 1) % channelCount;
            selectedPrimary   = observations[selectedChannel].primary;
            selectedRedundant = observations[selectedChannel].redundant;
        }

        if (primaryButton.pressEvent ())
        {
            selectedPrimary = static_cast<adk::InertObservation> (
                (static_cast<uint8_t> (selectedPrimary) + 1) % 4);
        }

        if (redundantButton.pressEvent ())
        {
            selectedRedundant = static_cast<adk::InertObservation> (
                (static_cast<uint8_t> (selectedRedundant) + 1) % 4);
        }

        if (applyButton.pressEvent ())
        {
            observations[selectedChannel].primary   = selectedPrimary;
            observations[selectedChannel].redundant = selectedRedundant;

            for (uint8_t channel = 0; channel < channelCount; ++channel)
            {
                observations[channel].observedAt = now;
            }
        }
    }

    bool decideInertShow (adk::TimePoint now)
    {
        const adk::CueOperatorInput operatorInput = {
            reviewButton.pressed     (), runButton.pressEvent     (),
            confirmButton.pressEvent (), skipButton.pressEvent    (),
            cancelButton.pressEvent  ()};
        const adk::InertShowInput input = {observations, channelCount, operatorInput};

        return simulator.update (now, input).ok ();
    }

    bool actuateInertCues ()
    {
        const adk::InertShowSnapshot snapshot = simulator.snapshot ();

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            const bool active =
                snapshot.state == adk::InertShowState::Running &&
                snapshot.schedule.phase == adk::CueSchedulerPhase::Active &&
                snapshot.schedule.hasCue &&
                cueChannelMap.channels[snapshot.schedule.cueIndex] == channel;

            if (!cueLeds[channel].set (active).ok ())
            {
                return false;
            }
        }

        return true;
    }

    bool actuateSimulatorState (adk::TimePoint now)
    {
        const adk::InertShowSnapshot snapshot = simulator.snapshot ();
        const adk::Rgb               off;
        const adk::Rgb               red   (96, 0, 0);
        const adk::Rgb               green (0, 96, 0);
        const adk::Rgb               blue  (0, 0, 96);

        switch (snapshot.state)
        {
            case adk::InertShowState::Startup:
            case adk::InertShowState::Review:
            case adk::InertShowState::Ready:
                return setStateColor (blue);
            case adk::InertShowState::Running:
                return setStateColor (green);
            case adk::InertShowState::Held:
                return setStateColor (
                    statePulse (now, 150, 800) ? blue : off);
            case adk::InertShowState::Complete:
                return setStateColor (
                    statePulse (now, 150, 1000) ? green : off);
            case adk::InertShowState::Cancelled:
            case adk::InertShowState::Fault: return setStateColor (red);
        }

        return false;
    }

    bool showStartupProgress (adk::TimePoint now)
    {
        const uint8_t completedChannels = static_cast<uint8_t> (
            now.elapsedSince (startedAt).milliseconds () / 250U);

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            if (!cueLeds[channel].set (channel <= completedChannels).ok ())
            {
                return false;
            }
        }

        return stateLed.set (adk::Rgb (0, 0, 96)).ok ();
    }

    bool setStateColor (const adk::Rgb& color)
    {
        return stateLed.off ().ok () && stateLed.set (color).ok ();
    }

    bool statePulse (adk::TimePoint now, uint16_t onMilliseconds,
                     uint16_t periodMilliseconds)
    {
        return now.milliseconds () % periodMilliseconds < onMilliseconds;
    }

    void stopSafely ()
    {
        running = false;
        simulator.shutdown ();
        stateLed.shutdown  ();

        for (uint8_t channel = channelCount; channel > 0; --channel)
        {
            cueLeds[channel - 1].shutdown ();
        }

        applyButton.shutdown     ();
        redundantButton.shutdown ();
        primaryButton.shutdown   ();
        channelButton.shutdown   ();
        cancelButton.shutdown    ();
        skipButton.shutdown      ();
        confirmButton.shutdown   ();
        runButton.shutdown       ();
        reviewButton.shutdown    ();
    }
} // namespace
