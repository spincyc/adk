# ADK

Deterministic C++ components and an evidence-centered electronics course for
the Arduino Mega 2560.

<nav class="landing-actions" aria-label="Start with ADK">
  <a class="landing-primary" href="start/">Set up and run Lesson 001</a>
  <a href="course/">See the course map</a>
  <a href="safety/">Read the safety rules</a>
</nav>

New here? Follow the three steps in order: set up the toolchain, walk the
course map, then open [Lesson 001](lessons/001.md). Every lesson links its
own HTML reference, print-edition PDF, source, and open bench record.
[Browse components](components.md) when you want the library rather than the
course.

> **Current boundary:** Lessons 001–072 and 079–080 are published and host
> verified at E0 — deterministic C++ policy, compiled for the Mega 2560, with
> every lesson's physical acceptance record still open. E0 means copied or
> synthetic evidence only: no lesson here establishes that a powered specimen
> works. [Lesson 080](lessons/080.md) is the current published lesson.
> Per-lesson scope, open gates, and specimen qualification are stated once in
> [the current status](about.md#current-status); planned rows below are
> commitments, not support claims.

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
| 059 · [open lesson](https://spincyc.github.io/adk/lessons/059/) | MAX7219 register presentation policy | Bounded command/receipt evidence, partial-prefix attribution, and no powered-matrix claim |
| 060 · [open lesson](https://spincyc.github.io/adk/lessons/060/) | Dual-display timing desk | One stopwatch snapshot, two generation-bound display intents, self-test, and attributed disagreement |
| 061 · [open lesson](https://spincyc.github.io/adk/lessons/061/) | Copied resistive-probe observations | Calibration, excitation-off evidence, freshness, ordering, and bounded corrosion duty without a powered-probe claim |
| 062 · [open lesson](https://spincyc.github.io/adk/lessons/062/) | Copied thermal and radiant observations | Independent identity, age, uncertainty, disagreement, pulse/sustained timing, and saturation without powered-sensor claims |
| 063 · [open lesson](https://spincyc.github.io/adk/lessons/063/) | Inert museum-case monitor | Simultaneous hazard evidence, latched acknowledgement/cooldown, and bounded audit intent without powered or durable claims |
| 064 · [open lesson](https://spincyc.github.io/adk/lessons/064/) | Bounded copied 1-Wire transactions | Typed microsecond intent/receipt sequencing, bounded ROM search, and release-confirmed rollback without a powered bus claim |
| 065 · [open lesson](https://spincyc.github.io/adk/lessons/065/) | Qualified four-probe 18B20 set | Fixed identities, correlated conversion evidence, CRC, freshness, disappearance, and byte-stable replay without a powered-probe claim |
| 066 · [open lesson](https://spincyc.github.io/adk/lessons/066/) | Thermal-gradient mapper | Ordered interval, fault, page, and volatile record intent from copied qualified slots without display, storage, or authentication claims |
| 067 · [open lesson](https://spincyc.github.io/adk/lessons/067/) | Normalized inertial records | Attributable copied source-frame values and a canonical 64-byte image without powered acquisition, qualification, or persistence claims |
| 068 · [open lesson](https://spincyc.github.io/adk/lessons/068/) | Configured inertial-record qualification | One configured copied-record stream, one proper rotation, and terminal stationary-window evidence without a powered-sensor claim |
| 069 · [open lesson](https://spincyc.github.io/adk/lessons/069/) | Qualified motion recorder | One-source-per-session scripted motion evidence, bounded volatile record images, and inert presentation/export intent |
| 070 · [open lesson](https://spincyc.github.io/adk/lessons/070/) | Descriptor-driven threshold modules | Explicit copied topology, polarity, timing, electrical declarations, and frame provenance without powered-specimen authority |
| 071 · [open lesson](https://spincyc.github.io/adk/lessons/071/) | Copied threshold characterization | Bounded three-leg replay, adjacent transition brackets, conservative intervals, and attributable disagreement |
| 072 · [open lesson](https://spincyc.github.io/adk/lessons/072/) | Inert module-characterization bench | Atomic terminal-envelope review, fault-dominant presentation, and one canonical 192-byte volatile record |
| 079 · [open lesson](https://spincyc.github.io/adk/lessons/079/) | Bounded low-side-driver policy | Checked current budgets, bounded duty history, stop-dominant all-off intent, and no powered-driver claim |
| 080 · [open lesson](https://spincyc.github.io/adk/lessons/080/) | Small-indicator semantics policy | Polarity, autonomy, safe-state, timing, and copied-observation agreement without a powered-indicator claim |

[Lesson 080 PDF](https://spincyc.github.io/adk/downloads/lessons/080.pdf) ·
[Mega example](https://github.com/spincyc/adk/tree/main/examples/Lesson080SmallIndicatorSemantics) ·
[public API](https://github.com/spincyc/adk/blob/main/src/bounded_low_side_driver_policy.h) ·
[host tests](https://github.com/spincyc/adk/blob/main/tests/bounded_low_side_driver_test.cpp)

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

- **073–075:** **Copied RTC Transaction Evidence**, **Qualified Clock
  Observation**, and the **Inert Time-Warp Detective Desk**. This selected E0
  arc uses copied DS1307-family evidence only; DS3231 remains a separate,
  independently gated variant.
- **076–078:** **Copied Sweep-Range Frames**, **Bounded Polar Occupancy Map**,
  and the **Inert Tabletop Sonar Desk**. This planned E0 arc replays copied
  angle–range evidence without moving a servo or powering a ranger.
- **081:** an inert component-qualification bench, composing the published
  bounded low-side-driver and small-indicator policies.

The historical DS1302, BMP180, and PCF8591 subjects remain excluded; selecting
Lessons 073–075 does not restore them or claim a powered RTC.

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
