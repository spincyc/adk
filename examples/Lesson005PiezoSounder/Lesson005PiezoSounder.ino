#include <Adk.h>

namespace {

    constexpr adk::PinId                   sounderPin = 6;
    constexpr adk::PiezoSounder::Frequency cueHz      = 440;
    const adk::Duration                    cueLength   (250);

    adk::Runtime      runtime;
    adk::PiezoSounder sounder (runtime.resources (), sounderPin);
    bool              halted = false;

    bool playWelcomeTone ();

} // namespace

void setup ()
{
    halted = !playWelcomeTone ();
}

void loop ()
{
    if (halted)
    {
        return;
    }

    const adk::TimePoint now (millis ());

    sounder.update (now);
}

namespace {

    bool playWelcomeTone ()
    {
        if (sounder.initialize () != adk::Status::Ok)
        {
            return false;
        }

        const adk::TimePoint now (millis ());

        if (sounder.play (cueHz, cueLength, now) == adk::Status::Ok)
        {
            return true;
        }

        sounder.shutdown ();
        return false;
    }

} // namespace
