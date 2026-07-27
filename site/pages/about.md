# About ADK

ADK is a small C++ library and progressive electronics course for the Arduino
Mega 2560. It treats circuit parts as objects with explicit resource ownership,
safe lifetimes, deterministic behavior, native tests, and a complete physical
lesson for every interface.

The teaching method repeats deliberately:

1. predict the circuit's behavior;
2. build it inside a stated safety boundary;
3. observe and measure it;
4. diagnose discrepancies;
5. support a claim with recorded evidence; and
6. compose the proven component into the next circuit.

## Current status

ADK is early-stage software. Lessons 001--003 cover the built-in LED, two
independent LEDs, and a common-cathode RGB LED. They compile against an imported
compatibility API based on global registration and `adk::initialize()`.

The target hierarchy is being developed in dependency order:

- hardware endpoints own pins, buses, timers, and interrupt claims;
- circuit components give endpoints physical meaning; and
- behaviors coordinate components without taking ownership of their physics.

The intended per-object lifecycle is transactional `initialize()`, idempotent
`shutdown() noexcept`, and destructor-driven cleanup. Documentation marks this
as a target until the replacement interfaces and lessons are complete.

The next hierarchy begins with lifecycle/resource claims, `DigitalInput`, and
`Button`. A deterministic four-button, four-LED Simon simulator is the midpoint
composition project. See the [roadmap](docs/ROADMAP.md) for the full
sequence.

## Principles

- Mega 2560 hardware first, with portability expressed through honest board
  profiles.
- Composition before inheritance.
- No internal exceptions, RTTI dependency, or dynamic allocation.
- Explicit clocks, inputs, seeds, and replayable traces.
- Clean public headers and size-conscious out-of-line implementations.
- Host tests and physical evidence are complementary, not interchangeable.
- Safety boundaries are part of each interface and lesson.

ADK is available under the [MIT License](license.md).
