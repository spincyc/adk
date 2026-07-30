# Lesson 080 small-indicator-semantics architecture stress pass

Status: initial pre-implementation E0 disposition. The controlling
[Lessons 079--081 plan](LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md)
freezes the public contract. Implementation, measured resources, independent
review, publication, and all physical acceptance remain open.

## Boundary and natural fit

`SmallIndicatorSemanticsPolicy` is one stateful pure policy over copied
`LowSideDriveIntent` and `SmallIndicatorObservation` values. It owns no pin,
endpoint, clock, scheduler, waveform, supply, transport, display, or storage.
Construction copies one immutable descriptor; initialize, begin-session,
apply, supplied-provenance cancel, reset, and shutdown use the repository
lifecycle and atomic candidate conventions.

The public product is closed to exactly five concrete kinds:
`ActiveBuzzer`, `TrafficLight`, `DualColorLed`, `AutoFlashLed`, and
`VoltageIndicator`. The plan's kind/autonomy/safe-state table and explicit
descriptor fields control. There is no generic role/medium product, variant
payload, universal three-pin-module abstraction, or permissive fallback.
Adding a sixth kind requires a new design pass and exact validation/tests.

The controlling plan freezes the only valid combinations:

| Kind | Autonomy | Safe state | AH | Exact mask | R | D |
|---|---|---|---:|---:|---:|---:|
| Active Buzzer | follows drive | drive inactive | 1 | Sound | 0 | 1 |
| Active Buzzer | autonomous enabled | unpowered | 1 | Sound | 0 | 1 |
| Traffic Light | follows drive | drive inactive | 1 | Red/Amber/Green | 1 | 0 |
| Dual Color LED | follows drive | drive inactive | 1 | Red/Green | 1 | 0 |
| Auto Flash LED | autonomous enabled | unpowered | 1 | Red/Green/Blue | 1 | 0 |
| Voltage Indicator | observation only | high impedance | 1 | Voltage | 1 | 0 |

`AH`, `R`, and `D` are exact active-high/populated-resistor/driver
declarations. The bounded
six-bit declared/expected/observed/semantic masks preserve Traffic Light and
Dual Color LED colors; bits 6--7 reject. Follows-drive masks match exactly,
autonomous masks remain nonzero declared subsets with copied transition
evidence, and observation-only expected mask is zero.

`NotObserved` and `Fault` use canonical zero expected/observed masks;
`Inactive` has zero observed mask. Multichannel rows require canonical false
`copiedLevelHigh` because masks alone carry color. For the single-channel
Buzzer and Voltage rows, copied level equals nonzero observed mask under the
frozen active-high declaration.

This is a natural component-layer extension because it interprets attributable
copied values without gaining hardware authority. Lesson 079 remains the sole
producer of bounded low-side intent; Lesson 081 composes both policies without
reinterpreting either.

## Exact semantics and lifecycle pressure

Every descriptor and observation binds schema, specimen/electrical/source
revisions, source-packet digest, configuration, lifecycle generation,
session/run/step, producer identity/configuration, sequence, supplied time,
and full producer status as frozen by the plan. Zero identities are invalid.
The immutable descriptor also freezes expected Lesson 079 specimen
reference/revision, electrical-evidence revision, and policy-configuration
identity/revision. Every copied
drive intent carries those fields; mismatch rejects before standalone L080
semantic interpretation.
It additionally freezes the expected canonical full Lesson 079 descriptor
identity digest. The drive intent carries the actual digest, and equality is
mandatory before those inspectable partial fields are considered. This binds
specimen-family reference, source-packet digest, and every other descriptor
field; tests mutate family and source packet independently. Digest zero is
valid and never means absent.
Both sides use the exact Lesson 079 digest domain: eight ASCII bytes
`ADK79DSC`, hex `41 44 4B 37 39 44 53 43`, with no NUL, length, or separator,
then canonical little-endian descriptor bytes under CRC-32/ISO-HDLC
(`0x04C11DB7`, reflected input/output, initial/final XOR `0xFFFFFFFF`;
reflected implementation `0xEDB88320`). Standalone goldens use the literal
bytes rather than production digest code and reject omitted,
extra-NUL-terminated, and wrong-tag variants.
Duplicates are idempotent only when fieldwise identical; changed duplicates,
gaps, regressions, stale/future/half-range times, identity drift, and exhausted
generations reject atomically.

`cancel(const SmallIndicatorControl&, ...)` receives the full supplied-time
and provenance value. No identifier-only cancellation API is permitted.
Cancellation forces the composed all-off path. When off is attributable and
confirmed, the result is cancelled. If an attributable producer/endpoint
failure prevents confirmation or reports stuck/failure-to-return-safe, the
terminal disposition is `ProducerFault`/fault while cancellation remains the
cause. Shutdown is terminal and idempotent; reset advances lifecycle
generation and cannot admit evidence from the prior generation.

The five kinds remain semantically distinct:

- `ActiveBuzzer` follows drive or is autonomous only as declared.
- `TrafficLight` and `DualColorLed` require explicit population declarations;
  E0 never invents channel topology.
- `AutoFlashLed` is `AutonomousWhileEnabled` and requires copied transition
  evidence.
- `VoltageIndicator` is observation-only and never receives active drive
  authority.

Warm-up, settling, maximum age, polarity, autonomous transition, and safe
state are independent checks. Safe-state evidence never doubles as resource
admission. A plausible observation cannot erase producer failure or stale
provenance. Missing evidence is incomplete, not inactive.

## Evidence and safety gates

E0 is copied/synthetic policy only. E1 is strictly unpowered identity,
primary-source, both-face, pin-trace, population, passive continuity,
resistance, and diode-mode work. Every energized direct/current-limited
indicator fixture is E2a. Every transistor-switched, externally powered, or
inductive fixture is E2b. E2c is separately selected presentation or
persistence work. No E1 record contains powered behavior.

Exact adapters remain separate owners with exact identity, resource,
electrical, rollback, safe-state, and bench acceptance. The indicator under
qualification cannot also be the independent observation proving itself.
Serial is supporting evidence only.

## Deterministic proof

Tests exhaust every encoding and valid/invalid kind cross-product; all five
concrete fixtures; every mask bit/subset/reserved bit and color mismatch;
zero/max identities and durations; lifecycle/session/source
correlation; sequence/time duplicate, gap, regression, rollover, half-range,
future, equality, and stale boundaries; off/active/rejected/cancelled/fault
driver intents; every observation state; warm-up, settling, polarity,
autonomy, and safe-state independence; supplied-provenance cancellation;
cancel/fault/all-off collisions; restart, generation exhaustion, reset, and
shutdown from every state; canaries and byte-stable nonmutation; and zero
hardware/resource/clock/storage calls.

The Lesson 081 maximum composition retains one production policy instance, not
a duplicate standalone child. Resource gates are exact:

| Metric | Target | Hard |
|---|---:|---:|
| flash | 14 KiB | 20 KiB |
| static SRAM | 1,024 B | 1,536 B |
| synchronous stack | 448 B | 640 B |
| policy | 384 B | 512 B |
| evidence value | 256 B | 384 B |

The exact fingerprint binds compiler, flags, source closure, probe, budgets,
and schema. A target miss needs independent fingerprint-bound disposition; a
hard miss blocks. Lifetime storage is counted once as specified by the plan.

## Initial gate result

Disposition: natural E0 fit under the controlling exact contract. Promotion
is blocked on implementation, deterministic and sanitizer proof, ordinary and
exact Mega evidence, terminal stress review, HTML/PDF/pencil gates, site and
packaging gates, and independent review. E1, E2a, E2b, and E2c remain open and
no physical-support claim is made.
