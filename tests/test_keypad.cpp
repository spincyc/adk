#include "keypad.h"

#include <assert.h>
namespace {

    using namespace adk;

    void sample (Keypad& keypad,
                 uint32_t time,
                 uint16_t mask,
                 bool     valid = true)
    {
        const Status status = keypad.update (TimePoint (time), {mask, valid});

        assert (status == Status::Ok || status == Status::HardwareFailure);
    }

    void settle (Keypad& keypad,
                 uint32_t start,
                 uint16_t mask,
                 bool     valid = true)
    {
        sample (keypad, start,       mask, valid);
        sample (keypad, start + 20U, mask, valid);
    }

    void acceptsOneDebouncedPressAfterRelease ()
    {
        KeypadConfig config;
        Keypad       keypad (config);

        assert (keypad.initialize () == Status::Ok);
        assert (keypad.initialize () == Status::Ok);

        settle (keypad, 0, 1U);
        assert (keypad.snapshot ().key        == KeypadKey::Digit1);
        assert (keypad.snapshot ().pressEvent);
        assert (keypad.snapshot ().pressEvent);

        settle (keypad, 30, 1U);
        assert (!keypad.snapshot ().pressEvent);

        settle (keypad, 60, 0U);
        assert (keypad.snapshot ().releaseEvent);

        settle (keypad, 90, 1U);
        assert (keypad.snapshot ().pressEvent);
    }

    void observesExactDebounceBoundary ()
    {
        Keypad keypad {KeypadConfig ()};

        assert (keypad.initialize () == Status::Ok);

        sample (keypad, 100U, 2U);
        sample (keypad, 119U, 2U);

        assert (keypad.snapshot ().state == KeypadState::Released);
        assert (!keypad.snapshot ().pressEvent);

        sample (keypad, 120U, 2U);

        assert (keypad.snapshot ().state == KeypadState::Pressed);
        assert (keypad.snapshot ().key   == KeypadKey::Digit2);
        assert (keypad.snapshot ().pressEvent);

        sample (keypad, 121U, 2U);

        assert (!keypad.snapshot ().pressEvent);
    }

    void mapsEverySupportedKey ()
    {
        const KeypadKey expected[] =
        {
            KeypadKey::Digit1,
            KeypadKey::Digit2,
            KeypadKey::Digit3,
            KeypadKey::Digit4,
            KeypadKey::Digit5,
            KeypadKey::Digit6,
            KeypadKey::Digit7,
            KeypadKey::Digit8,
            KeypadKey::Digit9,
            KeypadKey::Star,
            KeypadKey::Digit0,
            KeypadKey::Hash
        };

        for (uint8_t index = 0; index < 12U; ++index)
        {
            Keypad keypad {KeypadConfig ()};

            assert (keypad.initialize () == Status::Ok);

            settle (keypad,
                    static_cast<uint32_t> (index) * 30U,
                    static_cast<uint16_t> (1U << index));

            assert (keypad.snapshot ().key == expected[index]);
        }
    }

    void invalidChordRequiresARelease ()
    {
        KeypadConfig config;
        Keypad       keypad (config);

        assert (keypad.initialize () == Status::Ok);

        settle (keypad, 0, 3U);
        assert (keypad.snapshot ().state == KeypadState::InvalidChord);

        settle (keypad, 30, 1U);
        assert (!keypad.snapshot ().pressEvent);

        settle (keypad, 60, 0U);
        settle (keypad, 90, 1U);
        assert (keypad.snapshot ().pressEvent);
    }

    void faultRequiresAValidRelease ()
    {
        KeypadConfig config;
        Keypad       keypad (config);

        assert (keypad.initialize () == Status::Ok);

        settle (keypad, 0U, 1U);
        settle (keypad, 30U, 0U, false);

        assert (keypad.snapshot ().state  == KeypadState::Fault);
        assert (keypad.snapshot ().status == Status::HardwareFailure);

        settle (keypad, 60U, 1U);

        assert (keypad.snapshot ().state == KeypadState::Pressed);
        assert (!keypad.snapshot ().pressEvent);

        settle (keypad, 90U, 0U);
        settle (keypad, 120U, 1U);

        assert (keypad.snapshot ().pressEvent);
    }

    void acceptsWraparoundAndRejectsBackwardTime ()
    {
        Keypad keypad {KeypadConfig ()};

        assert (keypad.initialize () == Status::Ok);

        sample (keypad, 0xfffffff8UL, 1U);
        sample (keypad, 0x0000000bUL, 1U);

        assert (!keypad.snapshot ().pressEvent);

        sample (keypad, 0x0000000cUL, 1U);

        assert (keypad.snapshot ().pressEvent);
        assert (keypad.update (TimePoint (0x8000000cUL), {0U, true}) ==
                Status::InvalidArgument);
        assert (keypad.snapshot ().rawMask == 1U);

        assert (keypad.update (TimePoint (0x0000000dUL), {1U, true}) ==
                Status::Ok);
        assert (keypad.snapshot ().status == Status::Ok);
    }

    void rejectsUnsupportedMaskWithoutChangingSample ()
    {
        Keypad keypad {KeypadConfig ()};

        assert (keypad.initialize () == Status::Ok);

        settle (keypad, 0U, 1U);

        assert (keypad.update (TimePoint (21U), {0x1000U, true}) ==
                Status::InvalidArgument);
        assert (keypad.snapshot ().rawMask == 1U);
        assert (keypad.update (TimePoint (22U), {1U, true}) == Status::Ok);
        assert (keypad.snapshot ().status == Status::Ok);
    }

    void replayIsDeterministic ()
    {
        const KeypadSample trace[] =
        {
            {0U, true},
            {4U, true},
            {4U, true},
            {0U, true},
            {0U, true}
        };
        const uint32_t times[] = {0U, 10U, 30U, 40U, 60U};
        KeypadSnapshot first[5];

        for (uint8_t run = 0; run < 2U; ++run)
        {
            Keypad keypad {KeypadConfig ()};

            assert (keypad.initialize () == Status::Ok);

            for (uint8_t index = 0; index < 5U; ++index)
            {
                sample (keypad,
                        times[index],
                        trace[index].pressedMask,
                        trace[index].valid);

                const KeypadSnapshot observed = keypad.snapshot ();

                if (run == 0)
                {
                    first[index] = observed;
                    continue;
                }

                assert (observed.key          == first[index].key);
                assert (observed.state        == first[index].state);
                assert (observed.status       == first[index].status);
                assert (observed.rawMask      == first[index].rawMask);
                assert (observed.pressEvent   == first[index].pressEvent);
                assert (observed.releaseEvent == first[index].releaseEvent);
            }
        }
    }

    void lifecycleIsInertAndRepeatable ()
    {
        KeypadConfig invalid;

        invalid.debounce = Duration (0);

        Keypad keypad    (invalid);

        assert (keypad.update (TimePoint (0), {0U, true}) ==
                Status::NotInitialized);
        assert (keypad.initialize () == Status::InvalidArgument);

        keypad.shutdown ();
        keypad.shutdown ();

        assert (!keypad.initialized ());
        assert (keypad.snapshot ().status == Status::NotInitialized);

        invalid.debounce = Duration (0x80000000UL);

        Keypad ambiguous (invalid);

        assert (ambiguous.initialize () == Status::InvalidArgument);
    }
}

int main ()
{
    acceptsOneDebouncedPressAfterRelease ();

    observesExactDebounceBoundary ();

    mapsEverySupportedKey ();

    invalidChordRequiresARelease ();

    faultRequiresAValidRelease ();

    acceptsWraparoundAndRejectsBackwardTime ();

    rejectsUnsupportedMaskWithoutChangingSample ();

    replayIsDeterministic ();

    lifecycleIsInertAndRepeatable ();
}
