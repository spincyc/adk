#include <thermal_radiant_observation.h>

const adk::ThermalRadiantConfig config = {
    25000,
    30000,
    adk::Duration (1000),
    adk::Duration (100),
    adk::Duration (500),
};

adk::ThermalRadiantObservationPolicy policy (config);
adk::ThermalRadiantEnvelope inputEnvelope = {
    {1, 1, 1, 1, adk::TimePoint (100), 24000, 250, false,
     adk::StatusCode::Ok},
    {2, 1, 1, 1, adk::TimePoint (100), 100, adk::ThresholdState::Below,
     false, adk::StatusCode::Ok},
    {3, 1, 1, 1, adk::TimePoint (100), 100, adk::ThresholdState::Below,
     false, adk::StatusCode::Ok},
};
adk::ThermalRadiantObservation outputObservation = {};
volatile uint16_t observationWitness = 0;

void setup ()
{
    policy.initialize ();

    policy.update (adk::TimePoint (101), inputEnvelope);

    outputObservation  = policy.snapshot ();
    observationWitness = outputObservation.envelope.radiant.raw;
}

void loop ()
{
}
