# Lesson 081 inert-component-qualification-bench architecture stress pass

Status: initial pre-implementation E0 disposition under the controlling
[Lessons 079--081 plan](LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md).
Implementation, exact resources, independent review, publication, and all
physical acceptance remain open.

## Boundary and composition

`InertComponentQualificationBench` composes the stateful Lesson 079 and 080
policies over one immutable descriptor pair and one atomic copied envelope.
It owns no hardware, power, clock, transport, presentation endpoint, or
storage. One exact 256-byte caller-owned volatile image is its only encoded
artifact; it is not persistence, authentication, calibration, certification,
or authority to energize a specimen.

This is a natural project-layer composition only while identity, chronology,
fault attribution, and child ownership remain explicit. The bench contains
its production children; resource probes must not instantiate duplicate
standalone copies.

## Exact script and controls

The script has exactly five ordered steps:

1. `ReviewSource`;
2. `ConfirmInactive`;
3. `RequestBoundedIntent`;
4. `ObserveResponse`; and
5. `ConfirmSafeState`.

Record preparation is outside the script and legal only after a terminal
state. It never creates a sixth step or mutates encoded semantic state.
`recordPrepared` is absent from `ComponentQualificationSnapshot` and the
record; whether caller memory contains bytes is external state.

Every advance or cancel receives a complete
`ComponentQualificationControl`: lifecycle generation, session/run/step,
control identity, producer source/configuration, sequence, supplied
observation time, closed action, and producer status. Child cancellation uses
the corresponding full `LowSideControl` and `SmallIndicatorControl`.
Identifier-only cancellation APIs are prohibited.

Cancellation always forces the all-off path. If attributable evidence confirms
off, terminal disposition is `Cancelled`. If the driver, observation source,
or endpoint reports failure to command or confirm off, terminal state is
`Fault`; cancellation remains the causal reason and all producer statuses are
retained. A cancellation request cannot convert stuck-active evidence into a
benign terminal result.

Every call validates structure, identity, lifecycle/session, source,
configuration, sequence, and time before semantic precedence. A correlated
producer failure is admitted domain evidence. Candidate state is committed
once; rejected calls leave bench, children, output, record destinations, and
canaries byte-identical.

## Canonical 256-byte record

The plan's offset table is normative. Version 1 includes lifecycle generation,
session/run, qualification sequence, complete terminal control provenance,
driver and indicator specimen/source/configuration identities, compact
energy/electrical summaries, diode identity/orientation/return/rating,
terminal request/observation chronology, dispositions/statuses, presentation,
domain-separated descriptor/driver/indicator/envelope/script/source/electrical
digests, a fixed record sequence, reserved zero bytes, and CRC-32/ISO-HDLC.
The terminal control step identity is encoded explicitly. Packed fields at
offsets 89, 91, and 178 use the plan's exact bit assignments and reject every
reserved bit. Offset 173 reserves bits 2--7 zero; offsets 180, 181, 182, and
183 are respectively Source, Drive, Observation, and Safe State cells. Compact
driver identity fields and the full-replay digests bind the five Lesson 079
intent-correlation fields consumed by standalone Lesson 080.
Offset 184 is the actual canonical full driver-descriptor identity digest and
offset 234 is L080's expected digest; bytes 238--251 are reserved zero. The
full replay verifier recomputes the actual value over every descriptor field,
including specimen family and source packet, and proves equality. Compact
decode treats both values as opaque.
That recomputation begins with exactly eight ASCII bytes `ADK79DSC` (hex
`41 44 4B 37 39 44 53 43`), no NUL/length/separator, followed by canonical
little-endian descriptor bytes, using CRC-32/ISO-HDLC polynomial
`0x04C11DB7`, reflected input/output, initial/final XOR `0xFFFFFFFF`
(reflected implementation polynomial `0xEDB88320`). Record/replay goldens use
the literal tag bytes rather than production digest code and reject omitted,
extra-NUL-terminated, and wrong-tag variants.

Canonical omissions are full intermediate envelopes and snapshots,
duplicate/rejected candidates, raw C++ layouts/status representations, and
descriptor/evidence fields already bound by named domain digests. Active-high
and off-low are derived from the sole Lesson 079 topology. Record existence is
external and is neither encoded nor derived.

Decode precedence is length, framing, record CRC, then canonical compact
semantic validation. Every descriptor/source/electrical/script/evidence digest
is opaque to the compact decoder; zero is valid and compact fields are never
claimed sufficient for recomputation. `ComponentQualificationReplayVerifier`
receives the complete five-step replay, validates Lesson 079 arithmetic/duty,
Lesson 080 kind/channel semantics, chronology and correlation, recomputes all
eight domains, and compares the decoded digests. Encode invokes that verifier
before staging bytes. Decode or verify failure leaves output unchanged.

Preparation stages exactly 256 private bytes and copies them only after all
checks pass. Null, 255-, or 257-byte spans reject without mutation. Repeated
preparation from unchanged terminal state is byte-identical.

## Evidence levels

E0 is copied/synthetic replay, inert presentation, and volatile record only.
E1 is strictly unpowered identity/source/trace, photographs, markings, passive
continuity/resistance/diode-mode, topology, and energy classification. Direct,
low-energy, independently current-limited powered fixtures are E2a.
Transistor-switched, externally powered, or inductive fixtures are E2b. Exact
presentation or persistence is E2c. No powered observation is credited to E1,
and no E0/E1 result pre-fills an E2 acceptance record.

## Deterministic proof

Tests cover the exact five-step happy path and byte-identical replay; every
state/transition; early, late, duplicate, skipped, changed, cancel, reset, and
shutdown controls; lifecycle/session exhaustion; all descriptor/source/
configuration/request/observation correlations; all sequence/time boundaries;
all structural-versus-domain precedence; cancel plus successful/failed all-off
collisions; every Lesson 079 arithmetic/protection outcome; every Lesson 080
kind/semantic outcome; exact 256-byte vector and round trip; every-byte
corruption; repaired-CRC invalid framing/enums/flags/reserved/cross-fields;
valid zero digests; domain separation; decoder opacity; replay-verifier
mismatch; buffer and output canaries; and zero hardware/storage calls.
Bench reset and shutdown/reinitialize preserve the contained Lesson 079
reservation ring and chronology floor. Tests prove a new qualification session
cannot erase or backdate that history, retained duty may reject its bounded
request, and only a valid supplied-time child operation prunes it.

## Maximum composition and exact resources

The maximum fixture contains one production bench with its children, one full
envelope, one snapshot/presentation value, one decode output where its phase
requires it, and exactly two simultaneous images (private stage plus caller
destination = 512 B).
Standalone child objects are measured only in isolated Lesson 079/080 probes
and are never additional live objects in this maximum graph.

| Metric | Target | Hard |
|---|---:|---:|
| flash | 32 KiB | 40 KiB |
| static SRAM | 3,072 B | 4,096 B |
| synchronous stack | 1,024 B | 1,280 B |
| coordinator | 768 B | 1,024 B |
| one/two record images | exactly 256 B | exactly 512 B |
| residual SRAM | 4,096 B | 3,072 B |

Count storage once by actual lifetime. Measured static already includes
globals; measured stack includes live locals and hidden returns; objects or
images already resident there are not subtracted again. Mutually exclusive
phase locals may overlap only with compiler/call-graph evidence. Residual is
`8192 - measured static - conservative synchronous stack - 128-byte ISR
reserve`. The fingerprint binds compiler, flags, source closure, probe,
budgets, and layout. Target misses require independent fingerprint-bound
review; hard or residual-hard misses block.

## Initial gate result

Disposition: natural E0 composition under the exact controlling contract.
Promotion remains blocked on implementation, host/sanitizer/style/header
proof, ordinary and exact Mega measurement, terminal stress review, golden
record/replay review, HTML/PDF/pencil, site, packaging, shared indexes, and
independent review. All physical gates remain open.
