# Project cadence

Each checkpoint uses only earlier interfaces and must provide deterministic
host traces, a Mega 2560 example, safety evidence, terse HTML, and a rich bench
PDF.

| Lesson | Project | Status | Principal evidence |
|---:|---|---|---|
| 003 | Reaction timer | Host verified; bench open | Bounce, false-start, timeout, wrap, replay |
| 006 | Simon | Host verified; bench open | Stable sequence vectors and full input replay |
| 009 | Adaptive night light | Host verified; bench open | Calibration, filtering, hysteresis |
| 012 | Traffic junction | Host verified; bench open | No conflicting greens; all-red failure |
| 015 | Environmental station | Host verified; bench open | Sensor validity and stable records |
| 018 | Inert access trainer | Host verified; bench open | Lockout and corrupt-state recovery |
| 021 | Bench rover | Host verified; bench open | Scripted route and emergency stop |
| 024 | Greenhouse controller | Host verified; bench open | Schedules, faults, simulated loads |
| 027 | Telemetry console | Host verified; bench open | Receive-only observations and logging |
| 030 | Inert show-cue simulator | Host verified; bench open | Dry run, abort, faults, audit trail |
| 033 | Calibration console | Host verified; bench open | Preview, trim, atomic commit/cancel, replay |

The final project never controls an igniter, launcher, or cloned transmitter.
See the [safety policy](../safety.md), [curriculum](../docs/CURRICULUM.md), and
[full project briefs](../docs/PROJECTS.md).
