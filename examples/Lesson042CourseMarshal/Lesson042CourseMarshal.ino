// Replay-first E0 course policy: all PIR, button, optical, and range evidence
// is synthetic. Running this sketch adds an E1 presentation circuit: D22-D24
// drive the Lesson 010 74HC595 display fixture; D30-D32, D33, D34, D35, and
// D13 each drive one LED through 1 kOhm to GND. Compilation is not physical
// acceptance. The explicit button-event fixture is the sole start authority.
// PIR Motion evidence establishes eligibility but never starts a run.
#include <Adk.h>

namespace {

    constexpr uint8_t                buttonSourceId = 1;
    constexpr adk::ShiftRegisterPins displayPins    = {22, 23, 24};

    const adk::CourseMarshalConfig marshalConfig = {
        {{{1}, adk::OpticalSourceKind::ReflectiveAnalog, 10, 1},
         {{2}, adk::OpticalSourceKind::InterruptedDigital, 11, 1},
         {{3}, adk::OpticalSourceKind::ReflectiveAnalog, 12, 1},
         {}},
        3,
        buttonSourceId,
        adk::Duration (250),
        adk::Duration (25),
        adk::Duration (100),
        adk::Duration (10000)};

    adk::Runtime runtime;

    adk::SevenSegmentDisplay display (runtime.resources (), displayPins,
                                      adk::SevenSegmentPolarity::CommonCathode);
    adk::MonoLed             checkpointLeds[3] = {{runtime.resources (), 30},
                                                  {runtime.resources (), 31},
                                                  {runtime.resources (), 32}};
    adk::MonoLed             readyLed       (runtime.resources (), 33);
    adk::MonoLed             allRedLed      (runtime.resources (), 34);
    adk::MonoLed             heartbeatLed   (runtime.resources (), 35);
    adk::MonoLed             acquisitionLed (runtime.resources (), 13);

    adk::CourseRunStorage             runStorage;
    adk::CourseTriggerStorage         triggerStorage;
    adk::CourseTriggerPresenceStorage triggerPresenceStorage;
    adk::CourseReplayFrameStorage     replayFrameStorage;
    adk::CourseReplayPresenceStorage  replayPresenceStorage;
    adk::CourseReplayEventStorage     replayEventStorage;

    adk::CourseStartPolicy      startPolicy (buttonSourceId);
    adk::CourseMarshal          marshal     (marshalConfig, runStorage, triggerStorage,
                                         triggerPresenceStorage, replayFrameStorage,
                                         replayPresenceStorage, replayEventStorage);
    adk::CourseMarshalPresenter presenter (adk::Duration (100), adk::Duration (500));

    struct ReplaySchedule
    {
        uint8_t nextStage;

        ReplaySchedule () noexcept : nextStage (0)
        {
        }

        uint8_t advance (adk::Duration elapsed) noexcept
        {
            static const uint32_t stageTimes[] = {250U,  1000U, 2000U,
                                                  3000U, 4000U, 5000U};
            if (nextStage < 6U && elapsed.milliseconds () >= stageTimes[nextStage])
            {
                return nextStage++;
            }
            return 0xFFU;
        }
    };

    ReplaySchedule replay;
    adk::TimePoint replayStartedAt;
    adk::TimePoint lastFrameAt;
    bool           halted               = false;
    bool           hasFrame             = false;
    bool           presentationAcquired = false;

    bool acquirePresentation  ();
    void shutdownPresentation ();
    bool observeReplay        (adk::TimePoint now, adk::CourseMarshalInputView& input,
                        adk::PresenceSnapshot& presence,
                        adk::CheckpointEvent (&events)[4]);
    bool                    decideCourse    (const adk::CourseMarshalInputView& input);
    bool                    presentCourse   (adk::TimePoint now);
    void                    enterSafeFault  ();
    adk::PirPresenceState   eligiblePir     (adk::TimePoint now);
    adk::CheckpointEvent    checkpointEvent (uint8_t slot, adk::TimePoint now);
    adk::SevenSegmentGlyph  displayGlyph    (uint8_t value);

} // namespace

void setup ()
{
    if (!acquirePresentation ())
    {
        enterSafeFault ();
        return;
    }

    if (!startPolicy.initialize ().ok () || !marshal.initialize ().ok () ||
        !presenter.initialize ().ok ())
    {
        enterSafeFault ();
        return;
    }

    // TP-ACQUIRE remains high for 500 ms after every resource is acquired.
    delay                   (500);
    if (!acquisitionLed.off ().ok ())
    {
        enterSafeFault ();
        return;
    }

    replayStartedAt = adk::TimePoint (millis ());
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    if (hasFrame && now.elapsedSince (lastFrameAt).milliseconds () < 20U)
    {
        return;
    }
    lastFrameAt = now;
    hasFrame    = true;

    adk::PresenceSnapshot       presence;
    adk::CheckpointEvent        events[4] = {};
    adk::CourseMarshalInputView input;

    if (!observeReplay (now, input, presence, events) || !decideCourse (input) ||
        !presentCourse (now))
    {
        enterSafeFault ();
    }
}

namespace {

    bool acquirePresentation ()
    {
        for (uint8_t index = 0; index < 3U; ++index)
        {
            if (!checkpointLeds[index].initialize ().ok ())
            {
                shutdownPresentation ();
                return false;
            }
        }

        if (!display.initialize        ().ok () || !readyLed.initialize ().ok () ||
            !allRedLed.initialize      ().ok () ||
            !heartbeatLed.initialize   ().ok () ||
            !acquisitionLed.initialize ().ok ())
        {
            shutdownPresentation ();
            return false;
        }

        if (!acquisitionLed.on ().ok ())
        {
            shutdownPresentation ();
            return false;
        }

        presentationAcquired = true;
        return true;
    }

    void shutdownPresentation ()
    {
        display.shutdown        ();
        readyLed.shutdown       ();
        allRedLed.shutdown      ();
        heartbeatLed.shutdown   ();
        acquisitionLed.shutdown ();
        for (uint8_t index = 0; index < 3U; ++index)
        {
            checkpointLeds[index].shutdown ();
        }
        presentationAcquired = false;
    }

    bool observeReplay (adk::TimePoint now, adk::CourseMarshalInputView& input,
                        adk::PresenceSnapshot& presence,
                        adk::CheckpointEvent (&events)[4])
    {
        presence             = adk::PresenceSnapshot ();
        presence.pir         = eligiblePir           (now);
        presence.pirEligible = true;
        presence.quality     = adk::PresenceQuality::Valid;

        const uint8_t stage = replay.advance (now.elapsedSince (replayStartedAt));
        const bool    explicitButtonPress = stage == 1U;

        if (!startPolicy
                 .update     ({now, buttonSourceId, explicitButtonPress, presence.pir})
                 .ok         ())
        {
            return false;
        }

        input.observedAt = now;
        input.start      = startPolicy.snapshot ();
        input.presence   = &presence;
        input.events     = events;
        input.eventCount = 0;

        if (stage >= 2U && stage <= 4U)
        {
            events[0]        = checkpointEvent (static_cast<uint8_t> (stage - 2U), now);
            input.eventCount = 1;
        }
        else if (stage == 5U)
        {
            presence.finishGuard.available                      = true;
            presence.finishGuard.provenance.sourceId            = 20;
            presence.finishGuard.provenance.calibrationRevision = 1;
            presence.finishGuard.provenance.observedAt          = now;
            presence.finishGuard.quality         = adk::OpticalQuality::Valid;
            presence.finishGuard.valid           = true;
            presence.finishGuard.active          = true;
            presence.finishGuard.activationEvent = true;

            presence.range.available                   = true;
            presence.range.evidence.sourceId           = 30;
            presence.range.evidence.startedAt          = now;
            presence.range.evidence.completedAt        = now;
            presence.range.evidence.reading.state      = adk::RangeState::Valid;
            presence.range.evidence.reading.distanceMm = 180;
            presence.range.evidence.reading.valid      = true;
            presence.range.valid                       = true;
            presence.range.approachValid               = true;
        }
        return true;
    }

    bool decideCourse (const adk::CourseMarshalInputView& input)
    {
        return marshal.update (input).ok ();
    }

    bool presentCourse (adk::TimePoint now)
    {
        if (!presenter.update (now, marshal.snapshot ()).ok ())
        {
            return false;
        }

        const adk::CoursePresentationIntent intent = presenter.intent ();
        for (uint8_t checkpoint = 0; checkpoint < 3U; ++checkpoint)
        {
            if (!checkpointLeds[checkpoint]
                     .set    ((intent.acceptedMask & (1U << checkpoint)) != 0U)
                     .ok     ())
            {
                return false;
            }
        }

        return readyLed.set (intent.phase == adk::CoursePresentationPhase::Ready)
                   .ok () &&
               allRedLed.set    (intent.allRed).ok () &&
               heartbeatLed.set (intent.heartbeat).ok () &&
               display.show     (displayGlyph (intent.displayValue)).ok ();
    }

    void enterSafeFault ()
    {
        halted = true;
        if (!presentationAcquired)
        {
            shutdownPresentation ();
            return;
        }
        display.blank ();
        for (uint8_t index = 0; index < 3U; ++index)
        {
            checkpointLeds[index].off ();
        }
        readyLed.off       ();
        heartbeatLed.off   ();
        allRedLed.on       ();
        acquisitionLed.off ();
    }

    adk::PirPresenceState eligiblePir (adk::TimePoint now)
    {
        adk::PirPresenceState pir;
        pir.available           = true;
        pir.evidence.sourceId   = 40;
        pir.evidence.observedAt = now;
        pir.evidence.rawLevel   = adk::Level::High;
        pir.evidence.phase      = adk::PirPhase::Motion;
        pir.evidence.stableFor  = adk::Duration (1000);
        pir.valid               = true;
        return pir;
    }

    adk::CheckpointEvent checkpointEvent (uint8_t slot, adk::TimePoint now)
    {
        const adk::CheckpointBinding& binding = marshalConfig.orderedCheckpoints[slot];
        return {binding.checkpointId,
                binding.sourceKind,
                {binding.sourceId, binding.calibrationRevision, now},
                adk::OpticalQuality::Valid,
                adk::Status ()};
    }

    adk::SevenSegmentGlyph displayGlyph (uint8_t value)
    {
        if (value <= 15U)
        {
            return static_cast<adk::SevenSegmentGlyph> (value);
        }
        return adk::SevenSegmentGlyph::Dash;
    }

} // namespace
