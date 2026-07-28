// Mega 2560, USB 5 V only. D22-D29 each drive an LED through 1 kOhm to
// GND. Buttons D30-D33 connect to GND. TP28 is D30; TP29 is D22-D29.
#include <Adk.h>
#include <inert_channel_assessor.h>

namespace {

    constexpr uint8_t  channelCount      = adk::InertChannelAssessor::capacity;
    constexpr uint32_t startupDurationMs = 2000;
    constexpr uint32_t rewardDurationMs  = 1500;
    constexpr uint32_t sessionDurationMs = 120000;

    const adk::ButtonConfig channelButtonConfig   (30);
    const adk::ButtonConfig primaryButtonConfig   (31);
    const adk::ButtonConfig redundantButtonConfig (32);
    const adk::ButtonConfig applyButtonConfig     (33);

    adk::Runtime runtime;

    adk::Button channelButton   (runtime.resources (), channelButtonConfig);
    adk::Button primaryButton   (runtime.resources (), primaryButtonConfig);
    adk::Button redundantButton (runtime.resources (), redundantButtonConfig);
    adk::Button applyButton     (runtime.resources (), applyButtonConfig);

    adk::MonoLed channelLeds[channelCount] = {
        {runtime.resources (), 22}, {runtime.resources (), 23},
        {runtime.resources (), 24}, {runtime.resources (), 25},
        {runtime.resources (), 26}, {runtime.resources (), 27},
        {runtime.resources (), 28}, {runtime.resources (), 29}};

    adk::InertChannelAssessor assessor (adk::Duration (5000));

    adk::InertChannelObservation observations[channelCount];
    adk::InertChannelId          selectedChannel   = 0;
    adk::InertObservation        selectedPrimary   = adk::InertObservation::Open;
    adk::InertObservation        selectedRedundant = adk::InertObservation::Open;
    adk::TimePoint               startedAt;
    adk::TimePoint               rewardStartedAt;
    uint8_t                      completedStages = 0;
    bool                         running         = false;

    bool acquireAssessmentPanel ();
    void configureOpenChannels  (adk::TimePoint now);
    void startAssessmentPanel   (adk::TimePoint now);
    void observeOperator        (adk::TimePoint now);
    bool decideAssessments      (adk::TimePoint now);
    bool showAssessments        (adk::TimePoint now);
    bool showStartup            (adk::TimePoint now);
    bool showReward             (adk::TimePoint now);
    bool showChannelState       (uint8_t channel, adk::InertChannelState state,
                                 adk::TimePoint now);
    bool patternActive          (adk::InertChannelState state, uint32_t phase);
    bool stageComplete          ();
    void stopSafely             ();

} // namespace

void setup ()
{
    const adk::TimePoint now (millis ());

    if (acquireAssessmentPanel ())
    {
        configureOpenChannels (now);
        startAssessmentPanel  (now);
    }
}

void loop ()
{
    const adk::TimePoint now (millis ());

    if (!running)
    {
        return;
    }

    if (now.elapsedSince (startedAt).milliseconds () >= sessionDurationMs)
    {
        stopSafely ();
        return;
    }

    if (now.elapsedSince (startedAt).milliseconds () < startupDurationMs)
    {
        if (!showStartup (now))
        {
            stopSafely ();
        }

        return;
    }

    observeOperator (now);

    if (!decideAssessments (now) || !showAssessments (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireAssessmentPanel ()
    {
        if (!assessor.initialize ().ok () || !channelButton.initialize ().ok () ||
            !primaryButton.initialize   ().ok () ||
            !redundantButton.initialize ().ok () || !applyButton.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            if (!channelLeds[channel].initialize ().ok ())
            {
                stopSafely ();
                return false;
            }
        }

        return true;
    }

    void configureOpenChannels (adk::TimePoint now)
    {
        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            observations[channel] = {channel, adk::InertObservation::Open,
                                     adk::InertObservation::Open, now};
        }
    }

    void startAssessmentPanel (adk::TimePoint now)
    {
        const adk::Status status = assessor.update (now, observations, channelCount);

        if (!status.ok ())
        {
            stopSafely ();
            return;
        }

        startedAt       = now;
        rewardStartedAt = now;
        completedStages = 0;
        running         = true;
    }

    void observeOperator (adk::TimePoint now)
    {
        channelButton.update   (now);
        primaryButton.update   (now);
        redundantButton.update (now);
        applyButton.update     (now);

        if (channelButton.pressEvent ())
        {
            selectedChannel = (selectedChannel + 1) % channelCount;
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
            observations[selectedChannel] = {selectedChannel, selectedPrimary,
                                             selectedRedundant, now};

            if (stageComplete ())
            {
                ++completedStages;
                rewardStartedAt = now;
            }
        }
    }

    bool decideAssessments (adk::TimePoint now)
    {
        if (!applyButton.pressEvent ())
        {
            return true;
        }

        return assessor.update (now, observations, channelCount).ok ();
    }

    bool showAssessments (adk::TimePoint now)
    {
        if (completedStages > 0 &&
            now.elapsedSince (rewardStartedAt).milliseconds () < rewardDurationMs)
        {
            return showReward (now);
        }

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            const adk::Result<adk::InertChannelAssessment> result =
                assessor.assessment (channel, now);

            if (!result.ok () ||
                !showChannelState (channel, result.value ().state, now))
            {
                return false;
            }
        }

        return true;
    }

    bool showStartup (adk::TimePoint now)
    {
        const uint8_t activeChannel =
            static_cast<uint8_t> (now.elapsedSince (startedAt).milliseconds () / 250U) %
            channelCount;

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            if (!channelLeds[channel].set (channel == activeChannel).ok ())
            {
                return false;
            }
        }

        return true;
    }

    bool showReward (adk::TimePoint now)
    {
        const uint8_t visibleStages =
            completedStages < 3 ? completedStages : channelCount;
        const bool pulse =
            (now.elapsedSince (rewardStartedAt).milliseconds () / 150U) % 2U == 0U;

        for (uint8_t channel = 0; channel < channelCount; ++channel)
        {
            const bool active = completedStages < 3 ? channel < visibleStages : pulse;

            if (!channelLeds[channel].set (active).ok ())
            {
                return false;
            }
        }

        return true;
    }

    bool showChannelState (uint8_t channel, adk::InertChannelState state,
                           adk::TimePoint now)
    {
        const uint32_t phase = now.milliseconds () % 2000;

        return channelLeds[channel].set (patternActive (state, phase)).ok ();
    }

    bool patternActive (adk::InertChannelState state, uint32_t phase)
    {
        switch (state)
        {
            case adk::InertChannelState::Open: return false;
            case adk::InertChannelState::Closed: return true;
            case adk::InertChannelState::ShortSimulated: return phase < 1000;
            case adk::InertChannelState::Stale: return phase % 1000 < 200;
            case adk::InertChannelState::Contradictory: return phase % 400 < 200;
            case adk::InertChannelState::Unavailable:
                return phase < 120 || (phase >= 240 && phase < 360);
        }

        return false;
    }

    bool stageComplete ()
    {
        if (selectedChannel != completedStages)
        {
            return false;
        }

        switch (completedStages)
        {
            case 0:
                return selectedPrimary == adk::InertObservation::Closed &&
                       selectedRedundant == adk::InertObservation::Closed;
            case 1:
                return selectedPrimary == adk::InertObservation::Open &&
                       selectedRedundant == adk::InertObservation::Closed;
            case 2:
                return selectedPrimary == adk::InertObservation::Unavailable &&
                       selectedRedundant == adk::InertObservation::Closed;
        }

        return false;
    }

    void stopSafely ()
    {
        running = false;

        for (uint8_t channel = channelCount; channel > 0; --channel)
        {
            channelLeds[channel - 1].shutdown ();
        }

        applyButton.shutdown     ();
        redundantButton.shutdown ();
        primaryButton.shutdown   ();
        channelButton.shutdown   ();
        assessor.shutdown        ();
    }
} // namespace
