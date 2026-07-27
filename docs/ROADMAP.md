# Roadmap

`CURRICULUM.md` is the canonical lesson and project numbering authority.
`COMPONENTS.md` owns the target component inventory.

## Current host-verified slice

1. no-exception status, wrap-safe time, fixed resource claims, and runtime;
2. `DigitalOutput` and `MonoLed` for early visible diagnostics;
3. `DigitalInput` and deterministic `Button`;
4. deterministic Reaction Timer with lessons 001–003.

Physical Mega 2560 acceptance remains required before this slice is called
hardware supported.

## Next slice

1. `PwmOutput` and `RgbLed`;
2. timer ownership and passive piezo sound;
3. deterministic Simon at lesson 006.

Later slices follow the canonical three-lesson cadence through analog sensing,
displays, environmental records, bounded actuators, buses and storage,
receive-only observations, and the inert show-cue simulator.

Every component requires lifecycle tests, deterministic fakes, a canonical Mega
example, size evidence, terse HTML, a rich complementary PDF, and recorded
hardware acceptance. Work that misses a gate remains experimental.

ADK will not implement pyrotechnic ignition, launcher control, remote cloning,
or unknown-protocol transmission.
