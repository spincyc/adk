# Percussion sequencer design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the final hardware-independent Lesson 039 project engine. An initial review
blocked promotion because the engine copied and revalidated complete Lesson
037 and 038 snapshots. The bounded remediation below was completed and
re-reviewed before promotion.

## Boundary

- Name and lesson/project: `PercussionSequencer`, Lesson 039
- Reviewer and date: independent architecture review and remediation
  re-review, 2026-07-28
- Public types and operations: `PercussionMode`, `PercussionFaultSource`,
  `PercussionAssociation`, `PercussionHit`, `PercussionSequencerConfig`,
  `PercussionAcousticCompletion`, `PercussionSequencerInput`,
  `PercussionFrame`, `PercussionSequencerSnapshot`, and
  `PercussionSequencer::{initialize,shutdown,update,clear,initialized,snapshot,hit}`
- Direct dependencies: project-owned copied evidence, `Status`, `Result<T>`,
  `TimePoint`, `Duration`, and fixed-width integer types
- Existing decisions and interfaces reconsidered: copied-observation
  composition, status provenance, same-time identity, fixed-capacity
  atomicity, bounded record access, shutdown evidence retention, and
  deterministic replay

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural after bounded remediation.** The engine owns one project-level responsibility: group qualified attacks, associate relative acoustic intensity, record a bounded pattern, and publish playback intents. `PercussionSequencerInput` now contains only a four-bit attack mask, four source statuses, one acoustic status, an optional completion tuple, tempo, controls, and the shared timestamp. It no longer includes or reinterprets `ContactObservation`, `AcousticObservation`, their raw readings, counters, phases, or quality cross-products. The lesson adapter projects already-qualified source evidence; electrical ownership stays in endpoints and source components. |
| Ownership and lifecycle | **Natural.** Construction is inert; the engine is non-copyable and non-movable; it owns no pin, endpoint, timer, bus, callback, or external storage. `initialize()` is idempotent. Shutdown clears pending work, playback epoch, and output intents while retaining finalized hits as inspectable volatile evidence. Reinitialize resumes in `Full` only when the retained pattern has 32 hits; `clear()` is the explicit destructive operation. |
| Time and ordering | **Natural.** Every update supplies one shared `TimePoint`. Same-time identity compares only the semantic DTO, so unrelated source snapshot evolution cannot fault playback. Changed semantic evidence at the same time, apparent backward or half-range time, rollover, simultaneous-window edges, association timeout precedence, sparse playback updates, and tempo changes at step boundaries are explicit and tested. |
| Errors and status | **Natural.** Existing `Status` values travel through `surfaceStatus[4]` and `acousticStatus`; `PercussionFaultSource` preserves which input lane won deterministic fault precedence. There is no extra `inputValid` convention and no duplicated source phase/quality validator. `hit(index)` uses the existing `Result<PercussionHit>` convention for a value-bearing bounded lookup and returns `InvalidArgument` out of range. |
| Resource budget | **Natural and improved.** Storage and update work are fixed and bounded. The original full-snapshot design measured 172-byte input, 528-byte engine, 48-byte snapshot, and 8-byte hit on the host; its AVR probe measured 149-byte input, 488-byte engine, 39-byte snapshot, and 8-byte hit. The final narrow design measures 32-byte input, 400-byte engine, 56-byte snapshot, 16-byte completion, and 12-byte public hit on the host; AVR measures 25-byte input, 371-byte engine, 42-byte snapshot, 11-byte completion, and 9-byte public hit. The larger public snapshot/hit carries explicit fault and association provenance, while compact private stored hits plus a 32-bit association bitmap keep total engine storage below the original design. |
| Deterministic proof | **Natural.** Focused fixtures cover configuration, lifecycle, grouping and ordering, association edges and timeout, quantization, tempo, playback frames, sparse updates, rollover, fault precedence and provenance, clear, retained restart evidence, and fixed capacity. A group that would exceed remaining capacity is rejected atomically; no prefix is stored and ordinals do not advance. The versioned `PERCUSSION_TRACE_V1` fixture is replayed field by field and compared with `PERCUSSION_GOLDEN_V1`, including snapshots, bounded hit access, and association provenance. |
| Packaging and public surface | **Natural.** The standalone declarative header has an out-of-line implementation, the supported umbrella includes it, and the native host inventory has ordinary, exception-enabled, and sanitizer coverage without a special source path. The raw internal pointer was removed; callers use bounded `Result<PercussionHit> hit(uint8_t)`. Arduino discovery remains the normal public-`src` mechanism. |
| Persistence and recovery | **Natural.** All pattern state is intentionally volatile. Finalized hits survive only object shutdown/reinitialize, not reset or power loss; no EEPROM, RTC, removable medium, record schema, wear, torn write, or recovery claim is introduced. Clear is synchronous and complete. |
| Observation identity and provenance | **Natural.** One top-level timestamp identifies the projected contact and acoustic evidence. Per-surface and acoustic statuses preserve source provenance. `PercussionAssociation` distinguishes an acoustic completion from timeout-derived zero intensity in the snapshot and every returned hit. Same-time comparison and the versioned replay trace use the same fieldwise identity; C++ padding is not serialized or compared. |
| Actuation and safe-state interference | **Natural.** `PercussionFrame` is an intent value, not hardware ownership. Fault and shutdown invalidate and zero tone/light intents immediately. The adapter remains responsible for applying the frame to qualified LEDs, display, and `PiezoSounder`, preserving their resource, safe-state, and diagnostic-interference contracts. |
| Example and documentation fit | **Natural at E0.** The narrow input keeps the planned example readable as observe source components, project qualified evidence, decide the pattern, and actuate the frame. Code, trace, HTML, and PDF can use attack, association, hit, frame, and fault provenance consistently. The linked Mega example and authoritative schematic remain ordinary exact-specimen gates; architectural fit does not admit an unidentified contact, microphone board, or passive piezo. |
| Downstream effects | **Contained.** The remediation changed only the still-unpromoted Lesson 039 engine, fixtures, drafts, adapter contract, host inventory, and umbrella integration. Lessons 037 and 038 retain their public observations unchanged. Future additions may evolve those snapshots without forcing changes here unless the adapter's deliberately narrow project evidence changes semantically. No earlier supported component, example, lesson, physical record, or package consumer requires migration. |

## Prior-decision impact

- Hardware-neutral behavior with endpoint ownership in the adapter:
  **preserved**.
- Narrow copied semantic evidence rather than complete upstream snapshots:
  **extended** by a project-owned DTO; the previously challenged coupling is
  removed.
- Existing `Status` and `Result<T>` conventions: **preserved**. Source
  provenance is explicit and indexed record access is bounded.
- Explicit unsigned time, deterministic precedence, rollover, and no hidden
  clock: **preserved**.
- Fixed memory, no heap, and bounded update work: **preserved**, with lower
  host and AVR engine/input sizes than the rejected design.
- Fixed-capacity policy: **extended** by the recorded decision that one
  simultaneous group is atomic at exhaustion.
- Shutdown safe state and inspectable evidence: **extended** by the explicit
  decision to retain finalized volatile hits while clearing all active and
  output state.
- Deterministic replay: **extended** by versioned, fieldwise trace and golden
  files that include fault and association provenance.
- Generic cross-component event evidence: **preserved as deferred**. A shared
  abstraction was not introduced without a second demonstrated consumer.
- Canonical publication authority: **preserved and out of scope**.

## Stress disposition

**Bounded local remediation completed; natural fit verified.** The rejected
full-snapshot input would have made upstream schema and legal-tuple evolution a
Lesson 039 compatibility concern. The final project-owned DTO removes that
coupling, reduces memory, preserves exact fault provenance, and limits replay
identity to consumed semantic evidence.

The alternatives were resolved as follows:

1. Retaining complete source snapshots was rejected because it preserved
   irrelevant-field identity faults, duplicate tuple policy, and wide blast
   radius.
2. A generic shared event-evidence type remains deferred until another real
   consumer proves common semantics.
3. The narrow project-owned projection was selected and verified.

The same decision also selected atomic group rejection at capacity,
`Result<PercussionHit>` bounded access instead of a raw pointer, volatile hit
retention across shutdown/reinitialize, explicit fault and association
provenance, and a versioned fieldwise golden replay contract.

## Gate result

- Disposition: bounded local remediation completed; natural fit verified
- Open risks: exact contact, microphone, passive-piezo, pin, current, sampling,
  acoustic-exposure, flash, static-RAM, stack, and physical acceptance evidence
  remain ordinary Mega example/specimen gates
- Required discussion or decision IDs: resolved by the Lesson 039 narrow
  evidence, atomic-capacity, bounded-access, retention, provenance, and replay
  decisions recorded with the active integration task
- Remediation owner and next action: remediation is complete; the Lesson 039
  integration owner proceeds through the ordinary example, size,
  documentation, packaging, and publication gates without reopening Lessons
  037 or 038
- Verification commands and results:
  - strict focused host build and `test_percussion_sequencer`: passed
  - exception-enabled and sanitizer focused tests: passed
  - versioned fieldwise trace/golden replay: passed
  - host layout probe: old input/engine/snapshot/hit
    `172/528/48/8`; final input/engine/snapshot/completion/hit
    `32/400/56/16/12` bytes
  - AVR layout probe with avr-gcc 7.3.0: old
    input/engine/snapshot/hit `149/488/39/8`; final
    input/engine/snapshot/completion/hit `25/371/42/11/9` bytes
  - independent re-review: no remaining architectural blocker
  - template structure and `git diff --check`: passed
- Promotion permitted: yes by the architecture stress gate; exact-specimen
  Mega example, size, lesson, packaging, publication, and physical gates remain
  independently controlling
