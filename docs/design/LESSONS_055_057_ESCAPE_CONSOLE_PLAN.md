# Lessons 055--057 inert escape-console plan

Status: implementation-depth E0 plan; exact powered fixtures remain open.

This arc adds a project-specific constraint model, a pure fault-aware operator
panel, and one inert escape-console coordinator. E0 is fixed-storage policy over
copied values. It owns no endpoint, pin, interrupt, timer, display, storage
medium, servo, relay, lamp, latch, door, lock, or power path. Presentation,
latch, and lamp outputs are semantic intent only.

The console is a fictional puzzle instrument. It is not access control,
authentication, authorization, security, confinement, egress, emergency
release, or life-safety equipment. No person, animal, property, or safe exit
may depend on it.

## Evidence levels

| Level | Authorized work |
|---|---|
| E0 | Host and compile-only Mega replay over copied categorical clues, pure panel values, caller-owned audit images, and inert latch/lamp intent; zero hardware ownership |
| E1 | Separately qualified exact passive clue and operator inputs plus low-voltage presentation; servo, relay, latch, and other powered actuation physically absent |
| E2 | A restrained demonstration servo or inert current-limited low-voltage relay/lamp load, independent load-power removal, and measured bench acceptance; never a door, lock, occupied enclosure, egress route, alarm, or safety system |

E0 therefore makes no claim about input voltage, debounce, wiring, display,
storage durability, lamp current, relay contacts, servo travel, physical latch
state, room state, or occupant safety. E1 and E2 require separate admission;
they cannot be inferred from a passing E0 replay.

## Dependency order and ownership

| Lesson | Boundary | Depends on | Explicit exclusions |
|---:|---|---|---|
| 055 | `ClueConstraintModel`, a project-specific fixed-capacity DAG over 12 clues and 12 rules | `Status`, explicit microsecond time, copied categorical observations | Generic solver, endpoint ownership, callbacks, dynamic rules, identity/security inference |
| 056 | `FaultAwareOperatorPanel`, a pure copied-value panel with two-image audit protocol | `Status`, explicit time, fixed caller-owned audit image | Keypad/display/storage endpoint ownership, rendering, persistence claim, actuator authority |
| 057 | `InertEscapeConsole`, the sole owner of puzzle, panel, precedence, and intent policies | Lessons 055--056 | Door/lock control, generic automation, access decisions, arbitrary clue families, physical actuation |

Plan, pre-implementation stress passes, implementation, tests, canonical Mega
examples, resource probes, HTML, PDF, indexes, and promotion review proceed in
that order. Lesson 057 owns both child policies; the children do not refer to
each other. The project owns the six-family mapping, completion policy, fault
precedence, acknowledgement policy, audit admission, and latch/lamp intent.
There is no shared puzzle framework until another concrete consumer proves one
is needed.

## Shared copied-evidence and time rules

Every external clue, operator-control, or stop observation is copied in full
and retains:

- a nonzero source ID and configuration revision;
- a nonzero session epoch;
- a source sequence;
- the caller-supplied observation time;
- an ordinary `Status`.

Clues additionally retain categorical quality; controls and stop retain their
exact copied command/level payload.

Diagnostics and presentation evidence do not claim external source provenance.
They are internally attributable generation evidence: each diagnostic binds
the exact policy generation that produced it, and each presentation result
binds the exact intent generation it reports. Those bindings are not sensor,
operator, or endpoint provenance.

The copied value is immutable until the next admitted update. There are no
borrowed views, callbacks, references to caller storage, or pointers in live
policy state. A newer sequence with valid modular ordering may advance the
source. An exact duplicate sequence with an identical payload is idempotent.
A changed payload at the same sequence is a contradiction/source fault. A
regressing or exact-half-range sequence or time, future observation, unknown
configuration revision, zero identity, invalid category, or noncanonical
absent field rejects the whole input envelope without mutation.

All time comparisons use the repository unsigned modular half-range rule.
Durations must be strictly below the half range; Lesson 055
`maximumEvidenceAge` may additionally be zero to require same-time evidence. Overflow,
ambiguous ordering, or a backward apparent time rejects atomically; it is never
reinterpreted as immediately stale, stopped, or acknowledged. Each update does
bounded work. There is no heap allocation, recursion, callback, blocking wait,
retry loop, input-sized queue, or catch-up loop.

## Lesson 055: `ClueConstraintModel`

### Fixed domain

Lesson 055 is deliberately project-specific. It has exactly 12 clue slots and
12 rule slots. A rule contains at most four terms and at most four rule
prerequisites. Configuration is copied at construction and immutable after
successful initialization. The model validates identifiers, canonical unused
slots, duplicate terms and prerequisites, term categories, dependency
ordering, and cycles before accepting any observations.

The model consumes only explicit categorical observations. It does not sample
hardware, infer identity, run scripts, parse text, call user code, allocate
rules, or recursively evaluate a graph.

```cpp
#include <stdint.h>

namespace adk {

enum struct ClueCategory : uint8_t
{
    Absent,
    Low,
    Nominal,
    High,
    Active,
    Inactive,
    Match,
    Mismatch
};

enum struct ClueQuality : uint8_t
{
    Invalid,
    Qualified,
    Degraded,
    Stale,
    Contradictory,
    SourceFault,
    TimingFault
};

enum struct ClueTermRelation : uint8_t
{
    Equals,
    NotEquals
};

enum struct ClueRuleDisposition : uint8_t
{
    Unevaluated,
    BlockedByPrerequisite,
    MissingEvidence,
    InvalidEvidence,
    StaleEvidence,
    ContradictoryEvidence,
    Unsatisfied,
    Satisfied
};

enum struct ClueModelDisposition : uint8_t
{
    Uninitialized,
    Incomplete,
    InvalidEvidence,
    StaleEvidence,
    ContradictoryEvidence,
    Solved,
    InvalidConfiguration,
    InternalFault
};

struct ClueSourceIdentity
{
    uint16_t sourceId;
    uint16_t configurationRevision;
    uint32_t sessionEpoch;
};

struct ClueObservation
{
    uint8_t               clueId;
    ClueCategory          category;
    ClueQuality           quality;
    ClueSourceIdentity    source;
    uint32_t              sourceSequence;
    MicrosecondTimePoint  observedAt;
    Status                status;
};

struct ClueTerm
{
    uint8_t           clueId;
    ClueTermRelation  relation;
    ClueCategory      category;
};

struct ClueRuleDefinition
{
    uint8_t   ruleId;
    uint8_t   termCount;
    ClueTerm  terms[4];
    uint8_t   prerequisiteCount;
    uint8_t   prerequisiteRuleIds[4];
};

struct ClueConstraintConfig
{
    uint16_t             configurationRevision;
    uint32_t             instanceEpoch;
    MicrosecondDuration  maximumEvidenceAge;
    uint8_t              clueCount;
    uint8_t              ruleCount;
    ClueSourceIdentity   expectedSources[12];
    ClueRuleDefinition   rules[12];
};

struct ClueEvidenceSnapshot
{
    bool                  present;
    ClueCategory          category;
    ClueQuality           quality;
    ClueSourceIdentity    source;
    uint32_t              sourceSequence;
    MicrosecondTimePoint  observedAt;
    Status                status;
};

struct ClueRuleSnapshot
{
    uint8_t                  ruleId;
    ClueRuleDisposition      disposition;
    uint8_t                  firstBlockingTerm;
    uint8_t                  firstBlockingPrerequisite;
};

struct ClueConstraintSnapshot
{
    uint16_t                  configurationRevision;
    uint32_t                  instanceEpoch;
    uint32_t                  generation;
    uint16_t                  satisfiedRuleMask;
    uint16_t                  blockedRuleMask;
    ClueModelDisposition      disposition;
    Status                    status;
};

struct ClueConstraintUpdate
{
    MicrosecondTimePoint  now;
    uint16_t              observationMask;
    ClueObservation       observations[12];
};

struct ClueConstraintModel
{
    explicit ClueConstraintModel
        (const ClueConstraintConfig& config) noexcept;
    ~ClueConstraintModel () noexcept;

    ClueConstraintModel& operator= (const ClueConstraintModel&) = delete;
    ClueConstraintModel (const ClueConstraintModel&) = delete;
    ClueConstraintModel& operator= (ClueConstraintModel&&) = delete;
    ClueConstraintModel (ClueConstraintModel&&) = delete;

    Status initialize  () noexcept;
    void   shutdown    () noexcept;
    void   reset       () noexcept;
    bool   initialized () const noexcept;

    Status update (const ClueConstraintUpdate& input) noexcept;

    ClueConstraintSnapshot snapshot () const noexcept;
    Result<ClueEvidenceSnapshot>
        evidence (uint8_t clueId) const noexcept;
    Result<ClueRuleSnapshot>
        rule (uint8_t ruleId) const noexcept;

private:
    friend struct InertEscapeConsole;
    struct PreparedUpdate;

    Status preflightUpdate
        (const ClueConstraintUpdate& input,
         PreparedUpdate& prepared) const noexcept;
    void applyPreparedUpdate
        (const PreparedUpdate& prepared) noexcept;
};

} // namespace adk
```

`ClueConstraintConfig` field order is normative. `clueCount` and `ruleCount`
are each in `1..12`; IDs are dense and zero-based inside those counts.
Each configured clue has one exact nonzero expected source identity in
`expectedSources[clueId]`; every admitted observation matches all three
identity fields. An identity change rejects until reset and reinitialization
with a newly constructed configuration. All unused expected sources, rules,
terms, prerequisites, observations, and mask bits must be canonical zero
values. A rule cannot mention itself, duplicate a clue term,
duplicate a prerequisite, refer outside the configured counts, or use an
invalid category. A configuration with a cycle rejects before the object
becomes initialized.

Initialization uses an iterative four-color-equivalent/topological scan over
the fixed 12-rule table; runtime evaluation uses its copied topological order.
Neither operation recurses. Each admitted update first validates all 12 mask
positions and present values, then copies all present observations as one
atomic generation, then evaluates all rules once in topological order. A
rejection leaves every evidence cell, rule result, mask, generation, and
status unchanged.

Array slot `i` corresponds only to clue ID `i`, and observation-mask bit `i`
selects that slot for `i` in `0..11`. A present observation must have
`clueId == i`; absent slots are canonical zero. Observations are never packed
or reordered, so a duplicate clue ID cannot appear in a canonical update.

Within one rule, prerequisite failure precedes term evaluation. For terms,
invalid/source/timing quality precedes stale, stale precedes contradictory,
contradictory precedes missing, and missing precedes ordinary mismatch. The
snapshot publishes the first blocker in definition order only as diagnosis;
all terms are still boundedly classified and the complete satisfied/blocked
masks are deterministic. `maximumEvidenceAge` is accepted in
`[0, half-range)` and rejects the exact half range or larger. It uses `[observedAt,
observedAt + maximumEvidenceAge]`: evidence is fresh exactly at the age bound
and stale one tick later.

`firstBlockingTerm` and `firstBlockingPrerequisite` are `UINT8_MAX` when no
blocker of that kind exists; zero remains a valid index. A zero-term rule is
satisfied exactly when all prerequisites are satisfied, including vacuous
satisfaction when it has none. An invalid `evidence()` or `rule()` ID returns
`Result` failure with `StatusCode::InvalidArgument`, a canonical-zero record,
and no model mutation.
Every failed `Result<T>` in this arc carries a canonical-zero `T`.

Public `update()` wraps the private pure `preflightUpdate()` and then the
infallible `applyPreparedUpdate()`. `PreparedUpdate` is an owner-owned,
lossless transition value: it contains the complete proposed
`ClueConstraintSnapshot`, the complete proposed `ClueEvidenceSnapshot` for
each selected clue, the selected-evidence mask, and all proposed rule state
needed by `applyPreparedUpdate()`. It contains no borrowed pointer, view,
callback, or digest standing in for those values. Only the friend
`InertEscapeConsole` may inspect this private value. The console uses its
proposed snapshot and selected proposed evidence to derive the six family
summaries and the exact panel diagnostic before either child mutates.

The friend seam is pure: `preflightUpdate()` cannot reserve a candidate,
advance a generation, write caller storage, or mutate retained evidence.
`applyPreparedUpdate()` accepts only the exact value produced for the current
owner and lifecycle generation and is infallible after the parent completes
all child and parent checks. This does not create a public graph transaction
API; standalone callers continue to use `update()`.

### Lesson 055 deterministic proof

The host matrix includes:

- zero, minimum, and maximum clue/rule counts; all invalid counts and IDs;
- zero and exact-half-range age; every noncanonical unused slot and mask bit;
- exhaustive directed graphs over four labeled rules, including all `2^12`
  non-self edge masks, proving acceptance exactly for DAGs and stable
  topological evaluation for every accepted graph;
- representative maximum 12-rule chains, diamonds, four-prerequisite joins,
  disconnected graphs, duplicate edges, self-cycles, two-node cycles, and
  long cycles;
- term counts zero through five and prerequisite counts zero through five;
  every four-term relation/category combination used by the project fixture;
- all configured observation masks; every attempted clue-ID/slot mismatch;
  source-sequence permutations across distinct clue slots; repeated identical
  updates at one sequence; and same-sequence changed-payload collisions;
- invalid, qualified, degraded, stale, contradictory, source-fault, and
  timing-fault quality for every term position;
- observation time at one tick before, exactly at, and one tick after now and
  the freshness boundary; rollover and half-range ambiguity;
- mutation of every candidate configuration, observation, provenance, time,
  sequence, quality, status, and canonical padding field;
- failed initialization, repeated initialization, reset, shutdown, destruction,
  and two independent models replaying identical inputs field by field; and
- byte-stable public results by fieldwise comparison, never raw-struct or
  padding comparison.

The canonical example is an “evidence wall”: 12 labeled clue cells feed 12
fixed rules. It visibly separates copied observation, rule evaluation, and
solved policy. No rule completion produces actuator intent.

## Lesson 056: `FaultAwareOperatorPanel`

### Pure panel and presentation intent

`FaultAwareOperatorPanel` is a copied-value policy object. It owns no input
endpoint, keypad, button, display, LED, sounder, bus, clock, storage medium, or
actuator. The caller supplies one atomic input envelope and renders the copied
presentation intent elsewhere. Presentation success or failure is evidence
only; it never changes stop, acknowledgement, audit, or puzzle policy.

Stop is dominant after structural validation. It is level-sensitive copied
evidence, not an emergency stop. A valid asserted stop invalidates pending
acknowledgements, requests stopped presentation, and keeps the panel stopped
until a newer qualified deasserted stop observation is admitted and its exact
`StopReleased` audit record is committed. No
acknowledgement, restart image, display success, key chord, or clue state can
mask or clear it.

Acknowledgement is deliberately limited. It can acknowledge only the exact
currently published acknowledgeable diagnostic generation or exact prepared
audit record. It cannot clear invalid configuration, internal failure, stop,
source/configuration/timing faults, stale/contradictory inputs, audit
indeterminate/corrupt state, or an invalid operator chord. An exact
fieldwise copy of a live acknowledgement preview is accepted; mutated,
foreign-owner, stale-generation, reset-invalidated, shutdown-invalidated, and
consumed copies reject without mutation.

```cpp
namespace adk {

struct InertEscapeConsole;

enum struct OperatorControl : uint8_t
{
    None,
    Previous,
    Next,
    Select,
    Acknowledge
};

enum struct OperatorChordDisposition : uint8_t
{
    None,
    SingleControl,
    InvalidChord,
    InvalidEvidence
};

enum struct PanelDiagnostic : uint8_t
{
    None,
    InputRecovered,
    PresentationRecovered,
    ClueIncomplete,
    ClueInvalid,
    ClueStale,
    ClueContradictory,
    AuditPending,
    AuditIndeterminate,
    OperatorChordInvalid,
    SourceFault,
    ConfigurationFault,
    TimingFault,
    InternalFault,
    Stopped
};

enum struct PanelPresentationMode : uint8_t
{
    Blank,
    Ready,
    Reviewing,
    ConfirmationRequired,
    Solved,
    Fault,
    Stopped
};

enum struct PanelAuditKind : uint8_t
{
    None,
    AcknowledgedDiagnostic,
    PuzzleSolved,
    StopAsserted,
    StopReleased
};

enum struct PanelAuditSlotState : uint8_t
{
    Empty,
    Prepared,
    Committed
};

enum struct PanelAuditDisposition : uint8_t
{
    Empty,
    Ready,
    PrepareRequired,
    AcknowledgeRequired,
    Indeterminate,
    Corrupt
};

struct OperatorSourceIdentity
{
    uint16_t sourceId;
    uint16_t configurationRevision;
    uint32_t sessionEpoch;
};

struct OperatorControlEvidence
{
    uint8_t               pressedMask;
    OperatorSourceIdentity source;
    uint32_t              sourceSequence;
    MicrosecondTimePoint  observedAt;
    Status                status;
};

struct OperatorStopEvidence
{
    bool                   asserted;
    OperatorSourceIdentity source;
    uint32_t               sourceSequence;
    MicrosecondTimePoint   observedAt;
    Status                 status;
};

struct PanelPresentationIntent
{
    PanelPresentationMode  mode;
    PanelDiagnostic        diagnostic;
    uint8_t                selectedCell;
    uint32_t               diagnosticGeneration;
    bool                   acknowledgeAvailable;
};

struct PanelPresentationEvidence
{
    uint32_t               intentGeneration;
    MicrosecondTimePoint   observedAt;
    Status                 status;
};

struct PanelAuditRecord
{
    uint32_t            formatMagic;
    uint16_t            formatVersion;
    uint16_t            configurationRevision;
    uint32_t            instanceEpoch;
    uint32_t            recordSequence;
    uint32_t            operationId;
    PanelAuditKind      kind;
    PanelDiagnostic    diagnostic;
    uint32_t            diagnosticGeneration;
    uint16_t            parentConfigurationRevision;
    uint32_t            parentInstanceEpoch;
    uint32_t            parentGeneration;
    uint32_t            clueGeneration;
    uint16_t            satisfiedRuleMask;
    uint32_t            policyDigest;
    bool                stopPresent;
    bool                stopAsserted;
    OperatorSourceIdentity stopSource;
    uint32_t            stopSourceSequence;
    MicrosecondTimePoint stopObservedAt;
    MicrosecondTimePoint occurredAt;
    uint32_t            payloadDigest;
    uint32_t            checksum;
    PanelAuditSlotState state;
};

struct PanelAuditImage
{
    PanelAuditRecord slots[2];
};

struct PanelAuditPreview
{
    uintptr_t       ownerToken;
    uint32_t        lifecycleGeneration;
    uint16_t        configurationRevision;
    uint32_t        instanceEpoch;
    uint32_t        panelGeneration;
    uint32_t        operationId;
    uint8_t         slotIndex;
    PanelAuditRecord record;
    uint32_t        imageDigest;
};

struct PanelAcknowledgePreview
{
    uintptr_t        ownerToken;
    uint32_t         lifecycleGeneration;
    uint16_t         configurationRevision;
    uint32_t         instanceEpoch;
    uint32_t         panelGeneration;
    uint32_t         operationId;
    PanelDiagnostic  diagnostic;
    uint32_t         diagnosticGeneration;
    PanelAuditPreview audit;
};

struct FaultAwareOperatorPanelConfig
{
    uint16_t             configurationRevision;
    uint32_t             instanceEpoch;
    MicrosecondDuration  maximumInputAge;
    uint8_t              selectableCellCount;
    OperatorSourceIdentity controlSource;
    OperatorSourceIdentity stopSource;
};

struct FaultAwareOperatorPanelInput
{
    MicrosecondTimePoint      now;
    bool                      auditImagePresent;
    PanelAuditImage           auditImage;
    bool                      stopPresent;
    OperatorStopEvidence      stop;
    bool                      controlPresent;
    OperatorControlEvidence   control;
    bool                      diagnosticPresent;
    PanelDiagnostic           diagnostic;
    uint32_t                  diagnosticGeneration;
    bool                      auditAcknowledgePresent;
    PanelAuditPreview         auditAcknowledge;
    bool                      acknowledgePresent;
    PanelAcknowledgePreview   acknowledge;
    bool                      presentationPresent;
    PanelPresentationEvidence presentation;
};

struct FaultAwareOperatorPanelSnapshot
{
    uint16_t                  configurationRevision;
    uint32_t                  instanceEpoch;
    uint32_t                  generation;
    bool                      stopped;
    bool                      stopEvidencePresent;
    OperatorStopEvidence      stopEvidence;
    bool                      stopTransitionPending;
    uint8_t                   selectedCell;
    OperatorChordDisposition  chordDisposition;
    PanelDiagnostic           diagnostic;
    uint32_t                  diagnosticGeneration;
    PanelDiagnostic           externalDiagnostic;
    uint32_t                  externalDiagnosticGeneration;
    PanelDiagnostic           derivedClueDiagnostic;
    uint32_t                  derivedClueGeneration;
    PanelAuditDisposition     auditDisposition;
    PanelPresentationIntent   presentation;
    Status                    status;
};

struct FaultAwareOperatorPanel
{
    FaultAwareOperatorPanel
        (const FaultAwareOperatorPanelConfig& config) noexcept;
    ~FaultAwareOperatorPanel () noexcept;

    FaultAwareOperatorPanel&
        operator= (const FaultAwareOperatorPanel&) = delete;
    FaultAwareOperatorPanel
        (const FaultAwareOperatorPanel&) = delete;
    FaultAwareOperatorPanel&
        operator= (FaultAwareOperatorPanel&&) = delete;
    FaultAwareOperatorPanel
        (FaultAwareOperatorPanel&&) = delete;

    Status initialize  () noexcept;
    void   shutdown    () noexcept;
    void   reset       () noexcept;
    bool   initialized () const noexcept;

    Result<PanelAuditPreview>
        prepareAudit (uint32_t operationId,
                      PanelAuditKind kind,
                      MicrosecondTimePoint now) noexcept;
    bool canAcknowledgeAudit
        (const PanelAuditPreview& preview) const noexcept;
    Result<PanelAcknowledgePreview>
        prepareAcknowledge (uint32_t operationId,
                            MicrosecondTimePoint now) noexcept;
    Status update
        (const FaultAwareOperatorPanelInput& input) noexcept;

    FaultAwareOperatorPanelSnapshot snapshot () const noexcept;
    PanelAuditImage canonicalAuditImage () const noexcept;

private:
    friend struct InertEscapeConsole;
    struct PreparedUpdate;

    Result<PanelAuditPreview>
        preparePuzzleSolved
            (uint32_t operationId,
             uint16_t parentConfigurationRevision,
             uint32_t parentInstanceEpoch,
             uint32_t parentGeneration,
             uint32_t clueGeneration,
             uint16_t satisfiedRuleMask,
             uint32_t policyDigest,
             MicrosecondTimePoint now) noexcept;
    Status preflightUpdate
        (const FaultAwareOperatorPanelInput& input,
         PreparedUpdate& prepared) const noexcept;
    Status preflightProjectUpdate
        (MicrosecondTimePoint now,
         bool auditImagePresent,
         const PanelAuditImage& auditImage,
         bool stopPresent,
         const OperatorStopEvidence& stop,
         bool controlPresent,
         const OperatorControlEvidence& control,
         bool auditAcknowledgePresent,
         const PanelAuditPreview& auditAcknowledge,
         bool acknowledgePresent,
         const PanelAcknowledgePreview& acknowledge,
         bool presentationPresent,
         const PanelPresentationEvidence& presentation,
         PanelDiagnostic derivedClueDiagnostic,
         uint32_t derivedClueGeneration,
         bool puzzleSolveEligible,
         PreparedUpdate& prepared) const noexcept;
    void applyPreparedUpdate
        (const PreparedUpdate& prepared) noexcept;
};

} // namespace adk
```

The configuration and every public aggregate field order shown above is
normative. `selectableCellCount` is in `1..12`. Control and stop sources must
be nonzero and distinct. `pressedMask` uses bits 0--3 for Previous, Next,
Select, and Acknowledge respectively; every other bit must be zero. Zero or
one set bit is canonical. Two or more set bits is an observable invalid chord,
not a priority-selected command.

The API header includes `<stdint.h>` for `uintptr_t`. Every prepared panel
capability binds the exact live object through `ownerToken`; a fieldwise copy
is accepted only while that token and retained generation remain live. Equal
revisions, epochs, generations, and payloads in two simultaneous instances
cannot create a cross-instance capability collision. Reset, shutdown,
consumption, or destruction invalidates the binding. The token is opaque,
never persisted or hashed, and independently constructing or copying a token
from another object grants no authority.

### Caller-owned two-slot canonical audit image

The caller owns exactly one live `PanelAuditImage` for a panel. The panel
copies, validates, and returns complete image values but never performs a
storage read or write. Sharing or concurrently mutating one image across live
panels violates the caller contract; E0 does not invent an undetectable global
registry.

Each record is self-describing and checksummed over a frozen canonical byte
encoding. Empty slots are all-zero. Record sequences use modular half-range
ordering. Every nonempty record has
`formatMagic = UINT32_C(0x41444b41)` and
`formatVersion = UINT16_C(1)`; any other value rejects structurally. The
digest primitive is 32-bit FNV-1a with offset basis
`0x811c9dc5` and prime `0x01000193`. A digest starts at the offset basis,
consumes its domain tag including the terminating zero byte, then consumes the
listed fields in order. Unsigned integers are least-significant byte first at
their declared width; an enum is its underlying `uint8_t`; a `bool` is exactly
`0` or `1`; and a `Status` is its canonical status-code byte. Padding and
object representation are never hashed.

`payloadDigest` uses domain `"ADK.PANEL.PAYLOAD.V1\0"` and, in order:
`operationId`, `kind`, `diagnostic`, `diagnosticGeneration`,
`parentConfigurationRevision`, `parentInstanceEpoch`, `parentGeneration`,
`clueGeneration`, `satisfiedRuleMask`, `policyDigest`, `stopPresent`,
`stopAsserted`, `stopSource.sourceId`,
`stopSource.configurationRevision`, `stopSource.sessionEpoch`,
`stopSourceSequence`, `stopObservedAt`, and `occurredAt`. `checksum` uses
domain `"ADK.PANEL.RECORD.V1\0"` and, in order: `formatMagic`,
`formatVersion`, `configurationRevision`, `instanceEpoch`, `recordSequence`,
`operationId`, `kind`, `diagnostic`, `diagnosticGeneration`, the six parent
solve-binding fields in declaration order, `stopPresent`, `stopAsserted`, the
three `stopSource` fields, `stopSourceSequence`,
`stopObservedAt`, `occurredAt`, `payloadDigest`, and `state`; it excludes only
`checksum`. A record rejects unless both supplied digest values equal their
recomputations.

`imageDigest` uses domain `"ADK.PANEL.IMAGE.V1\0"`, then the slot-index byte
and every record field in declaration order for slot 0, followed by the same
for slot 1. It includes each record's `checksum` and does not recursively
include an image digest. Thus an empty record contributes its index and its
all-zero fields. These algorithms and constants are shared by preparation,
acknowledgement, reconciliation, and tests; no implementation hashes a C++
object representation.

The stop fields are canonical evidence, not descriptive metadata.
`StopAsserted` requires `stopPresent == true`, `stopAsserted == true`, the
exact nonzero configured stop-source identity, and the exact admitted source
sequence and observation time. `StopReleased` requires the same bindings with
`stopAsserted == false`. For every other kind, all stop fields are zero. The
six parent solve-binding fields are nonzero/canonical only for `PuzzleSolved`
and are all zero for every other kind.
Restart release is newer than assertion only when both the unsigned modular
half-range source-sequence comparison and the separate observation-time
comparison say newer; equality, backward order, or exact-half-range ambiguity
cannot clear stop.

The complete image classification is frozen:

| Canonical image form | `PanelAuditDisposition` |
|---|---|
| Both slots empty | `PrepareRequired` |
| One committed record and one empty slot | `Ready` |
| Two adjacent committed records with an unambiguous newest record | `Ready` |
| One empty slot plus the exact retained live sequence-one prepared record | `AcknowledgeRequired` |
| One committed predecessor plus its exact adjacent retained live prepared record | `AcknowledgeRequired` |
| Any record has bad format/configuration/epoch, noncanonical fields, or a digest/checksum failure | `Corrupt` |
| Individually canonical records have a duplicate, gap, backward or half-range-ambiguous relation, two prepared records, or an unattributable prepared record | `Indeterminate` |

`Empty` is reserved for the pre-initialization/no-image snapshot; a present
canonical all-zero image is `PrepareRequired`. A valid live prepared form is
always `AcknowledgeRequired`. Neither `Corrupt` nor `Indeterminate` can be
acknowledged. Two adjacent committed records are `Ready`; the older record is
the next replaceable slot.

There is one exact bootstrap exception. A fully empty image may accept one
canonical `Prepared` record with `recordSequence == 1`, no predecessor, and
the retained live preview's exact operation, payload, checksum, configuration,
epoch, slot, and image digest. Acknowledgement commits that record as the first
audit record. Every later prepared record requires the adjacent committed
predecessor with the exact preceding modular sequence.

`prepareAudit()` chooses the older/empty slot, constructs one complete
`Prepared` record, retains one live candidate, and returns the complete
candidate image digest without changing the caller image. The caller may model
a physical write by copying that record into its image. It then returns the
exact preview through `auditAcknowledge` in the atomic update. Acknowledgement
preflights every preview and currently observed image field and atomically
publishes the same record as `Committed`. A reused preview rejects.

`update()` is the sole restart, torn-write, image-reconciliation,
acknowledgement, and ordinary control ingress. When `auditImagePresent` is
true, it copies and classifies the complete observed two-slot image before any
transition. A valid prepared newest record is deterministically completed only
if its predecessor, or the exact first-record bootstrap exception, plus its
operation, payload, sequence, checksum, configuration, and epoch prove the
single retained transaction. Otherwise the image remains fail-closed as
indeterminate/corrupt. Image reconciliation never invents an audit record,
clears stop, acknowledges a diagnostic, or enables actuator intent.

Audit creation is closed by kind; `prepareAudit()` is not a generic event
writer. The admission and binding table is normative:

| `PanelAuditKind` | Admitting operation and exact binding |
|---|---|
| `None` | Never admitted. |
| `AcknowledgedDiagnostic` | Panel-only while committing the exact live acknowledgement for the currently published `InputRecovered` or `PresentationRecovered` generation. It binds that diagnostic and generation; all parent solve-binding and stop fields are zero. |
| `PuzzleSolved` | Parent-only through private friend-only `preparePuzzleSolved()` called by `InertEscapeConsole::prepareSolve`; every public `prepareAudit()` attempt rejects. It binds the exact parent configuration revision, instance epoch, generation, operation, solved clue generation, satisfied-rule mask, and policy digest; diagnostic and stop fields are zero. |
| `StopAsserted` | Panel-only after admission of the exact qualified released-to-asserted transition. It binds the configured source identity, sequence, observation time, and asserted level. A repeated level is not a transition and rejects; all parent solve-binding fields are zero. |
| `StopReleased` | Panel-only after admission of the exact qualified asserted-to-deasserted transition newer than the retained assertion under both comparisons above. It binds that release evidence; stop remains effective until this record commits, and all parent solve-binding fields are zero. |

Every kind not admitted by its named transition rejects without retaining a
candidate. Public `prepareAudit()` always rejects `PuzzleSolved`; only the
friend-only private `preparePuzzleSolved()` path accepts its exact parent
bindings. An implementation may share a lower private preparation helper, but
no public call may manufacture an arbitrary kind, diagnostic, or payload.
`reset()` and `shutdown()` are exact lifecycle boundaries, not audit kinds:
each invalidates every retained audit and acknowledgement candidate before it
clears volatile panel state. Neither may manufacture, commit, or reconcile an
audit record because neither receives an operation, time, or caller image.

The canonical image uses both slots indefinitely by replacing only the older
committed record after a successfully acknowledged newer record.
`AcknowledgeRequired` is reported while the exact attributable prepared
transaction occupies that replacement slot, so a second preparation cannot
start until acknowledgement or reconciliation. Two adjacent committed records
are `Ready`, and their unambiguously older record is replaceable. Ordering
ambiguity is `Indeterminate`. This is a bounded latest-two-record policy, not
an append-only history or physical durability claim.

### Atomic panel update and precedence

`update()` is the panel’s sole ordinary ingress. It first validates every
presence flag, canonical absent value, source, identity, time, operation, audit
preview, acknowledgement preview, and presentation evidence without mutation.
An envelope with both `auditAcknowledgePresent` and `acknowledgePresent`
rejects atomically; neither acknowledgement receives priority. Public
`update()` uses the same private pure `preflightUpdate()` plus infallible
`applyPreparedUpdate()` seam available only to the friend parent, after the
parent has preflighted its complete envelope.

`preflightProjectUpdate()` is a separate friend-only composition seam. On every
accepted parent envelope it replaces the retained parent-derived clue
diagnostic with the supplied canonical value and exact proposed Lesson 055
generation. Supplying `PanelDiagnostic::None` explicitly clears that derived
channel. It does not reuse `FaultAwareOperatorPanelInput::diagnosticPresent`,
does not alter or acknowledge an externally admitted panel diagnostic, and
cannot manufacture `InputRecovered` or `PresentationRecovered`. The parent
may supply only `None`, `ClueIncomplete`, `ClueInvalid`, `ClueStale`,
`ClueContradictory`, `SourceFault`, `ConfigurationFault`, or `TimingFault`;
every other value rejects without mutation. Thus a clue diagnostic cannot
stick after the proposed clue state becomes healthy, impersonate recovery, or
become acknowledgeable through the public panel path. When an external panel
diagnostic and the derived clue channel coexist, fixed precedence selects
the primary `diagnostic`/`diagnosticGeneration` presentation pair while
`externalDiagnostic`/`externalDiagnosticGeneration` and
`derivedClueDiagnostic`/`derivedClueGeneration` preserve both attributions
independently. A derived replacement never edits the external pair.

For a valid envelope it applies:

1. preserve invalid-configuration or internal-fault attribution;
2. apply valid stop evidence and invalidate acknowledgement candidates;
3. classify source/configuration/timing/stale/contradictory evidence;
4. retain audit indeterminate/corrupt state;
5. classify an invalid operator chord;
6. apply at most one exact audit acknowledgement or limited diagnostic
   acknowledgement;
7. apply a single ordinary navigation/control action; and
8. publish presentation intent, then separately retain presentation evidence.

Stop and independently classified lower causes remain observable together,
but the published primary disposition follows that order. A presentation
failure can add diagnostic evidence but cannot rewrite the primary snapshot or
the canonical audit image.

The acknowledgeability table is frozen: only `InputRecovered` and
`PresentationRecovered` are diagnostic-acknowledgeable, and only at their
exact currently published diagnostic generation. Every other
`PanelDiagnostic`, including `None`, is non-acknowledgeable. A successful
diagnostic acknowledgement writes `AcknowledgedDiagnostic`.
`InputRecovered` is generated only when a newer qualified control or stop
observation clears the immediately prior attributable input fault.
`PresentationRecovered` is generated only when successful presentation
evidence for the exact current intent generation follows the immediately prior
attributable presentation failure. Neither may be supplied as an arbitrary
external diagnostic assertion.
An envelope with `diagnosticPresent == true` and either `InputRecovered` or
`PresentationRecovered` is structurally invalid and rejects without mutation;
those values arise only from the internal qualified recovery transitions.

Construction, successful `initialize()`, and `reset()` start with stopped
state released and a blank presentation, but initialization consumes only
configuration and never an audit image. All audit-image admission and
reconciliation occurs through `update()`. On restart, the canonical latest
committed `StopAsserted` record restores stopped state. Only a newer qualified
deasserted stop observation whose exact `StopReleased` audit record is then
committed clears it. A blank image or an image with no attributable stop
record cannot infer release from an earlier assertion.

### Lesson 056 deterministic proof

The host matrix includes:

- all 16 valid low-nibble control masks, every invalid high bit, and all
  permutations/collisions of stop, control, diagnostic, audit
  acknowledgement, diagnostic acknowledgement, and presentation evidence;
- every selectable-cell boundary, Previous/Next wrap policy, Select, no input,
  held sequence duplicate, same-sequence payload collision, and invalid chord;
- asserted/deasserted stop, stop plus every other input, stale stop release,
  stop source/config/session mismatch, restart while stopped, and fresh
  qualified release;
- acknowledgement of every diagnostic, proving only the documented
  `InputRecovered` and `PresentationRecovered` diagnostics at their exact
  generation can clear and every other enum cannot; mutated,
  foreign, stale, consumed, reset, shutdown, and post-stop previews;
- every public `prepareAudit()` kind, proving `PuzzleSolved` always rejects,
  plus the friend-only parent path with every solve-binding field mutated;
- empty, one-committed, adjacent two-committed, prepared-newest, torn-field,
  torn-checksum, bad magic/version/config/epoch, duplicate, gap, rollover,
  ambiguous, and corrupt two-slot images;
- exact empty-image sequence-one bootstrap, every mutated bootstrap field, and
  rejection of every later prepared record without its committed predecessor;
- prepare before image copy, after prepared copy, before acknowledgement,
  after acknowledgement, and interruption at every field/slot boundary;
- reconcile after clean restart, prepared/torn restart, repeated reconcile,
  corrupt image, and a future or backward reconcile time;
- every single-field mutation of audit and acknowledgement candidates,
  independently constructed exact fieldwise copies while live, and candidate
  invalidation after reset/shutdown/consumption;
- freshness one tick before, exactly at, and one tick after the bound; source
  sequence and time rollover, regressions, and exact-half-range ambiguity;
- presentation disabled, delayed, stale, mismatched, successful, and failed,
  proving primary policy and audit results remain fieldwise identical; and
- two panels with disjoint caller-owned images replaying the same trace
  fieldwise, plus explicit documentation that one overlapping live image is a
  caller-contract violation.

The example is a “fault desk”: copied controls navigate 12 cells, stop visibly
dominates, acknowledgement is offered only for an eligible diagnostic, and
the two-slot image is printed before prepare, after modeled write, after
acknowledgement, and after restart reconciliation. Rendering remains caller
work.

## Lesson 057: `InertEscapeConsole`

### Six-family project policy

The project accepts exactly six semantic clue families:

1. `Sequence`;
2. `Pattern`;
3. `Orientation`;
4. `Presence`;
5. `Rhythm`; and
6. `Alignment`.

They are fictional puzzle categories, not hardware types or identity evidence.
No public operation accepts an arbitrary family number, script, expression,
name, callback, endpoint, address, or actuator command. The immutable project
configuration maps exactly two of the 12 clue IDs to each family and maps its
12 Lesson 055 rules to the completion fixture. The project owns this mapping
and all completion, acknowledgement, audit, precedence, and output-intent
policy.

```cpp
namespace adk {

enum struct EscapeClueFamily : uint8_t
{
    Sequence,
    Pattern,
    Orientation,
    Presence,
    Rhythm,
    Alignment
};

enum struct EscapeConsoleDisposition : uint8_t
{
    Uninitialized,
    AwaitingClues,
    AwaitingOperator,
    AuditPending,
    Solved,
    Stopped,
    InvalidOperatorChord,
    AuditIndeterminate,
    InvalidEvidence,
    StaleEvidence,
    ContradictoryEvidence,
    SourceFault,
    ConfigurationFault,
    TimingFault,
    InternalFault
};

enum struct EscapeLatchIntent : uint8_t
{
    Inactive,
    RequestDemonstrationRelease
};

enum struct EscapeLampIntent : uint8_t
{
    Off,
    Ready,
    Progress,
    Confirmation,
    Solved,
    Fault,
    Stopped
};

struct EscapeFamilySnapshot
{
    EscapeClueFamily     family;
    uint8_t              firstClueId;
    uint8_t              secondClueId;
    bool                 complete;
    ClueQuality          weakestQuality;
};

struct EscapeConsoleConfig
{
    uint16_t                      configurationRevision;
    uint32_t                      instanceEpoch;
    ClueConstraintConfig          clueModel;
    FaultAwareOperatorPanelConfig panel;
    EscapeClueFamily              clueFamilies[12];
    uint32_t                      policyDigest;
};

struct EscapeConsolePreview
{
    uintptr_t                 ownerToken;
    uint32_t                  lifecycleGeneration;
    uint16_t                configurationRevision;
    uint32_t                instanceEpoch;
    uint32_t                consoleGeneration;
    uint32_t                operationId;
    uint32_t                clueGeneration;
    uint16_t                satisfiedRuleMask;
    uint32_t                policyDigest;
    PanelAuditPreview       audit;
};

struct EscapeConsoleUpdate
{
    MicrosecondTimePoint           now;
    bool                           auditImagePresent;
    PanelAuditImage                auditImage;
    bool                           clueUpdatePresent;
    ClueConstraintUpdate           clueUpdate;
    bool                           stopPresent;
    OperatorStopEvidence           stop;
    bool                           controlPresent;
    OperatorControlEvidence        control;
    bool                           auditAcknowledgePresent;
    PanelAuditPreview              auditAcknowledge;
    bool                           acknowledgePresent;
    PanelAcknowledgePreview        acknowledge;
    bool                           presentationPresent;
    PanelPresentationEvidence      presentation;
    bool                           solvePreviewPresent;
    EscapeConsolePreview           solvePreview;
};

struct EscapeConsoleSnapshot
{
    uint16_t                  configurationRevision;
    uint32_t                  instanceEpoch;
    uint32_t                  generation;
    uint32_t                  operationId;
    EscapeFamilySnapshot      families[6];
    EscapeConsoleDisposition  disposition;
    EscapeLatchIntent         latchIntent;
    EscapeLampIntent          lampIntent;
    PanelPresentationIntent   presentation;
    PanelAuditDisposition     auditDisposition;
    Status                    status;
};

struct InertEscapeConsole
{
    InertEscapeConsole
        (const EscapeConsoleConfig& config) noexcept;
    ~InertEscapeConsole () noexcept;

    InertEscapeConsole& operator= (const InertEscapeConsole&) = delete;
    InertEscapeConsole (const InertEscapeConsole&) = delete;
    InertEscapeConsole& operator= (InertEscapeConsole&&) = delete;
    InertEscapeConsole (InertEscapeConsole&&) = delete;

    Status initialize  () noexcept;
    void   shutdown    () noexcept;
    void   reset       () noexcept;
    bool   initialized () const noexcept;

    Result<EscapeConsolePreview>
        prepareSolve (uint32_t operationId,
                      MicrosecondTimePoint now) noexcept;
    Result<PanelAuditPreview>
        preparePanelAudit (uint32_t operationId,
                           PanelAuditKind kind,
                           MicrosecondTimePoint now) noexcept;
    Result<PanelAcknowledgePreview>
        preparePanelAcknowledge
            (uint32_t operationId,
             MicrosecondTimePoint now) noexcept;
    bool canCommit
        (const EscapeConsolePreview& preview) const noexcept;
    Status update
        (const EscapeConsoleUpdate& input) noexcept;

    EscapeConsoleSnapshot snapshot () const noexcept;
    ClueConstraintSnapshot clueSnapshot () const noexcept;
    FaultAwareOperatorPanelSnapshot panelSnapshot () const noexcept;
    PanelAuditImage canonicalAuditImage () const noexcept;
};

} // namespace adk
```

The field order shown is normative. `EscapeConsoleConfig` must contain exactly
12 clues and 12 rules. Each family must appear exactly twice. The six family
enumerators above are the only valid values. The configuration, rule graph,
source identities, child revisions/epochs, completion fixture, and
`policyDigest` are cross-validated before either child initializes. Failure
rolls back both children and leaves latch/lamp intent inert/off.

The parent `configurationRevision` and `instanceEpoch` are independently
nonzero. Each child revision and epoch is also independently nonzero; parent
and child values are not required to equal one another. `policyDigest` is
derived rather than caller-selected: initialization recomputes it and rejects
unless the supplied value equals the result. The 32-bit FNV-1a encoding uses
offset `0x811c9dc5`, prime `0x01000193`, and domain
`"ADK.ESCAPE.POLICY.V1\0"`. It then consumes, in declaration order, the
parent revision and epoch; the complete canonical Lesson 055 configuration
(revision, epoch, maximum age, counts, all 12 expected source identities, and
all 12 rules including every used and canonical-unused rule, term, and
prerequisite slot); all 12 family enumerators in clue-slot order; and the
complete Lesson 056 configuration (revision, epoch, maximum input age,
selectable count, and every field of both configured source identities).
Unsigned integers are little-endian at their declared widths, enums use their
underlying `uint8_t`, and booleans are one byte `0` or `1`; padding and object
representation are excluded.

### One atomic envelope and frozen precedence

`EscapeConsoleUpdate` is the sole ordinary ingress for clues, stop, controls,
audit acknowledgement, diagnostic acknowledgement, presentation evidence, and
solve commit. There is no independent `observeClue()`, `stop()`, `acknowledge()`,
`show()`, or actuation call. The coordinator validates the complete envelope
without mutation. It obtains the Lesson 055 private prepared value, derives
the panel diagnostic and six family summaries from that value's complete
proposed snapshot and selected lossless proposed evidence, preflights the
owned panel through `preflightProjectUpdate()`, and then applies at most one
atomic child and parent transition. The parent-derived clue diagnostic is
replaced or explicitly cleared on every accepted envelope; it is not routed
through the public external-diagnostic field and cannot stick or impersonate
recovery.
For composition, the private internal `ProjectUpdateView` adapts the input
references synchronously inside the panel. No input reference survives the
call. The Lesson 057 caller supplies the full
`FaultAwareOperatorPanel::PreparedUpdate` output on its own update stack; the
panel copies every proposed field into that caller-owned value before return,
and no borrow remains afterward. The parent then performs final atomic checks
and passes the same copied value to infallible `applyPreparedUpdate()`. This
exact arrangement explains the measured 951 B maximum synchronous stack while
keeping the object at 1,024 B; it is a private friend adapter, not a public
transaction/view API.
It never derives same-envelope meaning from a hash, borrowed view, or the old
retained clue snapshot.

The two `preparePanel*()` operations are narrow capability forwarders required
because the console owns the panel. `preparePanelAudit()` forwards only an
operation, closed `PanelAuditKind`, and supplied time; it accepts no record
payload, storage image, diagnostic, clue result, or generic event.
`PanelAuditKind::PuzzleSolved` always rejects on this public path.
`preparePanelAcknowledge()` forwards only an operation and supplied time and
returns the panel's closed acknowledgement capability. Neither operation
writes or reconciles storage, commits panel state, changes puzzle truth, or
publishes intent. Every call attempt, including one that returns failure,
first invalidates any retained parent solve candidate and every nested child
audit or acknowledgement candidate before forwarding. This occurs even when
the request is malformed or otherwise fails; only a capability returned by a
successful new forwarding call may then be live. `update()` remains the sole ingress that can apply either
capability, acknowledge an image, commit child state, or change the console
snapshot.

The frozen primary precedence is:

```text
invalid configuration or internal fault
    > stop
    > source/configuration/timing/stale/contradictory evidence
    > audit indeterminate
    > invalid operator chord
    > normal puzzle progress
```

Structurally invalid envelopes reject before that semantic precedence and
cannot be repurposed as stop. Within the evidence tier the deterministic
diagnostic order is source, configuration, timing, stale, then contradiction;
all independently present causes remain in child snapshots. Corrupt audit data
is an internal/configuration fault when its canonical interpretation is
impossible; a valid but unresolved image is audit indeterminate. Stop
forces `latchIntent = Inactive` and `lampIntent = Stopped`. Every fault tier
forces `latchIntent = Inactive` and a distinguishable `Fault` lamp/presentation
intent. Only the normal tier may request progress or solve intent.

Presentation is computed last and cannot affect the primary result. The
project never exposes a relay state, servo angle, PWM duty, pin level, render
buffer, storage operation, or endpoint reference.

One narrower rule applies to the parent-private `PuzzleSolved` transaction:
the owned panel may preflight its candidate, but it cannot consume or commit
that candidate until the complete combined panel/parent preflight proves the
normal solve tier. Same-envelope stop, clue evidence or its derived
diagnostic, presentation fault, invalid operator chord, audit ambiguity, or
any higher-precedence cause suppresses both child consumption and parent solve
publication atomically. Presentation failure still cannot rewrite or erase
the independently retained primary cause; it merely makes a solve transaction
ineligible. Ordinary public stop-transition audit and limited diagnostic
acknowledgement retain the Lesson 056 precedence defined for their own
non-solve envelopes.

### Atomic solve and audit publication

`prepareSolve()` succeeds only when the exact retained Lesson 055 generation
is solved, the panel retains a present, qualified, deasserted stop observation,
there is no prepared or unreconciled stop transition, the panel has no
dominant diagnostic, no candidate is live, and the audit disposition is
either `Ready` or `PrepareRequired`. A default
`FaultAwareOperatorPanelSnapshot::stopped == false` is insufficient: absence
of qualified stop evidence, stale/faulted stop evidence, or a pending
`StopAsserted`/`StopReleased` record rejects. `PrepareRequired` is the canonical empty-image bootstrap:
the child may prepare sequence-one `PuzzleSolved` directly, without requiring
an unrelated manufactured record. `AcknowledgeRequired`, `Indeterminate`, and
`Corrupt` always reject solve preparation. It reserves
one exact child audit candidate and returns a copied parent preview binding:

- parent owner revision/epoch/generation and operation;
- the complete solved clue generation and satisfied-rule mask;
- the immutable policy digest;
- the child audit preview and image digest; and
- the exact solved inputs from which inert latch/lamp/presentation intent is
  recomputed.

The explicit call to `prepareSolve()` is the deliberate operator confirmation
for this inert demonstration. `PanelControl::Select`, selected-cell state, and
any clue observation are not hidden confirmation predicates and cannot
prepare or commit a solve. A caller must make the named preparation call and
later return its exact parent/child capabilities through `update()`.

The explicit preview above carries the binding fields; resulting intent is
recomputed and fieldwise compared during preflight rather than accepted from a
caller. An independently constructed exact fieldwise copy is valid while the
sole retained parent and child candidates remain live. Any mutation, foreign
owner, stale clue/panel generation, stop, new clue evidence, diagnostic,
reset, shutdown, reconcile, prior consumption, or any attempted
`preparePanelAudit()` or `preparePanelAcknowledge()` call invalidates it. The
forwarding attempt invalidates first, so failure cannot leave an older solve
candidate live.

`EscapeConsolePreview::ownerToken` distinguishes exact simultaneously live
console objects, while its nested preview tokens distinguish simultaneously
live child panels. Exact-looking candidates from another simultaneously live
instance reject even if every public revision, epoch, generation, operation,
and digest field collides. Reset, shutdown, and consumption invalidate the
retained candidate within that object lifetime; destruction ends the
lifetime. Reuse of the same storage address for a later reconstructed object
is not an authentication boundary, and these deterministic tokens are not
security capabilities.

Each policy also owns a nonzero `lifecycleGeneration`. It advances before an
actual uninitialized-to-initialized transition, every explicit `reset()`, and
an actual initialized-to-shutdown transition, and is copied into
every private prepared update and public preview. Every candidate comparison
requires an exact match. The value never resets within one object lifetime;
repeated `initialize()` while initialized and repeated `shutdown()` while
already shut down are idempotent and do not advance or invalidate anything.
If advancing would wrap to zero, initialization returns
`StatusCode::CapacityExceeded`; void `reset()` or `shutdown()` still completes
the requested inert transition and publishes `CapacityExceeded` in the
retained snapshot/status. Every wrap case invalidates all candidates and the
object cannot prepare new work. This prevents an old preview from becoming
equal to a later candidate after reset, shutdown, or reinitialization. The
value is not persisted or hashed and makes no cross-reconstruction or security
claim.
Lesson 055 private prepared updates, both Lesson 056 preview types, and the
Lesson 057 parent preview all follow this rule.

The caller models the prepared audit image and returns both the exact child
audit acknowledgement and the exact parent preview through the same atomic
update envelope, with `solvePreviewPresent` selecting `solvePreview`. The
coordinator fieldwise validates every parent binding, the complete child
preview and image digest, the currently observed canonical image, and the
recomputed result intent before mutation. Solve pairing uses both children's
complete **proposed** state, not their retained pre-envelope snapshots. Any
same-envelope change to clue generation, clue fault or derived diagnostic,
control state, panel generation, presentation evidence or intent, stop, or
audit state suppresses solve consumption even when the supplied previews still
match the earlier retained state.

The compact public preview does not duplicate panel generation, diagnostic, or
presentation fields. Equivalent authority comes from its complete nested
`PanelAuditPreview`, the panel's live retained candidate,
`canAcknowledgeAudit()`, and the current proposed child/panel state evaluated
inside the same parent preflight, combined with the parent configuration,
generation, clue generation/mask, and policy digest. The implementation
fieldwise compares the nested preview and every carried parent field, then
requires that the current/proposed panel remains acknowledgeable and normal.
Any intervening panel generation, diagnostic, presentation, control, stop,
audit, reset, or lifecycle change invalidates that current-state check even
though it is not redundantly serialized into `EscapeConsolePreview`.

`canCommit()` checks only retained
parent-candidate liveness and the fields carried directly by
`EscapeConsolePreview`: owner revision/epoch, console generation, operation,
clue generation, satisfied-rule mask, policy digest, and the complete copied
child preview including its image digest. It does not call a child commit,
reconcile an image, inspect current caller storage, or certify caller-owned
audit-image state that it has not received. `update()` alone performs the
complete preview/image pairing and
final commit preflight. A noncanonical absent solve preview or any mutated,
stale, foreign, consumed, or child-mismatched preview rejects the whole
envelope. After
complete parent/child preflight at one supplied time, child audit
acknowledgement is infallible; the parent then
publishes the same operation, `Solved` disposition,
`RequestDemonstrationRelease` latch intent, and `Solved` lamp intent in one
mutation. If implementation cannot preserve this property, promotion stops:
sequential best-effort child mutation may not be called atomic.
In particular, a matching child `PuzzleSolved` acknowledgement remains
pending until the parent has also proved normal solve precedence, including
the absence of a same-envelope clue-generation or fault change, stop, derived
clue diagnostic, control or panel-generation change, presentation change or
fault, invalid chord, or audit ambiguity.

Solved intent is volatile. Reset, restart, shutdown, stop, fault, ambiguous or
corrupt audit image, and a new configuration start with latch intent inactive.
Image reconciliation can recover the audit record but cannot automatically
restore release intent; a fresh qualified clue generation, deasserted stop,
and new operator-confirmed solve transaction are required.

### Lesson 057 deterministic proof

The host matrix includes:

- all assignments of 12 clue slots to six families that exercise missing,
  duplicate, out-of-range, uneven, and exact-two membership; mutation of every
  family and policy-digest field;
- all `2^12` clue presence masks, every canonical fixed-array order, relevant
  source-sequence permutations, same-sequence duplicates, and payload
  collisions against the canonical six-family fixture;
- solved, one-term-missing, one-rule-blocked, degraded, invalid, stale,
  contradictory, source-faulted, and timing-faulted evidence in every family;
- solve preparation with qualified deasserted stop, absent/default stop,
  asserted, stale, source-faulted, and timing-faulted stop, plus pending
  `StopAsserted` and `StopReleased` records; only the first admits;
- `Select`, every other ordinary control, and no control with solved clues,
  proving none prepares or commits a solve and the explicit `prepareSolve()`
  call is the sole deliberate operator confirmation;
- every meaningful permutation of clue update, stop, control, audit
  acknowledgement, diagnostic acknowledgement, and presentation evidence at
  one timestamp, including every presence mask and noncanonical absent value;
- pairwise and maximum collision rows for every precedence tier, proving the
  frozen order is independent of conceptual source order and latch intent is
  inactive for every non-normal tier;
- an exact live parent/child solve pair crossed individually and jointly with
  stop, every clue-derived diagnostic, presentation fault, invalid chord,
  `AcknowledgeRequired`, `Indeterminate`, and `Corrupt`, proving neither solve
  candidate is consumed and no solve record or intent is published;
- the same live solve pair crossed with a same-envelope clue-generation
  advance, healthy-to-fault and fault-to-healthy derived clue diagnostic,
  navigation/control change, panel-generation change, successful and failed
  presentation evidence, and presentation-intent change, proving pairing is
  against proposed child state and every change suppresses solve consumption;
- consecutive accepted parent envelopes that derive fault, the same fault,
  a different fault, and `None`, proving replacement and explicit clearing;
  coexistence with each public external diagnostic; rejection of every
  forbidden friend-only diagnostic value; and proof the derived channel can
  neither emit nor acknowledge either recovery diagnostic;
- all 16 operator masks within the full console, with every invalid chord
  crossed against stop, evidence fault, audit state, solved clues, and
  presentation failure;
- prepare/commit candidate mutation one field at a time; exact fieldwise
  copies, foreign owner, stale clue/panel generation, changed policy digest,
  stopped, reset, reconciled, shutdown, consumed, and reused candidates;
- each public `preparePanelAudit()` kind and
  `preparePanelAcknowledge()`, before and after solve preparation, proving
  closed child capability types only, public `PuzzleSolved` rejection, no
  storage or snapshot mutation, and unconditional parent plus nested-child
  candidate invalidation on successful and failed attempts;
- proposed-family and panel-diagnostic derivation from the Lesson 055 private
  prepared snapshot and selected lossless proposed evidence, including a clue
  that changes family state in the same envelope; mutation of every selected
  evidence field; and proof that retained-old-snapshot, pointer-lifetime, and
  digest-only substitutions are impossible;
- both solve-preview presence states, canonical absent payload, the exact
  parent-plus-child commit pair, and every parent/child cross-mismatch;
- failure injection before each parent preflight, after each child preflight,
  and at every allowed atomic mutation boundary, proving rejection leaves both
  children, audit image, parent generation, and intents unchanged;
- clean, prepared, acknowledged, torn, indeterminate, corrupt, rollover,
  and ambiguous audit images across restart; direct sequence-one solve
  preparation from a canonical empty `PrepareRequired` image; ordinary solve
  from `Ready`; rejection from `AcknowledgeRequired`, `Indeterminate`, and
  `Corrupt`; repeated reconciliation and fresh solve admission after recovery;
- timing at one tick before, exactly at, and one tick after every freshness
  boundary; future evidence, rollover, regression, exact-half-range ambiguity,
  and large elapsed jumps;
- reset, restart, shutdown, and destruction from every phase, proving inert
  latch and off/fault presentation defaults and no automatic release recovery;
- presentation absent, saturated, delayed, mismatched, and failed, proving the same
  primary snapshot and audit image; and
- two independently constructed consoles with disjoint audit images replaying
  the same complete trace field by field, including a second replay after
  restart/reconcile.

The canonical narrative is “six stations, one quiet console.” The learner
predicts which two clue cells feed each family, replays copied categorical
evidence, observes each rule and family become eligible, then deliberately
prepares and acknowledges the solve audit record. Stop, a stale clue, a
contradiction, an invalid chord, and a torn audit image each visibly keep
latch intent inactive. “Release” is always labeled demonstration intent, never
a door or access decision.

## Lifecycle, failure, and bounded-work contract

All three objects are inert after construction, non-copyable, and non-movable.
`initialize()` validates immutable configuration only and publishes a blank,
released, image-unreconciled state; it never accepts or rewrites a
caller-owned image. Every image is admitted and reconciled through `update()`.
Repeated initialization succeeds without resetting evidence. A partial child
initialization failure rolls back in reverse order and leaves every intent
inert. `shutdown()` is idempotent, invalidates every candidate, clears volatile
solve authority, and requests inactive/off intent. Destruction calls
`shutdown()`. `reset()` retains immutable configuration but clears volatile
observations, candidates, selection, solve authority, and intent; it does not
erase or fabricate caller-owned audit records.

No cleanup invokes caller code. No operation throws, allocates, blocks, owns a
clock, or reads a global. Each Lesson 055 update validates at most 12 copied
observations, then evaluates at most 12 rules × (4 prerequisites + 4 terms).
Each Lesson 056 update validates one fixed envelope and at most two audit
records. Each Lesson 057 update performs at most one bounded child update,
one panel transition, one candidate transition, and one fixed six-family
summary. Runtime graph traversal is iterative and fixed; no recursive DFS is
permitted.

The Lesson 055 private prepared value adds one complete proposed clue snapshot,
up to 12 selected complete evidence values, and bounded proposed rule state to
the largest synchronous Lesson 057 preflight path. The resource probe counts
that value in full. It also counts the caller-owned full panel prepared output,
private synchronous `ProjectUpdateView`, nested exact panel audit preview, the
live current-state/`canAcknowledgeAudit()` checks, and the panel snapshot's
separate external and derived diagnostic pairs. Those semantic checks may not
be replaced by a digest merely because the compact parent preview avoids
duplicating them. The two panel forwarding methods
add no persistent image or buffer and may retain only the panel's already
budgeted candidate. If this
path misses the stack hard ceiling or residual-SRAM gate, remediation must
reduce copied value shapes or split bounded internal work without borrowing
caller memory, replacing evidence with a hash, or weakening atomicity.

`Status` reports invocation/configuration success or failure. Domain
dispositions remain in snapshots and are not collapsed into transport status.
A semantically stale clue can therefore be a successfully admitted update
whose snapshot says `StaleEvidence`; a structurally invalid envelope returns a
non-OK status and leaves the prior snapshot unchanged.

## Maximum-composition collision matrix

The complete E0 composition gate uses the canonical maximum fixture: 12 clues,
12 rules, four terms and four prerequisites in the exercised join rules, all
six clue families, both live children, one caller-owned two-slot audit image,
one retained solve candidate, full presentation intent, diagnostics, and trace
export. It proves:

| Collision | Required result |
|---|---|
| Invalid config/internal + every other cause | Internal/configuration fault; no child mutation or release intent |
| Stop + valid/malformed/stale/contradictory clue + audit/invalid chord/solve | Stop primary, independent lower evidence retained, latch inactive |
| Source/config/timing/stale/contradiction + audit/invalid chord/solve | Evidence tier primary in fixed suborder, latch inactive |
| Audit indeterminate + invalid chord/solve | Audit tier primary, chord retained diagnostically, latch inactive |
| Invalid chord + otherwise solved | Invalid chord primary, no acknowledgement or release |
| Normal solved + exact live audit acknowledgement | One atomic solved audit/result publication and inert release intent |
| Presentation absent/delayed/failed in every row | Primary cause unchanged; failure suppresses solve-candidate consumption, and the canonical audit image remains unchanged |

Every valid-payload presence mask is exercised. For co-inputs whose conceptual
arrival order could differ, tests enumerate every permutation used to assemble
the same canonical envelope and require one fieldwise result. Invalid absent
payloads are separate atomic-rejection cases, not collision rows.

## Examples and learner narrative

Each canonical Mega example uses public `<Adk.h>` values only and owns no
hardware at E0.

### Lesson 055

Objects appear as fixture values, configuration, then
`ClueConstraintModel`. `setup()` reads acquire, configure, start. `loop()`
reads:

```text
observe copied clue cells
decide which rules are satisfied
actuate the evidence-wall presentation intent
```

The presentation is a printed or compile-time replay value, not a display
endpoint. The example shows a four-rule DAG first and the full 12-rule fixture
second, with provenance, observation time, sequence, and categorical quality
visible.

### Lesson 056

Objects appear as the caller-owned two-slot image, panel configuration, then
`FaultAwareOperatorPanel`. The high-level flow precedes audit mechanics:

```text
observe copied stop and controls
decide navigation or limited acknowledgement
actuate presentation intent
```

The example separately demonstrates `prepareAudit`, the caller’s modeled image
copy, exact acknowledgement in the atomic envelope, and restart image
reconciliation through `update()`. It predicts and shows that stop beats
acknowledgement and display failure changes no primary result.

### Lesson 057

Objects appear as the caller-owned audit image, complete child/project
configuration, then `InertEscapeConsole`. The console owns both child policies.
The example reads:

```text
observe the six copied clue families and operator envelope
decide puzzle, fault, stop, and audit policy
actuate inert latch, lamp, and presentation intent
```

The staged replay covers incomplete clues, all families eligible, operator
confirmation, prepared audit image, acknowledged solve, stop, contradiction,
torn restart image, `update()` reconciliation, and fresh recovery. No helper
is named `unlockDoor`; the vocabulary is `requestDemonstrationRelease`.
It also bootstraps the first solve record directly from the canonical empty
audit image, then demonstrates ordinary panel audit and diagnostic
acknowledgement preparation through the console's narrow forwarders.
Preparation alone changes neither puzzle progress nor the image, and even a
failed forwarding attempt invalidates an earlier solve preview; the caller
must prepare a fresh solve and return its exact capabilities through
`update()`.

Each example has a corresponding host replay using exactly the same fixture
values and timestamps. Serial may describe results, but it is not credited as
the circuit observation path at E1 or E2.

## Circuit-native experiments and staged hardware gates

E0 has zero hardware. Its non-Serial learning outcome is the deterministic
fieldwise host/compile-time replay artifact and rendered pencil worksheet; it
is explicitly not circuit evidence.

At E1, separately qualified passive clue inputs and operator controls feed a
presentation-only fixture. The required non-Serial paths are:

- a named progress indicator for ordinary clue progress;
- a distinguishable fault indicator for invalid/stale/contradictory evidence;
- a distinct stopped indication; and
- a presentation surface showing the selected cell and audit state.

The E1 lesson tells the learner to predict each signal, observe it at the named
LED/display/test point and supplied time, then interpret only presentation and
input qualification. Resource acquisition is proved separately from the
inactive actuator path. The servo, relay, latch, door, and powered load remain
physically absent.

E2, if later admitted, uses either a restrained no-load demonstration servo or
an inert current-limited relay-and-lamp fixture. It requires exact specimens,
primary sources, authoritative schematic, separate logic/load supplies where
applicable, driver protection, measured current, guarded travel, independent
physical load-power removal, and a signed bench record. It separately proves:

1. endpoint/resource acquisition;
2. inactive electrical state at startup and rollback;
3. bounded demonstration intent under the normal solved trace;
4. physical inactivity after stop, fault, missed service, reset, shutdown,
   destruction, logic-power loss, and load-power removal; and
5. presentation failure cannot retain physical actuation.

No E2 fixture attaches to a door, lock, gate, occupied enclosure, alarm,
emergency light, egress route, or life-safety system. Software stop remains a
policy input; removal of load power is the physical stop method.

## HTML and PDF publication map

HTML is the concise searchable reference. Each page links directly to its
public header, out-of-line implementation, focused tests, canonical example
and sketch download, PDF, previous/next lesson, and related child/project
references. It documents exact aggregate field order, lifecycle, precedence,
fixed capacities, status versus disposition, E0 limits, and “Use with”
relationships. Lesson 057 prominently repeats that the console is not access
control, confinement, egress, or life-safety equipment.

The PDFs are richer printable experiments:

| Lesson | Pencil visual and worksheet | Formal schematic |
|---:|---|---|
| 055 | Hand-drawn evidence wall; four-node DAG; 12-cell provenance sheet; freshness timeline; cycle diagnosis | None at E0 |
| 056 | Hand-drawn fault desk; stop-dominance flow; two-slot prepare/ack/reconcile sequence; torn-image diagnosis | None at E0 |
| 057 | Hand-drawn six-station console; one-envelope precedence funnel; atomic solve/audit sequence; E0/E1/E2 boundary plate | None at E0 |

Every listed E0 visual is preceded by `% ADK visual: pencil` and must visibly
use the pencil presentation, not merely grayscale or a pencil filename.
Timing, state, audit, and precedence diagrams are not schematic exceptions.
An E1/E2 formal electrical schematic is added only after exact fixtures are
qualified; it must use conventional symbols, be explicitly identified as
electrically authoritative, and carry `% ADK visual: schematic`. Pencil wiring
or orientation art still includes an adjacent pin-by-pin connection list.

Each PDF includes prediction, named observation place/time, interpretation,
fault diagnosis, exercises, reciprocal HTML/sketch links, metadata, contextual
alternative text, searchable code/text, and a blank staged bench worksheet.
E0 worksheets are labeled deterministic policy evidence, not hardware
acceptance. Publication runs `make lessons`, `make lessons-check`, strict site
validation, rendered grayscale/high-zoom inspection, font/text/link checks,
and the repository PDF-policy review.

## Resource budgets and mandatory probes

These are promotion gates, not estimated permission to exceed Mega 2560
capacity:

| Boundary | Flash target / hard | Static SRAM target / hard | Stack target / hard | Object target / hard |
|---|---:|---:|---:|---:|
| Lesson 055 standalone maximum | 16 / 20 KiB | 1,536 / 2,048 B | 384 / 512 B | 512 / 640 B |
| Lesson 056 standalone maximum | 24 / 28 KiB | 2,560 / 3,072 B | 640 / 768 B | 384 / 512 B |
| Lesson 057 complete maximum composition | 32 / 40 KiB | 4,096 / 4,608 B | 1,024 / 1,280 B | 1,024 / 1,280 B |

The first AVR layout/stack gate produced actionable pre-promotion evidence:

| Boundary | Initial object | Initial synchronous stack | Disposition |
|---|---:|---:|---|
| Lesson 055 | 636 B | 412 B | Both hard gates pass; both targets miss |
| Lesson 056 | 496 B | 745 B | Both hard gates pass; both targets miss |
| Lesson 057, before composition refactor | 1,348 B | 1,967 B | Both 1,280 B hard gates fail |

These are remediation inputs, not final acceptance measurements. The Lesson
057 hard failures trigger a compact retained parent candidate plus a
friend-only `preflightProjectUpdate()` seam. Its private `ProjectUpdateView`
borrows input references only synchronously inside the call, while the Lesson
057 caller owns the full copied `PreparedUpdate` output on its stack. No input
borrow survives return. Canonical state, candidates, prepared output, and
applied transitions remain owned values. This
repair neither weakens the public API nor replaces Lesson 055's lossless
owner-owned prepared value, and it does not authorize partial mutation. At
this initial-review boundary, final object, stack, static-SRAM, flash, and
residual-margin measurements were pending the post-refactor rerun recorded
next.

The stabilized full gate records:

| Lesson | Flash | Static SRAM | Stack | Object | Residual |
|---:|---:|---:|---:|---:|---:|
| 055 | 8,886 B | 1,261 B | 412 B | 636 B | Not an aggregate gate |
| 056 | 18,118 B | 1,454 B | 569 B | 365 B | Not an aggregate gate |
| 057 | 34,978 B | 3,655 B | 951 B | 1,024 B | 3,458 B |

Lesson 056 passes every target after remediation and needs no target-miss
review. Lesson 055's object and stack miss their 512 B and 384 B targets while
passing 640 B and 512 B hard ceilings.

The contract-preserving compact-parent refactor achieves the exact 1,024 B
Lesson 057 object target while retaining child ownership, exact public
snapshots, candidate fieldwise validation, lifecycle invalidation, atomic
preflight/apply, and the synchronous nonretained friend view. Stack, static
SRAM, object, and residual SRAM pass without review. Flash is 34,978 B: a
2,210 B target miss with 5,982 B remaining to the 40 KiB hard ceiling.

The compact representation costs 772 B more linked flash than the superseded
precompact 34,206 B build. Independent review attributes the retained code
primarily to required bounded behavior: approximately 6,504 B in panel
preflight, 2,618 B in parent update, and 2,012 B in the maximum fixture loop.
No dead dispatch table or bounded 2,210 B cleanup was found. Removing enough
code to meet the flash target would weaken validation, recovery, candidate
atomicity, or the canonical maximum fixture. The flash target miss is
therefore accepted; the 40 KiB hard gate and 2,048 B residual gate remain
non-reviewable.

The resource mechanism carries one exact reviewed marker per target-miss
metric and stale-fails when code, compiler/core version, flags, fixture, or
linked inventory changes it. These controlling-authority anchors are
intentional machine-readable lines:

Resource-review: lesson=055 metric=object observed=636 target=512 hard=640 disposition=accepted-target-miss

Resource-review: lesson=055 metric=synchronous_stack observed=412 target=384 hard=512 disposition=accepted-target-miss

Resource-review: lesson=057 metric=flash observed=34978 target=32768 hard=40960 disposition=accepted-target-miss

Target passes need no review marker. Object, stack, static-SRAM, flash,
per-buffer, and 2,048 B residual **hard** failures are never reviewable
exceptions.

The Lesson 055 object gate is intentionally larger than an ordinary reusable
component. The initial complete AVR object probe measured 636 B for the
lossless private state required by twelve copied rule definitions, twelve
complete evidence records, runtime results, topology, and metadata, superseding
an earlier partial-layout estimate of 527 B. Borrowing roughly 480 B of caller
storage would reduce the nominal object while leaving aggregate SRAM nearly
unchanged and introducing alias/lifetime coupling; reducing capacity would
change the curriculum. The reviewed bounded remediation therefore keeps
ownership explicit and raises the target/hard gate to 512/640 B. The target
miss is accepted for bounded local size remediation while the 640 B hard
ceiling passes. Because Lesson 057 owns that child, its coordinator gate
remains 1,024/1,280 B; the compact-candidate/nonretained-view repair is
complete at the exact 1,024 B target.

Lesson 057 must leave at least 2,048 B after maximum measured static SRAM plus
the conservative maximum stack/interrupt reserve:

```text
8192 - measuredStaticSram - measuredStackAndInterruptReserve >= 2048
```

This independent margin can block promotion even when the separate static and
stack hard ceilings pass. Every caller-owned fixed buffer or image is at most
512 B; the two-slot canonical audit image and every optional trace/export
buffer are reported separately and included in aggregate SRAM. No
implementation may split one logical buffer into hidden pieces to evade that
limit.

The six canonical solve-binding fields add 20 logical bytes to each audit
record before ABI padding. The mandatory AVR probe includes that cost in the
record, two-slot image, previews, panel object, and maximum composition.

Before public shape freezes and again before promotion, a compile-and-run
resource probe must report:

1. `sizeof`, alignment, copy/move traits, and fieldwise fixture sizes for every
   public configuration, observation, update, preview, snapshot, rule, audit
   record, audit image, and intent;
2. each reusable Lesson 055 and Lesson 056 object excluding caller-owned
   storage;
3. Lesson 057 with both owned children and its one caller-owned audit image;
4. each standalone linked canonical Mega example’s flash and static SRAM;
5. the maximum linked Lesson 055--057 composition with full fixture,
   presentation, diagnostics, trace/export buffer, and any existing runtime
   overhead that is live in the canonical example;
6. the largest synchronous validate/copy/evaluate/prepare/update/reconcile
   path, plus interrupt reserve and remaining Mega SRAM; and
7. proof that no symbol introduces allocation, recursion, callback storage, a
   hidden clock, or endpoint ownership.

Host executable size is not a substitute for the linked Mega probe. Stack
evidence may use reviewed compiler stack reports plus a conservative call-path
sum; it cannot quote only a leaf function. Measurements enter the lesson
acceptance record with tool/core version and exact command. A target miss
requires review and a size-focused repair; any hard-ceiling or 2,048 B margin
miss blocks promotion pending a durable budget decision.

## Architecture strain and bounded remediation

The design intentionally keeps strain local:

- Lesson 055 is project-specific instead of a premature generic constraint
  solver.
- Lesson 056 owns policy values but no endpoints, rendering, or storage.
- The caller-owned two-slot image makes interruption/restart replay explicit
  without pretending RAM is durable media.
- Lesson 057 owns cross-child precedence and the only atomic envelope.
- Latch and lamp results remain inert semantic intent at E0.

Before implementation, the stress pass must exercise the exact maximum
composition above. If `ClueConstraintModel` cannot fit 12/12 with four
terms/prerequisites, first use compact indices/masks and iterative fixed scans
without changing public semantics. If the panel/audit object strains SRAM,
keep the at-most-512 B image caller-owned, retain only one candidate, remove
duplicated derived snapshots, and compute bounded presentation intent from
canonical state. If Lesson 057 strains stack, change large by-value private
temporaries to caller-owned or object-resident fixed scratch with exclusive
lifetime and atomic preflight; do not add heap allocation, globals, borrowed
durable views, or partial mutation.

Stop implementation and discuss broader remediation if any of these becomes
necessary:

- a generic solver, scripting language, dynamic rule graph, callback, heap,
  recursion, or input-sized work;
- a clue family beyond the six fixed semantic families;
- child-owned endpoints, display rendering, physical storage, clock, or
  actuator resources;
- more than one ordinary project ingress or an order-dependent collision
  result;
- acknowledgement that can override stop or a non-acknowledgeable fault;
- audit recovery that guesses, silently discards an unresolved record, or
  claims physical durability;
- sequential child mutation that cannot be fully preflighted and committed as
  one parent operation;
- a raw servo/relay/pin command or physical latch state in the E0 API;
- use as access control, security, confinement, emergency egress, interlock,
  alarm, or life-safety equipment;
- a fixed buffer over 512 B, object/hard resource ceiling violation, or less
  than 2,048 B measured post-stack SRAM; or
- hardware behavior needed to substantiate an E0 claim.

A local repair is allowed only when it preserves the exact public behavior,
precedence, copied provenance, bounded capacities, replay, and prior
architectural decisions. A new shared abstraction, changed status/lifecycle
contract, different ownership rule, relaxed safety boundary, compatibility
change, or budget exception requires affected-consumer analysis, alternatives,
migration/resource costs, user discussion, and an immutable durable decision
before continuing.

## Acceptance and promotion boundary

E0 implementation is permitted only after the three pre-implementation
component stress passes approve these exact contracts and the resource probe
demonstrates credible target fit. Promotion additionally requires:

- strict standalone-header, style, ordinary, exception, sanitizer, trait,
  lifecycle, rollback, capacity, deterministic replay, and full collision
  tests;
- compile-only canonical Mega examples and measured resource evidence within
  every hard gate and post-stack margin;
- fieldwise replay comparisons, never serialization/hash/comparison of raw C++
  struct representation or padding;
- clean terminal architecture stress passes over the maximum composition;
- complete HTML/PDF/pencil/site/link/sketch-download review; and
- explicit recording that physical acceptance remains open.

E1 remains blocked on exact passive inputs and presentation specimens,
markings, primary sources, voltage/current/polarity/pull and bus qualification,
authoritative schematic, complete pin/interrupt/timer/bus/power/resource
budget, unpowered inspection, failure injection, circuit-native observations,
and signed bench evidence with every actuator absent. Audit images remain
copied E0 values at this stage; qualifying physical nonvolatile storage is not
part of the E1 arc.

E2 remains blocked on the exact demonstration actuator/driver/load, ratings,
separate current-limited load supply, protection, guarded geometry,
independent physical power removal, complete simultaneous-load and thermal
budget, and signed acceptance for startup, stop, fault, reset, shutdown,
destruction, communication loss, stall/jam where applicable, logic-power loss,
and load-power removal. No result may be marked hardware verified without that
record.
