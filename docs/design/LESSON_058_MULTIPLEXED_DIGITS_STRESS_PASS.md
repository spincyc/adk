# Lesson 058 multiplexed-digits architecture stress pass

Status: pre-implementation review and clean post-implementation reassessment.

This pass reviews the
[Lessons 058--060 implementation plan](LESSONS_058_060_DISPLAY_TIMING_DESK_PLAN.md).

`MultiplexedDigitPolicy` is a natural component-layer E0 policy only after
separating logical scheduling from the earlier cadence's physical endpoint
claim. It owns fixed copied state and supplied-time ordering, not GPIO or a
clock.

| Pressure | Disposition |
|---|---|
| Layering/API | Natural with logical three-stage transactions. A final mask alone would hide ghost-prevention order. |
| Prior interfaces | Requires one canonical pure Lesson 010 glyph encoder; silent table duplication is rejected. Segment and digit-select polarity remain independent. |
| Lifecycle | Inert construction, blank initialize/reset/shutdown, owner/lifecycle-bound previews, one explicit immediate first-service exception, atomic cycle-boundary frame swap, no borrowed state. |
| Timing | One phase per due call, no catch-up, actual-time reanchor, explicit equal/wrap/regression/half-range/loss behavior. |
| Errors | Structural rejection is atomic; overflow is a visible semantic value; refresh loss latches blank intent. |
| Resources | E0 claims none. Initial target/hard gates are 12/16 KiB flash, 768/1,024 B static SRAM, 320/448 B stack, and 192/256 B object. |
| Composition | Lesson 060 must service refresh without hidden time or blocking work and bind every intent to its immutable source snapshot. |
| E1 strain | Twelve direct signals do not establish a safe circuit. Exact transistor topology, one resistor per segment, port/current limits, polarity, ghosting, and measured blanking remain E1a blockers. |

The maximum trace installs a new frame during digit three, crosses the time
counter wrap, exercises all four polarity combinations, arrives exactly at
the refresh deadline and loss boundary, then one tick late. It must expose
ordered blank/segment/select stages, never select two digits, never burst,
swap only at digit zero, and remain blank after loss until explicit recovery.

The implemented boundary passes exhaustive glyph/decimal/leading-zero cases,
all four polarity pairs, frame boundaries, invalid configuration, first-call
and ordinary timing/loss edges, wrap and exact-half-range rejection,
transactional preview ownership, terminal lifecycle exhaustion, deterministic
replay, and independent code/example/publication review. After Lesson 060's
bounded diagnostic-glyph extension, the canonical sketch measures 6,540/781
bytes flash/static SRAM. The exact no-LTO gate measures 6,254 bytes flash, 781
bytes static SRAM, 196 bytes synchronous stack, and a 67-byte object, leaving
7,087 bytes of residual SRAM. The 781-byte static-SRAM result is a reviewed
miss against the 768-byte target and remains below the 1,024-byte hard limit;
every other target and every hard gate passes.

The terminal Lesson 060 composition reassessment found that the diagnostic
extension remains bounded and natural; it does not expose a raw segment mask
or change the transactional ownership model. Exact powered-display topology,
current, waveform, optical behavior, resource ownership, and observed
blanking remain E1-open.

The bounded diagnostic-glyph extension raises exact static SRAM to 781 bytes.
That exceeds the 768-byte target by 13 bytes but remains 243 bytes below the
1,024-byte hard limit. The increase is accepted because it closes the fixed
Lesson 060 self-test without exposing raw segment masks, allocation, another
display policy, or a hardware adapter.

Resource-review: lesson=058 metric=static_sram observed=781 target=768 hard=1024 disposition=accepted-target-miss
