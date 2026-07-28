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

ADK is early-stage software. Lessons 001--042 use the first-class per-object
RAII interfaces. Their APIs pass deterministic host tests and their canonical
examples compile for the Mega 2560, but physical acceptance remains open.
Lessons 037--039 use documented external reference fixtures; incoming
conformance and bench acceptance remain open. Lessons 040--042 publish
hardware-neutral optical, presence, and course-marshal policy; powered
adapters and exact specimens remain gated.

The hierarchy is developed in dependency order:

- hardware endpoints own pins, buses, timers, and interrupt claims;
- circuit components give endpoints physical meaning; and
- behaviors coordinate components without taking ownership of their physics.

The per-object lifecycle uses transactional `initialize()`, idempotent
`shutdown() noexcept`, and destructor-driven cleanup. The original
global-registration preview is frozen under [Legacy](legacy/index.md).

The latest promoted arc is Lessons 040--042: source-specific optical evidence,
bounded presence composition, and explicitly authorized course timing. The
planned sequence from Lesson 043 is engagement-first: balance, kinetic motion,
and a parts carousel lead the remaining sensor and display arcs. See the
[course map](course.md) for that order and the
[roadmap](docs/ROADMAP.md) for the full sequence.

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
