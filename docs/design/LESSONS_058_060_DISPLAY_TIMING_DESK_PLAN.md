# Lessons 058--060 display timing desk plan

Status: implementation-depth E0 plan; exact powered display endpoints remain
open.

This arc adds a pure four-digit multiplex policy, a MAX7219 register-frame
policy over a recording transport seam, and a deterministic timing-desk
coordinator. E0 owns no pin, timer, interrupt, bus wire, display, module,
supply, or powered circuit. It proves copied logical intent, bounded command
traces, and deterministic presentation attribution. It does not prove that a
physical display changed, blanked, refreshed without flicker, or agreed
optically with another display.

The earlier cadence described a four-digit resource owner and a powered
MAX7219 adapter. This plan deliberately separates those future E1 endpoints
from the E0 policies. Exact electrical ownership remains retained work rather
than an implied property of fake-backed examples.

## Evidence levels

| Level | Authorized work |
|---|---|
| E0 | Host and compile-only Mega replay over supplied time, copied controls, logical digit transactions, MAX7219 register intents, and recording transport evidence |
| E1a | Exact 5641AS or separately identified four-digit fixture, qualified segment and digit drivers, current limits, waveform evidence, and blank-on-release |
| E1b | Exact MAX7219 IC/module revision, schematic, RSET, matrix orientation, power/current budget, serial timing, and observed blanking |
| E1c | Combined timing desk after E1a and E1b independently pass, including optical agreement and simultaneous-current evidence |

The official inventory establishes only a four-digit display family and
randomly shipped MAX7219 variants; it does not prove the learner's model,
polarity, module revision, resistor network, or schematic. Those facts
authorize planning, not power; the checked-in
[authorized inventory](../inventory/AUTHORIZED_ELEGOO_SET.md) and
[kit coverage audit](ELEGOO_MEGA_KIT_COVERAGE_2026-07-27.md) retain the source
and specimen limitations.

## Dependency and ownership

| Lesson | Boundary | Depends on | Owns at E0 |
|---:|---|---|---|
| 058 | `MultiplexedDigitPolicy` | `Status`, supplied `TimePoint`, canonical seven-segment glyph encoding | Four copied glyph cells, timing state, generation, and ordered blank/segment/select intent |
| 059 | `Max7219PresentationPolicy` | `Status`, fixed matrix frame and copied recording-transport result | Configuration and row intents, desired/last-submitted state, generation, and fault provenance |
| 060 | `DualDisplayTimingDesk` | Lessons 058--059 plus copied button and acknowledgement evidence | Stopwatch state, one immutable presentation snapshot, two generation-bound intents, and disagreement policy |

No component reads a clock, retains caller pointers, allocates, recurses,
invokes callbacks, retries, sleeps, or catches up in a loop. Every update does
bounded work. The project owns its child policies; physical adapters remain
outside the E0 composition.

Lesson 010's `SevenSegmentGlyph` and polarity vocabulary remain canonical.
Implementation must extract one pure glyph encoder and regression-test Lesson
010 plus every current consumer in Lessons 036 and 039 rather than silently
duplicate its private truth table. Existing `encodedValue()` polarity and byte
semantics remain byte-identical. This is a bounded prior-interface evolution,
not a generic display framework.

## Public and file shape

The boundary adds `src/seven_segment_glyph.h`,
`src/multiplexed_digit_policy.{h,cpp}`,
`src/max7219_presentation_policy.{h,cpp}`, and
`src/dual_display_timing_desk.{h,cpp}`, corresponding host tests, three
canonical Mega examples, HTML references, TeX lessons, and exact probes.

Each policy is non-copyable/non-movable and exposes
`initialize()`/`reset()`/`shutdown()`/`initialized()`/`snapshot()`.
Lesson 058 also exposes `preview()`/`canCommit()`/`commit()` and
`refresh(now)`. Lesson 059 exposes the matching frame transaction plus
`service(receipt)`, returning at most one copied register command. Lesson 060
exposes `update(envelope)` and `snapshot()`; its result separates
`controlStatus`, `presentationDisposition`, and emitted child commands.
Public enums cover lifecycle, polarity, refresh fault, register operation,
transport disposition, stopwatch phase, self-test stage, and agreement.
Cross-call values carry owner/lifecycle/configuration/generation attribution.

## Lesson 058: multiplexed digit policy

The fixed logical display has four left-to-right cells and eight segment bits
`a,b,c,d,e,f,g,dp`. The input is `uint32_t`; values `0..9999` render
numerically and larger values render overflow. Leading zeros are either shown or
blanked, while zero always retains one zero. Decimal-mask bit zero names the
rightmost cell. Overflow renders four dashes and clears decimal points; it
never wraps or truncates.

Segment polarity and digit-select polarity are independent configuration
values. Common-anode/cathode metadata never infers the external digit-driver
topology.

Each due refresh emits one bounded three-stage transaction:

1. all digit selects inactive with the prior segment levels;
2. all selects inactive with the next segment levels;
3. exactly one selected digit active.

The phase order is left-to-right `0,1,2,3`. The default service period is one
millisecond and the maximum accepted gap is four milliseconds. Calls before
the period emit nothing. A due call advances exactly one digit and reanchors
to the supplied time; it never bursts to catch up. A gap greater than the
maximum latches `RefreshLost`, emits blank intent, and requires explicit
reset. The maximum gap is measured between accepted services; equality is
valid and the next tick faults. Equal timestamps are idempotent after the
first service. Backward or exact half-range time rejects atomically.

Formatting uses preview/can-commit/commit. A complete glyph bank becomes
visible only at the next digit-zero boundary, preventing a torn logical
value. Each value carries a local generation and caller-supplied source
snapshot sequence. Initialization establishes blank intent and arms digit
zero for one explicit first-service exception: the first same-time refresh
emits digit zero's complete three-stage transaction. `reset(now)` preserves
immutable configuration, invalidates previews, clears the frame and returns
to that initialized blank/armed state. Shutdown invalidates previews, is
idempotent, and requires a later `initialize(now)` before service resumes.

E1a must own and roll back every segment/digit resource, prove the exact
polarity and pin map, use one resistor per segment, use a rated digit-driver
topology rather than aggregate common current through a Mega pin, and measure
peak segment current, multiplex duty, digit-driver peak and continuous
ratings, per-port sourcing/sinking, worst-case simultaneous segments, loaded
rail/USB total, ghosting, refresh, and shutdown behavior.

## Lesson 059: MAX7219 presentation policy

Frame generation is an independent pure eight-row value. Supported
orientations are `Identity`, `Rotate90`, `Rotate180`, and `Rotate270`; row zero
is the logical top and bit seven is the logical left before transform.
Intensity is fixed to `1` in the E0 example and remains in the validated
`0..15` domain. The policy emits
exact 16-bit, MSB-first register intents for shutdown, display-test off,
decode off, scan limit seven, bounded intensity, rows one through eight, and
normal operation. Orientation is an explicit fixed transform. Initialization
keeps the device logically dark while configuring it and never uses
display-test as the learner self-test.

The E0 recording transport records chip-select framing and acceptance. A
successful write proves only that the fake transport accepted a command.
MAX7219 is write-only here: a multi-row update is not physically atomic, and
a failed prefix cannot be rolled back. State therefore distinguishes desired,
last fully submitted, partial-prefix, blank-requested, shutdown-command
accepted, and physically indeterminate. A failure retains the first failing
operation/register/row/status, enters cleanup-pending, and schedules one
best-effort shutdown command on a later service call. Cleanup status never
overwrites primary provenance.

Transport uses copied command/receipt values. One service call may publish one
command containing owner token, lifecycle generation, configuration revision,
presentation generation, operation index, one register-address byte, and one
data byte. The caller
executes it through the recording seam and later supplies a copied receipt
containing the same correlation fields, accepted-byte count, chip-select
cleanup evidence, observation time, and `Status`. `acceptedByteCount` is in
`0..2`. Success requires `Status::Ok`, count two, and chip select recorded
inactive afterward. A non-OK status may report zero or one accepted byte; a
non-OK/two-byte or OK/short-count combination is contradictory transport
evidence and faults without generation advance. The policy never calls
transport code. A call without the exact outstanding receipt changes nothing.
Foreign, stale, regressing, exact-half-range, or changed duplicate receipts
reject atomically; an identical duplicate is idempotent.

Frame candidates use owner/lifecycle/configuration/base-generation-bound
`preview()`/`canCommit()`/`commit()` just like Lesson 058. Commit changes the
desired frame without issuing transport. Under exclusive Lesson 060 ownership,
both child preflights occur before either commit, making the two pure commits
infallible. A submitted generation advances only after all eight row receipts
are accepted. Byte-position and chip-select cleanup tests belong to the
recording seam; policy tests prove receipt correlation. Progress is
caller-paced, never time-driven, and restart requires complete configuration.

The eventual adapter must reuse `SpiDevice`, not create another bus layer.
Before that adapter can promote, the repository-wide `SpiBus` terminal-fault
recovery ambiguity must be resolved: the current driver can release hardware
claims while `SpiBus` still reports initialized. This arc must not hide that
defect inside MAX7219-specific retry logic.

E1b records the exact IC/module and module schematic, matrix anode/column and
cathode/row mapping and orientation, RSET and decoupling, 4.0--5.5 V
compatibility, logic levels, serial timing, current, startup, fault, and
shutdown evidence. An unknown or low-value module current-setting resistor may
exceed this project's 100 mA USB campaign; the module remains unpowered until
its exact network is qualified.

## Lesson 060: dual-display timing desk

`DualDisplayTimingDesk` consumes copied start/pause, lap, and reset evidence.
Reset dominates all controls; simultaneous non-reset presses are ambiguous
and do not mutate the stopwatch. Held controls do not repeat. Source identity,
configuration revision, sequence, observation time, edge/level consistency,
freshness, and modular ordering are validated before mutation.

The stopwatch is stopped-zero, running, paused, or faulted. It derives elapsed
time only from supplied time. Start resumes, pause materializes elapsed, lap
copies the current elapsed value without stopping, and reset returns to
canonical zero. Capacity is `9:59.9`; reaching it saturates visibly and stops
rather than wrapping.

The project permits one outstanding presentation generation. While it is
pending, supplied-time updates continue the internal stopwatch but return
`ResourceBusy` for presentation and do not overwrite either intent. After the
generation is acknowledged or faults, the next update freezes the newest
stopwatch snapshot, derives both display values, preflights both pure policies,
and publishes both generation-bound intents. Atomic means semantic snapshot
coherence, never simultaneous photons or rollback-capable transports.

Copied endpoint receipts include owner/lifecycle/configuration identity,
requested and accepted generation, the reported complete logical frame and
reported frame digest, status,
blank-request acceptance, and observation time. Generations are nonzero
`uint32_t`, use modular half-range ordering, and fault before wrapping to zero.
Accepted means a complete logical digit bank or complete recording-transport
row sequence was admitted; it never means a scan, readback, or visible pixel.

Each side has its own digest. It is 32-bit FNV-1a (offset `0x811c9dc5`, prime
`0x01000193`) over domain `ADK.DIGIT.FRAME.V1` or
`ADK.MATRIX.FRAME.V1` as ASCII without a trailing NUL, then every fixed frame
byte, source snapshot generation,
and presentation mode, using little-endian fixed-width integers. The locally
retained full expected frame is also compared; a digest never substitutes for
structural equality.

`presentationGrace` defaults to 100 ms and must be in `[1, 1000]` ms, below
the modular half range. It anchors when the generation is published, remains
pending through the inclusive deadline, and expires on the next valid tick.
Backward and exact-half-range supplied time reject atomically. `InSync`
requires each side to match its own current generation, digest, and full
frame. One accepted
and one pending remains pending for the grace interval. Wrong digest,
stale or crossed generation, one-sided failure, refresh loss, or grace expiry
is presentation disagreement with side-specific provenance. It preserves
stopwatch history but requests both displays blank. A reset press atomically
clears elapsed/lap state, invalidates old receipts, and enters requalification;
malformed companion evidence rejects before reset precedence. Complete
self-test is required before Ready.

### Ordinary presentation mapping

Elapsed time is saturated to `9:59.9` and rendered left-to-right as minute,
seconds tens, seconds units, and tenths. Decimal mask `0x0A` lights the
leftmost separator and the seconds-unit separator. Stopped and running show
current elapsed time. A lap press freezes both frames at the lap snapshot for
exactly 2,000 ms while the internal running stopwatch continues; the inclusive
deadline still shows the lap and the next valid tick returns to current time.
Paused shows materialized elapsed. Fault and self-test frames are distinct.

The matrix dial uses the 28 perimeter pixels clockwise from top-left: across
the top, down the right, back across the bottom, and up the left without
duplicating corners. With `ms = elapsed % 60000`,
`litCount` is zero when `ms` is zero and otherwise
`1 + floor(ms * 28 / 60000)`, capped at 28; the first `litCount` pixels are
on. Thus the first millisecond lights the first pixel and the final bucket
lights all 28 before minute rollover.

The central 4x4 status glyph occupies logical rows/columns `2..5`. Its four
left-to-right row nibbles are stopped `{0xF,0x9,0x9,0xF}`, running
`{0x6,0x3,0xF,0x2}`, paused `{0xA,0xA,0xA,0xA}`, lap
`{0x8,0x8,0x8,0xF}`, and fault `{0x9,0x6,0x6,0x9}`. Lesson 059 orientation
applies only after this logical frame is complete. Golden tests fix zero, the
first pixel, every bucket boundary, full perimeter, minute rollover, and all
eight row bytes for each status.

### Coordinator service order

Each `update(envelope)` is one bounded service opportunity. It validates the
complete copied envelope, then in fixed order ingests child receipts, services
one due digit transaction, services at most one MAX command, evaluates
grace/self-test, applies valid control edges once, and advances the internal
stopwatch. If no generation remains outstanding, it then freezes the resulting
post-control stopwatch state, preflights both children, and commits/publishes
the next dual generation. Structural rejection mutates nothing. `controlStatus` reports
control/time admission independently from `presentationDisposition`;
presentation `ResourceBusy` never asks the caller to replay a control edge.
The caller must service the project often enough to meet the four-millisecond
digit gap.

Only one presentation generation may be outstanding. Grace starts after both
pure commits and before its first child service. The digit side accepts its
generation when the committed bank swaps at the next digit-zero boundary and
emits its copied complete-frame receipt. The MAX side accepts only after all
eight correlated row receipts. Initialization commands complete before
ordinary generation and grace tracking begin.

The deterministic self-test has fourteen common stages: blank; eight indexed
stages where the digit policy lights segment `a..dp` on digit `index mod 4`
and matrix policy lights pixel `(index,index)`; four digit-select stages where
the digit policy shows `8` on one digit and the matrix policy lights the
matching column; and final blank. Ready zero is the first ordinary generation.
A stage advances only after both complete correlated receipts match its
generation and expected side-specific frame. `selfTestTimeout` is 100 ms per
stage, inclusive at the deadline and faulting on the next valid tick. Failure
enters the same reset-and-requalify path. E0 proves commands and copied
acknowledgements only; E1c proves visibility.

## Deterministic proof

Tests cover every glyph, decimal and leading-zero combination, all four
segment/select polarity combinations, exact three-stage digit ordering,
frame-swap boundary, early/due/late calls, refresh loss, rollover and
half-range rejection. MAX tests cover exact configuration order and bytes,
all 64 pixels and orientation transforms, failure at both bytes of every
register/row, partial-prefix attribution, cleanup failure, restart, and
chip-select cleanup. Project tests cover every control mask and state,
capacity boundaries, lap/reset/wrap, each self-test stage, both one-sided
fault directions, disagreement/grace/recovery, shutdown, logical
owner/lifecycle/generation collisions, and byte-identical replay.

The exact resource gate starts with these target/hard ceilings:

| Boundary | Flash target/hard | Static SRAM target/hard | Stack target/hard | Object target/hard |
|---|---:|---:|---:|---:|
| 058 maximum | 12/16 KiB | 768/1,024 B | 320/448 B | 192/256 B |
| 059 maximum | 16/20 KiB | 1,024/1,536 B | 384/512 B | 192/256 B |
| 060 maximum | 24/32 KiB | 2,048/3,072 B | 640/896 B | 640/896 B |

The full E0 composition owns zero hardware resources. A future E1 topology
derives its exact pins, timers, interrupts, bus, and chip-select from qualified
drivers rather than inheriting an E0 estimate. The aggregate hard
gate retains at least 2,048 B SRAM after measured static and synchronous stack
with the repository interrupt/return-edge reserve.

The no-LTO AVR probe records tool/core/flags fingerprints, standalone linked
maximum sketches, public value sizes/alignment/traits, every policy object and
caller buffer, and the aggregate live fixture/diagnostics. Compiler-derived
synchronous call paths include the 128 B interrupt reserve and 3 B per
retained return edge. Each fixed buffer is target/hard 256/512 B. Residual SRAM is
`8192 - static - synchronous stack`; target/hard is 3072/2048 B. Target misses
require stale-failing machine-readable review markers; hard or residual misses
are non-reviewable blockers.

## Examples and publication

Canonical examples are E0 replays with `setup()` acquire copied fixtures,
configure, start and
`loop()` observe/decide/actuate. They use named volatile result cells, not
Serial, as intent evidence. Lesson 058 exposes digit phase, ordered blanking,
logical masks, refresh loss, and shutdown. Lesson 059 exposes register/row
commands, chip-select trace, partial prefix, fault, and blank request. Lesson
060 exposes stopwatch snapshot, both source generations/digests, self-test,
agreement, and fault owner.

HTML is the terse normative reference. PDFs are complementary prediction,
experiment, diagnosis, exercise, and acceptance workbooks. Every PDF visual
is pencil-drawn unless a later E1 artifact is an explicitly classified,
electrically authoritative formal schematic. E0 includes no schematic.

Promotion requires the pre- and post-implementation stress passes, complete
host/sanitizer/header tests, Mega builds and exact size evidence, package
checks, independently reviewed HTML/PDF/site artifacts, clean Quality and
Pages workflows, and live landing/lesson/PDF/sketch verification. Physical
cards remain visibly open.

This plan is governed by the [curriculum](../CURRICULUM.md),
[development](../DEVELOPMENT.md), [testing](../TESTING.md),
[safety](../SAFETY_MODEL.md), [packaging](../PACKAGING.md), and
[PDF](../PDF_POLICY.md) contracts, the authoritative
[work queue](../WORK_QUEUE.md), and the original
[cadence](../projects/component_project_cadence.md). The pre-implementation reviews are the
[Lesson 058](LESSON_058_MULTIPLEXED_DIGITS_STRESS_PASS.md),
[Lesson 059](LESSON_059_MAX7219_PRESENTATION_STRESS_PASS.md), and
[Lesson 060](LESSON_060_TIMING_DESK_STRESS_PASS.md) stress passes.
