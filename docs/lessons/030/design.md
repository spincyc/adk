# Lesson 030 readiness: inert show-cue simulator

Status: implemented and host verified; the E1 physical acceptance card remains
open.

## Purpose and boundary

Lesson 030 is an E0/E1 composition project. It combines the synthetic channel
assessments from Lesson 028 with the reviewed, deterministic cue schedule and
audit values from Lesson 029. Its only physical outputs are resistor-limited
LEDs, an optional passive piezo, and an optional character display. A visible
cue is a teaching-state presentation, not an actuation request.

The project must not export a generic output sink, callback, protocol field,
pin mapping, waveform, transmitter, launcher adapter, or electrical continuity
claim. It must not connect to a relay, transistor load driver, initiator,
commercial show connector, antenna, or unknown equipment. USB power removal is
the physical stop; software stop and RAII cleanup are safe-state behavior, not
an emergency stop.

## Readiness decisions

Lesson 029 resolved the scheduler and audit rules below. Lesson 030 must
preserve them while adding composition:

1. The scheduler owns a validated copy of its fixed-capacity plan. A caller
   cannot mutate a plan after initialization.
2. Every cue explicitly names one `InertChannelId`. Cue IDs remain opaque
   display labels and are never inferred to be channel IDs.
3. Cancel and an invalid input chord dominate every simultaneous event.
   Review-gate release dominates confirmation, run, and skip.
4. A due cue never becomes active until a fresh explicit confirmation is
   accepted inside its window. A delayed update never catches up by activating
   an elapsed cue.
5. Releasing review while a cue is visible hides it in that update and records
   the transition. Recovery requires fresh review and confirmation.
6. The audit buffer reserves capacity for a final `Shutdown` entry before a
   run can begin. If the next transition cannot be recorded, no corresponding
   state change or visible cue occurs.
7. The scheduler and capstone use one canonical owner for audit insertion.
   The capstone must not duplicate entries already produced by the scheduler.
8. A complete input frame is accepted once. Repeating an identical frame at
   the same timestamp is idempotent; changing it at that timestamp is rejected.

These are dependency corrections, not capstone-local workarounds.

### Reviewed implementation corrections

The pre-implementation audit resolved three gaps in the earlier sketch:

1. Lesson 029 remains independent of Lesson 028. Lesson 030 copies an
   `InertCueChannelMap` indexed by plan position. Its count must equal the
   scheduler's read-only configured cue count, and every mapped channel must
   be in the assessor's 0--7 range. Cue IDs and channel IDs deliberately differ
   in fixtures and are never inferred from each other.
2. The scheduler gains a channel-agnostic evidence gate with `Permit`, `Hold`,
   and `Fault` dispositions plus a status value. The scheduler remains the sole
   cue-audit writer. Lesson 030 derives the disposition from an assessment; it
   does not synthesize button input or append duplicate audit entries.
3. Dependencies are already constructed but uninitialized. While initialized,
   the simulator exclusively coordinates their active lifecycle: assessor,
   then scheduler (which initializes the audit), with reverse rollback.
   Shutdown stops the scheduler, then assessor, and deliberately leaves the
   scheduler-owned audit session readable.

The total precedence for a newly accepted frame is cancel, invalid operator
chord, evidence fault or hold, review release, phase action, then timed
transition. Full-frame identity is validated before that precedence: an
identical frame at the same timestamp is idempotent, while any changed
same-timestamp frame is rejected. A complete observation frame means exactly
all eight unique channel IDs; input order is canonicalized by channel for
identity and digest calculation.

## Public composition API

Prefer one value-oriented coordinator with explicit non-owning references to
already initialized dependency objects:

```cpp
enum struct InertShowState : uint8_t
{
    Startup,
    Review,
    Ready,
    Running,
    Held,
    Complete,
    Cancelled,
    Fault
};

enum struct InertShowFault : uint8_t
{
    None,
    ObservationOpen,
    ObservationShortSimulated,
    ObservationUnavailable,
    ObservationStale,
    ObservationContradictory,
    AuditFull,
    InvalidInput,
    InternalInvariant
};

struct InertShowInput
{
    const InertChannelObservation* observations;
    uint8_t                        observationCount;
    CueOperatorInput               operatorInput;
};

struct InertCueChannelMap
{
    InertChannelId channels[InertCuePlan::capacity];
    uint8_t        count;
};

struct InertShowSnapshot
{
    InertShowState         state;
    InertShowFault         fault;
    InertChannelAssessment selectedChannel;
    CueSchedulerSnapshot   schedule;
    uint32_t               auditSequence;
    uint32_t               traceDigest;
    Status                 status;
    bool                   hasSelectedChannel;
};

struct InertShowSimulator
{
    InertShowSimulator (const InertCueChannelMap& map,
                        InertChannelAssessor& assessor,
                        InertCueScheduler&     scheduler,
                        CueAuditBuffer&        audit) noexcept;
    ~InertShowSimulator () noexcept;

    InertShowSimulator             (const InertShowSimulator&) = delete;
    InertShowSimulator& operator=  (const InertShowSimulator&) = delete;

    Status            initialize  () noexcept;
    void              shutdown    () noexcept;
    bool              initialized () const noexcept;
    Status            update      (TimePoint now,
                                   const InertShowInput& input) noexcept;
    InertShowSnapshot snapshot    () const noexcept;
};
```

Use one `update()` transaction instead of a public `observe()`/`decide()` split.
The split creates an externally visible half-frame and makes shutdown,
same-timestamp identity, and audit atomicity harder to specify. Private
`observeFrame()` and `decideFrame()` helpers may preserve the instructional
argument without exposing partial state.

The simulator contains no endpoints and performs no I/O. It copies the map and
coordinates the assessor and scheduler by reference; the audit reference is
read-only observation of the scheduler-owned ledger. All referenced lifetimes
must exceed its lifetime. Construction is inert, no heap is used, and
initialization is all-or-none. The coordinator is non-copyable and non-movable.
`shutdown()` first forces the scheduler's no-visible-cue state and canonical
Shutdown record, then shuts the assessor down. The ledger remains readable
until its external storage owner explicitly clears it. Repeated shutdown and
destruction are safe.

The exact `InertShowInput` storage rule must be documented: `update()` consumes
the pointed-to observations synchronously and retains no pointer. The frame
must contain exactly all eight unique channels. Null storage, another count,
an invalid or duplicate channel, an invalid observation, or an ambiguous
timestamp rejects the frame according to the precedence above.

## Composition rule

For the scheduler's pending cue:

- `Closed` permits confirmation and visible activation;
- `Open` and `ShortSimulated` hold with distinct nonterminal conditions;
- `Stale` and `Unavailable` hold and require a fresh complete frame;
- `Contradictory` enters a latched fault;
- cancel, invalid input, internal invariant failure, and audit exhaustion
  guarantee that no cue is visible.

Observation recovery never resumes a cue automatically. It returns to review,
where the operator must inspect and confirm again. Terminal cancel and
invariant faults require reinitialization.

## Deterministic proof matrix

Host tests must cover:

- construction, successful and repeated initialization, dependency failure at
  every step, reverse rollback, repeated shutdown, destruction while active,
  and reinitialization after terminal states;
- copy/move traits and dependency lifetime assumptions;
- each channel assessment for every scheduler phase;
- selected cue-to-channel mapping when cue and channel IDs differ;
- exact confirmation-window boundaries, review release immediately before and
  during visibility, skip, cancel, and invalid chords;
- simultaneous-event precedence, including cancel plus confirm and review
  release plus confirm;
- audit one-before-full, the reserved shutdown slot, full capacity, and proof
  that an unrecordable transition produces no state or output transition;
- complete-frame rejection for null storage, wrong count, duplicate,
  missing, invalid, future, changed-same-time, and replayed frames;
- identical full-frame same-timestamp replay, half-range rejection,
  timestamp rollover, and delayed updates with no catch-up activation;
- recovery only through renewed review and confirmation;
- shutdown from every state with a snapshot that requests no visible cue;
- happy, every-fault, and restart traces replayed twice with byte-identical
  snapshots, audit bytes, and final digest;
- digest corruption detection without any authentication claim.

Tests compare public values and ordered audit records. They do not inspect
Arduino pins or depend on wall time. The trace schema contains only synthetic
observations, debounced operator events, expected values, audit sequence, and
version; it contains no protocol or external electrical field.

## Narrative Mega composition

The canonical sketch declares objects in dependency order: runtime and claims,
buttons, cue LEDs and health presentation, Lesson 028 source and assessor,
Lesson 029 plan/scheduler/audit storage, then `InertShowSimulator`.

`setup()` reads as acquire, configure, start. `loop()` reads:

```cpp
void loop ()
{
    const adk::TimePoint now (millis ());

    readSimulatorInput (now);
    updateSimulator    (now);
    showInertCues      ();
    showSimulatorState (now);
}
```

`showInertCues()` addresses only eight statically constructed `MonoLed`
objects. It consumes a snapshot and cannot call back into the simulator. An
optional sound cue is bounded and supplementary; it cannot be the sole
evidence. Serial may print the stable audit grammar or be snooped through a
Make target, but the circuit works and can be verified without it.

## Physical observability

Use a startup sweep to prove endpoint acquisition separately from safe-state
evidence. Then use:

- TP28 at the selected synthetic input switch;
- TP29 at the selected resistor-limited cue LED output;
- the RGB health indicator for review, confirmation, held, cancelled, and
  fault states;
- the LCD or a bounded LED selection pattern to identify the pending cue.

The bench worksheet follows:

```text
TP28 button level
    -> debounced event and synthetic observation
        -> assessed channel value
            -> reviewed schedule decision
                -> TP29 cue LED voltage
                    -> audit entry
```

Every row records predict, observe, and interpret, plus what the evidence
cannot prove. A lit LED proves only a simulator presentation. Measure the
inactive level while initialized, then measure high impedance after shutdown;
do not treat one as evidence for the other. Leave the physical acceptance card
open until the Mega 2560, resistors, supply, instruments, observations, date,
and reviewer are recorded.

## Lesson and package acceptance

The capstone package is complete only with:

- clean declarative headers, out-of-line implementation, aggregate-header
  registration, and standalone header compilation;
- strict and sanitizer host tests plus public replay fixtures;
- `make inert-show-test`, `make inert-show-replay TRACE=...`, and
  `make inert-show-audit TRACE=...` targets;
- a narrative Mega 2560 example, measured flash/static-RAM result, and an
  explicit budget with margin;
- HTML containing the exact API, state/fault/precedence tables, trace schema,
  source/test/example links, CLI commands, corrections, limitations, and the
  prominent E0/E1 safety boundary;
- a complementary black-and-white PDF containing the pencil orientation
  drawing, authoritative schematic and wiring list, state/timing diagrams,
  trace prediction exercises, audit reconciliation worksheets,
  troubleshooting tree, claim-evidence-reasoning exercise, and blank hardware
  acceptance record;
- grayscale, metadata, font, extraction, link, overflow, site, package-smoke,
  Arduino lint, and Mega compilation gates;
- an independent safety review confirming there is no generic actuation,
  ignition, launcher, unknown-radio, or external-output path.

Promote Lesson 030 only after Lessons 028 and 029 are committed as supported
dependencies and all non-hardware gates pass. Its truthful release state is
“host verified; bench open” until measured E1 evidence exists.
