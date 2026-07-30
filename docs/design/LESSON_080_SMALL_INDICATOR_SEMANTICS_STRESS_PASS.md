# Lesson 080 small-indicator-semantics architecture stress pass

Status: initial pre-implementation E0 disposition. The controlling
[Lessons 079--081 plan](LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md)
freezes the public contract. Implementation, measured resources, independent
review, publication, and all physical acceptance remain open.

## Boundary and natural fit

`SmallIndicatorSemanticsPolicy` is one stateful pure policy over a copied
`LowSideDriveIntent`, L080-owned `SmallIndicatorSemanticRequest`, and
independently attributable `SmallIndicatorObservation`. It owns no hardware.
The request is necessary selected-channel and correlation authority: without
it a multichannel observation would self-declare the color it is meant to
prove. It does not own an endpoint or change Lesson 079's published contract.

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
declarations. The bounded six-bit declared/selected/observed/semantic masks
preserve color; bits 6--7 reject. Follows-drive requests select one declared
channel when active and zero when off. Autonomous requests select the complete
declared enable mask, while observations may report nonzero subsets.
Observation-only requests select zero.

The plan separates malformed encoding from well-formed semantic disagreement.
NotObserved and Fault have zero masks; Fault alone has non-OK status. Inactive
has zero mask. Active/Alternating have a nonzero declared subset and allow
copied-level/transition values that may prove polarity, unexpected autonomy,
or missing autonomous waveform failures. Safe state is false for nonzero
selection and missing/fault evidence; for a zero selection it is independently
evaluated, including while a Voltage Indicator reports voltage. Thus semantic
failure reasons remain reachable rather than being API errors.

This is a natural component-layer extension because it interprets attributable
copied values without gaining hardware authority. Lesson 079 remains the sole
producer of bounded low-side intent; Lesson 081 composes both policies without
reinterpreting either.

## Exact semantics and lifecycle pressure

Only schema revision `1` is supported. Durations zero through `0x7fffffff`
are valid; `0x80000000` through `UINT32_MAX` reject. Zero warm-up and settling
are immediately satisfied; zero maximum age admits only equal observation and
policy time. Identities are nonzero except the explicitly zero-valid driver
digest.

Request and observation repeat lifecycle/session/run/step/request correlation,
but are independently attributable source/configuration/sequence/time/status
streams. Their sources latch separately and may differ. Control belongs to the
request/control source, not the observation source. The Lesson 079 intent has
no source, source-configuration, sequence, or observation-time field and the
contract does not invent one.
The immutable descriptor also freezes expected Lesson 079 specimen
reference/revision, electrical-evidence revision, and policy-configuration
identity/revision. Every copied drive intent carries those actual inspectable
fields; mismatch rejects before standalone L080 semantic interpretation.
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
Request, observation, and the shared apply/cancel policy stream start at one
and advance contiguously; wrap to zero is exhaustion. A complete immediate
duplicate, including `now`, is idempotent; a changed duplicate is atomically
invalid. Gap, regression, exhaustion, or timestamp discontinuity publishes a
terminal rejection, caches only that rejected tuple for replay, and does not
advance accepted anchors. Reset is the sole recovery.
Incomplete alone admits a later contiguous apply and advances accepted
anchors. Other structurally valid terminal results advance anchors, cache
their tuple, and admit only replay/reset; discontinuity is the deliberate
no-anchor-advance exception.

Chronology is session start <= request <= observation <= now, forward-or-equal
within the modular half range. Warm-up derives from start-to-now, settling
from request-to-observation, and freshness from observation-to-now; copied
satisfaction assertions do not exist. Future/backward/half-range time is a
timestamp discontinuity; age at the maximum passes and one tick over is stale.

`cancel(const SmallIndicatorControl&, ...)` receives the full supplied-time
and provenance value. No identifier-only cancellation API is permitted.
Cancellation has two canonical tuples. Confirmed+OK publishes
Cancelled/Cancelled, Inactive, zero/inactive output, safe-state true, and OK.
Unconfirmed+non-OK publishes ProducerFault/Cancelled, Fault, zero/inactive
output, safe-state false, and the failing status. The other two pairings are
malformed. Construction is generation zero; initialize advances it; one
session begins per generation. Reset advances generation and clears session
and anchors atomically; exhaustion does not mutate. Shutdown is idempotent,
clears authority/anchors, and retains generation; reinitialize advances it.

The five kinds remain semantically distinct:

- `ActiveBuzzer` follows drive or is autonomous only as declared.
- `TrafficLight` and `DualColorLed` require explicit population declarations;
  E0 never invents channel topology.
- `AutoFlashLed` is `AutonomousWhileEnabled` and requires copied transition
  evidence.
- `VoltageIndicator` is observation-only and never receives active drive
  authority.

The plan freezes exact L079 mappings, including `Off/Expired`,
`Fault/ProducerFault`, and failed-cancel `Fault/Cancelled`. Unlisted
state/reason/activity/level/status tuples are malformed. Precedence after
structure/correlation is cancel/shutdown; request, drive, then observation
producer fault; mapped drive terminal; chronology/freshness; warm-up;
settling; semantic disagreement; safe-state; acceptance. Missing evidence is
incomplete, never inactive, and plausible evidence cannot erase a fault.
Exact incomplete results are `ObservationMissing`, `WarmupUnsatisfied`, and
`SettlingUnsatisfied`; semantic and safe-state disagreements are named
rejections. Initialize/reset are Idle, begin-session is Eligible, and only
complete matching evidence is Accepted.

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

Tests exhaust every encoding/cross-product; all five fixtures; request-selected
colors and every observation row/mutation; schema and duration boundaries;
independent request/observation source latching; all three sequence streams;
duplicate, gap, regression, exhaustion, rollover, half-range, future,
equality, and stale time; every canonical and malformed L079 tuple; computed
warm-up/settling/freshness; polarity/autonomy/safe-state independence; both
valid and malformed cancel tuples; precedence collisions; lifecycle/reset/
shutdown; canaries and nonmutation; and zero hardware calls.

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

## Terminal gate result

Disposition: natural E0 fit, host verified, with no powered-hardware claim.
The ordinary canonical Mega replay measures 10,866 bytes flash and 520 bytes
static SRAM. The exact no-LTO probe measures 14,910 bytes flash, 683 bytes
static SRAM, 230 bytes conservative synchronous stack, a 293-byte policy
object, a 39-byte observation evidence value beside the 35-byte request and
30-byte result, and 7,151 bytes residual SRAM after the 128-byte ISR reserve.
Static SRAM, synchronous stack, policy object, evidence value, and every hard
gate pass.

The exact review fingerprint is
`b0e4ee371afa072c9e2b1c0afabffe3c2a4a31f1f613da72225b2958ebfd4e7f`.
Flash is an independently accepted target miss: observed 14910 bytes against
the 14336-byte target and 20480-byte hard limit. The measured source closure
retains the complete closed five-kind validity table, the full Lesson 079
state/reason mapping, the CRC-32 driver-descriptor digest check, three
independent contiguous sequence streams with idempotent replay caching, the
computed warm-up/settling/freshness chronology, and the full precedence
ladder. Removing those proofs to recover 574 bytes would weaken the terminal
contract; the result retains 5570 bytes of hard-limit margin, and future
flash growth requires a new fingerprint-bound disposition.

Terminal promotion preserves the E0 boundary: no GPIO, endpoint, resource
claim, clock, waveform, supply, indicator, sound, light, display, storage, or
powered observation exists. Exact E1/E2a/E2b/E2c work and physical acceptance
remain open.
