# ADK

Deterministic C++ components and an evidence-centered electronics course for
the Arduino Mega 2560.

<nav class="landing-actions" aria-label="Start with ADK">
  <a class="landing-primary" href="start/">Start with the command line</a>
  <a href="lessons/">Open the course</a>
  <a href="components/">Browse components</a>
  <a href="safety/">Read the safety rules</a>
</nav>

> **Current boundary:** Lessons 001–035 are published, host verified, and
> compiled for the Mega 2560. All lessons remain experimental, physical
> acceptance remains open, and the exact electrical revisions used by Lessons
> 031–035 are not yet qualified. Lesson 036 is the active planned project
> boundary; planned rows below are commitments, not support claims.
> [Lesson 035](lessons/035.md) is the newest published component.

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
| 034–035 · [start at 034](https://spincyc.github.io/adk/lessons/034/) · [continue at 035](lessons/035.md) | Magnetic observations and passage qualification | Qualified magnetic evidence, bounded direction, timeout, and suppression |

[View the complete lesson index](https://spincyc.github.io/adk/lessons/) or the
[supported API](https://spincyc.github.io/adk/api-supported/).

## Planned course

These entries are deliberately linkless. A planned subject becomes a link only
after its implementation, deterministic tests, Mega example, size evidence,
HTML reference, and PDF lesson exist.

| Lessons | Planned subjects | Planned project |
|---:|---|---|
| 036 | Active planned composition | Magnetic passage logger |
| 037–039 | Contact dynamics; acoustic envelopes | Percussion sequencer |
| 040–042 | Reflective/interrupted light; presence and passage | Tabletop course marshal |
| 043–045 | Resistive environmental probes; thermal/radiant observations | Museum-case monitor |
| 046–048 | Touch/proximity demonstrations; bounded stepper motion | Kinetic light sculpture |
| 049–051 | Local identity records; positioning and homing | Tabletop parts carousel |
| 052–054 | Known IR captures; bounded known-code transmission | IR command translator |
| 055–057 | Threshold-module descriptors; characterization runs | Module characterization bench |
| 058–060 | Constraint/clue model; fault-aware operator panel | Inert escape-room console |
| 061–063 | Revision-neutral inertial samples; orientation presentation | Balance-table instrument |
| 064–066 | Inertial record normalization; source qualification | Interchangeable motion recorder |
| 067–069 | Single-wire transactions; qualified thermal probes | Thermal gradient mapper |
| 070–072 | Multiplexed digits; MAX7219 presentation | Dual-display timing desk |
| 073–075 | Authorized-family replacements for excluded DS1302, BMP180, and PCF8591 subjects | Replacement project after re-scope |
| 076–078 | Authorized-family replacement for the excluded color-sensor subject; follow-on component | Replacement project after re-scope |
| 079–081 | Bounded low-side driver; indicator-module semantics | Component qualification bench |

Exact specimen identity, ratings, primary sources, and bench evidence remain
gates. Planned coverage does not mean an unidentified kit module is supported.

## Network research

The network tracks are Linux/FPGA/SoC research, not Mega payload paths or
working endpoint products.

| Track | Evidence available now | Next unverified milestone |
|---|---|---|
| [Transparent USB](https://spincyc.github.io/adk/projects/mesh-roadmap/) | Deterministic routing, fencing, profiles, controller models, and hardware studies | Independent computer/peripheral attachment hardware and native enumeration |
| [HDMI mesh](https://spincyc.github.io/adk/projects/mesh-roadmap/) | Endpoint, route, profile, and reconstruction architecture with host models | Licensed receiver/transmitter endpoints and measured local-loop video |
| [Shared fabric](https://spincyc.github.io/adk/docs/research/SHARED_USB_HDMI_FABRIC/) | Admission, headroom, failure, and observability policy | Managed-switch capacity, QoS, PoE, congestion, and simultaneous-load measurements |

No endpoint hardware has been qualified. USB/IP is a learning prototype, and
nominal USB, HDMI, or Ethernet rates are not performance evidence.

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

[Source](https://github.com/spincyc/adk) ·
[Roadmap](docs/ROADMAP.md) ·
[License](https://github.com/spincyc/adk/blob/main/LICENSE)
