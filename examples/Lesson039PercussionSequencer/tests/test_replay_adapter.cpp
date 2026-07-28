#include <Arduino.h>

#define setup lesson039Setup
#define loop  lesson039Loop
#include "../Lesson039PercussionSequencer.ino"
#undef loop
#undef setup

#include <assert.h>

namespace {

    bool isSourcePin (uint8_t pin)
    {
        return (pin >= 38 && pin <= 44) || pin == 62 || pin == 63;
    }

    uint32_t operationCount (adk::test::arduino::OperationKind kind)
    {
        uint32_t count = 0;
        for (const auto& operation : adk::test::arduino::trace ())
        {
            if (operation.kind == kind)
            {
                ++count;
            }
        }
        return count;
    }

} // namespace

int main ()
{
    adk::test::arduino::reset ();

    lesson039Setup ();

    assert (!halted);

    adk::test::arduino::clearTrace ();
    bool     sawPlayback   = false;
    bool     haveFrame     = false;
    uint8_t  previousStep  = 0;
    uint32_t cueStartedAt  = 0;
    uint32_t cueDurationMs = 0;
    uint32_t toneCalls     = 0;
    for (uint32_t nowMs = 0; nowMs <= 5000; ++nowMs)
    {
        adk::test::arduino::setTimeUs (static_cast<uint64_t> (nowMs) * 1000U);

        lesson039Loop ();

        const auto snapshot = sequencer.snapshot ();
        if (snapshot.frameValid)
        {
            sawPlayback             = true;
            const bool     newFrame = !haveFrame || snapshot.frame.step != previousStep;
            const uint32_t currentToneCalls =
                operationCount (adk::test::arduino::OperationKind::Tone);
            if (newFrame && snapshot.frame.frequencyHz != 0)
            {
                assert (currentToneCalls == toneCalls + 1);
                cueStartedAt  = nowMs;
                cueDurationMs = snapshot.frame.toneDuration.milliseconds ();
            }
            else
            {
                assert (currentToneCalls == toneCalls);
                if (newFrame)
                {
                    cueDurationMs = 0;
                }
            }
            toneCalls    = currentToneCalls;
            haveFrame    = true;
            previousStep = snapshot.frame.step;

            assert (display.glyph () ==
                    static_cast<adk::SevenSegmentGlyph> (snapshot.frame.step));
            for (uint8_t surface = 0; surface < 4; ++surface)
            {
                const bool active = (snapshot.frame.surfaceMask & (1U << surface)) != 0;
                assert (adk::test::arduino::digitalOutput (static_cast<uint8_t> (
                            45 + surface)) == (active ? HIGH : LOW));
            }
            const bool status = !snapshot.frameValid || snapshot.frame.heartbeat;
            assert (adk::test::arduino::digitalOutput (52) == (status ? HIGH : LOW));
            assert (adk::test::arduino::digitalOutput (53) == LOW);

            const bool cueActive =
                cueDurationMs != 0 && nowMs - cueStartedAt < cueDurationMs;
            assert (adk::test::arduino::toneFrequency (6) ==
                    (cueActive ? snapshot.frame.frequencyHz : 0));
        }
    }
    assert (sawPlayback);

    const uint32_t beforeSparse =
        operationCount (adk::test::arduino::OperationKind::Tone);
    adk::test::arduino::setTimeUs (UINT64_C (7600) * 1000U);

    lesson039Loop ();

    const auto sparse = sequencer.snapshot ();

    assert (sparse.frameValid);
    assert (operationCount (adk::test::arduino::OperationKind::Tone) <=
            beforeSparse + 1);
    for (uint8_t surface = 0; surface < 4; ++surface)
    {
        const bool active = (sparse.frame.surfaceMask & (1U << surface)) != 0;
        assert (adk::test::arduino::digitalOutput (
                    static_cast<uint8_t> (45 + surface)) == (active ? HIGH : LOW));
    }

    for (const auto& operation : adk::test::arduino::trace ())
    {
        assert (!isSourcePin (operation.pin));
    }

    enterSafeFault (Lesson039AdapterFault::Sound);

    assert (halted);
    assert (faultSource == Lesson039AdapterFault::Sound);
    assert (adk::test::arduino::toneFrequency (6) == 0);
    for (uint8_t pin = 45; pin <= 53; ++pin)
    {
        assert (adk::test::arduino::mode (pin) == INPUT);
    }
    assert (adk::test::arduino::mode (6) == INPUT);
}
