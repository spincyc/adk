# Lesson 079 bounded-low-side-driver architecture stress pass

Status: initial pre-implementation E0 disposition under the controlling
[Lessons 079--081 plan](LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md).
Implementation, measured resources, independent review, publication, endpoint
ownership, and every physical claim remain open.

## Boundary and topology

`BoundedLowSideDriverPolicy` is pure copied-evidence policy. It owns no GPIO,
pin, registry claim, clock, timer, transport, supply, transistor, diode, load,
or storage. It models exactly one active-high bare-NPN low-side topology.
There is no active-low Boolean variant and no PNP, MOSFET, Darlington,
high-side, H-bridge, relay-module, or generic driver interpretation.

The descriptor and plan-controlled arithmetic explicitly include expiry,
maximum active duration, duty window/limit, voltage extrema, resistor
tolerance, exact diode identity/revision/orientation/return/rating fields, and
all current ceilings. Inductive declarations require complete nonzero diode
fields; resistive declarations require canonical absence. E0 validates copied
declaration consistency but authenticates and measures nothing.

The policy computes one canonical CRC-32/ISO-HDLC identity digest beginning
with exactly 8 ASCII bytes `ADK79DSC` (hex
`41 44 4B 37 39 44 53 43`), no NUL/length/separator, then canonical
little-endian descriptor fields. The variant is polynomial `0x04C11DB7`,
reflected input/output, initial/final XOR `0xFFFFFFFF` (reflected
implementation polynomial `0xEDB88320`). It covers every immutable descriptor
field, including specimen-family identity,
source-packet digest, all revisions, configuration, topology/protection,
diode, budget, voltage, tolerance, duration, and duty fields. Every intent
carries that exact digest; zero is valid. Independent goldens and one-field
mutation tests prove complete coverage rather than relying on a partial tuple;
goldens explicitly reject omitted, NUL-terminated, or altered domain tags.

This fits naturally above endpoints because intent is data and never executes
itself. Embedding `DigitalOutput` would improperly turn declaration review
into electrical authority and remains prohibited.

## Lifecycle, control, and fault pressure

Construction is inert. Initialization validates one immutable descriptor and
publishes off. Begin-session fixes nonzero lifecycle/session/run identity.
Apply and `cancel(const LowSideControl&, ...)` receive full source,
configuration, sequence, supplied-time, lifecycle, session, run, step, and
control provenance. Identifier-only cancel APIs are prohibited.

Active intent is possible only for a healthy, complete, current, correlated
request within current, voltage, duration, duty, tolerance, and flyback
bounds. Off is canonical low for the sole active-high topology. Cancellation
always removes authority and forces the all-off path. Confirmed off produces
`Cancelled`; an attributable failure to command or confirm off produces
`Fault`, retaining cancellation as cause and the producer status for
attribution. Expiry, reset, shutdown, invalid arithmetic, and producer fault
can never leave active intent.

The exact plan API makes those limits enforceable: every active request
supplies a nonzero bounded duration, every admitted intent publishes its
supplied-time deadline, and `update(now)` expires at equality without new
evidence. A fixed eight-entry ring retains active reservations, prunes
intervals ending at/before the rolling-window start, sums widened overlap, and
admits only at or below the permille duty inequality. Early off/cancel shortens
the current interval; a ninth still-overlapping interval preserves all eight
reservations but atomically publishes an off
`Rejected/CapacityExceeded` snapshot and caller result. Equality/one-over,
rollover, half-range,
early-close, pruning, replay-without-extension, and update atomicity are
mandatory tests.

The eight reservations and last accepted supplied-time floor persist for the
entire object lifetime. Reset and shutdown publish off and invalidate session
authority but do not clear, shorten, or prune history; shutdown/reinitialize
on the same object preserves it. With no supplied time, an active reservation
remains charged through its original deadline. Only a chronology-valid
supplied-time apply, update, or cancel may prune expired entries. A new
session's sequence may restart, but its first time cannot move behind the
preserved floor.

Construction alone begins empty. Destruction/reconstruction loses volatile
E0 history, but that fact grants no drive authority and proves no physical
cooldown. This policy is not a physical interlock. Any E2 fixture must
independently own duration/duty/thermal bounds, retained or fail-closed restart
state, time evidence, power removal, and bench acceptance; it cannot interpret
an empty reconstructed E0 policy as permission to energize hardware.

All arithmetic is widened and checked, with required base current rounded up
and supported load rounded down. No saturation, typical gain, absolute
maximum, or Boolean command substitutes for conservative bounds. Every
mutating call stages a candidate; invalid structure/correlation leaves state
and output byte-identical.

Tolerance arithmetic is exact. For `t` in 0--999 permille, lower resistance is
`floor(R*(1000-t)/1000)` and upper resistance is
`ceil(R*(1000+t)/1000)`, with widened checked products. A zero lower bound
rejects. Lowest resistance and zero assumed minimum base-emitter drop produce
`ceil(logicHighMaximumMv*1000/lowerR)`, which must not exceed the declared
GPIO/base-path ceiling. Highest resistance produces
`floor(max(0, logicHighMinimumMv-baseEmitterMaximumMv)*1000/upperR)`;
requested base current must fit that minimum before forced-gain/load
sufficiency is evaluated. Thus safety uses the low-resistance/high-current
side while drive sufficiency uses the high-resistance/low-current side.

## Evidence gates

E0 is copied/synthetic policy only. E1 is strictly unpowered identity,
source, trace, markings, passive continuity/resistance/diode-mode, topology,
and energy classification. Powered direct/current-limited fixtures are E2a.
The transistor-switched/external/inductive fixture described here is E2b and
requires exact specimen/diode/load identities, formal schematic, current
limits, independent power removal, named test points, stored-energy
disposition, rollback, and bench acceptance. No powered observation is E1.

## Deterministic and composition proof

Tests exhaust every encoding and canonical absence; exact identity/revision
correlation; sole-topology enforcement; every zero/maximum divisor and widened
overflow; tolerance 0/1/999, lower-floor/upper-ceil remainder vectors,
zero-lower-R, maximum-current-ceil/minimum-drive-floor, GPIO/base and required
drive one-below/equal/one-above, voltage-product and widened-resistance
boundaries; duration/duty boundaries; conservative arithmetic goldens;
complete/missing/reversed/under-rated flyback declarations; source
ineligibility; healthy active/off; expiry; supplied-provenance cancellation;
producer and all-off faults; all collision precedence; duplicate/gap/
regression/rollover/half-range/future/stale chronology; lifecycle reset and
generation exhaustion; reset/shutdown/reinitialize with preserved reservation
ring and chronology floor; backdated new-session rejection; pruning only on
valid supplied-time apply/update/cancel; construction-empty and reconstructed-
no-authority fixtures; shutdown from active; canaries/nonmutation; and zero
hardware/resource/clock/storage calls.

Exact initial resource gates are:

| Metric | Target | Hard |
|---|---:|---:|
| flash | 10 KiB | 14 KiB |
| static SRAM | 768 B | 1,024 B |
| synchronous stack | 320 B | 448 B |
| policy | 192 B | 256 B |
| descriptor | 96 B | 128 B |

The maximum Lesson 081 fixture counts the production child once. The exact
fingerprint binds compiler, flags, source closure, probe, budgets, and schema.
A target miss needs independent fingerprint-bound disposition; a hard miss
blocks. Storage follows the plan's count-once lifetime rule.

## Initial gate result

Disposition: natural E0 fit under the exact plan contract. Promotion remains
blocked on implementation, deterministic/sanitizer proof, ordinary and exact
Mega evidence, terminal stress review, HTML/PDF/pencil, site, packaging, and
independent review. Exact E1/E2a/E2b/E2c physical work remains separately
blocked and no supported-specimen claim is made.

## Terminal gate result

Disposition: natural E0 fit, host verified, with no powered-hardware claim.
The ordinary canonical Mega replay measures 11,560 bytes flash and 561 bytes
static SRAM. The isolated no-LTO probe measures 12,974 bytes flash, 561 bytes
static SRAM, 320 bytes conservative synchronous stack, a 240-byte policy, a
96-byte descriptor, and 7,183 bytes residual SRAM after the 128-byte ISR
reserve. Static SRAM, stack, descriptor, residual SRAM, and every hard gate
pass.

The exact review fingerprint is
`4972bd9733d608d52ac81bfb1320e61088b10c3e59910f3be8c439b5838d33e7`.
Flash is an independently accepted target miss: observed 12974 bytes against
the 10240-byte target and 14336-byte hard limit. The complete checked
arithmetic, descriptor digest, chronology, fixed duty history, reset-generation
handling, and atomic all-off behavior remain in the measured source closure,
which retains 1362 bytes of hard-limit margin. Removing those proofs to recover
the target would weaken the terminal contract.

Policy size is an independently accepted target miss: observed 240 bytes
against the 192-byte target and 256-byte hard limit. Its copied descriptor,
eight bounded duty reservations, chronology floor, reset generation,
lifecycle/session authority, and complete published intent are required
value-owned state and are counted exactly once. The result retains 16 bytes of
hard-limit margin; future object growth requires a new fingerprint-bound
disposition.

Terminal promotion preserves the E0 boundary: no GPIO, endpoint, resource
claim, clock, supply, transistor, diode, load, persistent store, or powered
observation exists. Exact E1/E2a/E2b/E2c work and physical acceptance remain
open.
