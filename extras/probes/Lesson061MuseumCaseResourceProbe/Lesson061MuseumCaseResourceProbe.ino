#include <resistive_probe_observation.h>

const adk::ResistiveProbeConfig config = {
    1023,
    900,
    200,
    8,
    8,
    300,
    700,
    adk::Duration (1000),
    adk::Duration (25),
    50,
};

adk::ResistiveProbeObservationPolicy policy (config);
adk::ResistiveProbeSample inputSample = {
    1,
    1,
    1,
    1,
    adk::TimePoint (100),
    700,
    0,
    adk::Duration (5),
    adk::Duration (100),
    true,
    adk::StatusCode::Ok,
};
adk::ResistiveProbeObservation outputObservation = {};
volatile uint16_t observationWitness = 0;

void setup ()
{
    policy.initialize ();

    policy.update (adk::TimePoint (101), inputSample);

    outputObservation  = policy.snapshot ();
    observationWitness = outputObservation.normalizedPermille;
}

void loop ()
{
}
