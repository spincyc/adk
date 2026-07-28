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

Lessons 037–081 are retained canonical work;
each later three-lesson arc requires an implementation-depth brief before code
begins.

The [canonical curriculum](docs/CURRICULUM.md) owns lesson numbers,
prerequisites, and acceptance gates. [Project briefs](docs/PROJECTS.md) explain
how each checkpoint composes the earlier layers.

Historical preview material is available under [Legacy](legacy/index.md), but
is not a prerequisite or current API reference.
