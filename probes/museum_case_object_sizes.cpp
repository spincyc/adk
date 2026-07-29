#include <resistive_probe_observation.h>
#if defined (ADK_HAS_LESSON_062)
#include <thermal_radiant_observation.h>
#endif

unsigned char resistiveProbeObservationPolicyObjectBytes
    [sizeof (adk::ResistiveProbeObservationPolicy)];
unsigned char probeQualityBytes[sizeof (adk::ProbeQuality)];
unsigned char resistiveProbeSampleBytes[sizeof (adk::ResistiveProbeSample)];
unsigned char resistiveProbeConfigBytes[sizeof (adk::ResistiveProbeConfig)];
unsigned char resistiveProbeObservationBytes
    [sizeof (adk::ResistiveProbeObservation)];

#if defined (ADK_HAS_LESSON_062)
unsigned char thermalRadiantObservationPolicyObjectBytes
    [sizeof (adk::ThermalRadiantObservationPolicy)];
unsigned char thresholdStateBytes[sizeof (adk::ThresholdState)];
unsigned char thermalQualityBytes[sizeof (adk::ThermalQuality)];
unsigned char radiantQualityBytes[sizeof (adk::RadiantQuality)];
unsigned char convertedThermalSampleBytes
    [sizeof (adk::ConvertedThermalSample)];
unsigned char categoricalThresholdSampleBytes
    [sizeof (adk::CategoricalThresholdSample)];
unsigned char thermalRadiantEnvelopeBytes
    [sizeof (adk::ThermalRadiantEnvelope)];
unsigned char thermalRadiantConfigBytes
    [sizeof (adk::ThermalRadiantConfig)];
unsigned char thermalRadiantObservationBytes
    [sizeof (adk::ThermalRadiantObservation)];
#endif
