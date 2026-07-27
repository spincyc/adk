#include "pulse_capture.h"

#include <assert.h>
#include <stdint.h>
#include <type_traits>

namespace adk {

    struct PulseCaptureTestAccess
    {
        static void setNextSequence (PulseCapture& capture, uint32_t sequence) noexcept
        {
            capture.nextSequence_ = sequence;
        }
    };
}

namespace {

    struct FakeCaptureIo : adk::PulseCaptureIo
    {
        adk::Status start (adk::PinId pin, adk::PulseCapture& capture) noexcept override
        {
            ++startCalls;
            startedPin = pin;
            target     = &capture;
            if (!startStatus.ok ())
            {
                return startStatus;
            }
            started = true;
            return adk::StatusCode::Ok;
        }

        void stop () noexcept override
        {
            ++stopCalls;
            started = false;
            target  = nullptr;
        }

        bool inputHigh () const noexcept override
        {
            return high;
        }

        adk::MicrosecondTimePoint now () const noexcept override
        {
            return time;
        }

        void lock () noexcept override
        {
            ++lockCalls;
        }

        void unlock () noexcept override
        {
            ++unlockCalls;
        }

        void edge (uint32_t microseconds, bool inputHigh)
        {
            time = adk::MicrosecondTimePoint (microseconds);
            high = inputHigh;
            target->recordActiveLowEdge (time, high);
        }

        adk::Status               startStatus = adk::StatusCode::Ok;
        adk::MicrosecondTimePoint time        = adk::MicrosecondTimePoint ();
        adk::PulseCapture*        target      = nullptr;
        adk::PinId                startedPin  = 0;
        uint16_t                  startCalls  = 0;
        uint16_t                  stopCalls   = 0;
        uint16_t                  lockCalls   = 0;
        uint16_t                  unlockCalls = 0;
        bool                      high        = true;
        bool                      started     = false;
    };

    const adk::PulseCaptureConfig validConfig = {adk::MicrosecondDuration (10000),
                                                 adk::MicrosecondDuration (100),
                                                 adk::MicrosecondDuration (9000)};

    void armAfterGap (adk::PulseCapture& capture, uint32_t at = 10000)
    {
        assert (capture.update (adk::MicrosecondTimePoint (at)).ok ());
    }

    void testConfigurationAndCapabilityValidation ()
    {
        const adk::PulseCaptureConfig invalid[] = {
            {adk::MicrosecondDuration (1000), adk::MicrosecondDuration (0),
             adk::MicrosecondDuration (500)},
            {adk::MicrosecondDuration (1000), adk::MicrosecondDuration (600),
             adk::MicrosecondDuration (500)},
            {adk::MicrosecondDuration (1000), adk::MicrosecondDuration (100),
             adk::MicrosecondDuration (1000)},
            {adk::MicrosecondDuration (0x80000000UL), adk::MicrosecondDuration (100),
             adk::MicrosecondDuration (1000)}};

        for (const auto& config : invalid)
        {
            adk::ResourceRegistry resources;
            FakeCaptureIo         io;
            adk::PulseCapture     capture (resources, io, 2, config);
            assert                        (capture.initialize ().error () == adk::StatusCode::InvalidArgument);
            assert                        (!resources.claimed ({adk::ResourceKind::Pin, 2}));
        }

        adk::ResourceRegistry resources;
        FakeCaptureIo         io;
        adk::PulseCapture     invalidPin  (resources, io, 70, validConfig);
        adk::PulseCapture     ordinaryPin (resources, io, 4, validConfig);
        assert                            (invalidPin.initialize ().error () == adk::StatusCode::InvalidPin);
        assert                            (ordinaryPin.initialize ().error () == adk::StatusCode::Unsupported);
    }

    void testInitializationRollbackAndReuse ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         failingIo;
        failingIo.startStatus = adk::StatusCode::HardwareFailure;
        adk::PulseCapture failing (resources, failingIo, 2, validConfig);
        assert                    (failing.initialize ().error () == adk::StatusCode::HardwareFailure);
        assert                    (!resources.claimed ({adk::ResourceKind::Pin, 2}));
        assert                    (!resources.claimed ({adk::ResourceKind::Interrupt, 0}));

        adk::ResourceClaim interruptBlocker;
        assert (resources.claim ({adk::ResourceKind::Interrupt, 0}, interruptBlocker)
                    .ok ());
        FakeCaptureIo     blockedIo;
        adk::PulseCapture blocked (resources, blockedIo, 2, validConfig);
        assert                    (blocked.initialize ().error () == adk::StatusCode::ResourceBusy);
        assert                    (!resources.claimed ({adk::ResourceKind::Pin, 2}));
        interruptBlocker.release  ();

        FakeCaptureIo     firstIo;
        FakeCaptureIo     secondIo;
        adk::PulseCapture first  (resources, firstIo, 2, validConfig);
        adk::PulseCapture second (resources, secondIo, 2, validConfig);
        assert                   (first.initialize ().ok ());
        assert                   (first.initialize ().ok ());
        assert                   (firstIo.startCalls == 1);
        assert                   (second.initialize ().error () == adk::StatusCode::ResourceBusy);
        first.shutdown           ();
        first.shutdown           ();
        assert                   (firstIo.stopCalls == 1);
        assert                   (second.initialize ().ok ());
    }

    void testActiveLowCaptureAndStableAcknowledgement ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         io;
        adk::PulseCapture     capture (resources, io, 2, validConfig);
        assert                        (capture.initialize ().ok ());

        io.edge     (5000, false);
        assert      (capture.frame ().state == adk::CaptureState::Idle);
        armAfterGap (capture, 15000);
        io.edge     (16000, false);
        io.edge     (16900, true);
        io.edge     (17450, false);
        assert      (capture.update (adk::MicrosecondTimePoint (27450)).ok ());

        const adk::PulseFrame frame = capture.frame ();
        assert                                      (frame.state == adk::CaptureState::Complete);
        assert                                      (frame.size == 2);
        assert                                      (frame.data[0].level == adk::PulseLevel::Mark);
        assert                                      (frame.data[0].duration.microseconds () == 900);
        assert                                      (frame.data[1].level == adk::PulseLevel::Space);
        assert                                      (frame.data[1].duration.microseconds () == 550);

        io.edge (28000, true);
        assert  (capture.update (adk::MicrosecondTimePoint (38000)).error () ==
                adk::StatusCode::CapacityExceeded);
        assert (capture.frame ().sequence == frame.sequence);
        assert (capture.frame ().data[0].duration.microseconds () == 900);
        assert (capture.acknowledge (frame.sequence + 1).error () ==
                adk::StatusCode::InvalidArgument);
        assert (capture.acknowledge (frame.sequence).ok ());
        assert (capture.acknowledge (frame.sequence).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testTimingFaultOverflowAndRollover ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         io;
        io.time = adk::MicrosecondTimePoint (0xffffd000UL);
        adk::PulseCapture capture           (resources, io, 3, validConfig);
        assert                              (capture.initialize ().ok ());
        armAfterGap                         (capture, 0xfffff710UL);
        io.edge                             (0xfffff800UL, false);
        io.edge                             (0xfffff864UL, true);
        assert                              (capture.update (adk::MicrosecondTimePoint (0x00001f74UL)).ok ());
        assert                              (capture.frame ().state == adk::CaptureState::Complete);
        assert                              (capture.frame ().data[0].duration.microseconds () == 100);
        assert                              (capture.acknowledge (capture.frame ().sequence).ok ());

        armAfterGap (capture, 0x00004700UL);
        io.edge     (0x00004800UL, false);
        io.edge     (0x00004863UL, true);
        assert      (capture.update (adk::MicrosecondTimePoint (0x00004863UL)).ok ());
        assert      (capture.frame ().state == adk::CaptureState::TimingFault);
        assert      (capture.acknowledge (capture.frame ().sequence).ok ());

        armAfterGap (capture, 0x00007000UL);
        uint32_t time = 0x00007100UL;
        io.edge (time, false);
        bool high = true;
        for (uint16_t index = 0; index <= adk::PulseCapture::capacity; ++index)
        {
            time += 100;
            io.edge (time, high);
            high = !high;
        }
        assert (capture.update (adk::MicrosecondTimePoint (time)).ok ());
        assert (capture.frame ().state == adk::CaptureState::Overflow);
        assert (capture.frame ().size == adk::PulseCapture::capacity);
    }

    void testDestructionReleasesClaims ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         io;
        {
            adk::PulseCapture capture (resources, io, 18, validConfig);
            assert                    (capture.initialize ().ok ());
        }
        assert (!resources.claimed ({adk::ResourceKind::Pin, 18}));
        assert (!resources.claimed ({adk::ResourceKind::Interrupt, 5}));
        assert (!io.started);
    }

    void completeOnePulse (adk::PulseCapture& capture, FakeCaptureIo& io,
                           uint32_t start)
    {
        assert  (capture.update (adk::MicrosecondTimePoint (start)).ok ());
        io.edge (start + 1, false);
        io.edge (start + 101, true);
        assert  (capture.update (adk::MicrosecondTimePoint (start + 10101)).ok ());
    }

    void testDestructionDuringActiveAndPublishedStates ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         activeIo;
        {
            adk::PulseCapture active (resources, activeIo, 2, validConfig);
            assert                   (active.initialize ().ok ());
            armAfterGap              (active);
            activeIo.edge            (10001, false);
            assert                   (active.update (adk::MicrosecondTimePoint (10001)).ok ());
        }
        assert (!resources.claimed ({adk::ResourceKind::Pin, 2}));
        assert (!resources.claimed ({adk::ResourceKind::Interrupt, 0}));

        FakeCaptureIo publishedIo;
        {
            adk::PulseCapture published (resources, publishedIo, 2, validConfig);
            assert                      (published.initialize ().ok ());
            completeOnePulse            (published, publishedIo, 10000);
            assert                      (published.frame ().state == adk::CaptureState::Complete);
        }

        FakeCaptureIo     reusedIo;
        adk::PulseCapture reused (resources, reusedIo, 2, validConfig);
        assert                   (reused.initialize ().ok ());
    }

    void testShutdownDuringActiveAndPublishedStates ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         activeIo;
        adk::PulseCapture     active (resources, activeIo, 2, validConfig);
        assert                       (active.initialize ().ok ());
        armAfterGap                  (active);
        activeIo.edge                (10001, false);
        assert                       (active.update (adk::MicrosecondTimePoint (10001)).ok ());
        active.shutdown              ();

        FakeCaptureIo     publishedIo;
        adk::PulseCapture published        (resources, publishedIo, 2, validConfig);
        assert                             (published.initialize ().ok ());
        completeOnePulse                   (published, publishedIo, 10000);
        assert                             (published.frame ().state == adk::CaptureState::Complete);
        published.shutdown                 ();

        FakeCaptureIo     reusedIo;
        adk::PulseCapture reused (resources, reusedIo, 2, validConfig);
        assert                   (reused.initialize ().ok ());
    }

    void testSequenceRolloverSkipsReservedZero ()
    {
        adk::ResourceRegistry resources;
        FakeCaptureIo         io;
        adk::PulseCapture capture                              (resources, io, 2, validConfig);
        assert                                                 (capture.initialize ().ok ());
        adk::PulseCaptureTestAccess::setNextSequence           (capture, UINT32_MAX);

        completeOnePulse (capture, io, 10000);
        assert           (capture.frame ().sequence == UINT32_MAX);
        assert           (capture.acknowledge (UINT32_MAX).ok ());

        completeOnePulse (capture, io, 30000);
        assert           (capture.frame ().sequence == 1);
    }
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::PulseCapture>::value,
                   "capture owns a pin and interrupt");
    static_assert (!std::is_move_constructible<adk::PulseCapture>::value,
                   "capture address is bound to an ISR");

    testConfigurationAndCapabilityValidation      ();
    testInitializationRollbackAndReuse            ();
    testActiveLowCaptureAndStableAcknowledgement  ();
    testTimingFaultOverflowAndRollover            ();
    testDestructionReleasesClaims                 ();
    testDestructionDuringActiveAndPublishedStates ();
    testShutdownDuringActiveAndPublishedStates    ();
    testSequenceRolloverSkipsReservedZero         ();
}
