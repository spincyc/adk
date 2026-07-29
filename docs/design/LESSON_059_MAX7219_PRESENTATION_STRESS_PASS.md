# Lesson 059 MAX7219 presentation architecture stress pass

Status: post-implementation assessment; host verified, E1 fixture open.

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
| Resources | E0 claims none. The canonical Mega replay measures 5,480 B flash and 640 B static SRAM. The exact no-LTO gate measures 6,208 B flash, 640 B static SRAM, 210 B synchronous stack, and a 108 B policy object, leaving 7,214 B residual SRAM. Every target and hard gate passes. |
| Composition | Lesson 060 compares generation-bound copied command receipts, not pixels. One outstanding command and one command per service call keep multiplex service bounded. |
| E1 strain | Unknown module revision, RSET, decoupling, orientation, current, and MAX7219 timing keep the fixture unpowered. Display-test is excluded from routine self-test. |

The maximum failure trace injects a fault at both byte positions of each
configuration and row register, then faults the bounded shutdown attempt. The
policy must never advance the submitted generation, must report the exact
partial prefix and both statuses after the later cleanup service, must leave
chip-select inactive in the recording trace, and must label physical
presentation indeterminate.

The promoted implementation supplies golden register order, every
pixel/orientation transform, recording-seam framing isolation, lifecycle and
restart traces, byte-identical replay, resource evidence, and this
post-implementation assessment. The architecture did not buckle under the
component: the implementation retained the planned pure E0 policy boundary,
one-command service cadence, copied receipts, bounded cleanup, and zero
resource ownership without changing prior display or SPI contracts. E1 still
requires exact specimen and current qualification plus a repository-wide SPI
recovery decision.
