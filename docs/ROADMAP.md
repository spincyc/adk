# Roadmap

`CURRICULUM.md` is the canonical lesson and project numbering authority.
`COMPONENTS.md` owns the target component inventory.

## Current host-verified slices

1. no-exception status, wrap-safe time, fixed resource claims, and runtime;
2. `DigitalOutput` and `MonoLed` for early visible diagnostics;
3. `DigitalInput` and deterministic `Button`;
4. deterministic Reaction Timer with lessons 001–003;
5. `PwmOutput`, `RgbLed`, and shared default-timer leases;
6. timer ownership and the nonblocking `PiezoSounder`;
7. deterministic `Simon`, fixed and seeded cue sources, and lessons 004–006.

The component APIs and behavior engines pass deterministic host tests and
compile for the Mega 2560. The complete Simon hardware adapter and physical
acceptance cards remain open, so this work is experimental rather than
hardware supported.

## Next slice

1. `AnalogInput`, potentiometer calibration, and explicit sample validity;
2. photoresistor and thermistor filtering from supplied samples;
3. adaptive night light at lesson 009.

Later slices follow the canonical three-lesson cadence through analog sensing,
displays, environmental records, bounded actuators, buses and storage,
receive-only observations, and the inert show-cue simulator.

Every component requires lifecycle tests, deterministic fakes, a canonical Mega
example, size evidence, terse HTML, a rich complementary PDF, and recorded
hardware acceptance. Work that misses a gate remains experimental.

ADK will not implement pyrotechnic ignition, launcher control, remote cloning,
or unknown-protocol transmission.
