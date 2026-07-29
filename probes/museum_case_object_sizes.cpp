#include <resistive_probe_observation.h>
#if defined (ADK_HAS_LESSON_062)
#include <thermal_radiant_observation.h>
#endif
#if defined (ADK_HAS_LESSON_063)
#include <museum_case_monitor.h>
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

#if defined (ADK_HAS_LESSON_063)
unsigned char museumCaseMonitorObjectBytes[sizeof (adk::MuseumCaseMonitor)];
unsigned char museumCaseMaximumOwnedObjectsBytes
    [sizeof (adk::ResistiveProbeObservationPolicy)
     + sizeof (adk::ThermalRadiantObservationPolicy)
     + sizeof (adk::MuseumCaseMonitor)];
unsigned char museumCaseHealthBytes[sizeof (adk::MuseumCaseHealth)];
unsigned char museumHazardBytes[sizeof (adk::MuseumHazard)];
unsigned char museumReedEvidenceBytes[sizeof (adk::MuseumReedEvidence)];
unsigned char museumAcknowledgeEvidenceBytes
    [sizeof (adk::MuseumAcknowledgeEvidence)];
unsigned char museumAuditIntentBytes[sizeof (adk::MuseumAuditIntent)];
unsigned char museumAuditReceiptBytes[sizeof (adk::MuseumAuditReceipt)];
unsigned char museumCaseConfigBytes[sizeof (adk::MuseumCaseConfig)];
unsigned char museumCaseEnvelopeBytes[sizeof (adk::MuseumCaseEnvelope)];
unsigned char museumCaseIntentBytes[sizeof (adk::MuseumCaseIntent)];
unsigned char museumCaseResultBytes[sizeof (adk::MuseumCaseResult)];
#endif
