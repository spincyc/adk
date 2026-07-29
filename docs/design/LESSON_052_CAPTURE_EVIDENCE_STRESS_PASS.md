# Lesson 052 capture-evidence architecture stress pass

This architecture stress pass records both the pre-implementation decision and
the completed independent post-implementation review for Lesson 052 in the
infrared protocol-workbench arc. It evaluates the implemented additive,
receive-only copied-evidence boundary over the promoted Lesson 025 capture and
decoder contracts. E0 promotion does not authorize an emitter, transmission,
unknown-protocol replay, receiver wiring, an exact infrared specimen, or
physical acceptance.

## Boundary

- Name and lesson/project: copied infrared capture evidence, Lesson 052
- Reviewer and date: pre-implementation architecture review, 2026-07-28;
  independent clean post-implementation review, 2026-07-29
- Proposed public responsibility: synchronously decode and copy one published
  Lesson 025 pulse frame into bounded caller-owned storage, retain its capture
  and source provenance, attach a categorical `EvidenceStrength`, and expose a
  stable read-only view without assigning unknown evidence command meaning
- Direct dependencies: `Status`/`Result<T>`, `MicrosecondTimePoint`,
  lesson-specific `IrPulseStorage`, the promoted `PulseFrame` and
  `InfraredFrame` value contracts, and a borrowed receive-only
  `InfraredDecoder`
- Existing decisions and interfaces reconsidered: Lesson 025 owns electrical
  capture and acknowledgement; completed frames remain stable only until their
  matching acknowledgement; unknown evidence is observable but never
  replayable; time and source identity are explicit; fixed caller storage is
  preferable to hidden heap or another interrupt owner

Lesson 052 must be additive. `PulseCapture` continues to own the pin,
interrupt, edge queues, publication lifetime, and acknowledgement. The new
component must not inherit from it, borrow its internal arrays after
acknowledgement, duplicate its interrupt path, or widen its published API.
Before acknowledging a published Lesson 025 frame, the caller asks Lesson 052
to admit it. Lesson 052 calls the promoted `InfraredDecoder` synchronously on
that same borrowed `PulseFrame`, requires
`InfraredFrame::captureSequence == PulseFrame::sequence`, derives its
categorical fields from that result, and copies the admitted pulse words. It
does not accept a caller-authored `InfraredFrame`, decoder disposition, decoded
address, or decoded command. A decode or copy failure leaves the prior Lesson
052 record unchanged and leaves the Lesson 025 frame available for an explicit
caller retry or discard decision.

The proposed compact storage unit is one `uint32_t` per pulse. Bit 31 records
`Mark` versus `Space`; bits 0--30 record the nonzero duration in
microseconds. The maximum is the existing `PulseCapture::capacity` of 100
words, or 400 bytes. This encoding is an ordinary public value representation,
not a packed C++ struct and not a wire or persistence format. Durations of
zero or greater than `0x7fffffff` reject before mutation. The implementation
must use shifts and masks, never struct representation, type punning, or
implementation-defined bitfields.

The caller supplies exactly one `IrPulseStorage` mutable word span with capacity
`1..100`; configured maximum pulse count cannot exceed that extent. The
component exclusively writes that span while initialized, never retains a
borrow outside its lifetime, and exposes a read-only bounded view valid until
the next successful non-idempotent admission, reset, shutdown, or destruction.
The component retains the storage pointer and bounded state; snapshots contain
only provenance, category, status, and counters; and views contain only a
read-only pointer, extent, owner identity, and generation. No 400-byte array is
embedded in a returned value or copied on the stack.

## Proposed evidence contract

The clean-reviewed implementation-depth plan freezes the public names, values,
and rules summarized here:

- source kind, nonzero source ID, nonzero source-configuration revision, and
  nonzero capture-session epoch;
- source `Status`, retained in full provenance;
- Lesson 025 capture sequence and supplied observation time;
- pulse count and exact compact pulse words;
- capture disposition: complete, overflow, or timing fault;
- decoder disposition: known valid, known repeat, unknown protocol, timing
  invalid, integrity invalid, truncated, or overflow;
- decoded protocol/address/command only for known valid evidence; repeat
  carries no newly inferred address or command; and every unknown or malformed
  category has canonical zero decoded fields;
- a nonzero evidence generation identifying the current copied record; and
- exact `requiredWords()` and `exportWords()` result counts.

The constructor borrows `InfraredDecoder&`, accepts `IrPulseStorage` and a
configured maximum pulse count, and owns neither dependency. `admit()` accepts
only the published `PulseFrame`, `IrSourceIdentity`, source `Status`, and
`observedAt`. A non-OK source status publishes `SourceFault` with canonical
decoded fields while retaining the source status and attribution. The exact
overflow distinction is also frozen: complete capture plus decoder overflow
is `DecoderOverflow`; capture overflow plus canonical decoder overflow is
`CaptureOverflow`; every other overflow pairing rejects atomically.

`EvidenceStrength` has exactly three categorical values:
`None`, `ShapeRecognized`, and `IntegrityVerified`. It is not a score,
probability, confidence estimate, or authority. A known valid frame maps to
`IntegrityVerified`. An NEC result with repeat, timing-invalid,
integrity-invalid, truncated, or overflow validity maps to
`ShapeRecognized`. An unknown-protocol result, an unknown-protocol truncated
result, capture overflow, and capture timing fault map to `None`.
`ShapeRecognized` means only that the promoted decoder recognized an NEC
leader or repeat shape. Repeat never inherits the address, command, strength,
or authorization of an earlier frame.

`Known`, `Repeat`, `Unknown`, and `Malformed` are presentation groupings, not a
lossy replacement for the decoder disposition. In particular, timing,
integrity, truncation, capture overflow, and capture timing fault remain
separately testable. A capture state and decoder disposition that cannot
coexist reject atomically instead of being normalized into a plausible
category. Unknown pulse words may be inspected or exported as receive
evidence, but no Lesson 052 operation produces carrier timing, transmit
commands, or a replay-ready emitter request.

Same-sequence admission is fieldwise, excluding only observation time. An
exact duplicate from the same source, configuration, session, pulse words,
source status, capture state, decoder disposition, evidence strength, protocol,
and decoded fields is idempotent, retains the original observation time, and
does not advance the evidence generation. Any change to those identity fields
at the same source sequence is a source fault and leaves the prior record
intact.
Forward sequence order uses the existing unsigned half-range convention; zero,
ambiguous half-range, and regressing sequences reject. The decoder result's
capture sequence must exactly equal the borrowed capture sequence on every
admission. Source, configuration, or session changes require `reset()` and
cannot silently continue sequence history.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Passed as copied component-layer evidence.** Lesson 025 retains endpoint/resource capture and protocol decoding. Lesson 052 adds stable evidence, categorical meaning, and bounded export without accessing a pin, ISR, timer, or decoder register. The implementation introduces no second capture owner, `PulseCapture` change, or protocol-emission path. |
| Ownership and lifecycle | **Passed for E0.** Construction borrows one caller-owned `IrPulseStorage` that outlives the initialized component. The component is non-copyable and non-movable, writes the storage exclusively, and is inert until `initialize()`. Tests prove that failed initialization, failed admission, stale input, and undersized output leave the prior record unchanged. A view is a non-owning pointer/count plus owner identity and evidence generation; tests prove its exact invalidation rules and foreign/stale rejection. |
| Time and ordering | **Passed with supplied observation time.** Lesson 052 has no clock, ISR, wait, debounce, or retry loop. One call admits at most one already-published frame. The supplied time records when software observed/copied the published frame; it is not capture occurrence time and cannot be causally validated because `PulseFrame` publishes no occurrence timestamp. Tests cover exact duplicate, changed same-sequence, wrap, half-range ambiguity, delayed observation, reset, and source-session change. |
| Errors and status | **Passed with structural failure separate from evidence categories.** Tests prove that invalid configuration, null/short storage, malformed enum, impossible state combinations, bad duration, stale source, and short export use ordinary non-mutating failure status. Complete/overflow/timing-fault capture and known/repeat/unknown/malformed decode outcomes remain copied evidence, not operation failures. The implementation adds no diagnostic string, exception, silent truncation, or category fallback. |
| Resource budget | **Measured and passed for E0.** The canonical standalone Mega sketch measures 5,530 B flash and 1,096 B static SRAM. `sizeof(CapturedIrEvidence)` is 48 B, below its 96/128 B target/hard ceiling, and its exact caller-owned `uint32_t[100]` storage is 400 B. The conservative live stack path is 197 B, below the 640/768 B target/hard gate. All standalone results pass the 16/20 KiB flash and 3,072/3,584 B static-SRAM target/hard gates. E0 owns zero pins, timers, interrupts, buses, ADC channels, endpoints, registry claims, heap, or external power. The separate complete Lessons 052--054 maximum-composition measurement accounts for the promoted Lesson 025 footprint and passes its reviewed hard gates. |
| Deterministic proof | **Passed for E0.** Executable host tests supply every pulse word, source field, state, category, sequence, time, storage extent, and output extent; the exact vectors and capacity/failure matrix below pass under strict and sanitizer gates. No host result establishes receiver compatibility, carrier frequency, optical range, or physical timing accuracy. |
| Packaging and public surface | **Passed for E0.** The standalone declarative header, out-of-line implementation, umbrella export, native/archive inventories, partitioned test binary, canonical Mega replay, size evidence, HTML reference, pencil-drawing PDF, indexes, downloads, and release package passed their gates. No lesson-only decoder fork, Arduino-only representation, hidden generated fixture, or generic serialization framework was introduced. |
| Example and documentation fit | **Passed for E0.** The canonical sketch uses a fixed synthetic `PulseFrame` fixture, configures caller storage and source identity, admits the fixture, and actuates only visible semantic receive/category result cells. It initializes no `PulseCapture`, receiver, interrupt, or optical hardware. The HTML documents the stable-copy API, and the PDF teaches provenance and categorical evidence. Every non-schematic visual is classified and presented as a pencil drawing; E0 contains no formal schematic. Rendered-page and link review passed. |
| Downstream effects | **Contained and independently reviewed.** Lesson 053 consumes only its separate learner-created known-code table, never a Lesson 052 capture. Lesson 054 may display source/category/strength and round-trip evidence, but it cannot reinterpret a Lesson 052 view or export as transmit authorization. Compile-fail and runtime tests enforce the separation; Lesson 025 remains source-compatible and behaviorally unchanged. |

## Required deterministic matrix

The passing E0 test matrix covers all of the following:

1. configuration and lifecycle: null storage, capacities 0, 1, 99, 100, and
   101; configured maximum below, at, and above the span; initialize twice;
   shutdown before/after initialize; reset; destruction; and a composition
   fixture proving that two live components receive disjoint storage under
   the documented caller-ownership precondition;
2. pulse representation: zero, one, 99, and 100 pulses; alternating and
   repeated levels; duration 0, 1, configured exact minimum and maximum,
   `0x7fffffff`, and `0x80000000`; every compact-word bit; and encode/decode
   round trip without reading padding;
3. capture/category cross-product: complete with each decoder disposition,
   overflow with canonical decoder overflow, timing fault with canonical timing
   invalid, and every prohibited mismatch;
4. categorical evidence: every decoder disposition maps to the exact
   `EvidenceStrength` rule above; known valid retains exact
   protocol/address/command; repeat is distinct and zeros new decoded fields;
   unknown protocol, integrity invalid, timing invalid, truncated, and overflow
   retain exact pulse words while decoded fields remain canonical zero; repeat
   cannot inherit earlier command identity or strength;
5. provenance: source kind/ID, configuration revision, session epoch, capture
   sequence, observation time, and evidence generation; zero fields; decoder
   capture-sequence mismatch; exact duplicate with a changed observation time
   retaining the original time; changed same-sequence in every identity field
   and pulse word; forward order; wrap; regression; half-range ambiguity;
   source/configuration/session change; and reset before a new session;
6. atomicity and view lifetime: failure on the first, middle, and last pulse;
   prior bytes and snapshot unchanged on rejection; no view before admission;
   view valid across observations; invalid after successful replacement,
   reset, shutdown, and destruction; stale/foreign generation rejected; and
   no pointer retained to the Lesson 025 frame;
7. view and copied-word export: destination word spans of sizes 0,
   required-minus-one, exact, and larger; deterministic exact required/written
   word counts; exact numeric compact-word vectors; unchanged destination on
   preflight failure; foreign/stale view rejection; and identical word output
   from the same supplied fields. This API is not byte serialization and
   promises no byte order or persistence format; and
8. composition collision: a full 100-pulse unknown frame, capture overrun
   pending behind it, a changed duplicate, a diagnostic output failure, and a
   simultaneous Lesson 054 keypad request. The accepted capture remains
   attributable, the pending frame is not acknowledged accidentally, and no
   transmit eligibility is produced.

Strict warnings, sanitizer execution, format/style/diff gates, canonical Mega
compilation, archive/package checks, and measured AVR object/flash/SRAM/stack
evidence accompanied this matrix and passed. Tests compare fields and compact
words, never whole structs with padding.

## Composition pressure scenario

The maximum currently authorized E0 composition is one promoted Lesson 025
`PulseCapture`, one `InfraredDecoder`, one Lesson 052 evidence component with
one 100-word caller buffer, two independent receive/category LED intents, one
bounded fixture-export destination, and the planned Lesson 054 pure-policy
translator with keypad and display evidence. No emitter endpoint or carrier
timer exists in this E0 composition.

The stress trace begins with a maximum-length unknown frame at sequence
`0xffffffff`, copies it while diagnostics fail, and leaves a second capture
pending. It then presents an exact duplicate, a one-word-changed duplicate,
wraps to sequence 1, changes the source configuration without reset, resets
into a new session, admits a known valid frame, and collides its display update
with a keypad-selected local command. The unknown record remains inspectable
but never becomes a transmission candidate. The known command may be selected
only from the independent local table fixed by Lessons 053--054.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Passed for E0.** Copy and classification remain bounded O(n) for configured capacity at most 100, with no catch-up loop. Deterministic replay covers the 100-word path, simultaneous time, delayed calls, sequence wrap, pending evidence, and maximum diagnostic work. The compiler-derived conservative standalone stack path is 197 B. Physical acknowledgement latency and endpoint scheduling remain E1 evidence, not E0 claims. |
| Total memory and hardware resources | **Measured and passed for E0.** The canonical standalone result is 5,530 B flash, 1,096 B static SRAM, a 48 B `CapturedIrEvidence` object, one exact 400 B caller buffer, and a 197 B conservative stack path. Those values pass every standalone target and hard gate. E0 adds zero hardware resources. The separately measured full Lessons 052--054 composition includes the Lesson 025 capture/ISR state and passes its hard gates. A second 400-byte retained frame remains unauthorized. |
| Shared bus or transport | **Not applicable at E0.** Copied frames and caller storage perform no bus or transport operation. A qualified receiver adapter remains owned by Lesson 025. Future LCD or storage transport belongs to an explicit owner in Lesson 054; its failure cannot change capture category. |
| Persistence and recovery | **Not applicable by explicit volatility.** The caller buffer and evidence generation are cleared on reset/shutdown and are not reconstructed after power loss. Fixture export is a caller-requested copied artifact, not a durable commit or physical-media claim. Any persisted capture format requires a separate schema, configuration identity, corruption, torn-write, capacity, privacy, and recovery decision. |
| Motion, external power, or stored energy | **Not applicable to Lesson 052.** The boundary has no actuation path, emitter, carrier timer, switched supply, or stored-energy output. Lesson 053 makes optical emission applicable only after exact emitter, current limit, carrier ownership, cancellation, duty/burst bounds, exposure-bounded operation, and independent physical qualification are separately designed and qualified. Current limiting and bounded duty reduce exposure; they do not establish eye safety. |
| Observation identity and provenance | **Passed for E0 and central.** Source kind/ID, configuration revision, session epoch, capture sequence, first observation time, capture state, decoder disposition, evidence strength, pulse count/words, and decoded fields form one tested record. Same-sequence identity is fieldwise except for a later observation time, which cannot restamp the accepted record. Tests prove that unknown and malformed evidence retains its origin without acquiring command meaning and that values from different sessions or configurations cannot be combined or restamped. |
| Diagnostic interference | **Passed for E0.** Receive, known, repeat, unknown, and malformed result cells plus export work have explicit bounded behavior. Tests prove that their failure, disablement, or full destination cannot alter the retained record, acknowledge a pending Lesson 025 frame, change category, or authorize transmission. Serial remains supporting evidence only. |
| Failure collision and recovery | **Passed for E0.** Structural/provenance failure rejects before mutation; capture overflow and timing fault remain categorical evidence. Changed same-sequence faults dominate idempotence. Tests prove that copy/export/diagnostic failures preserve the prior accepted record and exact source attribution, while reset invalidates views and starts a new session without converting unknown evidence into local authority. |

## E0 and E1 separation

E0 is a deterministic copied-evidence publication. It owns no receiver pin,
interrupt, timer, bus, emitter, LED endpoint, display endpoint, or physical
storage. Its canonical sketch may replay synthetic Lesson 025 frames and
publish semantic LED/display result cells. A compiled Mega sketch and host
vectors prove only bounded software behavior.

E1 begins only after the exact receiver specimen, owned harmless remote,
supply range, pin order, idle polarity, output level, carrier band, Mega input
compatibility, interrupt/pin allocation, current budget, and authoritative
schematic are established from primary sources. Bench evidence must separately
record resource acquisition/rollback, TP-IR pulse observation, visible
category indication, capture overflow/timing-fault behavior, shutdown pin
state, reset, receiver saturation, missing carrier, and power removal. The
receiver and diagnostic endpoints require their own current and safe-state
measurements. E1 may validate receive evidence; it still does not authorize
emission or unknown replay.

Lesson 053 emission is a separately gated output boundary, not an automatic
E1 extension of Lesson 052. Exact emitter topology, wavelength, resistor or
driver sizing, peak and average current, timer/carrier ownership, burst and
duty limits, cancellation latency, exposure-bounded handling, optical
observation, and shutdown require their own stress pass, primary evidence,
schematic, and bench record. Neither Lesson 052 nor bounded current/duty makes
an eye-safety claim.

## Prior-decision impact

- Four-layer dependency direction and Lesson 025 ownership: **preserved** by
  consuming only published copied values and acknowledging only after a
  successful copy.
- Lesson 025 frame lifetime and acknowledgement: **preserved**; Lesson 052
  never retains the borrowed `PulseFrame::data` pointer.
- Existing `InfraredDecoder` categories: **preserved and projected into
  presentation** through the fixed categorical `EvidenceStrength` mapping, not
  collapsed into a new decoder, score, confidence heuristic, or caller-supplied
  interpretation.
- Receive-only unknown evidence and prohibition on unknown replay:
  **preserved**. No exported evidence is transmit authority.
- Explicit time, rollover-safe sequence order, bounded work, and stable
  snapshots: **preserved** with new source/session provenance.
- Fixed storage, no heap/exceptions/RTTI, and no diagnostic strings:
  **preserved** through one exact caller-owned word span.
- Existing `Status` and `Result<T>` meaning: **preserved**; categorical
  evidence is not encoded as operational failure.
- Circuit-native non-Serial observation: **extended** to distinct receive and
  category intent, while physical endpoints remain E1-gated.
- Exact specimen qualification before wiring, formal schematics, or physical
  claims: **preserved**.
- Pencil drawing for all non-schematic PDF visuals: **preserved**.
- Lesson 053 learner-created known-code transmission: **preserved by
  separation**; it cannot import unknown Lesson 052 records into its table.

## Design-buckling review

The design does not buckle if Lesson 052 remains a small copied-evidence
component above Lesson 025. Existing capture ownership, decoder semantics,
status conventions, explicit time, and fixed storage express the requirement
without a new layer or shared framework. The 400-byte maximum record is
material on a Mega 2560, but caller ownership keeps it visible in composition
budgets and avoids embedding it in snapshots, previews, or stack values.

Three future changes would buckle the promoted design and require a new
architecture discussion before modification:

1. changing `PulseCapture` storage, ISR behavior, frame lifetime, or
   acknowledgement to make evidence copying convenient;
2. creating a generic capture persistence/replay framework or treating fixture
   export as a durable storage transaction; or
3. allowing any unknown, malformed, or captured record to become emitter
   input, local command authority, or a protocol-transparency claim.

If the measured aggregate cannot fit one 400-byte record plus the existing
Lesson 025 capture queues and required stack/ISR reserve, implementation stops
for a size-focused repair or durable budget decision and this pass is rerun.
Lowering the canonical maximum below 100 would change the frozen plan and is
not an implicit local remedy. Compressing durations below exact microseconds,
sharing interrupt buffers after acknowledgement, allocating dynamically, or
silently dropping pulses would materially change evidence and is not an
acceptable fix.

## Stress disposition

**E0 implementation and promotion passed.** The final implementation conforms
to the clean-reviewed Lessons 052--054 plan: it preserves the exact public API,
storage/view lifetime, compact-word codec, source/session identity,
state/category compatibility table, same-sequence rule, and no-capture-to-
transmission boundary. Deterministic and sanitizer tests, the canonical
compile-only Mega example, measured resources, HTML, pencil-drawing PDF,
indexes, archive/release packaging, and the independent post-implementation
review all pass.

The standalone E0 result measures 5,530 B flash, 1,096 B static SRAM, a 48 B
`CapturedIrEvidence` object, one explicit 400 B caller-owned pulse store, and a
197 B conservative stack path. These results are below every Lesson 052 target
and hard gate. The independent clean review found no blocking discrepancy
between the plan, implementation, tests, example, measurements, or publication
artifacts and confirmed that no emitter path accepts Lesson 052 repeat,
unknown, or malformed records.

E1 remains open. It requires exact receiver qualification, primary electrical
evidence, resource allocation, authoritative schematic, and separately
recorded bench acceptance; E0 implementation and tests cannot satisfy or
silently authorize those physical gates.

## Gate result

- Disposition: E0 implementation and promotion passed after independent clean
  post-implementation review
- Closed E0 risks: storage/view invalidation, categorical compatibility,
  exact source/session provenance, same-sequence behavior, diagnostic
  interference, capture-to-transmit isolation, standalone memory, object size,
  and conservative stack fit
- Required discussion or decision IDs: none if the additive boundary and
  bounded repairs above are retained; required before any Lesson 025 API/ISR
  change, generic persistence/replay abstraction, evidence-loss compression,
  or captured-record transmission authority
- Remediation owner and next action: none for E0; preserve the frozen boundary
  and route any future public/API/resource change through a new stress pass
- Verification commands and results: `make headers-check`, `make check`,
  `make sanitize-check`, `make arduino`, `make lessons-check`,
  `make site-check`, and `make release-check` passed; strict warning,
  format/style/diff, archive identity, PDF classification/render, link/index,
  download, and independent review gates passed
- Measured proof: 5,530 B flash; 1,096 B static SRAM; 48 B reusable object;
  400 B explicit caller storage; 197 B conservative stack path; every Lesson
  052 target and hard gate passed
- Maximum-composition scenario and proof: deterministic collision replay and
  the separate complete Lessons 052--054 linked composition passed their
  reviewed E0 hard gates
- E0 implementation permitted: yes
- Promotion permitted: yes for E0
- E1 permitted: no; exact physical qualification and bench acceptance remain
  open
