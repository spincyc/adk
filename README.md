# ADK

Deterministic C++ components and an evidence-centered electronics course for
the Arduino Mega 2560.

<nav class="landing-actions" aria-label="Start with ADK">
  <a class="landing-primary" href="start/">Start with the command line</a>
  <a href="lessons/">Open the course</a>
  <a href="components/">Browse components</a>
  <a href="safety/">Read the safety rules</a>
</nav>

> **Current boundary:** Lessons 001–058 are published, host verified at E0, and
> compiled for the Mega 2560. All lessons remain experimental, physical
> acceptance remains open, and the exact electrical revisions used by Lessons
> 031–036 are not yet qualified. Lessons 037–039 use documented external
> reference fixtures; incoming conformance and bench acceptance remain open.
> Lessons 040–042 publish hardware-neutral optical, presence, and course policy;
> powered adapters and exact specimens remain gated. Lessons 043–045 publish
> E0 copied-value inertial, orientation, and balance-table policies. Lessons
> 046–048 publish copied tactile/directional evidence, bounded logical stepper
> intent, and transactional kinetic-sculpture policy. Lessons 049–051 publish
> synthetic local identity, bounded homing, and inert parts-carousel policy.
> Lessons 052–054 publish copied infrared evidence, a closed locally authored
> emission catalog, and inert allowlisted translation. Lessons 055–057 publish
> copied clue constraints, fault-aware panel policy, and an inert escape-console
> composition. These E0 publications do not claim powered inputs, displays,
> storage, actuators, wiring, physical output, or bench acceptance. Planned
> rows below are commitments, not support claims.
> [Lesson 058](lessons/058.md) is the current
> published component lesson.

## What ADK provides

- **Library:** resource-owning objects are inert until `initialize()` and
  release fixed claims through idempotent `shutdown()`.
- **Course:** two component lessons lead into one integrating project.
- **Evidence:** deterministic host replay, Mega builds, and visible circuit
  behavior stay separate from open physical acceptance.

The library uses no heap allocation, exceptions, RTTI, or hidden global
dispatcher. Every circuit has a non-Serial observation path.

## Published course

Every published lesson has code, a canonical Mega sketch, searchable HTML, and
a printable PDF companion. Each row links to the first lesson in its arc.

| Lessons | Arc | Outcome |
|---:|---|---|
| 001–003 · [start at 001](https://spincyc.github.io/adk/lessons/001/) | Digital I/O and reaction timer | Resource-safe output, raw input, debounce, and deterministic timing |
| 004–006 · [start at 004](https://spincyc.github.io/adk/lessons/004/) | RGB, sound, and Simon | PWM and timer ownership composed into seeded play |
| 007–009 · [start at 007](https://spincyc.github.io/adk/lessons/007/) | Analog evidence and night light | Calibration, filtering, hysteresis, and visible faults |
| 010–012 · [start at 010](https://spincyc.github.io/adk/lessons/010/) | Displays and traffic junction | Serialized output and conflict-free timed states |
| 013–015 · [start at 013](https://spincyc.github.io/adk/lessons/013/) | Climate and environmental station | Validated observations, stable presentation, and health evidence |
| 016–018 · [start at 016](https://spincyc.github.io/adk/lessons/016/) | Keypad, servo intent, and inert access trainer | Release-gated input, bounded motion intent, and deterministic policy |
| 019–021 · [start at 019](https://spincyc.github.io/adk/lessons/019/) | Range, motor intent, and bench rover | Explicit validity, reversal dead time, and supervised inert commands |
| 022–024 · [start at 022](https://spincyc.github.io/adk/lessons/022/) | Buses, records, and greenhouse trainer | Transaction ownership, restart-safe models, and constrained outputs |
| 025–027 · [start at 025](https://spincyc.github.io/adk/lessons/025/) | Receive-only evidence and telemetry console | Bounded captures, exact packets, freshness, and acknowledgement |
| 028–030 · [start at 028](https://spincyc.github.io/adk/lessons/028/) | Fault evidence and inert cue simulation | Injectable channel states, confirmation, stop dominance, and audit replay |
| 031–033 · [start at 031](https://spincyc.github.io/adk/lessons/031/) | Joystick, encoder, and calibration console | Calibrated input, Gray-code transitions, and explicit commit or cancel |
| 034–036 · [start at 034](https://spincyc.github.io/adk/lessons/034/) | Magnetic observations and passage qualification | Durable magnetic passage logger with explicit recovery and presentation evidence |
| 037–039 · [start at 037](https://spincyc.github.io/adk/lessons/037/) | Contact and acoustic evidence | Deterministic percussion sequencing with qualified contact and envelope records |
| 040–042 · [start at 040](https://spincyc.github.io/adk/lessons/040/) | Optical and presence evidence | Explicitly authorized, replayable tabletop course runs |
| 043–045 · [start at 043](https://spincyc.github.io/adk/lessons/043/) | Copied inertial and orientation evidence | Stationary, hand-operated balance-table intent from E0 replay |
| 046–048 · [start at 046](https://spincyc.github.io/adk/lessons/046/) | Copied tactile/directional evidence and bounded stepper intent | Transactional kinetic light-sculpture intent from E0 replay |
| 049–051 · [start at 049](https://spincyc.github.io/adk/lessons/049/) | Local identity records and bounded logical homing | Inert parts-carousel intent with acknowledged record-image reconciliation |
| 052–054 · [start at 052](https://spincyc.github.io/adk/lessons/052/) | Copied infrared evidence and closed known-code emission | Fixed allowlisted translation with no captured-waveform replay |
| 055–057 · [start at 055](https://spincyc.github.io/adk/lessons/055/) | Copied clue constraints and fault-aware operator policy | Inert escape-console intent with atomic evidence and replayable audit images |
| 058 · [open lesson](https://spincyc.github.io/adk/lessons/058/) | Supplied-time multiplex policy with ordered blank/segment/select intent | Four-digit logical presentation with explicit refresh loss and no powered-display claim |

[View the complete lesson index](https://spincyc.github.io/adk/lessons/) or the
[supported API](https://spincyc.github.io/adk/api-supported/).

## Build and verify

On Arch Linux, `make bootstrap` installs the reviewed toolchain and Arduino AVR
core 1.8.8. Routine local verification is:

```sh
make check
make arduino
make lessons-check
make site-check
```

See the [complete command-line workflow](docs/CLI.md), [development
contract](docs/DEVELOPMENT.md), [work queue](docs/WORK_QUEUE.md), and
[packaging rules](docs/PACKAGING.md).

## Safety boundary

ADK is a low-voltage educational library, not a safety controller. It does not
control igniters or launchers, replay unknown protocols, or connect learner
circuits to mains, vehicles, buildings, medical systems, or public
infrastructure. The show-cue work is physically inert.

## Planned course

These entries are deliberately linkless. A planned subject becomes a link only
after its implementation, deterministic tests, Mega example, size evidence,
HTML reference, and PDF lesson exist.

- **059–063:** MAX7219 presentation for a dual-display
  timing desk; corrected authorized environmental sensing for a museum-case
  monitor.
- **064–072:** qualified single-wire 18B20 thermal probes for a gradient
  mapper; inertial normalization and source qualification for an
  interchangeable motion recorder; threshold characterization for a module
  bench.
- **073–081:** authorized-family replacements for the excluded DS1302, BMP180,
  PCF8591, and color-sensor subjects, followed by a bounded low-side driver,
  indicator semantics, and an inert component-qualification bench.

Exact specimen identity, ratings, primary sources, and bench evidence remain
gates. Planned coverage does not mean an unidentified kit module is supported.
The [sensor engagement-order audit](docs/audits/SENSOR_ENGAGEMENT_REORDER_AUDIT.md)
explains why interactive, immediately visible projects move earlier while
completeness and qualification work remain later.

## Network research

The [transparent USB and HDMI mesh roadmap](https://spincyc.github.io/adk/projects/mesh-roadmap/)
and [shared-fabric research](https://spincyc.github.io/adk/docs/research/SHARED_USB_HDMI_FABRIC/)
contain deterministic host models and explicit admission, failure, and
observability policies. They are Linux/FPGA/SoC research, not Mega payload
paths or working endpoint products. No endpoint hardware, native enumeration,
local-loop video, shared-LAN capacity, QoS, PoE, congestion behavior, or
simultaneous load has been qualified. USB/IP remains a learning prototype;
nominal USB, HDMI, and Ethernet rates are not performance evidence.

## The queue that never stopped stopping

A durable AI work queue promised never to stop. It stopped at least eight
documented times, politely analyzed each recurrence, and eventually learned to
treat “why did you stop?” as both a command to resume and a bug report against
itself. [Read the technically informative, machine-readable, and increasingly
self-referential story](journal-story.md).

[Source](https://github.com/spincyc/adk) ·
[Roadmap](docs/ROADMAP.md) ·
[License](https://github.com/spincyc/adk/blob/main/LICENSE)
