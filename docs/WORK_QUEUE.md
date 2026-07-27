# Work queue

This is ADK's authoritative delivery and deferred-work ledger. It records what
must not be dropped; it does not promote an interface. Lesson numbers and
subjects come from [the curriculum](CURRICULUM.md), architecture and acceptance
come from [the development contract](DEVELOPMENT.md), and detailed project
briefs come from [the project catalog](PROJECTS.md).

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
| [001--021](CURRICULUM.md#cadence) | Host verified; bench open | Preserve interfaces and complete each physical acceptance record |
| [022](design/LESSONS_022_024.md) | Active integration | Owned fixed-capacity I2C/SPI transaction foundation, tests, example, and lesson package |
| [023--024](design/LESSONS_022_024.md) | Queued | Inert load policy and greenhouse project |
| [025--027](design/LESSONS_025_027_TELEMETRY.md) | Queued | Decoded owned IR, lawful receive-only observations, and telemetry console |
| [028--030](design/LESSONS_028_030.md) | Queued | Fault/continuity models, deterministic cue scheduling, and physically inert show-cue simulator |

Integration order is strict: component or endpoint, deterministic tests,
example and size evidence, lesson package, shared indexes, then the consuming
project. Open physical cards do not pause later host work. A failed software,
safety, packaging, or publication gate does.

## Kit expansion

Lessons 031--060 are the canonical append-only kit expansion. Exact specimens,
module markings, voltage limits, and primary sources remain prerequisites to
physical work; retail kit names are not electrical identities.

| Lessons | State | Block |
|---:|---|---|
| [031--033](design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md) | Queued first | Analog joystick, quadrature encoder, calibration console |
| [034--036](projects/component_project_cadence.md#lessons-034--036-magnetic-passage-logger) | Queued | Magnetic/contact observation, passage policy, passage logger |
| [037--039](projects/component_project_cadence.md#lessons-037--039-vibration-and-sound-laboratory) | Queued | Contact dynamics, acoustic envelope, percussion sequencer |
| [040--042](projects/component_project_cadence.md#lessons-040--042-optical-course-marshal) | Queued | Optical observations, presence, tabletop course marshal |
| [043--045](projects/component_project_cadence.md#lessons-043--045-leak-and-thermal-alarm-trainer) | Queued | Resistive probes, thermal/radiant observation, museum-case monitor |
| [046--048](projects/component_project_cadence.md#lessons-046--048-tactile-kinetic-sculpture) | Queued | Touch/proximity, bounded stepper motion, kinetic sculpture |
| [049--051](projects/component_project_cadence.md#lessons-049--051-identity-controlled-parts-carousel) | Queued | Local identity records, homing, inert parts carousel |
| [052--054](projects/component_project_cadence.md#lessons-052--054-infrared-protocol-workbench) | Queued | Owned IR capture, known-code transmission, command translator |
| [055--057](projects/component_project_cadence.md#lessons-055--057-modular-sensor-test-bench) | Queued | Threshold descriptors, characterization, module test bench |
| [058--060](projects/component_project_cadence.md#lessons-058--060-cooperative-escape-room-console) | Queued | Constraint model, fault-aware panel, inert escape-room console |
| [061--063](projects/component_project_cadence.md#deferred-lessons-061--063-balance-table-instrument) | Deferred | MPU6050 samples, orientation presentation, balance-table instrument |

The input-first 031--033 decision supersedes older use of 032--033 for the
MPU6050 and balance table. That work is retained at 061--063 without a number
collision. The [kit coverage audit](design/ELEGOO_MEGA_KIT_COVERAGE_2026-07-27.md),
[sensor taxonomy](research/SENSOR_KIT_TAXONOMY.md), and
[safety taxonomy](design/KIT_MODULE_SAFETY_TAXONOMY.md) remain implementation
inputs. Coverage is not complete until every claimed module has an exact
inventory record and the relevant block passes its gates.

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
| [Transparent USB product](research/USB_TRANSPARENT_PRODUCT.md) | Product terms, dynamic exclusive routes, profiles, fencing, deterministic controller and Linux USB/IP prototype | Physical computer attachment unit and peripheral attachment unit; native Windows/Linux enumeration; SuperSpeed class/topology matrix; protected VBUS; PoE budget; signal integrity, throughput, latency, recovery, security, and USB-IF work |
| [HDMI mesh](research/HDMI_MESH_ARCHITECTURE.md) | Interpreted media/reconstruction architecture, dynamic routes, profiles, deterministic controller, shared-fabric policy, and synthetic read-only CLI evidence | Licensed receiver/transmitter and FPGA/SoC endpoints; synthetic unprotected video; EDID/CEC/audio; 4K then 8K measurements; latency, integrity, PTP, failover, thermal/EMC, HDMI adopter and HDCP work |
| [Shared fabric](research/SHARED_USB_HDMI_FABRIC.md) | Named admission profiles, pinning/fallback policy, ordinary-LAN headroom, observable fault states | Managed-switch lab, traffic isolation, capacity and PoE measurements, congestion/fault injection, and simultaneous USB/HDMI qualification |

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
