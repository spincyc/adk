# Status and result facades for state machines

This note audits the proposed `Status` value-object migration across the
state-machine APIs. It is an API design record, not a supported-interface
change.

## Rule

`Status` describes whether an operation could honor its contract. Callers use
`ok()` for control flow and inspect `error()` only when a specific
`StatusCode` matters. Pass the complete object to logging, propagation, and
policy code. A domain state describes what happened in the modeled activity.
They are independent.

- Invalid configuration, missing initialization, resource contention, invalid
  time progression, and hardware access failure are `Status` concerns.
- A timeout, wrong Simon cue, premature reaction, invalid keypad chord, stale
  climate sample, and completed traffic phase are domain concerns.
- `Result<T>` is appropriate only when an operation produces a value that is
  unusable on operational failure.
- A snapshot remains directly returnable when it is meaningful during failure.

Do not convert a normal negative domain outcome into a generic error. Do not
wrap snapshots in `Result<T>` merely to make them expose `ok()`.

## Global `transient()` limitation

The current `Status::transient()` classifies `ResourceBusy` and
`HardwareFailure` globally. Recovery is actually owned by the component:

- a busy resource may remain occupied until another owner is destroyed;
- a traffic hardware failure is deliberately latched until reset;
- an invalid keypad sample may recover after later valid, debounced samples;
- a climate transport failure may be followed by a valid frame.

Therefore callers must not use `Status::transient()` as retry policy. Prefer a
component-level recovery facade or an explicit policy decision by the
coordinator. If the generic method remains, document it as “potentially
recoverable cause,” not “retry this operation.”

## Per-component decisions

| Component | Keep domain-specific | Useful facade | Avoid |
|---|---|---|---|
| `ReactionTimer` | `ReactionState`, `ReactionOutcome`, cue and reaction measurements | `running()`, `complete()`, and `succeeded()` derived from the snapshot | Treating premature press or response timeout as `Status` errors |
| `Simon` | `SimonPhase`, `SimonOutcome`, sequence and observed-cue evidence | `running()`, `roundComplete()`, `gameComplete()`, and `gameSucceeded()` | Treating mismatch, chord rejection, or player timeout as operational errors |
| `TrafficJunction` | `TrafficPhase`, pending request, transition evidence, and signal image | `healthy()` plus `faultStatus()`; recovery is explicitly `reset()` | Marking a latched traffic fault transient or hiding its all-red snapshot in `Result<T>` |
| `Keypad` | `KeypadState`, key identity, raw mask, and edge events | `sampleUsable()`, `hasKey()`, and `inputFault()` | Treating an invalid chord as hardware failure or returning `Result<KeypadSnapshot>` |
| Climate sampling | `ClimateSampleState`, values, observation time, and staleness | `valid()`, `hasMeasurement()`, and `mayRecoverWithNextSample()` | Collapsing timeout, checksum, range, stale, and timing states into one `Status` |

These facades should be small `const noexcept` predicates implemented from the
canonical snapshot fields. They add vocabulary without adding state.

## Component findings

### Reaction timer

`ReactionOutcome::PrematurePress` and `ReactionOutcome::Timeout` are successful
executions of the experiment that produced negative results. `update()` should
still report a status for which `ok()` is true. Configuration and timestamp failures remain
operational statuses.

The snapshot remains valid before, during, and after a trial, so
`Result<ReactionTimerSnapshot>` would make observation harder without adding
information. A teaching-facing `complete()` predicate can group `Success` and
`Failure`; `succeeded()` can test only `ReactionOutcome::Success`.

### Simon

Mismatch, invalid player input, and player timeout are reproducible game
outcomes. They are not transport errors. `SourceFailure` is different: the
domain outcome explains why the game stopped while `status` preserves the
source operation's concrete failure.

Keep both fields. Add derived predicates only. In particular, do not make
`SimonOutcome::InvalidInput` alias `StatusCode::InvalidArgument`; the former is an
observed chord in a valid game, while the latter means the API call or time
trace violated its contract.

### Traffic junction

The fault snapshot is essential evidence because it proves both roads are red
and pedestrian walk is inactive. It must remain available without unwrapping a
failed result.

`healthy()` should mean “initialized and not fault-latched.” `faultStatus()`
should preserve `HardwareFailure` versus invalid timestamp progression.
Neither implies that physical lamps were measured; the signal image is a
commanded logical state. A latched fault becomes recoverable only through the
documented reset path, so generic transient classification must not trigger an
automatic update retry.

### Keypad

An invalid chord is valid evidence about user input and belongs in
`KeypadState::InvalidChord`. It should not become an operational error. A
sample with failed electrical validity belongs in `KeypadState::Fault` and may
retain `StatusCode::HardwareFailure` through its complete status, while raw mask and edge suppression remain
observable.

Because a later stable valid sample can recover the keypad, the component may
offer `inputFault()` and document recovery through subsequent `update()` calls.
That recovery rule is more precise than the global transient bit.

### Climate

Climate is the clearest case against a generic result wrapper. A stale or
out-of-range sample still carries the measured values, timestamp, and exact
quality reason needed for diagnosis. `ClimateSampleState` should remain the
authoritative sample-quality model.

`ClimateSensor::update()` continues to return operational status for the
attempt to advance the transport. `sample()` continues to return the latest
evidence. Derived predicates can group states for presentation:

- `valid()` only for `Valid`;
- `hasMeasurement()` for states carrying decoded values, including range and
  stale states;
- `mayRecoverWithNextSample()` for unavailable, timeout, checksum, range, and
  stale states, but not invalid limits or ambiguous timing.

The exact grouping must be tested as part of the public contract.

## Migration order

1. Document `Status::transient()` as cause classification rather than retry
   instruction.
2. Add snapshot/sample predicates one component at a time, without changing
   stored layout.
3. Add truth-table tests for every enum value before using a facade in an
   example.
4. Change examples to speak in domain predicates while retaining detailed
   enum evidence for diagnosis.
5. Consider removing global `transient()` only at a versioned API boundary
   after every caller has migrated to component recovery policy.

No state machine currently benefits from replacing its snapshot return with
`Result<Snapshot>`.
