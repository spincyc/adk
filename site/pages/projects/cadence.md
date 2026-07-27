# Project cadence

Each checkpoint uses only earlier interfaces and must provide deterministic
host traces, a Mega 2560 example, safety evidence, terse HTML, and a rich bench
PDF.

| Lesson | Project | Principal evidence |
|---:|---|---|
| 003 | Reaction timer | Bounce, false-start, timeout, wrap, replay — hardware experimental |
| 006 | Simon | Stable sequence vectors and full input replay — hardware experimental |
| 009 | Adaptive night light | Calibration, filtering, hysteresis — hardware experimental |
| 012 | Traffic junction | No conflicting greens; all-red failure |
| 015 | Environmental station | Sensor validity and stable records |
| 018 | Inert access trainer | Lockout and corrupt-state recovery |
| 021 | Bench rover | Scripted route and emergency stop |
| 024 | Greenhouse controller | Schedules, faults, simulated loads |
| 027 | Telemetry console | Receive-only observations and logging |
| 030 | Inert show-cue simulator | Dry run, abort, faults, audit trail |

The final project never controls an igniter, launcher, or cloned transmitter.
See the [safety policy](../safety.md), [curriculum](../docs/CURRICULUM.md), and
[full project briefs](../docs/PROJECTS.md).
