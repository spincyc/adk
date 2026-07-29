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

ADK is early-stage software. Lessons 001--058 use the first-class per-object
RAII interfaces. Their APIs pass deterministic host tests and their canonical
examples compile for the Mega 2560, but physical acceptance remains open.
Lessons 037--039 use documented external reference fixtures; incoming
conformance and bench acceptance remain open. Lessons 040--042 publish
hardware-neutral optical, presence, and course-marshal policy; powered
adapters and exact specimens remain gated. Lessons 043--045 publish E0
copied-value inertial, orientation, and stationary balance-table policy; no
powered sensor adapter, wiring, or physical measurement is claimed.
Lessons 046--048 publish copied tactile and directional intent, bounded
logical step sequencing, and transactional kinetic-light composition at E0.
They own no live input, motor driver, coil, timer, or moving hardware.
Lessons 049--051 publish synthetic local identity, bounded logical homing, and
an inert parts-carousel transaction at E0. They own no RFID, keypad, home or
stop endpoint, durable-media transport, energized actuator, or mechanism.
Lessons 052--054 publish host-verified E0 copied infrared receive evidence,
bounded inert carrier-envelope intent from an immutable local catalog, and a
fixed allowlisted translator. Their E0 boundary owns no
receiver, emitter, pin, timer, interrupt, carrier endpoint, optical power path,
or controlled device.
Lessons 055--057 publish a fixed clue-constraint model, a copied-image
fault-aware operator panel, and their inert escape-console composition. Their
E0 boundary owns no live input, storage transport, display, latch, lamp,
relay, servo, lock, or access-control mechanism.

The hierarchy is developed in dependency order:

- hardware endpoints own pins, buses, timers, and interrupt claims;
- circuit components give endpoints physical meaning; and
- behaviors coordinate components without taking ownership of their physics.

The per-object lifecycle uses transactional `initialize()`, idempotent
`shutdown() noexcept`, and destructor-driven cleanup. The original
global-registration preview is frozen under [Legacy](legacy/index.md).

The latest completed project arc is Lessons 055--057: copied clue evidence, a
fault-aware operator model, and an inert escape-console transaction that
cannot command physical access or actuation. The planned sequence from
Lesson 058 now publishes the engagement-first multiplex policy. See the
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
