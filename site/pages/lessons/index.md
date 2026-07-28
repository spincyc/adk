# First-class lessons

The sequence adds one ownership or composition idea at a time, then uses every
third lesson for a deterministic integration project.

| Lesson | Circuit | Main idea |
|---|---|---|
| [Lesson 001 — Digital output](001.md) | Mega built-in LED | RAII output and visible diagnostics |
| [Lesson 002 — Digital input](002.md) | D22 button input | Pull-up wiring and raw observation |
| [Lesson 003 — Reaction timer](003.md) | Reaction timer | Debounce, explicit time, and replay |
| [Lesson 004 — PWM and RGB](004.md) | Common-cathode RGB LED | PWM ownership and composition |
| [Lesson 005 — Piezo sounder](005.md) | Passive piezo | Timer ownership and explicit duration |
| [Lesson 006 — Simon](006.md) | Simon | Seeded cues and complete input snapshots |
| [Lesson 007 — Analog input](007.md) | Potentiometer and PWM LED | Raw ADC evidence and explicit calibration |
| [Lesson 008 — Sampled signal](008.md) | Sample filter | Reproducible filtering and fault observation |
| [Lesson 009 — Adaptive night light](009.md) | Adaptive night light | Hysteresis and multi-component diagnosis |
| [Lesson 010 — Shift-register display](010.md) | Shift-register display | Serialized output and visible glyphs |
| [Lesson 011 — Timed traffic states](011.md) | Timed traffic states | Explicit deadlines and request retention |
| [Lesson 012 — Traffic junction](012.md) | Tabletop traffic junction | Conflict-free signals and all-red failure |
| [Lesson 013 — DHT11 climate sensor](013.md) | DHT11 climate sensor | Timed acquisition and validated samples |
| [Lesson 014 — Character display](014.md) | Character display | Staged startup and stable records |
| [Lesson 015 — Environmental station](015.md) | Environmental station | Deterministic climate project |
| [Lesson 016 — Matrix keypad](016.md) | Matrix keypad | Scanning and operator events |
| [Lesson 017 — Bounded servo intent](017.md) | Bounded servo | Calibrated motion intent and safe pulse evidence |
| [Lesson 018 — Inert access trainer](018.md) | Inert access trainer | Keypad policy, visible state, and bounded audit intent |
| [Lesson 019 — Ultrasonic range](019.md) | Ultrasonic range | Explicit echo timing, timeout, and range validity |
| [Lesson 020 — Motor intent](020.md) | Motor intent | Direction, bounded duty, reversal dead time, and stop dominance |
| [Lesson 021 — Bench rover](021.md) | Bench rover | Range-aware supervision and inert motor-command evidence |
| [Lesson 022 — Owned buses and durable records](022.md) | Owned buses and durable records | Transaction ownership, explicit clock state, and restart recovery |
| [Lesson 023 — Inert load interlock](023.md) | Inert load interlock | Mutually exclusive LED loads and safe watering policy |
| [Lesson 024 — Greenhouse trainer](024.md) | Observable greenhouse trainer | Coherent stages, health evidence, and durable replay |
| [Lesson 025 — Infrared evidence](025.md) | Infrared frame evidence | Owned capture, classic NEC validity, and stable records |
| [Lesson 026 — Receive-only telemetry](026.md) | Receive-only telemetry | Exact packets, freshness, and visible evidence |
| [Lesson 027 — Telemetry console](027.md) | Telemetry console | Deterministic selection, health, acknowledgement, and bounded records |
| [Lesson 028 — Inert channel assessment](028.md) | Inert channel assessment | Recorded open, short, stale, and contradictory evidence |
| [Lesson 029 — Inert cue scheduling and audit](029.md) | Inert cue scheduling and audit | Confirmation windows, inert intervals, and bounded replayable records |
| [Lesson 030 — Inert show-cue simulator](030.md) | Inert show-cue simulator | Continuity-gated cues, stop dominance, and deterministic audit replay |
| [Lesson 031 — Analog joystick](031.md) | Two-axis joystick | Calibrated axes, dead zone, and separate select events |
| [Lesson 032 — Quadrature encoder](032.md) | Rotary encoder | Gray-code edges, invalid transitions, and saturating position |
| [Lesson 033 — Calibration console](033.md) | Calibration console | Joystick selection, encoder trim, and explicit commit or cancel |
| [Lesson 034 — Magnetic observations](034.md) | Qualified magnetic evidence | Raw analog/contact observations, hysteresis, dwell, and specimen gates |
| [Lesson 035 — Passage qualification](035.md) | Qualified passage evidence | Direction, timeout, suppression, and optional position corroboration |

These interfaces are host verified and their canonical examples compile for the
Mega 2560. Every circuit remains experimental until its physical acceptance
card is recorded. Historical preview lessons are preserved under
[Legacy](../legacy/index.md).

Pencil drawings provide orientation only. Build from each PDF’s exact schematic
and connection table.
