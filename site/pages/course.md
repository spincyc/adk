# Course map

ADK introduces two component layers and then exercises them in a project. Every
lesson number divisible by three is project-bearing.

| Lessons | New components | Project | Status |
|---:|---|---|---|
| 001–003 | Digital output, digital input, Button | Reaction timer | Host verified; bench open |
| 004–006 | PWM, RGB LED, sounder | Simon | Host verified; bench open |
| 007–009 | Analog input, calibration, filtering | Adaptive night light | Host verified; bench open |
| 010–012 | Shift register, display, timed states | Traffic junction | Host verified; bench open |
| 013 | DHT11 sensor and validated samples | — | Host verified; bench open |
| 014–015 | Character display and stable records | Environmental station | Host verified; bench open |
| 016 | Matrix keypad | — | Host verified; bench open |
| 017 | Bounded servo | — | Host verified; E2 bench open |
| 018 | Keypad policy and audit intent | Inert access trainer | Host verified; E1 bench open |
| 019 | Ultrasonic range and explicit validity | — | Host verified; bench open |
| 020 | Motor intent and stop policy | — | Host verified; E1 bench open |
| 021 | Rover supervision | Bench rover | Host verified; E1 bench open |
| 022 | Owned buses, RTC state, and deterministic durable records | Bus and storage trainer | Host verified; bench open |
| 023 | Constrained output simulation | Inert load interlock | Host verified; E1 bench open |
| 024 | Deterministic sensing, output, and records | Greenhouse controller | Host verified; E1 bench open |
| 025 | Owned infrared capture and classic NEC evidence | Infrared evidence trainer | Host verified; E1 bench open |
| 026 | Receive-only packets and freshness | Telemetry evidence trainer | Host verified; E1 bench open |
| 027 | Deterministic scheduling and presentation | Telemetry console | Host verified; E1 bench open |
| 028 | Inert channel assessment | — | Host verified; E1 bench open |
| 029 | Cue scheduling and logs | — | Host verified; E1 bench open |
| 030 | Inert composition | Inert show-cue simulator | Host verified; E1 bench open |
| 031 | Calibrated joystick | — | Host verified; E1 bench open |
| 032 | Quadrature encoder | — | Host verified; E1 bench open |
| 033 | Calibration policy | Calibration console | Host verified; E1 bench open |
| 034 | Magnetic observations | Qualified analog/contact evidence | Host verified; exact specimen and E1 bench open |
| 035 | Passage qualification | Bounded direction and passage evidence | Host verified; exact specimen and E1 bench open |
| 036 | Durable passage records | Magnetic passage logger | Host verified; exact specimen and E1 bench open |
| 037 | Contact dynamics | Qualified attack and release evidence | Host verified; incoming conformance and E1 bench open |
| 038 | Acoustic envelope | Relative intensity and threshold evidence | Host verified; incoming conformance and E1 bench open |
| 039 | Contact and acoustic composition | Percussion sequencer | Host verified; incoming conformance and E1 bench open |

The retained Lessons 040–081 sequence is front-loaded for learner engagement.
This is planned work, not a support or bench-verification claim:

| Lessons | Planned focus | Project |
|---:|---|---|
| 040–042 | Optical observations and presence | Tabletop course marshal |
| 043–045 | Inertial samples and orientation | Balance-table instrument |
| 046–048 | Authorized tactile/directional inputs and bounded stepper motion | Kinetic sculpture |
| 049–051 | Local identity records and homing | Inert parts carousel |
| 052–054 | Known-family IR capture and bounded emission | IR command translator |
| 055–057 | Constraint and fault-aware operator models | Inert escape-room console |
| 058–060 | Multiplexed digits and matrix presentation | Dual-display timing desk |
| 061–063 | Resistive and thermal/radiant observations | Museum-case monitor |
| 064–066 | Single-wire probes and qualified thermal sets | Thermal gradient mapper |
| 067–069 | Normalized inertial records and source qualification | Interchangeable motion recorder |
| 070–072 | Threshold descriptors and characterization sweeps | Module characterization bench |
| 073–075 | Authorized-family replacements pending | Project pending |
| 076–078 | Authorized-family replacements pending | Project pending |
| 079–081 | Bounded load driver and indicator semantics | Component qualification bench |

Each later three-lesson arc requires an implementation-depth brief before code
begins. Lessons 073–078 retain their numbers but require authorized-family
subjects before activation.

The [canonical curriculum](docs/CURRICULUM.md) owns lesson numbers,
prerequisites, and acceptance gates. [Project briefs](docs/PROJECTS.md) explain
how each checkpoint composes the earlier layers.

Historical preview material is available under [Legacy](legacy/index.md), but
is not a prerequisite or current API reference.
