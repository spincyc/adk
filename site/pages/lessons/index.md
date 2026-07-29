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
| [Lesson 036 — Magnetic passage logger](036.md) | Durable passage project | Frozen evidence, two-slot recovery, retry, and post-commit presentation |
| [Lesson 037 — Contact dynamics](037.md) | Rolling-ball contact | Qualified attack and release evidence with explicit timing |
| [Lesson 038 — Acoustic envelope](038.md) | Sound Detector envelope and gate | Relative intensity, clipping, and threshold agreement |
| [Lesson 039 — Percussion sequencer](039.md) | Four contact lanes and acoustic overlap | Deterministic capture, grouping, quantization, and replay |
| [Lesson 040 — Reflective and interrupted light](040.md) | Source-specific optical evidence | Calibration, timing, transitions, and faults remain explicit |
| [Lesson 041 — Presence and passage](041.md) | PIR, beam, and range evidence | Eligibility, validity, age, and disagreement compose without hidden resampling |
| [Lesson 042 — Tabletop course marshal](042.md) | Ordered checkpoint evidence | Explicit button authorization produces replayable timed runs |
| [Lesson 043 — Copied inertial observations](043.md) | Revision-neutral six-axis values | Provenance, freshness, saturation, and faults remain explicit |
| [Lesson 044 — Board-frame orientation](044.md) | Gravity-relative pitch and roll | Fixed-point orientation becomes bounded light and tone intent |
| [Lesson 045 — Stationary balance instrument](045.md) | Copied inertial and control evidence | Button-authorized freeze and sensitivity compose into replayable E0 frames |
| [Lesson 046 — Copied tactile and directional intent](046.md) | Copied contact and directional evidence | Atomic qualification produces bounded interaction intent |
| [Lesson 047 — Bounded logical step sequencing](047.md) | Explicit commands and supplied time | Logical position and coil intent remain bounded and replayable |
| [Lesson 048 — Transactional kinetic light sculpture](048.md) | Interaction, logical motion, and light intent | Independent stop evidence dominates atomic E0 composition |
| [Lesson 049 — Local identity records](049.md) | Copied identity evidence and fixed record images | Explicit reconciliation admits local bindings without a durability claim |
| [Lesson 050 — Bounded logical homing](050.md) | Copied home and stop evidence | Release-first homing produces bounded semantic step intent |
| [Lesson 051 — Inert tabletop parts carousel](051.md) | Identity, confirmation, homing, logical motion, and audit images | An acknowledged start-record image gates inert carousel intent |
| [Lesson 052 — Copied infrared capture evidence](052.md) | Attributable copied receive evidence | Known, repeat, unknown, malformed, overflow, and source-fault results remain receive-only |
| [Lesson 053 — Known local infrared emission](053.md) | Immutable local catalog and bounded envelope intent | Only locally authored symbolic commands can produce inert carrier intent |
| [Lesson 054 — Inert IR command translator](054.md) | Fixed receive-to-local-symbol allowlist | Valid known evidence maps to a different local symbol without captured-waveform replay |
| [Lesson 055 — Constraint and clue model](055.md) | Fixed copied clue observations and project-specific rules | Explicit freshness, contradictions, and dependencies produce deterministic puzzle dispositions |
| [Lesson 056 — Fault-aware operator panel](056.md) | Atomic copied operator, presentation, and audit evidence | Stop dominance, diagnostics, acknowledgement, and restart-safe intent remain replayable |
| [Lesson 057 — Six stations, one quiet console](057.md) | Clue model and fault-aware panel composition | Six fixed clue families produce only inert, bounded console intent |
| [Lesson 058 — Nonblocking multiplexed digits](058.md) | Supplied-time logical digit transactions | Blank-before-select ordering, atomic frame swaps, and explicit refresh loss |
| [Lesson 059 — MAX7219 presentation policy](059.md) | Bounded register command/receipt transactions | Partial-prefix attribution, cleanup evidence, generation binding, and dark-start intent |
| [Lesson 060 — Dual-display timing desk](060.md) | One stopwatch snapshot and two display policies | Self-test, generation-bound receipts, agreement, and attributed disagreement |
| [Lesson 061 — Qualifying copied resistive-probe observations](061.md) | Copied Water Level acquisition evidence | Calibration, excitation-off, freshness, ordering, corrosion duty, and explicit quality precedence |
| [Lesson 062 — Combining copied thermal and radiant observations](062.md) | Three independent copied environmental roles | Uncertainty, categorical disagreement, independent ages, pulse/sustained timing, and saturation |
| [Lesson 063 — Monitoring one inert museum case](063.md) | Copied environmental, reed, acknowledgement, and audit evidence | Additive hazards, alarm latch, cooldown, inert output intent, and bounded audit delivery |
| [Lesson 064 — Bounded copied 1-Wire transactions](064.md) | Typed copied single-wire intent and receipt evidence | Microsecond windows, correlated phases, bounded ROM search, explicit rollback, and release confirmation |

Lessons 001--064 are host verified and their canonical examples compile for
the Mega 2560. Lessons 055--057 publish E0 replay only: exact inputs,
presentation, storage, actuators, and bench acceptance remain open. Lessons
058--060 likewise leave their exact powered display fixtures and bench
acceptance open. Lessons 061--063 are copied-evidence E0 policy only; their
exact powered probes/modules, adapters, wiring, physical behavior, and E1
bench acceptance remain open. Lesson 064 publishes only copied 1-Wire
transaction intent and receipts; exact endpoints, pull-ups, specimens, timing,
power behavior, and bench acceptance remain open. Every
circuit remains experimental until its physical acceptance card is recorded.
Historical preview lessons are preserved under
[Legacy](../legacy/index.md).

## Planned engagement-first sequence

The next arcs retain the engagement-first order. They remain planned—not
published or bench-verified:

| Lessons | Planned project |
|---:|---|
| 065–066 | Complete the thermal gradient mapper arc |
| 067–069 | Interchangeable motion recorder |
| 070–072 | Module characterization bench |
| 073–078 | Authorized-family subjects pending |
| 079–081 | Component qualification bench |

Pencil drawings provide orientation only. Build from each PDF’s exact schematic
and connection table.
