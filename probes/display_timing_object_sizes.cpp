#include <multiplexed_digit_policy.h>

#if defined (ADK_HAS_LESSON_059)
#include <max7219_presentation_policy.h>
#endif

#if defined (ADK_HAS_LESSON_060)
#include <dual_display_timing_desk.h>
#endif

unsigned char multiplexedDigitPolicyObjectBytes
    [sizeof (adk::MultiplexedDigitPolicy)];
unsigned char multiplexedDigitFrameBytes
    [sizeof (adk::MultiplexedDigitFrame)];
unsigned char multiplexedDigitTransactionBytes
    [sizeof (adk::MultiplexedDigitTransaction)];
unsigned char multiplexedDigitSnapshotBytes
    [sizeof (adk::MultiplexedDigitSnapshot)];

#if defined (ADK_HAS_LESSON_059)
unsigned char max7219PresentationPolicyObjectBytes
    [sizeof (adk::Max7219PresentationPolicy)];
unsigned char max7219PresentationConfigBytes
    [sizeof (adk::Max7219PresentationConfig)];
unsigned char max7219FrameBytes[sizeof (adk::Max7219Frame)];
unsigned char max7219CommandBytes[sizeof (adk::Max7219Command)];
unsigned char max7219ReceiptBytes[sizeof (adk::Max7219Receipt)];
unsigned char max7219FailureBytes[sizeof (adk::Max7219Failure)];
unsigned char max7219PresentationSnapshotBytes
    [sizeof (adk::Max7219PresentationSnapshot)];
unsigned char max7219PresentationPreviewBytes
    [sizeof (adk::Max7219PresentationPreview)];
#endif

#if defined (ADK_HAS_LESSON_060)
unsigned char dualDisplayTimingDeskObjectBytes
    [sizeof (adk::DualDisplayTimingDesk)];
#endif
