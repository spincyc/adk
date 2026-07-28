#define private public
#include <acoustic_envelope.h>
#undef private

#include <cstdlib>
#include <iostream>
#include <type_traits>

// clang-format off
namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::AcousticEnvelopeConfig config (
        bool hasThreshold = false,
        adk::Level thresholdActiveLevel = adk::Level::Low)
    {
        return adk::AcousticEnvelopeConfig (
            hasThreshold, thresholdActiveLevel, 10, 100, 20, 2,
            adk::Duration (10), adk::Duration (20), adk::Duration (5),

            adk::Duration (10));
    }

    adk::AcousticSample sample (
        uint32_t milliseconds, uint16_t raw,
        bool hasThreshold = false,
        adk::Level thresholdLevel = adk::Level::Low,
        adk::Status analogStatus = adk::StatusCode::Ok,
        adk::Status thresholdStatus = adk::StatusCode::Ok)
    {
        return {adk::TimePoint (milliseconds), raw, hasThreshold, thresholdLevel,
                analogStatus, thresholdStatus};
    }

    bool sameObservation (const adk::AcousticObservation& left,
                          const adk::AcousticObservation& right)
    {
        return left.observedAt == right.observedAt && left.raw == right.raw &&
               left.baseline == right.baseline &&
               left.amplitude == right.amplitude &&
               left.peakAmplitude == right.peakAmplitude &&
               left.relativeIntensity == right.relativeIntensity &&
               left.rawThresholdActive == right.rawThresholdActive &&
               left.eventStarted == right.eventStarted &&
               left.eventCompleted == right.eventCompleted &&
               left.eventStartedAt == right.eventStartedAt &&
               left.eventDuration == right.eventDuration &&
               left.phase == right.phase && left.quality == right.quality &&
               left.status == right.status;
    }

    void calibrate (adk::AcousticEnvelope& envelope, uint32_t start = 0,
                    uint16_t raw = 500)
    {
        require (envelope.initialize ().ok (), "initialize");

        require (envelope.update (sample (start, raw)).ok (), "first calibration");

        require (envelope.update (sample (start + 10, raw)).ok (),
                 "calibration boundary");
        require (envelope.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "quiet after calibration boundary");
    }

    void testLifecycleAndClosedConfiguration ()
    {
        adk::AcousticEnvelope envelope (config ());

        require (!envelope.initialized (), "construction inert");

        require (envelope.update (sample (0, 500)).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejected");
        require (envelope.snapshot ().status.error () ==
                     adk::StatusCode::NotInitialized,
                 "rejected update does not mutate snapshot");
        require (envelope.initialize ().ok (), "valid initialize");

        require (envelope.initialize ().ok (), "initialize idempotent");

        envelope.update (sample (0, 500));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Calibrating &&

                     envelope.snapshot ().baseline == 500 &&

                     envelope.snapshot ().amplitude == 0,
                 "first healthy sample starts calibration");

        envelope.reset ();

        require (envelope.snapshot ().phase == adk::AcousticPhase::Calibrating &&

                     envelope.snapshot ().quality ==
                         adk::AcousticQuality::Unqualified &&
                     envelope.snapshot ().status.ok (),
                 "reset restores initialized calibration state");

        const adk::AcousticEnvelopeConfig invalid[] = {
            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 0, 100, 20, 2, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 512, 100, 20, 2, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 10, 0, 0, 2, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 10, 100, 100, 2, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 10, 100, 20, 0, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 10, 100, 20, 9, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 10, 100, 20, 2, adk::Duration (),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (
                false, adk::Level::Low, 10, 100, 20, 2,
                adk::Duration (0x80000000UL), adk::Duration (20),

                adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::Low, 10, 100, 20, 2, adk::Duration (10),

                adk::Duration (4), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (

                false, adk::Level::High, 10, 100, 20, 2, adk::Duration (10),

                adk::Duration (20), adk::Duration (5), adk::Duration (10)),

            adk::AcousticEnvelopeConfig (
                true, static_cast<adk::Level> (2), 10, 100, 20, 2,
                adk::Duration (10), adk::Duration (20), adk::Duration (5),

                adk::Duration (10))};

        for (const auto& value : invalid)
        {
            adk::AcousticEnvelope rejected (value);

            require (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidArgument,
                     "closed invalid configuration rejected");
            require (!rejected.initialized () &&

                         rejected.snapshot ().status.error () ==
                             adk::StatusCode::NotInitialized,
                     "invalid configuration leaves state unchanged");
        }

        static_assert (
            !std::is_copy_constructible<adk::AcousticEnvelope>::value, "");
        static_assert (!std::is_copy_assignable<adk::AcousticEnvelope>::value, "");

        static_assert (
            !std::is_move_constructible<adk::AcousticEnvelope>::value, "");
        static_assert (!std::is_move_assignable<adk::AcousticEnvelope>::value, "");
    }

    void testCalibrationBaselineAndRailFaults ()
    {
        adk::AcousticEnvelope envelope (config ());

        require (envelope.initialize ().ok (), "calibration initialize");

        envelope.update (sample (0, 10));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Fault &&

                     envelope.snapshot ().quality ==
                         adk::AcousticQuality::ClippedLow,
                 "low rail is inclusive and does not start calibration");

        envelope.reset ();

        envelope.update (sample (1, 1013));

        require (envelope.snapshot ().quality ==
                     adk::AcousticQuality::ClippedHigh,
                 "high rail is inclusive");

        envelope.reset ();

        envelope.update (sample (0, 500));

        envelope.update (sample (4, 504));

        require (envelope.snapshot ().baseline == 501 &&

                     envelope.snapshot ().amplitude == 3,
                 "positive integer baseline step");
        envelope.update (sample (9, 497));

        require (envelope.snapshot ().baseline == 500,
                 "negative division truncates toward zero");
        envelope.update (sample (10, 500));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "calibration includes exact boundary sample");

        envelope.reset ();

        envelope.update (sample (0, 500));

        envelope.update (sample (1, 1013));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Fault,
                 "calibration clipping latches fault");
        envelope.update (sample (2, 500));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Fault,
                 "fault cannot silently recover");
    }

    void testEventCloseIntensityAndRefractory ()
    {
        adk::AcousticEnvelope envelope (config ());

        calibrate (envelope);

        envelope.update (sample (11, 600));

        require (envelope.snapshot ().eventStarted &&

                     envelope.snapshot ().phase ==
                         adk::AcousticPhase::EventOpen &&
                     envelope.snapshot ().peakAmplitude == 100,
                 "attack opens at exact threshold");

        envelope.update (sample (12, 800));

        require (!envelope.snapshot ().eventStarted &&

                     envelope.snapshot ().peakAmplitude == 300,
                 "event flag clears and peak retained");

        envelope.update (sample (13, 510));

        envelope.update (sample (17, 510));

        require (!envelope.snapshot ().eventCompleted,
                 "quiet close waits one tick before boundary");
        envelope.update (sample (18, 510));

        const auto complete = envelope.snapshot ();

        require (complete.phase == adk::AcousticPhase::Refractory &&
                     complete.quality == adk::AcousticQuality::ValidEvent &&
                     complete.status.ok () && complete.eventCompleted &&

                     complete.eventStartedAt == adk::TimePoint (11) &&

                     complete.eventDuration == adk::Duration (7),
                 "quiet close publishes eligible completion tuple");

        const uint16_t headroom = 513;
        const uint16_t expected = static_cast<uint16_t> (
            (static_cast<uint32_t> (200) * 1000U) /
            static_cast<uint16_t> (headroom - 100));
        require (complete.relativeIntensity == expected,
                 "intensity uses exact widened integer mapping");

        envelope.update (sample (27, 700));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Refractory &&

                     !envelope.snapshot ().eventCompleted &&

                     envelope.snapshot ().quality ==
                         adk::AcousticQuality::ValidQuiet &&
                     envelope.snapshot ().peakAmplitude == 0 &&
                     envelope.snapshot ().relativeIntensity == 0 &&

                     envelope.snapshot ().eventStartedAt == adk::TimePoint () &&

                     envelope.snapshot ().eventDuration == adk::Duration (),
                 "attack suppressed one tick before refractory boundary");
        envelope.update (sample (28, 700));

        require (envelope.snapshot ().phase == adk::AcousticPhase::EventOpen &&

                     envelope.snapshot ().eventStarted,
                 "attack accepted at exact refractory boundary");

        adk::AcousticEnvelope timeout (config ());

        calibrate (timeout);

        timeout.update (sample (11, 650));

        timeout.update (sample (31, 650));

        require (timeout.snapshot ().eventCompleted &&

                     timeout.snapshot ().eventDuration == adk::Duration (20),
                 "maximum window closes at exact boundary");
    }

    void testThresholdDisagreementAndSources ()
    {
        adk::AcousticEnvelope envelope (config (true, adk::Level::High));

        require (envelope.initialize ().ok (), "threshold initialize");

        envelope.update (sample (0, 500, true, adk::Level::Low));

        envelope.update (sample (10, 500, true, adk::Level::Low));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "threshold calibration");

        envelope.update (sample (11, 500, true, adk::Level::High));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Quiet &&

                     envelope.snapshot ().rawThresholdActive,
                 "single disagreement visible but not fault");
        envelope.update (sample (15, 500, true, adk::Level::High));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "disagreement waits before duration");
        envelope.update (sample (16, 500, true, adk::Level::High));

        require (envelope.snapshot ().phase == adk::AcousticPhase::Fault &&

                     envelope.snapshot ().quality ==
                         adk::AcousticQuality::ThresholdDisagreement &&
                     envelope.snapshot ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                 "sustained disagreement faults");

        adk::AcousticEnvelope analogFailure (config ());

        calibrate (analogFailure);

        analogFailure.update (

            sample (11, 500, false, adk::Level::Low,
                    adk::StatusCode::ResourceBusy));
        require (analogFailure.snapshot ().quality ==
                     adk::AcousticQuality::SourceFault &&
                     analogFailure.snapshot ().status.error () ==
                         adk::StatusCode::ResourceBusy,
                 "analog source status propagates exactly");

        adk::AcousticEnvelope precedence (config ());

        calibrate (precedence);

        precedence.update (

            sample (11, 1024, false, adk::Level::Low,
                    adk::StatusCode::ResourceBusy));

        require (precedence.snapshot ().quality ==
                     adk::AcousticQuality::SourceFault &&
                     precedence.snapshot ().status.error () ==
                         adk::StatusCode::ResourceBusy,
                 "source failure precedes invalid raw interpretation");

        adk::AcousticEnvelope thresholdFailure (

            config (true, adk::Level::High));

        thresholdFailure.initialize ();

        thresholdFailure.update (sample (0, 500, true, adk::Level::Low));

        thresholdFailure.update (sample (10, 500, true, adk::Level::Low));

        thresholdFailure.update (

            sample (11, 500, true, adk::Level::Low,
                    adk::StatusCode::Ok, adk::StatusCode::Unsupported));
        require (thresholdFailure.snapshot ().quality ==
                     adk::AcousticQuality::SourceFault &&
                     thresholdFailure.snapshot ().status.error () ==
                         adk::StatusCode::Unsupported,
                 "threshold source status propagates exactly");
    }

    void testTimeRulesAndRuntimeValidation ()
    {
        adk::AcousticEnvelope envelope (config ());

        calibrate (envelope);

        const auto before = envelope.snapshot ();

        require (envelope.update (sample (10, 500)).ok (),
                 "identical same-time sample idempotent");
        require (envelope.snapshot ().baseline == before.baseline,
                 "same-time replay leaves state stable");
        require (envelope.update (sample (10, 501)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed same-time sample faults");
        require (envelope.snapshot ().quality ==
                     adk::AcousticQuality::TimingFault,
                 "changed same-time quality");

        adk::AcousticEnvelope half (config ());

        calibrate (half);

        half.update (sample (0x8000000AUL, 500));

        require (half.snapshot ().quality == adk::AcousticQuality::TimingFault,
                 "exact half-range faults");

        adk::AcousticEnvelope rollover (config ());

        require (rollover.initialize ().ok (), "rollover initialize");

        rollover.update (sample (0xFFFFFFF0UL, 500));

        rollover.update (sample (0xFFFFFFFAUL, 500));

        rollover.update (sample (4, 600));

        require (rollover.snapshot ().phase == adk::AcousticPhase::EventOpen,
                 "natural rollover remains valid");

        adk::AcousticEnvelope rawRange (config ());

        require (rawRange.initialize ().ok (), "raw-range initialize");

        rawRange.update (sample (0, 1024));

        require (rawRange.snapshot ().phase == adk::AcousticPhase::Fault &&

                     rawRange.snapshot ().quality ==
                         adk::AcousticQuality::Unqualified,
                 "out-of-range ADC sample rejected before interpretation");

        const auto edgeConfig = adk::AcousticEnvelopeConfig (

            false, adk::Level::Low, 10, 600, 20, 1, adk::Duration (1),

            adk::Duration (2), adk::Duration (1), adk::Duration (1));

        adk::AcousticEnvelope headroom (edgeConfig);

        require (headroom.initialize ().ok (), "closed edge config initializes");

        headroom.update (sample (0, 500));

        headroom.update (sample (1, 500));

        require (headroom.snapshot ().phase == adk::AcousticPhase::Fault &&

                     headroom.snapshot ().quality ==
                         adk::AcousticQuality::Unqualified,
                 "insufficient runtime headroom faults");

        adk::AcousticEnvelope drifting (edgeConfig);

        require (drifting.initialize ().ok (), "drift initialize");

        drifting.update (sample (0, 100));

        drifting.update (sample (1, 100));

        require (drifting.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "off-center baseline has initial headroom");

        drifting.update (sample (2, 500));

        drifting.update (sample (3, 500));

        drifting.update (sample (4, 500));

        require (drifting.snapshot ().phase == adk::AcousticPhase::Fault &&
                     drifting.snapshot ().quality ==
                         adk::AcousticQuality::Unqualified &&
                     drifting.snapshot ().status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "quiet baseline drift rechecks runtime headroom");
    }

    void testExcursionParityAndIntensityEndpoints ()
    {
        adk::AcousticEnvelope positive (config ());
        adk::AcousticEnvelope negative (config ());

        calibrate (positive);

        calibrate (negative);

        positive.update (sample (11, 650));

        negative.update (sample (11, 350));

        require (positive.snapshot ().amplitude == 150 &&
                     negative.snapshot ().amplitude == 150 &&
                     positive.snapshot ().peakAmplitude ==
                         negative.snapshot ().peakAmplitude,
                 "equal positive and negative excursions have equal amplitude");

        positive.update (sample (12, 500));

        negative.update (sample (12, 500));

        positive.update (sample (17, 500));

        negative.update (sample (17, 500));

        require (positive.snapshot ().eventCompleted &&
                     negative.snapshot ().eventCompleted &&
                     positive.snapshot ().relativeIntensity ==
                         negative.snapshot ().relativeIntensity,
                 "equal excursions map to equal completed intensity");

        adk::AcousticEnvelope endpoints (config ());

        calibrate (endpoints);

        endpoints.snapshot_.peakAmplitude = 100;

        require (endpoints.completedIntensity () == 0,
                 "attack boundary clamps to zero intensity");

        endpoints.snapshot_.peakAmplitude = 1013;

        require (endpoints.completedIntensity () == 1000,
                 "headroom overflow clamps to maximum intensity");
    }

    void testThresholdRecoveryPresenceAndFaultClearing ()
    {
        adk::AcousticEnvelope chatter (config (true, adk::Level::High));

        require (chatter.initialize ().ok (), "chatter initialize");

        chatter.update (sample (0, 500, true, adk::Level::Low));

        chatter.update (sample (10, 500, true, adk::Level::Low));

        chatter.update (sample (11, 500, true, adk::Level::High));

        chatter.update (sample (12, 500, true, adk::Level::Low));

        chatter.update (sample (15, 500, true, adk::Level::High));

        chatter.update (sample (19, 500, true, adk::Level::High));

        require (chatter.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "agreement reset prevents separated mismatch accumulation");

        chatter.update (sample (20, 500, true, adk::Level::Low));

        chatter.update (sample (24, 500, true, adk::Level::High));

        chatter.update (sample (25, 500, true, adk::Level::Low));

        require (chatter.snapshot ().phase == adk::AcousticPhase::Quiet &&
                     chatter.snapshot ().quality ==
                         adk::AcousticQuality::ValidQuiet,
                 "comparator chatter recovers on each agreement");

        adk::AcousticEnvelope missing (config (true, adk::Level::High));

        require (missing.initialize ().ok (), "presence initialize");

        missing.update (sample (0, 500));

        require (missing.snapshot ().phase == adk::AcousticPhase::Fault &&
                     missing.snapshot ().quality ==
                         adk::AcousticQuality::SourceFault &&
                     missing.snapshot ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                 "configured threshold presence mismatch faults");

        missing.reset ();

        missing.update (sample (1, 500, true, adk::Level::Low));

        missing.update (sample (11, 500, true, adk::Level::Low));

        require (missing.snapshot ().phase == adk::AcousticPhase::Quiet,
                 "explicit reset permits complete recalibration recovery");

        adk::AcousticEnvelope sourceAfterEvent (config ());

        calibrate (sourceAfterEvent);

        sourceAfterEvent.update (sample (11, 650));

        sourceAfterEvent.update (sample (12, 500));

        sourceAfterEvent.update (sample (17, 500));

        require (sourceAfterEvent.snapshot ().relativeIntensity != 0,
                 "completed event establishes retained evidence");

        sourceAfterEvent.update (

            sample (18, 500, false, adk::Level::Low,
                    adk::StatusCode::ResourceBusy));

        require (sourceAfterEvent.snapshot ().quality ==
                     adk::AcousticQuality::SourceFault &&
                     sourceAfterEvent.snapshot ().peakAmplitude == 0 &&
                     sourceAfterEvent.snapshot ().relativeIntensity == 0 &&

                     sourceAfterEvent.snapshot ().eventStartedAt ==
                         adk::TimePoint () &&

                     sourceAfterEvent.snapshot ().eventDuration ==
                         adk::Duration (),
                 "source fault clears completed public evidence");

        adk::AcousticEnvelope clippingAfterEvent (config ());

        calibrate (clippingAfterEvent);

        clippingAfterEvent.update (sample (11, 650));

        clippingAfterEvent.update (sample (12, 500));

        clippingAfterEvent.update (sample (17, 500));

        clippingAfterEvent.update (sample (18, 10));

        require (clippingAfterEvent.snapshot ().quality ==
                     adk::AcousticQuality::ClippedLow &&
                     clippingAfterEvent.snapshot ().peakAmplitude == 0 &&
                     clippingAfterEvent.snapshot ().relativeIntensity == 0,
                 "clipping clears completed public evidence");
    }

    void testBackwardTimeAndDeterministicReplay ()
    {
        adk::AcousticEnvelope backward (config ());

        calibrate (backward);

        backward.update (sample (9, 500));

        require (backward.snapshot ().phase == adk::AcousticPhase::Fault &&
                     backward.snapshot ().quality ==
                         adk::AcousticQuality::TimingFault &&
                     backward.snapshot ().status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "ordinary backward time faults distinctly from half range");

        adk::AcousticEnvelope first (config (true, adk::Level::High));

        adk::AcousticEnvelope second (config (true, adk::Level::High));
        const adk::AcousticSample trace[] = {
            sample (0, 500, true, adk::Level::Low),
            sample (5, 504, true, adk::Level::Low),
            sample (10, 500, true, adk::Level::Low),
            sample (11, 650, true, adk::Level::High),
            sample (12, 700, true, adk::Level::High),
            sample (13, 505, true, adk::Level::Low),
            sample (18, 505, true, adk::Level::Low),
            sample (28, 500, true, adk::Level::Low)};

        require (first.initialize ().ok () && second.initialize ().ok (),
                 "replay pair initializes");

        for (const auto& frame : trace)
        {
            require (first.update (frame) == second.update (frame) &&
                         sameObservation (first.snapshot (), second.snapshot ()),
                     "identical trace produces byte-equivalent observation fields");
        }
    }
} // namespace

int main ()
{
    testLifecycleAndClosedConfiguration ();

    testCalibrationBaselineAndRailFaults ();

    testEventCloseIntensityAndRefractory ();

    testThresholdDisagreementAndSources ();

    testTimeRulesAndRuntimeValidation ();

    testExcursionParityAndIntensityEndpoints ();

    testThresholdRecoveryPresenceAndFaultClearing ();

    testBackwardTimeAndDeterministicReplay ();
}
// clang-format on
