#include <assert.h>
#include <stdint.h>

#include "infrared_decoder.h"

namespace {

    constexpr uint8_t pulseCapacity = 67;

    adk::Pulse pulse (adk::PulseLevel level, uint32_t duration)
    {
        const adk::Pulse result = {level, adk::MicrosecondDuration (duration)};
        return result;
    }

    adk::PulseFrame makeNec (adk::Pulse* storage, uint8_t address, uint8_t command,
                             uint32_t sequence)
    {
        const uint32_t bits =
            static_cast<uint32_t> (address) |
            static_cast<uint32_t> (static_cast<uint8_t> (~address)) << 8 |
            static_cast<uint32_t> (command) << 16 |
            static_cast<uint32_t> (static_cast<uint8_t> (~command)) << 24;

        storage[0] = pulse (adk::PulseLevel::Mark, 9000);
        storage[1] = pulse (adk::PulseLevel::Space, 4500);

        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            storage[2 + bit * 2] = pulse (adk::PulseLevel::Mark, 560);
            storage[3 + bit * 2] =
                pulse (adk::PulseLevel::Space,
                       (bits & (static_cast<uint32_t> (1) << bit)) ? 1690 : 560);
        }

        storage[66] = pulse (adk::PulseLevel::Mark, 560);

        const adk::PulseFrame frame = {storage, pulseCapacity, sequence,
                                       adk::CaptureState::Complete};
        return frame;
    }

    void testDecodesOwnedNecEvidence ()
    {
        adk::Pulse            storage[pulseCapacity];
        const adk::PulseFrame capture = makeNec (storage, 0x12, 0xa5, 17);
        adk::InfraredFrame    frame   = {};

        const adk::Status status = adk::InfraredDecoder ().decode (capture, frame);

        assert (status.ok ());
        assert (frame.protocol == adk::InfraredProtocol::Nec);
        assert (frame.validity == adk::FrameValidity::Valid);
        assert (frame.address == 0x12);
        assert (frame.command == 0xa5);
        assert (frame.captureSequence == 17);
    }

    void testAcceptsInclusiveTimingBoundaries ()
    {
        adk::Pulse         storage[pulseCapacity];
        adk::PulseFrame    capture = makeNec (storage, 0, 0xff, 1);
        adk::InfraredFrame frame   = {};

        storage[0].duration  = adk::MicrosecondDuration (8000);
        storage[1].duration  = adk::MicrosecondDuration (5000);
        storage[2].duration  = adk::MicrosecondDuration (400);
        storage[3].duration  = adk::MicrosecondDuration (700);
        storage[66].duration = adk::MicrosecondDuration (700);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Valid);
    }

    void testEveryNecTimingWindowIsInclusiveAndBounded ()
    {
        adk::Pulse         storage[pulseCapacity];
        adk::InfraredFrame frame = {};

        const uint32_t leaderMarkAccepted[]  = {8000, 10000};
        const uint32_t leaderSpaceAccepted[] = {4000, 5000};
        const uint32_t bitMarkAccepted[]     = {400, 700};
        const uint32_t zeroSpaceAccepted[]   = {400, 700};
        const uint32_t oneSpaceAccepted[]    = {1400, 1900};

        for (uint8_t index = 0; index < 2; ++index)
        {
            adk::PulseFrame capture = makeNec (storage, 1, 1, index);

            storage[0].duration = adk::MicrosecondDuration (leaderMarkAccepted[index]);
            storage[1].duration = adk::MicrosecondDuration (leaderSpaceAccepted[index]);
            storage[2].duration = adk::MicrosecondDuration (bitMarkAccepted[index]);
            storage[5].duration = adk::MicrosecondDuration (zeroSpaceAccepted[index]);
            storage[3].duration = adk::MicrosecondDuration (oneSpaceAccepted[index]);

            assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
            assert (frame.validity == adk::FrameValidity::Valid);
        }

        struct OutsideTiming
        {
            uint8_t            pulseIndex;
            uint32_t           duration;
            adk::FrameValidity validity;
        };

        const OutsideTiming outside[] = {
            {0, 7999, adk::FrameValidity::UnknownProtocol},
            {0, 10001, adk::FrameValidity::UnknownProtocol},
            {1, 3999, adk::FrameValidity::UnknownProtocol},
            {1, 5001, adk::FrameValidity::UnknownProtocol},
            {2, 399, adk::FrameValidity::TimingInvalid},
            {2, 701, adk::FrameValidity::TimingInvalid},
            {5, 399, adk::FrameValidity::TimingInvalid},
            {5, 701, adk::FrameValidity::TimingInvalid},
            {3, 1399, adk::FrameValidity::TimingInvalid},
            {3, 1901, adk::FrameValidity::TimingInvalid}};

        for (uint8_t index = 0; index < sizeof (outside) / sizeof (outside[0]); ++index)
        {
            adk::PulseFrame capture = makeNec (storage, 1, 1, index);
            storage[outside[index].pulseIndex].duration =
                adk::MicrosecondDuration (outside[index].duration);

            assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
            assert (frame.validity == outside[index].validity);
        }
    }

    void testRejectsIntegrityAndTimingFailures ()
    {
        adk::Pulse         storage[pulseCapacity];
        adk::PulseFrame    capture = makeNec (storage, 0x44, 0x20, 2);
        adk::InfraredFrame frame   = {};

        storage[19].duration = adk::MicrosecondDuration (560);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::IntegrityInvalid);
        assert (frame.address == 0);
        assert (frame.command == 0);

        capture              = makeNec (storage, 0x44, 0x20, 2);

        storage[20].duration = adk::MicrosecondDuration (701);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::TimingInvalid);
    }

    void testRejectsPrefixSuffixAndBoundaryNoise ()
    {
        adk::Pulse         storage[pulseCapacity + 1];
        adk::PulseFrame    capture = makeNec (storage, 0, 0, 5);
        adk::InfraredFrame frame   = {};

        storage[0].duration = adk::MicrosecondDuration (7999);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::UnknownProtocol);

        capture      = makeNec (storage, 0xff, 0xff, 6);

        storage[67]  = pulse (adk::PulseLevel::Space, 560);
        capture.size = 68;

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Overflow);
    }

    void testReportsRepeatUnknownTruncatedAndOverflow ()
    {
        adk::Pulse         repeat[] = {pulse (adk::PulseLevel::Mark, 9000),
                                       pulse (adk::PulseLevel::Space, 2250),
                                       pulse (adk::PulseLevel::Mark, 560)};
        adk::PulseFrame    capture  = {repeat, 3, 9, adk::CaptureState::Complete};
        adk::InfraredFrame frame    = {};

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Repeat);

        repeat[0] = pulse (adk::PulseLevel::Space, 9000);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::UnknownProtocol);

        adk::Pulse storage[pulseCapacity];
        capture      = makeNec (storage, 1, 2, 10);
        capture.size = 66;
        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Truncated);

        capture.state = adk::CaptureState::Overflow;
        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Overflow);
    }

    void testRepeatTimingIsExplicitAndBounded ()
    {
        adk::Pulse         repeat[] = {pulse (adk::PulseLevel::Mark, 8000),
                                       pulse (adk::PulseLevel::Space, 2000),
                                       pulse (adk::PulseLevel::Mark, 400)};
        adk::PulseFrame    capture  = {repeat, 3, 14, adk::CaptureState::Complete};
        adk::InfraredFrame frame    = {};

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.protocol == adk::InfraredProtocol::Nec);
        assert (frame.validity == adk::FrameValidity::Repeat);
        assert (frame.address == 0);
        assert (frame.command == 0);

        repeat[0].duration = adk::MicrosecondDuration (10000);
        repeat[1].duration = adk::MicrosecondDuration (2500);
        repeat[2].duration = adk::MicrosecondDuration (700);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Repeat);

        repeat[1].duration = adk::MicrosecondDuration (2501);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::UnknownProtocol);
        assert (frame.address == 0);
        assert (frame.command == 0);
    }

    void testInvalidStorageLeavesOutputUnchanged ()
    {
        const adk::PulseFrame capture = {nullptr, 1, 4, adk::CaptureState::Complete};
        adk::InfraredFrame    frame   = {adk::InfraredProtocol::Nec,
                                         adk::FrameValidity::Valid, 1, 2, 3};

        const adk::Status status = adk::InfraredDecoder ().decode (capture, frame);

        assert (status.error () == adk::StatusCode::InvalidArgument);
        assert (frame.protocol == adk::InfraredProtocol::Nec);
        assert (frame.validity == adk::FrameValidity::Valid);
        assert (frame.captureSequence == 3);
    }

    void testReportsCaptureStatesAndRejectsUnknownState ()
    {
        const adk::Pulse   pulseStorage[] = {pulse (adk::PulseLevel::Mark, 560)};
        adk::PulseFrame    capture = {pulseStorage, 1, 12, adk::CaptureState::Idle};
        adk::InfraredFrame frame   = {};

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Truncated);

        capture.state = adk::CaptureState::Capturing;
        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::Truncated);

        capture.state = adk::CaptureState::TimingFault;
        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.validity == adk::FrameValidity::TimingInvalid);

        frame = {adk::InfraredProtocol::Nec, adk::FrameValidity::Valid, 1, 2, 3};
        capture.state = static_cast<adk::CaptureState> (255);
        assert (adk::InfraredDecoder ().decode (capture, frame).error () ==
                adk::StatusCode::InvalidArgument);
        assert (frame.address == 1);
        assert (frame.command == 2);
        assert (frame.captureSequence == 3);
    }

    void testEmptyCompleteCaptureIsTruncatedEvidence ()
    {
        const adk::PulseFrame capture = {nullptr, 0, 21, adk::CaptureState::Complete};
        adk::InfraredFrame    frame   = {};

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.protocol == adk::InfraredProtocol::Unknown);
        assert (frame.validity == adk::FrameValidity::Truncated);
        assert (frame.address == 0);
        assert (frame.command == 0);
    }

    void testUnknownPulseLevelCannotBecomeValidEvidence ()
    {
        adk::Pulse            storage[pulseCapacity];
        const adk::PulseFrame capture = makeNec (storage, 3, 4, 22);
        adk::InfraredFrame    frame   = {};
        storage[2].level              = static_cast<adk::PulseLevel> (255);

        assert (adk::InfraredDecoder ().decode (capture, frame).ok ());
        assert (frame.protocol == adk::InfraredProtocol::Nec);
        assert (frame.validity == adk::FrameValidity::TimingInvalid);
        assert (frame.address == 0);
        assert (frame.command == 0);
    }

    void testIdenticalTracesProduceIdenticalFrames ()
    {
        adk::Pulse            leftStorage[pulseCapacity];
        adk::Pulse            rightStorage[pulseCapacity];
        const adk::PulseFrame left       = makeNec (leftStorage, 7, 11, 31);
        const adk::PulseFrame right      = makeNec (rightStorage, 7, 11, 31);
        adk::InfraredFrame    leftFrame  = {};
        adk::InfraredFrame    rightFrame = {};

        adk::InfraredDecoder ().decode (left, leftFrame);
        adk::InfraredDecoder ().decode (right, rightFrame);

        assert (leftFrame.protocol == rightFrame.protocol);
        assert (leftFrame.validity == rightFrame.validity);
        assert (leftFrame.address == rightFrame.address);
        assert (leftFrame.command == rightFrame.command);
        assert (leftFrame.captureSequence == rightFrame.captureSequence);
    }
} // namespace

int main ()
{
    testDecodesOwnedNecEvidence                    ();
    testAcceptsInclusiveTimingBoundaries           ();
    testEveryNecTimingWindowIsInclusiveAndBounded  ();
    testRejectsIntegrityAndTimingFailures          ();
    testRejectsPrefixSuffixAndBoundaryNoise        ();
    testReportsRepeatUnknownTruncatedAndOverflow   ();
    testRepeatTimingIsExplicitAndBounded           ();
    testInvalidStorageLeavesOutputUnchanged        ();
    testReportsCaptureStatesAndRejectsUnknownState ();
    testEmptyCompleteCaptureIsTruncatedEvidence    ();
    testUnknownPulseLevelCannotBecomeValidEvidence ();
    testIdenticalTracesProduceIdenticalFrames      ();
}
