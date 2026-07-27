#include <Adk.h>

adk::Runtime       runtime;
adk::PiezoSounder sounder (runtime.resources (), 6);

void setup ()
{
    const adk::Status status = sounder.initialize ();

    if (status == adk::Status::Ok)
    {
        sounder.play
        (
            440,
            adk::Duration  (250),
            adk::TimePoint (millis ())
        );
    }
}

void loop ()
{
    sounder.update (adk::TimePoint (millis ()));
}
