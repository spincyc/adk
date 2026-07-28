# Work queue

This is ADK's authoritative delivery and deferred-work ledger. It records what
must not be dropped; it does not promote an interface. Lesson numbers and
subjects come from [the curriculum](CURRICULUM.md), architecture and acceptance
come from [the development contract](DEVELOPMENT.md), and detailed project
briefs come from [the project catalog](PROJECTS.md) and the
[extended cadence](projects/component_project_cadence.md).

## Restart checkpoint

Last reconciled: 2026-07-28 on `main`.

The repository-wide recovery audit and ordered journal-ingestion prompts are
recorded in
[the work-queue recovery audit](audits/REPOSITORY_WORK_QUEUE_RECOVERY_AUDIT.md).
Its findings must be reconciled through bounded tasks; the audit itself does
not promote work or replace this ledger.

- Lessons 001--045 are promoted and host verified; physical cards remain open.
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
| [046--048](projects/component_project_cadence.md) | Queued; authorized specimens only | Authorized tactile/directional inputs, bounded stepper motion, kinetic sculpture; excluded or unidentified pulse/gesture modules are not claimed |
| [049--051](projects/component_project_cadence.md) | Queued | Local identity records, homing, inert parts carousel |
| [052--054](projects/component_project_cadence.md) | Queued; exact emitter gated | Known-kit IR capture, bounded listed IR-emission family, command translator |
| [055--057](projects/component_project_cadence.md) | Queued | Constraint model, fault-aware panel, inert escape-room console |
| [058--060](projects/component_project_cadence.md) | Queued | Multiplexed digits, MAX7219 display transport, timing desk |
| [061--063](projects/component_project_cadence.md) | Queued; corrected authorized scope | Resistive probes and qualified thermal/radiant observations composed into a museum-case monitor |
| [064--066](projects/component_project_cadence.md) | Queued | Single-wire transport, listed 18B20 temperature family, thermal mapper |
| [067--069](projects/component_project_cadence.md) | Queued | Normalized inertial records, source qualification, motion recorder |
| [070--072](projects/component_project_cadence.md) | Queued | Threshold descriptors, characterization, module test bench |
| [073--075](projects/component_project_cadence.md) | Re-scope required | DS1302, BMP180, and PCF8591 are not in the authorized Elegoo union; retain numbers but replace subjects before activation |
| [076--078](projects/component_project_cadence.md) | Re-scope required | Color sensor is not in the authorized Elegoo union; retain numbers but replace subjects before activation |
| [079--081](projects/component_project_cadence.md) | Queued | Bounded low-side driver, indicator semantics, inert qualification bench |

The input-first 031--033 decision supersedes older use of 032--033 for the
MPU6050 and balance table. The engagement-order audit now retains that work at
043--045 without a number collision and orders the later arcs by learner
payoff rather than inventory taxonomy alone. The
[kit coverage audit](design/ELEGOO_MEGA_KIT_COVERAGE_2026-07-27.md),
[sensor taxonomy](research/SENSOR_KIT_TAXONOMY.md), and
[safety taxonomy](design/KIT_MODULE_SAFETY_TAXONOMY.md) remain implementation
inputs. Coverage is not complete until every claimed module has an exact
inventory record and the relevant block passes its gates.

Lessons 031--045 have complete implementation-ready design briefs and are host
verified. Lessons 037--039 are published against exact external reference
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
published boundary preserves exact MPU6050 and QMI8658 variants as independent
future adapter gates; exact-specimen qualification, powered adapters,
authoritative schematics, and E1 bench acceptance remain open. The later
067--069 normalization, qualification, comparison, and recorder scope remains
intact. Lessons 046--081 remain canonical subjects and retained work in the
engagement order above, but their cadence entries are not
implementation-ready lesson plans. Before code begins for each later
three-lesson
arc, expand it to the same depth as 031--033: public values and interfaces,
resource and pin budgets, deterministic fixture and failure matrices,
narrative example flow, staged circuit-native experiments, HTML/PDF division,
specimen gates, and explicit bench acceptance. “Planned specimen coverage”
never means that an exact kit module is supported.

The PCF8574 LCD backpack, gas-response experiments, unidentified emitters,
physiological claims, and prototype/power construction variants still require
separately scoped, inventory-gated planning. Lesson 072 covers only identified
low-voltage analog/comparator characterization; it is not a catch-all claim
for every product sold as a 37-sensor kit.

The source audit found that the retained DS1302, BMP180, PCF8591, and color
sensor subjects came from other vendor taxonomies. They are not authorized
specimens and must not be implemented under the Elegoo-set task. Re-scoping
keeps lesson numbers 073--078 stable.

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
The newest published lesson is Lesson 045. The post-deploy verifier follows
Lesson 045 and has a regression check that
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
