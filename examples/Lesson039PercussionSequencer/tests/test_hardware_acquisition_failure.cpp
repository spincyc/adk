#include <Arduino.h>

#define ADK_LESSON039_USE_REFERENCE_HARDWARE 1
#define setup                                lesson039Setup
#define loop                                 lesson039Loop
#include "../Lesson039PercussionSequencer.ino"
#undef loop
#undef setup

#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

    void verifyRollback (adk::ResourceId blocked)
    {
        adk::test::arduino::reset ();

        adk::ResourceClaim blocker;
        assert (runtime.resources ().claim (blocked, blocker).ok ());

        lesson039Setup ();

        assert (halted);
        assert (faultSource == Lesson039AdapterFault::Acquisition);
        assert (blocker.active ());

        for (uint8_t pin = 38; pin <= 53; ++pin)
        {
            const bool externallyBlocked =
                blocked.kind == adk::ResourceKind::Pin && blocked.index == pin;
            assert (runtime.resources ().claimed ({adk::ResourceKind::Pin, pin}) ==
                    externallyBlocked);
            assert (adk::test::arduino::mode (pin) == INPUT);
        }
        for (uint8_t pin = 62; pin <= 63; ++pin)
        {
            const bool externallyBlocked =
                blocked.kind == adk::ResourceKind::Pin && blocked.index == pin;
            assert (runtime.resources ().claimed ({adk::ResourceKind::Pin, pin}) ==
                    externallyBlocked);
        }
        assert (!runtime.resources ().claimed ({adk::ResourceKind::Pin, 6}));
        assert (adk::test::arduino::toneFrequency (6) == 0);

        blocker.release ();
    }

    void runIsolated (adk::ResourceId blocked)
    {
        const pid_t child = fork ();

        assert (child >= 0);
        if (child == 0)
        {
            verifyRollback (blocked);

            _exit (0);
        }

        int status = 0;

        assert (waitpid (child, &status, 0) == child);
        assert (WIFEXITED (status));
        assert (WEXITSTATUS (status) == 0);
    }

} // namespace

int main ()
{
    runIsolated ({adk::ResourceKind::Pin, 40});
    runIsolated ({adk::ResourceKind::Pin, 63});
    runIsolated ({adk::ResourceKind::Pin, 49});
    runIsolated ({adk::ResourceKind::Timer, 2});
}
