// Reference fixture: four C&K RB-220-07A R contacts, each from its
// TP-C0..TP-C3 node to GND, with an individual external 10 kOhm pull-up
// from that node to Mega 5 V. TP-C0..TP-C3 connect to D40..D43.
// SparkFun SEN-12642 V10: ENVELOPE -> A8, GATE -> D44, VCC -> 5 V,
// GND -> GND, AUDIO disconnected. A 10 kOhm tempo pot uses A9.
// D45..D48, D52, and D53 each drive an LED through 1 kOhm to GND.
// D49..D51 drive the Lesson 010 74HC595 display fixture. D6 drives an
// identified passive piezo; D38/D39 are pull-up buttons.
// Power the reference module and Mega together. E1 acceptance remains open.
#include <Adk.h>

#include "FrameCueGate.h"
#include "Lesson039AdapterPolicy.h"

#ifndef ADK_LESSON039_USE_REFERENCE_HARDWARE
#define ADK_LESSON039_USE_REFERENCE_HARDWARE 0
#endif

namespace {

    constexpr bool useReferenceHardware = ADK_LESSON039_USE_REFERENCE_HARDWARE != 0;

    const adk::ContactDynamicsConfig contactConfig (adk::Level::Low, adk::Duration (8),
                                                    adk::Duration (12),
                                                    adk::Duration (80),
                                                    adk::Duration (1500));
    const adk::AcousticEnvelopeConfig
        acousticConfig (true, adk::Level::High, 8, 28, 12, 4, adk::Duration (500),
                        adk::Duration (120), adk::Duration (20), adk::Duration (80));
    const adk::PercussionSequencerConfig
        sequencerConfig (16, 60, 180, adk::Duration (20), adk::Duration (180));
    constexpr adk::ShiftRegisterPins displayPins = {49, 50, 51};

    adk::Runtime      runtime;
    adk::DigitalInput contacts[4] = {{runtime.resources (), 40, adk::Pull::None},
                                     {runtime.resources (), 41, adk::Pull::None},
                                     {runtime.resources (), 42, adk::Pull::None},
                                     {runtime.resources (), 43, adk::Pull::None}};
    adk::DigitalInput gate           (runtime.resources (), 44, adk::Pull::None);
    adk::AnalogInput  envelopeInput  (runtime.resources (), 62);
    adk::AnalogInput  tempoInput     (runtime.resources (), 63);
    adk::Button       playButton     (runtime.resources (), {38});
    adk::Button       clearButton    (runtime.resources (), {39});

    adk::MonoLed      surfaceLeds[4] = {{runtime.resources (), 45},
                                        {runtime.resources (), 46},
                                        {runtime.resources (), 47},
                                        {runtime.resources (), 48}};
    adk::SevenSegmentDisplay display (runtime.resources (), displayPins,
                                      adk::SevenSegmentPolarity::CommonCathode);
    adk::MonoLed             statusLed  (runtime.resources (), 52);
    adk::MonoLed             captureLed (runtime.resources (), 53);
    adk::PiezoSounder        sounder    (runtime.resources (), 6);

    adk::ContactDynamics     contact0 (contactConfig);
    adk::ContactDynamics     contact1 (contactConfig);
    adk::ContactDynamics     contact2 (contactConfig);
    adk::ContactDynamics     contact3 (contactConfig);
    adk::ContactDynamics*    contactPolicies[4] = {&contact0, &contact1, &contact2,
                                                   &contact3};
    adk::AcousticEnvelope    acoustic  (acousticConfig);
    adk::PercussionSequencer sequencer (sequencerConfig);
    FrameCueGate             frameCueGate;
    Lesson039ReplaySchedule  replaySchedule;
    Lesson039SampleCadence   sampleCadence (20);
    Lesson039EvidenceLatch   surfaceEvidence[4] = {
        Lesson039EvidenceLatch (180), Lesson039EvidenceLatch (180),
        Lesson039EvidenceLatch (180), Lesson039EvidenceLatch (180)};

    bool                  halted      = false;
    bool                  captureOn   = false;
    Lesson039AdapterFault faultSource = Lesson039AdapterFault::None;

    bool acquirePresentation      ();
    bool acquireSources           ();
    bool observePercussionInputs  (adk::TimePoint                 now,
                                  adk::PercussionSequencerInput& input);
    bool observeReferenceInputs (adk::TimePoint                 now,
                                 adk::PercussionSequencerInput& input);
    void observeReplayInputs   (adk::TimePoint now, adk::PercussionSequencerInput& input);
    bool decidePattern         (const adk::PercussionSequencerInput& input);
    bool presentPlaybackFrame  (adk::TimePoint                          now,
                               const adk::PercussionSequencerSnapshot& snapshot);
    bool soundPlaybackFrame (adk::TimePoint                          now,
                             const adk::PercussionSequencerSnapshot& snapshot);
    void enterSafeFault (Lesson039AdapterFault source);

} // namespace

void setup ()
{
    if ((useReferenceHardware && !acquireSources ()) || !acquirePresentation () ||
        !sequencer.initialize ().ok () || !statusLed.set (true).ok ())
    {
        enterSafeFault (Lesson039AdapterFault::Acquisition);
    }
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (static_cast<adk::TimePoint::Raw> (millis ()));

    if (!sampleCadence.due (now.milliseconds ()))
    {
        return;
    }

    adk::PercussionSequencerInput input;

    if (!observePercussionInputs (now, input))
    {
        enterSafeFault (Lesson039AdapterFault::Observation);
        return;
    }

    if (!decidePattern (input))
    {
        enterSafeFault (Lesson039AdapterFault::Decision);
        return;
    }

    const auto snapshot = sequencer.snapshot ();

    if (!presentPlaybackFrame (now, snapshot))
    {
        enterSafeFault (Lesson039AdapterFault::Presentation);
        return;
    }
    if (!soundPlaybackFrame (now, snapshot))
    {
        enterSafeFault (Lesson039AdapterFault::Sound);
    }
}

namespace {

    bool acquirePresentation ()
    {
        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            if (!surfaceLeds[surface].initialize ().ok ())
            {
                return false;
            }
        }

        return display.initialize ().ok () && statusLed.initialize ().ok () &&
               captureLed.initialize ().ok () && sounder.initialize ().ok ();
    }

    bool acquireSources ()
    {
        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            if (!contacts[surface].initialize ().ok () ||
                !contactPolicies[surface]->initialize ().ok ())
            {
                return false;
            }
        }

        return envelopeInput.initialize ().ok () &&
               gate.initialize          ().ok () &&
               tempoInput.initialize    ().ok () &&
               playButton.initialize    ().ok () &&
               clearButton.initialize   ().ok () &&
               acoustic.initialize      ().ok ();
    }

    bool observePercussionInputs (adk::TimePoint                 now,
                                  adk::PercussionSequencerInput& input)
    {
        if (useReferenceHardware)
        {
            return observeReferenceInputs (now, input);
        }

        observeReplayInputs (now, input);
        return true;
    }

    bool observeReferenceInputs (adk::TimePoint                 now,
                                 adk::PercussionSequencerInput& input)
    {
        captureOn = !captureOn;
        if (!captureLed.set (captureOn).ok ())
        {
            return false;
        }

        input.observedAt = now;
        input.attackMask = 0;

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            contacts[surface].update ();

            const adk::Level contactSample = contacts[surface].read ();

            input.surfaceStatus[surface]   = contactPolicies[surface]->update (
                {now, contactSample, adk::StatusCode::Ok});
            if (contactPolicies[surface]->snapshot ().attackEvent)
            {
                input.attackMask |= static_cast<uint8_t> (1U << surface);
            }
        }

        gate.update          ();
        envelopeInput.update ();
        tempoInput.update    ();
        playButton.update    (now);
        clearButton.update   (now);

        const uint16_t   envelopeSample = envelopeInput.read ();
        const adk::Level gateSample     = gate.read          ();
        const uint16_t   tempoSample    = tempoInput.read    ();
        input.acousticStatus =
            acoustic.update ({now, envelopeSample, true, gateSample,
                              adk::StatusCode::Ok, adk::StatusCode::Ok});
        const auto acousticEvidence = acoustic.snapshot ();
        if (acousticEvidence.eventCompleted)
        {
            input.acousticCompletion.present        = true;
            input.acousticCompletion.eventStartedAt = acousticEvidence.eventStartedAt;
            input.acousticCompletion.eventDuration  = acousticEvidence.eventDuration;
            input.acousticCompletion.intensity = acousticEvidence.relativeIntensity;
        }
        input.tempoPosition = lesson039NormalizeAdc  (tempoSample);
        input.playEvent     = playButton.pressEvent  ();
        input.clearEvent    = clearButton.pressEvent ();
        return true;
    }

    void observeReplayInputs (adk::TimePoint now, adk::PercussionSequencerInput& input)
    {
        const Lesson039ReplayEvent event = replaySchedule.advance (now.milliseconds ());
        input.observedAt                 = now;
        input.tempoPosition              = 500;
        input.attackMask                 = event.attackMask;
        input.playEvent                  = event.play;
        input.clearEvent                 = event.clear;

        if (event.completion)
        {
            input.acousticCompletion.present = true;
            input.acousticCompletion.eventStartedAt =
                adk::TimePoint (event.completionStartedAtMs);
            input.acousticCompletion.eventDuration = adk::Duration (100);
            input.acousticCompletion.intensity     = event.completionIntensity;
        }
    }

    bool decidePattern (const adk::PercussionSequencerInput& input)
    {
        return sequencer.update (input).ok ();
    }

    bool presentPlaybackFrame (adk::TimePoint                          now,
                               const adk::PercussionSequencerSnapshot& snapshot)
    {
        if (!statusLed
                 .set (snapshot.status.ok () &&
                       (!snapshot.frameValid || snapshot.frame.heartbeat))
                 .ok ())
        {
            return false;
        }

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            const bool playbackActive =
                snapshot.frameValid &&
                (snapshot.frame.surfaceMask & (1U << surface)) != 0;
            const bool accepted =
                snapshot.hitAccepted && snapshot.lastHit.surface == surface;
            if (accepted)
            {
                surfaceEvidence[surface].trigger (now.milliseconds ());
            }
            const bool active =
                playbackActive || surfaceEvidence[surface].active (now.milliseconds ());
            if (!surfaceLeds[surface].set (active).ok ())
            {
                return false;
            }
        }

        if (snapshot.frameValid &&
            !display.show (static_cast<adk::SevenSegmentGlyph> (snapshot.frame.step))
                 .ok ())
        {
            return false;
        }
        if (!snapshot.frameValid && !display.blank ().ok ())
        {
            return false;
        }
        return true;
    }

    bool soundPlaybackFrame (adk::TimePoint                          now,
                             const adk::PercussionSequencerSnapshot& snapshot)
    {
        sounder.update (now);
        const uint32_t stepDurationMs =
            snapshot.tempoBpm == 0 ? 0 : 60000UL / snapshot.tempoBpm / 4UL;
        const bool newFrame = frameCueGate.advance (
            snapshot.frameValid, snapshot.frame.step, now.milliseconds (),
            stepDurationMs * sequencerConfig.steps);
        if (newFrame && snapshot.frame.frequencyHz != 0)
        {
            return sounder.play (
                snapshot.frame.frequencyHz, snapshot.frame.toneDuration, now).ok ();
        }
        if (!snapshot.frameValid || (newFrame && snapshot.frame.frequencyHz == 0))
        {
            sounder.stop ();
        }
        return true;
    }

    void enterSafeFault (Lesson039AdapterFault source)
    {
        faultSource = source;
        sounder.shutdown   ();

        captureLed.set      (false);
        captureLed.shutdown ();

        statusLed.set      (false);
        statusLed.shutdown ();

        display.blank      ();
        display.shutdown   ();
        for (uint8_t surface = 4; surface > 0; --surface)
        {
            surfaceLeds[surface - 1].set      (false);
            surfaceLeds[surface - 1].shutdown ();
        }
        clearButton.shutdown   ();
        playButton.shutdown    ();
        tempoInput.shutdown    ();
        gate.shutdown          ();
        envelopeInput.shutdown ();
        for (uint8_t surface = 4; surface > 0; --surface)
        {
            contacts[surface - 1].shutdown ();
        }
        sequencer.shutdown ();
        halted = true;
    }

} // namespace
