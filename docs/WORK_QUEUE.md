# Work queue

This is ADK's authoritative delivery and deferred-work ledger. It records what
must not be dropped; it does not promote an interface. Lesson numbers and
subjects come from [the curriculum](CURRICULUM.md), architecture and acceptance
come from [the development contract](DEVELOPMENT.md), and detailed project
briefs come from [the project catalog](PROJECTS.md) and the
[extended cadence](projects/component_project_cadence.md).

## Restart checkpoint

Last reconciled: 2026-07-27 on `main`.

The repository-wide recovery audit and ordered journal-ingestion prompts are
recorded in
[the work-queue recovery audit](audits/REPOSITORY_WORK_QUEUE_RECOVERY_AUDIT.md).
Its findings must be reconciled through bounded tasks; the audit itself does
not promote work or replace this ledger.

- Lessons 001--029 are promoted and host verified; physical cards remain open.
- Lesson 029 has an independently reviewed scheduler and audit core, canonical
  Mega example, measured size baseline, HTML reference, monochrome PDF lesson,
  downloads, and navigation. Its E1 physical acceptance card remains open.
- Lesson 030 has a committed implementation contract. Implementation follows
  Lesson 029 promotion.
- The USB research track has a deterministic product-native `Cau`/`Pau`
  `ColdMove` model. It performs no USB action and makes no transparency claim.
- The exact kit inventory template and honest planned-versus-supported coverage
  language are committed. Physical inventory records remain open.

Do not lose these release blockers:

1. correct lesson 014's nonexistent `make serial-monitor` command and
   reproducibly rebuild `doc/lessons/014.pdf`;
2. add `git` to the Arch bootstrap package set, pin the local AVR core to the CI
   version, and preserve monitor failures through the logging pipeline;
3. add native archive/export and consumer-smoke targets before claiming
   general non-Arduino C++ installation;
4. finish the landing/navigation audit and add post-deploy checks for the
   newest lesson and PDF;
5. run the full clean quality gate, release, push, and verify GitHub Pages only
   after the active lesson boundary is clean.

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

| Boundary | State | Required next result |
|---|---|---|
| [001--029](CURRICULUM.md#cadence) | Host verified; bench open | Preserve interfaces and complete each physical acceptance record |
| [030](design/LESSONS_028_030.md) | Queued | Physically inert show-cue simulator |

Integration order is strict: component or endpoint, deterministic tests,
example and size evidence, lesson package, shared indexes, then the consuming
project. Open physical cards do not pause later host work. A failed software,
safety, packaging, or publication gate does.

Deferred but retained: physical RTC and removable-media adapters require exact
specimen selection, primary datasheets, electrical qualification, and their
own failure and bench evidence. Lesson 022 supports deterministic RTC state and
fixed-storage durability models, not physical RTC or SD hardware. Lesson 017's
configuration codec has exhaustive every-byte corruption, erased, oversized,
generation-wrap, and deterministic replay coverage.

## Kit expansion

Lessons 031--081 are the canonical append-only kit expansion. Exact specimens,
module markings, voltage limits, and primary sources remain prerequisites to
physical work; retail kit names are not electrical identities.

| Lessons | State | Block |
|---:|---|---|
| [031--033](design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md) | Queued first | Analog joystick, quadrature encoder, calibration console |
| [034--036](projects/component_project_cadence.md) | Queued | Magnetic/contact observation, passage policy, passage logger |
| [037--039](projects/component_project_cadence.md) | Queued | Contact dynamics, acoustic envelope, percussion sequencer |
| [040--042](projects/component_project_cadence.md) | Queued | Optical observations, presence, tabletop course marshal |
| [043--045](projects/component_project_cadence.md) | Queued | Resistive probes, thermal/radiant observation, museum-case monitor |
| [046--048](projects/component_project_cadence.md) | Queued | Touch/proximity, bounded stepper motion, kinetic sculpture |
| [049--051](projects/component_project_cadence.md) | Queued | Local identity records, homing, inert parts carousel |
| [052--054](projects/component_project_cadence.md) | Queued | Owned IR capture, known-code transmission, command translator |
| [055--057](projects/component_project_cadence.md) | Queued | Threshold descriptors, characterization, module test bench |
| [058--060](projects/component_project_cadence.md) | Queued | Constraint model, fault-aware panel, inert escape-room console |
| [061--063](projects/component_project_cadence.md) | Queued | Revision-neutral MPU6050/QMI8658 samples, orientation presentation, balance-table instrument |
| [064--066](projects/component_project_cadence.md) | Queued | Normalized inertial records, source qualification, motion recorder |
| [067--069](projects/component_project_cadence.md) | Queued | Owned single-wire transport, DS18B20 probe set, thermal mapper |
| [070--072](projects/component_project_cadence.md) | Queued | Multiplexed digits, MAX7219 display transport, timing desk |
| [073--075](projects/component_project_cadence.md) | Queued | DS1302 three-wire clock, BMP180/PCF8591 acquisition, pressure and analog station |
| [076--078](projects/component_project_cadence.md) | Queued | Identified color adapter, fixed calibration, sorting trainer |
| [079--081](projects/component_project_cadence.md) | Queued | Bounded low-side driver, indicator semantics, inert qualification bench |

The input-first 031--033 decision supersedes older use of 032--033 for the
MPU6050 and balance table. That work is retained at 061--063 without a number
collision. The [kit coverage audit](design/ELEGOO_MEGA_KIT_COVERAGE_2026-07-27.md),
[sensor taxonomy](research/SENSOR_KIT_TAXONOMY.md), and
[safety taxonomy](design/KIT_MODULE_SAFETY_TAXONOMY.md) remain implementation
inputs. Coverage is not complete until every claimed module has an exact
inventory record and the relevant block passes its gates.

Lessons 031--033 have an implementation-ready design brief. Lessons 034--081
are canonical subjects and retained work, but their cadence entries are not
implementation-ready lesson plans. Before code begins for each three-lesson
arc, expand it to the same depth as 031--033: public values and interfaces,
resource and pin budgets, deterministic fixture and failure matrices,
narrative example flow, staged circuit-native experiments, HTML/PDF division,
specimen gates, and explicit bench acceptance. “Planned specimen coverage”
never means that an exact kit module is supported.

The PCF8574 LCD backpack, gas-response experiments, unidentified emitters,
physiological claims, and prototype/power construction variants still require
separately scoped, inventory-gated planning. Lesson 057 covers only identified
low-voltage analog/comparator characterization; it is not a catch-all claim
for every product sold as a 37-sensor kit.

## Physical acceptance campaign

Every lesson's PDF contains its open bench record; use the
[acceptance template](templates/hardware-acceptance.md) and
[testing contract](TESTING.md#hardware-acceptance). Work through 001 upward on
an Arduino Mega 2560 and record board revision, exact specimen, supply,
instrument, tool version, prediction, observation, interpretation, and commit.

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
| [Transparent USB product](research/USB_TRANSPARENT_PRODUCT.md) | Native Windows/Linux contract; one Type-B Cau; four-root Type-A Pau; atomic real hubs; ColdMove; display modes; Cat6A 10GBASE-T PoE++; fencing, profiles, controller, and USB/IP prototype | Independent Cau/Pau hardware; no-driver enumeration; protected VBUS/discharge; auxiliary-DC and no-backfeed proof; PoE/thermal and signal-integrity qualification; class/topology matrix; throughput, latency, recovery, security, and USB-IF work |
| [HDMI mesh](research/HDMI_MESH_ARCHITECTURE.md) | Connector-transparent interpreted media/reconstruction, no-signal default, fixed endpoint evidence, dynamic routes, pinned profiles, deterministic Linux controller, shared 10GBASE-T/LAN policy, and synthetic read-only CLI evidence | Licensed receiver/transmitter and FPGA/SoC endpoints; measured shared-LAN compressed profiles; synthetic unprotected video; EDID/CEC/audio; 4K then 8K measurements; latency, integrity, PTP, failover, thermal/EMC, HDMI adopter and HDCP work |
| [Shared fabric](research/SHARED_USB_HDMI_FABRIC.md) | One household physical network; named profiles; pinning/fallback; ordinary-LAN headroom; observable fault states | Shared-LAN managed-switch tests, logical VLAN/QoS policy, capacity and PoE measurements, fail-closed congestion injection, and simultaneous USB/HDMI/ordinary-LAN qualification |

The Mega 2560 may provide buttons, status, telemetry, and fault-injection
controls. It carries neither USB SuperSpeed nor HDMI media. USB/IP is a
learning prototype, not the transparent product. Unknown-protocol replay,
content-protection bypass, credential capture, and pyrotechnic control remain
out of scope. Controller high availability remains explicitly deferred.

## Publication and release

At the next clean hierarchy boundary:

1. reconcile curriculum, component, project, roadmap, site, size, and changelog
   claims;
2. run the complete host, unwind, sanitizer, style, header, Mega, size, lesson,
   monochrome-PDF, site, package-consumer, lint, and link gates;
3. finish the concise landing-page hierarchy and verify every lesson exposes
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
