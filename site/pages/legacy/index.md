# Legacy compatibility preview

This section records ADK's original compatibility preview. It is frozen,
unsupported, and retained only so early sketches and lessons remain
understandable.

Do not start new code with these interfaces. They predate component-owned
resources, transactional initialization, deterministic updates, explicit safe
shutdown, and host-testable hardware adapters. The supported API replaces them
rather than preserving source compatibility.

## What remains useful

- the predict → build → observe → diagnose → explain lesson method;
- conservative wiring and current-limiting guidance;
- measurement records and fault-isolation exercises; and
- the original sketches as migration examples.

## Frozen material

- [Legacy API notes](api.md)
- [Lesson 001: built-in LED](lessons/001.md)
- [Lesson 002: two independent LEDs](lessons/002.md)
- [Lesson 003: RGB PWM](lessons/003.md)

Archived downloads live under `downloads/legacy/`. They are historical inputs,
not supported library examples.

