# Work queue

This is ADK's authoritative delivery and deferred-work ledger. It records what
must not be dropped; it does not promote an interface. Lesson numbers and
subjects come from [the curriculum](CURRICULUM.md), architecture and acceptance
come from [the development contract](DEVELOPMENT.md), and detailed project
briefs come from [the project catalog](PROJECTS.md) and the
[extended cadence](projects/component_project_cadence.md).

## Restart checkpoint

Last reconciled: 2026-07-29 on `main`.

The repository-wide recovery audit and ordered recovery outcomes are
recorded in
[the work-queue recovery audit](audits/REPOSITORY_WORK_QUEUE_RECOVERY_AUDIT.md).
Its findings must be reconciled through bounded tasks; the audit itself does
not promote work, replace this product ledger, or serve as an AIQ queue.

- Lessons 001--072 are promoted and host verified; physical cards remain open.
- Lesson 030 has an independently reviewed composition core, canonical Mega
  example, measured size baseline, HTML reference, monochrome PDF lesson,
  downloads, and navigation. Its E1 physical acceptance card remains open.
- Lessons 034--039 are complete through their non-hardware gates. Lessons
  037--039 use documented external reference fixtures.
  Their [sensor evidence report](research/LESSONS_037_039_SENSOR_EVIDENCE_REPORT.md)
  preserves all six Elegoo exact-specimen requirements as historical optional
  substitution-conformance work. Those requirements were superseded as
  blockers for canonical reference publication, not answered. Incoming
  conformance and all E1 bench gates remain open.
- The USB research track has a deterministic product-native `Cau`/`Pau`
  `ColdMove` model. It performs no USB action and makes no transparency claim.
- The exact kit inventory template and honest planned-versus-supported coverage
  language are committed. Physical inventory records remain open.

Do not lose this release blocker:

1. run the full clean quality gate, release, push, and verify GitHub Pages only
   after the promoted lesson boundary is clean.

The canonical post-030 order remains lessons 031--081, one implementation-depth
three-lesson brief at a time. HDMI and USB product work remain research tracks;
physical endpoints, interoperability, performance, compliance, and shared-LAN
qualification are not yet supported.

## Status meanings

| State | Meaning |
|---|---|
| Host verified; bench open | First-class code, deterministic tests, Mega build, size evidence, HTML, and PDF exist; measured hardware acceptance is absent |
| Active integration | Files may exist, but the dependency-ordered commit and all non-hardware gates are not yet complete |
| Queued | Scope and order are fixed; implementation is not a support claim |
| Research | Architecture or executable host models only; no product or physical-performance claim |
| Deferred | Deliberately retained after a named prerequisite; not abandoned |

## Lesson delivery

The repository-wide
[lesson PDF visual-style audit](audits/LESSON_PDF_VISUAL_STYLE_AUDIT.md) is
complete for Lessons 001--033. Every non-schematic visual uses pencil-drawing
presentation; formal electrical schematics are the sole exception. The
canonical PDF policy and automated classification gate apply the same rule to
every future lesson.

The [lesson progression and learner-flow audit](audits/LESSON_PROGRESSION_AUDIT.md)
is complete for Lessons 001--033. Learner artifacts use dependent visible
stages and keep repository administration, command-line maintenance, and
physical-acceptance paperwork outside the lesson flow.

The [sensor engagement-order audit](audits/SENSOR_ENGAGEMENT_REORDER_AUDIT.md)
front-loads immediately legible, interactive outcomes while retaining
dependency, specimen, safety, and verification gates. It is a curriculum
ordering decision, not evidence that any queued sensor is electrically
qualified.

| Boundary | State | Required next result |
|---|---|---|
| [001--036](CURRICULUM.md#cadence) | Host verified; bench open | Preserve interfaces and complete each physical acceptance record |
| [037--039](design/LESSONS_037_039_PERCUSSION_PLAN.md) | Host verified; bench/incoming conformance open | Preserve the canonical reference publication; complete incoming conformance and each E1 acceptance record |
| [040--042](design/LESSONS_040_042_OPTICAL_COURSE_MARSHAL_PLAN.md) | Host verified; powered adapter/bench open | Preserve the pure-policy publication; qualify exact specimens before powered adapters, wiring, schematics, or E1 acceptance |
| [043--045](design/LESSONS_043_045_BALANCE_TABLE_PLAN.md) | Host verified; powered adapter/bench open | Preserve the E0 synthetic-replay publication; qualify exact MPU/QMI specimens independently before powered adapters, wiring, schematics, or E1 acceptance |
| [046--048](design/LESSONS_046_048_KINETIC_SCULPTURE_PLAN.md) | Host verified; powered input/motion bench open | Preserve the E0 copied-evidence and logical-intent publication; qualify exact inputs at E1 and the exact restrained stepper system independently at E2 |
| [049--051](design/LESSONS_049_051_PARTS_CAROUSEL_PLAN.md) | Host verified; powered endpoints/media/motion bench open | Preserve the E0 synthetic identity, bounded homing, and inert carousel policies, including attributable `Stopped` reconciliation after an exposed start and the intentional zero-coil atomic-step result; the Lesson 051 object is 380 B (320 B target miss, 384 B hard pass); retain exact powered endpoints/media at E1 and restrained motion at E2 as separate open gates |
| [052--054](design/LESSONS_052_054_IR_TRANSLATOR_PLAN.md) | Host verified; exact powered fixtures/bench open | Preserve copied receive provenance, the immutable local catalog, and fixed different-symbol translation as inert E0 policy; retain exact receiver/emitter fixtures, electrical and optical endpoints, observation paths, and measured acceptance as open E1 gates |
| [055--057](design/LESSONS_055_057_ESCAPE_CONSOLE_PLAN.md) | Host verified; exact passive fixtures/restrained demonstration bench open | Preserve the fixed clue graph, copied audit-image recovery, stop precedence, atomic solve transaction, and inert presentation/release intent; retain passive input/presentation qualification at E1 and any restrained no-load demonstration fixture at E2 |
| [058--060](design/LESSONS_058_060_DISPLAY_TIMING_DESK_PLAN.md) | Host verified; exact powered display fixtures open | Preserve supplied-time digit transactions, bounded MAX7219 command/receipt evidence, and the timing-desk composition, including explicit refresh loss, partial-prefix attribution, cleanup, generation binding, self-test, disagreement attribution, and the zero-hardware E0 boundary |
| [061--063](design/LESSONS_061_063_MUSEUM_CASE_MONITOR_PLAN.md) | Host verified; E1a--E1d/E2 open | Preserve the published copied-evidence policies and inert museum monitor; exact powered specimens, persistence, presentation, and relay work remain separately gated |
| [064--066](design/LESSONS_064_066_THERMAL_MAPPER_PLAN.md) | Host verified; E1a--E1d open | Preserve the published E0 [Lesson 064 transaction](design/LESSON_064_ONE_WIRE_TRANSACTION_STRESS_PASS.md), [Lesson 065 probe set](design/LESSON_065_QUALIFIED_PROBE_SET_STRESS_PASS.md), and [Lesson 066 mapper](design/LESSON_066_THERMAL_GRADIENT_MAPPER_STRESS_PASS.md); retain exact specimens, powered single-wire behavior, thermal accuracy, presentation, persistence, authentication, and E1a--E1d acceptance as open gates |
| [067--069](design/LESSONS_067_069_MOTION_RECORDER_PLAN.md) | Published; host verified; powered acceptance open | Preserve Lesson 067's copied source-frame record and fixed 64-byte image, Lesson 068's one-source qualifier, and Lesson 069's one-source-per-session volatile recorder and presentation intent; Lesson 068's ordinary flash miss and Lesson 069's exact no-LTO flash miss are independently reviewed below their hard limits; exact MPU6050/QMI8658 acquisition, powered presentation, RTC/media persistence, and bench acceptance remain separately gated E1a--E1c work |
| [070--072](design/LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md) | Host verified and published; powered acceptance open | Preserve the published E0 [Lesson 070 descriptor](design/LESSON_070_THRESHOLD_MODULE_DESCRIPTOR_STRESS_PASS.md), [071 characterization](design/LESSON_071_THRESHOLD_CHARACTERIZATION_STRESS_PASS.md), and [072 inert bench](design/LESSON_072_INERT_MODULE_CHARACTERIZATION_BENCH_STRESS_PASS.md); exact specimens, powered acquisition, presentation, and bench acceptance remain E1-open, while persistence remains outside this arc |
| [073--075](design/LESSONS_073_075_RTC_INTEGRITY_RESCOPE.md) | Re-scope decision complete; implementation not started | Produce the mandatory implementation-depth plan and initial architecture stress passes for **Copied RTC Transaction Evidence**, **Qualified Clock Observation**, and the **Inert Time-Warp Detective Desk**, and make the affected `Rtc` seam decision before any code; retain copied E0 DS1307-family scope, keep DS3231 separately gated, and leave powered acquisition, clock accuracy, display, persistence, and bench acceptance open |
| [076--078](design/LESSONS_076_078_TABLETOP_SONAR_RESCOPE.md) | Re-scope decision complete; implementation not started | Produce the mandatory implementation-depth plan and initial architecture stress passes for **Copied Sweep-Range Frames**, **Bounded Polar Occupancy Map**, and the **Inert Tabletop Sonar Desk** before any code; preserve copied E0 angle/range evidence, keep exact ultrasonic acquisition and restrained servo motion separately gated at E1 and E2, and leave physical ranging, mapping accuracy, powered presentation, and bench acceptance open |
| [079--081](design/LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md) | Lessons 079--080 host verified and published; Lesson 081 queued | Preserve the published E0 `BoundedLowSideDriverPolicy` boundary and `SmallIndicatorSemanticsPolicy` copied-evidence boundary; implement the inert qualification bench next, while E1 identity and E2 powered fixture acceptance remain open |

Integration order is strict: component or endpoint, deterministic tests,
example and size evidence, lesson package, shared indexes, then the consuming
project. Open physical cards do not pause later host work. A failed software,
safety, packaging, or publication gate does.

Every component, endpoint, and composing project also requires the
[architecture stress pass](templates/component-design-stress-pass.md) before
its public shape is fixed and again before promotion. Architectural strain
cannot be hidden inside a lesson boundary: record affected prior decisions and
downstream consumers, discuss materially broad remediation, and make a durable
decision before changing shared contracts. The first recorded pass identifies
[canonical-publication coupling](design/CANONICAL_PUBLICATION_COUPLING_STRESS_PASS.md);
its publication-authority/resolver choice remains a separate decision and must
not be folded opportunistically into Lessons 037--039 publication.

Deferred but retained: physical RTC and removable-media adapters require exact
specimen selection, primary datasheets, electrical qualification, and their
own failure and bench evidence. Lesson 022 supports deterministic RTC state and
fixed-storage durability models, not physical RTC or SD hardware. Lesson 017's
configuration codec has exhaustive every-byte corruption, erased, oversized,
generation-wrap, and deterministic replay coverage.

## Kit expansion

Lessons 031--081 are the canonical append-only kit expansion. Their
[listing-authorized family scope](inventory/AUTHORIZED_ELEGOO_SET.md) is the
deduplicated union of the cited official Elegoo Mega Most Complete and
Upgraded 37-in-1 manifests. Exact revisions, module markings, voltage limits,
and primary sources remain prerequisites to electrical claims and powered
work; retail kit names are not electrical identities.

| Lessons | State | Block |
|---:|---|---|
| [031--033](design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md) | Host verified; bench open | Analog joystick, quadrature encoder, calibration console |
| [034--036](design/LESSONS_034_036_MAGNETIC_PASSAGE_PLAN.md) | Host verified; bench open | Magnetic observations, passage qualification, magnetic passage logger |
| [037--039](design/LESSONS_037_039_PERCUSSION_PLAN.md) | Host verified; bench/incoming conformance open | Contact dynamics, acoustic envelopes, and percussion sequencer published against documented external references; E1 and incoming conformance open |
| [040--042](design/LESSONS_040_042_OPTICAL_COURSE_MARSHAL_PLAN.md) | Host verified; powered adapter/bench open | Optical observations, presence, and tabletop course marshal are published as pure policy; explicit button authorization is fixed, while powered exact-specimen and bench gates remain open |
| [043--045](design/LESSONS_043_045_BALANCE_TABLE_PLAN.md) | Host verified; powered adapter/bench open | Copied inertial samples, pure orientation/presentation intent, and stationary hand-operated tabletop balance instrument are published as E0 replay; no powered adapter, I2C, wiring, schematic, or E1 claim |
| [046--048](design/LESSONS_046_048_KINETIC_SCULPTURE_PLAN.md) | Host verified; powered input/motion bench open | `InteractionIntentPolicy`, `BoundedStepperSequence`, and `KineticLightSculpture` publish copied tactile/directional evidence, bounded logical coil intent, transactional authorization, independent stop precedence, and semantic light intent at E0; powered inputs remain E1-gated and energized motion remains E2-gated |
| [049--051](design/LESSONS_049_051_PARTS_CAROUSEL_PLAN.md) | Host verified; powered endpoints/media/motion bench open | `LocalIdentityRegistry`, `BoundedHomingPolicy`, and `InertPartsCarousel` publish copied evidence, bounded logical home/position, acknowledged record-image reconciliation, atomic one-step application, and intentional zero-coil intent at E0; E1 endpoints/media and E2 motion remain open |
| [052--054](design/LESSONS_052_054_IR_TRANSLATOR_PLAN.md) | Host verified; exact powered fixtures/bench open | `CapturedIrEvidence`, `KnownIrEmissionPolicy`, and `InertIrTranslator` publish copied Lesson 025 receive evidence, immutable locally authored catalog intent, and fixed different-symbol translation at E0; exact receiver/emitter fixtures and E1 acceptance remain open |
| [055--057](design/LESSONS_055_057_ESCAPE_CONSOLE_PLAN.md) | Host verified; exact passive fixtures/restrained demonstration bench open | Fixed clue-constraint model, copied-value fault-aware panel, and inert escape-room console |
| [058--060](design/LESSONS_058_060_DISPLAY_TIMING_DESK_PLAN.md) | Host verified; exact powered display fixtures open | Multiplexed digits, MAX7219 presentation, and the dual-display timing desk are published at E0; exact powered endpoints and physical acceptance remain separate E1 gates |
| [061--063](design/LESSONS_061_063_MUSEUM_CASE_MONITOR_PLAN.md) | Host verified; E1a--E1d/E2 open | Copied resistive, thermal/radiant, reed, acknowledgement, and receipt evidence compose into an inert monitor; powered specimens, persistence, presentation, and relay work remain separately gated |
| [064--066](design/LESSONS_064_066_THERMAL_MAPPER_PLAN.md) | Host verified; E1a--E1d open | `OneWireTransactionPolicy`, `Qualified18B20ProbeSetPolicy`, and `ThermalGradientMapper` are published at E0 with copied requests, receipts, fixed identities, conversion correlation, CRC, freshness, disappearance, spatial intervals, fault incidence, bounded pages, and volatile record intent; no powered adapter, wiring, thermal-accuracy, presentation, persistence, authentication, or E1 support claim |
| [067--069](design/LESSONS_067_069_MOTION_RECORDER_PLAN.md) | Published; host verified; powered acceptance open | Copied source-frame normalization, one-source qualification, and one-source-per-session volatile motion-recorder intent are host verified; Lesson 068 measures 16,702 B ordinary flash against a 16 KiB target and 24 KiB hard limit; Lesson 069's exact no-LTO flash target miss is independently reviewed below its hard limit; exact MPU/QMI acquisition, powered presentation, RTC/media persistence, and bench acceptance remain E1a--E1c open |
| [070--072](design/LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md) | Host verified and published; powered acceptance open | `ModuleThresholdDescriptor` and `ModuleThresholdFrame` publish copied declaration and provenance validation; `ModuleCharacterizationPolicy` publishes bounded three-leg evidence; `InertModuleCharacterizationBench` publishes one-envelope review and a canonical 192-byte volatile record at E0; exact specimens, powered acquisition, presentation, and bench acceptance remain E1-open |
| [073--075](design/LESSONS_073_075_RTC_INTEGRITY_RESCOPE.md) | Re-scope decision complete; detailed planning queued | **Copied RTC Transaction Evidence**, **Qualified Clock Observation**, and the **Inert Time-Warp Detective Desk** over copied DS1307-family values; decide the existing `Rtc` seam in the implementation-depth plan and architecture stress passes before code, while DS3231 and every powered or physical claim remain separately gated |
| [076--078](design/LESSONS_076_078_TABLETOP_SONAR_RESCOPE.md) | Re-scope decision complete; detailed planning queued | **Copied Sweep-Range Frames**, **Bounded Polar Occupancy Map**, and the **Inert Tabletop Sonar Desk** over copied angle/range evidence; require the implementation-depth plan and architecture stress passes before code, while exact ultrasonic acquisition remains E1-gated and restrained servo motion remains E2-gated |
| [079--081](design/LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md) | Lessons 079--080 host verified and published; Lesson 081 implemented but promotion blocked | `BoundedLowSideDriverPolicy` and `SmallIndicatorSemanticsPolicy` are published at E0. The Lesson 081 E0 core is implemented and host verified on a branch, but promotion is blocked by a [terminal resource strain](design/LESSON_081_TERMINAL_RESOURCE_STRAIN.md): the bench measures 2,919 B against a 1,024 B hard limit because `prepareRecord` cannot receive the five-entry replay its encoder requires. An authoring decision is needed before promotion |

The input-first 031--033 decision supersedes older use of 032--033 for the
MPU6050 and balance table. The engagement-order audit now retains that work at
043--045 without a number collision and orders the later arcs by learner
payoff rather than inventory taxonomy alone. The
[kit coverage audit](design/ELEGOO_MEGA_KIT_COVERAGE_2026-07-27.md),
[sensor taxonomy](research/SENSOR_KIT_TAXONOMY.md), and
[safety taxonomy](design/KIT_MODULE_SAFETY_TAXONOMY.md) remain implementation
inputs. Coverage is not complete until every claimed module has an exact
inventory record and the relevant block passes its gates.

Lessons 031--054 have complete design briefs and are host verified. Lessons
037--039 are published against exact external reference
fixtures; incoming conformance and E1 physical acceptance remain open. The six
earlier Elegoo exact-specimen requirements are preserved as historical
optional substitution-conformance work and were superseded as canonical
publication blockers rather than answered. Lessons 040--042 publish their
reconciled pure-policy interfaces, deterministic traces, Mega replays, HTML,
and pencil-drawing PDFs. A debounced explicit button action is the sole start
authorization; PIR evidence may establish eligibility but can never start a
run. Powered adapters, wiring, formal schematics, and E1 claims remain gated
by exact-specimen qualification.
Lessons 043--045 publish copied inertial observations, pure board-frame
orientation and presentation intent, and a stationary tabletop balance
instrument through deterministic E0 host replay, compile-only Mega sketches,
measured size baselines, HTML references, and pencil-drawing PDF lessons. The
published boundary reserves exact MPU6050 and QMI8658 acquisition adapters as
independent future E1a gates; no such adapter exists in Lessons 043--045.
Lessons 067--069 consume copied record fixtures at E0 and do not close
exact-specimen, powered-adapter, presentation, persistence, authoritative
schematic, or bench gates. Lessons 046--048 publish the `InteractionIntentPolicy`
`initialize`/`reset`/`preview`/`canCommit`/`commit`/`snapshot` transaction,
the `BoundedStepperSequence`
`initialize`/`reset`/`preview`/`canCommit`/`commit`/`stop`/`snapshot`
transaction, and the composing `KineticLightSculpture`
`initialize`/`shutdown`/`update`/`snapshot` API. The publication includes
deterministic host replay, compile-only Mega sketches, measured size
baselines, HTML references, and pencil-drawing PDF lessons. E0 owns zero
pins, ADC channels, timers, interrupts, buses, resource-registry entries,
endpoints, supplies, or moving hardware. Exact tactile, contact, joystick,
stop, and indicator endpoints, their authoritative schematic, and their
resource and safe-state evidence remain E1-gated. Exact 28BYJ-48 motor and
ULN2003 identities, winding and clamp topology, separate current-limited
supply, coil current and thermal limits, mechanical restraint, independent
stop and power removal, and measured motion acceptance remain E2-gated.
Lessons 049--081 remain canonical subjects in the engagement order above.
Lessons 049--054 are published and therefore precede the later-arc planning
hold. The 049--051
[implementation-depth plan](design/LESSONS_049_051_PARTS_CAROUSEL_PLAN.md)
and the Lesson
[049 identity](design/LESSON_049_IDENTITY_STRESS_PASS.md),
[050 homing](design/LESSON_050_HOMING_STRESS_PASS.md), and
[051 carousel](design/LESSON_051_CAROUSEL_STRESS_PASS.md) stress passes
support the promoted E0 implementation: copied synthetic identity and key
evidence, fixed caller-owned record images, pure bounded homing and logical
position, deterministic reconstruction, acknowledged authorization-record
reconciliation, and inert gate, actuator, and presentation intent. The
Lesson 051 atomic one-step application deliberately publishes zero coil
intent; Lesson 047 remains the nonzero-coil teaching surface. This is not a
nonvolatile-media durability claim. E1 remains separately gated on exact RFID,
keypad, home, stop,
indicator, display, and nonvolatile-media endpoints; their primary evidence,
pin and resource maps, authoritative schematics, rollback, safe-state, and
bench records remain open. E2 remains separately gated on exact
stepper/driver and servo identities, independently switchable current-limited
actuator power, restraint, stop and power removal, and measured homing,
position, gate, current, thermal, and motion acceptance.

Lessons 052--054 are host verified under their clean-reviewed
[implementation-depth plan](design/LESSONS_052_054_IR_TRANSLATOR_PLAN.md).
E0 reuses Lesson 025
without changing its public API, copies attributable receive evidence, admits
transmission intent only from an immutable firmware-authored catalog, and
translates a fixed allowlist of valid receive symbols to different local
symbols. Repeat, unknown, malformed, self-echo, and arbitrary captured
evidence can never become transmit authority. E0 owns no receiver, emitter,
pin, timer, interrupt, carrier output, or optical power path. Exact receiver,
emitter, driver, resistor, timer/channel/pin allocation, supply, observation
path, and bench acceptance remain open E1 gates. Canonical Mega replays measure
5,530/1,096, 4,854/276, and 16,162/1,343 bytes of flash/static SRAM. The
maximum composition measures 21,864/3,531 bytes; the Lesson 054 object is
407 B with a caller-owned 400 B pulse buffer, and the conservative stack
estimate is 888 B with 3,773 B remaining after static storage and stack.
These measurements do not establish powered, optical, or physical behavior.
The authorized Elegoo
inventory does not establish that the kit contains an IR emitter, so this arc
makes no known-kit-emitter claim.

Lessons 055--057 are host verified under their independently reviewed
implementation-depth plan and terminal architecture stress passes. Their E0
publication provides a fixed clue graph, copied attributable evidence,
caller-supplied two-slot audit images, stop-dominant panel policy, and one
atomic inert escape-console transaction. Canonical Mega replays measure
8,596/1,261, 16,028/1,454, and 32,332/3,655 bytes of flash/static SRAM.
The exact no-LTO resource gate records 412/636, 569/365, and 951/1,024 bytes
of synchronous stack/object storage for Lessons 055--057 respectively, with
reviewed target misses and every hard/residual gate passing. Exact passive
input/presentation fixtures remain E1-open; restrained no-load servo or inert
current-limited relay/lamp demonstration work remains E2-open and may never
control access, confinement, egress, or safety.

Lesson 058 publishes `MultiplexedDigitPolicy` as E0 copied presentation
policy. It owns no hardware and emits bounded three-stage
blank/select/segment intent from supplied time. Its canonical Mega replay is
6,540/781 bytes flash/static SRAM after Lesson 060's bounded diagnostic-glyph
extension. The exact no-LTO gate measures 6,254 bytes flash, 781 bytes static
SRAM, 196 bytes synchronous stack, and a 67-byte object, leaving 7,087 bytes
of residual SRAM. Static SRAM is a reviewed miss against the 768-byte target
and remains below the 1,024-byte hard limit; every other target and every hard
gate passes. The exact powered digit fixture, driver topology, current,
waveform, optical behavior, and observed blanking remain E1-open. Lesson 059
publishes `Max7219PresentationPolicy` as E0 copied
register-command and receipt policy. Its canonical Mega replay measures
5,480/640 bytes flash/static SRAM. The exact no-LTO gate measures 6,208 bytes
flash, 640 bytes static SRAM, 210 bytes synchronous stack, and a 108-byte
object, leaving 7,214 bytes of residual SRAM; every target and hard gate
passes. Exact MAX7219 identity, current, orientation, transport, optical
behavior, and bench acceptance remain E1-open. Lesson 060 publishes
`DualDisplayTimingDesk` as an E0 composition over copied controls, supplied
time, and explicit child receipts. Its canonical Mega replay and exact
no-LTO resource gate pass; the latter measures 20,250 bytes flash, 1,366
bytes static SRAM, 771 bytes synchronous stack, and a 561-byte object, leaving
5,927 bytes of residual SRAM. The stack target miss is independently reviewed
below its hard limit. Exact display identities, electrical transports,
current, waveform and optical behavior, self-test observation, and bench
acceptance remain E1-open.

Lesson 061 publishes `ResistiveProbeObservationPolicy` as E0 copied
acquisition policy. Calibration slope, excitation-off evidence, per-cycle
duty, sequence ordering, freshness, provenance, and quality precedence remain
explicit, while rejected samples leave state unchanged. Its canonical Mega
replay measures 5,662/624 bytes flash/static SRAM. The isolated no-LTO gate
measures 3,566 bytes flash, 169 bytes static SRAM, 123 bytes conservative
synchronous stack, and a 69-byte object; caller-owned input/output buffers
measure 27+37 bytes and residual SRAM is 7,772 bytes after the specified ISR
reserve. Exact probe identity, adapter, wiring, excitation timing, discharge,
corrosion behavior, liquid response, non-Serial observation, and physical
safe state remain E1a-open. Lesson 062 publishes
`ThermalRadiantObservationPolicy` over three independent copied roles.
Thermistor uncertainty, categorical Digital Temperature and radiant evidence,
source-specific ordering and ages, disagreement, pulse/sustained timing, and
saturation remain explicit. Its canonical Mega replay measures 10,076/1,133
bytes of flash/static SRAM; the reviewed static-SRAM target miss remains below
the 1,536-byte hard limit. The exact no-LTO gate measures 5,156 bytes flash,
279 bytes static SRAM, 264 bytes synchronous stack, and a 112-byte object,
leaving 7,521 bytes residual SRAM. Exact powered modules, adapters, response,
stimulus, and physical safe state remain E1b-open. Lesson 063 publishes
`MuseumCaseMonitor` as an E0 copied-evidence composition with additive hazards,
latched alarm recovery, inert output intent, and one bounded audit transaction.
Its ordinary Mega replay measures 21,348/1,405 bytes flash/static SRAM; exact
no-LTO evidence measures 22,980 flash, 1,405 static, 819 stack, and 709 bytes of
aggregate objects, leaving 5,840 bytes residual SRAM. The stack target miss is
independently reviewed below its hard limit. E1c/E1d powered
presentation/combined sensing and E2 relay-lamp acceptance remain open.

Lessons 058--066 are host verified. Lesson 064 publishes the E0
`OneWireTransactionPolicy` under the
[implementation-depth plan](design/LESSONS_064_066_THERMAL_MAPPER_PLAN.md)
and
[bounded-transaction stress pass](design/LESSON_064_ONE_WIRE_TRANSACTION_STRESS_PASS.md).
Lesson 065 publishes the E0
`Qualified18B20ProbeSetPolicy` under the
[qualified probe-set stress pass](design/LESSON_065_QUALIFIED_PROBE_SET_STRESS_PASS.md).
Its canonical replay measures 13,662 bytes flash and 1,438 bytes static SRAM;
the exact no-LTO replay measures 16,196 bytes flash and 1,438 bytes static
SRAM, with 533 bytes conservative synchronous stack, a 764-byte policy, a
477-byte caller-owned builder, and a 180-byte snapshot. Lesson 066 publishes
`ThermalGradientMapper` under the
[thermal-mapper stress pass](design/LESSON_066_THERMAL_GRADIENT_MAPPER_STRESS_PASS.md).
Its canonical replay measures 16,662 bytes flash and 2,210 bytes static SRAM;
exact no-LTO evidence measures 18,822/2,210 bytes with 855 bytes conservative
synchronous stack, a 448-byte mapper, 202-byte envelope, 377-byte result,
229-byte record, 1,943-byte recurring composition, 579-byte phase storage,
2,522-byte lifetime storage, and 4,999 bytes residual SRAM. Six target misses
were independently reviewed below their hard limits. The mapper validates
copied structure rather than authenticating a source and emits volatile intent
without driving a display or writing storage.
Exact powered specimens, electrical single-wire behavior,
thermal accuracy, presentation, persistence, and E1a--E1d acceptance remain
open.

Lessons 067--069 are host verified under their
[motion-recorder implementation-depth plan](design/LESSONS_067_069_MOTION_RECORDER_PLAN.md).
Lesson 068's ordinary Mega replay measures 16,702 bytes flash and 766 bytes
static SRAM. The flash result is a reviewed miss against the 16 KiB target and
passes the independently reviewed 24 KiB hard limit; static SRAM remains below
its 1,024-byte limit. Lesson 069's honest ordinary Mega composition measures
39,428 bytes flash and 2,347 bytes static SRAM. Its exact no-LTO composition
measures 35,144 bytes flash, 2,347 bytes static SRAM, 861 bytes stack, a
509-byte object, a 128-byte record image, and 4,856 bytes residual SRAM. Exact
flash, static SRAM, and stack miss their targets but pass independently
reviewed fingerprint-bound hard gates of 40 KiB, 3,072 bytes, and 1,024 bytes.
These E0 results establish
deterministic copied-record policy only. Exact
MPU6050/QMI8658 acquisition, powered presentation, RTC/media persistence, and
bench acceptance remain open E1a--E1c work.

Lessons 070--072 have an independently reviewed
[implementation-depth plan](design/LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md)
and architecture stress passes. Lesson 070 publishes
`ModuleThresholdDescriptor`, `ModuleThresholdFrame`, stateless structural
validation, and explicit comparator-assertion meaning over copied E0 values.
Its canonical Mega replay measures 4,564 bytes flash and 684 bytes static
SRAM. It owns no module, endpoint, rail, clock, transport, display, or storage;
descriptor validity and declaration completeness are not physical-specimen
authority. Lesson 071 publishes bounded ascending, descending, and verification
streams over copied frames. Its ordinary Mega replay measures 10,200 bytes
flash and 1,160 bytes static SRAM. Exact no-LTO evidence measures 11,562 bytes
flash, 1,160 bytes static SRAM, 339 bytes stack, a 498-byte policy, a 375-byte
evidence value, a 57-byte caller-local point, and 6,565 bytes residual SRAM.
Static SRAM and evidence size miss their targets but pass independently
reviewed hard limits for fingerprint
`e1cfc001cc95937e25337eb53a7a4c8b3602d6a046b23b0a8b94fe7a342d562b`;
further evidence ABI growth requires a fresh disposition. Lesson 072 publishes
one atomic terminal envelope, a fixed five-step review, fault-dominant semantic
presentation, and one exact 192-byte caller-owned volatile record. Its ordinary
Mega composition measures 24,860 bytes flash and 1,998 bytes static SRAM under
the repository's canonical Mega build invocation. Exact no-LTO evidence
measures 27,354 bytes flash, 2,002 bytes static SRAM, 740 bytes stack, a
436-byte bench, exactly 192/384 bytes for one/two live record images, and
5,322 bytes residual SRAM. Flash misses its 24,576-byte
target but passes the independently reviewed 32,768-byte hard limit for
fingerprint `b56bd8ef7a12328b80ad613b2a8b41f2cbde8e6fbd5ff0aa408208f50e3b6679`;
all other targets pass. The Lessons 073--075
[re-scope decision](design/LESSONS_073_075_RTC_INTEGRITY_RESCOPE.md) completes
the authorized-family replacement decision, but it is not an
implementation-ready lesson plan. Its mandatory next boundary is an
implementation-depth plan, initial architecture stress passes, and a durable
decision about whether the existing `Rtc` seam can represent malformed
calendar evidence without buckling prior contracts. No Lesson 073 code begins
before that boundary is clean. The Lessons 076--078
[re-scope decision](design/LESSONS_076_078_TABLETOP_SONAR_RESCOPE.md) likewise
selects its subjects without making them implementation-ready. Its mandatory
next boundary is an implementation-depth plan and initial architecture stress
passes for all three lessons; no Lesson 076 code begins before that boundary
is clean. The Lessons 079--081 controlling implementation plan and initial
stress passes are complete. Lessons 079--080 are published at their accepted
E0 boundaries; Lesson 081 is queued in dependency order under that
plan. Their remaining implementation must preserve the declared public
interfaces, resource and pin budgets, deterministic fixture and failure
matrices, narrative example flow, staged circuit-native experiments, HTML/PDF
division, specimen gates, and explicit bench acceptance. “Planned specimen
coverage” never means that an exact kit module is supported.

Lessons 070--072 are scoped to publish copied E0 policy only: descriptor
declarations do not identify hardware, characterization brackets do not invent
exact thresholds, and the fixed 192-byte volatile image is neither physical
acceptance nor durable storage. The PCF8574 LCD backpack, gas-response
experiments, unidentified emitters,
physiological claims, and prototype/power construction variants still require
separately scoped, inventory-gated planning. Lesson 072 covers copied
declared-fixture low-voltage analog/comparator characterization at E0; exact
identified physical specimens remain E1-only. It is not a catch-all claim for
every product sold as a 37-sensor kit.

The source audit found that the historical DS1302, BMP180, PCF8591, and color
sensor subjects came from other vendor taxonomies. They are not authorized
specimens and must not be implemented under the Elegoo-set task. The
Lessons 073--075 decision replaces the first three historical subjects without
quietly authorizing them. The
[Lessons 076--078 decision](design/LESSONS_076_078_TABLETOP_SONAR_RESCOPE.md)
replaces the color-sensor assignment with copied sweep-range evidence, a
bounded polar occupancy map, and an inert tabletop sonar desk. It deepens
already authorized ultrasonic and servo teaching surfaces without claiming a
new sensor family: exact powered ranging remains E1-gated, and restrained
powered motion remains E2-gated. Lesson numbers 073--078 remain stable.

## Deep project re-evaluation (2026-07-30)

The re-evaluation audit is complete and synthesized in
[the deep re-evaluation audit](audits/DEEP_REEVALUATION_2026-07-30.md) with
its machine-readable findings appendix. Its phased remediation program is the
controlling order for the four tracks below. The record-sink length-validation
fix landed with the audit.

Four re-evaluation tracks are queued alongside the canonical lesson order.
Their audits run before Lesson 081 implementation so findings can inform it;
they do not supersede the Lessons 079--081 plan or any published boundary.

1. Object-design cleanliness across the full `src/` scope: inheritance,
   composition, and construction flow; call-site repetition in examples and
   projects; type shapes that read like the components they represent.
   Findings require the architecture stress-pass discipline before any shared
   contract changes.
2. Lesson content pass over every published lesson: natural narrative flow,
   many hands-on verification steps, no standalone assessment sections
   (assessment folds into the lesson's own steps), and sufficient
   assembly-representative pencil drawings per lesson.
3. Landing-page layout and navigation redo, preserving the scan-first
   hierarchy and every publication gate.
4. Site aesthetic design pass within the pencil-drawing and monochrome-PDF
   policies where they apply.

## Physical acceptance campaign

Every lesson's PDF contains its open bench record; use the
[acceptance template](templates/hardware-acceptance.md) and
[testing contract](TESTING.md#hardware-acceptance). Work through 001 upward on
an Arduino Mega 2560 and record board revision, exact specimen, supply,
instrument, tool version, prediction, observation, interpretation, and commit.
The software-preparation audit and lesson-by-lesson child-creation gates are
recorded in the
[physical acceptance campaign plan](audits/PHYSICAL_ACCEPTANCE_CAMPAIGN_PLAN.md).

The campaign must separately prove:

1. resource acquisition and rollback;
2. primary circuit behavior;
3. the non-Serial diagnostic or named test point;
4. shutdown, reset, and power-removal safe state; and
5. any external-power, motion, radio, or storage boundary.

Blank cards stay blank. Firmware compilation, host replay, a PDF, or a visual
inspection is never physical acceptance.

## USB and HDMI mesh research

These tracks share an ordinary switched network and one normal Linux
controller. They remain outside the Arduino lesson support claim.

| Track | Current evidence | Physical work still required |
|---|---|---|
| [Transparent USB product](research/USB_TRANSPARENT_PRODUCT.md) | Native Windows/Linux contract; one Type-B Cau; four-root Type-A Pau; atomic real hubs; ColdMove; display modes; Cat6A 10GBASE-T PoE++; fencing, profiles, controller, USB/IP prototype, and a bounded [host-only durable-controller milestone](research/USB_HOST_ONLY_DURABLE_CONTROLLER_MILESTONE.md) | Durable-controller host harness, then independent Cau/Pau hardware; no-driver enumeration; protected VBUS/discharge; auxiliary-DC and no-backfeed proof; PoE/thermal and signal-integrity qualification; class/topology matrix; throughput, latency, recovery, security, and USB-IF work |
| [HDMI mesh](research/HDMI_MESH_ARCHITECTURE.md) | Connector-transparent interpreted media/reconstruction, no-signal default, fixed endpoint evidence, dynamic routes, pinned profiles, deterministic Linux controller, shared 10GBASE-T/LAN policy, synthetic read-only CLI evidence, and a bounded [host-only admission milestone](research/HDMI_SHARED_FABRIC_NEXT_MILESTONE.md) | Licensed receiver/transmitter and FPGA/SoC endpoints; measured shared-LAN compressed profiles; synthetic unprotected video; EDID/CEC/audio; 4K then 8K measurements; latency, integrity, PTP, failover, thermal/EMC, HDMI adopter and HDCP work |
| [Shared fabric](research/SHARED_USB_HDMI_FABRIC.md) | One household physical network; named profiles; pinning/fallback; ordinary-LAN headroom; observable fault states; next synthetic capacity/admission boundary selected | Shared-LAN managed-switch tests, logical VLAN/QoS policy, capacity and PoE measurements, fail-closed congestion injection, and simultaneous USB/HDMI/ordinary-LAN qualification |

The Mega 2560 may provide buttons, status, telemetry, and fault-injection
controls. It carries neither USB SuperSpeed nor HDMI media. USB/IP is a
learning prototype, not the transparent product. Unknown-protocol replay,
content-protection bypass, credential capture, and pyrotechnic control remain
out of scope. Controller high availability remains explicitly deferred.

## Publication and release

The landing page now uses one canonical source, a compact top navigation,
linked published arcs, and linkless planned rows through Lesson 081 plus the
retained research tracks. Preserve that scan-first hierarchy as work advances.
The newest published lesson is Lesson 080. The post-deploy verifier follows
the configured publication boundary and has a regression check that
must advance with the newest published lesson.

At the next clean hierarchy boundary:

1. reconcile curriculum, component, project, roadmap, site, size, and changelog
   claims;
2. run the complete host, unwind, sanitizer, style, header, Mega, size, lesson,
   monochrome-PDF, site, package-consumer, lint, and link gates;
3. preserve the concise landing-page hierarchy and verify every lesson exposes
   complementary HTML, black-and-white PDF, source, and next-step links;
4. retain every open physical card and research limitation in release notes;
5. create a coherent release commit and tag only after the tree is clean;
6. push the ordered commits and tag, enable or verify GitHub Pages, and check
   the live landing page, lesson HTML/PDF links, and downloads.

No release tag or publication is complete until the live checks pass. See
[packaging](PACKAGING.md), [PDF policy](PDF_POLICY.md), and
[the roadmap](ROADMAP.md).

## Update rule

Change this ledger whenever work becomes supported, active, queued, deferred,
or removed for a documented reason. Never erase unfinished work merely because
another boundary ships. A release audit must fail when this ledger,
`CURRICULUM.md`, `PROJECTS.md`, `ROADMAP.md`, or the published site disagree.

The integrator and every agent must re-read this file before work assignment,
after each lesson or project integration, and before release or publication.
New conversation scope enters this ledger before it can be treated as safely
queued.

## Continuous execution rule

Proceed through every safe, discoverable item in this ledger without routine
confirmation. Commit at coherent dependency boundaries and continue to the
next unblocked item. A failed gate records an owner, evidence, and next action;
it does not stop unrelated work. Ask the user one question at a time only when
a consequential choice cannot be resolved from this ledger, the canonical
design documents, repository evidence, or an already agreed preference.

Conversation history is not the work queue. Add every agreed deliverable,
constraint, deferral, and acceptance condition here or in a linked canonical
document during the same work boundary. Before ending an execution turn,
reconcile new scope and completed work with this ledger so context loss cannot
silently discard it.
