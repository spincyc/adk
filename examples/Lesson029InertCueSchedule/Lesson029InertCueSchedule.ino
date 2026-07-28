// Mega 2560, USB 5 V only. D22-D29 each drive an LED through 1 kOhm
// to GND. D30-D34 buttons connect to GND. D5-D7 drive a common-cathode
// RGB LED through one 1 kOhm resistor per channel. TP29 is the selected
// D22-D29 output measured relative to Mega GND.
#include <Adk.h>

namespace {

    constexpr uint8_t cueLedCount = 8;
    constexpr uint8_t auditCapacity = 64;
    constexpr uint32_t sessionDurationMs = 120000;

    const adk::ButtonConfig reviewButtonConfig  (30);
    const adk::ButtonConfig runButtonConfig     (31);
    const adk::ButtonConfig confirmButtonConfig (32);
    const adk::ButtonConfig skipButtonConfig    (33);
    const adk::ButtonConfig cancelButtonConfig  (34);

    adk::Runtime runtime;

    adk::Button reviewButton  (runtime.resources (), reviewButtonConfig);
    adk::Button runButton     (runtime.resources (), runButtonConfig);
    adk::Button confirmButton (runtime.resources (), confirmButtonConfig);
    adk::Button skipButton    (runtime.resources (), skipButtonConfig);
    adk::Button cancelButton  (runtime.resources (), cancelButtonConfig);

    adk::MonoLed cueLeds[cueLedCount] =
    {
        {runtime.resources (), 22},
        {runtime.resources (), 23},
        {runtime.resources (), 24},
        {runtime.resources (), 25},
        {runtime.resources (), 26},
        {runtime.resources (), 27},
        {runtime.resources (), 28},
        {runtime.resources (), 29}
    };

    adk::RgbLed stateLed (runtime.resources (), {5, 1000}, {6, 1000}, {7, 1000});

    const adk::InertCueSchedulerConfig schedulerConfig =
    {
        {{
            {3,  adk::Duration (2000),  adk::Duration (2000)},
            {7,  adk::Duration (7000),  adk::Duration (2000)},
            {12, adk::Duration (12000), adk::Duration (2000)},
            {29, adk::Duration (17000), adk::Duration (2000)}
        }, 4},
        adk::Duration (3000)
    };

    adk::CueAuditEntry     auditStorage[auditCapacity];
    adk::CueAuditBuffer    audit     (auditStorage, auditCapacity);
    adk::InertCueScheduler scheduler (schedulerConfig, audit);

    adk::TimePoint lastSampleAt;
    adk::TimePoint startedAt;
    bool           hasLastSample = false;
    bool           running = false;

    bool acquireCuePanel   ();
    bool configureCuePanel ();
    void startCuePanel     (adk::TimePoint now);
    void observeOperator   (adk::TimePoint now);
    bool decideCueSchedule (adk::TimePoint now);
    bool presentCueSnapshot
        (adk::TimePoint now);
    bool showCue
        (const adk::CueSchedulerSnapshot& snapshot);
    bool showPhase
        (const adk::CueSchedulerSnapshot& snapshot, adk::TimePoint now);
    bool phasePulse
        (adk::TimePoint now, uint16_t onMilliseconds,
         uint16_t periodMilliseconds);
    void stopSafely        ();

} // namespace

void setup ()
{
    if (acquireCuePanel () && configureCuePanel ())
    {
        startCuePanel (adk::TimePoint (millis ()));
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

    if (hasLastSample && now == lastSampleAt)
    {
        return;
    }

    lastSampleAt = now;
    hasLastSample = true;

    observeOperator (now);

    if (!decideCueSchedule (now) || !presentCueSnapshot (now))
    {
        stopSafely ();
    }
}

namespace {

    bool acquireCuePanel ()
    {
        if (!reviewButton.initialize  ().ok () ||
            !runButton.initialize     ().ok () ||
            !confirmButton.initialize ().ok () ||
            !skipButton.initialize    ().ok () ||
            !cancelButton.initialize  ().ok ())
        {
            stopSafely ();
            return false;
        }

        for (uint8_t index = 0; index < cueLedCount; ++index)
        {
            if (!cueLeds[index].initialize ().ok ())
            {
                stopSafely ();
                return false;
            }
        }

        if (!stateLed.initialize ().ok () || !scheduler.initialize ().ok ())
        {
            stopSafely ();
            return false;
        }

        return true;
    }

    bool configureCuePanel ()
    {
        for (uint8_t index = 0; index < cueLedCount; ++index)
        {
            if (!cueLeds[index].off ().ok ())
            {
                return false;
            }
        }

        return stateLed.off ().ok ();
    }

    void startCuePanel (adk::TimePoint now)
    {
        startedAt = now;
        running = true;
    }

    void observeOperator (adk::TimePoint now)
    {
        reviewButton.update  (now);
        runButton.update     (now);
        confirmButton.update (now);
        skipButton.update    (now);
        cancelButton.update  (now);
    }

    bool decideCueSchedule (adk::TimePoint now)
    {
        const adk::CueOperatorInput input =
        {
            reviewButton.pressed     (),
            runButton.pressEvent     (),
            confirmButton.pressEvent (),
            skipButton.pressEvent    (),
            cancelButton.pressEvent  ()
        };

        return scheduler.update (now, input).ok ();
    }

    bool presentCueSnapshot (adk::TimePoint now)
    {
        const adk::CueSchedulerSnapshot snapshot = scheduler.snapshot ();

        return showCue (snapshot) && showPhase (snapshot, now);
    }

    bool showCue (const adk::CueSchedulerSnapshot& snapshot)
    {
        for (uint8_t index = 0; index < cueLedCount; ++index)
        {
            const bool active = snapshot.phase == adk::CueSchedulerPhase::Active &&
                                snapshot.hasCue && snapshot.cueIndex == index;

            if (!cueLeds[index].set (active).ok ())
            {
                return false;
            }
        }

        return true;
    }

    bool showPhase (const adk::CueSchedulerSnapshot& snapshot,
                    adk::TimePoint now)
    {
        const adk::Rgb off;
        const adk::Rgb red   (96, 0, 0);
        const adk::Rgb green (0, 96, 0);
        const adk::Rgb blue  (0, 0, 96);

        switch (snapshot.phase)
        {
            case adk::CueSchedulerPhase::Idle:
            case adk::CueSchedulerPhase::Review:
                return stateLed.set (blue).ok ();
            case adk::CueSchedulerPhase::Waiting:
                return stateLed.set (phasePulse (now, 150, 1000) ? blue : off).ok ();
            case adk::CueSchedulerPhase::Confirmation:
                return stateLed.set (phasePulse (now, 150, 400) ? red : off).ok ();
            case adk::CueSchedulerPhase::Active:
            case adk::CueSchedulerPhase::Complete:
                return stateLed.set (green).ok ();
            case adk::CueSchedulerPhase::Held:
                return stateLed.set (phasePulse (now, 150, 800) ? blue : off).ok ();
            case adk::CueSchedulerPhase::Cancelled:
            case adk::CueSchedulerPhase::Fault:
                return stateLed.set (red).ok ();
        }

        return false;
    }

    bool phasePulse (adk::TimePoint now, uint16_t onMilliseconds,
                     uint16_t periodMilliseconds)
    {
        return now.milliseconds () % periodMilliseconds < onMilliseconds;
    }

    void stopSafely ()
    {
        running = false;
        scheduler.shutdown ();
        stateLed.shutdown  ();

        for (uint8_t index = cueLedCount; index > 0; --index)
        {
            cueLeds[index - 1].shutdown ();
        }

        cancelButton.shutdown  ();
        skipButton.shutdown    ();
        confirmButton.shutdown ();
        runButton.shutdown     ();
        reviewButton.shutdown  ();
    }
} // namespace
