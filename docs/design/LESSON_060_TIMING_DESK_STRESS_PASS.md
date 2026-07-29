# Lesson 060 timing-desk architecture stress pass

Status: pre-implementation review.

This pass reviews the
[Lessons 058--060 implementation plan](LESSONS_058_060_DISPLAY_TIMING_DESK_PLAN.md).

`DualDisplayTimingDesk` is a natural project composition when it owns one
deterministic stopwatch snapshot and two pure presentation policies. It is not
a distributed transaction manager and cannot make two physical displays
change atomically.

| Pressure | Disposition |
|---|---|
| Ownership | Own the child policies and copied evidence; retain no buttons, endpoints, callbacks, bus, clock, or caller pointers. |
| Controls/time | Reset dominates; simultaneous non-reset presses are ambiguous; held edges do not repeat. Supplied time defines pause/lap/capacity/wrap behavior. |
| Atomicity | Freeze one stopwatch snapshot, preflight both values, then commit two source-generation-bound intents. Atomicity is semantic only. |
| Disagreement | Compare exact current-generation digests and receipts with a bounded pending grace. Preserve stopwatch history, request both blank, and attribute the failed side. |
| Recovery | Fault recovery requires reset and the complete distinct two-display self-test; stale acknowledgements cannot clear it. |
| Resources | E0 owns none. Initial target/hard gates are 24/32 KiB flash, 2,048/3,072 B SRAM, 640/896 B stack, and 640/896 B full object composition. |
| Diagnostic interference | Named memory result cells and optional Serial cannot change time, precedence, refresh, transport intent, or agreement. |
| Physical pressure | Optical agreement, simultaneous current, waveform service, transport delivery, and blanking remain E1c evidence after both independent fixtures pass. |

The maximum collision trace combines a reset/start/lap envelope, counter wrap,
digit refresh loss, a partial MAX row failure, crossed acknowledgements,
pending-grace expiry, self-test restart, and shutdown. Structural invalidity
rejects before semantic precedence. Reset dominates valid controls. A display
fault preserves elapsed history while publishing attributable blank requests.
No combination may claim optical agreement or physical blanking.

Promotion requires all eight control masks in every stopwatch phase, exact
capacity and time boundaries, every self-test stage/failure, both one-sided
and simultaneous faults, stale/wrong digest/generation receipts, deterministic
replay, aggregate resource evidence, and a terminal architecture review.

The exact no-LTO call graph measures 771 bytes of synchronous stack. This
exceeds the 640-byte target but remains 125 bytes below the 896-byte hard limit
after reducing the initial 1,022-byte hard failure. The remaining depth is
accepted for the bounded coordinator-to-child preview path: it retains
semantic frame preflight and atomic two-child admission without heap storage,
borrowed child lifetimes, or duplicated presentation policy.

Resource-review: lesson=060 metric=synchronous_stack observed=771 target=640 hard=896 disposition=accepted-target-miss
