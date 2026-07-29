#include <clue_constraint_model.h>
#include <fault_aware_operator_panel.h>

#if defined (ADK_HAS_INERT_ESCAPE_CONSOLE)
#include <inert_escape_console.h>
#endif

unsigned char
    clueConstraintModelObjectBytes[sizeof (adk::ClueConstraintModel)];
unsigned char faultAwareOperatorPanelObjectBytes
    [sizeof (adk::FaultAwareOperatorPanel)];

#if defined (ADK_HAS_INERT_ESCAPE_CONSOLE)
unsigned char inertEscapeConsoleObjectBytes[sizeof (adk::InertEscapeConsole)];
unsigned char escapeConsoleSnapshotBytes[sizeof (adk::EscapeConsoleSnapshot)];
unsigned char escapeFamilySnapshotBytes[sizeof (adk::EscapeFamilySnapshot)];
#endif
