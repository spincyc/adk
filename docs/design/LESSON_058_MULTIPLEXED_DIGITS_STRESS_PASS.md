# Lesson 058 multiplexed-digits architecture stress pass

Status: pre-implementation review.

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

Promotion additionally requires exhaustive glyph/decimal/leading-zero cases,
all frame boundaries, every invalid configuration, deterministic replay,
resource measurement, and a second stress pass against the implemented
Lesson 060 composition.
