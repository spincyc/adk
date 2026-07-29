# Lesson 059 MAX7219 presentation architecture stress pass

Status: pre-implementation review.

This pass reviews the
[Lessons 058--060 implementation plan](LESSONS_058_060_DISPLAY_TIMING_DESK_PLAN.md).

`Max7219PresentationPolicy` fits as fixed register/frame policy with a
recording transport seam. It must not claim physical rollback, readback, or
blanking from a write-only device.

| Pressure | Disposition |
|---|---|
| Layering/API | Frame generation remains pure and independent. A later electrical adapter reuses `SpiDevice`; no second bus abstraction is authorized. |
| Lifecycle | Configuration starts dark, clears test/decode/rows, and enables last. Restart fully reconfigures; cached state is not display evidence. |
| Partial writes | Desired, last-fully-submitted, partial-prefix, blank-requested, cleanup-pending/accepted, and physically-indeterminate states remain distinct. Cleanup is a later one-command service state, never a second command in the failing call. |
| Errors | Preserve first failing operation/register/row/status and cleanup status. Cleanup is bounded and never overwrites primary provenance. |
| SPI recovery | Existing terminal-driver-fault versus `SpiBus::initialized()` behavior is a cross-cutting blocker for the future adapter; resolve centrally before E1 promotion. |
| Resources | E0 claims none. Initial target/hard gates are 16/20 KiB flash, 1,024/1,536 B SRAM, 384/512 B stack, and 192/256 B policy object. |
| Composition | Lesson 060 compares generation-bound copied command receipts, not pixels. One outstanding command and one command per service call keep multiplex service bounded. |
| E1 strain | Unknown module revision, RSET, decoupling, orientation, current, and MAX7219 timing keep the fixture unpowered. Display-test is excluded from routine self-test. |

The maximum failure trace injects a fault at both byte positions of each
configuration and row register, then faults the bounded shutdown attempt. The
policy must never advance the submitted generation, must report the exact
partial prefix and both statuses after the later cleanup service, must leave
chip-select inactive in the recording trace, and must label physical
presentation indeterminate.

Promotion requires golden register order, every pixel/orientation transform,
recording-seam framing isolation, lifecycle and restart traces, byte-identical replay,
resource evidence, and a post-implementation review. E1 additionally requires
exact specimen and current qualification plus a repository-wide SPI recovery
decision.
