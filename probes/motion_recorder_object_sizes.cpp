#include <inertial_record.h>

unsigned char inertialRecordNormalizerObjectBytes
    [sizeof (adk::InertialRecordNormalizer)];
unsigned char inertialRecordCodecObjectBytes
    [sizeof (adk::InertialRecordCodec)];
unsigned char inertialRecordImageCallerBufferBytes
    [adk::InertialRecordCodec::size];
unsigned char InertialRecordConfigBytes[sizeof (adk::InertialRecordConfig)];
unsigned char InertialRecordBytes[sizeof (adk::InertialRecord)];
unsigned char InertialRecordStateBytes[sizeof (adk::InertialRecordState)];
unsigned char InertialRecordValidityBytes[sizeof (adk::InertialRecordValidity)];

#if defined (ADK_HAS_LESSON_068)
#include <inertial_record_qualification.h>

unsigned char inertialRecordQualificationPolicyObjectBytes
    [sizeof (adk::InertialRecordQualificationPolicy)];
unsigned char InertialRecordQualificationConfigBytes
    [sizeof (adk::InertialRecordQualificationConfig)];
unsigned char InertialWideVectorBytes[sizeof (adk::InertialWideVector)];
unsigned char InertialQualificationEvidenceBytes
    [sizeof (adk::InertialQualificationEvidence)];
unsigned char InertialQualificationStateBytes
    [sizeof (adk::InertialQualificationState)];
unsigned char InertialQualificationReasonBytes
    [sizeof (adk::InertialQualificationReason)];
#endif

#if defined (ADK_HAS_LESSON_069)
#include <qualified_motion_recorder.h>

unsigned char qualifiedMotionRecorderObjectBytes
    [sizeof (adk::QualifiedMotionRecorder)];
unsigned char MotionRecorderConfigBytes[sizeof (adk::MotionRecorderConfig)];
unsigned char MotionRecorderControlBytes[sizeof (adk::MotionRecorderControl)];
unsigned char MotionPresentationIntentBytes
    [sizeof (adk::MotionPresentationIntent)];
unsigned char MotionRecordImageBytes[sizeof (adk::MotionRecordImage)];
unsigned char MotionRecorderResultBytes[sizeof (adk::MotionRecorderResult)];
#endif
